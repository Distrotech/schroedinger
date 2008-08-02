 
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <schroedinger/schro.h>
#include <schroedinger/opengl/schroopengl.h>
#include <schroedinger/opengl/schroopenglcanvas.h>
#include <schroedinger/opengl/schroopenglframe.h>
#include <schroedinger/opengl/schroopenglmotion.h>
#include <schroedinger/opengl/schroopenglshader.h>
#include <stdio.h>

typedef struct _SchroOpenGLMotion SchroOpenGLMotion;

struct _SchroOpenGLMotion {
  SchroMotion *motion;
  SchroOpenGLCanvas *src_canvases[2][4];
  SchroOpenGLShader *shader_dc;
  SchroOpenGLShader *shader_ref_prec_0;
  SchroOpenGLShader *shader_ref_prec_0_weight;
  SchroOpenGLShader *shader_ref_prec_3a;
  SchroOpenGLShader *shader_ref_prec_3a_weight;
  SchroOpenGLShader *shader_ref_prec_3b;
  SchroOpenGLShader *shader_ref_prec_3b_weight;
  SchroOpenGLShader *shader_biref_prec_0_0;
  SchroOpenGLShader *shader_biref_prec_0_0_weight;
  SchroOpenGLShader *shader_biref_prec_0_3a;
  SchroOpenGLShader *shader_biref_prec_0_3a_weight;
  SchroOpenGLShader *shader_biref_prec_0_3b;
  SchroOpenGLShader *shader_biref_prec_0_3b_weight;
  SchroOpenGLShader *shader_biref_prec_3a_0;
  SchroOpenGLShader *shader_biref_prec_3a_0_weight;
  SchroOpenGLShader *shader_biref_prec_3a_3a;
  SchroOpenGLShader *shader_biref_prec_3a_3a_weight;
  SchroOpenGLShader *shader_biref_prec_3a_3b;
  SchroOpenGLShader *shader_biref_prec_3a_3b_weight;
  SchroOpenGLShader *shader_biref_prec_3b_0;
  SchroOpenGLShader *shader_biref_prec_3b_0_weight;
  SchroOpenGLShader *shader_biref_prec_3b_3a;
  SchroOpenGLShader *shader_biref_prec_3b_3a_weight;
  SchroOpenGLShader *shader_biref_prec_3b_3b;
  SchroOpenGLShader *shader_biref_prec_3b_3b_weight;
};

struct _SchroOpenGLSpatialWeightBlock {
  SchroOpenGLSpatialWeightPool* pool;
  int refcount;

  int edges[4]; /* left, top, right, bottom */
  int xbsep;
  int ybsep;
  int xblen;
  int yblen;
  SchroOpenGLCanvas* canvas;
};

struct _SchroOpenGLSpatialWeightGrid {
  SchroOpenGLSpatialWeightPool* pool;
  int refcount;

  int width;
  int height;
  int x_num_blocks;
  int y_num_blocks;
  int xbsep;
  int ybsep;
  int xblen;
  int yblen;
  SchroOpenGLSpatialWeightBlock **blocks;
};

#define SCHRO_OPENGL_SPATIAL_WEIGHT_POOL_LIMIT 1024

struct _SchroOpenGLSpatialWeightPool {
  SchroOpenGLSpatialWeightBlock *blocks[SCHRO_OPENGL_SPATIAL_WEIGHT_POOL_LIMIT];
  int block_count;

  SchroOpenGLSpatialWeightGrid *grids[SCHRO_OPENGL_SPATIAL_WEIGHT_POOL_LIMIT];
  int grid_count;
};

static void
schro_opengl_motion_render_dc_block (SchroOpenGLMotion *opengl_motion, int i,
    int x, int y, int u, int v)
{
  int xblen, yblen;
  SchroMotion *motion;
  SchroMotionVectorDC *motion_vector_dc;
  uint8_t dc;

  motion = opengl_motion->motion;
  motion_vector_dc = (SchroMotionVectorDC *)
      &motion->motion_vectors[v * motion->params->x_num_blocks + u];
  dc = (int) motion_vector_dc->dc[i] + 128;

  glUseProgramObjectARB (opengl_motion->shader_dc->program);

  glUniform2fARB (opengl_motion->shader_dc->origin, x, y);
  glUniform1fARB (opengl_motion->shader_dc->dc, dc);

  if (x < 0) {
    xblen = motion->xblen + x;
    x = 0;
  } else {
    xblen = motion->xblen;
  }

  if (y < 0) {
    yblen = motion->yblen + y;
    y = 0;
  } else {
    yblen = motion->yblen;
  }

  schro_opengl_render_quad (x, y, xblen, yblen);
}

