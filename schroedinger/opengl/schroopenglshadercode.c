/* WARNING! Generated code, do not edit! */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <schroedinger/schrodebug.h>
#include <schroedinger/opengl/schroopenglshader.h>
#include <schroedinger/opengl/schroopenglshadercode.h>

#define SHADER_HEADER \
    "#version 110\n" \
    "#extension GL_ARB_draw_buffers : require\n" \
    "#extension GL_ARB_texture_rectangle : require\n"

#define SHADER_HEADER_INTEGER \
    "#version 120\n" \
    "#extension GL_ARB_draw_buffers : require\n" \
    "#extension GL_ARB_texture_rectangle : require\n" \
    "#extension GL_EXT_gpu_shader4 : require\n" \
    "#define uint unsigned int\n"

#define SHADER_READ_U8(_name) \
    "uniform sampler2DRect "_name";\n" \
    "float read_"_name"_u8 (vec2 coordinate) {\n" \
    "  float fp = texture2DRect ("_name", coordinate).r;\n" \
    /* scale from FP range [0..1] to U8 [0..255] range and apply proper
       rounding */ \
    "  return floor (fp * 255.0 + 0.5);\n" \
    "}\n"

#define SHADER_READ_U8_INTEGER(_name) \
    "uniform usampler2DRect "_name";\n" \
    "uint read_"_name"_u8 (vec2 coordinate) {\n" \
    "  return texture2DRect ("_name", coordinate).a;\n" \
    "}\n"

#define SHADER_READ_U8_RAW(_name) \
    "uniform sampler2DRect "_name";\n" \
    "float read_"_name"_u8_raw (vec2 coordinate) {\n" \
    "  return texture2DRect ("_name", coordinate).r;\n" \
    "}\n"

#define SHADER_READ_U8_RAW_INTEGER(_name) \
    "uniform usampler2DRect "_name";\n" \
    "uint read_"_name"_u8_raw (vec2 coordinate) {\n" \
    "  return texture2DRect ("_name", coordinate).a;\n" \
    "}\n"

#define SHADER_READ_VEC4_U8(_name) \
    "uniform sampler2DRect "_name";\n" \
    "vec4 read_"_name"_vec4_u8 (vec2 coordinate) {\n" \
    "  vec4 fp = texture2DRect ("_name", coordinate);\n" \
    /* scale from FP range [0..1] to U8 range [0..255] and apply proper
       rounding */ \
    "  return floor (fp * 255.0 + vec4 (0.5));\n" \
    "}\n"

#define SHADER_READ_VEC4_U8_INTEGER(_name) \
    "uniform usampler2DRect "_name";\n" \
    "uvec4 read_"_name"_vec4_u8 (vec2 coordinate) {\n" \
    "  return texture2DRect ("_name", coordinate);\n" \
    "}\n"

#define SHADER_READ_VEC4_U8_RAW(_name) \
    "uniform sampler2DRect "_name";\n" \
    "vec4 read_"_name"_vec4_u8_raw (vec2 coordinate) {\n" \
    "  return texture2DRect ("_name", coordinate);\n" \
    "}\n"

#define SHADER_READ_VEC4_U8_RAW_INTEGER(_name) \
    "uniform usampler2DRect "_name";\n" \
    "uvec4 read_"_name"_vec4_u8_raw (vec2 coordinate) {\n" \
    "  return texture2DRect ("_name", coordinate);\n" \
    "}\n"

#define SHADER_WRITE_U8 \
    "void write_u8 (float u8) {\n" \
    /* scale from U8 range [0..255] to FP range [0..1] */ \
    "  gl_FragColor = vec4 (u8 / 255.0);\n" \
    "}\n"

#define SHADER_WRITE_U8_INTEGER \
    "varying out uvec4 fragcolor_u8;\n" \
    "void write_u8 (uint u8) {\n" \
    "  fragcolor_u8 = uvec4 (u8);\n" \
    "}\n"

#define SHADER_WRITE_U8_RAW \
    "void write_u8_raw (float u8) {\n" \
    "  gl_FragColor = vec4 (u8);\n" \
    "}\n"

#define SHADER_WRITE_U8_RAW_INTEGER \
    "varying out uvec4 fragcolor_u8;\n" \
    "void write_u8_raw (uint u8) {\n" \
    "  fragcolor_u8 = uvec4 (u8);\n" \
    "}\n"

#define SHADER_WRITE_VEC4_U8 \
    "void write_vec4_u8 (vec4 u8) {\n" \
    /* scale from U8 range [0..255] to FP range [0..1] */ \
    "  gl_FragColor = u8 / 255.0;\n" \
    "}\n"

#define SHADER_WRITE_VEC4_U8_INTEGER \
    "varying out uvec4 fragcolor_u8;\n" \
    "void write_vec4_u8 (uvec4 u8) {\n" \
    "  fragcolor_u8 = u8;\n" \
    "}\n"

#define SHADER_WRITE_VEC4_U8_RAW \
    "void write_vec4_u8_raw (vec4 u8) {\n" \
    "  gl_FragColor = u8;\n" \
    "}\n"

#define SHADER_WRITE_VEC4_U8_RAW_INTEGER \
    "varying out uvec4 fragcolor_u8;\n" \
    "void write_vec4_u8_raw (uvec4 u8) {\n" \
    "  fragcolor_u8 = u8;\n" \
    "}\n"

#define SHADER_READ_S16(_name) \
    "uniform sampler2DRect "_name";\n" \
    "float read_"_name"_s16 (vec2 coordinate) {\n" \
    "  float fp = texture2DRect ("_name", coordinate).r;\n" \
    /* scale and bias from FP range [0..1] to S16 range [-32768..32767] */ \
    "  return floor (fp * 65535.0 + 0.5) - 32768.0;\n" \
    "}\n"

#define SHADER_READ_S16_INTEGER(_name) \
    "uniform isampler2DRect "_name";\n" \
    "int read_"_name"_s16 (vec2 coordinate) {\n" \
    /* bias from U16 range [0..65535] to S16 range [-32768..32767] */ \
    "  return texture2DRect ("_name", coordinate).a - 32768;\n" \
    "}\n"

#define SHADER_READ_S16_RAW(_name) \
    "uniform sampler2DRect "_name";\n" \
    "float read_"_name"_s16_raw (vec2 coordinate) {\n" \
    "  return texture2DRect ("_name", coordinate).r;\n" \
    "}\n"

#define SHADER_READ_S16_RAW_INTEGER(_name) \
    "uniform isampler2DRect "_name";\n" \
    "int read_"_name"_s16_raw (vec2 coordinate) {\n" \
    "  return texture2DRect ("_name", coordinate).a;\n" \
    "}\n"

#define SHADER_READ_VEC4_S16(_name) \
    "uniform sampler2DRect "_name";\n" \
    "vec4 read_"_name"_vec4_s16 (vec2 coordinate) {\n" \
    "  vec4 fp = texture2DRect ("_name", coordinate);\n" \
    /* scale and bias from FP range [0..1] to S16 range [-32768..32767] */ \
    "  return floor (fp * 65535.0 + vec4 (0.5)) - vec4 (32768.0);\n" \
    "}\n"

#define SHADER_READ_VEC4_S16_INTEGER(_name) \
    "uniform isampler2DRect "_name";\n" \
    "ivec4 read_"_name"_vec4_s16 (vec2 coordinate) {\n" \
    /* bias from U16 range [0..65535] to S16 range [-32768..32767] */ \
    "  return texture2DRect ("_name", coordinate) - vec4 (32768.0);\n" \
    "}\n"

#define SHADER_READ_VEC4_S16_RAW(_name) \
    "uniform sampler2DRect "_name";\n" \
    "vec4 read_"_name"_vec4_s16_raw (vec2 coordinate) {\n" \
    "  return texture2DRect ("_name", coordinate);\n" \
    "}\n"

#define SHADER_READ_VEC4_S16_RAW_INTEGER(_name) \
    "uniform isampler2DRect "_name";\n" \
    "ivec4 read_"_name"_vec4_s16_raw (vec2 coordinate) {\n" \
    "  return texture2DRect ("_name", coordinate);\n" \
    "}\n"

#define SHADER_WRITE_S16 \
    "void write_s16 (float s16) {\n" \
    /* bias and scale from S16 range [-32768..32767] to FP range [0..1] */ \
    "  gl_FragColor = vec4 ((s16 + 32768.0) / 65535.0);\n" \
    "}\n"

#define SHADER_WRITE_S16_INTEGER \
    "varying out ivec4 fragcolor_s16;\n" \
    "void write_s16 (int s16) {\n" \
    /* bias from S16 range [-32768..32767] to U16 range [0..65535] */ \
    "  fragcolor_s16 = ivec4 (s16 + 32768);\n" \
    "}\n"

#define SHADER_WRITE_S16_RAW \
    "void write_s16_raw (float s16) {\n" \
    "  gl_FragColor = vec4 (s16);\n" \
    "}\n"

#define SHADER_WRITE_S16_RAW_INTEGER \
    "varying out ivec4 fragcolor_s16;\n" \
    "void write_s16_raw (int s16) {\n" \
    "  fragcolor_s16 = ivec4 (s16);\n" \
    "}\n"

#define SHADER_WRITE_VEC4_S16 \
    "void write_vec4_s16 (vec4 s16) {\n" \
    /* bias and scale from S16 range [-32768..32767] to FP range [0..1] */ \
    "  gl_FragColor = (s16 + vec4 (32768.0)) / 65535.0;\n" \
    "}\n"

#define SHADER_WRITE_VEC4_S16_INTEGER \
    "varying out ivec4 fragcolor_s16;\n" \
    "void write_vec4_s16 (ivec4 s16) {\n" \
    /* bias from S16 range [-32768..32767] to U16 range [0..65535] */ \
    "  fragcolor_s16 = s16 + ivec4 (32768);\n" \
    "}\n"

#define SHADER_WRITE_VEC4_S16_RAW \
    "void write_vec4_s16_raw (vec4 s16) {\n" \
    "  gl_FragColor = s16;\n" \
    "}\n"

#define SHADER_WRITE_VEC4_S16_RAW_INTEGER \
    "varying out ivec4 fragcolor_s16;\n" \
    "void write_vec4_s16_raw (ivec4 s16) {\n" \
    "  fragcolor_s16 = s16;\n" \
    "}\n"

#define SHADER_COPY_U8(_name) \
    "uniform sampler2DRect "_name";\n" \
    "void copy_"_name"_u8 (vec2 coordinate) {\n" \
    "  gl_FragColor = texture2DRect ("_name", coordinate);\n" \
    "}\n"

#define SHADER_COPY_U8_INTEGER(_name) \
    "uniform usampler2DRect "_name";\n" \
    "varying out uvec4 fragcolor_u8;\n" \
    "void copy_"_name"_u8 (vec2 coordinate) {\n" \
    "  fragcolor_u8 = texture2DRect ("_name", coordinate);\n" \
    "}\n"

#define SHADER_COPY_S16(_name) \
    "uniform sampler2DRect "_name";\n" \
    "void copy_"_name"_s16 (vec2 coordinate) {\n" \
    "  gl_FragColor = texture2DRect ("_name", coordinate);\n" \
    "}\n"

#define SHADER_COPY_S16_INTEGER(_name) \
    "uniform isampler2DRect "_name";\n" \
    "varying out ivec4 fragcolor_s16;\n" \
    "void copy_"_name"_s16 (vec2 coordinate) {\n" \
    "  fragcolor_s16 = texture2DRect ("_name", coordinate);\n" \
    "}\n"

#define SHADER_CAST_S16_U8 \
    "float cast_s16_u8 (float u8) {\n" \
    "  return u8;\n" \
    "}\n"

#define SHADER_CAST_S16_U8_INTEGER \
    "int cast_s16_u8 (uint u8) {\n" \
    "  return int (u8);\n" \
    "}\n"

#define SHADER_CAST_U8_S16 \
    "float cast_u8_s16 (float s16) {\n" \
    "  return clamp (s16, 0.0, 255.0);\n" \
    "}\n"

#define SHADER_CAST_U8_S16_INTEGER \
    "uint cast_u8_s16 (int s16) {\n" \
    "  return uint (clamp (s16, 0, 255));\n" \
    "}\n"

#define SHADER_CONVERT_RAW_U8 \
    "float convert_raw_u8 (float u8) {\n" \
    /* scale from U8 range [0..255] to FP range [0..1] */ \
    "  return u8 / 255.0;\n" \
    "}\n"

#define SHADER_CONVERT_RAW_U8_INTEGER \
    "uint convert_raw_u8 (uint u8) {\n" \
    "  return u8;\n" \
    "}\n"

#define SHADER_CONVERT_RAW_S16 \
    "float convert_raw_s16 (float s16) {\n" \
    /* bias and scale from S16 range [-32768..32767] to FP range [0..1] */ \
    "  return (s16 + 32768.0) / 65535.0;\n" \
    "}\n"

#define SHADER_CONVERT_RAW_S16_INTEGER \
    "int convert_raw_s16 (int s16) {\n" \
    /* bias from S16 range [-32768..32767] to U16 range [0..65535] */ \
    "  return s16 + 32768;\n" \
    "}\n"

#define SHADER_DIVIDE_S16 \
    "float divide_s16 (float value, float divisor) {\n" \
    "  return floor (value / divisor);\n" \
    "}\n"

#if 0
#define SHADER_DIVIDE_S16_INTEGER \
    "int divide_s16 (int value, int divisor) {\n" \
    "  return value < 0\n" \
    "      ? (value - (divisor - ((-value) % divisor))) / divisor\n" \
    "      : value / divisor;\n" \
    "}\n"
#else
#define SHADER_DIVIDE_S16_INTEGER \
    "int divide_s16 (int value, int divisor) {\n" \
    "  return int (floor (float (value) / float (divisor)));\n" \
    "}\n"
#endif

#define SHADER_CROSSFOOT2_S16 \
    "float crossfoot2_s16 (vec2 value) {\n" \
    "  return value.x + value.y;\n" \
    "}\n"

