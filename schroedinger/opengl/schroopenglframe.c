
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <schroedinger/schrodebug.h>
#include <schroedinger/opengl/schroopengl.h>
#include <schroedinger/opengl/schroopenglcanvas.h>
#include <schroedinger/opengl/schroopenglframe.h>
#include <schroedinger/opengl/schroopenglshader.h>
#include <schroedinger/opengl/schroopenglshadercode.h>
#include <schroedinger/opengl/schroopenglwavelet.h>
#include <stdio.h>

static void
schro_opengl_frame_free_callback (SchroFrame *frame, void *priv)
{
  SCHRO_ASSERT (SCHRO_FRAME_IS_OPENGL (frame));

  schro_opengl_frame_cleanup (frame);
}

void
schro_opengl_frame_setup (SchroOpenGL *opengl, SchroFrame *frame)
{
  int i;
  int components;
  int width, height;
  SchroFrameFormat format;
  SchroOpenGLCanvas *canvas;

  SCHRO_ASSERT (frame != NULL);
  SCHRO_ASSERT (SCHRO_FRAME_IS_OPENGL (frame));

  components = SCHRO_FRAME_IS_PACKED (frame->format) ? 1 : 3;

  /* use the free callback to do OpenGL specific cleanup. the free callback is
     public API, but OpenGL frames are internal to the decoder, so it's safe to
     use the free callback for them */
  schro_frame_set_free_callback (frame,
      (SchroFrameFreeFunc) schro_opengl_frame_free_callback, NULL);

  for (i = 0; i < components; ++i) {
    format = frame->components[i].format;
    width = frame->components[i].width;
    height = frame->components[i].height;

    SCHRO_ASSERT (frame->format == format);

    canvas = schro_opengl_canvas_new (opengl, SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY,
        format, width, height);

    SCHRO_OPNEGL_CANVAS_TO_FRAMEDATA (frame->components + i, canvas);
  }
}

void
schro_opengl_frame_cleanup (SchroFrame *frame)
{
  int i;
  int components;
  SchroOpenGLCanvas *canvas;

  SCHRO_ASSERT (frame != NULL);
  SCHRO_ASSERT (SCHRO_FRAME_IS_OPENGL (frame));

  components = SCHRO_FRAME_IS_PACKED (frame->format) ? 1 : 3;
  canvas = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA (frame->components + 0);

  SCHRO_ASSERT (canvas != NULL);

  for (i = 0; i < components; ++i) {
    canvas = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA (frame->components + i);

    SCHRO_ASSERT (canvas != NULL);

    schro_opengl_canvas_unref (canvas);
  }
}

SchroFrame *
schro_opengl_frame_new (SchroOpenGL *opengl,
    SchroMemoryDomain *opengl_domain, SchroFrameFormat format, int width,
    int height)
{
  SchroFrame *opengl_frame = schro_frame_new ();
  int bytes_pp;
  int h_shift, v_shift;
  int chroma_width;
  int chroma_height;

  SCHRO_ASSERT (width > 0);
  SCHRO_ASSERT (height > 0);
  SCHRO_ASSERT (opengl_domain->flags & SCHRO_MEMORY_DOMAIN_OPENGL);

  opengl_frame->format = format;
  opengl_frame->width = width;
  opengl_frame->height = height;
  opengl_frame->domain = opengl_domain;

  if (SCHRO_FRAME_IS_PACKED (format)) {
    opengl_frame->components[0].format = format;
    opengl_frame->components[0].width = width;
    opengl_frame->components[0].height = height;

    if (format == SCHRO_FRAME_FORMAT_AYUV) {
      opengl_frame->components[0].stride = width * 4;
    } else {
      opengl_frame->components[0].stride = ROUND_UP_POW2 (width, 1) * 2;
    }

    opengl_frame->components[0].length
        = opengl_frame->components[0].stride * height;
    opengl_frame->components[0].data = NULL;
    opengl_frame->components[0].v_shift = 0;
    opengl_frame->components[0].h_shift = 0;

    schro_opengl_frame_setup (opengl, opengl_frame);

    return opengl_frame;
  }

  switch (SCHRO_FRAME_FORMAT_DEPTH (format)) {
    case SCHRO_FRAME_FORMAT_DEPTH_U8:
      bytes_pp = 1;
      break;
    case SCHRO_FRAME_FORMAT_DEPTH_S16:
      bytes_pp = 2;
      break;
    case SCHRO_FRAME_FORMAT_DEPTH_S32:
      bytes_pp = 4;
      break;
    default:
      SCHRO_ASSERT(0);
      bytes_pp = 0;
      break;
  }

  h_shift = SCHRO_FRAME_FORMAT_H_SHIFT (format);
  v_shift = SCHRO_FRAME_FORMAT_V_SHIFT (format);
  chroma_width = ROUND_UP_SHIFT (width, h_shift);
  chroma_height = ROUND_UP_SHIFT (height, v_shift);

  opengl_frame->components[0].format = format;
  opengl_frame->components[0].width = width;
  opengl_frame->components[0].height = height;
  opengl_frame->components[0].stride = ROUND_UP_4 (width * bytes_pp);
  opengl_frame->components[0].length
      = opengl_frame->components[0].stride * opengl_frame->components[0].height;
  opengl_frame->components[0].v_shift = 0;
  opengl_frame->components[0].h_shift = 0;

  opengl_frame->components[1].format = format;
  opengl_frame->components[1].width = chroma_width;
  opengl_frame->components[1].height = chroma_height;
  opengl_frame->components[1].stride = ROUND_UP_4 (chroma_width * bytes_pp);
  opengl_frame->components[1].length
      = opengl_frame->components[1].stride * opengl_frame->components[1].height;
  opengl_frame->components[1].v_shift = v_shift;
  opengl_frame->components[1].h_shift = h_shift;

  opengl_frame->components[2].format = format;
  opengl_frame->components[2].width = chroma_width;
  opengl_frame->components[2].height = chroma_height;
  opengl_frame->components[2].stride = ROUND_UP_4 (chroma_width * bytes_pp);
  opengl_frame->components[2].length
      = opengl_frame->components[2].stride * opengl_frame->components[2].height;
  opengl_frame->components[2].v_shift = v_shift;
  opengl_frame->components[2].h_shift = h_shift;

  opengl_frame->components[0].data = NULL;
  opengl_frame->components[1].data = NULL;
  opengl_frame->components[2].data = NULL;

  schro_opengl_frame_setup (opengl, opengl_frame);

  return opengl_frame;
}