static void
schro_opengl_motion_render_ref_block (SchroOpenGLMotion *opengl_motion,
    int i, int x, int y, int u, int v, int ref)
{
  int dx, dy, px, py, hx, hy, rx, ry;
  int weight, addend, divisor;
  int needs_weighting;
  SchroMotion *motion;
  SchroMotionVector *motion_vector;
  SchroChromaFormat chroma_format;
  SchroOpenGLShader *shader = NULL;

  motion = opengl_motion->motion;
  motion_vector = &motion->motion_vectors[v * motion->params->x_num_blocks + u];
  chroma_format = motion->params->video_format->chroma_format;

  SCHRO_ASSERT (motion_vector->using_global == FALSE);

  dx = motion_vector->dx[ref];
  dy = motion_vector->dy[ref];

  if (i > 0) {
    dx >>= SCHRO_CHROMA_FORMAT_H_SHIFT (chroma_format);
    dy >>= SCHRO_CHROMA_FORMAT_V_SHIFT (chroma_format);
  }

  px = (x << motion->mv_precision) + dx;
  py = (y << motion->mv_precision) + dy;
  weight = motion->ref1_weight + motion->ref2_weight;
  addend = 1 << (motion->ref_weight_precision - 1);
  divisor = 1 << motion->ref_weight_precision;
  needs_weighting = weight != divisor;

  #define SETUP_UNIFORMS_FOR_PREC_0_BLOCK(_ref, _index, _x, _y) \
      do { \
        glActiveTextureARB (GL_TEXTURE2_ARB + (_index)); \
        glBindTexture (GL_TEXTURE_RECTANGLE_ARB, \
            opengl_motion->src_canvases[_ref][0]->texture); \
        glActiveTextureARB (GL_TEXTURE0_ARB); \
        glUniform2fARB (shader->offsets[_index], (_x) - x, (_y) - y); \
      } while (0)

  #define SETUP_UNIFORMS_FOR_PREC_1_BLOCK(_ref, _index, _x, _y) \
      do { \
        int sub = (((_y) & 1) << 1) | ((_x) & 1); \
        glActiveTextureARB (GL_TEXTURE2_ARB + (_index)); \
        glBindTexture (GL_TEXTURE_RECTANGLE_ARB, \
            opengl_motion->src_canvases[_ref][sub]->texture); \
        glActiveTextureARB (GL_TEXTURE0_ARB); \
        glUniform2fARB (shader->offsets[_index], ((_x) >> 1) - x, \
            ((_y) >> 1) - y); \
      } while (0)

  switch (motion->mv_precision) {
    case 0: // schro_upsampled_frame_get_block_fast_prec0
      if (needs_weighting) {
        shader = opengl_motion->shader_ref_prec_0_weight;
        //SCHRO_ERROR ("shader_ref_prec_0_weight");
      } else {
        shader = opengl_motion->shader_ref_prec_0;
        //SCHRO_ERROR ("shader_ref_prec_0");
      }

      glUseProgramObjectARB (shader->program);

      SETUP_UNIFORMS_FOR_PREC_0_BLOCK (ref, 0, px, py);
      break;
    case 1: // schro_upsampled_frame_get_block_fast_prec1
      if (needs_weighting) {
        shader = opengl_motion->shader_ref_prec_0_weight;
        //SCHRO_ERROR ("shader_ref_prec_0_weight 1");
      } else {
        shader = opengl_motion->shader_ref_prec_0;
        //SCHRO_ERROR ("shader_ref_prec_0 1");
      }

      glUseProgramObjectARB (shader->program);

      SETUP_UNIFORMS_FOR_PREC_1_BLOCK (ref, 0, px, py);
      break;
    case 2:
      px <<= 1;
      py <<= 1;
      /* fall through */
    case 3: // schro_upsampled_frame_get_block_fast_prec3
      hx = px >> 2;
      hy = py >> 2;
      rx = px & 0x3;
      ry = py & 0x3;

      switch ((ry << 2) | rx) {
        case 0: // schro_upsampled_frame_get_block_fast_prec1
          if (needs_weighting) {
            shader = opengl_motion->shader_ref_prec_0_weight;
            //SCHRO_ERROR ("shader_ref_prec_0_weight 3 0");
          } else {
            shader = opengl_motion->shader_ref_prec_0;
            //SCHRO_ERROR ("shader_ref_prec_0 3 0");
          }

          glUseProgramObjectARB (shader->program);

          SETUP_UNIFORMS_FOR_PREC_1_BLOCK (ref, 0, hx, hy);
          break;
        case 2:
        case 8:
          if (needs_weighting) {
            shader = opengl_motion->shader_ref_prec_3a_weight;
            //SCHRO_ERROR ("shader_ref_prec_3a_weight");
          } else {
            shader = opengl_motion->shader_ref_prec_3a;
            //SCHRO_ERROR ("shader_ref_prec_3a");
          }

          glUseProgramObjectARB (shader->program);

          SETUP_UNIFORMS_FOR_PREC_1_BLOCK (ref, 0, hx, hy);

          if (rx == 0) {
            SETUP_UNIFORMS_FOR_PREC_1_BLOCK (ref, 1, hx, hy + 1);
          } else {
            SETUP_UNIFORMS_FOR_PREC_1_BLOCK (ref, 1, hx + 1, hy);
          }

          break;
        default:
          if (needs_weighting) {
            shader = opengl_motion->shader_ref_prec_3b_weight;
            //SCHRO_ERROR ("shader_ref_prec_3b_weight");
          } else {
            shader = opengl_motion->shader_ref_prec_3b;
            //SCHRO_ERROR ("shader_ref_prec_3b");
          }

          glUseProgramObjectARB (shader->program);

          SETUP_UNIFORMS_FOR_PREC_1_BLOCK (ref, 0, hx,     hy);
          SETUP_UNIFORMS_FOR_PREC_1_BLOCK (ref, 1, hx + 1, hy);
          SETUP_UNIFORMS_FOR_PREC_1_BLOCK (ref, 2, hx,     hy + 1);
          SETUP_UNIFORMS_FOR_PREC_1_BLOCK (ref, 3, hx + 1, hy + 1);

          glUniform4fARB (shader->linear_weights[0], (4 - ry) * (4 - rx),
              (4 - ry) * rx, ry * (4 - rx), ry * rx);
          break;
      }

      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }

  #undef SETUP_UNIFORMS_FOR_PREC_0_BLOCK
  #undef SETUP_UNIFORMS_FOR_PREC_1_BLOCK

  SCHRO_ASSERT (shader != NULL);

  glUniform2fARB (shader->origin, x, y);

  if (needs_weighting) {
    glUniform1fARB (shader->ref_weights[0], weight);
    glUniform1fARB (shader->ref_addend, addend);
    glUniform1fARB (shader->ref_divisor, divisor);
  }

  schro_opengl_render_quad (x, y, motion->xblen, motion->yblen);
}