#define SHADER_CROSSFOOT2_S16_INTEGER \
    "int crossfoot2_s16 (ivec2 value) {\n" \
    "  return value.x + value.y;\n" \
    "}\n"

#define SHADER_CROSSFOOT4_S16 \
    "float crossfoot4_s16 (vec4 value) {\n" \
    "  return value.x + value.y + value.z + value.w;\n" \
    "}\n"

#define SHADER_CROSSFOOT4_S16_INTEGER \
    "int crossfoot4_s16 (ivec4 value) {\n" \
    "  return value.x + value.y + value.z + value.w;\n" \
    "}\n"

#define SHADER_REF_WEIGHTING_S16 \
    "uniform float ref_weight;\n" /* ref1_weight + ref2_weight */ \
    "uniform float ref_addend;\n" /* 1 << (ref_weight_precision - 1) */ \
    "uniform float ref_divisor;\n" /* 1 << ref_weight_precision */ \
    "float ref_weighting_s16 (float value) {\n" \
    "  return divide_s16 (value * ref_weight + ref_addend, ref_divisor);\n" \
    "}\n"

#define SHADER_REF_WEIGHTING_S16_INTEGER \
    "uniform int ref_weight;\n" /* ref1_weight + ref2_weight */ \
    "uniform int ref_addend;\n" /* 1 << (ref_weight_precision - 1) */ \
    "uniform int ref_divisor;\n" /* 1 << ref_weight_precision */ \
    "int ref_weighting_s16 (int value) {\n" \
    "  return divide_s16 (value * ref_weight + ref_addend, ref_divisor);\n" \
    "}\n"

#define SHADER_BIREF_WEIGHTING_S16 \
    "uniform float ref_weight1;\n" /* ref1_weight  */ \
    "uniform float ref_weight2;\n" /* ref2_weight */ \
    "uniform float ref_addend;\n" /* 1 << (ref_weight_precision - 1) */ \
    "uniform float ref_divisor;\n" /* 1 << ref_weight_precision */ \
    "float biref_weighting_s16 (float value1, float value2) {\n" \
    "  return divide_s16 (value1 * ref_weight1 + value2 * ref_weight2\n" \
    "      + ref_addend, ref_divisor);\n" \
    "}\n"

#define SHADER_BIREF_WEIGHTING_S16_INTEGER \
    "uniform int ref_weight1;\n" /* ref1_weight */ \
    "uniform int ref_weight2;\n" /* ref2_weight */ \
    "uniform int ref_addend;\n" /* 1 << (ref_weight_precision - 1) */ \
    "uniform int ref_divisor;\n" /* 1 << ref_weight_precision */ \
    "int biref_weighting_s16 (int value1, int value2) {\n" \
    "  return divide_s16 (value1 * ref_weight1 + value2 * ref_weight2\n" \
    "      + ref_addend, ref_divisor);\n" \
    "}\n"

