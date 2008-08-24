
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <schroedinger/schrodebug.h>
#include <schroedinger/opengl/schroopengl.h>
#include <schroedinger/opengl/schroopenglcanvas.h>
#include <stdio.h>

uint32_t _schro_opengl_canvas_flags
    = 0
    | SCHRO_OPENGL_CANVAS_STORE_PACKED_AS_RGBA
    //| SCHRO_OPENGL_CANVAS_STORE_PACKED_AS_BGRA /* FIXME: currently broken in convert */

    //| SCHRO_OPENGL_CANVAS_STORE_U8_AS_U8
    //| SCHRO_OPENGL_CANVAS_STORE_U8_AS_F16
    //| SCHRO_OPENGL_CANVAS_STORE_U8_AS_F32
    | SCHRO_OPENGL_CANVAS_STORE_U8_AS_UI8

    //| SCHRO_OPENGL_CANVAS_STORE_S16_AS_S16
    //| SCHRO_OPENGL_CANVAS_STORE_S16_AS_U16
    /* FIXME: STORE_S16_AS_F16 is currently broken, simply pushing/pulling
       random S16 values breaks and doesn't give the values back that have be
       pushed, there are small errors */
    //| SCHRO_OPENGL_CANVAS_STORE_S16_AS_F16
    //| SCHRO_OPENGL_CANVAS_STORE_S16_AS_F32
    //| SCHRO_OPENGL_CANVAS_STORE_S16_AS_I16 /* FIXME: doesn't yield a useable mapping in shader */
    | SCHRO_OPENGL_CANVAS_STORE_S16_AS_UI16


    | SCHRO_OPENGL_CANVAS_PUSH_SUBIMAGE
    //| SCHRO_OPENGL_CANVAS_PUSH_RENDER_QUAD
    //| SCHRO_OPENGL_CANVAS_PUSH_DRAWPIXELS /* FIXME: currently broken */

    //| SCHRO_OPENGL_CANVAS_PUSH_U8_PIXELBUFFER
    //| SCHRO_OPENGL_CANVAS_PUSH_S16_PIXELBUFFER

    | SCHRO_OPENGL_CANVAS_PUSH_U8_AS_U8
    //| SCHRO_OPENGL_CANVAS_PUSH_U8_AS_F32

    //| SCHRO_OPENGL_CANVAS_PUSH_S16_AS_S16
    | SCHRO_OPENGL_CANVAS_PUSH_S16_AS_U16
    //| SCHRO_OPENGL_CANVAS_PUSH_S16_AS_F32


    //| SCHRO_OPENGL_CANVAS_PULL_U8_PIXELBUFFER
    //| SCHRO_OPENGL_CANVAS_PULL_S16_PIXELBUFFER

    | SCHRO_OPENGL_CANVAS_PULL_U8_AS_U8
    //| SCHRO_OPENGL_CANVAS_PULL_U8_AS_F32

    //| SCHRO_OPENGL_CANVAS_PULL_S16_AS_S16
    | SCHRO_OPENGL_CANVAS_PULL_S16_AS_U16
    //| SCHRO_OPENGL_CANVAS_PULL_S16_AS_F32
    ;

/* results on a NVIDIA 8800 GT with nvidia-glx-new drivers on Ubuntu Hardy */

/* U8: 259.028421/502.960679 mbyte/sec *//*
unsigned int _schro_opengl_canvas_flags
    = 0;*/

/* U8: 382.692291/447.573619 mbyte/sec *//*
unsigned int _schro_opengl_canvas_flags
    = SCHRO_OPENGL_CANVAS_PUSH_RENDER_QUAD
    | SCHRO_OPENGL_CANVAS_PUSH_U8_PIXELBUFFER;*/

/* U8: 972.809028/962.217704 mbyte/sec *//*
unsigned int _schro_opengl_canvas_flags
    = SCHRO_OPENGL_CANVAS_STORE_U8_AS_UI8;*/

/* U8: 1890.699986/848.954058 mbyte/sec *//*
unsigned int _schro_opengl_canvas_flags
    = SCHRO_OPENGL_CANVAS_STORE_U8_AS_UI8
    | SCHRO_OPENGL_CANVAS_PUSH_U8_PIXELBUFFER;*/

/* U8: 2003.478261/462.976159 mbyte/sec *//*
unsigned int _schro_opengl_canvas_flags
    = SCHRO_OPENGL_CANVAS_PUSH_U8_PIXELBUFFER;*/

/* S16: 22.265474/492.245509 mbyte/sec *//*
unsigned int _schro_opengl_canvas_flags
    = SCHRO_OPENGL_CANVAS_PUSH_S16_AS_U16
    | SCHRO_OPENGL_CANVAS_PUSH_S16_PIXELBUFFER
    | SCHRO_OPENGL_CANVAS_PULL_S16_AS_U16;*/

/* S16: 85.136173/499.591624 mbyte/sec *//*
unsigned int _schro_opengl_canvas_flags
    = SCHRO_OPENGL_CANVAS_PULL_S16_AS_U16;*/

/* S16: 266.568537/490.034023 mbyte/sec *//*
unsigned int _schro_opengl_canvas_flags
    = SCHRO_OPENGL_CANVAS_PUSH_S16_AS_U16
    | SCHRO_OPENGL_CANVAS_PULL_S16_AS_U16;*/

/* S16: 601.249413/914.319981 mbyte/sec *//*
unsigned int _schro_opengl_canvas_flags
    = SCHRO_OPENGL_CANVAS_STORE_S16_AS_UI16
    | SCHRO_OPENGL_CANVAS_PUSH_S16_AS_U16
    | SCHRO_OPENGL_CANVAS_PULL_S16_AS_U16;*/

/**
 * schro_opengl_canvas_setup_flags:
 *
 * Set canvas flags depending on available OpenGL extensions.  GLEW must be
 * initialized before calling this function.
 */
