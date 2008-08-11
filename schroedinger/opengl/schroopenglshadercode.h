/* WARNING! Generated header, do not edit! */

#ifndef __SCHRO_OPENGL_SHADER_CODE_H__
#define __SCHRO_OPENGL_SHADER_CODE_H__

#include <schroedinger/schroutils.h>
#include <schroedinger/opengl/schroopengltypes.h>
#include <GL/glew.h>

SCHRO_BEGIN_DECLS

#define SCHRO_OPENGL_SHADER_COPY_U8 0
#define SCHRO_OPENGL_SHADER_COPY_S16 1
#define SCHRO_OPENGL_SHADER_CONVERT_U8_S16 2
#define SCHRO_OPENGL_SHADER_CONVERT_S16_U8 3
#define SCHRO_OPENGL_SHADER_CONVERT_U8_U8 4
#define SCHRO_OPENGL_SHADER_CONVERT_S16_S16 5
#define SCHRO_OPENGL_SHADER_CONVERT_U8_Y4_YUYV 6
#define SCHRO_OPENGL_SHADER_CONVERT_U8_U2_YUYV 7
#define SCHRO_OPENGL_SHADER_CONVERT_U8_V2_YUYV 8
#define SCHRO_OPENGL_SHADER_CONVERT_U8_Y4_UYVY 9
#define SCHRO_OPENGL_SHADER_CONVERT_U8_U2_UYVY 10
#define SCHRO_OPENGL_SHADER_CONVERT_U8_V2_UYVY 11
#define SCHRO_OPENGL_SHADER_CONVERT_U8_Y4_AYUV 12
#define SCHRO_OPENGL_SHADER_CONVERT_U8_U4_AYUV 13
#define SCHRO_OPENGL_SHADER_CONVERT_U8_V4_AYUV 14
#define SCHRO_OPENGL_SHADER_CONVERT_YUYV_U8_422 15
#define SCHRO_OPENGL_SHADER_CONVERT_UYVY_U8_422 16
#define SCHRO_OPENGL_SHADER_CONVERT_AYUV_U8_444 17
#define SCHRO_OPENGL_SHADER_ADD_S16_U8 18
#define SCHRO_OPENGL_SHADER_ADD_S16_S16 19
#define SCHRO_OPENGL_SHADER_SUBTRACT_S16_U8 20
#define SCHRO_OPENGL_SHADER_SUBTRACT_S16_S16 21
#define SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_9_7_Lp 22
#define SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_9_7_Hp 23
#define SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_LE_GALL_5_3_Lp 24
#define SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_LE_GALL_5_3_Hp 25
#define SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_13_7_Lp 26
#define SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_13_7_Hp 27
#define SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_HAAR_Lp 28
#define SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_HAAR_Hp 29
#define SCHRO_OPENGL_SHADER_IIWT_S16_VERTICAL_DEINTERLEAVE_L 30
#define SCHRO_OPENGL_SHADER_IIWT_S16_VERTICAL_DEINTERLEAVE_H 31
#define SCHRO_OPENGL_SHADER_IIWT_S16_VERTICAL_INTERLEAVE 32
#define SCHRO_OPENGL_SHADER_IIWT_S16_HORIZONTAL_INTERLEAVE 33
#define SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_SHIFT 34
#define SCHRO_OPENGL_SHADER_UPSAMPLE_U8 35
#define SCHRO_OPENGL_SHADER_OBMC_SPATIAL_WEIGHT 36
#define SCHRO_OPENGL_SHADER_OBMC_CLEAR 37
#define SCHRO_OPENGL_SHADER_OBMC_SHIFT 38
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_DC 39
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_0 40
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_0_WEIGHT 41
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a 42
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a_WEIGHT 43
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b 44
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b_WEIGHT 45
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0 46
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0_WEIGHT 47
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a 48
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a_WEIGHT 49
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b 50
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT 51
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0 52
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0_WEIGHT 53
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a 54
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a_WEIGHT 55
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b 56
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT 57
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0 58
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT 59
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a 60
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT 61
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b 62
#define SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT 63

#define SCHRO_OPENGL_SHADER_COUNT 64

struct _SchroOpenGLShaderCode {
  int index;
  const char *name;
  const char *code;
  const char *code_integer;
};

extern SchroOpenGLShaderCode _schro_opengl_shader_code_list[];