static void
schro_opengl_motion_render_biref_block (SchroOpenGLMotion *opengl_motion,
    int i, int x, int y, int u, int v)
{
  int dx[2], dy[2], px[2], py[2], hx[2], hy[2], rx[2], ry[2];
  int weights[2], addend, divisor;
  int needs_weighting;
  SchroMotion *motion;
  SchroMotionVector *motion_vector;
  SchroChromaFormat chroma_format;
  SchroOpenGLShader *shader = NULL;

  motion = opengl_motion->motion;
  motion_vector = &motion->motion_vectors[v * motion->params->x_num_blocks + u];
  chroma_format = motion->params->video_format->chroma_format;

  SCHRO_ASSERT (motion_vector->using_global == FALSE);

  dx[0] = motion_vector->dx[0];
  dy[0] = motion_vector->dy[0];
  dx[1] = motion_vector->dx[1];
  dy[1] = motion_vector->dy[1];

  if (i > 0) {
    dx[0] >>= SCHRO_CHROMA_FORMAT_H_SHIFT (chroma_format);
    dy[0] >>= SCHRO_CHROMA_FORMAT_V_SHIFT (chroma_format);
    dx[1] >>= SCHRO_CHROMA_FORMAT_H_SHIFT (chroma_format);
    dy[1] >>= SCHRO_CHROMA_FORMAT_V_SHIFT (chroma_format);
  }

  px[0] = (x << motion->mv_precision) + dx[0];
  py[0] = (y << motion->mv_precision) + dy[0];
  px[1] = (x << motion->mv_precision) + dx[1];
  py[1] = (y << motion->mv_precision) + dy[1];
  weights[0] = motion->ref1_weight;
  weights[1] = motion->ref2_weight;
  addend = 1 << (motion->ref_weight_precision - 1);
  divisor = 1 << motion->ref_weight_precision;
  needs_weighting = weights[0] != 1 || weights[1] != 1 || divisor != 2;

  #define SETUP_UNIFORMS_FOR_PREC_0_BLOCK(_ref, _index, _x, _y) \
      do { \
        glActiveTextureARB (GL_TEXTURE2_ARB + (_index)); \
        glBindTexture (GL_TEXTURE_RECTANGLE_ARB, \
            opengl_motion->src_canvases[_ref][0]->texture); \
        glActiveTextureARB (GL_TEXTURE0_ARB); \
        glUniform2fARB (shader->offsets[_index], (_x) - x, (_y) - y); \
      } while (0)

  #define SETUP_UNIFORMS_FOR_PREC_1_BLOCK(_ref, _index, _x, _y) \
      do { \
        int sub = (((_y) & 1) << 1) | ((_x) & 1); \
        glActiveTextureARB (GL_TEXTURE2_ARB + (_index)); \
        glBindTexture (GL_TEXTURE_RECTANGLE_ARB, \
            opengl_motion->src_canvases[_ref][sub]->texture); \
        glActiveTextureARB (GL_TEXTURE0_ARB); \
        glUniform2fARB (shader->offsets[_index], ((_x) >> 1) - x, \
            ((_y) >> 1) - y); \
      } while (0)

  switch (motion->mv_precision) {
    case 0: // schro_upsampled_frame_get_block_fast_prec0
      if (needs_weighting) {
        shader = opengl_motion->shader_biref_prec_0_0_weight;
      } else {
        shader = opengl_motion->shader_biref_prec_0_0;
      }

      glUseProgramObjectARB (shader->program);

      SETUP_UNIFORMS_FOR_PREC_0_BLOCK (0, 0, px[0], py[0]);
      SETUP_UNIFORMS_FOR_PREC_0_BLOCK (1, 1, px[1], py[1]);
      break;
    case 1: // schro_upsampled_frame_get_block_fast_prec1
      if (needs_weighting) {
        shader = opengl_motion->shader_biref_prec_0_0_weight;
      } else {
        shader = opengl_motion->shader_biref_prec_0_0;
      }

      glUseProgramObjectARB (shader->program);

      SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 0, px[0], py[0]);
      SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 1, px[1], py[1]);
      break;
    case 2:
      px[0] <<= 1;
      py[0] <<= 1;
      px[1] <<= 1;
      py[1] <<= 1;
      /* fall through */
    case 3: // schro_upsampled_frame_get_block_fast_prec3
      hx[0] = px[0] >> 2;
      hy[0] = py[0] >> 2;
      hx[1] = px[1] >> 2;
      hy[1] = py[1] >> 2;
      rx[0] = px[0] & 0x3;
      ry[0] = py[0] & 0x3;
      rx[1] = px[1] & 0x3;
      ry[1] = py[1] & 0x3;

      switch ((ry[0] << 2) | rx[0]) {
        case 0: // schro_upsampled_frame_get_block_fast_prec1
          switch ((ry[1] << 2) | rx[1]) {
            case 0: // schro_upsampled_frame_get_block_fast_prec1
              if (needs_weighting) {
                shader = opengl_motion->shader_biref_prec_0_0_weight;
              } else {
                shader = opengl_motion->shader_biref_prec_0_0;
              }

              glUseProgramObjectARB (shader->program);

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 0, hx[0], hy[0]);

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 1, hx[1], hy[1]);
              break;
            case 2:
            case 8:
              if (needs_weighting) {
                shader = opengl_motion->shader_biref_prec_0_3a_weight;
              } else {
                shader = opengl_motion->shader_biref_prec_0_3a;
              }

              glUseProgramObjectARB (shader->program);

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 0, hx[0], hy[0]);

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 1, hx[1], hy[1]);

              if (rx[1] == 0) {
                SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 2, hx[1], hy[1] + 1);
              } else {
                SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 2, hx[1] + 1, hy[1]);
              }

              break;
            default:
              if (needs_weighting) {
                shader = opengl_motion->shader_biref_prec_0_3b_weight;
              } else {
                shader = opengl_motion->shader_biref_prec_0_3b;
              }

              glUseProgramObjectARB (shader->program);

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 0, hx[0], hy[0]);

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 1, hx[1],     hy[1]);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 2, hx[1] + 1, hy[1]);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 3, hx[1],     hy[1] + 1);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 4, hx[1] + 1, hy[1] + 1);

              glUniform4fARB (shader->linear_weights[0],
                  (4 - ry[1]) * (4 - rx[1]), (4 - ry[1]) * rx[1],
                  ry[1] * (4 - rx[1]), ry[1] * rx[1]);
              break;
          }

          break;
        case 2:
        case 8:
          switch ((ry[1] << 2) | rx[1]) {
            case 0: // schro_upsampled_frame_get_block_fast_prec1
              if (needs_weighting) {
                shader = opengl_motion->shader_biref_prec_3a_0_weight;
              } else {
                shader = opengl_motion->shader_biref_prec_3a_0;
              }

              glUseProgramObjectARB (shader->program);

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 0, hx[0], hy[0]);

              if (rx[0] == 0) {
                SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 1, hx[0], hy[0] + 1);
              } else {
                SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 1, hx[0] + 1, hy[0]);
              }

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 2, hx[1], hy[1]);
              break;
            case 2:
            case 8:
              if (needs_weighting) {
                shader = opengl_motion->shader_biref_prec_3a_3a_weight;
              } else {
                shader = opengl_motion->shader_biref_prec_3a_3a;
              }

              glUseProgramObjectARB (shader->program);

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 0, hx[0], hy[0]);

              if (rx[0] == 0) {
                SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 1, hx[0], hy[0] + 1);
              } else {
                SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 1, hx[0] + 1, hy[0]);
              }

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 2, hx[1], hy[1]);

              if (rx[1] == 0) {
                SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 3, hx[1], hy[1] + 1);
              } else {
                SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 3, hx[1] + 1, hy[1]);
              }

              break;
            default:
              if (needs_weighting) {
                shader = opengl_motion->shader_biref_prec_3a_3b_weight;
              } else {
                shader = opengl_motion->shader_biref_prec_3a_3b;
              }

              glUseProgramObjectARB (shader->program);

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 0, hx[0], hy[0]);

              if (rx[0] == 0) {
                SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 1, hx[0], hy[0] + 1);
              } else {
                SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 1, hx[0] + 1, hy[0]);
              }

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 2, hx[1],     hy[1]);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 3, hx[1] + 1, hy[1]);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 4, hx[1],     hy[1] + 1);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 5, hx[1] + 1, hy[1] + 1);

              glUniform4fARB (shader->linear_weights[0],
                  (4 - ry[1]) * (4 - rx[1]), (4 - ry[1]) * rx[1],
                  ry[1] * (4 - rx[1]), ry[1] * rx[1]);
              break;
          }

          break;
        default:
          switch ((ry[1] << 2) | rx[1]) {
            case 0: // schro_upsampled_frame_get_block_fast_prec1
              if (needs_weighting) {
                shader = opengl_motion->shader_biref_prec_3b_0_weight;
              } else {
                shader = opengl_motion->shader_biref_prec_3b_0;
              }

              glUseProgramObjectARB (shader->program);

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 0, hx[0],     hy[0]);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 1, hx[0] + 1, hy[0]);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 2, hx[0],     hy[0] + 1);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 3, hx[0] + 1, hy[0] + 1);

              glUniform4fARB (shader->linear_weights[0],
                  (4 - ry[0]) * (4 - rx[0]), (4 - ry[0]) * rx[0],
                  ry[0] * (4 - rx[0]), ry[0] * rx[0]);

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 4, hx[1], hy[1]);
              break;
            case 2:
            case 8:
              if (needs_weighting) {
                shader = opengl_motion->shader_biref_prec_3b_3a_weight;
              } else {
                shader = opengl_motion->shader_biref_prec_3b_3a;
              }

              glUseProgramObjectARB (shader->program);

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 0, hx[0],     hy[0]);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 1, hx[0] + 1, hy[0]);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 2, hx[0],     hy[0] + 1);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 3, hx[0] + 1, hy[0] + 1);

              glUniform4fARB (shader->linear_weights[0],
                  (4 - ry[0]) * (4 - rx[0]), (4 - ry[0]) * rx[0],
                  ry[0] * (4 - rx[0]), ry[0] * rx[0]);

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 4, hx[1], hy[1]);

              if (rx[1] == 0) {
                SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 5, hx[1], hy[1] + 1);
              } else {
                SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 5, hx[1] + 1, hy[1]);
              }

              break;
            default:
              if (needs_weighting) {
                shader = opengl_motion->shader_biref_prec_3b_3b_weight;
              } else {
                shader = opengl_motion->shader_biref_prec_3b_3b;
              }

              glUseProgramObjectARB (shader->program);

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 0, hx[0],     hy[0]);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 1, hx[0] + 1, hy[0]);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 2, hx[0],     hy[0] + 1);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (0, 3, hx[0] + 1, hy[0] + 1);

              glUniform4fARB (shader->linear_weights[0],
                  (4 - ry[0]) * (4 - rx[0]), (4 - ry[0]) * rx[0],
                  ry[0] * (4 - rx[0]), ry[0] * rx[0]);

              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 4, hx[1],     hy[1]);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 5, hx[1] + 1, hy[1]);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 6, hx[1],     hy[1] + 1);
              SETUP_UNIFORMS_FOR_PREC_1_BLOCK (1, 7, hx[1] + 1, hy[1] + 1);

              glUniform4fARB (shader->linear_weights[1],
                  (4 - ry[1]) * (4 - rx[1]), (4 - ry[1]) * rx[1],
                  ry[1] * (4 - rx[1]), ry[1] * rx[1]);
              break;
          }

          break;
      }

      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }

  #undef SETUP_UNIFORMS_FOR_PREC_0_BLOCK
  #undef SETUP_UNIFORMS_FOR_PREC_1_BLOCK

  SCHRO_ASSERT (shader != NULL);

  glUniform2fARB (shader->origin, x, y);

  if (needs_weighting) {
    glUniform1fARB (shader->ref_weights[0], weights[0]);
    glUniform1fARB (shader->ref_weights[1], weights[1]);
    glUniform1fARB (shader->ref_addend, addend);
    glUniform1fARB (shader->ref_divisor, divisor);
  }

  schro_opengl_render_quad (x, y, motion->xblen, motion->yblen);
}