SchroOpenGLShaderCode _schro_opengl_shader_code_list[] = {
  { SCHRO_OPENGL_SHADER_COPY_U8,
      "copy/u8",
      SHADER_HEADER
      SHADER_COPY_U8 ("source")
      "void main (void) {\n"
      "  copy_source_u8 (gl_TexCoord[0].xy);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_COPY_U8_INTEGER ("source")
      "void main (void) {\n"
      "  copy_source_u8 (gl_TexCoord[0].xy);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_COPY_S16,
      "copy/s16",
      SHADER_HEADER
      SHADER_COPY_S16 ("source")
      "void main (void) {\n"
      "  copy_source_s16 (gl_TexCoord[0].xy);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_COPY_S16_INTEGER ("source")
      "void main (void) {\n"
      "  copy_source_s16 (gl_TexCoord[0].xy);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_U8_S16,
      "convert/u8_s16",
      SHADER_HEADER
      SHADER_CAST_U8_S16
      SHADER_WRITE_U8
      SHADER_READ_S16 ("source")
      "void main (void) {\n"
      "  write_u8 (cast_u8_s16 (read_source_s16 (gl_TexCoord[0].xy) + 128.0));\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_U8_S16_INTEGER
      SHADER_WRITE_U8_INTEGER
      SHADER_READ_S16_INTEGER ("source")
      "void main (void) {\n"
      "  write_u8 (cast_u8_s16 (read_source_s16 (gl_TexCoord[0].xy) + 128));\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_S16_U8,
      "convert/s16_u8",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_WRITE_S16
      SHADER_READ_U8 ("source")
      "void main (void) {\n"
      "  write_s16 (cast_s16_u8 (read_source_u8 (gl_TexCoord[0].xy)) - 128.0);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_WRITE_S16_INTEGER
      SHADER_READ_U8_INTEGER ("source")
      "void main (void) {\n"
      "  write_s16 (cast_s16_u8 (read_source_u8 (gl_TexCoord[0].xy)) - 128);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_U8_U8,
      "convert/u8_u8",
      SHADER_HEADER
      SHADER_COPY_U8 ("source")
      "void main (void) {\n"
      "  copy_source_u8 (gl_TexCoord[0].xy);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_COPY_U8_INTEGER ("source")
      "void main (void) {\n"
      "  copy_source_u8 (gl_TexCoord[0].xy);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_S16_S16,
      "convert/s16_s16",
      SHADER_HEADER
      SHADER_COPY_S16 ("source")
      "void main (void) {\n"
      "  copy_source_s16 (gl_TexCoord[0].xy);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_COPY_S16_INTEGER ("source")
      "void main (void) {\n"
      "  copy_source_s16 (gl_TexCoord[0].xy);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_U8_Y4_YUYV,
      "convert_u8_yuyv/y4",
      SHADER_HEADER
      SHADER_WRITE_U8_RAW
      SHADER_READ_VEC4_U8_RAW ("yuyv")
      "void main (void) {\n"
      "  float x = floor (gl_TexCoord[0].x) / 2.0;\n"
      "  float y = gl_TexCoord[0].y;\n"
      "  vec2 coordinate = vec2 (floor (x) + 0.5, y);\n"
      "  vec4 yuyv = read_yuyv_vec4_u8_raw (coordinate);\n"
      "  if (fract (x) < 0.25) {\n"
      "    write_u8_raw (yuyv.r);\n"
      "  } else {\n"
      "    write_u8_raw (yuyv.b);\n"
      "  }\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_U8_RAW_INTEGER
      SHADER_READ_VEC4_U8_RAW_INTEGER ("yuyv")
      "void main (void) {\n"
      "  float x = floor (gl_TexCoord[0].x) / 2.0;\n"
      "  float y = gl_TexCoord[0].y;\n"
      "  vec2 coordinate = vec2 (floor (x) + 0.5, y);\n"
      "  uvec4 yuyv = read_yuyv_vec4_u8_raw (coordinate);\n"
      "  if (fract (x) < 0.25) {\n"
      "    write_u8_raw (yuyv.r);\n"
      "  } else {\n"
      "    write_u8_raw (yuyv.b);\n"
      "  }\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_U8_U2_YUYV,
      "convert_u8_yuyv/u2",
      SHADER_HEADER
      SHADER_WRITE_U8_RAW
      SHADER_READ_VEC4_U8_RAW ("yuyv")
      "void main (void) {\n"
      "  vec4 yuyv = read_yuyv_vec4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_u8_raw (yuyv.g);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_U8_RAW_INTEGER
      SHADER_READ_VEC4_U8_RAW_INTEGER ("yuyv")
      "void main (void) {\n"
      "  uvec4 yuyv = read_yuyv_vec4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_u8_raw (yuyv.g);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_U8_V2_YUYV,
      "convert_u8_yuyv/v2",
      SHADER_HEADER
      SHADER_WRITE_U8_RAW
      SHADER_READ_VEC4_U8_RAW ("yuyv")
      "void main (void) {\n"
      "  vec4 yuyv = read_yuyv_vec4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_u8_raw (yuyv.a);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_U8_RAW_INTEGER
      SHADER_READ_VEC4_U8_RAW_INTEGER ("yuyv")
      "void main (void) {\n"
      "  uvec4 yuyv = read_yuyv_vec4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_u8_raw (yuyv.a);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_U8_Y4_UYVY,
      "convert_u8_uyvy/y4",
      SHADER_HEADER
      SHADER_WRITE_U8_RAW
      SHADER_READ_VEC4_U8_RAW ("uyvy")
      "void main (void) {\n"
      "  float x = floor (gl_TexCoord[0].x) / 2.0;\n"
      "  float y = gl_TexCoord[0].y;\n"
      "  vec2 coordinate = vec2 (floor (x) + 0.5, y);\n"
      "  vec4 uyvy = read_uyvy_vec4_u8_raw (coordinate);\n"
      "  if (fract (x) < 0.25) {\n"
      "    write_u8_raw (uyvy.g);\n"
      "  } else {\n"
      "    write_u8_raw (uyvy.a);\n"
      "  }\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_U8_RAW_INTEGER
      SHADER_READ_VEC4_U8_RAW_INTEGER ("uyvy")
      "void main (void) {\n"
      "  float x = floor (gl_TexCoord[0].x) / 2.0;\n"
      "  float y = gl_TexCoord[0].y;\n"
      "  vec2 coordinate = vec2 (floor (x) + 0.5, y);\n"
      "  uvec4 uyvy = read_uyvy_vec4_u8_raw (coordinate);\n"
      "  if (fract (x) < 0.25) {\n"
      "    write_u8_raw (uyvy.g);\n"
      "  } else {\n"
      "    write_u8_raw (uyvy.a);\n"
      "  }\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_U8_U2_UYVY,
      "convert_u8_uyvy/u2",
      SHADER_HEADER
      SHADER_WRITE_U8_RAW
      SHADER_READ_VEC4_U8_RAW ("uyvy")
      "void main (void) {\n"
      "  vec4 uyvy = read_uyvy_vec4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_u8_raw (uyvy.r);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_U8_RAW_INTEGER
      SHADER_READ_VEC4_U8_RAW_INTEGER ("uyvy")
      "void main (void) {\n"
      "  uvec4 uyvy = read_uyvy_vec4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_u8_raw (uyvy.r);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_U8_V2_UYVY,
      "convert_u8_uyvy/v2",
      SHADER_HEADER
      SHADER_WRITE_U8_RAW
      SHADER_READ_VEC4_U8_RAW ("uyvy")
      "void main (void) {\n"
      "  vec4 uyvy = read_uyvy_vec4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_u8_raw (uyvy.b);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_U8_RAW_INTEGER
      SHADER_READ_VEC4_U8_RAW_INTEGER ("uyvy")
      "void main (void) {\n"
      "  uvec4 uyvy = read_uyvy_vec4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_u8_raw (uyvy.b);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_U8_Y4_AYUV,
      "convert_u8_ayuv/y4",
      SHADER_HEADER
      SHADER_WRITE_U8_RAW
      SHADER_READ_VEC4_U8_RAW ("ayuv")
      "void main (void) {\n"
      "  vec4 ayuv = read_ayuv_vec4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_u8_raw (ayuv.g);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_U8_RAW_INTEGER
      SHADER_READ_VEC4_U8_RAW_INTEGER ("ayuv")
      "void main (void) {\n"
      "  uvec4 ayuv = read_ayuv_vec4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_u8_raw (ayuv.g);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_U8_U4_AYUV,
      "convert_u8_ayuv/u4",
      SHADER_HEADER
      SHADER_WRITE_U8_RAW
      SHADER_READ_VEC4_U8_RAW ("ayuv")
      "void main (void) {\n"
      "  vec4 ayuv = read_ayuv_vec4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_u8_raw (ayuv.b);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_U8_RAW_INTEGER
      SHADER_READ_VEC4_U8_RAW_INTEGER ("ayuv")
      "void main (void) {\n"
      "  uvec4 ayuv = read_ayuv_vec4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_u8_raw (ayuv.b);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_U8_V4_AYUV,
      "convert_u8_ayuv/v4",
      SHADER_HEADER
      SHADER_WRITE_U8_RAW
      SHADER_READ_VEC4_U8_RAW ("ayuv")
      "void main (void) {\n"
      "  vec4 ayuv = read_ayuv_vec4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_u8_raw (ayuv.a);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_U8_RAW_INTEGER
      SHADER_READ_VEC4_U8_RAW_INTEGER ("ayuv")
      "void main (void) {\n"
      "  uvec4 ayuv = read_ayuv_vec4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_u8_raw (ayuv.a);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_YUYV_U8_422,
      "convert_yuyv_u8_422/normal",
      SHADER_HEADER
      SHADER_WRITE_VEC4_U8_RAW
      SHADER_READ_U8_RAW ("u2")
      SHADER_READ_U8_RAW ("v2")
      SHADER_READ_U8_RAW ("y4")
      "void main (void) {\n"
      "  float x = floor (gl_TexCoord[0].x) * 2.0;\n"
      "  float y = gl_TexCoord[0].y;\n"
      "  vec2 coordinate1 = vec2 (x + 0.5, y);\n"
      "  vec2 coordinate2 = vec2 (x + 1.5, y);\n"
      "  vec4 yuyv;\n"
      "  yuyv.r = read_y4_u8_raw (coordinate1);\n"
      "  yuyv.g = read_u2_u8_raw (gl_TexCoord[0].xy);\n"
      "  yuyv.b = read_y4_u8_raw (coordinate2);\n"
      "  yuyv.a = read_v2_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_vec4_u8_raw (yuyv);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_VEC4_U8_RAW_INTEGER
      SHADER_READ_U8_RAW_INTEGER ("u2")
      SHADER_READ_U8_RAW_INTEGER ("v2")
      SHADER_READ_U8_RAW_INTEGER ("y4")
      "void main (void) {\n"
      "  float x = floor (gl_TexCoord[0].x) * 2.0;\n"
      "  float y = gl_TexCoord[0].y;\n"
      "  vec2 coordinate1 = vec2 (x + 0.5, y);\n"
      "  vec2 coordinate2 = vec2 (x + 1.5, y);\n"
      "  uvec4 yuyv;\n"
      "  yuyv.r = read_y4_u8_raw (coordinate1);\n"
      "  yuyv.g = read_u2_u8_raw (gl_TexCoord[0].xy);\n"
      "  yuyv.b = read_y4_u8_raw (coordinate2);\n"
      "  yuyv.a = read_v2_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_vec4_u8_raw (yuyv);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_UYVY_U8_422,
      "convert_uyvy_u8_422/normal",
      SHADER_HEADER
      SHADER_WRITE_VEC4_U8_RAW
      SHADER_READ_U8_RAW ("u2")
      SHADER_READ_U8_RAW ("v2")
      SHADER_READ_U8_RAW ("y4")
      "void main (void) {\n"
      "  float x = floor (gl_TexCoord[0].x) * 2.0;\n"
      "  float y = gl_TexCoord[0].y;\n"
      "  vec2 coordinate1 = vec2 (x + 0.5, y);\n"
      "  vec2 coordinate2 = vec2 (x + 1.5, y);\n"
      "  vec4 uyvy;\n"
      "  uyvy.r = read_u2_u8_raw (gl_TexCoord[0].xy);\n"
      "  uyvy.g = read_y4_u8_raw (coordinate1);\n"
      "  uyvy.b = read_v2_u8_raw (gl_TexCoord[0].xy);\n"
      "  uyvy.a = read_y4_u8_raw (coordinate2);\n"
      "  write_vec4_u8_raw (uyvy);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_VEC4_U8_RAW_INTEGER
      SHADER_READ_U8_RAW_INTEGER ("u2")
      SHADER_READ_U8_RAW_INTEGER ("v2")
      SHADER_READ_U8_RAW_INTEGER ("y4")
      "void main (void) {\n"
      "  float x = floor (gl_TexCoord[0].x) * 2.0;\n"
      "  float y = gl_TexCoord[0].y;\n"
      "  vec2 coordinate1 = vec2 (x + 0.5, y);\n"
      "  vec2 coordinate2 = vec2 (x + 1.5, y);\n"
      "  uvec4 uyvy;\n"
      "  uyvy.r = read_u2_u8_raw (gl_TexCoord[0].xy);\n"
      "  uyvy.g = read_y4_u8_raw (coordinate1);\n"
      "  uyvy.b = read_v2_u8_raw (gl_TexCoord[0].xy);\n"
      "  uyvy.a = read_y4_u8_raw (coordinate2);\n"
      "  write_vec4_u8_raw (uyvy);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_CONVERT_AYUV_U8_444,
      "convert_ayuv_u8_444/normal",
      SHADER_HEADER
      SHADER_CONVERT_RAW_U8
      SHADER_WRITE_VEC4_U8_RAW
      SHADER_READ_U8_RAW ("u4")
      SHADER_READ_U8_RAW ("v4")
      SHADER_READ_U8_RAW ("y4")
      "void main (void) {\n"
      "  vec4 ayuv;\n"
      "  ayuv.r = convert_raw_u8 (float (255.0));\n"
      "  ayuv.g = read_y4_u8_raw (gl_TexCoord[0].xy);\n"
      "  ayuv.b = read_u4_u8_raw (gl_TexCoord[0].xy);\n"
      "  ayuv.a = read_v4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_vec4_u8_raw (ayuv);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CONVERT_RAW_U8_INTEGER
      SHADER_WRITE_VEC4_U8_RAW_INTEGER
      SHADER_READ_U8_RAW_INTEGER ("u4")
      SHADER_READ_U8_RAW_INTEGER ("v4")
      SHADER_READ_U8_RAW_INTEGER ("y4")
      "void main (void) {\n"
      "  uvec4 ayuv;\n"
      "  ayuv.r = convert_raw_u8 (uint (255));\n"
      "  ayuv.g = read_y4_u8_raw (gl_TexCoord[0].xy);\n"
      "  ayuv.b = read_u4_u8_raw (gl_TexCoord[0].xy);\n"
      "  ayuv.a = read_v4_u8_raw (gl_TexCoord[0].xy);\n"
      "  write_vec4_u8_raw (ayuv);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_ADD_S16_U8,
      "add_s16_u8/normal",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      SHADER_READ_S16 ("source1")
      SHADER_READ_U8 ("source2")
      "void main (void) {\n"
      "  write_s16 (read_source1_s16 (XY) + cast_s16_u8 (read_source2_u8 (XY)));\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      SHADER_READ_S16_INTEGER ("source1")
      SHADER_READ_U8_INTEGER ("source2")
      "void main (void) {\n"
      "  write_s16 (read_source1_s16 (XY) + cast_s16_u8 (read_source2_u8 (XY)));\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_ADD_S16_S16,
      "add_s16_s16/normal",
      SHADER_HEADER
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      SHADER_READ_S16 ("source1")
      SHADER_READ_S16 ("source2")
      "void main (void) {\n"
      "  write_s16 (read_source1_s16 (XY) + read_source2_s16 (XY));\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      SHADER_READ_S16_INTEGER ("source1")
      SHADER_READ_S16_INTEGER ("source2")
      "void main (void) {\n"
      "  write_s16 (read_source1_s16 (XY) + read_source2_s16 (XY));\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_SUBTRACT_S16_U8,
      "subtract_s16_u8/normal",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      SHADER_READ_S16 ("source1")
      SHADER_READ_U8 ("source2")
      "void main (void) {\n"
      "  write_s16 (read_source1_s16 (XY) - cast_s16_u8 (read_source2_u8 (XY)));\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      SHADER_READ_S16_INTEGER ("source1")
      SHADER_READ_U8_INTEGER ("source2")
      "void main (void) {\n"
      "  write_s16 (read_source1_s16 (XY) - cast_s16_u8 (read_source2_u8 (XY)));\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_SUBTRACT_S16_S16,
      "subtract_s16_s16/normal",
      SHADER_HEADER
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      SHADER_READ_S16 ("source1")
      SHADER_READ_S16 ("source2")
      "void main (void) {\n"
      "  write_s16 (read_source1_s16 (XY) - read_source2_s16 (XY));\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      SHADER_READ_S16_INTEGER ("source1")
      SHADER_READ_S16_INTEGER ("source2")
      "void main (void) {\n"
      "  write_s16 (read_source1_s16 (XY) - read_source2_s16 (XY));\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_9_7_Lp,
      "iiwt_s16_filter_deslauriers_dubuc_9_7_lp/normal",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 decrease1;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16 ("source")
      "float filter (float h1m, float h0) {\n"
      "  return divide_s16 (h1m + h0 + 2.0, 4.0);\n"
      "}\n"
      "void main (void) {\n"
      "  float l0  = read_source_s16 (XY);\n"
      "  float h1m = read_source_s16 (XY + offset - decrease1);\n"
      "  float h0  = read_source_s16 (XY + offset);\n"
      "  write_s16 (l0 - filter (h1m, h0));\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 decrease1;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16_INTEGER ("source")
      "int filter (int h1m, int h0) {\n"
      "  return divide_s16 (h1m + h0 + 2, 4);\n"
      "}\n"
      "void main (void) {\n"
      "  int l0  = read_source_s16 (XY);\n"
      "  int h1m = read_source_s16 (XY + offset - decrease1);\n"
      "  int h0  = read_source_s16 (XY + offset);\n"
      "  write_s16 (l0 - filter (h1m, h0));\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_9_7_Hp,
      "iiwt_s16_filter_deslauriers_dubuc_9_7_hp/normal",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 decrease1;\n"
      "uniform vec2 increase1;\n"
      "uniform vec2 increase2;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16 ("source")
      "float filter (float l1m, float l0, float l1p, float l2p) {\n"
      "  return divide_s16 (-l1m + 9.0 * (l0 + l1p) - l2p + 8.0, 16.0);\n"
      "}\n"
      "void main (void) {\n"
      "  float l1m = read_source_s16 (XY - offset - decrease1);\n"
      "  float l0  = read_source_s16 (XY - offset);\n"
      "  float l1p = read_source_s16 (XY - offset + increase1);\n"
      "  float l2p = read_source_s16 (XY - offset + increase2);\n"
      "  float h0  = read_source_s16 (XY);\n"
      "  write_s16 (h0 + filter (l1m, l0, l1p, l2p));\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 decrease1;\n"
      "uniform vec2 increase1;\n"
      "uniform vec2 increase2;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16_INTEGER ("source")
      "int filter (int l1m, int l0, int l1p, int l2p) {\n"
      "  return divide_s16 (-l1m + 9 * (l0 + l1p) - l2p + 8, 16);\n"
      "}\n"
      "void main (void) {\n"
      "  int l1m = read_source_s16 (XY - offset - decrease1);\n"
      "  int l0  = read_source_s16 (XY - offset);\n"
      "  int l1p = read_source_s16 (XY - offset + increase1);\n"
      "  int l2p = read_source_s16 (XY - offset + increase2);\n"
      "  int h0  = read_source_s16 (XY);\n"
      "  write_s16 (h0 + filter (l1m, l0, l1p, l2p));\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_LE_GALL_5_3_Lp,
      "iiwt_s16_filter_le_gall_5_3_lp/normal",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 decrease1;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16 ("source")
      "float filter (float h1m, float h0) {\n"
      "  return divide_s16 (h1m + h0 + 2.0, 4.0);\n"
      "}\n"
      "void main (void) {\n"
      "  float l0  = read_source_s16 (XY);\n"
      "  float h1m = read_source_s16 (XY + offset - decrease1);\n"
      "  float h0  = read_source_s16 (XY + offset);\n"
      "  write_s16 (l0 - filter (h1m, h0));\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 decrease1;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16_INTEGER ("source")
      "int filter (int h1m, int h0) {\n"
      "  return divide_s16 (h1m + h0 + 2, 4);\n"
      "}\n"
      "void main (void) {\n"
      "  int l0  = read_source_s16 (XY);\n"
      "  int h1m = read_source_s16 (XY + offset - decrease1);\n"
      "  int h0  = read_source_s16 (XY + offset);\n"
      "  write_s16 (l0 - filter (h1m, h0));\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_LE_GALL_5_3_Hp,
      "iiwt_s16_filter_le_gall_5_3_hp/normal",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 increase1;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16 ("source")
      "float filter (float l0, float l1p) {\n"
      "  return divide_s16 (l0 + l1p + 1.0, 2.0);\n"
      "}\n"
      "void main (void) {\n"
      "  float l0  = read_source_s16 (XY - offset);\n"
      "  float l1p = read_source_s16 (XY - offset + increase1);\n"
      "  float h0  = read_source_s16 (XY);\n"
      "  write_s16 (h0 + filter (l0, l1p));\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 increase1;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16_INTEGER ("source")
      "int filter (int l0, int l1p) {\n"
      "  return divide_s16 (l0 + l1p + 1, 2);\n"
      "}\n"
      "void main (void) {\n"
      "  int l0  = read_source_s16 (XY - offset);\n"
      "  int l1p = read_source_s16 (XY - offset + increase1);\n"
      "  int h0  = read_source_s16 (XY);\n"
      "  write_s16 (h0 + filter (l0, l1p));\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_13_7_Lp,
      "iiwt_s16_filter_deslauriers_dubuc_13_7_lp/normal",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 decrease1;\n"
      "uniform vec2 decrease2;\n"
      "uniform vec2 increase1;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16 ("source")
      "float filter (float h2m, float h1m, float h0, float h1p) {\n"
      "  return divide_s16 (-h2m + 9.0 * (h1m + h0) - h1p + 16.0, 32.0);\n"
      "}\n"
      "void main (void) {\n"
      "  float l0  = read_source_s16 (XY);\n"
      "  float h2m = read_source_s16 (XY + offset - decrease2);\n"
      "  float h1m = read_source_s16 (XY + offset - decrease1);\n"
      "  float h0  = read_source_s16 (XY + offset);\n"
      "  float h1p = read_source_s16 (XY + offset + increase1);\n"
      "  write_s16 (l0 - filter (h2m, h1m, h0, h1p));\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 decrease1;\n"
      "uniform vec2 decrease2;\n"
      "uniform vec2 increase1;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16_INTEGER ("source")
      "int filter (int h2m, int h1m, int h0, int h1p) {\n"
      "  return divide_s16 (-h2m + 9 * (h1m + h0) - h1p + 16, 32);\n"
      "}\n"
      "void main (void) {\n"
      "  int l0  = read_source_s16 (XY);\n"
      "  int h2m = read_source_s16 (XY + offset - decrease2);\n"
      "  int h1m = read_source_s16 (XY + offset - decrease1);\n"
      "  int h0  = read_source_s16 (XY + offset);\n"
      "  int h1p = read_source_s16 (XY + offset + increase1);\n"
      "  write_s16 (l0 - filter (h2m, h1m, h0, h1p));\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_13_7_Hp,
      "iiwt_s16_filter_deslauriers_dubuc_13_7_hp/normal",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 decrease1;\n"
      "uniform vec2 increase1;\n"
      "uniform vec2 increase2;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16 ("source")
      "float filter (float l1m, float l0, float l1p, float l2p) {\n"
      "  return divide_s16 (-l1m + 9.0 * (l0 + l1p) - l2p + 8.0, 16.0);\n"
      "}\n"
      "void main (void) {\n"
      "  float l1m = read_source_s16 (XY - offset - decrease1);\n"
      "  float l0  = read_source_s16 (XY - offset);\n"
      "  float l1p = read_source_s16 (XY - offset + increase1);\n"
      "  float l2p = read_source_s16 (XY - offset + increase2);\n"
      "  float h0  = read_source_s16 (XY);\n"
      "  write_s16 (h0 + filter (l1m, l0, l1p, l2p));\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 decrease1;\n"
      "uniform vec2 increase1;\n"
      "uniform vec2 increase2;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16_INTEGER ("source")
      "int filter (int l1m, int l0, int l1p, int l2p) {\n"
      "  return divide_s16 (-l1m + 9 * (l0 + l1p) - l2p + 8, 16);\n"
      "}\n"
      "void main (void) {\n"
      "  int l1m = read_source_s16 (XY - offset - decrease1);\n"
      "  int l0  = read_source_s16 (XY - offset);\n"
      "  int l1p = read_source_s16 (XY - offset + increase1);\n"
      "  int l2p = read_source_s16 (XY - offset + increase2);\n"
      "  int h0  = read_source_s16 (XY);\n"
      "  write_s16 (h0 + filter (l1m, l0, l1p, l2p));\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_HAAR_Lp,
      "iiwt_s16_filter_haar_lp/normal",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "uniform vec2 offset;\n"
      SHADER_READ_S16 ("source")
      "float filter (float h0) {\n"
      "  return divide_s16 (h0 + 1.0, 2.0);\n"
      "}\n"
      "void main (void) {\n"
      "  float l0 = read_source_s16 (gl_TexCoord[0].xy);\n"
      "  float h0 = read_source_s16 (gl_TexCoord[0].xy + offset);\n"
      "  write_s16 (l0 - filter (h0));\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "uniform vec2 offset;\n"
      SHADER_READ_S16_INTEGER ("source")
      "int filter (int h0) {\n"
      "  return divide_s16 (h0 + 1, 2);\n"
      "}\n"
      "void main (void) {\n"
      "  int l0 = read_source_s16 (gl_TexCoord[0].xy);\n"
      "  int h0 = read_source_s16 (gl_TexCoord[0].xy + offset);\n"
      "  write_s16 (l0 - filter (h0));\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_HAAR_Hp,
      "iiwt_s16_filter_haar_hp/normal",
      SHADER_HEADER
      SHADER_WRITE_S16
      "uniform vec2 offset;\n"
      SHADER_READ_S16 ("source")
      "void main (void) {\n"
      "  float l0 = read_source_s16 (gl_TexCoord[0].xy - offset);\n"
      "  float h0 = read_source_s16 (gl_TexCoord[0].xy);\n"
      "  write_s16 (h0 + l0);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_S16_INTEGER
      "uniform vec2 offset;\n"
      SHADER_READ_S16_INTEGER ("source")
      "void main (void) {\n"
      "  int l0 = read_source_s16 (gl_TexCoord[0].xy - offset);\n"
      "  int h0 = read_source_s16 (gl_TexCoord[0].xy);\n"
      "  write_s16 (h0 + l0);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_IIWT_S16_VERTICAL_DEINTERLEAVE_L,
      "iiwt_s16_vertical_deinterleave/l",
      SHADER_HEADER
      SHADER_COPY_S16 ("source")
      "void main (void) {\n"
      "  float x = gl_TexCoord[0].x;\n"
      "  float y = floor (gl_TexCoord[0].y);\n"
      "  vec2 coordinate = vec2 (x, y * 2.0 + 0.5);\n"
      "  copy_source_s16 (coordinate);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_COPY_S16_INTEGER ("source")
      "void main (void) {\n"
      "  float x = gl_TexCoord[0].x;\n"
      "  float y = floor (gl_TexCoord[0].y);\n"
      "  vec2 coordinate = vec2 (x, y * 2.0 + 0.5);\n"
      "  copy_source_s16 (coordinate);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_IIWT_S16_VERTICAL_DEINTERLEAVE_H,
      "iiwt_s16_vertical_deinterleave/h",
      SHADER_HEADER
      "uniform vec2 offset;\n"
      SHADER_COPY_S16 ("source")
      "void main (void) {\n"
      "  float x = gl_TexCoord[0].x;\n"
      "  float y = floor (gl_TexCoord[0].y) - offset.y;\n"
      "  vec2 coordinate = vec2 (x, y * 2.0 + 1.5);\n"
      "  copy_source_s16 (coordinate);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      "uniform vec2 offset;\n"
      SHADER_COPY_S16_INTEGER ("source")
      "void main (void) {\n"
      "  float x = gl_TexCoord[0].x;\n"
      "  float y = floor (gl_TexCoord[0].y) - offset.y;\n"
      "  vec2 coordinate = vec2 (x, y * 2.0 + 1.5);\n"
      "  copy_source_s16 (coordinate);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_IIWT_S16_VERTICAL_INTERLEAVE,
      "iiwt_s16_interleave/vertical",
      SHADER_HEADER
      "uniform vec2 offset;\n"
      SHADER_COPY_S16 ("source")
      "void main (void) {\n"
      "  float x = gl_TexCoord[0].x;\n"
      "  float y = floor (gl_TexCoord[0].y);\n"
      "  if (mod (y, 2.0) < 0.5) {\n"
      "    y = floor (y / 2.0);\n"
      "  } else {\n"
      "    y = floor (y / 2.0) + offset.y;\n"
      "  }\n"
      "  vec2 coordinate = vec2 (x, y + 0.5);\n"
      "  copy_source_s16 (coordinate);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      "uniform vec2 offset;\n"
      SHADER_COPY_S16_INTEGER ("source")
      "void main (void) {\n"
      "  float x = gl_TexCoord[0].x;\n"
      "  float y = floor (gl_TexCoord[0].y);\n"
      "  if (mod (y, 2.0) < 0.5) {\n"
      "    y = floor (y / 2.0);\n"
      "  } else {\n"
      "    y = floor (y / 2.0) + offset.y;\n"
      "  }\n"
      "  vec2 coordinate = vec2 (x, y + 0.5);\n"
      "  copy_source_s16 (coordinate);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_IIWT_S16_HORIZONTAL_INTERLEAVE,
      "iiwt_s16_interleave/horizontal",
      SHADER_HEADER
      "uniform vec2 offset;\n"
      SHADER_COPY_S16 ("source")
      "void main (void) {\n"
      "  float x = floor (gl_TexCoord[0].x);\n"
      "  float y = gl_TexCoord[0].y;\n"
      "  if (mod (x, 2.0) < 0.5) {\n"
      "    x = floor (x / 2.0);\n"
      "  } else {\n"
      "    x = floor (x / 2.0) + offset.x;\n"
      "  }\n"
      "  vec2 coordinate = vec2 (x + 0.5, y);\n"
      "  copy_source_s16 (coordinate);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      "uniform vec2 offset;\n"
      SHADER_COPY_S16_INTEGER ("source")
      "void main (void) {\n"
      "  float x = floor (gl_TexCoord[0].x);\n"
      "  float y = gl_TexCoord[0].y;\n"
      "  if (mod (x, 2.0) < 0.5) {\n"
      "    x = floor (x / 2.0);\n"
      "  } else {\n"
      "    x = floor (x / 2.0) + offset.x;\n"
      "  }\n"
      "  vec2 coordinate = vec2 (x + 0.5, y);\n"
      "  copy_source_s16 (coordinate);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_SHIFT,
      "iiwt_s16_filter_shift/normal",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      SHADER_READ_S16 ("source")
      "void main (void) {\n"
      "  write_s16 (divide_s16 (read_source_s16 (gl_TexCoord[0].xy) + 1.0, 2.0));\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      SHADER_READ_S16_INTEGER ("source")
      "void main (void) {\n"
      "  write_s16 (divide_s16 (read_source_s16 (gl_TexCoord[0].xy) + 1, 2));\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_UPSAMPLE_U8,
      "upsample_u8/normal",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_CAST_U8_S16
      SHADER_DIVIDE_S16
      SHADER_WRITE_U8
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 decrease1;\n"
      "uniform vec2 decrease2;\n"
      "uniform vec2 decrease3;\n"
      "uniform vec2 increase1;\n"
      "uniform vec2 increase2;\n"
      "uniform vec2 increase3;\n"
      "uniform vec2 increase4;\n"
      SHADER_READ_U8 ("source")
      "void main (void) {\n"
      "  float s3m = cast_s16_u8 (read_source_u8 (XY - decrease3));\n"
      "  float s2m = cast_s16_u8 (read_source_u8 (XY - decrease2));\n"
      "  float s1m = cast_s16_u8 (read_source_u8 (XY - decrease1));\n"
      "  float s0  = cast_s16_u8 (read_source_u8 (XY));\n"
      "  float s1p = cast_s16_u8 (read_source_u8 (XY + increase1));\n"
      "  float s2p = cast_s16_u8 (read_source_u8 (XY + increase2));\n"
      "  float s3p = cast_s16_u8 (read_source_u8 (XY + increase3));\n"
      "  float s4p = cast_s16_u8 (read_source_u8 (XY + increase4));\n"
      "  float sum = divide_s16 (-s3m + 3.0 * s2m - 7.0 * s1m + 21.0 * s0 + 21.0 * s1p - 7.0 * s2p + 3.0 * s3p - s4p + 16.0, 32.0);\n"
      "  write_u8 (cast_u8_s16 (sum));\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CAST_U8_S16_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_U8_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 decrease1;\n"
      "uniform vec2 decrease2;\n"
      "uniform vec2 decrease3;\n"
      "uniform vec2 increase1;\n"
      "uniform vec2 increase2;\n"
      "uniform vec2 increase3;\n"
      "uniform vec2 increase4;\n"
      SHADER_READ_U8_INTEGER ("source")
      "void main (void) {\n"
      "  int s3m = cast_s16_u8 (read_source_u8 (XY - decrease3));\n"
      "  int s2m = cast_s16_u8 (read_source_u8 (XY - decrease2));\n"
      "  int s1m = cast_s16_u8 (read_source_u8 (XY - decrease1));\n"
      "  int s0  = cast_s16_u8 (read_source_u8 (XY));\n"
      "  int s1p = cast_s16_u8 (read_source_u8 (XY + increase1));\n"
      "  int s2p = cast_s16_u8 (read_source_u8 (XY + increase2));\n"
      "  int s3p = cast_s16_u8 (read_source_u8 (XY + increase3));\n"
      "  int s4p = cast_s16_u8 (read_source_u8 (XY + increase4));\n"
      "  int sum = divide_s16 (-s3m + 3 * s2m - 7 * s1m + 21 * s0 + 21 * s1p - 7 * s2p + 3 * s3p - s4p + 16, 32);\n"
      "  write_u8 (cast_u8_s16 (sum));\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_SPATIAL_WEIGHT,
      "obmc_spatial_weight/normal",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "uniform vec2 block_length;\n"
      "uniform vec2 block_offset;\n"
      "uniform vec2 edge1;\n"
      "uniform vec2 edge2;\n"
      "float ramp (float coordinate, float block_offset) {\n"
      "  if (block_offset == 1.0) {\n"
      "    if (coordinate == 0.0) {\n"
      "      return 3.0;\n"
      "    } else {\n"
      "      return 5.0;\n"
      "    }\n"
      "  } else {\n"
      "    return 1.0 + divide_s16 (6.0 * coordinate + block_offset - 1.0, 2.0 * block_offset - 1.0);\n"
      "  }\n"
      "}\n"
      "float spatial_weight (float coordinate, float block_length, float block_offset, vec2 edge) {\n"
      "  if (block_offset == 0.0) {\n"
      "    return 8.0;\n"
      "  } else if (coordinate < 2.0 * block_offset) {\n"
      "    if (edge.x > 0.5) {\n"
      "      return 8.0;\n"
      "    } else {\n"
      "      return ramp (coordinate, block_offset);\n"
      "    }\n"
      "  } else if (block_length - 1.0 - coordinate < 2.0 * block_offset) {\n"
      "    if (edge.y > 0.5) {\n"
      "      return 8.0;\n"
      "    } else {\n"
      "      return ramp (block_length - 1.0 - coordinate, block_offset);\n"
      "    }\n"
      "  } else {\n"
      "    return 8.0;\n"
      "  }\n"
      "}\n"
      "void main (void) {\n"
      "  vec2 coordinate = vec2 (floor (gl_TexCoord[0].xy));\n"
      "  float weight_x = spatial_weight (coordinate.x, block_length.x, block_offset.x, vec2 (edge1.x, edge2.x));\n"
      "  float weight_y = spatial_weight (coordinate.y, block_length.y, block_offset.y, vec2 (edge1.y, edge2.y));\n"
      "  write_s16 (weight_x * weight_y);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "uniform ivec2 block_length;\n"
      "uniform ivec2 block_offset;\n"
      "uniform vec2 edge1;\n"
      "uniform vec2 edge2;\n"
      "int ramp (int coordinate, int block_offset) {\n"
      "  if (block_offset == 1) {\n"
      "    if (coordinate == 0) {\n"
      "      return 3;\n"
      "    } else {\n"
      "      return 5;\n"
      "    }\n"
      "  } else {\n"
      "    return 1 + divide_s16 (6 * coordinate + block_offset - 1, 2 * block_offset - 1);\n"
      "  }\n"
      "}\n"
      "int spatial_weight (int coordinate, int block_length, int block_offset, vec2 edge) {\n"
      "  if (block_offset == 0) {\n"
      "    return 8;\n"
      "  } else if (coordinate < 2 * block_offset) {\n"
      "    if (edge.x > 0.5) {\n"
      "      return 8;\n"
      "    } else {\n"
      "      return ramp (coordinate, block_offset);\n"
      "    }\n"
      "  } else if (block_length - 1 - coordinate < 2 * block_offset) {\n"
      "    if (edge.y > 0.5) {\n"
      "      return 8;\n"
      "    } else {\n"
      "      return ramp (block_length - 1 - coordinate, block_offset);\n"
      "    }\n"
      "  } else {\n"
      "    return 8;\n"
      "  }\n"
      "}\n"
      "void main (void) {\n"
      "  ivec2 coordinate = ivec2 (floor (gl_TexCoord[0].xy));\n"
      "  int weight_x = spatial_weight (coordinate.x, block_length.x, block_offset.x, vec2 (edge1.x, edge2.x));\n"
      "  int weight_y = spatial_weight (coordinate.y, block_length.y, block_offset.y, vec2 (edge1.y, edge2.y));\n"
      "  write_s16 (weight_x * weight_y);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_CLEAR,
      "obmc_clear/normal",
      SHADER_HEADER
      SHADER_WRITE_S16
      "void main (void) {\n"
      "  write_s16 (0.0);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_S16_INTEGER
      "void main (void) {\n"
      "  write_s16 (0);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_SHIFT,
      "obmc_shift/normal",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      SHADER_READ_S16 ("source")
      "void main (void) {\n"
      "  write_s16 (divide_s16 (read_source_s16 (gl_TexCoord[0].xy) - 8160.0, 64.0));\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      SHADER_READ_S16_INTEGER ("source")
      "void main (void) {\n"
      "  write_s16 (divide_s16 (read_source_s16 (gl_TexCoord[0].xy) - 8160, 64));\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_DC,
      "obmc_render_dc/normal",
      SHADER_HEADER
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform float dc;\n"
      "uniform vec2 block_origin;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  write_s16 (previous + dc * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform int dc;\n"
      "uniform vec2 block_origin;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  write_s16 (previous + dc * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_0,
      "obmc_render_ref_prec/0_normal",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate;\n"
      "  intermediate = cast_s16_u8 (read_upsampled_u8 (XY + offset));\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate;\n"
      "  intermediate = cast_s16_u8 (read_upsampled_u8 (XY + offset));\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_0_WEIGHT,
      "obmc_render_ref_prec/0_weight",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_DIVIDE_S16
      SHADER_REF_WEIGHTING_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate;\n"
      "  intermediate = cast_s16_u8 (read_upsampled_u8 (XY + offset));\n"
      "  write_s16 (previous + ref_weighting_s16 (intermediate) * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_REF_WEIGHTING_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate;\n"
      "  intermediate = cast_s16_u8 (read_upsampled_u8 (XY + offset));\n"
      "  write_s16 (previous + ref_weighting_s16 (intermediate) * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a,
      "obmc_render_ref_prec/3a_normal",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT2_S16
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1;\n"
      "uniform vec2 offset2;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1")
      SHADER_READ_U8 ("upsampled2")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate;\n"
      "  vec2 upsampled;\n"
      "  upsampled.x = cast_s16_u8 (read_upsampled1_u8 (XY + offset1));\n"
      "  upsampled.y = cast_s16_u8 (read_upsampled2_u8 (XY + offset2));\n"
      "  intermediate = divide_s16 (crossfoot2_s16 (upsampled) + 1.0, 2.0);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT2_S16_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1;\n"
      "uniform vec2 offset2;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1")
      SHADER_READ_U8_INTEGER ("upsampled2")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate;\n"
      "  ivec2 upsampled;\n"
      "  upsampled.x = cast_s16_u8 (read_upsampled1_u8 (XY + offset1));\n"
      "  upsampled.y = cast_s16_u8 (read_upsampled2_u8 (XY + offset2));\n"
      "  intermediate = divide_s16 (crossfoot2_s16 (upsampled) + 1, 2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a_WEIGHT,
      "obmc_render_ref_prec/3a_weight",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT2_S16
      SHADER_DIVIDE_S16
      SHADER_REF_WEIGHTING_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1;\n"
      "uniform vec2 offset2;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1")
      SHADER_READ_U8 ("upsampled2")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate;\n"
      "  vec2 upsampled;\n"
      "  upsampled.x = cast_s16_u8 (read_upsampled1_u8 (XY + offset1));\n"
      "  upsampled.y = cast_s16_u8 (read_upsampled2_u8 (XY + offset2));\n"
      "  intermediate = divide_s16 (crossfoot2_s16 (upsampled) + 1.0, 2.0);\n"
      "  write_s16 (previous + ref_weighting_s16 (intermediate) * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT2_S16_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_REF_WEIGHTING_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1;\n"
      "uniform vec2 offset2;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1")
      SHADER_READ_U8_INTEGER ("upsampled2")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate;\n"
      "  ivec2 upsampled;\n"
      "  upsampled.x = cast_s16_u8 (read_upsampled1_u8 (XY + offset1));\n"
      "  upsampled.y = cast_s16_u8 (read_upsampled2_u8 (XY + offset2));\n"
      "  intermediate = divide_s16 (crossfoot2_s16 (upsampled) + 1, 2);\n"
      "  write_s16 (previous + ref_weighting_s16 (intermediate) * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b,
      "obmc_render_ref_prec/3b_normal",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT4_S16
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1;\n"
      "uniform vec2 offset2;\n"
      "uniform vec2 offset3;\n"
      "uniform vec2 offset4;\n"
      "uniform vec4 linear_weight;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1")
      SHADER_READ_U8 ("upsampled2")
      SHADER_READ_U8 ("upsampled3")
      SHADER_READ_U8 ("upsampled4")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate;\n"
      "  vec4 upsampled;\n"
      "  upsampled.x = cast_s16_u8 (read_upsampled1_u8 (XY + offset1));\n"
      "  upsampled.y = cast_s16_u8 (read_upsampled2_u8 (XY + offset2));\n"
      "  upsampled.z = cast_s16_u8 (read_upsampled3_u8 (XY + offset3));\n"
      "  upsampled.w = cast_s16_u8 (read_upsampled4_u8 (XY + offset4));\n"
      "  intermediate = divide_s16 (crossfoot4_s16 (upsampled * linear_weight) + 8.0, 16.0);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT4_S16_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform ivec4 linear_weight;\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1;\n"
      "uniform vec2 offset2;\n"
      "uniform vec2 offset3;\n"
      "uniform vec2 offset4;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1")
      SHADER_READ_U8_INTEGER ("upsampled2")
      SHADER_READ_U8_INTEGER ("upsampled3")
      SHADER_READ_U8_INTEGER ("upsampled4")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate;\n"
      "  ivec4 upsampled;\n"
      "  upsampled.x = cast_s16_u8 (read_upsampled1_u8 (XY + offset1));\n"
      "  upsampled.y = cast_s16_u8 (read_upsampled2_u8 (XY + offset2));\n"
      "  upsampled.z = cast_s16_u8 (read_upsampled3_u8 (XY + offset3));\n"
      "  upsampled.w = cast_s16_u8 (read_upsampled4_u8 (XY + offset4));\n"
      "  intermediate = divide_s16 (crossfoot4_s16 (upsampled * linear_weight) + 8, 16);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b_WEIGHT,
      "obmc_render_ref_prec/3b_weight",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT4_S16
      SHADER_DIVIDE_S16
      SHADER_REF_WEIGHTING_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1;\n"
      "uniform vec2 offset2;\n"
      "uniform vec2 offset3;\n"
      "uniform vec2 offset4;\n"
      "uniform vec4 linear_weight;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1")
      SHADER_READ_U8 ("upsampled2")
      SHADER_READ_U8 ("upsampled3")
      SHADER_READ_U8 ("upsampled4")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate;\n"
      "  vec4 upsampled;\n"
      "  upsampled.x = cast_s16_u8 (read_upsampled1_u8 (XY + offset1));\n"
      "  upsampled.y = cast_s16_u8 (read_upsampled2_u8 (XY + offset2));\n"
      "  upsampled.z = cast_s16_u8 (read_upsampled3_u8 (XY + offset3));\n"
      "  upsampled.w = cast_s16_u8 (read_upsampled4_u8 (XY + offset4));\n"
      "  intermediate = divide_s16 (crossfoot4_s16 (upsampled * linear_weight) + 8.0, 16.0);\n"
      "  write_s16 (previous + ref_weighting_s16 (intermediate) * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT4_S16_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_REF_WEIGHTING_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform ivec4 linear_weight;\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1;\n"
      "uniform vec2 offset2;\n"
      "uniform vec2 offset3;\n"
      "uniform vec2 offset4;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1")
      SHADER_READ_U8_INTEGER ("upsampled2")
      SHADER_READ_U8_INTEGER ("upsampled3")
      SHADER_READ_U8_INTEGER ("upsampled4")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate;\n"
      "  ivec4 upsampled;\n"
      "  upsampled.x = cast_s16_u8 (read_upsampled1_u8 (XY + offset1));\n"
      "  upsampled.y = cast_s16_u8 (read_upsampled2_u8 (XY + offset2));\n"
      "  upsampled.z = cast_s16_u8 (read_upsampled3_u8 (XY + offset3));\n"
      "  upsampled.w = cast_s16_u8 (read_upsampled4_u8 (XY + offset4));\n"
      "  intermediate = divide_s16 (crossfoot4_s16 (upsampled * linear_weight) + 8, 16);\n"
      "  write_s16 (previous + ref_weighting_s16 (intermediate) * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0,
      "obmc_render_biref_prec/0_0_normal",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset_ref1;\n"
      "uniform vec2 offset_ref2;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled_ref1")
      SHADER_READ_U8 ("upsampled_ref2")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  intermediate_ref1 = cast_s16_u8 (read_upsampled_ref1_u8 (XY + offset_ref1));\n"
      "  intermediate_ref2 = cast_s16_u8 (read_upsampled_ref2_u8 (XY + offset_ref2));\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1.0, 2.0);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset_ref1;\n"
      "uniform vec2 offset_ref2;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled_ref1")
      SHADER_READ_U8_INTEGER ("upsampled_ref2")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  intermediate_ref1 = cast_s16_u8 (read_upsampled_ref1_u8 (XY + offset_ref1));\n"
      "  intermediate_ref2 = cast_s16_u8 (read_upsampled_ref2_u8 (XY + offset_ref2));\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1, 2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0_WEIGHT,
      "obmc_render_biref_prec/0_0_weight",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_BIREF_WEIGHTING_S16
      SHADER_CAST_S16_U8
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset_ref1;\n"
      "uniform vec2 offset_ref2;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled_ref1")
      SHADER_READ_U8 ("upsampled_ref2")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  intermediate_ref1 = cast_s16_u8 (read_upsampled_ref1_u8 (XY + offset_ref1));\n"
      "  intermediate_ref2 = cast_s16_u8 (read_upsampled_ref2_u8 (XY + offset_ref2));\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_BIREF_WEIGHTING_S16_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset_ref1;\n"
      "uniform vec2 offset_ref2;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled_ref1")
      SHADER_READ_U8_INTEGER ("upsampled_ref2")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  intermediate_ref1 = cast_s16_u8 (read_upsampled_ref1_u8 (XY + offset_ref1));\n"
      "  intermediate_ref2 = cast_s16_u8 (read_upsampled_ref2_u8 (XY + offset_ref2));\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a,
      "obmc_render_biref_prec/0_3a_normal",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT2_S16
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset_ref1;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref2")
      SHADER_READ_U8 ("upsampled2_ref2")
      SHADER_READ_U8 ("upsampled_ref1")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  intermediate_ref1 = cast_s16_u8 (read_upsampled_ref1_u8 (XY + offset_ref1));\n"
      "  vec2 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot2_s16 (upsampled_ref2) + 1.0, 2.0);\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1.0, 2.0);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT2_S16_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset_ref1;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref2")
      SHADER_READ_U8_INTEGER ("upsampled2_ref2")
      SHADER_READ_U8_INTEGER ("upsampled_ref1")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  intermediate_ref1 = cast_s16_u8 (read_upsampled_ref1_u8 (XY + offset_ref1));\n"
      "  ivec2 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot2_s16 (upsampled_ref2) + 1, 2);\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1, 2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a_WEIGHT,
      "obmc_render_biref_prec/0_3a_weight",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_BIREF_WEIGHTING_S16
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT2_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset_ref1;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref2")
      SHADER_READ_U8 ("upsampled2_ref2")
      SHADER_READ_U8 ("upsampled_ref1")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  intermediate_ref1 = cast_s16_u8 (read_upsampled_ref1_u8 (XY + offset_ref1));\n"
      "  vec2 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot2_s16 (upsampled_ref2) + 1.0, 2.0);\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_BIREF_WEIGHTING_S16_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT2_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset_ref1;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref2")
      SHADER_READ_U8_INTEGER ("upsampled2_ref2")
      SHADER_READ_U8_INTEGER ("upsampled_ref1")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  intermediate_ref1 = cast_s16_u8 (read_upsampled_ref1_u8 (XY + offset_ref1));\n"
      "  ivec2 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot2_s16 (upsampled_ref2) + 1, 2);\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b,
      "obmc_render_biref_prec/0_3b_normal",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT4_S16
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref2;\n"
      "uniform vec2 offset4_ref2;\n"
      "uniform vec2 offset_ref1;\n"
      "uniform vec4 linear_weight_ref2;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref2")
      SHADER_READ_U8 ("upsampled2_ref2")
      SHADER_READ_U8 ("upsampled3_ref2")
      SHADER_READ_U8 ("upsampled4_ref2")
      SHADER_READ_U8 ("upsampled_ref1")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  intermediate_ref1 = cast_s16_u8 (read_upsampled_ref1_u8 (XY + offset_ref1));\n"
      "  vec4 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  upsampled_ref2.z = cast_s16_u8 (read_upsampled3_ref2_u8 (XY + offset3_ref2));\n"
      "  upsampled_ref2.w = cast_s16_u8 (read_upsampled4_ref2_u8 (XY + offset4_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot4_s16 (upsampled_ref2 * linear_weight_ref2) + 8.0, 16.0);\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1.0, 2.0);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT4_S16_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform ivec4 linear_weight_ref2;\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref2;\n"
      "uniform vec2 offset4_ref2;\n"
      "uniform vec2 offset_ref1;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref2")
      SHADER_READ_U8_INTEGER ("upsampled2_ref2")
      SHADER_READ_U8_INTEGER ("upsampled3_ref2")
      SHADER_READ_U8_INTEGER ("upsampled4_ref2")
      SHADER_READ_U8_INTEGER ("upsampled_ref1")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  intermediate_ref1 = cast_s16_u8 (read_upsampled_ref1_u8 (XY + offset_ref1));\n"
      "  ivec4 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  upsampled_ref2.z = cast_s16_u8 (read_upsampled3_ref2_u8 (XY + offset3_ref2));\n"
      "  upsampled_ref2.w = cast_s16_u8 (read_upsampled4_ref2_u8 (XY + offset4_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot4_s16 (upsampled_ref2 * linear_weight_ref2) + 8, 16);\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1, 2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT,
      "obmc_render_biref_prec/0_3b_weight",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_BIREF_WEIGHTING_S16
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT4_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref2;\n"
      "uniform vec2 offset4_ref2;\n"
      "uniform vec2 offset_ref1;\n"
      "uniform vec4 linear_weight_ref2;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref2")
      SHADER_READ_U8 ("upsampled2_ref2")
      SHADER_READ_U8 ("upsampled3_ref2")
      SHADER_READ_U8 ("upsampled4_ref2")
      SHADER_READ_U8 ("upsampled_ref1")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  intermediate_ref1 = cast_s16_u8 (read_upsampled_ref1_u8 (XY + offset_ref1));\n"
      "  vec4 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  upsampled_ref2.z = cast_s16_u8 (read_upsampled3_ref2_u8 (XY + offset3_ref2));\n"
      "  upsampled_ref2.w = cast_s16_u8 (read_upsampled4_ref2_u8 (XY + offset4_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot4_s16 (upsampled_ref2 * linear_weight_ref2) + 8.0, 16.0);\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_BIREF_WEIGHTING_S16_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT4_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform ivec4 linear_weight_ref2;\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref2;\n"
      "uniform vec2 offset4_ref2;\n"
      "uniform vec2 offset_ref1;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref2")
      SHADER_READ_U8_INTEGER ("upsampled2_ref2")
      SHADER_READ_U8_INTEGER ("upsampled3_ref2")
      SHADER_READ_U8_INTEGER ("upsampled4_ref2")
      SHADER_READ_U8_INTEGER ("upsampled_ref1")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  intermediate_ref1 = cast_s16_u8 (read_upsampled_ref1_u8 (XY + offset_ref1));\n"
      "  ivec4 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  upsampled_ref2.z = cast_s16_u8 (read_upsampled3_ref2_u8 (XY + offset3_ref2));\n"
      "  upsampled_ref2.w = cast_s16_u8 (read_upsampled4_ref2_u8 (XY + offset4_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot4_s16 (upsampled_ref2 * linear_weight_ref2) + 8, 16);\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0,
      "obmc_render_biref_prec/3a_0_normal",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT2_S16
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset_ref2;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref1")
      SHADER_READ_U8 ("upsampled2_ref1")
      SHADER_READ_U8 ("upsampled_ref2")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  vec2 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot2_s16 (upsampled_ref1) + 1.0, 2.0);\n"
      "  intermediate_ref2 = cast_s16_u8 (read_upsampled_ref2_u8 (XY + offset_ref2));\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1.0, 2.0);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT2_S16_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset_ref2;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref1")
      SHADER_READ_U8_INTEGER ("upsampled2_ref1")
      SHADER_READ_U8_INTEGER ("upsampled_ref2")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  ivec2 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot2_s16 (upsampled_ref1) + 1, 2);\n"
      "  intermediate_ref2 = cast_s16_u8 (read_upsampled_ref2_u8 (XY + offset_ref2));\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1, 2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0_WEIGHT,
      "obmc_render_biref_prec/3a_0_weight",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_BIREF_WEIGHTING_S16
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT2_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset_ref2;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref1")
      SHADER_READ_U8 ("upsampled2_ref1")
      SHADER_READ_U8 ("upsampled_ref2")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  vec2 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot2_s16 (upsampled_ref1) + 1.0, 2.0);\n"
      "  intermediate_ref2 = cast_s16_u8 (read_upsampled_ref2_u8 (XY + offset_ref2));\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_BIREF_WEIGHTING_S16_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT2_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset_ref2;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref1")
      SHADER_READ_U8_INTEGER ("upsampled2_ref1")
      SHADER_READ_U8_INTEGER ("upsampled_ref2")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  ivec2 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot2_s16 (upsampled_ref1) + 1, 2);\n"
      "  intermediate_ref2 = cast_s16_u8 (read_upsampled_ref2_u8 (XY + offset_ref2));\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a,
      "obmc_render_biref_prec/3a_3a_normal",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT2_S16
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref1")
      SHADER_READ_U8 ("upsampled1_ref2")
      SHADER_READ_U8 ("upsampled2_ref1")
      SHADER_READ_U8 ("upsampled2_ref2")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  vec2 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot2_s16 (upsampled_ref1) + 1.0, 2.0);\n"
      "  vec2 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot2_s16 (upsampled_ref2) + 1.0, 2.0);\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1.0, 2.0);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT2_S16_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref1")
      SHADER_READ_U8_INTEGER ("upsampled1_ref2")
      SHADER_READ_U8_INTEGER ("upsampled2_ref1")
      SHADER_READ_U8_INTEGER ("upsampled2_ref2")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  ivec2 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot2_s16 (upsampled_ref1) + 1, 2);\n"
      "  ivec2 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot2_s16 (upsampled_ref2) + 1, 2);\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1, 2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a_WEIGHT,
      "obmc_render_biref_prec/3a_3a_weight",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_BIREF_WEIGHTING_S16
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT2_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref1")
      SHADER_READ_U8 ("upsampled1_ref2")
      SHADER_READ_U8 ("upsampled2_ref1")
      SHADER_READ_U8 ("upsampled2_ref2")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  vec2 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot2_s16 (upsampled_ref1) + 1.0, 2.0);\n"
      "  vec2 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot2_s16 (upsampled_ref2) + 1.0, 2.0);\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_BIREF_WEIGHTING_S16_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT2_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref1")
      SHADER_READ_U8_INTEGER ("upsampled1_ref2")
      SHADER_READ_U8_INTEGER ("upsampled2_ref1")
      SHADER_READ_U8_INTEGER ("upsampled2_ref2")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  ivec2 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot2_s16 (upsampled_ref1) + 1, 2);\n"
      "  ivec2 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot2_s16 (upsampled_ref2) + 1, 2);\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b,
      "obmc_render_biref_prec/3a_3b_normal",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT2_S16
      SHADER_CROSSFOOT4_S16
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref2;\n"
      "uniform vec2 offset4_ref2;\n"
      "uniform vec4 linear_weight_ref2;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref1")
      SHADER_READ_U8 ("upsampled1_ref2")
      SHADER_READ_U8 ("upsampled2_ref1")
      SHADER_READ_U8 ("upsampled2_ref2")
      SHADER_READ_U8 ("upsampled3_ref2")
      SHADER_READ_U8 ("upsampled4_ref2")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  vec2 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot2_s16 (upsampled_ref1) + 1.0, 2.0);\n"
      "  vec4 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  upsampled_ref2.z = cast_s16_u8 (read_upsampled3_ref2_u8 (XY + offset3_ref2));\n"
      "  upsampled_ref2.w = cast_s16_u8 (read_upsampled4_ref2_u8 (XY + offset4_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot4_s16 (upsampled_ref2 * linear_weight_ref2) + 8.0, 16.0);\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1.0, 2.0);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT2_S16_INTEGER
      SHADER_CROSSFOOT4_S16_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform ivec4 linear_weight_ref2;\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref2;\n"
      "uniform vec2 offset4_ref2;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref1")
      SHADER_READ_U8_INTEGER ("upsampled1_ref2")
      SHADER_READ_U8_INTEGER ("upsampled2_ref1")
      SHADER_READ_U8_INTEGER ("upsampled2_ref2")
      SHADER_READ_U8_INTEGER ("upsampled3_ref2")
      SHADER_READ_U8_INTEGER ("upsampled4_ref2")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  ivec2 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot2_s16 (upsampled_ref1) + 1, 2);\n"
      "  ivec4 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  upsampled_ref2.z = cast_s16_u8 (read_upsampled3_ref2_u8 (XY + offset3_ref2));\n"
      "  upsampled_ref2.w = cast_s16_u8 (read_upsampled4_ref2_u8 (XY + offset4_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot4_s16 (upsampled_ref2 * linear_weight_ref2) + 8, 16);\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1, 2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT,
      "obmc_render_biref_prec/3a_3b_weight",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_BIREF_WEIGHTING_S16
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT2_S16
      SHADER_CROSSFOOT4_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref2;\n"
      "uniform vec2 offset4_ref2;\n"
      "uniform vec4 linear_weight_ref2;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref1")
      SHADER_READ_U8 ("upsampled1_ref2")
      SHADER_READ_U8 ("upsampled2_ref1")
      SHADER_READ_U8 ("upsampled2_ref2")
      SHADER_READ_U8 ("upsampled3_ref2")
      SHADER_READ_U8 ("upsampled4_ref2")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  vec2 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot2_s16 (upsampled_ref1) + 1.0, 2.0);\n"
      "  vec4 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  upsampled_ref2.z = cast_s16_u8 (read_upsampled3_ref2_u8 (XY + offset3_ref2));\n"
      "  upsampled_ref2.w = cast_s16_u8 (read_upsampled4_ref2_u8 (XY + offset4_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot4_s16 (upsampled_ref2 * linear_weight_ref2) + 8.0, 16.0);\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_BIREF_WEIGHTING_S16_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT2_S16_INTEGER
      SHADER_CROSSFOOT4_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform ivec4 linear_weight_ref2;\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref2;\n"
      "uniform vec2 offset4_ref2;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref1")
      SHADER_READ_U8_INTEGER ("upsampled1_ref2")
      SHADER_READ_U8_INTEGER ("upsampled2_ref1")
      SHADER_READ_U8_INTEGER ("upsampled2_ref2")
      SHADER_READ_U8_INTEGER ("upsampled3_ref2")
      SHADER_READ_U8_INTEGER ("upsampled4_ref2")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  ivec2 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot2_s16 (upsampled_ref1) + 1, 2);\n"
      "  ivec4 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  upsampled_ref2.z = cast_s16_u8 (read_upsampled3_ref2_u8 (XY + offset3_ref2));\n"
      "  upsampled_ref2.w = cast_s16_u8 (read_upsampled4_ref2_u8 (XY + offset4_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot4_s16 (upsampled_ref2 * linear_weight_ref2) + 8, 16);\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0,
      "obmc_render_biref_prec/3b_0_normal",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT4_S16
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset3_ref1;\n"
      "uniform vec2 offset4_ref1;\n"
      "uniform vec2 offset_ref2;\n"
      "uniform vec4 linear_weight_ref1;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref1")
      SHADER_READ_U8 ("upsampled2_ref1")
      SHADER_READ_U8 ("upsampled3_ref1")
      SHADER_READ_U8 ("upsampled4_ref1")
      SHADER_READ_U8 ("upsampled_ref2")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  vec4 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  upsampled_ref1.z = cast_s16_u8 (read_upsampled3_ref1_u8 (XY + offset3_ref1));\n"
      "  upsampled_ref1.w = cast_s16_u8 (read_upsampled4_ref1_u8 (XY + offset4_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot4_s16 (upsampled_ref1 * linear_weight_ref1) + 8.0, 16.0);\n"
      "  intermediate_ref2 = cast_s16_u8 (read_upsampled_ref2_u8 (XY + offset_ref2));\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1.0, 2.0);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT4_S16_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform ivec4 linear_weight_ref1;\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset3_ref1;\n"
      "uniform vec2 offset4_ref1;\n"
      "uniform vec2 offset_ref2;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref1")
      SHADER_READ_U8_INTEGER ("upsampled2_ref1")
      SHADER_READ_U8_INTEGER ("upsampled3_ref1")
      SHADER_READ_U8_INTEGER ("upsampled4_ref1")
      SHADER_READ_U8_INTEGER ("upsampled_ref2")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  ivec4 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  upsampled_ref1.z = cast_s16_u8 (read_upsampled3_ref1_u8 (XY + offset3_ref1));\n"
      "  upsampled_ref1.w = cast_s16_u8 (read_upsampled4_ref1_u8 (XY + offset4_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot4_s16 (upsampled_ref1 * linear_weight_ref1) + 8, 16);\n"
      "  intermediate_ref2 = cast_s16_u8 (read_upsampled_ref2_u8 (XY + offset_ref2));\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1, 2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT,
      "obmc_render_biref_prec/3b_0_weight",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_BIREF_WEIGHTING_S16
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT4_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset3_ref1;\n"
      "uniform vec2 offset4_ref1;\n"
      "uniform vec2 offset_ref2;\n"
      "uniform vec4 linear_weight_ref1;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref1")
      SHADER_READ_U8 ("upsampled2_ref1")
      SHADER_READ_U8 ("upsampled3_ref1")
      SHADER_READ_U8 ("upsampled4_ref1")
      SHADER_READ_U8 ("upsampled_ref2")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  vec4 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  upsampled_ref1.z = cast_s16_u8 (read_upsampled3_ref1_u8 (XY + offset3_ref1));\n"
      "  upsampled_ref1.w = cast_s16_u8 (read_upsampled4_ref1_u8 (XY + offset4_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot4_s16 (upsampled_ref1 * linear_weight_ref1) + 8.0, 16.0);\n"
      "  intermediate_ref2 = cast_s16_u8 (read_upsampled_ref2_u8 (XY + offset_ref2));\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_BIREF_WEIGHTING_S16_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT4_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform ivec4 linear_weight_ref1;\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset3_ref1;\n"
      "uniform vec2 offset4_ref1;\n"
      "uniform vec2 offset_ref2;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref1")
      SHADER_READ_U8_INTEGER ("upsampled2_ref1")
      SHADER_READ_U8_INTEGER ("upsampled3_ref1")
      SHADER_READ_U8_INTEGER ("upsampled4_ref1")
      SHADER_READ_U8_INTEGER ("upsampled_ref2")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  ivec4 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  upsampled_ref1.z = cast_s16_u8 (read_upsampled3_ref1_u8 (XY + offset3_ref1));\n"
      "  upsampled_ref1.w = cast_s16_u8 (read_upsampled4_ref1_u8 (XY + offset4_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot4_s16 (upsampled_ref1 * linear_weight_ref1) + 8, 16);\n"
      "  intermediate_ref2 = cast_s16_u8 (read_upsampled_ref2_u8 (XY + offset_ref2));\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a,
      "obmc_render_biref_prec/3b_3a_normal",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT2_S16
      SHADER_CROSSFOOT4_S16
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref1;\n"
      "uniform vec2 offset4_ref1;\n"
      "uniform vec4 linear_weight_ref1;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref1")
      SHADER_READ_U8 ("upsampled1_ref2")
      SHADER_READ_U8 ("upsampled2_ref1")
      SHADER_READ_U8 ("upsampled2_ref2")
      SHADER_READ_U8 ("upsampled3_ref1")
      SHADER_READ_U8 ("upsampled4_ref1")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  vec4 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  upsampled_ref1.z = cast_s16_u8 (read_upsampled3_ref1_u8 (XY + offset3_ref1));\n"
      "  upsampled_ref1.w = cast_s16_u8 (read_upsampled4_ref1_u8 (XY + offset4_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot4_s16 (upsampled_ref1 * linear_weight_ref1) + 8.0, 16.0);\n"
      "  vec2 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot2_s16 (upsampled_ref2) + 1.0, 2.0);\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1.0, 2.0);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT2_S16_INTEGER
      SHADER_CROSSFOOT4_S16_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform ivec4 linear_weight_ref1;\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref1;\n"
      "uniform vec2 offset4_ref1;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref1")
      SHADER_READ_U8_INTEGER ("upsampled1_ref2")
      SHADER_READ_U8_INTEGER ("upsampled2_ref1")
      SHADER_READ_U8_INTEGER ("upsampled2_ref2")
      SHADER_READ_U8_INTEGER ("upsampled3_ref1")
      SHADER_READ_U8_INTEGER ("upsampled4_ref1")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  ivec4 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  upsampled_ref1.z = cast_s16_u8 (read_upsampled3_ref1_u8 (XY + offset3_ref1));\n"
      "  upsampled_ref1.w = cast_s16_u8 (read_upsampled4_ref1_u8 (XY + offset4_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot4_s16 (upsampled_ref1 * linear_weight_ref1) + 8, 16);\n"
      "  ivec2 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot2_s16 (upsampled_ref2) + 1, 2);\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1, 2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT,
      "obmc_render_biref_prec/3b_3a_weight",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_BIREF_WEIGHTING_S16
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT2_S16
      SHADER_CROSSFOOT4_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref1;\n"
      "uniform vec2 offset4_ref1;\n"
      "uniform vec4 linear_weight_ref1;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref1")
      SHADER_READ_U8 ("upsampled1_ref2")
      SHADER_READ_U8 ("upsampled2_ref1")
      SHADER_READ_U8 ("upsampled2_ref2")
      SHADER_READ_U8 ("upsampled3_ref1")
      SHADER_READ_U8 ("upsampled4_ref1")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  vec4 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  upsampled_ref1.z = cast_s16_u8 (read_upsampled3_ref1_u8 (XY + offset3_ref1));\n"
      "  upsampled_ref1.w = cast_s16_u8 (read_upsampled4_ref1_u8 (XY + offset4_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot4_s16 (upsampled_ref1 * linear_weight_ref1) + 8.0, 16.0);\n"
      "  vec2 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot2_s16 (upsampled_ref2) + 1.0, 2.0);\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_BIREF_WEIGHTING_S16_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT2_S16_INTEGER
      SHADER_CROSSFOOT4_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform ivec4 linear_weight_ref1;\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref1;\n"
      "uniform vec2 offset4_ref1;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref1")
      SHADER_READ_U8_INTEGER ("upsampled1_ref2")
      SHADER_READ_U8_INTEGER ("upsampled2_ref1")
      SHADER_READ_U8_INTEGER ("upsampled2_ref2")
      SHADER_READ_U8_INTEGER ("upsampled3_ref1")
      SHADER_READ_U8_INTEGER ("upsampled4_ref1")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  ivec4 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  upsampled_ref1.z = cast_s16_u8 (read_upsampled3_ref1_u8 (XY + offset3_ref1));\n"
      "  upsampled_ref1.w = cast_s16_u8 (read_upsampled4_ref1_u8 (XY + offset4_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot4_s16 (upsampled_ref1 * linear_weight_ref1) + 8, 16);\n"
      "  ivec2 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot2_s16 (upsampled_ref2) + 1, 2);\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b,
      "obmc_render_biref_prec/3b_3b_normal",
      SHADER_HEADER
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT4_S16
      SHADER_DIVIDE_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref1;\n"
      "uniform vec2 offset3_ref2;\n"
      "uniform vec2 offset4_ref1;\n"
      "uniform vec2 offset4_ref2;\n"
      "uniform vec4 linear_weight_ref1;\n"
      "uniform vec4 linear_weight_ref2;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref1")
      SHADER_READ_U8 ("upsampled1_ref2")
      SHADER_READ_U8 ("upsampled2_ref1")
      SHADER_READ_U8 ("upsampled2_ref2")
      SHADER_READ_U8 ("upsampled3_ref1")
      SHADER_READ_U8 ("upsampled3_ref2")
      SHADER_READ_U8 ("upsampled4_ref1")
      SHADER_READ_U8 ("upsampled4_ref2")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  vec4 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  upsampled_ref1.z = cast_s16_u8 (read_upsampled3_ref1_u8 (XY + offset3_ref1));\n"
      "  upsampled_ref1.w = cast_s16_u8 (read_upsampled4_ref1_u8 (XY + offset4_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot4_s16 (upsampled_ref1 * linear_weight_ref1) + 8.0, 16.0);\n"
      "  vec4 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  upsampled_ref2.z = cast_s16_u8 (read_upsampled3_ref2_u8 (XY + offset3_ref2));\n"
      "  upsampled_ref2.w = cast_s16_u8 (read_upsampled4_ref2_u8 (XY + offset4_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot4_s16 (upsampled_ref2 * linear_weight_ref2) + 8.0, 16.0);\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1.0, 2.0);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT4_S16_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform ivec4 linear_weight_ref1;\n"
      "uniform ivec4 linear_weight_ref2;\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref1;\n"
      "uniform vec2 offset3_ref2;\n"
      "uniform vec2 offset4_ref1;\n"
      "uniform vec2 offset4_ref2;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref1")
      SHADER_READ_U8_INTEGER ("upsampled1_ref2")
      SHADER_READ_U8_INTEGER ("upsampled2_ref1")
      SHADER_READ_U8_INTEGER ("upsampled2_ref2")
      SHADER_READ_U8_INTEGER ("upsampled3_ref1")
      SHADER_READ_U8_INTEGER ("upsampled3_ref2")
      SHADER_READ_U8_INTEGER ("upsampled4_ref1")
      SHADER_READ_U8_INTEGER ("upsampled4_ref2")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  ivec4 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  upsampled_ref1.z = cast_s16_u8 (read_upsampled3_ref1_u8 (XY + offset3_ref1));\n"
      "  upsampled_ref1.w = cast_s16_u8 (read_upsampled4_ref1_u8 (XY + offset4_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot4_s16 (upsampled_ref1 * linear_weight_ref1) + 8, 16);\n"
      "  ivec4 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  upsampled_ref2.z = cast_s16_u8 (read_upsampled3_ref2_u8 (XY + offset3_ref2));\n"
      "  upsampled_ref2.w = cast_s16_u8 (read_upsampled4_ref2_u8 (XY + offset4_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot4_s16 (upsampled_ref2 * linear_weight_ref2) + 8, 16);\n"
      "  intermediate = divide_s16 (intermediate_ref1 + intermediate_ref2 + 1, 2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },
  { SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT,
      "obmc_render_biref_prec/3b_3b_weight",
      SHADER_HEADER
      SHADER_DIVIDE_S16
      SHADER_BIREF_WEIGHTING_S16
      SHADER_CAST_S16_U8
      SHADER_CROSSFOOT4_S16
      SHADER_WRITE_S16
      "#define XY gl_TexCoord[0].xy\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref1;\n"
      "uniform vec2 offset3_ref2;\n"
      "uniform vec2 offset4_ref1;\n"
      "uniform vec2 offset4_ref2;\n"
      "uniform vec4 linear_weight_ref1;\n"
      "uniform vec4 linear_weight_ref2;\n"
      SHADER_READ_S16 ("previous")
      SHADER_READ_S16 ("spatial_weight")
      SHADER_READ_U8 ("upsampled1_ref1")
      SHADER_READ_U8 ("upsampled1_ref2")
      SHADER_READ_U8 ("upsampled2_ref1")
      SHADER_READ_U8 ("upsampled2_ref2")
      SHADER_READ_U8 ("upsampled3_ref1")
      SHADER_READ_U8 ("upsampled3_ref2")
      SHADER_READ_U8 ("upsampled4_ref1")
      SHADER_READ_U8 ("upsampled4_ref2")
      "void main (void) {\n"
      "  float previous = read_previous_s16 (XY);\n"
      "  float spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  float intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  vec4 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  upsampled_ref1.z = cast_s16_u8 (read_upsampled3_ref1_u8 (XY + offset3_ref1));\n"
      "  upsampled_ref1.w = cast_s16_u8 (read_upsampled4_ref1_u8 (XY + offset4_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot4_s16 (upsampled_ref1 * linear_weight_ref1) + 8.0, 16.0);\n"
      "  vec4 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  upsampled_ref2.z = cast_s16_u8 (read_upsampled3_ref2_u8 (XY + offset3_ref2));\n"
      "  upsampled_ref2.w = cast_s16_u8 (read_upsampled4_ref2_u8 (XY + offset4_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot4_s16 (upsampled_ref2 * linear_weight_ref2) + 8.0, 16.0);\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n",
      SHADER_HEADER_INTEGER
      SHADER_DIVIDE_S16_INTEGER
      SHADER_BIREF_WEIGHTING_S16_INTEGER
      SHADER_CAST_S16_U8_INTEGER
      SHADER_CROSSFOOT4_S16_INTEGER
      SHADER_WRITE_S16_INTEGER
      "#define XY gl_TexCoord[0].xy\n"
      "uniform ivec4 linear_weight_ref1;\n"
      "uniform ivec4 linear_weight_ref2;\n"
      "uniform vec2 block_origin;\n"
      "uniform vec2 offset1_ref1;\n"
      "uniform vec2 offset1_ref2;\n"
      "uniform vec2 offset2_ref1;\n"
      "uniform vec2 offset2_ref2;\n"
      "uniform vec2 offset3_ref1;\n"
      "uniform vec2 offset3_ref2;\n"
      "uniform vec2 offset4_ref1;\n"
      "uniform vec2 offset4_ref2;\n"
      SHADER_READ_S16_INTEGER ("previous")
      SHADER_READ_S16_INTEGER ("spatial_weight")
      SHADER_READ_U8_INTEGER ("upsampled1_ref1")
      SHADER_READ_U8_INTEGER ("upsampled1_ref2")
      SHADER_READ_U8_INTEGER ("upsampled2_ref1")
      SHADER_READ_U8_INTEGER ("upsampled2_ref2")
      SHADER_READ_U8_INTEGER ("upsampled3_ref1")
      SHADER_READ_U8_INTEGER ("upsampled3_ref2")
      SHADER_READ_U8_INTEGER ("upsampled4_ref1")
      SHADER_READ_U8_INTEGER ("upsampled4_ref2")
      "void main (void) {\n"
      "  int previous = read_previous_s16 (XY);\n"
      "  int spatial_weight = read_spatial_weight_s16 (XY - block_origin);\n"
      "  int intermediate_ref1, intermediate_ref2, intermediate;\n"
      "  ivec4 upsampled_ref1;\n"
      "  upsampled_ref1.x = cast_s16_u8 (read_upsampled1_ref1_u8 (XY + offset1_ref1));\n"
      "  upsampled_ref1.y = cast_s16_u8 (read_upsampled2_ref1_u8 (XY + offset2_ref1));\n"
      "  upsampled_ref1.z = cast_s16_u8 (read_upsampled3_ref1_u8 (XY + offset3_ref1));\n"
      "  upsampled_ref1.w = cast_s16_u8 (read_upsampled4_ref1_u8 (XY + offset4_ref1));\n"
      "  intermediate_ref1 = divide_s16 (crossfoot4_s16 (upsampled_ref1 * linear_weight_ref1) + 8, 16);\n"
      "  ivec4 upsampled_ref2;\n"
      "  upsampled_ref2.x = cast_s16_u8 (read_upsampled1_ref2_u8 (XY + offset1_ref2));\n"
      "  upsampled_ref2.y = cast_s16_u8 (read_upsampled2_ref2_u8 (XY + offset2_ref2));\n"
      "  upsampled_ref2.z = cast_s16_u8 (read_upsampled3_ref2_u8 (XY + offset3_ref2));\n"
      "  upsampled_ref2.w = cast_s16_u8 (read_upsampled4_ref2_u8 (XY + offset4_ref2));\n"
      "  intermediate_ref2 = divide_s16 (crossfoot4_s16 (upsampled_ref2 * linear_weight_ref2) + 8, 16);\n"
      "  intermediate = biref_weighting_s16 (intermediate_ref1, intermediate_ref2);\n"
      "  write_s16 (previous + intermediate * spatial_weight);\n"
      "}\n" },

  { -1, NULL }
};