void
schro_opengl_canvas_setup_flags (void)
{
  _schro_opengl_canvas_flags = 0;

  SCHRO_OPENGL_CANVAS_SET_FLAG (STORE_PACKED_AS_RGBA);

  SCHRO_OPENGL_CANVAS_SET_FLAG (PUSH_SUBIMAGE);

  if (GLEW_ARB_vertex_buffer_object && GLEW_ARB_pixel_buffer_object) {
    SCHRO_OPENGL_CANVAS_SET_FLAG (PUSH_U8_PIXELBUFFER);
  }

  if (GLEW_EXT_texture_integer && GLEW_EXT_gpu_shader4) {
    SCHRO_OPENGL_CANVAS_SET_FLAG (STORE_U8_AS_UI8);
    SCHRO_OPENGL_CANVAS_SET_FLAG (PUSH_U8_AS_U8);
    SCHRO_OPENGL_CANVAS_SET_FLAG (PULL_U8_AS_U8);

    SCHRO_OPENGL_CANVAS_SET_FLAG (STORE_S16_AS_UI16);
    SCHRO_OPENGL_CANVAS_SET_FLAG (PUSH_S16_AS_U16);
    SCHRO_OPENGL_CANVAS_SET_FLAG (PULL_S16_AS_U16);
  } else if (GLEW_ARB_texture_float || GLEW_ATI_texture_float) {
    SCHRO_OPENGL_CANVAS_SET_FLAG (STORE_U8_AS_F16);
    SCHRO_OPENGL_CANVAS_SET_FLAG (PUSH_U8_AS_U8);
    SCHRO_OPENGL_CANVAS_SET_FLAG (PULL_U8_AS_U8);

    SCHRO_OPENGL_CANVAS_SET_FLAG (STORE_S16_AS_F32);
    SCHRO_OPENGL_CANVAS_SET_FLAG (PUSH_S16_AS_U16);
    SCHRO_OPENGL_CANVAS_SET_FLAG (PULL_S16_AS_U16);
  } else {
    SCHRO_OPENGL_CANVAS_SET_FLAG (STORE_U8_AS_U8);
    SCHRO_OPENGL_CANVAS_SET_FLAG (PUSH_U8_AS_U8);
    SCHRO_OPENGL_CANVAS_SET_FLAG (PULL_U8_AS_U8);

    SCHRO_OPENGL_CANVAS_SET_FLAG (STORE_S16_AS_U16);
    SCHRO_OPENGL_CANVAS_SET_FLAG (PUSH_S16_AS_U16);
    SCHRO_OPENGL_CANVAS_SET_FLAG (PULL_S16_AS_U16);
  }

  schro_opengl_canvas_check_flags ();
}

/* canvas flag combinations, key:
       1 = must be set
       0 = must not be set
       - = indifferent
       # = the flag itself, may be set or not set idenpendent of other flags
   [a-z] = the flag itself, flags with the same letter form an exclusive group,
           exactly one flag in such a group must be set */
static const char *schro_opengl_canvas_flag_combinations[] = {
  "a-""----""------"   "---""--""--""---"   "--""--""---", // STORE_PACKED_AS_RGBA
  "-a""----""------"   "---""--""--""---"   "--""--""---", // STORE_PACKED_AS_BGRA

  "--""b---""----00"   "---""--""--""---"   "--""--""---", // STORE_U8_AS_U8
  "--""-b--""----00"   "---""--""--""---"   "--""--""---", // STORE_U8_AS_F16
  "--""--b-""----00"   "---""--""--""---"   "--""--""---", // STORE_U8_AS_F32
  "--""---b""0000--"   "---""--""1-""---"   "--""1-""---", // STORE_U8_AS_UI8

  "--""---0""c-----"   "---""--""--""---"   "--""--""0--", // STORE_S16_AS_S16
  "--""---0""-c----"   "---""--""--""---"   "--""--""0--", // STORE_S16_AS_U16
  "--""---0""--c---"   "---""--""--""---"   "--""--""0--", // STORE_S16_AS_F16
  "--""---0""---c--"   "---""--""--""---"   "--""--""0--", // STORE_S16_AS_F32
  "--""---1""----c-"   "---""--""--""1--"   "--""--""1--", // STORE_S16_AS_I16
  "--""---1""-----c"   "---""--""--""-1-"   "--""--""-1-", // STORE_S16_AS_UI16


  "--""----""------"   "d--""--""--""---"   "--""--""---", // PUSH_SUBIMAGE
  "--""----""------"   "-d-""--""--""---"   "--""--""---", // PUSH_RENDER_QUAD
  "--""----""------"   "--d""--""--""---"   "--""--""---", // PUSH_DRAWPIXELS

  "--""----""------"   "---""#-""--""---"   "--""--""---", // PUSH_U8_PIXELBUFFER
  "--""----""------"   "---""-#""--""---"   "--""--""---", // PUSH_S16_PIXELBUFFER

  "--""----""------"   "---""--""e-""---"   "--""--""---", // PUSH_U8_AS_U8
  "--""---0""------"   "---""--""-e""---"   "--""--""---", // PUSH_U8_AS_F32

  "--""----""----1-"   "---""--""--""f--"   "--""--""---", // PUSH_S16_AS_S16
  "--""----""------"   "---""--""--""-f-"   "--""--""---", // PUSH_S16_AS_U16
  "--""----""-----0"   "---""--""--""--f"   "--""--""---", // PUSH_S16_AS_F32


  "--""----""------"   "---""--""--""---"   "#-""--""---", // PULL_U8_PIXELBUFFER
  "--""----""------"   "---""--""--""---"   "-#""--""---", // PULL_S16_PIXELBUFFER

  "--""----""------"   "---""--""--""---"   "--""g-""---", // PULL_U8_AS_U8
  "--""---0""------"   "---""--""--""---"   "--""-g""---", // PULL_U8_AS_F32

  "--""----""----1-"   "---""--""--""---"   "--""--""h--", // PULL_S16_AS_S16
  "--""----""------"   "---""--""--""---"   "--""--""-h-", // PULL_S16_AS_U16
  "--""----""-----0"   "---""--""--""---"   "--""--""--h"  // PULL_S16_AS_F32
};

#define SCHRO_OPENGL_CANVAS_FLAG_GROUP_COUNT 8

static const char *schro_opengl_canvas_flag_names[] = {
  "store packed as RGBA",      // STORE_PACKED_AS_RGBA
  "store packed as BGRA",      // STORE_PACKED_AS_BGRA
  "store U8 as U8",            // STORE_U8_AS_U8
  "store U8 as F16",           // STORE_U8_AS_F16
  "store U8 as F32",           // STORE_U8_AS_F32
  "store U8 as UI8",           // STORE_U8_AS_UI8
  "store S16 as S16",          // STORE_S16_AS_S16
  "store S16 as U16",          // STORE_S16_AS_U16
  "store S16 as F16",          // STORE_S16_AS_F16
  "store S16 as F32",          // STORE_S16_AS_F32
  "store S16 as I16",          // STORE_S16_AS_I16
  "store S16 as UI16",         // STORE_S16_AS_UI16

  "push by subimage",          // PUSH_SUBIMAGE
  "push by render quad",       // PUSH_RENDER_QUAD
  "push by drawpixels",        // PUSH_DRAWPIXELS
  "push U8 with pixelbuffer",  // PUSH_U8_PIXELBUFFER
  "push S16 with pixelbuffer", // PUSH_S16_PIXELBUFFER
  "push U8 as U8",             // PUSH_U8_AS_U8
  "push U8 as F32",            // PUSH_U8_AS_F32
  "push S16 as S16",           // PUSH_S16_AS_S16
  "push S16 as U16",           // PUSH_S16_AS_U16
  "push S16 as F32",           // PUSH_S16_AS_F32

  "pull U8 with pixelbuffer",  // PULL_U8_PIXELBUFFER
  "pull S16 with pixelbuffer", // PULL_S16_PIXELBUFFER
  "pull U8 as U8",             // PULL_U8_AS_U8
  "pull U8 as F32",            // PULL_U8_AS_F32
  "pull S16 as S16",           // PULL_S16_AS_S16
  "pull S16 as U16",           // PULL_S16_AS_U16
  "pull S16 as F32"            // PULL_S16_AS_F32
};