static void
schro_opengl_motion_render_block (SchroOpenGLMotion *opengl_motion, int i,
    int x, int y, int u, int v)
{
  SchroMotion *motion;
  SchroMotionVector *motion_vector;

  motion = opengl_motion->motion;
  motion_vector = &motion->motion_vectors[v * motion->params->x_num_blocks + u];

  switch (motion_vector->pred_mode) {
    case 0:
      schro_opengl_motion_render_dc_block (opengl_motion, i, x, y, u, v);
      break;
    case 1:
      schro_opengl_motion_render_ref_block (opengl_motion, i, x, y, u, v, 0);
      break;
    case 2:
      schro_opengl_motion_render_ref_block (opengl_motion, i, x, y, u, v, 1);
      break;
    case 3:
      schro_opengl_motion_render_biref_block (opengl_motion, i, x, y, u, v);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_motion_render (SchroMotion *motion, SchroFrame *dest)
{
  int i, k, u, v;
  int x, y;
  SchroParams *params = motion->params;
  SchroOpenGLCanvas *dest_canvas;
  SchroChromaFormat chroma_format;
  SchroOpenGLShader *shader_copy;
  SchroOpenGLShader *shader_clear;
  SchroOpenGLShader *shader_shift;
  SchroOpenGLMotion opengl_motion;
  SchroOpenGLSpatialWeightGrid *spatial_weight_grid;
  SchroOpenGLSpatialWeightBlock *spatial_weight_block;
  SchroOpenGLSpatialWeightBlock *previous_spatial_weight_block = NULL;

  SCHRO_ASSERT (SCHRO_FRAME_IS_OPENGL (dest));
  SCHRO_ASSERT (SCHRO_FRAME_FORMAT_DEPTH (dest->format)
      == SCHRO_FRAME_FORMAT_DEPTH_S16);

  SCHRO_ASSERT (params->picture_weight_1 < (1 << params->picture_weight_bits));
  SCHRO_ASSERT (params->picture_weight_2 < (1 << params->picture_weight_bits));

  if (params->num_refs == 1) {
    SCHRO_ASSERT (params->picture_weight_2 == 1);
  }

  dest_canvas = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA (dest->components + 0);

  SCHRO_ASSERT (dest_canvas != NULL);

  schro_opengl_lock_context (dest_canvas->opengl);

  shader_copy = schro_opengl_shader_get (dest_canvas->opengl,
      SCHRO_OPENGL_SHADER_COPY_S16);
  shader_clear = schro_opengl_shader_get (dest_canvas->opengl,
      SCHRO_OPENGL_SHADER_OBMC_CLEAR);
  shader_shift = schro_opengl_shader_get (dest_canvas->opengl,
      SCHRO_OPENGL_SHADER_OBMC_SHIFT);

  SCHRO_ASSERT (shader_copy != NULL);
  SCHRO_ASSERT (shader_clear != NULL);
  SCHRO_ASSERT (shader_shift != NULL);

  opengl_motion.motion = motion;

  #define GET_SHADER(_variable, _name) \
      opengl_motion.shader_##_variable = schro_opengl_shader_get \
          (dest_canvas->opengl, SCHRO_OPENGL_SHADER_OBMC_RENDER_##_name); \
      SCHRO_ASSERT (opengl_motion.shader_##_variable != NULL);

  GET_SHADER (dc,                      DC)
  GET_SHADER (ref_prec_0,              REF_PREC_0)
  GET_SHADER (ref_prec_0_weight,       REF_PREC_0_WEIGHT)
  GET_SHADER (ref_prec_3a,             REF_PREC_3a)
  GET_SHADER (ref_prec_3a_weight,      REF_PREC_3a_WEIGHT)
  GET_SHADER (ref_prec_3b,             REF_PREC_3b)
  GET_SHADER (ref_prec_3b_weight,      REF_PREC_3b_WEIGHT)
  GET_SHADER (biref_prec_0_0,          BIREF_PREC_0_0)
  GET_SHADER (biref_prec_0_0_weight,   BIREF_PREC_0_0_WEIGHT)
  GET_SHADER (biref_prec_0_3a,         BIREF_PREC_0_3a)
  GET_SHADER (biref_prec_0_3a_weight,  BIREF_PREC_0_3a_WEIGHT)
  GET_SHADER (biref_prec_0_3b,         BIREF_PREC_0_3b)
  GET_SHADER (biref_prec_0_3b_weight,  BIREF_PREC_0_3b_WEIGHT)
  GET_SHADER (biref_prec_3a_0,         BIREF_PREC_3a_0)
  GET_SHADER (biref_prec_3a_0_weight,  BIREF_PREC_3a_0_WEIGHT)
  GET_SHADER (biref_prec_3a_3a,        BIREF_PREC_3a_3a)
  GET_SHADER (biref_prec_3a_3a_weight, BIREF_PREC_3a_3a_WEIGHT)
  GET_SHADER (biref_prec_3a_3b,        BIREF_PREC_3a_3b)
  GET_SHADER (biref_prec_3a_3b_weight, BIREF_PREC_3a_3b_WEIGHT)
  GET_SHADER (biref_prec_3b_0,         BIREF_PREC_3b_0)
  GET_SHADER (biref_prec_3b_0_weight,  BIREF_PREC_3b_0_WEIGHT)
  GET_SHADER (biref_prec_3b_3a,        BIREF_PREC_3b_3a)
  GET_SHADER (biref_prec_3b_3a_weight, BIREF_PREC_3b_3a_WEIGHT)
  GET_SHADER (biref_prec_3b_3b,        BIREF_PREC_3b_3b)
  GET_SHADER (biref_prec_3b_3b_weight, BIREF_PREC_3b_3b_WEIGHT)

  #undef GET_SHADER

  motion->ref_weight_precision = params->picture_weight_bits;
  motion->ref1_weight = params->picture_weight_1;
  motion->ref2_weight = params->picture_weight_2;
  motion->mv_precision = params->mv_precision;

  chroma_format = params->video_format->chroma_format;

  for (i = 0; i < 3; ++i) {
    dest_canvas = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA (dest->components + i);

    SCHRO_ASSERT (dest_canvas != NULL);

    for (k = 0; k < 4; ++k) {
      if (motion->src1->frames[k]) {
        SCHRO_ASSERT (SCHRO_FRAME_IS_OPENGL (motion->src1->frames[k]));

        opengl_motion.src_canvases[0][k] = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA
            (motion->src1->frames[k]->components + i);

        SCHRO_ASSERT (opengl_motion.src_canvases[0][k] != NULL);
        SCHRO_ASSERT (opengl_motion.src_canvases[0][k]->opengl
            == dest_canvas->opengl);
      } else {
        opengl_motion.src_canvases[0][k] = NULL;
      }
    }

    if (params->num_refs > 1) {
      for (k = 0; k < 4; ++k) {
        if (motion->src2->frames[k]) {
          SCHRO_ASSERT (SCHRO_FRAME_IS_OPENGL (motion->src2->frames[k]));

          opengl_motion.src_canvases[1][k] = SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA
              (motion->src2->frames[k]->components + i);

          SCHRO_ASSERT (opengl_motion.src_canvases[1][k] != NULL);
          SCHRO_ASSERT (opengl_motion.src_canvases[1][k]->opengl
              == dest_canvas->opengl);
        } else {
          opengl_motion.src_canvases[1][k] = NULL;
        }
      }
    }

    if (i == 0) {
      motion->xbsep = params->xbsep_luma;
      motion->ybsep = params->ybsep_luma;
      motion->xblen = params->xblen_luma;
      motion->yblen = params->yblen_luma;
    } else {
      motion->xbsep = params->xbsep_luma
          >> SCHRO_CHROMA_FORMAT_H_SHIFT (chroma_format);
      motion->ybsep = params->ybsep_luma
          >> SCHRO_CHROMA_FORMAT_V_SHIFT (chroma_format);
      motion->xblen = params->xblen_luma
          >> SCHRO_CHROMA_FORMAT_H_SHIFT (chroma_format);
      motion->yblen = params->yblen_luma
          >> SCHRO_CHROMA_FORMAT_V_SHIFT (chroma_format);
    }

    SCHRO_ASSERT (motion->xblen <= 2 * motion->xbsep);
    SCHRO_ASSERT (motion->yblen <= 2 * motion->ybsep);
    SCHRO_ASSERT (motion->xblen >= motion->xbsep);
    SCHRO_ASSERT (motion->yblen >= motion->ybsep);
    SCHRO_ASSERT ((motion->xblen - motion->xbsep) % 2 == 0);
    SCHRO_ASSERT ((motion->yblen - motion->ybsep) % 2 == 0);

    motion->width = dest->components[i].width;
    motion->height = dest->components[i].height;
    motion->xoffset = (motion->xblen - motion->xbsep) / 2;
    motion->yoffset = (motion->yblen - motion->ybsep) / 2;
    motion->max_fast_x = (motion->width - motion->xblen)
        << motion->mv_precision;
    motion->max_fast_y = (motion->height - motion->yblen)
        << motion->mv_precision;

    spatial_weight_grid = schro_opengl_spatial_weight_grid_new
        (dest_canvas->opengl, motion->width, motion->height,
        params->x_num_blocks, params->y_num_blocks, motion->xbsep,
        motion->ybsep, motion->xblen, motion->yblen);

    /* clear */
    schro_opengl_setup_viewport (motion->width, motion->height);

    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT,
        dest_canvas->secondary->framebuffer);

    glUseProgramObjectARB (shader_clear->program);

    schro_opengl_render_quad (0, 0, motion->width, motion->height);

    SCHRO_OPENGL_CHECK_ERROR

    glFlush();

    /* render blocks */
    int passes[4][2] = { { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 } };

    for (k = 0; k < 4; ++k) {
      /* copy */
      /*if (GLEW_EXT_framebuffer_blit) {
        glBindFramebufferEXT (GL_READ_FRAMEBUFFER_EXT,
            dest_canvas->framebuffers[1]);
        glBindFramebufferEXT (GL_DRAW_FRAMEBUFFER_EXT,
            dest_canvas->framebuffers[0]);
        glBlitFramebufferEXT (0, 0, motion->width, motion->height, 0, 0,
            motion->width, motion->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        SCHRO_OPENGL_CHECK_ERROR

        glFlush();
      } else*/ {
        glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, dest_canvas->framebuffer);
        glBindTexture (GL_TEXTURE_RECTANGLE_ARB,
            dest_canvas->secondary->texture);

        glUseProgramObjectARB (shader_copy->program);

        schro_opengl_render_quad (0, 0, motion->width, motion->height);

        SCHRO_OPENGL_CHECK_ERROR

        glFlush();
      }

      /* render */
      glBindFramebufferEXT (GL_FRAMEBUFFER_EXT,
          dest_canvas->secondary->framebuffer);

      glBindTexture (GL_TEXTURE_RECTANGLE_ARB, dest_canvas->texture);

      SCHRO_OPENGL_CHECK_ERROR

      previous_spatial_weight_block = NULL;

      for (v = passes[k][0]; v < params->y_num_blocks; v += 2) {
        y = motion->ybsep * v - motion->yoffset;

        for (u = passes[k][1]; u < params->x_num_blocks; u += 2) {
          x = motion->xbsep * u - motion->xoffset;
          spatial_weight_block
              = spatial_weight_grid->blocks[v * params->x_num_blocks + u];

          if (spatial_weight_block != previous_spatial_weight_block) {
            previous_spatial_weight_block = spatial_weight_block;

            glActiveTextureARB (GL_TEXTURE1_ARB);
            glBindTexture (GL_TEXTURE_RECTANGLE_ARB,
                spatial_weight_block->canvas->texture);
            glActiveTextureARB (GL_TEXTURE0_ARB);
          }

          schro_opengl_motion_render_block (&opengl_motion, i, x, y, u, v);
        }
      }

      SCHRO_OPENGL_CHECK_ERROR

      glFlush();
    }

    /* shift */
    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, dest_canvas->framebuffer);
    glBindTexture (GL_TEXTURE_RECTANGLE_ARB, dest_canvas->secondary->texture);

    glUseProgramObjectARB (shader_shift->program);

    schro_opengl_render_quad (0, 0, motion->width, motion->height);

    SCHRO_OPENGL_CHECK_ERROR

    glFlush();

    schro_opengl_spatial_weight_grid_unref (spatial_weight_grid);
  }

  glUseProgramObjectARB (0);
#if SCHRO_OPENGL_UNBIND_TEXTURES
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);
  glActiveTextureARB (GL_TEXTURE1_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);
  glActiveTextureARB (GL_TEXTURE2_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);
  glActiveTextureARB (GL_TEXTURE3_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);
  glActiveTextureARB (GL_TEXTURE4_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);
  glActiveTextureARB (GL_TEXTURE5_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);
  glActiveTextureARB (GL_TEXTURE6_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);
  glActiveTextureARB (GL_TEXTURE7_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);
  glActiveTextureARB (GL_TEXTURE8_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);
  glActiveTextureARB (GL_TEXTURE9_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);
  glActiveTextureARB (GL_TEXTURE0_ARB);
#endif
  glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);

  /*if (GLEW_EXT_framebuffer_blit) {
    glBindFramebufferEXT (GL_READ_FRAMEBUFFER_EXT, 0);
    glBindFramebufferEXT (GL_DRAW_FRAMEBUFFER_EXT, 0);
  }*/

  schro_opengl_unlock_context (dest_canvas->opengl);
}