void
schro_opengl_shader_resolve_uniform_locations (SchroOpenGLShader* shader)
{
  #define GET_UNIFORM_LOCATION(_name) \
      shader->uniforms->_name \
          = glGetUniformLocationARB (shader->program, #_name)

  GET_UNIFORM_LOCATION (ayuv);
  GET_UNIFORM_LOCATION (block_length);
  GET_UNIFORM_LOCATION (block_offset);
  GET_UNIFORM_LOCATION (block_origin);
  GET_UNIFORM_LOCATION (dc);
  GET_UNIFORM_LOCATION (decrease1);
  GET_UNIFORM_LOCATION (decrease2);
  GET_UNIFORM_LOCATION (decrease3);
  GET_UNIFORM_LOCATION (edge1);
  GET_UNIFORM_LOCATION (edge2);
  GET_UNIFORM_LOCATION (increase1);
  GET_UNIFORM_LOCATION (increase2);
  GET_UNIFORM_LOCATION (increase3);
  GET_UNIFORM_LOCATION (increase4);
  GET_UNIFORM_LOCATION (linear_weight);
  GET_UNIFORM_LOCATION (linear_weight_ref1);
  GET_UNIFORM_LOCATION (linear_weight_ref2);
  GET_UNIFORM_LOCATION (offset);
  GET_UNIFORM_LOCATION (offset1);
  GET_UNIFORM_LOCATION (offset1_ref1);
  GET_UNIFORM_LOCATION (offset1_ref2);
  GET_UNIFORM_LOCATION (offset2);
  GET_UNIFORM_LOCATION (offset2_ref1);
  GET_UNIFORM_LOCATION (offset2_ref2);
  GET_UNIFORM_LOCATION (offset3);
  GET_UNIFORM_LOCATION (offset3_ref1);
  GET_UNIFORM_LOCATION (offset3_ref2);
  GET_UNIFORM_LOCATION (offset4);
  GET_UNIFORM_LOCATION (offset4_ref1);
  GET_UNIFORM_LOCATION (offset4_ref2);
  GET_UNIFORM_LOCATION (offset_ref1);
  GET_UNIFORM_LOCATION (offset_ref2);
  GET_UNIFORM_LOCATION (previous);
  GET_UNIFORM_LOCATION (ref_addend);
  GET_UNIFORM_LOCATION (ref_divisor);
  GET_UNIFORM_LOCATION (ref_weight);
  GET_UNIFORM_LOCATION (ref_weight1);
  GET_UNIFORM_LOCATION (ref_weight2);
  GET_UNIFORM_LOCATION (source);
  GET_UNIFORM_LOCATION (source1);
  GET_UNIFORM_LOCATION (source2);
  GET_UNIFORM_LOCATION (spatial_weight);
  GET_UNIFORM_LOCATION (u2);
  GET_UNIFORM_LOCATION (u4);
  GET_UNIFORM_LOCATION (upsampled);
  GET_UNIFORM_LOCATION (upsampled1);
  GET_UNIFORM_LOCATION (upsampled1_ref1);
  GET_UNIFORM_LOCATION (upsampled1_ref2);
  GET_UNIFORM_LOCATION (upsampled2);
  GET_UNIFORM_LOCATION (upsampled2_ref1);
  GET_UNIFORM_LOCATION (upsampled2_ref2);
  GET_UNIFORM_LOCATION (upsampled3);
  GET_UNIFORM_LOCATION (upsampled3_ref1);
  GET_UNIFORM_LOCATION (upsampled3_ref2);
  GET_UNIFORM_LOCATION (upsampled4);
  GET_UNIFORM_LOCATION (upsampled4_ref1);
  GET_UNIFORM_LOCATION (upsampled4_ref2);
  GET_UNIFORM_LOCATION (upsampled_ref1);
  GET_UNIFORM_LOCATION (upsampled_ref2);
  GET_UNIFORM_LOCATION (uyvy);
  GET_UNIFORM_LOCATION (v2);
  GET_UNIFORM_LOCATION (v4);
  GET_UNIFORM_LOCATION (y4);
  GET_UNIFORM_LOCATION (yuyv);

  #undef GET_UNIFORM_LOCATION

  glUseProgramObjectARB (shader->program);

  #define UNIFORM(_name, _read_slot) \
      glUniform1iARB (shader->uniforms->_name, _read_slot)

  UNIFORM (ayuv, 0);
  UNIFORM (previous, 2);
  UNIFORM (source, 0);
  UNIFORM (source1, 9);
  UNIFORM (source2, 0);
  UNIFORM (spatial_weight, 3);
  UNIFORM (u2, 5);
  UNIFORM (u4, 7);
  UNIFORM (upsampled, 0);
  UNIFORM (upsampled1, 4);
  UNIFORM (upsampled1_ref1, 9);
  UNIFORM (upsampled1_ref2, 8);
  UNIFORM (upsampled2, 1);
  UNIFORM (upsampled2_ref1, 6);
  UNIFORM (upsampled2_ref2, 7);
  UNIFORM (upsampled3, 0);
  UNIFORM (upsampled3_ref1, 1);
  UNIFORM (upsampled3_ref2, 0);
  UNIFORM (upsampled4, 5);
  UNIFORM (upsampled4_ref1, 5);
  UNIFORM (upsampled4_ref2, 4);
  UNIFORM (upsampled_ref1, 1);
  UNIFORM (upsampled_ref2, 0);
  UNIFORM (uyvy, 0);
  UNIFORM (v2, 6);
  UNIFORM (v4, 8);
  UNIFORM (y4, 4);
  UNIFORM (yuyv, 0);

  #undef UNIFORM

  glUseProgramObjectARB (0);
}

void
schro_opengl_shader_bind_ayuv (GLuint texture)
{
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
}

void
schro_opengl_shader_bind_previous (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 2);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_source (GLuint texture)
{
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
}

void
schro_opengl_shader_bind_source1 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 9);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_source2 (GLuint texture)
{
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
}