struct _SchroOpenGLUniforms {
  GLint ayuv;
  GLint block_length;
  GLint block_offset;
  GLint block_origin;
  GLint dc;
  GLint decrease1;
  GLint decrease2;
  GLint decrease3;
  GLint edge1;
  GLint edge2;
  GLint increase1;
  GLint increase2;
  GLint increase3;
  GLint increase4;
  GLint linear_weight;
  GLint linear_weight_ref1;
  GLint linear_weight_ref2;
  GLint offset;
  GLint offset1;
  GLint offset1_ref1;
  GLint offset1_ref2;
  GLint offset2;
  GLint offset2_ref1;
  GLint offset2_ref2;
  GLint offset3;
  GLint offset3_ref1;
  GLint offset3_ref2;
  GLint offset4;
  GLint offset4_ref1;
  GLint offset4_ref2;
  GLint offset_ref1;
  GLint offset_ref2;
  GLint previous;
  GLint ref_addend;
  GLint ref_divisor;
  GLint ref_weight;
  GLint ref_weight1;
  GLint ref_weight2;
  GLint source;
  GLint source1;
  GLint source2;
  GLint spatial_weight;
  GLint u2;
  GLint u4;
  GLint upsampled;
  GLint upsampled1;
  GLint upsampled1_ref1;
  GLint upsampled1_ref2;
  GLint upsampled2;
  GLint upsampled2_ref1;
  GLint upsampled2_ref2;
  GLint upsampled3;
  GLint upsampled3_ref1;
  GLint upsampled3_ref2;
  GLint upsampled4;
  GLint upsampled4_ref1;
  GLint upsampled4_ref2;
  GLint upsampled_ref1;
  GLint upsampled_ref2;
  GLint uyvy;
  GLint v2;
  GLint v4;
  GLint y4;
  GLint yuyv;
};

void schro_opengl_shader_resolve_uniform_locations (SchroOpenGLShader* shader);

void schro_opengl_shader_bind_ayuv (GLuint texture);
void schro_opengl_shader_bind_previous (GLuint texture);
void schro_opengl_shader_bind_source (GLuint texture);
void schro_opengl_shader_bind_source1 (GLuint texture);
void schro_opengl_shader_bind_source2 (GLuint texture);
void schro_opengl_shader_bind_spatial_weight (GLuint texture);
void schro_opengl_shader_bind_u2 (GLuint texture);
void schro_opengl_shader_bind_u4 (GLuint texture);
void schro_opengl_shader_bind_upsampled (GLuint texture);
void schro_opengl_shader_bind_upsampled1 (GLuint texture);
void schro_opengl_shader_bind_upsampled1_ref1 (GLuint texture);
void schro_opengl_shader_bind_upsampled1_ref2 (GLuint texture);
void schro_opengl_shader_bind_upsampled2 (GLuint texture);
void schro_opengl_shader_bind_upsampled2_ref1 (GLuint texture);
void schro_opengl_shader_bind_upsampled2_ref2 (GLuint texture);
void schro_opengl_shader_bind_upsampled3 (GLuint texture);
void schro_opengl_shader_bind_upsampled3_ref1 (GLuint texture);
void schro_opengl_shader_bind_upsampled3_ref2 (GLuint texture);
void schro_opengl_shader_bind_upsampled4 (GLuint texture);
void schro_opengl_shader_bind_upsampled4_ref1 (GLuint texture);
void schro_opengl_shader_bind_upsampled4_ref2 (GLuint texture);
void schro_opengl_shader_bind_upsampled_ref1 (GLuint texture);
void schro_opengl_shader_bind_upsampled_ref2 (GLuint texture);
void schro_opengl_shader_bind_uyvy (GLuint texture);
void schro_opengl_shader_bind_v2 (GLuint texture);
void schro_opengl_shader_bind_v4 (GLuint texture);
void schro_opengl_shader_bind_y4 (GLuint texture);
void schro_opengl_shader_bind_yuyv (GLuint texture);
void schro_opengl_shader_bind_block_length (SchroOpenGLShader* shader, int16_t v0, int16_t v1);
void schro_opengl_shader_bind_block_offset (SchroOpenGLShader* shader, int16_t v0, int16_t v1);
void schro_opengl_shader_bind_block_origin (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_dc (SchroOpenGLShader* shader, int16_t v0);
void schro_opengl_shader_bind_decrease1 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_decrease2 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_decrease3 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_edge1 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_edge2 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_increase1 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_increase2 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_increase3 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_increase4 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_linear_weight (SchroOpenGLShader* shader, int16_t v0, int16_t v1, int16_t v2, int16_t v3);
void schro_opengl_shader_bind_linear_weight_ref1 (SchroOpenGLShader* shader, int16_t v0, int16_t v1, int16_t v2, int16_t v3);
void schro_opengl_shader_bind_linear_weight_ref2 (SchroOpenGLShader* shader, int16_t v0, int16_t v1, int16_t v2, int16_t v3);
void schro_opengl_shader_bind_offset (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_offset1 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_offset1_ref1 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_offset1_ref2 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_offset2 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_offset2_ref1 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_offset2_ref2 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_offset3 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_offset3_ref1 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_offset3_ref2 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_offset4 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_offset4_ref1 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_offset4_ref2 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_offset_ref1 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_offset_ref2 (SchroOpenGLShader* shader, float v0, float v1);
void schro_opengl_shader_bind_ref_addend (SchroOpenGLShader* shader, int16_t v0);
void schro_opengl_shader_bind_ref_divisor (SchroOpenGLShader* shader, int16_t v0);
void schro_opengl_shader_bind_ref_weight (SchroOpenGLShader* shader, int16_t v0);
void schro_opengl_shader_bind_ref_weight1 (SchroOpenGLShader* shader, int16_t v0);
void schro_opengl_shader_bind_ref_weight2 (SchroOpenGLShader* shader, int16_t v0);

SCHRO_END_DECLS

#endif

