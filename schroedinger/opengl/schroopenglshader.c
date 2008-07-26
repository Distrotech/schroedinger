
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <schroedinger/schrodebug.h>
#include <schroedinger/opengl/schroopengl.h>
#include <schroedinger/opengl/schroopenglcanvas.h>
#include <schroedinger/opengl/schroopenglshader.h>
#include <stdio.h>
#include <string.h>

static char*
schro_opengl_shader_add_linenumbers (const char* string)
{
  const char *src = string;
  char *dest;
  char *linenumbered;
  char number[16];
  int lines = 1;
  int size;

  while (*src) {
    if (*src == '\n') {
      ++lines;
    }

    ++src;
  }

  snprintf (number, sizeof (number) - 1, "%3i: ", lines);

  size = strlen (string) + 1 + lines * strlen (number);
  linenumbered = schro_malloc0 (size);
  src = string;
  dest = linenumbered;

  strcpy (dest, "  1: ");

  dest += strlen ("  1: ");
  lines = 2;

  while (*src) {
    *dest++ = *src;

    if (*src == '\n') {
      snprintf (number, sizeof (number) - 1, "%3i: ", lines);
      strcpy (dest, number);

      dest += strlen (number);
      ++lines;
    }

    ++src;
  }

  return linenumbered;
}

static void
schro_opengl_shader_trim_right (char* string)
{
  int length = strlen (string);
  char* temp;

  if (length < 1) {
    return;
  }

  temp = string + length - 1;

  while (string <= temp && (*temp == ' ' || *temp == '\n')) {
    *temp-- = '\0';
  }
}

static void
schro_opengl_shader_strip_false_positives (char* infolog)
{
  int length;
  char* temp1;
  char* temp2;

  temp1 = strstr (infolog, "Fragment info\n-------------\n");

  if (temp1) {
    length = strlen ("Fragment info\n-------------\n");

    memmove (temp1, temp1 + length, strlen (temp1) - length + 1);
  }

  temp1 = strstr (infolog, "warning C7050: \"$temp");

  while (temp1) {
    temp2 = temp1;

    while (temp1 != infolog && temp1[-1] != '\n') {
      --temp1;
    }

    while (temp2[1] != '\0' && temp2[-1] != '\n') {
      ++temp2;
    }

    memmove (temp1, temp2, strlen (temp2) + 1);

    temp1 = strstr (infolog, "warning C7050: \"$temp");
  }
}

static int
schro_opengl_shader_check_status (GLhandleARB handle, GLenum status,
    const char* code, const char* name)
{
  GLint result;
  GLint length;
  char* infolog;
  char* linenumbered_code;

  glGetObjectParameterivARB (handle, status, &result);
  glGetObjectParameterivARB (handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);

  if (length < 1) {
    return result != 0;
  }

  infolog = schro_malloc0 (length * sizeof (char));

  glGetInfoLogARB (handle, length, &length, infolog);

  if (length > 0) {
    linenumbered_code = schro_opengl_shader_add_linenumbers (code);

    schro_opengl_shader_strip_false_positives (infolog);
    schro_opengl_shader_trim_right (infolog);

    if (strlen (infolog) > 0) {
      SCHRO_ERROR ("\nshadername:\n%s\nshadercode:\n%s\ninfolog:\n%s", name,
          linenumbered_code, infolog);
    }

    schro_free (linenumbered_code);
  }

  schro_free (infolog);

  return result != 0;
}

