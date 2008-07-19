
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <schroedinger/schrodebug.h>
#include <schroedinger/opengl/schroopengl.h>
#include <schroedinger/opengl/schroopenglcanvas.h>
#include <stdio.h>

unsigned int _schro_opengl_canvas_flags
    = 0
    //| SCHRO_OPENGL_CANVAS_STORE_BGRA /* FIXME: currently broken with packed formats in convert */
//    | SCHRO_OPENGL_CANVAS_STORE_U8_AS_UI8
    //| SCHRO_OPENGL_CANVAS_STORE_U8_AS_F16
    //| SCHRO_OPENGL_CANVAS_STORE_U8_AS_F32
//    | SCHRO_OPENGL_CANVAS_STORE_S16_AS_UI16
    //| SCHRO_OPENGL_CANVAS_STORE_S16_AS_I16 /* FIXME: doesn't yield a useable mapping in shader */
    //| SCHRO_OPENGL_CANVAS_STORE_S16_AS_U16
    //| SCHRO_OPENGL_CANVAS_STORE_S16_AS_F16 /* FIXME: currently broken in shader */
    //| SCHRO_OPENGL_CANVAS_STORE_S16_AS_F32 /* FIXME: currently broken in shader */

    //| SCHRO_OPENGL_CANVAS_PUSH_RENDER_QUAD
    //| SCHRO_OPENGL_CANVAS_PUSH_SHADER
    //| SCHRO_OPENGL_CANVAS_PUSH_DRAWPIXELS /* FIXME: currently broken */
    | SCHRO_OPENGL_CANVAS_PUSH_U8_PIXELBUFFER
    //| SCHRO_OPENGL_CANVAS_PUSH_U8_AS_F32
    //| SCHRO_OPENGL_CANVAS_PUSH_S16_PIXELBUFFER
    | SCHRO_OPENGL_CANVAS_PUSH_S16_AS_U16
    //| SCHRO_OPENGL_CANVAS_PUSH_S16_AS_F32

    //| SCHRO_OPENGL_CANVAS_PULL_PIXELBUFFER
    //| SCHRO_OPENGL_CANVAS_PULL_U8_AS_F32
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