SchroOpenGLSpatialWeightBlock *
schro_opengl_spatial_weight_block_new (SchroOpenGL *opengl, int* edges, int xbsep,
    int ybsep, int xblen, int yblen)
{
  int i;
  int xoffset, yoffset;
  SchroOpenGLSpatialWeightPool* pool;
  SchroOpenGLSpatialWeightBlock* block;
  SchroOpenGLShader *shader;

  pool = schro_opengl_get_spatial_weight_pool (opengl);

  /* try to reuse existing block */
  SCHRO_ASSERT (pool->block_count >= 0);
  SCHRO_ASSERT (pool->block_count <= SCHRO_OPENGL_SPATIAL_WEIGHT_POOL_LIMIT);

  for (i = 0; i < pool->block_count; ++i) {
    block = pool->blocks[i];

    if (block->edges[0] == edges[0] && block->edges[1] == edges[1] &&
        block->edges[2] == edges[2] && block->edges[3] == edges[3] &&
        block->xbsep == xbsep && block->ybsep == ybsep &&
        block->xblen == xblen && block->yblen == yblen) {
      ++block->refcount;

      return block;
    }
  }

  /* create new block */
  block = schro_malloc0 (sizeof (SchroOpenGLSpatialWeightBlock));

  block->pool = pool;
  block->refcount = 2; /* 1 ref for caller + 1 ref for pool */
  block->edges[0] = edges[0];
  block->edges[1] = edges[1];
  block->edges[2] = edges[2];
  block->edges[3] = edges[3];
  block->xbsep = xbsep;
  block->ybsep = ybsep;
  block->xblen = xblen;
  block->yblen = yblen;
  block->canvas = schro_opengl_canvas_new (opengl,
      SCHRO_OPENGL_CANVAS_TYPE_SPATIAL_WEIGHT, SCHRO_FRAME_FORMAT_S16_444,
      MAX(xblen, 16), MAX(yblen, 16));

  xoffset = (xblen - xbsep) / 2;
  yoffset = (yblen - ybsep) / 2;

  schro_opengl_lock_context (opengl);

  shader = schro_opengl_shader_get (opengl,
      SCHRO_OPENGL_SHADER_OBMC_SPATIAL_WEIGHT);

  SCHRO_ASSERT (shader != NULL);

  schro_opengl_setup_viewport (xblen, yblen);

  glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, block->canvas->framebuffer);

  glUseProgramObjectARB (shader->program);
  glUniform2fARB (shader->offsets[0], xoffset, yoffset);
  glUniform2fARB (shader->edges[0], edges[0], edges[1]);
  glUniform2fARB (shader->edges[1], edges[2], edges[3]);
  glUniform2fARB (shader->size, xblen, yblen);

  schro_opengl_render_quad (0, 0, xblen, yblen);

  glUseProgramObjectARB (0);

  SCHRO_OPENGL_CHECK_ERROR

  glFlush();

  schro_opengl_unlock_context (opengl);

  /* add new block to pool */
  SCHRO_ASSERT (pool->block_count >= 0);
  SCHRO_ASSERT (pool->block_count < SCHRO_OPENGL_SPATIAL_WEIGHT_POOL_LIMIT);

  pool->blocks[pool->block_count] = block;
  ++pool->block_count;

  return block;
}

