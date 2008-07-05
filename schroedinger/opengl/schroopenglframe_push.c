
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <schroedinger/schro.h>
#include <schroedinger/opengl/schroopengl.h>
#include <schroedinger/opengl/schroopenglframe.h>
#include <schroedinger/opengl/schroopenglshader.h>
#include <liboil/liboil.h>

static void
schro_opengl_frame_push_convert (SchroFrameData *dest, SchroFrameData *src,
    void *texture_data, int y_offset, int height)
{
  int x, y;
  int width, depth;
  int frame_stride, texture_stride, texture_channels;
  SchroOpenGLCanvas *dest_canvas = NULL;
  uint8_t *frame_data_u8 = NULL;
  int16_t *frame_data_s16 = NULL;
  uint8_t *texture_data_u8 = NULL;
  uint16_t *texture_data_u16 = NULL;
  int16_t *texture_data_s16 = NULL;
  float *texture_data_f32 = NULL;

  SCHRO_ASSERT (dest != NULL);
  SCHRO_ASSERT (src != NULL);
  SCHRO_ASSERT (dest->format == src->format);
  SCHRO_ASSERT (texture_data != NULL);
  SCHRO_ASSERT (dest->stride == src->stride);
  SCHRO_ASSERT (dest->width == src->width);

  width = src->width;
  depth = SCHRO_FRAME_FORMAT_DEPTH (src->format);
  frame_stride = src->stride;
  // FIXME: hack to store custom data per frame component
  dest_canvas = *((SchroOpenGLCanvas **) dest->data);
  texture_stride = dest_canvas->push.stride;
  texture_channels = SCHRO_FRAME_IS_PACKED (src->format)
      ? 1 : dest_canvas->texture.channels;

  if (depth == SCHRO_FRAME_FORMAT_DEPTH_U8) {
    frame_data_u8 = SCHRO_FRAME_DATA_GET_LINE (src, y_offset);

    if (SCHRO_OPENGL_FRAME_IS_FLAG_SET (PUSH_U8_AS_F32)) {
      texture_data_f32 = (float *) texture_data;

      for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
          texture_data_f32[x * texture_channels]
              = (float) frame_data_u8[x] / 255.0;
        }

        texture_data_f32 = OFFSET (texture_data_f32, texture_stride);
        frame_data_u8 = OFFSET (frame_data_u8, frame_stride);
      }
    } else {
      texture_data_u8 = (uint8_t *) texture_data;

      if (texture_channels > 1) {
        for (y = 0; y < height; ++y) {
          for (x = 0; x < width; ++x) {
            texture_data_u8[x * texture_channels] = frame_data_u8[x];
          }

          texture_data_u8 = OFFSET (texture_data_u8, texture_stride);
          frame_data_u8 = OFFSET (frame_data_u8, frame_stride);
        }
      } else {
        for (y = 0; y < height; ++y) {
          oil_memcpy (texture_data_u8, frame_data_u8, width);

          texture_data_u8 = OFFSET (texture_data_u8, texture_stride);
          frame_data_u8 = OFFSET (frame_data_u8, frame_stride);
        }
      }
    }
  } else if (depth == SCHRO_FRAME_FORMAT_DEPTH_S16) {
    frame_data_s16 = SCHRO_FRAME_DATA_GET_LINE (src, y_offset);

    if (SCHRO_OPENGL_FRAME_IS_FLAG_SET (PUSH_S16_AS_U16)) {
      texture_data_u16 = (uint16_t *) texture_data;

      for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
          texture_data_u16[x * texture_channels]
              = (uint16_t) ((int32_t) frame_data_s16[x] + 32768);
        }

        texture_data_u16 = OFFSET (texture_data_u16, texture_stride);
        frame_data_s16 = OFFSET (frame_data_s16, frame_stride);
      }
    } else if (SCHRO_OPENGL_FRAME_IS_FLAG_SET (PUSH_S16_AS_F32)) {
      texture_data_f32 = (float *) texture_data;

      for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
          texture_data_f32[x * texture_channels]
              = (float) ((int32_t) frame_data_s16[x] + 32768) / 65535.0;
        }

        texture_data_f32 = OFFSET (texture_data_f32, texture_stride);
        frame_data_s16 = OFFSET (frame_data_s16, frame_stride);
      }
    } else {
      texture_data_s16 = (int16_t *) texture_data;

      if (texture_channels > 1) {
        for (y = 0; y < height; ++y) {
          for (x = 0; x < width; ++x) {
            texture_data_s16[x * texture_channels] = frame_data_s16[x];
          }

          texture_data_s16 = OFFSET (texture_data_s16, texture_stride);
          frame_data_s16 = OFFSET (frame_data_s16, frame_stride);
        }
      } else {
        for (y = 0; y < height; ++y) {
          oil_memcpy (texture_data_s16, frame_data_s16,
              width * sizeof (int16_t));

          texture_data_s16 = OFFSET (texture_data_s16, texture_stride);
          frame_data_s16 = OFFSET (frame_data_s16, frame_stride);
        }
      }
    }
  } else {
    SCHRO_ERROR ("unhandled depth");
    SCHRO_ASSERT (0);
  }
}