void
schro_opengl_canvas_check_flags (void)
{
  int count;

  /* store */
  if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_BGRA)
      && !GLEW_EXT_bgra) {
    SCHRO_ERROR ("missing extension GL_EXT_bgra, disabling BGRA storing");
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_BGRA);
  }

  count = 0;

  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_UI8)   ? 1 : 0;
  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_UI16) ? 1 : 0;
  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_I16)  ? 1 : 0;

  if (count > 0 && (!GLEW_EXT_texture_integer || !GLEW_EXT_gpu_shader4)) {
    SCHRO_ERROR ("missing extension GL_EXT_texture_integer or "
        "GLEW_EXT_gpu_shader4, can't store U8/S16 as UI8/UI16/I16, disabling "
        "U8/S16 as UI8/UI16/I16 storing");
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_U8_AS_UI8);
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_S16_AS_UI16);
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_S16_AS_I16);
  }

  count = 0;

  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_F16)  ? 1 : 0;
  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_F32)  ? 1 : 0;
  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_F16) ? 1 : 0;
  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_F32) ? 1 : 0;

  if (count > 0 && !GLEW_ARB_texture_float && !GLEW_ATI_texture_float) {
    SCHRO_ERROR ("missing extension GL_{ARB|ATI}_texture_float, can't store "
        "U8/S16 as F16/F32, disabling U8/S16 as F16/F32 storing");
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_U8_AS_F16);
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_U8_AS_F32);
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_S16_AS_F16);
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_S16_AS_F32);
  }

  /* store U8 */
  count = 0;

  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_UI8) ? 1 : 0;
  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_F16) ? 1 : 0;
  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_F32) ? 1 : 0;

  if (count > 1) {
    SCHRO_ERROR ("multiple flags for U8 storage type are set, disabling all");
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_U8_AS_UI8);
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_U8_AS_F16);
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_U8_AS_F32);
  }

  /* store S16 */
  count = 0;

  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_UI16) ? 1 : 0;
  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_I16)  ? 1 : 0;
  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_U16)  ? 1 : 0;
  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_F16)  ? 1 : 0;
  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_F32)  ? 1 : 0;

  if (count > 1) {
    SCHRO_ERROR ("multiple flags for S16 storage type are set, disabling all");
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_S16_AS_UI16);
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_S16_AS_I16);
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_S16_AS_U16);
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_S16_AS_F16);
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_S16_AS_F32);
  }

  /* push */
  if (!SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_RENDER_QUAD) &&
      SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_SHADER)) {
    SCHRO_ERROR ("can't use shader to push without render quad, disabling "
        "shader");
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (PUSH_SHADER);
  }

  count = 0;

  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_UI8) ? 1 : 0;
  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_UI16) ? 1 : 0;
  count += SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_I16)  ? 1 : 0;

  if (count > 0 &&
      SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_RENDER_QUAD) &&
      !SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_SHADER)) {
    SCHRO_ERROR ("can't push U8/S16 as UI8/UI16/I16 shader, disabling "
        "pushing U8/S16 as UI8/UI16/I16");
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_U8_AS_UI8);
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_S16_AS_UI16);
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (STORE_S16_AS_I16);
  }

  if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_RENDER_QUAD) &&
      SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_DRAWPIXELS)) {
    SCHRO_ERROR ("can't render quad and drawpixels to push, disabling "
        "drawpixels push");
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (PUSH_DRAWPIXELS);
  }

  if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_DRAWPIXELS) &&
      !GLEW_ARB_window_pos) {
    SCHRO_ERROR ("missing extension GL_ARB_window_pos, disabling drawpixels "
        "push");
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (PUSH_DRAWPIXELS);
  }

  if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_U8_PIXELBUFFER) &&
      (!GLEW_ARB_vertex_buffer_object || !GLEW_ARB_pixel_buffer_object)) {
    SCHRO_ERROR ("missing extensions GL_ARB_vertex_buffer_object and/or "
        "GL_ARB_pixel_buffer_object, disabling U8 pixelbuffer push");
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (PUSH_U8_PIXELBUFFER);
  }

  if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_S16_PIXELBUFFER) &&
      (!GLEW_ARB_vertex_buffer_object || !GLEW_ARB_pixel_buffer_object)) {
    SCHRO_ERROR ("missing extensions GL_ARB_vertex_buffer_object and/or "
        "GL_ARB_pixel_buffer_object, disabling S16 pixelbuffer push");
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (PUSH_S16_PIXELBUFFER);
  }

  if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_S16_AS_U16) &&
      SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_S16_AS_F32)) {
    SCHRO_ERROR ("can't push S16 as U16 and F32, disabling U16 push");
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (PUSH_S16_AS_U16);
  }

  /* pull */
  if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_PIXELBUFFER) &&
      (!GLEW_ARB_vertex_buffer_object || !GLEW_ARB_pixel_buffer_object)) {
    SCHRO_ERROR ("missing extensions GL_ARB_vertex_buffer_object and/or "
        "GL_ARB_pixel_buffer_object, disabling S16 pixelbuffer pull");
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (PULL_PIXELBUFFER);
  }

  if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_S16_AS_U16) &&
      SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_S16_AS_F32)) {
    SCHRO_ERROR ("can't pull S16 as U16 and F32, disabling U16 pull");
    SCHRO_OPENGL_CANVAS_CLEAR_FLAG (PULL_S16_AS_U16);
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

  PRINT_FLAG ("BGRA:            ", STORE_BGRA);
  PRINT_FLAG ("U8 as UI8:       ", STORE_U8_AS_UI8);
  PRINT_FLAG ("U8 as F16:       ", STORE_U8_AS_F16);
  PRINT_FLAG ("U8 as F32:       ", STORE_U8_AS_F32);
  PRINT_FLAG ("S16 as UI16:     ", STORE_S16_AS_UI16);
  PRINT_FLAG ("S16 as I16:      ", STORE_S16_AS_I16);
  PRINT_FLAG ("S16 as U16:      ", STORE_S16_AS_U16);
  PRINT_FLAG ("S16 as F16:      ", STORE_S16_AS_F16);
  PRINT_FLAG ("S16 as F32:      ", STORE_S16_AS_F32);

  printf ("%spush flags\n", indent);

  PRINT_FLAG ("render quad:     ", PUSH_RENDER_QUAD);
  PRINT_FLAG ("shader:          ", PUSH_SHADER);
  PRINT_FLAG ("drawpixels:      ", PUSH_DRAWPIXELS);
  PRINT_FLAG ("U8 pixelbuffer:  ", PUSH_U8_PIXELBUFFER);
  PRINT_FLAG ("U8 as F32:       ", PUSH_U8_AS_F32);
  PRINT_FLAG ("S16 pixelbuffer: ", PUSH_S16_PIXELBUFFER);
  PRINT_FLAG ("S16 as U16:      ", PUSH_S16_AS_U16);
  PRINT_FLAG ("S16 as F32:      ", PUSH_S16_AS_F32);

  printf ("%spull flags\n", indent);

  PRINT_FLAG ("pixelbuffer:     ", PULL_PIXELBUFFER);
  PRINT_FLAG ("U8 as F32:       ", PULL_U8_AS_F32);
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
  SchroOpenGLResources *resources;

  SCHRO_ASSERT (type >= SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY);
  SCHRO_ASSERT (type <= SCHRO_OPENGL_CANVAS_TYPE_SECONDARY);

  schro_opengl_lock_resources (opengl);

  resources = schro_opengl_get_resources (opengl);

  /* try to reuse existing canvas */
  SCHRO_ASSERT (resources->canvas_count[type] >= 0);
  SCHRO_ASSERT (resources->canvas_count[type] <= SCHRO_OPENGL_RESOURCES_LIMIT);

  for (i = 0; i < resources->canvas_count[type]; ++i) {
    canvas = resources->canvases[type][i];

    /* only reuse primary cansaves, if they are not in use by a caller, only
       in the resources */
    if (type == SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY && canvas->refcount != 1) {
      continue;
    }

    if (canvas->format == format && canvas->width == width &&
        canvas->height == height) {
      ++canvas->refcount;

      schro_opengl_unlock_resources (opengl);

      return canvas;
    }

    ++canvas->uselessness;
  }

  schro_opengl_unlock_resources (opengl);

  /* create new canvas */
  canvas = schro_malloc0 (sizeof (SchroOpenGLCanvas));

  canvas->opengl = opengl;
  canvas->type = type;
  canvas->refcount = 2; /* 1 ref for caller + 1 ref for resources */
  canvas->uselessness = 0;
  canvas->format = format;
  canvas->width = width;
  canvas->height = height;

  schro_opengl_canvas_check_flags (); // FIXME

  switch (SCHRO_FRAME_FORMAT_DEPTH (format)) {
    case SCHRO_FRAME_FORMAT_DEPTH_U8:
      if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_F16)) {
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
        /* must use RGBA format here, because other formats are in general
           not supported by framebuffers */
        canvas->internal_format = GL_RGBA8;
        canvas->storage_type = GL_UNSIGNED_BYTE;
      }

      if (SCHRO_FRAME_IS_PACKED (format)) {
        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_BGRA)) {
          if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_UI8)) {
            canvas->pixel_format = GL_BGRA_INTEGER_EXT;
          } else {
            canvas->pixel_format = GL_BGRA;
          }

          canvas->channels = 4;
        } else {
          if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_UI8)) {
            canvas->pixel_format = GL_RGBA_INTEGER_EXT;
          } else {
            canvas->pixel_format = GL_RGBA;
          }

          canvas->channels = 4;
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
        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_U8_AS_F32)) {
          canvas->push_type = GL_FLOAT;
          canvas->push_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (float));
        } else {
          canvas->push_type = GL_UNSIGNED_BYTE;
          canvas->push_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (uint8_t));
        }

        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_U8_PIXELBUFFER)) {
          canvas->push_pixelbuffer = schro_opengl_pixelbuffer_new (opengl,
              SCHRO_OPENGL_PIXELBUFFER_TYPE_PUSH, width, height,
              canvas->push_stride);
        } else {
          canvas->push_pixelbuffer = NULL;
        }

        /* pull */
        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_U8_AS_F32)) {
          canvas->pull_type = GL_FLOAT;
          canvas->pull_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (float));
        } else {
          canvas->pull_type = GL_UNSIGNED_BYTE;
          canvas->pull_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (uint8_t));
        }

        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_PIXELBUFFER)) {
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
      if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_F16)) {
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
      } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_UI16)) {
        if (SCHRO_FRAME_IS_PACKED (format)) {
          canvas->internal_format = GL_RGBA16UI_EXT;
        } else {
          canvas->internal_format = GL_ALPHA16UI_EXT;
        }

        canvas->storage_type = GL_UNSIGNED_SHORT;
      } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_I16)) {
        if (SCHRO_FRAME_IS_PACKED (format)) {
          canvas->internal_format = GL_RGBA16I_EXT;
        } else {
          canvas->internal_format = GL_ALPHA16I_EXT;
        }

        canvas->type = GL_SHORT;
      } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_U16)) {
        /* must use RGBA format here, because other formats are in general
           not supported by framebuffers */
        canvas->internal_format = GL_RGBA16;
        canvas->storage_type = GL_UNSIGNED_SHORT;
      } else {
        /* must use RGBA format here, because other formats are in general
           not supported by framebuffers */
        canvas->internal_format = GL_RGBA16;
        canvas->storage_type = GL_SHORT;
      }

      if (SCHRO_FRAME_IS_PACKED (format)) {
        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_BGRA)) {
          if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_UI16) ||
              SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_I16)) {
            canvas->pixel_format = GL_BGRA_INTEGER_EXT;
          } else {
            canvas->pixel_format = GL_BGRA;
          }

          canvas->channels = 4;
        } else {
          if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_UI16) ||
              SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_I16)) {
            canvas->pixel_format = GL_RGBA_INTEGER_EXT;
          } else {
            canvas->pixel_format = GL_RGBA;
          }

          canvas->channels = 4;
        }
      } else {
        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_UI16) ||
            SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_I16)) {
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
        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_S16_AS_U16)) {
          canvas->push_type = GL_UNSIGNED_SHORT;
          canvas->push_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (uint16_t));
        } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_S16_AS_F32)) {
          canvas->push_type = GL_FLOAT;
          canvas->push_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (float));
        } else {
          canvas->push_type = GL_SHORT;
          canvas->push_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (int16_t));
        }

        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PUSH_S16_PIXELBUFFER)) {
          canvas->push_pixelbuffer = schro_opengl_pixelbuffer_new (opengl,
              SCHRO_OPENGL_PIXELBUFFER_TYPE_PUSH, width, height,
              canvas->push_stride);
        } else {
          canvas->push_pixelbuffer = NULL;
        }

        /* pull */
        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_S16_AS_U16)) {
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
          /* FIXME: pulling S16 as GL_SHORT doesn't work in general, maybe
             it's the right mode if the internal format is an integer format
             but for some reason storing as I16 doesn't work either and only
             gives garbage pull results */
          canvas->pull_type = GL_SHORT;
          canvas->pull_stride = ROUND_UP_4 (width * canvas->channels
              * sizeof (int16_t));
        }

        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_PIXELBUFFER)) {
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

  schro_opengl_lock_context (opengl);

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

  schro_opengl_unlock_context (opengl);

  /* add new canvas to resources */
  schro_opengl_lock_resources (opengl);

  SCHRO_ASSERT (resources->canvas_count[type] >= 0);
  SCHRO_ASSERT (resources->canvas_count[type] < SCHRO_OPENGL_RESOURCES_LIMIT);

  resources->canvases[type][resources->canvas_count[type]] = canvas;
  ++resources->canvas_count[type];

  schro_opengl_unlock_resources (opengl);

  return canvas;
}