void
schro_opengl_spatial_weight_block_unref (SchroOpenGLSpatialWeightBlock *block)
{
  int i;
  SchroOpenGLSpatialWeightPool *pool;

  SCHRO_ASSERT (block->refcount > 0);

  --block->refcount;

  if (block->refcount > 0) {
    return;
  }

  /* remove block from the pool */
  pool = block->pool;

  SCHRO_ASSERT (pool->block_count >= 1);
  SCHRO_ASSERT (pool->block_count <= SCHRO_OPENGL_SPATIAL_WEIGHT_POOL_LIMIT);

  for (i = 0; i < pool->block_count; ++i) {
    if (pool->blocks[i] == block) {
      --pool->block_count;

      /* move the last block in the array to the slot of the removed
         one to maintain the array continuous in memory */
      pool->blocks[i] = pool->blocks[pool->block_count];

      break;
    }
  }

  /* unref canvas */
  schro_opengl_canvas_unref (block->canvas);

  schro_free (block);
}

SchroOpenGLSpatialWeightGrid *
schro_opengl_spatial_weight_grid_new (SchroOpenGL *opengl,
    int width, int height, int x_num_blocks, int y_num_blocks, int xbsep,
    int ybsep, int xblen, int yblen)
{
  int i, u, v;
  int x, y;
  int xoffset, yoffset;
  int edges[4];
  SchroOpenGLSpatialWeightPool* pool;
  SchroOpenGLSpatialWeightGrid* grid;

  pool = schro_opengl_get_spatial_weight_pool (opengl);

  /* try to reuse existing grid */
  SCHRO_ASSERT (pool->grid_count >= 0);
  SCHRO_ASSERT (pool->grid_count <= SCHRO_OPENGL_SPATIAL_WEIGHT_POOL_LIMIT);

  for (i = 0; i < pool->grid_count; ++i) {
    grid = pool->grids[i];

    if (grid->width == width && grid->height == height &&
        grid->x_num_blocks == x_num_blocks &&
        grid->y_num_blocks == y_num_blocks && grid->xbsep == xbsep &&
        grid->ybsep == ybsep && grid->xblen == xblen && grid->yblen == yblen) {
      ++grid->refcount;

      return grid;
    }
  }

  /* create new grid */
  grid = schro_malloc0 (sizeof (SchroOpenGLSpatialWeightGrid));

  grid->pool = pool;
  grid->refcount = 2; /* 1 ref for caller + 1 ref for pool */
  grid->width = width;
  grid->height = height;
  grid->x_num_blocks = x_num_blocks;
  grid->y_num_blocks = y_num_blocks;
  grid->xbsep = xbsep;
  grid->ybsep = ybsep;
  grid->xblen = xblen;
  grid->yblen = yblen;
  grid->blocks = schro_malloc0 (sizeof (SchroOpenGLSpatialWeightBlock*)
      * y_num_blocks * x_num_blocks);

  xoffset = (xblen - xbsep) / 2;
  yoffset = (yblen - ybsep) / 2;

  for (v = 0; v < y_num_blocks; ++v) {
    y = ybsep * v - yoffset;

    for (u = 0; u < x_num_blocks; ++u) {
      x = xbsep * u - xoffset;

      edges[0] = u == 0 ? 1 : 0;                /* left */
      edges[1] = v == 0 ? 1 : 0;                /* top */
      edges[2] = u == x_num_blocks - 1 ? 1 : 0; /* right */
      edges[3] = v == y_num_blocks - 1 ? 1 : 0; /* bottom */

      grid->blocks[v * x_num_blocks + u] = schro_opengl_spatial_weight_block_new
          (opengl, edges, xbsep, ybsep, xblen, yblen);
    }
  }

  /* add new grid to pool */
  SCHRO_ASSERT (pool->grid_count >= 0);
  SCHRO_ASSERT (pool->grid_count < SCHRO_OPENGL_SPATIAL_WEIGHT_POOL_LIMIT);

  pool->grids[pool->grid_count] = grid;
  ++pool->grid_count;

  return grid;
}