static SchroOpenGLShader *
schro_opengl_shader_new (const char* code, const char* name)
{
  SchroOpenGLShader *shader;
  GLhandleARB handle;
  int ok;

  shader = schro_malloc0 (sizeof (SchroOpenGLShader));
  handle = glCreateShaderObjectARB (GL_FRAGMENT_SHADER_ARB);

  glShaderSourceARB (handle, 1, (const char**)&code, 0);
  glCompileShaderARB (handle);

  ok = schro_opengl_shader_check_status (handle, GL_OBJECT_COMPILE_STATUS_ARB,
      code, name);

  SCHRO_ASSERT (ok);

  shader->program = glCreateProgramObjectARB ();

  glAttachObjectARB (shader->program, handle);
  glDeleteObjectARB (handle);
  glLinkProgramARB (shader->program);

  ok = schro_opengl_shader_check_status (shader->program,
      GL_OBJECT_LINK_STATUS_ARB, code, name);

  SCHRO_ASSERT (ok);

  glValidateProgramARB (shader->program);

  ok = schro_opengl_shader_check_status (shader->program,
      GL_OBJECT_VALIDATE_STATUS_ARB, code, name);

  SCHRO_ASSERT (ok);

  #define UNIFORM_LOCATION_SAMPLER(_name, _member) \
      do { \
        if (strstr (code, "uniform sampler2DRect "#_name";") || \
            strstr (code, "uniform usampler2DRect "#_name";") || \
            strstr (code, "uniform isampler2DRect "#_name";")) { \
          shader->_member = glGetUniformLocationARB (shader->program, #_name); \
        } else { \
          shader->_member = -1; \
        } \
      } while (0)

  UNIFORM_LOCATION_SAMPLER (texture1,  textures[0]);
  UNIFORM_LOCATION_SAMPLER (texture2,  textures[1]);
  UNIFORM_LOCATION_SAMPLER (texture3,  textures[2]);
  UNIFORM_LOCATION_SAMPLER (texture4,  textures[3]);
  UNIFORM_LOCATION_SAMPLER (texture5,  textures[4]);
  UNIFORM_LOCATION_SAMPLER (texture6,  textures[5]);
  UNIFORM_LOCATION_SAMPLER (texture7,  textures[6]);
  UNIFORM_LOCATION_SAMPLER (texture8,  textures[7]);
  UNIFORM_LOCATION_SAMPLER (texture9,  textures[8]);
  UNIFORM_LOCATION_SAMPLER (texture10, textures[9]);

  #undef UNIFORM_LOCATION

  #define UNIFORM_LOCATION(_type, _name, _member) \
      do { \
        if (strstr (code, "uniform "#_type" "#_name";")) { \
          shader->_member = glGetUniformLocationARB (shader->program, #_name); \
        } else { \
          shader->_member = -1; \
        } \
      } while (0)

  UNIFORM_LOCATION (vec2,  offset1,        offsets[0]);
  UNIFORM_LOCATION (vec2,  offset2,        offsets[1]);
  UNIFORM_LOCATION (vec2,  offset3,        offsets[2]);
  UNIFORM_LOCATION (vec2,  offset4,        offsets[3]);
  UNIFORM_LOCATION (vec2,  offset5,        offsets[4]);
  UNIFORM_LOCATION (vec2,  offset6,        offsets[5]);
  UNIFORM_LOCATION (vec2,  offset7,        offsets[6]);
  UNIFORM_LOCATION (vec2,  offset8,        offsets[7]);
  UNIFORM_LOCATION (vec2,  origin,         origin);
  UNIFORM_LOCATION (vec2,  size,           size);
  UNIFORM_LOCATION (vec2,  remaining1,     remainings[0]);
  UNIFORM_LOCATION (vec2,  remaining2,     remainings[1]);
  UNIFORM_LOCATION (vec2,  four_decrease,  four_decrease);
  UNIFORM_LOCATION (vec2,  three_decrease, three_decrease);
  UNIFORM_LOCATION (vec2,  two_decrease,   two_decrease);
  UNIFORM_LOCATION (vec2,  one_decrease,   one_decrease);
  UNIFORM_LOCATION (vec2,  one_increase,   one_increase);
  UNIFORM_LOCATION (vec2,  two_increase,   two_increase);
  UNIFORM_LOCATION (vec2,  three_increase, three_increase);
  UNIFORM_LOCATION (vec2,  four_increase,  four_increase);
  UNIFORM_LOCATION (float, dc,             dc);
  UNIFORM_LOCATION (float, ref_weight1,    ref_weights[0]);
  UNIFORM_LOCATION (float, ref_weight2,    ref_weights[1]);
  UNIFORM_LOCATION (float, ref_addend,     ref_addend);
  UNIFORM_LOCATION (float, ref_divisor,    ref_divisor);

  #undef UNIFORM_LOCATION_SAMPLER

  if (GLEW_EXT_gpu_shader4) {
    if (strstr (code, "varying out uvec4 fragcolor_u8;")) {
      glBindFragDataLocationEXT (shader->program, 0, "fragcolor_u8");
    } else if (strstr (code, "varying out ivec4 fragcolor_s16;")) {
      glBindFragDataLocationEXT (shader->program, 0, "fragcolor_s16");
    }
  }

  return shader;
}

static void
schro_opengl_shader_free (SchroOpenGLShader *shader)
{
  SCHRO_ASSERT (shader != NULL);

  glDeleteObjectARB (shader->program);

  schro_free (shader);
}

#include "schroopenglshadercompiler.output"

struct _SchroOpenGLShaderLibrary {
  SchroOpenGL *opengl;
  SchroOpenGLShader *shaders[SCHRO_OPENGL_SHADER_COUNT];
};

SchroOpenGLShaderLibrary *
schro_opengl_shader_library_new (SchroOpenGL *opengl)
{
  int i;
  SchroOpenGLShaderLibrary* shader_library
      = schro_malloc0 (sizeof (SchroOpenGLShaderLibrary));

  shader_library->opengl = opengl;

  schro_opengl_lock_context (opengl);

  schro_opengl_canvas_check_flags ();

  for (i = 0; i < SCHRO_OPENGL_SHADER_COUNT; ++i) {
    SCHRO_ASSERT (schro_opengl_shader_code_list[i].index == i);

    if (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_U8_AS_UI8) &&
        (SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_UI16) ||
        SCHRO_OPENGL_CANVAS_IS_FLAG_SET (STORE_S16_AS_U16))) {
      shader_library->shaders[i]
          = schro_opengl_shader_new (schro_opengl_shader_code_list[i].code_integer,
          schro_opengl_shader_code_list[i].name);
    } else {
      shader_library->shaders[i]
          = schro_opengl_shader_new (schro_opengl_shader_code_list[i].code,
          schro_opengl_shader_code_list[i].name);
    }
  }

  schro_opengl_unlock_context (opengl);

  return shader_library;
}

void
schro_opengl_shader_library_free (SchroOpenGLShaderLibrary *shader_library)
{
  int i;

  SCHRO_ASSERT (shader_library != NULL);

  schro_opengl_lock_context (shader_library->opengl);

  for (i = 0; i < SCHRO_OPENGL_SHADER_COUNT; ++i) {
    if (shader_library->shaders[i]) {
      schro_opengl_shader_free (shader_library->shaders[i]);
    }
  }

  schro_opengl_unlock_context (shader_library->opengl);

  schro_free (shader_library);
}

SchroOpenGLShader *
schro_opengl_shader_get (SchroOpenGL *opengl, int index)
{
  SchroOpenGLShaderLibrary* shader_library;

  SCHRO_ASSERT (index >= 0);
  SCHRO_ASSERT (index < SCHRO_OPENGL_SHADER_COUNT);

  shader_library = schro_opengl_get_shader_library (opengl);

  return shader_library->shaders[index];
}