void
schro_opengl_canvas_unref (SchroOpenGLCanvas *canvas)
{
  int i;
  SchroOpenGLResources *resources;

  schro_opengl_lock_resources (canvas->opengl);

  SCHRO_ASSERT (canvas->refcount > 0);

  --canvas->refcount;

  if (canvas->refcount > 0) {
    schro_opengl_unlock_resources (canvas->opengl);
    return;
  }

  /* remove canvas from the resources */
  resources = schro_opengl_get_resources (canvas->opengl);

  SCHRO_ASSERT (canvas->type >= SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY);
  SCHRO_ASSERT (canvas->type <= SCHRO_OPENGL_CANVAS_TYPE_SECONDARY);
  SCHRO_ASSERT (resources->canvas_count[canvas->type] >= 1);
  SCHRO_ASSERT (resources->canvas_count[canvas->type]
      <= SCHRO_OPENGL_RESOURCES_LIMIT);

  for (i = 0; i < resources->canvas_count[canvas->type]; ++i) {
    if (resources->canvases[canvas->type][i] == canvas) {
      --resources->canvas_count[canvas->type];

      /* move the last canvas in the array to the slot of the removed
         one to maintain the array continuous in memory */
      resources->canvases[canvas->type][i]
          = resources->canvases[canvas->type]
          [resources->canvas_count[canvas->type]];

      break;
    }
  }

  schro_opengl_unlock_resources (canvas->opengl);

  /* free OpenGL handles */
  schro_opengl_lock_context (canvas->opengl);

  SCHRO_ASSERT (glIsTexture (canvas->texture));
  SCHRO_ASSERT (glIsFramebufferEXT (canvas->framebuffer));

  glDeleteTextures (1, &canvas->texture);
  glDeleteFramebuffersEXT (1, &canvas->framebuffer);

  schro_opengl_unlock_context (canvas->opengl);

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
  SchroOpenGLResources *resources;

  SCHRO_ASSERT (type >= SCHRO_OPENGL_PIXELBUFFER_TYPE_PUSH);
  SCHRO_ASSERT (type <= SCHRO_OPENGL_PIXELBUFFER_TYPE_PULL);

  schro_opengl_lock_resources (opengl);

  resources = schro_opengl_get_resources (opengl);

  /* try to reuse existing pixelbuffer */
  SCHRO_ASSERT (resources->pixelbuffer_count[type] >= 0);
  SCHRO_ASSERT (resources->pixelbuffer_count[type]
      <= SCHRO_OPENGL_RESOURCES_LIMIT);

  for (i = 0; i < resources->pixelbuffer_count[type]; ++i) {
    pixelbuffer = resources->pixelbuffers[type][i];

    if (pixelbuffer->width == width && pixelbuffer->height == height &&
        pixelbuffer->stride == stride) {
      ++pixelbuffer->refcount;

      schro_opengl_unlock_resources (opengl);

      return pixelbuffer;
    }
  }

  schro_opengl_unlock_resources (opengl);

  /* create new pixelbuffer */
  pixelbuffer = schro_malloc0 (sizeof (SchroOpenGLPixelbuffer));

  pixelbuffer->opengl = opengl;
  pixelbuffer->type = type;
  pixelbuffer->refcount = 2; /* 1 ref for caller + 1 ref for resources */
  pixelbuffer->uselessness = 0;
  pixelbuffer->width = width;
  pixelbuffer->height = height;
  pixelbuffer->stride = stride;

  schro_opengl_lock_context (opengl);

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

  schro_opengl_unlock_context (opengl);

  /* add new pixelbuffer to resources */
  schro_opengl_lock_resources (opengl);

  SCHRO_ASSERT (resources->pixelbuffer_count[type] >= 0);
  SCHRO_ASSERT (resources->pixelbuffer_count[type]
      < SCHRO_OPENGL_RESOURCES_LIMIT);

  resources->pixelbuffers[type][resources->pixelbuffer_count[type]]
      = pixelbuffer;
  ++resources->pixelbuffer_count[type];

  schro_opengl_unlock_resources (opengl);

  return pixelbuffer;
}