void
schro_opengl_shader_bind_spatial_weight (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 3);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_u2 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 5);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_u4 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 7);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_upsampled (GLuint texture)
{
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
}

void
schro_opengl_shader_bind_upsampled1 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 4);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_upsampled1_ref1 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 9);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_upsampled1_ref2 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 8);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_upsampled2 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 1);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_upsampled2_ref1 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 6);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_upsampled2_ref2 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 7);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_upsampled3 (GLuint texture)
{
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
}

void
schro_opengl_shader_bind_upsampled3_ref1 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 1);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_upsampled3_ref2 (GLuint texture)
{
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
}

void
schro_opengl_shader_bind_upsampled4 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 5);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_upsampled4_ref1 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 5);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_upsampled4_ref2 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 4);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_upsampled_ref1 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 1);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_upsampled_ref2 (GLuint texture)
{
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
}

void
schro_opengl_shader_bind_uyvy (GLuint texture)
{
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
}

void
schro_opengl_shader_bind_v2 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 6);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_v4 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 8);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_y4 (GLuint texture)
{
  glActiveTextureARB (GL_TEXTURE0_ARB + 4);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glActiveTextureARB (GL_TEXTURE0_ARB);
}

void
schro_opengl_shader_bind_yuyv (GLuint texture)
{
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
}

