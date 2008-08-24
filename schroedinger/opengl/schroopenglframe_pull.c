
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <schroedinger/schrodebug.h>
#include <schroedinger/opengl/schroopengl.h>
#include <schroedinger/opengl/schroopenglcanvas.h>
#include <schroedinger/opengl/schroopenglframe.h>
#include <liboil/liboil.h>

static void
schro_opengl_canvas_pull_convert (SchroFrameData *dest, SchroOpenGLCanvas *src,
    void *texture_data, int y_offset, int height)
{
  int x, y;
  int width;
  int frame_stride, texture_stride, texture_channels;
  float temp_f32;
  uint8_t *texture_data_u8 = NULL;
  uint16_t *texture_data_u16 = NULL;
  int16_t *texture_data_s16 = NULL;
  float *texture_data_f32 = NULL;
  uint8_t *frame_data_u8 = NULL;
  int16_t *frame_data_s16 = NULL;

  SCHRO_ASSERT (dest != NULL);
  SCHRO_ASSERT (src != NULL);
  SCHRO_ASSERT (dest->format == src->format);
  SCHRO_ASSERT (dest->width == src->width);
  SCHRO_ASSERT (dest->height == src->height);
  SCHRO_ASSERT (texture_data != NULL);

  width = dest->width;
  frame_stride = dest->stride;
  texture_stride = src->pull_stride;
  texture_channels = SCHRO_FRAME_IS_PACKED (dest->format) ? 1 : src->channels;

  switch (SCHRO_FRAME_FORMAT_DEPTH (dest->format)) {
    case SCHRO_FRAME_FORMAT_DEPTH_U8:
      frame_data_u8 = SCHRO_FRAME_DATA_GET_LINE (dest, y_offset);

      if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_U8_AS_U8)) {
        texture_data_u8 = (uint8_t *) texture_data;

        if (texture_channels > 1) {
          for (y = 0; y < height; ++y) {
            for (x = 0; x < width; ++x) {
              frame_data_u8[x] = texture_data_u8[x * texture_channels];
            }

            frame_data_u8 = OFFSET (frame_data_u8, frame_stride);
            texture_data_u8 = OFFSET (texture_data_u8, texture_stride);
          }
        } else {
          for (y = 0; y < height; ++y) {
            oil_memcpy (frame_data_u8, texture_data_u8, width);

            frame_data_u8 = OFFSET (frame_data_u8, frame_stride);
            texture_data_u8 = OFFSET (texture_data_u8, texture_stride);
          }
        }
      } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_U8_AS_F32)) {
        texture_data_f32 = (float *) texture_data;

        if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_F16)) {
          // FIXME: need to scale with 256.0 instead of 255.0 if pulling U8
          // values as F32 stored as F16 to get correct values back
          for (y = 0; y < height; ++y) {
            for (x = 0; x < width; ++x) {
              temp_f32 = texture_data_f32[x * texture_channels] * 256.0;
              frame_data_u8[x] = (uint8_t) MIN (temp_f32, 255.0);
            }

            frame_data_u8 = OFFSET (frame_data_u8, frame_stride);
            texture_data_f32 = OFFSET (texture_data_f32, texture_stride);
          }
        } else {
          for (y = 0; y < height; ++y) {
            for (x = 0; x < width; ++x) {
              frame_data_u8[x]
                  = (uint8_t) (texture_data_f32[x * texture_channels] * 255.0);
            }

            frame_data_u8 = OFFSET (frame_data_u8, frame_stride);
            texture_data_f32 = OFFSET (texture_data_f32, texture_stride);
          }
        }
      } else {
        SCHRO_ERROR ("invalid canvas flags combination, one U8 pull type "
            "flag must be set");
        SCHRO_ASSERT (0);
      }

      break;
    case SCHRO_FRAME_FORMAT_DEPTH_S16:
      frame_data_s16 = SCHRO_FRAME_DATA_GET_LINE (dest, y_offset);

      if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_S16_AS_S16)) {
        texture_data_s16 = (int16_t *) texture_data;

        if (texture_channels > 1) {
          for (y = 0; y < height; ++y) {
            for (x = 0; x < width; ++x) {
              frame_data_s16[x] = texture_data_s16[x * texture_channels];
            }

            frame_data_s16 = OFFSET (frame_data_s16, frame_stride);
            texture_data_s16 = OFFSET (texture_data_s16, texture_stride);
          }
        } else {
          for (y = 0; y < height; ++y) {
            oil_memcpy (frame_data_s16, texture_data_s16,
                width * sizeof (int16_t));

            frame_data_s16 = OFFSET (frame_data_s16, frame_stride);
            texture_data_s16 = OFFSET (texture_data_s16, texture_stride);
          }
        }
      } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_S16_AS_U16)) {
        texture_data_u16 = (uint16_t *) texture_data;

        for (y = 0; y < height; ++y) {
          for (x = 0; x < width; ++x) {
            frame_data_s16[x]
                = (int16_t) ((int32_t) texture_data_u16[x * texture_channels]
                - 32768);
          }

          frame_data_s16 = OFFSET (frame_data_s16, frame_stride);
          texture_data_u16 = OFFSET (texture_data_u16, texture_stride);
        }
      } else if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_S16_AS_F32)) {
        texture_data_f32 = (float *) texture_data;

        for (y = 0; y < height; ++y) {
          for (x = 0; x < width; ++x) {
            // FIXME: for some unknown reason I need to scale with 65536.0
            // instead of 65535.0 to get correct S16 value back. I also get
            // correct S16 values with rounding: round (x) := floor (x + 0.5)
            // but thats way to expensive
            frame_data_s16[x]
                = (int16_t) ((int32_t) (texture_data_f32[x * texture_channels]
                * 65536.0) - 32768);
          }

          frame_data_s16 = OFFSET (frame_data_s16, frame_stride);
          texture_data_f32 = OFFSET (texture_data_f32, texture_stride);
        }
      } else {
        SCHRO_ERROR ("invalid canvas flags combination, one S16 pull type "
            "flag must be set");
        SCHRO_ASSERT (0);
      }

      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_frame_pull (SchroFrame *dest, SchroFrame *src)
{
  int i;
  int components;
  SchroOpenGLCanvas *src_canvas;

  SCHRO_ASSERT (dest != NULL);
  SCHRO_ASSERT (src != NULL);
  SCHRO_ASSERT (!SCHRO_FRAME_IS_OPENGL (dest));
  SCHRO_ASSERT (SCHRO_FRAME_IS_OPENGL (src));
  SCHRO_ASSERT (dest->format == src->format);

  components = SCHRO_FRAME_IS_PACKED (src->format) ? 1 : 3;
  src_canvas = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA (src->components + 0);

  SCHRO_ASSERT (src_canvas != NULL);

  SCHRO_OPENGL_LOCK_CONTEXT (src_canvas->opengl);

  for (i = 0; i < components; ++i) {
    src_canvas = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA (src->components + i);

    SCHRO_ASSERT (src_canvas != NULL);

    schro_opengl_canvas_pull (dest->components + i, src_canvas);
  }

  SCHRO_OPENGL_UNLOCK_CONTEXT (src_canvas->opengl);
}