void
schro_opengl_pixelbuffer_unref (SchroOpenGLPixelbuffer *pixelbuffer)
{
  int i;
  SchroOpenGLResources *resources;

  schro_opengl_lock_resources (pixelbuffer->opengl);

  SCHRO_ASSERT (pixelbuffer->refcount > 0);

  --pixelbuffer->refcount;

  if (pixelbuffer->refcount > 0) {
    schro_opengl_unlock_resources (pixelbuffer->opengl);
    return;
  }

  /* remove pixelbuffer from the resources */
  resources = schro_opengl_get_resources (pixelbuffer->opengl);

  SCHRO_ASSERT (pixelbuffer->type >= SCHRO_OPENGL_PIXELBUFFER_TYPE_PUSH);
  SCHRO_ASSERT (pixelbuffer->type <= SCHRO_OPENGL_PIXELBUFFER_TYPE_PULL);
  SCHRO_ASSERT (resources->pixelbuffer_count[pixelbuffer->type] >= 1);
  SCHRO_ASSERT (resources->pixelbuffer_count[pixelbuffer->type]
      <= SCHRO_OPENGL_RESOURCES_LIMIT);

  for (i = 0; i < resources->pixelbuffer_count[pixelbuffer->type]; ++i) {
    if (resources->pixelbuffers[pixelbuffer->type][i] == pixelbuffer) {
      --resources->pixelbuffer_count[pixelbuffer->type];

      /* move the last pixelbuffer in the array to the slot of the removed
         one to maintain the array continuous in memory */
      resources->pixelbuffers[pixelbuffer->type][i]
          = resources->pixelbuffers[pixelbuffer->type]
          [resources->pixelbuffer_count[pixelbuffer->type]];

      break;
    }
  }

  schro_opengl_unlock_resources (pixelbuffer->opengl);

  /* free OpenGL handles */
  schro_opengl_lock_context (pixelbuffer->opengl);

  for (i = 0; i < SCHRO_OPENGL_PIXELBUFFER_BLOCKS; ++i) {
    SCHRO_ASSERT (glIsBufferARB (pixelbuffer->handles[i]));

    glDeleteBuffersARB (1, &pixelbuffer->handles[i]);
  }

  schro_opengl_unlock_context (pixelbuffer->opengl);

  schro_free (pixelbuffer);
}