typedef int (* SchroOpenGLExtensionFunc) (const char **name);

static int
schro_opengl_has_extension_bgra (const char **name)
{
  static const char *static_name = "EXT_bgra";

  *name = static_name;

  return GLEW_EXT_bgra;
}

static int
schro_opengl_has_extension_window_pos (const char **name)
{
  static const char *static_name = "ARB_window_pos";

  *name = static_name;

  return GLEW_ARB_window_pos;
}

static int
schro_opengl_has_extension_texture_float (const char **name)
{
  static const char *static_name = "ARB_texture_float or ATI_texture_float";

  *name = static_name;

  return GLEW_ARB_texture_float || GLEW_ATI_texture_float;
}

static int
schro_opengl_has_extension_texture_integer (const char **name)
{
  static const char *static_name = "EXT_texture_integer and EXT_gpu_shader4";

  *name = static_name;

  return GLEW_EXT_texture_integer && GLEW_EXT_gpu_shader4;
}

static int
schro_opengl_has_extension_pixelbuffer (const char **name)
{
  static const char *static_name
      = "ARB_vertex_buffer_object and ARB_pixel_buffer_object";

  *name = static_name;

  return GLEW_ARB_vertex_buffer_object && GLEW_ARB_pixel_buffer_object;
}

SchroOpenGLExtensionFunc schro_opengl_canvas_flag_extensions[] = {
  NULL,                                       // STORE_PACKED_AS_RGBA
  schro_opengl_has_extension_bgra,            // STORE_PACKED_AS_BGRA
  NULL,                                       // STORE_U8_AS_U8
  schro_opengl_has_extension_texture_float,   // STORE_U8_AS_F16
  schro_opengl_has_extension_texture_float,   // STORE_U8_AS_F32
  schro_opengl_has_extension_texture_integer, // STORE_U8_AS_UI8
  NULL,                                       // STORE_S16_AS_S16
  NULL,                                       // STORE_S16_AS_U16
  schro_opengl_has_extension_texture_float,   // STORE_S16_AS_F16
  schro_opengl_has_extension_texture_float,   // STORE_S16_AS_F32
  schro_opengl_has_extension_texture_integer, // STORE_S16_AS_I16
  schro_opengl_has_extension_texture_integer, // STORE_S16_AS_UI16

  NULL,                                       // PUSH_SUBIMAGE
  NULL,                                       // PUSH_RENDER_QUAD
  schro_opengl_has_extension_window_pos,      // PUSH_DRAWPIXELS
  schro_opengl_has_extension_pixelbuffer,     // PUSH_U8_PIXELBUFFER
  schro_opengl_has_extension_pixelbuffer,     // PUSH_S16_PIXELBUFFER
  NULL,                                       // PUSH_U8_AS_U8
  NULL,                                       // PUSH_U8_AS_F32
  NULL,                                       // PUSH_S16_AS_S16
  NULL,                                       // PUSH_S16_AS_U16
  NULL,                                       // PUSH_S16_AS_F32

  schro_opengl_has_extension_pixelbuffer,     // PULL_U8_PIXELBUFFER
  schro_opengl_has_extension_pixelbuffer,     // PULL_S16_PIXELBUFFER
  NULL,                                       // PULL_U8_AS_U8
  NULL,                                       // PULL_U8_AS_F32
  NULL,                                       // PULL_S16_AS_S16
  NULL,                                       // PULL_S16_AS_U16
  NULL                                        // PULL_S16_AS_F32
};

void
schro_opengl_canvas_check_flags (void)
{
  int i, k;
  int value;
  int flags_per_group[SCHRO_OPENGL_CANVAS_FLAG_GROUP_COUNT];
  const char* extention = NULL;

  for (i = 0; i < SCHRO_OPENGL_CANVAS_FLAG_GROUP_COUNT; i++) {
    flags_per_group[i] = 0;
  }

  for (i = 0; i < SCHRO_OPENGL_CANVAS_FLAG_COUNT; ++i) {
    if (_schro_opengl_canvas_flags & (1 << i)) {
      for (k = 0; k < SCHRO_OPENGL_CANVAS_FLAG_COUNT; ++k) {
        value = schro_opengl_canvas_flag_combinations[i][k];

        if (value >= 'a' && value <= 'z') {
          if (i != k) {
            SCHRO_ERROR ("'%c' at wrong index %i %i", value, i, k);
            SCHRO_ASSERT (0);
          }

          if (_schro_opengl_canvas_flags & (1 << k)) {
            ++flags_per_group[value - 'a'];
          }
        } else {
          switch (value) {
            case '1':
              if (!(_schro_opengl_canvas_flags & (1 << k))) {
                SCHRO_ERROR ("flag '%s' is cleared but should be set if "
                    "flag '%s' is set", schro_opengl_canvas_flag_names[k],
                    schro_opengl_canvas_flag_names[i]);
                SCHRO_ASSERT (0);
              }

              break;
            case '0':
              if (_schro_opengl_canvas_flags & (1 << k)) {
                SCHRO_ERROR ("flag '%s' is set but should be cleared if "
                    "flag '%s' is set", schro_opengl_canvas_flag_names[k],
                    schro_opengl_canvas_flag_names[i]);
                SCHRO_ASSERT (0);
              }

              break;
            case '-':
              break;
            case '#':
              if (i != k) {
                SCHRO_ERROR ("'#' at wrong index %i %i", i, k);
                SCHRO_ASSERT (0);
              }

              break;
            case '\0':
              SCHRO_ERROR ("out of bounds at index %i %i", i, k);
              SCHRO_ASSERT (0);
              break;
            default:
              SCHRO_ERROR ("invalid value '%c' at index %i %i", value, i, k);
              SCHRO_ASSERT (0);
              break;
          }
        }
      }
    }
  }

  for (i = 0; i < SCHRO_OPENGL_CANVAS_FLAG_GROUP_COUNT; i++) {
    if (flags_per_group[i] != 1) {
      SCHRO_ERROR ("%i flags in group '%c' set, exactly one must be set",
          flags_per_group[i], 'a' + i);
      SCHRO_ASSERT (0);
    }
  }

  for (i = 0; i < SCHRO_OPENGL_CANVAS_FLAG_COUNT; ++i) {
    if (_schro_opengl_canvas_flags & (1 << i) &&
        schro_opengl_canvas_flag_extensions[i]) {
      if (!schro_opengl_canvas_flag_extensions[i](&extention)) {
        SCHRO_ERROR ("flag '%s' depends on unsupported OpenGL extention '%s'",
            schro_opengl_canvas_flag_names[i], extention);
        SCHRO_ASSERT (0);
      }
    }
  }

  if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_PACKED_AS_BGRA)) {
    SCHRO_ERROR ("store packed as BGRA is currently broken");
    SCHRO_ASSERT (0);
  }

  if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_F16)) {
    SCHRO_ERROR ("store S16 as F16 is currently broken");
    SCHRO_ASSERT (0);
  }

  if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_I16)) {
    SCHRO_ERROR ("store S16 as I16 is currently broken");
    SCHRO_ASSERT (0);
  }

  if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_DRAWPIXELS)) {
    SCHRO_ERROR ("push by drawpixels is currently broken");
    SCHRO_ASSERT (0);
  }
}