void
schro_opengl_spatial_weight_grid_unref (SchroOpenGLSpatialWeightGrid *grid)
{
  int i, u, v;
  SchroOpenGLSpatialWeightPool *pool;

  SCHRO_ASSERT (grid->refcount > 0);

  --grid->refcount;

  if (grid->refcount > 0) {
    return;
  }

  /* remove grid from the pool */
  pool = grid->pool;

  SCHRO_ASSERT (pool->grid_count >= 1);
  SCHRO_ASSERT (pool->grid_count <= SCHRO_OPENGL_SPATIAL_WEIGHT_POOL_LIMIT);

  for (i = 0; i < pool->grid_count; ++i) {
    if (pool->grids[i] == grid) {
      --pool->grid_count;

      /* move the last grid in the array to the slot of the removed
         one to maintain the array continuous in memory */
      pool->grids[i] = pool->grids[pool->grid_count];

      break;
    }
  }

  /* unref blocks */
  for (v = 0; v < grid->y_num_blocks; ++v) {
    for (u = 0; u < grid->x_num_blocks; ++u) {
      schro_opengl_spatial_weight_block_unref
          (grid->blocks[v * grid->x_num_blocks + u]);
    }
  }

  schro_free (grid->blocks);

  schro_free (grid);
}

SchroOpenGLSpatialWeightPool *
schro_opengl_spatial_weight_pool_new (void)
{
  SchroOpenGLSpatialWeightPool *pool;

  pool = schro_malloc0 (sizeof (SchroOpenGLSpatialWeightPool));

  pool->block_count = 0;
  pool->grid_count = 0;

  return pool;
}