SchroOpenGLResources *
schro_opengl_resources_new (SchroOpenGL *opengl)
{
  SchroOpenGLResources *resources;

  resources = schro_malloc0 (sizeof (SchroOpenGLResources));

  resources->opengl = opengl;
  resources->canvas_count[SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY] = 0;
  resources->canvas_count[SCHRO_OPENGL_CANVAS_TYPE_SECONDARY] = 0;
  resources->pixelbuffer_count[SCHRO_OPENGL_PIXELBUFFER_TYPE_PUSH] = 0;
  resources->pixelbuffer_count[SCHRO_OPENGL_PIXELBUFFER_TYPE_PULL] = 0;

  return resources;
}

void
schro_opengl_resources_free (SchroOpenGLResources* resources)
{
  int i;

  schro_opengl_lock_resources (resources->opengl);

  #define UNREF_CANVASES(_type) \
      do { \
        for (i = 0; i < resources->canvas_count[_type]; ++i) { \
          SCHRO_ASSERT (resources->canvases[_type][i]->refcount == 1); \
        } \
        while (resources->canvas_count[_type] > 0) { \
          schro_opengl_canvas_unref (resources->canvases[_type][0]); \
        } \
        SCHRO_ASSERT (resources->canvas_count[_type] == 0); \
      } while (0)

  #define UNREF_PIXELBUFFERS(_type) \
      do { \
        for (i = 0; i < resources->pixelbuffer_count[_type]; ++i) { \
          SCHRO_ASSERT (resources->pixelbuffers[_type][i]->refcount == 1); \
        } \
        while (resources->pixelbuffer_count[_type] > 0) { \
          schro_opengl_pixelbuffer_unref (resources->pixelbuffers[_type][0]); \
        } \
        SCHRO_ASSERT (resources->pixelbuffer_count[_type] == 0); \
      } while (0)

  UNREF_CANVASES (SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY);
  UNREF_CANVASES (SCHRO_OPENGL_CANVAS_TYPE_SECONDARY);
  UNREF_PIXELBUFFERS (SCHRO_OPENGL_PIXELBUFFER_TYPE_PUSH);
  UNREF_PIXELBUFFERS (SCHRO_OPENGL_PIXELBUFFER_TYPE_PULL);

  #undef UNREF_CANVASES
  #undef UNREF_PIXELBUFFERS

  schro_opengl_unlock_resources (resources->opengl);

  schro_free (resources);
}