void
schro_opengl_canvas_print_flags (const char* indent)
{
  schro_opengl_canvas_check_flags ();

  #define PRINT_FLAG(_text, _flag) \
      printf ("%s  "_text"%s\n", indent, \
          SCHRO_OPENGL_CANVAS_IS_FLAG_SET (_flag) ? "on" : "off")

  printf ("%sstore flags\n", indent);

  PRINT_FLAG ("packed as RGBA:  ", STORE_PACKED_AS_RGBA);
  PRINT_FLAG ("packed as BGRA:  ", STORE_PACKED_AS_BGRA);
  PRINT_FLAG ("U8 as U8:        ", STORE_U8_AS_U8);
  PRINT_FLAG ("U8 as F16:       ", STORE_U8_AS_F16);
  PRINT_FLAG ("U8 as F32:       ", STORE_U8_AS_F32);
  PRINT_FLAG ("U8 as UI8:       ", STORE_U8_AS_UI8);
  PRINT_FLAG ("S16 as S16:      ", STORE_S16_AS_S16);
  PRINT_FLAG ("S16 as U16:      ", STORE_S16_AS_U16);
  PRINT_FLAG ("S16 as F16:      ", STORE_S16_AS_F16);
  PRINT_FLAG ("S16 as F32:      ", STORE_S16_AS_F32);
  PRINT_FLAG ("S16 as I16:      ", STORE_S16_AS_I16);
  PRINT_FLAG ("S16 as UI16:     ", STORE_S16_AS_UI16);

  printf ("%spush flags\n", indent);

  PRINT_FLAG ("subimage:        ", PUSH_SUBIMAGE);
  PRINT_FLAG ("render quad:     ", PUSH_RENDER_QUAD);
  PRINT_FLAG ("drawpixels:      ", PUSH_DRAWPIXELS);
  PRINT_FLAG ("U8 pixelbuffer:  ", PUSH_U8_PIXELBUFFER);
  PRINT_FLAG ("S16 pixelbuffer: ", PUSH_S16_PIXELBUFFER);
  PRINT_FLAG ("U8 as U8:        ", PUSH_U8_AS_U8);
  PRINT_FLAG ("U8 as F32:       ", PUSH_U8_AS_F32);
  PRINT_FLAG ("S16 as S16:      ", PUSH_S16_AS_S16);
  PRINT_FLAG ("S16 as U16:      ", PUSH_S16_AS_U16);
  PRINT_FLAG ("S16 as F32:      ", PUSH_S16_AS_F32);

  printf ("%spull flags\n", indent);

  PRINT_FLAG ("U8 pixelbuffer:  ", PULL_U8_PIXELBUFFER);
  PRINT_FLAG ("S16 pixelbuffer: ", PULL_S16_PIXELBUFFER);
  PRINT_FLAG ("U8 as U8:        ", PULL_U8_AS_U8);
  PRINT_FLAG ("U8 as F32:       ", PULL_U8_AS_F32);
  PRINT_FLAG ("S16 as S16:      ", PULL_S16_AS_S16);
  PRINT_FLAG ("S16 as U16:      ", PULL_S16_AS_U16);
  PRINT_FLAG ("S16 as F32:      ", PULL_S16_AS_F32);

  #undef PRINT_FLAG
}

