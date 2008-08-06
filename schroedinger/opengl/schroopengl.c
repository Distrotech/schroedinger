
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <schroedinger/schro.h>
#include <schroedinger/opengl/schroopengl.h>
#include <schroedinger/opengl/schroopenglcanvas.h>
#include <schroedinger/opengl/schroopenglmotion.h>
#include <schroedinger/opengl/schroopenglshader.h>
#include <limits.h>
#include <GL/glew.h>
#include <GL/glxew.h>

/* OBMC in biref precision 2 and 3 mode needs 10 textures at the same time */
#define SCHRO_OPENGL_REQUIRED_TEXTURE_UNITS 10

struct _SchroOpenGL {
  int is_initialized;
  int is_usable;
  int is_visible;

  SchroMutex *context_mutex;
  int context_lock_count;

  SchroMutex *canvas_pool_mutex;
  int canvas_pool_lock_count;

  Display *display;
  Window root;
  int screen;
  XVisualInfo *visual_info;
  GLXContext context;
  Window window;

  void *tmp;
  int tmp_size;

  SchroOpenGLShaderLibrary *shader_library;

  SchroOpenGLCanvasPool* canvas_pool;

  SchroOpenGLSpatialWeightPool *spatial_weight_pool;
};

static int
schro_opengl_x_error_handler (Display *display, XErrorEvent *event)
{
  char message[512];

  XGetErrorText (display, event->error_code, message, sizeof (message));

  SCHRO_ERROR ("Xlib error: %s", message);
  SCHRO_ASSERT (0);

  return 0;
}

static int
schro_opengl_open_display (SchroOpenGL *opengl, const char *display_name)
{
  SCHRO_ASSERT (opengl->display == NULL);

  opengl->display = XOpenDisplay (display_name);

  if (opengl->display == NULL) {
    SCHRO_ERROR ("failed to open display '%s'", display_name);
    return FALSE;
  }

  XSynchronize (opengl->display, True); // True for debug, False for speed
  XSetErrorHandler (schro_opengl_x_error_handler);

  opengl->root = DefaultRootWindow (opengl->display);
  opengl->screen = DefaultScreen (opengl->display);

  return TRUE;
}

static void
schro_opengl_close_display (SchroOpenGL *opengl)
{
  if (opengl->display) {
    XCloseDisplay (opengl->display);
    opengl->display = NULL;
  }
}

static int
schro_opengl_create_window (SchroOpenGL *opengl)
{
  int error_base;
  int event_base;
  int result;
  int visual_attr[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_RED_SIZE, 8,
      GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, None };
  int mask;
  XSetWindowAttributes window_attr;

  SCHRO_ASSERT (opengl->display != NULL);
  SCHRO_ASSERT (opengl->root != None);

  result = glXQueryExtension (opengl->display, &error_base, &event_base);

  if (!result) {
    SCHRO_ERROR ("missing GLX extension");
    return FALSE;
  }

  opengl->visual_info = glXChooseVisual (opengl->display, opengl->screen,
      visual_attr);

  if (opengl->visual_info == NULL) {
    SCHRO_ERROR ("no usable visual");
    return FALSE;
  }

  opengl->context = glXCreateContext (opengl->display, opengl->visual_info,
      NULL, True);

  if (opengl->context == NULL) {
    SCHRO_ERROR ("failed to create direct GLX context");

    XFree (opengl->visual_info);
    opengl->visual_info = NULL;

    return FALSE;
  }

  mask = CWBackPixel | CWBorderPixel | CWColormap | CWOverrideRedirect;

  window_attr.background_pixel = 0;
  window_attr.border_pixel = 0;
  window_attr.colormap = XCreateColormap (opengl->display, opengl->root,
      opengl->visual_info->visual, AllocNone);
  window_attr.override_redirect = False;

  opengl->window = XCreateWindow (opengl->display, opengl->root, 0, 0,
      100, 100, 0, opengl->visual_info->depth, InputOutput,
      opengl->visual_info->visual, mask, &window_attr);

  if (opengl->window == None) {
    SCHRO_ERROR ("failed to create window with visual %ld",
        opengl->visual_info->visualid);

    glXDestroyContext (opengl->display, opengl->context);
    opengl->context = NULL;

    XFree (opengl->visual_info);
    opengl->visual_info = NULL;

    return FALSE;
  }

  XSync (opengl->display, FALSE);

  return TRUE;
}