void
schro_opengl_shader_bind_block_length (SchroOpenGLShader* shader, int16_t v0, int16_t v1)
{
  if (shader->is_integer) {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_SPATIAL_WEIGHT:
        glUniform2iARB (shader->uniforms->block_length, v0, v1);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  } else {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_SPATIAL_WEIGHT:
        glUniform2fARB (shader->uniforms->block_length, v0, v1);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  }
}

void
schro_opengl_shader_bind_block_offset (SchroOpenGLShader* shader, int16_t v0, int16_t v1)
{
  if (shader->is_integer) {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_SPATIAL_WEIGHT:
        glUniform2iARB (shader->uniforms->block_offset, v0, v1);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  } else {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_SPATIAL_WEIGHT:
        glUniform2fARB (shader->uniforms->block_offset, v0, v1);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  }
}

void
schro_opengl_shader_bind_block_origin (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_DC:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_0:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_0_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
      glUniform2fARB (shader->uniforms->block_origin, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_dc (SchroOpenGLShader* shader, int16_t v0)
{
  if (shader->is_integer) {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_DC:
        glUniform1iARB (shader->uniforms->dc, v0);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  } else {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_DC:
        glUniform1fARB (shader->uniforms->dc, v0);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  }
}

void
schro_opengl_shader_bind_decrease1 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_9_7_Lp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_9_7_Hp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_LE_GALL_5_3_Lp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_13_7_Lp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_13_7_Hp:
    case SCHRO_OPENGL_SHADER_UPSAMPLE_U8:
      glUniform2fARB (shader->uniforms->decrease1, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_decrease2 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_13_7_Lp:
    case SCHRO_OPENGL_SHADER_UPSAMPLE_U8:
      glUniform2fARB (shader->uniforms->decrease2, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_decrease3 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_UPSAMPLE_U8:
      glUniform2fARB (shader->uniforms->decrease3, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_edge1 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_SPATIAL_WEIGHT:
      glUniform2fARB (shader->uniforms->edge1, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_edge2 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_SPATIAL_WEIGHT:
      glUniform2fARB (shader->uniforms->edge2, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_increase1 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_9_7_Hp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_LE_GALL_5_3_Hp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_13_7_Lp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_13_7_Hp:
    case SCHRO_OPENGL_SHADER_UPSAMPLE_U8:
      glUniform2fARB (shader->uniforms->increase1, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_increase2 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_9_7_Hp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_13_7_Hp:
    case SCHRO_OPENGL_SHADER_UPSAMPLE_U8:
      glUniform2fARB (shader->uniforms->increase2, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_increase3 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_UPSAMPLE_U8:
      glUniform2fARB (shader->uniforms->increase3, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_increase4 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_UPSAMPLE_U8:
      glUniform2fARB (shader->uniforms->increase4, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_linear_weight (SchroOpenGLShader* shader, int16_t v0, int16_t v1, int16_t v2, int16_t v3)
{
  if (shader->is_integer) {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b_WEIGHT:
        glUniform4iARB (shader->uniforms->linear_weight, v0, v1, v2, v3);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  } else {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b_WEIGHT:
        glUniform4fARB (shader->uniforms->linear_weight, v0, v1, v2, v3);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  }
}

void
schro_opengl_shader_bind_linear_weight_ref1 (SchroOpenGLShader* shader, int16_t v0, int16_t v1, int16_t v2, int16_t v3)
{
  if (shader->is_integer) {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
        glUniform4iARB (shader->uniforms->linear_weight_ref1, v0, v1, v2, v3);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  } else {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
        glUniform4fARB (shader->uniforms->linear_weight_ref1, v0, v1, v2, v3);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  }
}

void
schro_opengl_shader_bind_linear_weight_ref2 (SchroOpenGLShader* shader, int16_t v0, int16_t v1, int16_t v2, int16_t v3)
{
  if (shader->is_integer) {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
        glUniform4iARB (shader->uniforms->linear_weight_ref2, v0, v1, v2, v3);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  } else {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
        glUniform4fARB (shader->uniforms->linear_weight_ref2, v0, v1, v2, v3);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  }
}

void
schro_opengl_shader_bind_offset (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_9_7_Lp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_9_7_Hp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_LE_GALL_5_3_Lp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_LE_GALL_5_3_Hp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_13_7_Lp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_DESLAURIERS_DUBUC_13_7_Hp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_HAAR_Lp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_FILTER_HAAR_Hp:
    case SCHRO_OPENGL_SHADER_IIWT_S16_VERTICAL_DEINTERLEAVE_H:
    case SCHRO_OPENGL_SHADER_IIWT_S16_VERTICAL_INTERLEAVE:
    case SCHRO_OPENGL_SHADER_IIWT_S16_HORIZONTAL_INTERLEAVE:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_0:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_0_WEIGHT:
      glUniform2fARB (shader->uniforms->offset, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_offset1 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b_WEIGHT:
      glUniform2fARB (shader->uniforms->offset1, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_offset1_ref1 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
      glUniform2fARB (shader->uniforms->offset1_ref1, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_offset1_ref2 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
      glUniform2fARB (shader->uniforms->offset1_ref2, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_offset2 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b_WEIGHT:
      glUniform2fARB (shader->uniforms->offset2, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_offset2_ref1 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
      glUniform2fARB (shader->uniforms->offset2_ref1, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_offset2_ref2 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
      glUniform2fARB (shader->uniforms->offset2_ref2, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_offset3 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b_WEIGHT:
      glUniform2fARB (shader->uniforms->offset3, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_offset3_ref1 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
      glUniform2fARB (shader->uniforms->offset3_ref1, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_offset3_ref2 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
      glUniform2fARB (shader->uniforms->offset3_ref2, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_offset4 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b_WEIGHT:
      glUniform2fARB (shader->uniforms->offset4, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_offset4_ref1 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
      glUniform2fARB (shader->uniforms->offset4_ref1, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_offset4_ref2 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
      glUniform2fARB (shader->uniforms->offset4_ref2, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_offset_ref1 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
      glUniform2fARB (shader->uniforms->offset_ref1, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_offset_ref2 (SchroOpenGLShader* shader, float v0, float v1)
{
  switch (shader->index) {
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0_WEIGHT:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0:
    case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
      glUniform2fARB (shader->uniforms->offset_ref2, v0, v1);
      break;
    default:
      SCHRO_ASSERT (0);
      break;
  }
}

void
schro_opengl_shader_bind_ref_addend (SchroOpenGLShader* shader, int16_t v0)
{
  if (shader->is_integer) {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
        glUniform1iARB (shader->uniforms->ref_addend, v0);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  } else {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
        glUniform1fARB (shader->uniforms->ref_addend, v0);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  }
}

void
schro_opengl_shader_bind_ref_divisor (SchroOpenGLShader* shader, int16_t v0)
{
  if (shader->is_integer) {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
        glUniform1iARB (shader->uniforms->ref_divisor, v0);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  } else {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
        glUniform1fARB (shader->uniforms->ref_divisor, v0);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  }
}

void
schro_opengl_shader_bind_ref_weight (SchroOpenGLShader* shader, int16_t v0)
{
  if (shader->is_integer) {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b_WEIGHT:
        glUniform1iARB (shader->uniforms->ref_weight, v0);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  } else {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_REF_PREC_3b_WEIGHT:
        glUniform1fARB (shader->uniforms->ref_weight, v0);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  }
}

void
schro_opengl_shader_bind_ref_weight1 (SchroOpenGLShader* shader, int16_t v0)
{
  if (shader->is_integer) {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
        glUniform1iARB (shader->uniforms->ref_weight1, v0);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  } else {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
        glUniform1fARB (shader->uniforms->ref_weight1, v0);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  }
}

void
schro_opengl_shader_bind_ref_weight2 (SchroOpenGLShader* shader, int16_t v0)
{
  if (shader->is_integer) {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
        glUniform1iARB (shader->uniforms->ref_weight2, v0);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  } else {
    switch (shader->index) {
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_0_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3a_3b_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_0_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3a_WEIGHT:
      case SCHRO_OPENGL_SHADER_OBMC_RENDER_BIREF_PREC_3b_3b_WEIGHT:
        glUniform1fARB (shader->uniforms->ref_weight2, v0);
        break;
      default:
        SCHRO_ASSERT (0);
        break;
    }
  }
}