SchroOpenGLCanvas *
schro_opengl_canvas_new (SchroOpenGL *opengl, SchroOpenGLCanvasType type,
    SchroFrameFormat format, int width, int height)
{
  int i;
  SchroOpenGLCanvas *canvas;
  SchroOpenGLCanvasPool *pool;

  SCHRO_ASSERT (type >= SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY);
  SCHRO_ASSERT (type <= SCHRO_OPENGL_CANVAS_TYPE_SPATIAL_WEIGHT);

  schro_opengl_lock_canvas_pool (opengl);

  pool = schro_opengl_get_canvas_pool (opengl);

  /* try to reuse existing canvas */
  SCHRO_ASSERT (pool->canvas_count[type] >= 0);
  SCHRO_ASSERT (pool->canvas_count[type] <= SCHRO_OPENGL_CANVAS_POOL_LIMIT);

  for (i = 0; i < pool->canvas_count[type]; ++i) {
    canvas = pool->canvases[type][i];

    /* only reuse primary and spatial weight cansaves, if they are not in use
       by a caller, only in the pool */
    if ((type == SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY ||
        type == SCHRO_OPENGL_CANVAS_TYPE_SPATIAL_WEIGHT) &&
        canvas->refcount != 1) {
      continue;
    }

    if (canvas->format == format && canvas->width == width &&
        canvas->height == height) {
      ++canvas->refcount;

      schro_opengl_unlock_canvas_pool (opengl);

      return canvas;
    }

    ++canvas->uselessness;
  }

  schro_opengl_unlock_canvas_pool (opengl);

  /* create new canvas */
  canvas = schro_malloc0 (sizeof (SchroOpenGLCanvas));

  canvas->opengl = opengl;
  canvas->type = type;
  canvas->refcount = 2; /* 1 ref for caller + 1 ref for pool */
  canvas->uselessness = 0;
  canvas->format = format;
  canvas->width = width;
  canvas->height = height;

  schro_opengl_canvas_check_flags (); // FIXME

  switch (SCHRO_FRAME_FORMAT_DEPTH (format)) {
    case SCHRO_FRAME_FORMAT_DEPTH_U8:
      if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_U8)) {
        /* must use RGBA format here, because other formats are in general
           not supported by framebuffers */
        canvas->internal_format = GL_RGBA8;
        canvas->storage_type = GL_UNSIGNED_BYTE;
      } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_F16)) {
        if (!SCHRO_FRAME_IS_PACKED (format) && GLEW_NV_float_buffer) {
          canvas->internal_format = GL_FLOAT_R16_NV;
        } else {
          canvas->internal_format = GL_RGBA16F_ARB;
        }

        canvas->storage_type = GL_FLOAT;
      } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_F32)) {
        if (!SCHRO_FRAME_IS_PACKED (format) && GLEW_NV_float_buffer) {
          canvas->internal_format = GL_FLOAT_R32_NV;
        } else {
          canvas->internal_format = GL_RGBA32F_ARB;
        }

        canvas->storage_type = GL_FLOAT;
      } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_UI8)) {
        if (SCHRO_FRAME_IS_PACKED (format)) {
          canvas->internal_format = GL_RGBA8UI_EXT;
        } else {
          canvas->internal_format = GL_ALPHA8UI_EXT;
        }

        canvas->storage_type = GL_UNSIGNED_BYTE;
      } else {
        SCHRO_ERROR ("invalid canvas flags combination, one U8 storage type "
            "flag must be set");
        SCHRO_ASSERT (0);
      }

      if (SCHRO_FRAME_IS_PACKED (format)) {
        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_PACKED_AS_RGBA)) {
          if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_UI8)) {
            canvas->pixel_format = GL_RGBA_INTEGER_EXT;
          } else {
            canvas->pixel_format = GL_RGBA;
          }

          canvas->channels = 4;
        } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_PACKED_AS_BGRA)) {
          if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_UI8)) {
            canvas->pixel_format = GL_BGRA_INTEGER_EXT;
          } else {
            canvas->pixel_format = GL_BGRA;
          }

          canvas->channels = 4;
        } else {
          SCHRO_ERROR ("invalid canvas flags combination, one channel order "
              "flag must be set");
          SCHRO_ASSERT (0);
        }
      } else {
        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_UI8)) {
          canvas->pixel_format = GL_ALPHA_INTEGER_EXT;
        } else {
          canvas->pixel_format = GL_RED;
        }

        canvas->channels = 1;
      }

      if (type == SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY) {
        /* secondary */
        canvas->secondary = schro_opengl_canvas_new (opengl,
            SCHRO_OPENGL_CANVAS_TYPE_SECONDARY, format, width, height);

        /* push */
        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_U8_AS_U8)) {
          canvas->push_type = GL_UNSIGNED_BYTE;
          canvas->push_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (uint8_t));
        } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_U8_AS_F32)) {
          canvas->push_type = GL_FLOAT;
          canvas->push_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (float));
        } else {
          SCHRO_ERROR ("invalid canvas flags combination, one U8 push type "
              "flag must be set");
          SCHRO_ASSERT (0);
        }

        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_U8_PIXELBUFFER)) {
          canvas->push_pixelbuffer = schro_opengl_pixelbuffer_new (opengl,
              SCHRO_OPENGL_PIXELBUFFER_TYPE_PUSH, width, height,
              canvas->push_stride);
        } else {
          canvas->push_pixelbuffer = NULL;
        }

        /* pull */
        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_U8_AS_U8)) {
          canvas->pull_type = GL_UNSIGNED_BYTE;
          canvas->pull_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (uint8_t));
        } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_U8_AS_F32)) {
          canvas->pull_type = GL_FLOAT;
          canvas->pull_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (float));
        } else {
          SCHRO_ERROR ("invalid canvas flags combination, one U8 pull type "
              "flag must be set");
          SCHRO_ASSERT (0);
        }

        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_U8_PIXELBUFFER)) {
          canvas->pull_pixelbuffer = schro_opengl_pixelbuffer_new (opengl,
              SCHRO_OPENGL_PIXELBUFFER_TYPE_PULL, width, height,
              canvas->pull_stride);
        } else {
          canvas->pull_pixelbuffer = NULL;
        }
      } else {
        canvas->secondary = NULL;
        canvas->push_pixelbuffer = NULL;
        canvas->pull_pixelbuffer = NULL;
      }

      break;
    case SCHRO_FRAME_FORMAT_DEPTH_S16:
      if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_S16)) {
        /* must use RGBA format here, because other formats are in general
           not supported by framebuffers */
        canvas->internal_format = GL_RGBA16;
        canvas->storage_type = GL_SHORT;
      } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_U16)) {
        /* must use RGBA format here, because other formats are in general
           not supported by framebuffers */
        canvas->internal_format = GL_RGBA16;
        canvas->storage_type = GL_UNSIGNED_SHORT;
      } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_F16)) {
        if (!SCHRO_FRAME_IS_PACKED (format) && GLEW_NV_float_buffer) {
          canvas->internal_format = GL_FLOAT_R16_NV;
        } else {
          canvas->internal_format = GL_RGBA16F_ARB;
        }

        canvas->storage_type = GL_FLOAT;
      } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_F32)) {
        if (!SCHRO_FRAME_IS_PACKED (format) && GLEW_NV_float_buffer) {
          canvas->internal_format = GL_FLOAT_R32_NV;
        } else {
          canvas->internal_format = GL_RGBA32F_ARB;
        }

        canvas->storage_type = GL_FLOAT;
      } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_I16)) {
        if (SCHRO_FRAME_IS_PACKED (format)) {
          canvas->internal_format = GL_RGBA16I_EXT;
        } else {
          canvas->internal_format = GL_ALPHA16I_EXT;
        }

        canvas->storage_type = GL_SHORT;
      } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_UI16)) {
        if (SCHRO_FRAME_IS_PACKED (format)) {
          canvas->internal_format = GL_RGBA16UI_EXT;
        } else {
          canvas->internal_format = GL_ALPHA16UI_EXT;
        }

        canvas->storage_type = GL_UNSIGNED_SHORT;
      } else {
        SCHRO_ERROR ("invalid canvas flags combination, one S16 storage type "
            "flag must be set");
        SCHRO_ASSERT (0);
      }

      if (SCHRO_FRAME_IS_PACKED (format)) {
        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_PACKED_AS_RGBA)) {
          if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_I16) ||
              SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_UI16)) {
            canvas->pixel_format = GL_RGBA_INTEGER_EXT;
          } else {
            canvas->pixel_format = GL_RGBA;
          }

          canvas->channels = 4;
        } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_PACKED_AS_BGRA)) {
          if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_I16) ||
              SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_UI16)) {
            canvas->pixel_format = GL_BGRA_INTEGER_EXT;
          } else {
            canvas->pixel_format = GL_BGRA;
          }

          canvas->channels = 4;
        } else {
          SCHRO_ERROR ("invalid canvas flags combination, one channel order "
              "flag must be set");
          SCHRO_ASSERT (0);
        }
      } else {
        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_I16) ||
            SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_UI16)) {
          canvas->pixel_format = GL_ALPHA_INTEGER_EXT;
        } else {
          canvas->pixel_format = GL_RED;
        }

        canvas->channels = 1;
      }

      if (type == SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY) {
        /* secondary */
        canvas->secondary = schro_opengl_canvas_new (opengl,
            SCHRO_OPENGL_CANVAS_TYPE_SECONDARY, format, width, height);

        /* push */
        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_S16_AS_S16)) {
          canvas->push_type = GL_SHORT;
          canvas->push_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (int16_t));
        } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_S16_AS_U16)) {
          canvas->push_type = GL_UNSIGNED_SHORT;
          canvas->push_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (uint16_t));
        } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_S16_AS_F32)) {
          canvas->push_type = GL_FLOAT;
          canvas->push_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (float));
        } else {
          SCHRO_ERROR ("invalid canvas flags combination, one S16 push type "
              "flag must be set");
          SCHRO_ASSERT (0);
        }

        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_S16_PIXELBUFFER)) {
          canvas->push_pixelbuffer = schro_opengl_pixelbuffer_new (opengl,
              SCHRO_OPENGL_PIXELBUFFER_TYPE_PUSH, width, height,
              canvas->push_stride);
        } else {
          canvas->push_pixelbuffer = NULL;
        }

        /* pull */
        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_S16_AS_S16)) {
          /* FIXME: pulling S16 as GL_SHORT doesn't work in general, maybe
             it's the right mode if the internal format is an integer format
             but for some reason storing as I16 doesn't work either and only
             gives garbage pull results */
          canvas->pull_type = GL_SHORT;
          canvas->pull_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (int16_t));
        } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_S16_AS_U16)) {
          /* must pull S16 as GL_UNSIGNED_SHORT instead of GL_SHORT because
             the OpenGL mapping form internal float represenation into S16
             values with GL_SHORT maps 0.0 to 0 and 1.0 to 32767 clamping all
             negative values to 0, see glReadPixel documentation. so the pull
             is done with GL_UNSIGNED_SHORT and the resulting U16 values are
             manually shifted to S16 */
          canvas->pull_type = GL_UNSIGNED_SHORT;
          canvas->pull_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (uint16_t));
        } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_S16_AS_F32)) {
          canvas->pull_type = GL_FLOAT;
          canvas->pull_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (float));
        } else {
          SCHRO_ERROR ("invalid canvas flags combination, one S16 pull type "
              "flag must be set");
          SCHRO_ASSERT (0);
        }

        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_S16_PIXELBUFFER)) {
          canvas->pull_pixelbuffer = schro_opengl_pixelbuffer_new (opengl,
              SCHRO_OPENGL_PIXELBUFFER_TYPE_PULL, width, height,
              canvas->pull_stride);
        } else {
          canvas->pull_pixelbuffer = NULL;
        }
      } else {
        canvas->secondary = NULL;
        canvas->push_pixelbuffer = NULL;
        canvas->pull_pixelbuffer = NULL;
      }

      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }

  SCHRO_OPENGL_LOCK_CONTEXT (opengl);

  /* texture */
  glGenTextures (1, &canvas->texture);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, canvas->texture);
  glTexImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, canvas->internal_format, width,
      height, 0, canvas->pixel_format, canvas->storage_type, NULL);
  glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  SCHRO_OPENGL_CHECK_ERROR