void
schro_opengl_frame_push (SchroFrame *dest, SchroFrame *src)
{
  int i, k;
  int width, height, depth;
  int components;
  int pixelbuffer_y_offset, pixelbuffer_height;
  SchroOpenGLCanvas *dest_canvas = NULL;
  SchroOpenGL *opengl = NULL;
  GLuint src_texture = 0; // FIXME: don't create a new locale texture here
                          // but use a single global texture for such temporary
                          // purpose
  void *mapped_data = NULL;
  void *tmp_data = NULL;
  SchroOpenGLShader *shader;

  SCHRO_ASSERT (dest != NULL);
  SCHRO_ASSERT (src != NULL);
  SCHRO_ASSERT (SCHRO_FRAME_IS_OPENGL (dest));
  SCHRO_ASSERT (!SCHRO_FRAME_IS_OPENGL (src));
  SCHRO_ASSERT (dest->format == src->format);

  components = SCHRO_FRAME_IS_PACKED (src->format) ? 1 : 3;
  // FIXME: hack to store custom data per frame component
  dest_canvas = *((SchroOpenGLCanvas **) dest->components[0].data);

  SCHRO_ASSERT (dest_canvas != NULL);

  opengl = dest_canvas->opengl;

  schro_opengl_lock (opengl);

  for (i = 0; i < components; ++i) {
    // FIXME: hack to store custom data per frame component
    dest_canvas = *((SchroOpenGLCanvas **) dest->components[i].data);

    SCHRO_ASSERT (dest_canvas != NULL);
    SCHRO_ASSERT (dest_canvas->opengl == opengl);

    width = dest->components[i].width;
    height = dest->components[i].height;
    depth = SCHRO_FRAME_FORMAT_DEPTH (src->format);

    //SCHRO_ASSERT (stride == src->components[i].stride);
    SCHRO_ASSERT (width == src->components[i].width);
    SCHRO_ASSERT (height == src->components[i].height);

    if (SCHRO_OPENGL_FRAME_IS_FLAG_SET (PUSH_RENDER_QUAD)) {
      glGenTextures (1, &src_texture);
      glBindTexture (GL_TEXTURE_RECTANGLE_ARB, src_texture);
      glTexImage2D (GL_TEXTURE_RECTANGLE_ARB, 0,
          dest_canvas->texture.internal_format, width, height, 0,
          dest_canvas->texture.pixel_format, dest_canvas->texture.type, NULL);
      glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
          GL_NEAREST);
      glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
          GL_NEAREST);
      glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

      SCHRO_OPENGL_CHECK_ERROR
    }

    if ((SCHRO_OPENGL_FRAME_IS_FLAG_SET (PUSH_U8_PIXELBUFFER) &&
        depth == SCHRO_FRAME_FORMAT_DEPTH_U8) ||
        (SCHRO_OPENGL_FRAME_IS_FLAG_SET (PUSH_S16_PIXELBUFFER) &&
        depth == SCHRO_FRAME_FORMAT_DEPTH_S16)) {
      pixelbuffer_y_offset = 0;

      for (k = 0; k < SCHRO_OPENGL_TRANSFER_PIXELBUFFERS; ++k) {
        pixelbuffer_height = dest_canvas->push.heights[k];

        glBindBufferARB (GL_PIXEL_UNPACK_BUFFER_EXT,
            dest_canvas->push.pixelbuffers[k]);
        glBufferDataARB (GL_PIXEL_UNPACK_BUFFER_ARB,
            dest_canvas->push.stride * pixelbuffer_height, NULL,
            GL_STREAM_DRAW_ARB);

        mapped_data = glMapBufferARB (GL_PIXEL_UNPACK_BUFFER_EXT,
            GL_WRITE_ONLY_ARB);

        schro_opengl_frame_push_convert (dest->components + i,
            src->components + i, mapped_data, pixelbuffer_y_offset,
            pixelbuffer_height);

        glUnmapBufferARB (GL_PIXEL_UNPACK_BUFFER_EXT);

        pixelbuffer_y_offset += pixelbuffer_height;

        SCHRO_OPENGL_CHECK_ERROR
      }

      if (!SCHRO_OPENGL_FRAME_IS_FLAG_SET (PUSH_RENDER_QUAD)) {
        glBindTexture (GL_TEXTURE_RECTANGLE_ARB,
            dest_canvas->texture.handles[0]);
      }

      pixelbuffer_y_offset = 0;

      if (dest_canvas->push.type == GL_SHORT) {
        /* OpenGL maps signed values different to float values than unsigned
           values. for S16 -32768 is mapped to -1.0 and 32767 to 1.0, for U16
           0 is mapped to 0.0 and 65535 to 1.0. after this mapping scale and
           bias are applied and the resulting value is clamped to [0..1].
           with default scale = 1 and default bias = 0 all negative values
           from S16 are clamped to 0.0, changing scale and bias to 0.5 gives
           a unclamped mapping that doesn't discard all negative values for
           S16 */
        glPixelTransferf (GL_RED_SCALE, 0.5);
        glPixelTransferf (GL_RED_BIAS, 0.5);
      }

      for (k = 0; k < SCHRO_OPENGL_TRANSFER_PIXELBUFFERS; ++k) {
        pixelbuffer_height = dest_canvas->push.heights[k];

        glBindBufferARB (GL_PIXEL_UNPACK_BUFFER_EXT,
            dest_canvas->push.pixelbuffers[k]);

        SCHRO_OPENGL_CHECK_ERROR

        if (SCHRO_OPENGL_FRAME_IS_FLAG_SET (PUSH_DRAWPIXELS)) {
          glBindFramebufferEXT (GL_FRAMEBUFFER_EXT,
              dest_canvas->framebuffers[0]);

          glWindowPos2iARB (0, pixelbuffer_y_offset);
          glDrawPixels (width, pixelbuffer_height,
              dest_canvas->texture.pixel_format, dest_canvas->push.type, NULL);

          glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
        } else {
          glTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0,
              pixelbuffer_y_offset, width, pixelbuffer_height,
              dest_canvas->texture.pixel_format, dest_canvas->push.type, NULL);
        }

        SCHRO_OPENGL_CHECK_ERROR

        pixelbuffer_y_offset += pixelbuffer_height;
      }

      if (dest_canvas->push.type == GL_SHORT) {
        glPixelTransferf (GL_RED_SCALE, 1);
        glPixelTransferf (GL_RED_BIAS, 0);
      }

      glBindBufferARB (GL_PIXEL_UNPACK_BUFFER_EXT, 0);
    } else {
      tmp_data = schro_opengl_get_tmp (opengl,
          dest_canvas->push.stride * height);

      schro_opengl_frame_push_convert (dest->components + i,
          src->components + i, tmp_data, 0, height);

      if (!SCHRO_OPENGL_FRAME_IS_FLAG_SET (PUSH_RENDER_QUAD)) {
        glBindTexture (GL_TEXTURE_RECTANGLE_ARB,
            dest_canvas->texture.handles[0]);
      }

      if (dest_canvas->push.type == GL_SHORT) {
        /* OpenGL maps signed values different to float values than unsigned
           values. for S16 -32768 is mapped to -1.0 and 32767 to 1.0, for U16
           0 is mapped to 0.0 and 65535 to 1.0. after this mapping scale and
           bias are applied and the resulting value is clamped to [0..1].
           with default scale = 1 and default bias = 0 all negative values
           from S16 are clamped to 0.0, changing scale and bias to 0.5 gives
           a unclamped mapping that doesn't discard all negative values for
           S16 */
        glPixelTransferf (GL_RED_SCALE, 0.5);
        glPixelTransferf (GL_RED_BIAS, 0.5);
      }

      if (SCHRO_OPENGL_FRAME_IS_FLAG_SET (PUSH_DRAWPIXELS)) {
        glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, dest_canvas->framebuffers[0]);

        glWindowPos2iARB (0, 0);
        glDrawPixels (width, height, dest_canvas->texture.pixel_format,
            dest_canvas->push.type, tmp_data);

        glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
      } else {
        glTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, width, height,
            dest_canvas->texture.pixel_format, dest_canvas->push.type,
            tmp_data);
      }

      if (dest_canvas->push.type == GL_SHORT) {
        glPixelTransferf (GL_RED_SCALE, 1);
        glPixelTransferf (GL_RED_BIAS, 0);
      }
    }

    SCHRO_OPENGL_CHECK_ERROR

    if (SCHRO_OPENGL_FRAME_IS_FLAG_SET (PUSH_RENDER_QUAD)) {
      schro_opengl_setup_viewport (width, height);

      glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, dest_canvas->framebuffers[0]);

      SCHRO_OPENGL_CHECK_ERROR

      if (SCHRO_OPENGL_FRAME_IS_FLAG_SET (PUSH_SHADER)) {
        shader = schro_opengl_shader_get (opengl,
            SCHRO_OPENGL_SHADER_IDENTITY);

        glUseProgramObjectARB (shader->program);
        glUniform1iARB (shader->textures[0], 0);
      }

      schro_opengl_render_quad (0, 0, width, height);

      if (SCHRO_OPENGL_FRAME_IS_FLAG_SET (PUSH_SHADER)) {
        glUseProgramObjectARB (0);
      }

      glFlush ();

      glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);

      SCHRO_OPENGL_CHECK_ERROR
    }

    glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);

    if (SCHRO_OPENGL_FRAME_IS_FLAG_SET (PUSH_RENDER_QUAD)) {
      glDeleteTextures (1, &src_texture);
    }
  }

  schro_opengl_unlock (opengl);
}