static void
schro_opengl_destroy_window (SchroOpenGL *opengl)
{
  if (opengl->window != None) {
    XDestroyWindow (opengl->display, opengl->window);
    opengl->window = None;
  }

  if (opengl->context) {
    glXDestroyContext (opengl->display, opengl->context);
    opengl->context = NULL;
  }

  if (opengl->visual_info) {
    XFree (opengl->visual_info);
    opengl->visual_info = NULL;
  }
}

static int
schro_opengl_init_glew (SchroOpenGL *opengl)
{
  int ok = TRUE;
  int major, minor, micro;
  GLenum error;

  schro_opengl_lock_context (opengl);

  error = glewInit ();

  if (error != GLEW_OK) {
    SCHRO_ERROR ("GLEW error: %s", glewGetErrorString (error));
    ok = FALSE;
  }

  major = atoi ((const char*) glewGetString (GLEW_VERSION_MAJOR));
  minor = atoi ((const char*) glewGetString (GLEW_VERSION_MINOR));
  micro = atoi ((const char*) glewGetString (GLEW_VERSION_MICRO));

  if (major < 1) {
    SCHRO_ERROR ("missing GLEW >= 1.5.0");
    ok = FALSE;
  } else if (major == 1 && minor < 5) {
    SCHRO_ERROR ("missing GLEW >= 1.5.0");
    ok = FALSE;
  } else if (major == 1 && minor == 5 && micro < 0) {
    SCHRO_ERROR ("missing GLEW >= 1.5.0");
    ok = FALSE;
  }

  schro_opengl_unlock_context (opengl);

  return ok;
}