void
schro_opengl_spatial_weight_pool_free (SchroOpenGLSpatialWeightPool* pool)
{
  int i;

  #define UNREF(_type) \
      do { \
        for (i = 0; i < pool->_type##_count; ++i) { \
          SCHRO_ASSERT (pool->_type##s[i]->refcount == 1); \
        } \
        while (pool->_type##_count > 0) { \
          schro_opengl_spatial_weight_##_type##_unref (pool->_type##s[0]); \
        } \
        SCHRO_ASSERT (pool->_type##_count == 0); \
      } while (0)

  UNREF (grid);
  UNREF (block);

  #undef UNREF

  schro_free (pool);
}

void
schro_opengl_spatial_weight_pool_squeeze (SchroOpenGLSpatialWeightPool* pool)
{
  int i;
  int done = FALSE;

  #define SQUEEZE(_type) \
      do { \
        done = FALSE; \
        while (!done) { \
          done = TRUE; \
          for (i = 0; i < pool->_type##_count; ++i) { \
            if (pool->_type##s[i]->refcount == 1) { \
              /* item only in pool */ \
              schro_opengl_spatial_weight_##_type##_unref (pool->_type##s[i]); \
              done = FALSE; \
              break; \
            } \
          } \
        } \
      } while (0)

  SQUEEZE (grid);
  SQUEEZE (block);

  #undef SQUEEZE
  #undef SQUEEZE
}