SchroFrame *
schro_opengl_frame_clone (SchroFrame *opengl_frame)
{
  SchroOpenGLCanvas *canvas;

  SCHRO_ASSERT (opengl_frame != NULL);
  SCHRO_ASSERT (SCHRO_FRAME_IS_OPENGL (opengl_frame));

  canvas = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA (opengl_frame->components + 0);

  SCHRO_ASSERT (canvas != NULL);

  return schro_opengl_frame_new (canvas->opengl, opengl_frame->domain,
      opengl_frame->format, opengl_frame->width, opengl_frame->height);
}

SchroFrame *
schro_opengl_frame_clone_and_push (SchroOpenGL *opengl,
    SchroMemoryDomain *opengl_domain, SchroFrame *cpu_frame)
{
  SchroFrame *opengl_frame;

  SCHRO_ASSERT (opengl_domain->flags & SCHRO_MEMORY_DOMAIN_OPENGL);
  SCHRO_ASSERT (!SCHRO_FRAME_IS_OPENGL (cpu_frame));

  opengl_frame = schro_frame_clone (opengl_domain, cpu_frame);

  schro_opengl_frame_setup (opengl, opengl_frame);
  schro_opengl_frame_push (opengl_frame, cpu_frame);

  return opengl_frame;
}

void
schro_opengl_frame_inverse_iwt_transform (SchroFrame *frame,
    SchroParams *params)
{
  int i;
  int width, height, level;
  SchroOpenGLCanvas *canvas;

  canvas = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA (frame->components + 0);

  SCHRO_ASSERT (canvas != NULL);

  schro_opengl_lock_context (canvas->opengl);

  for (i = 0; i < 3; ++i) {
    canvas = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA (frame->components + i);

    SCHRO_ASSERT (canvas != NULL);

    if (i == 0) {
      width = params->iwt_luma_width;
      height = params->iwt_luma_height;
    } else {
      width = params->iwt_chroma_width;
      height = params->iwt_chroma_height;
    }

    for (level = params->transform_depth - 1; level >= 0; --level) {
      SchroFrameData frame_data;

      frame_data.format = frame->format;
      frame_data.data = frame->components[i].data;
      frame_data.width = width >> level;
      frame_data.height = height >> level;
      frame_data.stride = frame->components[i].stride << level;

      schro_opengl_wavelet_inverse_transform (&frame_data,
          params->wavelet_filter_index);
    }
  }

  schro_opengl_unlock_context (canvas->opengl);
}