static int
schro_opengl_check_essential_extensions (SchroOpenGL *opengl)
{
  int ok = TRUE;
  GLint texture_units;

  schro_opengl_lock_context (opengl);

  #define CHECK_EXTENSION(_name) \
    if (!GLEW_##_name) { \
      SCHRO_ERROR ("missing essential extension GL_" #_name); \
      ok = FALSE; \
    }

  #define CHECK_EXTENSION_GROUPS(_group1, _group2, _name) \
    if (!GLEW_##_group1##_##_name && !GLEW_##_group2##_##_name) { \
      SCHRO_ERROR ("missing essential extension GL_{" #_group1 "|"  #_group2 \
          "}_" #_name); \
      ok = FALSE; \
    }

  CHECK_EXTENSION (EXT_framebuffer_object)
  CHECK_EXTENSION_GROUPS (ARB, NV, texture_rectangle)
  CHECK_EXTENSION (ARB_multitexture)
  CHECK_EXTENSION (ARB_shader_objects)
  CHECK_EXTENSION (ARB_shading_language_100)
  CHECK_EXTENSION (ARB_fragment_shader)
  CHECK_EXTENSION (ARB_fragment_program) /* GL_MAX_TEXTURE_IMAGE_UNITS_ARB */

  #undef CHECK_EXTENSION
  #undef CHECK_EXTENSION_GROUPS

  if (ok) {
    glGetIntegerv (GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &texture_units);

    if (texture_units < SCHRO_OPENGL_REQUIRED_TEXTURE_UNITS) {
      SCHRO_ERROR ("GL_MAX_TEXTURE_IMAGE_UNITS_ARB >= %i required, have %i",
          SCHRO_OPENGL_REQUIRED_TEXTURE_UNITS, texture_units);

      ok = FALSE;
    }
  }

  schro_opengl_canvas_check_flags ();

  schro_opengl_unlock_context (opengl);

  return ok;
}

void schro_opengl_init (void)
{
#if SCHRO_OPENGL_XLOCKDISPLAY
  XInitThreads ();
#endif
}

SchroOpenGL *
schro_opengl_new (void)
{
  SchroOpenGL *opengl = schro_malloc0 (sizeof (SchroOpenGL));

  opengl->is_initialized = FALSE;
  opengl->is_usable = TRUE;
  opengl->is_visible = FALSE;
  opengl->context_mutex = schro_mutex_new_recursive ();
  opengl->context_lock_count = 0;
  opengl->canvas_pool_mutex = schro_mutex_new_recursive ();
  opengl->canvas_pool_lock_count = 0;
  opengl->display = NULL;
  opengl->root = None;
  opengl->screen = 0;
  opengl->visual_info = NULL;
  opengl->context = NULL;
  opengl->window = None;
  opengl->tmp = NULL;
  opengl->tmp_size = 0;
  opengl->shader_library = NULL;
  opengl->canvas_pool = NULL;
  opengl->spatial_weight_pool = NULL;

  if (!schro_opengl_open_display (opengl, getenv ("SCHRO_OPENGL_DISPLAY"))) {
    opengl->is_usable = FALSE;
    return opengl;
  }

  if (!schro_opengl_create_window (opengl)) {
    opengl->is_usable = FALSE;
    return opengl;
  }

  if (!schro_opengl_init_glew (opengl)) {
    opengl->is_usable = FALSE;
    return opengl;
  }

  if (!schro_opengl_check_essential_extensions (opengl)) {
    opengl->is_usable = FALSE;
    return opengl;
  }

  opengl->shader_library = schro_opengl_shader_library_new (opengl);
  opengl->canvas_pool = schro_opengl_canvas_pool_new (opengl);
  opengl->spatial_weight_pool = schro_opengl_spatial_weight_pool_new ();

  schro_opengl_lock_context (opengl);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  glEnable (GL_TEXTURE_RECTANGLE_ARB);

  schro_opengl_unlock_context (opengl);

  opengl->is_initialized = TRUE;

  //schro_opengl_set_visible (opengl, TRUE);

  return opengl;
}

void
schro_opengl_free (SchroOpenGL *opengl)
{
  SCHRO_ASSERT (opengl->context_lock_count == 0);
  SCHRO_ASSERT (opengl->canvas_pool_lock_count == 0);

  if (opengl->shader_library) {
    schro_opengl_shader_library_free (opengl->shader_library);
    opengl->shader_library = NULL;
  }

  if (opengl->spatial_weight_pool) {
    schro_opengl_spatial_weight_pool_free (opengl->spatial_weight_pool);
    opengl->spatial_weight_pool = NULL;
  }

  if (opengl->canvas_pool) {
    schro_opengl_canvas_pool_free (opengl->canvas_pool);
    opengl->canvas_pool = NULL;
  }

  SCHRO_ASSERT (opengl->context_lock_count == 0);
  SCHRO_ASSERT (opengl->canvas_pool_lock_count == 0);

  schro_opengl_destroy_window (opengl);
  schro_opengl_close_display (opengl);

  if (opengl->tmp) {
    schro_free (opengl->tmp);
    opengl->tmp = NULL;
  }

  schro_mutex_free (opengl->context_mutex);
  schro_mutex_free (opengl->canvas_pool_mutex);

  schro_free (opengl);
}

int
schro_opengl_is_usable (SchroOpenGL *opengl) {
  return opengl->is_usable;
}

void
schro_opengl_lock_context (SchroOpenGL *opengl)
{
  SCHRO_ASSERT (opengl->display != NULL);
  SCHRO_ASSERT (opengl->window != None);
  SCHRO_ASSERT (opengl->context != NULL);
  SCHRO_ASSERT (opengl->context_lock_count < (INT_MAX - 1));

  schro_mutex_lock (opengl->context_mutex);

  if (opengl->context_lock_count == 0) {
#if SCHRO_OPENGL_XLOCKDISPLAY
    XLockDisplay (opengl->display);
#endif

    if (!glXMakeCurrent (opengl->display, opengl->window, opengl->context)) {
      SCHRO_ERROR ("glXMakeCurrent failed");
    }

#if SCHRO_OPENGL_XLOCKDISPLAY
    XUnlockDisplay (opengl->display);
#endif
  }

  ++opengl->context_lock_count;

  SCHRO_OPENGL_CHECK_ERROR
}

void
schro_opengl_unlock_context (SchroOpenGL *opengl)
{
#if SCHRO_OPENGL_UNBIND_TEXTURES
  int i;
  GLint texture;
#endif
  GLint framebuffer;
  GLint active_texture_unit;

  SCHRO_ASSERT (opengl->display != NULL);
  SCHRO_ASSERT (opengl->context_lock_count > 0);

  SCHRO_OPENGL_CHECK_ERROR

  --opengl->context_lock_count;

  if (opengl->context_lock_count == 0) {
    if (opengl->is_initialized) {
      glGetIntegerv (GL_ACTIVE_TEXTURE_ARB, &active_texture_unit);

      SCHRO_ASSERT (active_texture_unit == GL_TEXTURE0_ARB);

#if SCHRO_OPENGL_UNBIND_TEXTURES
      for (i = 0; i < SCHRO_OPENGL_REQUIRED_TEXTURE_UNITS; ++i) {
        glActiveTextureARB (GL_TEXTURE0_ARB + i);
        glGetIntegerv (GL_TEXTURE_BINDING_RECTANGLE_ARB, &texture);

        SCHRO_ASSERT (!glIsTexture (texture));
        SCHRO_ASSERT (texture == 0);
      }

      glActiveTextureARB (GL_TEXTURE0_ARB);
#endif

      glGetIntegerv (GL_FRAMEBUFFER_BINDING_EXT, &framebuffer);

      SCHRO_ASSERT (!glIsFramebufferEXT (framebuffer));
      SCHRO_ASSERT (framebuffer == 0);

      if (GLEW_EXT_framebuffer_blit) {
        glGetIntegerv (GL_READ_FRAMEBUFFER_BINDING_EXT, &framebuffer);

        SCHRO_ASSERT (!glIsFramebufferEXT (framebuffer));
        SCHRO_ASSERT (framebuffer == 0);

        glGetIntegerv (GL_DRAW_FRAMEBUFFER_BINDING_EXT, &framebuffer);

        SCHRO_ASSERT (!glIsFramebufferEXT (framebuffer));
        SCHRO_ASSERT (framebuffer == 0);
      }
    }

#if SCHRO_OPENGL_XLOCKDISPLAY
    XLockDisplay (opengl->display);
#endif

    if (!glXMakeCurrent (opengl->display, None, NULL)) {
      SCHRO_ERROR ("glXMakeCurrent failed");
    }

#if SCHRO_OPENGL_XLOCKDISPLAY
    XUnlockDisplay (opengl->display);
#endif
  }

  schro_mutex_unlock (opengl->context_mutex);
}

void
schro_opengl_lock_canvas_pool (SchroOpenGL *opengl)
{
  SCHRO_ASSERT (opengl->canvas_pool_lock_count < (INT_MAX - 1));

  schro_mutex_lock (opengl->canvas_pool_mutex);

  ++opengl->canvas_pool_lock_count;
}

void
schro_opengl_unlock_canvas_pool (SchroOpenGL *opengl)
{
  SCHRO_ASSERT (opengl->canvas_pool_lock_count > 0);

  --opengl->canvas_pool_lock_count;

  schro_mutex_unlock (opengl->canvas_pool_mutex);
}

void
schro_opengl_check_error (const char* file, int line, const char* func)
{
  GLenum error = glGetError ();

  if (error) {
    SCHRO_ERROR ("GL Error 0x%04x in %s(%d) %s", (int) error, file, line, func);
    SCHRO_ASSERT (0);
  }
}

void
schro_opengl_check_framebuffer (const char *file, int line, const char *func)
{
  switch (glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT)) {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
      return;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
      SCHRO_ERROR ("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT in %s(%d) %s",
          file, line, func);
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      SCHRO_ERROR ("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT in "
          "%s(%d) %s", file, line, func);
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      SCHRO_ERROR ("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT in %s(%d) %s",
          file, line, func);
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
      SCHRO_ERROR ("GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT in %s(%d) %s",
          file, line, func);
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      SCHRO_ERROR ("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT in %s(%d) %s",
          file, line, func);
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      SCHRO_ERROR ("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT in %s(%d) %s",
          file, line, func);
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      SCHRO_ERROR ("GL_FRAMEBUFFER_UNSUPPORTED_EXT in %s(%d)", file, line,
         func);
      break;
    default:
      SCHRO_ERROR ("unknown error from glCheckFramebufferStatusEXT in "
          "%s(%d) %s", file, line, func);
      break;
  }

  SCHRO_ASSERT (0);
}

void
schro_opengl_set_visible (SchroOpenGL *opengl, int visible)
{
  if (opengl->is_visible == visible) {
    return;
  }

  opengl->is_visible = visible;

  if (opengl->is_visible) {
    XMapWindow (opengl->display, opengl->window);
  } else {
    XUnmapWindow (opengl->display, opengl->window);
  }

  XSync (opengl->display, FALSE);
}

void
schro_opengl_setup_viewport (int width, int height)
{
  glViewport (0, 0, width, height);

  glLoadIdentity ();
  glOrtho (0, width, 0, height, -1, 1);
}

void
schro_opengl_render_quad (int x, int y, int width, int height)
{
  glBegin (GL_QUADS);
  glTexCoord2f (x,         y);          glVertex3f (x,         y,          0);
  glTexCoord2f (x + width, y);          glVertex3f (x + width, y,          0);
  glTexCoord2f (x + width, y + height); glVertex3f (x + width, y + height, 0);
  glTexCoord2f (x,         y + height); glVertex3f (x,         y + height, 0);
  glEnd ();
}

void *
schro_opengl_get_tmp (SchroOpenGL *opengl, int size)
{
  SCHRO_ASSERT (size > 0);

  if (opengl->tmp_size < size || !opengl->tmp) {
    opengl->tmp_size = size;

    if (!opengl->tmp) {
      opengl->tmp = schro_malloc (opengl->tmp_size);
    } else {
      opengl->tmp = schro_realloc (opengl->tmp, opengl->tmp_size);
    }
  }

  return opengl->tmp;
}

SchroOpenGLShaderLibrary *
schro_opengl_get_shader_library (SchroOpenGL *opengl)
{
  return opengl->shader_library;
}

SchroOpenGLCanvasPool *
schro_opengl_get_canvas_pool (SchroOpenGL *opengl)
{
  return opengl->canvas_pool;
}

SchroOpenGLSpatialWeightPool *
schro_opengl_get_spatial_weight_pool (SchroOpenGL *opengl)
{
  return opengl->spatial_weight_pool;
}

void
schro_opengl_squeeze_pools (SchroOpenGL *opengl)
{
  schro_opengl_canvas_pool_squeeze (opengl->canvas_pool);
  schro_opengl_spatial_weight_pool_squeeze (opengl->spatial_weight_pool);
}

static void *
schro_opengl_domain_alloc (int size)
{
  return schro_malloc0 (size);
}

static void
schro_opengl_domain_free (void *ptr, int size)
{
  schro_free (ptr);
}

SchroMemoryDomain *
schro_memory_domain_new_opengl (void)
{
  SchroMemoryDomain *domain;

  domain = schro_memory_domain_new ();
  domain->flags = SCHRO_MEMORY_DOMAIN_OPENGL;
  domain->alloc = schro_opengl_domain_alloc;
  domain->free = schro_opengl_domain_free;

  return domain;
}