#if SCHRO_OPENGL_UNBIND_TEXTURES
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);
#endif

  /* framebuffer */
  glGenFramebuffersEXT (1, &canvas->framebuffer);
  glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, canvas->framebuffer);
  glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
      GL_TEXTURE_RECTANGLE_ARB, canvas->texture, 0);
  glDrawBuffer (GL_COLOR_ATTACHMENT0_EXT);
  glReadBuffer (GL_COLOR_ATTACHMENT0_EXT);

  SCHRO_OPENGL_CHECK_ERROR
  SCHRO_OPENGL_CHECK_FRAMEBUFFER

  glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);

  SCHRO_OPENGL_UNLOCK_CONTEXT (opengl);

  /* add new canvas to pool */
  schro_opengl_lock_canvas_pool (opengl);

  SCHRO_ASSERT (pool->canvas_count[type] >= 0);
  SCHRO_ASSERT (pool->canvas_count[type] < SCHRO_OPENGL_CANVAS_POOL_LIMIT);

  pool->canvases[type][pool->canvas_count[type]] = canvas;
  ++pool->canvas_count[type];

  schro_opengl_unlock_canvas_pool (opengl);

  return canvas;
}

void
schro_opengl_canvas_unref (SchroOpenGLCanvas *canvas)
{
  int i;
  SchroOpenGLCanvasPool *pool;

  schro_opengl_lock_canvas_pool (canvas->opengl);

  SCHRO_ASSERT (canvas->refcount > 0);

  --canvas->refcount;

  if (canvas->refcount > 0) {
    schro_opengl_unlock_canvas_pool (canvas->opengl);
    return;
  }

  /* remove canvas from the pool */
  pool = schro_opengl_get_canvas_pool (canvas->opengl);

  SCHRO_ASSERT (canvas->type >= SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY);
  SCHRO_ASSERT (canvas->type <= SCHRO_OPENGL_CANVAS_TYPE_SPATIAL_WEIGHT);
  SCHRO_ASSERT (pool->canvas_count[canvas->type] >= 1);
  SCHRO_ASSERT (pool->canvas_count[canvas->type]
      <= SCHRO_OPENGL_CANVAS_POOL_LIMIT);

  for (i = 0; i < pool->canvas_count[canvas->type]; ++i) {
    if (pool->canvases[canvas->type][i] == canvas) {
      --pool->canvas_count[canvas->type];

      /* move the last canvas in the array to the slot of the removed
         one to maintain the array continuous in memory */
      pool->canvases[canvas->type][i] = pool->canvases[canvas->type]
          [pool->canvas_count[canvas->type]];

      break;
    }
  }

  schro_opengl_unlock_canvas_pool (canvas->opengl);

  /* free OpenGL handles */
  SCHRO_OPENGL_LOCK_CONTEXT (canvas->opengl);

  SCHRO_ASSERT (glIsTexture (canvas->texture));
  SCHRO_ASSERT (glIsFramebufferEXT (canvas->framebuffer));

  glDeleteTextures (1, &canvas->texture);
  glDeleteFramebuffersEXT (1, &canvas->framebuffer);

  SCHRO_OPENGL_UNLOCK_CONTEXT (canvas->opengl);

  switch (canvas->type) {
    case SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY:
      SCHRO_ASSERT (canvas->secondary != NULL);

      schro_opengl_canvas_unref (canvas->secondary);

      if (canvas->push_pixelbuffer) {
        schro_opengl_pixelbuffer_unref (canvas->push_pixelbuffer);
      }

      if (canvas->pull_pixelbuffer) {
        schro_opengl_pixelbuffer_unref (canvas->pull_pixelbuffer);
      }

      break;
    case SCHRO_OPENGL_CANVAS_TYPE_SECONDARY:
    case SCHRO_OPENGL_CANVAS_TYPE_SPATIAL_WEIGHT:
      SCHRO_ASSERT (canvas->secondary == NULL);
      SCHRO_ASSERT (canvas->push_pixelbuffer == NULL);
      SCHRO_ASSERT (canvas->pull_pixelbuffer == NULL);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }

  schro_free (canvas);
}