static void
schro_opengl_upsampled_frame_render_quad (SchroOpenGLShader *shader, int x,
    int y, int quad_width, int quad_height, int total_width, int total_height)
{
  int inverse[] = { 0, 0 };
  int four[] = { 0, 0 }, three[] = { 0, 0 }, two[] = { 0, 0 }, one[] = { 0, 0 };

  inverse[0] = total_width - x - quad_width;
  inverse[1] = total_height - y - quad_height;

  if (quad_width == total_width && quad_height < total_height) {
    four[1] = 4;
    three[1] = 3;
    two[1] = 2;
    one[1] = 1;
  } else if (quad_width < total_width && quad_height == total_height) {
    four[0] = 4;
    three[0] = 3;
    two[0] = 2;
    one[0] = 1;
  } else {
    SCHRO_ERROR ("invalid quad to total relation");
    SCHRO_ASSERT (0);
  }

  SCHRO_ASSERT (inverse[0] >= 0);
  SCHRO_ASSERT (inverse[1] >= 0);

  #define UNIFORM(_operation, _number, _x, _y) \
      do { \
        if (shader->uniforms->_operation != -1) { \
          schro_opengl_shader_bind_##_operation (shader, \
              _x < _number[0] ? _x : _number[0], \
              _y < _number[1] ? _y : _number[1]); \
        } \
      } while (0)

  UNIFORM (decrease3, three, x, y);
  UNIFORM (decrease2, two,   x, y);
  UNIFORM (decrease1, one,   x, y);
  UNIFORM (increase1, one,   inverse[0], inverse[1]);
  UNIFORM (increase2, two,   inverse[0], inverse[1]);
  UNIFORM (increase3, three, inverse[0], inverse[1]);
  UNIFORM (increase4, four,  inverse[0], inverse[1]);

  #undef UNIFORM

  schro_opengl_render_quad (x, y, quad_width, quad_height);
}

