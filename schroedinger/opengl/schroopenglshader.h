
#ifndef __SCHRO_OPENGL_SHADER_H__
#define __SCHRO_OPENGL_SHADER_H__

#include <schroedinger/schroutils.h>
#include <schroedinger/opengl/schroopengltypes.h>
#include <GL/glew.h>

SCHRO_BEGIN_DECLS

struct _SchroOpenGLShader {
  int index;
  GLhandleARB program;
  SchroOpenGLUniforms* uniforms;
  int is_integer;
};

SchroOpenGLShaderLibrary *schro_opengl_shader_library_new (SchroOpenGL *opengl);
void schro_opengl_shader_library_free (SchroOpenGLShaderLibrary *shader_library);
SchroOpenGLShader *schro_opengl_shader_get (SchroOpenGL *opengl, int index);

SCHRO_END_DECLS

#endif