SchroOpenGLPixelbuffer *
schro_opengl_pixelbuffer_new (SchroOpenGL *opengl,
    SchroOpenGLPixelbufferType type, int width, int height, int stride)
{
  int i;
  SchroOpenGLPixelbuffer *pixelbuffer;
  SchroOpenGLCanvasPool *pool;

  SCHRO_ASSERT (type >= SCHRO_OPENGL_PIXELBUFFER_TYPE_PUSH);
  SCHRO_ASSERT (type <= SCHRO_OPENGL_PIXELBUFFER_TYPE_PULL);

  schro_opengl_lock_canvas_pool (opengl);

  pool = schro_opengl_get_canvas_pool (opengl);

  /* try to reuse existing pixelbuffer */
  SCHRO_ASSERT (pool->pixelbuffer_count[type] >= 0);
  SCHRO_ASSERT (pool->pixelbuffer_count[type]
      <= SCHRO_OPENGL_CANVAS_POOL_LIMIT);

  for (i = 0; i < pool->pixelbuffer_count[type]; ++i) {
    pixelbuffer = pool->pixelbuffers[type][i];

    if (pixelbuffer->width == width && pixelbuffer->height == height &&
        pixelbuffer->stride == stride) {
      ++pixelbuffer->refcount;

      schro_opengl_unlock_canvas_pool (opengl);

      return pixelbuffer;
    }
  }

  schro_opengl_unlock_canvas_pool (opengl);

  /* create new pixelbuffer */
  pixelbuffer = schro_malloc0 (sizeof (SchroOpenGLPixelbuffer));

  pixelbuffer->opengl = opengl;
  pixelbuffer->type = type;
  pixelbuffer->refcount = 2; /* 1 ref for caller + 1 ref for pool */
  pixelbuffer->uselessness = 0;
  pixelbuffer->width = width;
  pixelbuffer->height = height;
  pixelbuffer->stride = stride;

  SCHRO_OPENGL_LOCK_CONTEXT (opengl);

  for (i = 0; i < SCHRO_OPENGL_PIXELBUFFER_BLOCKS; ++i) {
    if (i == SCHRO_OPENGL_PIXELBUFFER_BLOCKS - 1) {
      pixelbuffer->heights[i]
          = height - (height / SCHRO_OPENGL_PIXELBUFFER_BLOCKS) * i;
    } else {
      pixelbuffer->heights[i] = height / SCHRO_OPENGL_PIXELBUFFER_BLOCKS;
    }

    switch (type) {
      case SCHRO_OPENGL_PIXELBUFFER_TYPE_PUSH:
        glGenBuffersARB (1, &pixelbuffer->handles[i]);
        glBindBufferARB (GL_PIXEL_UNPACK_BUFFER_ARB, pixelbuffer->handles[i]);
        glBufferDataARB (GL_PIXEL_UNPACK_BUFFER_ARB,
            stride * pixelbuffer->heights[i], NULL, GL_STREAM_DRAW_ARB);
        glBindBufferARB (GL_PIXEL_UNPACK_BUFFER_ARB, 0);
        break;
      case SCHRO_OPENGL_PIXELBUFFER_TYPE_PULL:
        glGenBuffersARB (1, &pixelbuffer->handles[i]);
        glBindBufferARB (GL_PIXEL_PACK_BUFFER_ARB, pixelbuffer->handles[i]);
        glBufferDataARB (GL_PIXEL_PACK_BUFFER_ARB,
            stride * pixelbuffer->heights[i], NULL, GL_STATIC_READ_ARB);
        glBindBufferARB (GL_PIXEL_PACK_BUFFER_ARB, 0);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }

    SCHRO_OPENGL_CHECK_ERROR
  }

  SCHRO_OPENGL_UNLOCK_CONTEXT (opengl);

  /* add new pixelbuffer to pool */
  schro_opengl_lock_canvas_pool (opengl);

  SCHRO_ASSERT (pool->pixelbuffer_count[type] >= 0);
  SCHRO_ASSERT (pool->pixelbuffer_count[type] < SCHRO_OPENGL_CANVAS_POOL_LIMIT);

  pool->pixelbuffers[type][pool->pixelbuffer_count[type]] = pixelbuffer;
  ++pool->pixelbuffer_count[type];

  schro_opengl_unlock_canvas_pool (opengl);

  return pixelbuffer;
}