void
schro_opengl_upsampled_frame_upsample (SchroUpsampledFrame *upsampled_frame)
{
  int i;
  int width, height;
  SchroOpenGLCanvas *canvases[4];
  SchroOpenGL *opengl;
  SchroOpenGLShader *shader = NULL;

  SCHRO_ASSERT (upsampled_frame->frames[0] != NULL);
  SCHRO_ASSERT (upsampled_frame->frames[1] == NULL);
  SCHRO_ASSERT (upsampled_frame->frames[2] == NULL);
  SCHRO_ASSERT (upsampled_frame->frames[3] == NULL);
  SCHRO_ASSERT (SCHRO_FRAME_IS_OPENGL (upsampled_frame->frames[0]));
  SCHRO_ASSERT (!SCHRO_FRAME_IS_PACKED (upsampled_frame->frames[0]->format));

  canvases[0] = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA
      (upsampled_frame->frames[0]->components + 0);

  SCHRO_ASSERT (canvases[0] != NULL);

  opengl = canvases[0]->opengl;

  schro_opengl_lock_context (opengl);

  upsampled_frame->frames[1]
      = schro_opengl_frame_clone (upsampled_frame->frames[0]);
  upsampled_frame->frames[2]
      = schro_opengl_frame_clone (upsampled_frame->frames[0]);
  upsampled_frame->frames[3]
      = schro_opengl_frame_clone (upsampled_frame->frames[0]);

  shader = schro_opengl_shader_get (opengl, SCHRO_OPENGL_SHADER_UPSAMPLE_U8);

  SCHRO_ASSERT (shader != NULL);

  glUseProgramObjectARB (shader->program);

  SCHRO_OPENGL_CHECK_ERROR

  for (i = 0; i < 3; ++i) {
    canvases[0] = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA
        (upsampled_frame->frames[0]->components + i);
    canvases[1] = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA
        (upsampled_frame->frames[1]->components + i);
    canvases[2] = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA
        (upsampled_frame->frames[2]->components + i);
    canvases[3] = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA
        (upsampled_frame->frames[3]->components + i);

    SCHRO_ASSERT (canvases[0] != NULL);
    SCHRO_ASSERT (canvases[1] != NULL);
    SCHRO_ASSERT (canvases[2] != NULL);
    SCHRO_ASSERT (canvases[3] != NULL);
    SCHRO_ASSERT (canvases[0]->opengl == opengl);
    SCHRO_ASSERT (canvases[1]->opengl == opengl);
    SCHRO_ASSERT (canvases[2]->opengl == opengl);
    SCHRO_ASSERT (canvases[3]->opengl == opengl);

    width = upsampled_frame->frames[0]->components[i].width;
    height = upsampled_frame->frames[0]->components[i].height;

    SCHRO_ASSERT (width >= 2);
    SCHRO_ASSERT (height >= 2);
    SCHRO_ASSERT (width % 2 == 0);
    SCHRO_ASSERT (height % 2 == 0);

    schro_opengl_setup_viewport (width, height);

    /* horizontal filter 0 -> 1 */
    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, canvases[1]->framebuffer);

    schro_opengl_shader_bind_input (shader, canvases[0]->texture);

    SCHRO_OPENGL_CHECK_ERROR

    #define RENDER_QUAD_HORIZONTAL(_x, _quad_width) \
        schro_opengl_upsampled_frame_render_quad (shader, _x, 0,  _quad_width, \
            height, width, height)

    RENDER_QUAD_HORIZONTAL (0, 1);

    if (width > 2) {
      RENDER_QUAD_HORIZONTAL (1, 1);

      if (width > 4) {
        RENDER_QUAD_HORIZONTAL (2, 1);

        if (width > 6) {
          RENDER_QUAD_HORIZONTAL (3, 1);

           if (width > 8) {
             RENDER_QUAD_HORIZONTAL (4, width - 8);
           }

           RENDER_QUAD_HORIZONTAL (width - 4, 1);
        }

        RENDER_QUAD_HORIZONTAL (width - 3, 1);
      }

      RENDER_QUAD_HORIZONTAL (width - 2, 1);
    }

    RENDER_QUAD_HORIZONTAL (width - 1, 1);

    #undef RENDER_QUAD_HORIZONTAL

    /* vertical filter 0 -> 2 */
    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, canvases[2]->framebuffer);

    schro_opengl_shader_bind_input (shader, canvases[0]->texture);

    SCHRO_OPENGL_CHECK_ERROR

    #define RENDER_QUAD_VERTICAL(_y, _quad_height) \
        schro_opengl_upsampled_frame_render_quad (shader, 0, _y,  width, \
            _quad_height, width, height)

    RENDER_QUAD_VERTICAL (0, 1);

    if (height > 2) {
      RENDER_QUAD_VERTICAL (1, 1);

      if (height > 4) {
        RENDER_QUAD_VERTICAL (2, 1);

        if (height > 6) {
          RENDER_QUAD_VERTICAL (3, 1);

           if (height > 8) {
             RENDER_QUAD_VERTICAL (4, height - 8);
           }

           RENDER_QUAD_VERTICAL (height - 4, 1);
        }

        RENDER_QUAD_VERTICAL (height - 3, 1);
      }

      RENDER_QUAD_VERTICAL (height - 2, 1);
    }

    RENDER_QUAD_VERTICAL (height - 1, 1);

    #undef RENDER_QUAD_VERTICAL

    /* horizontal filter 2 -> 3 */
    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, canvases[3]->framebuffer);

    schro_opengl_shader_bind_input (shader, canvases[2]->texture);

    SCHRO_OPENGL_CHECK_ERROR

    #define RENDER_QUAD_HORIZONTAL(_x, _quad_width) \
        schro_opengl_upsampled_frame_render_quad (shader, _x, 0,  _quad_width, \
            height, width, height)

    RENDER_QUAD_HORIZONTAL (0, 1);

    if (width > 2) {
      RENDER_QUAD_HORIZONTAL (1, 1);

      if (width > 4) {
        RENDER_QUAD_HORIZONTAL (2, 1);

        if (width > 6) {
          RENDER_QUAD_HORIZONTAL (3, 1);

           if (width > 8) {
             RENDER_QUAD_HORIZONTAL (4, width - 8);
           }

           RENDER_QUAD_HORIZONTAL (width - 4, 1);
        }

        RENDER_QUAD_HORIZONTAL (width - 3, 1);
      }

      RENDER_QUAD_HORIZONTAL (width - 2, 1);
    }

    RENDER_QUAD_HORIZONTAL (width - 1, 1);

    #undef RENDER_QUAD_HORIZONTAL
  }

  glUseProgramObjectARB (0);
#if SCHRO_OPENGL_UNBIND_TEXTURES
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);
#endif
  glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);

  schro_opengl_unlock_context (opengl);
}

void
schro_frame_print (SchroFrame *frame, const char* name)
{
  printf ("schro_frame_print: %s\n", name);

  switch (SCHRO_FRAME_FORMAT_DEPTH (frame->format)) {
    case SCHRO_FRAME_FORMAT_DEPTH_U8:
      printf ("  depth:  U8\n");
      break;
    case SCHRO_FRAME_FORMAT_DEPTH_S16:
      printf ("  depth:  S16\n");
      break;
    case SCHRO_FRAME_FORMAT_DEPTH_S32:
      printf ("  depth:  S32\n");
      break;
    default:
      printf ("  depth:  unknown\n");
      break;
  }

  printf ("  packed: %s\n", SCHRO_FRAME_IS_PACKED (frame->format) ? "yes": "no");
  printf ("  width:  %i\n", frame->width);
  printf ("  height: %i\n", frame->height);
  printf ("  opengl: %s\n", SCHRO_FRAME_IS_OPENGL (frame) ? "yes": "no");
}