void
schro_opengl_canvas_pull (SchroFrameData *dest, SchroOpenGLCanvas *src)
{
  int i;
  int width, height, depth;
  int pixelbuffer_y_offset, pixelbuffer_height;
  void *mapped_data = NULL;
  void *tmp_data = NULL;

  SCHRO_ASSERT (src->type == SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY);
  SCHRO_ASSERT (dest->format == src->format);
  SCHRO_ASSERT (dest->width == src->width);
  SCHRO_ASSERT (dest->height == src->height);

  width = dest->width;
  height = dest->height;
  depth = SCHRO_FRAME_FORMAT_DEPTH (dest->format);

  SCHRO_OPENGL_LOCK_CONTEXT (src->opengl);

  if ((SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_U8_PIXELBUFFER) &&
      depth == SCHRO_FRAME_FORMAT_DEPTH_U8) ||
      (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (PULL_S16_PIXELBUFFER) &&
      depth == SCHRO_FRAME_FORMAT_DEPTH_S16)) {
    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, src->framebuffer);

    pixelbuffer_y_offset = 0;

    for (i = 0; i < SCHRO_OPENGL_PIXELBUFFER_BLOCKS; ++i) {
      pixelbuffer_height = src->pull_pixelbuffer->heights[i];

      glBindBufferARB (GL_PIXEL_PACK_BUFFER_ARB,
          src->pull_pixelbuffer->handles[i]);
      glReadPixels (0, pixelbuffer_y_offset, width, pixelbuffer_height,
          src->pixel_format, src->pull_type, NULL);

      pixelbuffer_y_offset += pixelbuffer_height;

      SCHRO_OPENGL_CHECK_ERROR
    }

    pixelbuffer_y_offset = 0;

    for (i = 0; i < SCHRO_OPENGL_PIXELBUFFER_BLOCKS; ++i) {
      pixelbuffer_height = src->pull_pixelbuffer->heights[i];

      glBindBufferARB (GL_PIXEL_PACK_BUFFER_ARB,
          src->pull_pixelbuffer->handles[i]);

      mapped_data = glMapBufferARB (GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);

      schro_opengl_canvas_pull_convert (dest, src, mapped_data,
          pixelbuffer_y_offset, pixelbuffer_height);

      glUnmapBufferARB (GL_PIXEL_PACK_BUFFER_ARB);

      pixelbuffer_y_offset += pixelbuffer_height;

      SCHRO_OPENGL_CHECK_ERROR
    }

    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
  } else {
    tmp_data = schro_opengl_get_tmp (src->opengl, src->pull_stride * height);

    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, src->framebuffer);

    glReadPixels (0, 0, width, height, src->pixel_format, src->pull_type,
        tmp_data);

    SCHRO_OPENGL_CHECK_ERROR

    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);

    schro_opengl_canvas_pull_convert (dest, src, tmp_data, 0, height);
  }

  SCHRO_OPENGL_UNLOCK_CONTEXT (src->opengl);
}