void
schro_opengl_pixelbuffer_unref (SchroOpenGLPixelbuffer *pixelbuffer)
{
  int i;
  SchroOpenGLCanvasPool *pool;

  schro_opengl_lock_canvas_pool (pixelbuffer->opengl);

  SCHRO_ASSERT (pixelbuffer->refcount > 0);

  --pixelbuffer->refcount;

  if (pixelbuffer->refcount > 0) {
    schro_opengl_unlock_canvas_pool (pixelbuffer->opengl);
    return;
  }

  /* remove pixelbuffer from the pool */
  pool = schro_opengl_get_canvas_pool (pixelbuffer->opengl);

  SCHRO_ASSERT (pixelbuffer->type >= SCHRO_OPENGL_PIXELBUFFER_TYPE_PUSH);
  SCHRO_ASSERT (pixelbuffer->type <= SCHRO_OPENGL_PIXELBUFFER_TYPE_PULL);
  SCHRO_ASSERT (pool->pixelbuffer_count[pixelbuffer->type] >= 1);
  SCHRO_ASSERT (pool->pixelbuffer_count[pixelbuffer->type]
      <= SCHRO_OPENGL_CANVAS_POOL_LIMIT);

  for (i = 0; i < pool->pixelbuffer_count[pixelbuffer->type]; ++i) {
    if (pool->pixelbuffers[pixelbuffer->type][i] == pixelbuffer) {
      --pool->pixelbuffer_count[pixelbuffer->type];

      /* move the last pixelbuffer in the array to the slot of the removed
         one to maintain the array continuous in memory */
      pool->pixelbuffers[pixelbuffer->type][i]
          = pool->pixelbuffers[pixelbuffer->type]
          [pool->pixelbuffer_count[pixelbuffer->type]];

      break;
    }
  }

  schro_opengl_unlock_canvas_pool (pixelbuffer->opengl);

  /* free OpenGL handles */
  SCHRO_OPENGL_LOCK_CONTEXT (pixelbuffer->opengl);

  for (i = 0; i < SCHRO_OPENGL_PIXELBUFFER_BLOCKS; ++i) {
    SCHRO_ASSERT (glIsBufferARB (pixelbuffer->handles[i]));

    glDeleteBuffersARB (1, &pixelbuffer->handles[i]);
  }

  SCHRO_OPENGL_UNLOCK_CONTEXT (pixelbuffer->opengl);

  schro_free (pixelbuffer);
}

SchroOpenGLCanvasPool *
schro_opengl_canvas_pool_new (SchroOpenGL *opengl)
{
  SchroOpenGLCanvasPool *pool;

  pool = schro_malloc0 (sizeof (SchroOpenGLCanvasPool));

  pool->opengl = opengl;
  pool->canvas_count[SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY] = 0;
  pool->canvas_count[SCHRO_OPENGL_CANVAS_TYPE_SECONDARY] = 0;
  pool->canvas_count[SCHRO_OPENGL_CANVAS_TYPE_SPATIAL_WEIGHT] = 0;
  pool->pixelbuffer_count[SCHRO_OPENGL_PIXELBUFFER_TYPE_PUSH] = 0;
  pool->pixelbuffer_count[SCHRO_OPENGL_PIXELBUFFER_TYPE_PULL] = 0;

  return pool;
}

void
schro_opengl_canvas_pool_free (SchroOpenGLCanvasPool* pool)
{
  int i;

  schro_opengl_lock_canvas_pool (pool->opengl);

  #define UNREF_CANVASES(_type) \
      do { \
        for (i = 0; i < pool->canvas_count[_type]; ++i) { \
          SCHRO_ASSERT (pool->canvases[_type][i]->refcount == 1); \
        } \
        while (pool->canvas_count[_type] > 0) { \
          schro_opengl_canvas_unref (pool->canvases[_type][0]); \
        } \
        SCHRO_ASSERT (pool->canvas_count[_type] == 0); \
      } while (0)

  #define UNREF_PIXELBUFFERS(_type) \
      do { \
        for (i = 0; i < pool->pixelbuffer_count[_type]; ++i) { \
          SCHRO_ASSERT (pool->pixelbuffers[_type][i]->refcount == 1); \
        } \
        while (pool->pixelbuffer_count[_type] > 0) { \
          schro_opengl_pixelbuffer_unref (pool->pixelbuffers[_type][0]); \
        } \
        SCHRO_ASSERT (pool->pixelbuffer_count[_type] == 0); \
      } while (0)

  UNREF_CANVASES (SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY);
  UNREF_CANVASES (SCHRO_OPENGL_CANVAS_TYPE_SECONDARY);
  UNREF_CANVASES (SCHRO_OPENGL_CANVAS_TYPE_SPATIAL_WEIGHT);
  UNREF_PIXELBUFFERS (SCHRO_OPENGL_PIXELBUFFER_TYPE_PUSH);
  UNREF_PIXELBUFFERS (SCHRO_OPENGL_PIXELBUFFER_TYPE_PULL);

  #undef UNREF_CANVASES
  #undef UNREF_PIXELBUFFERS

  schro_opengl_unlock_canvas_pool (pool->opengl);

  schro_free (pool);
}

void
schro_opengl_canvas_pool_squeeze (SchroOpenGLCanvasPool* pool)
{
  int i;
  int done = FALSE;

  schro_opengl_lock_canvas_pool (pool->opengl);

  #define SQUEEZE_CANVASES(_type) \
      do { \
        done = FALSE; \
        while (!done) { \
          done = TRUE; \
          for (i = 0; i < pool->canvas_count[_type]; ++i) { \
            if (pool->canvases[_type][i]->refcount == 1) { \
              /* canvas only in pool */ \
              schro_opengl_canvas_unref (pool->canvases[_type][i]); \
              done = FALSE; \
              break; \
            } \
          } \
        } \
      } while (0)

  #define SQUEEZE_PIXELBUFFERS(_type) \
      do { \
        done = FALSE; \
        while (!done) { \
          done = TRUE; \
          for (i = 0; i < pool->pixelbuffer_count[_type]; ++i) { \
            if (pool->pixelbuffers[_type][i]->refcount == 1) { \
              /* pixelbuffer only in pool */ \
              schro_opengl_pixelbuffer_unref (pool->pixelbuffers[_type][i]); \
              done = FALSE; \
              break; \
            } \
          } \
        } \
      } while (0)

  SQUEEZE_CANVASES (SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY);
  SQUEEZE_CANVASES (SCHRO_OPENGL_CANVAS_TYPE_SECONDARY);
  SQUEEZE_CANVASES (SCHRO_OPENGL_CANVAS_TYPE_SPATIAL_WEIGHT);
  SQUEEZE_PIXELBUFFERS (SCHRO_OPENGL_PIXELBUFFER_TYPE_PUSH);
  SQUEEZE_PIXELBUFFERS (SCHRO_OPENGL_PIXELBUFFER_TYPE_PULL);

  #undef SQUEEZE_CANVASES
  #undef SQUEEZE_PIXELBUFFERS

  schro_opengl_unlock_canvas_pool (pool->opengl);
}

