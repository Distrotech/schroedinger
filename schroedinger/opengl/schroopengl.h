
#ifndef __SCHRO_OPENGL_H__
#define __SCHRO_OPENGL_H__

#include <GL/glew.h>

SCHRO_BEGIN_DECLS

#define SCHRO_OPENGL_CHECK_ERROR \
    schro_opengl_check_error (__FILE__, __LINE__, __FUNCTION__);

#define SCHRO_OPENGL_CHECK_FRAMEBUFFER \
    schro_opengl_check_framebuffer (__FILE__, __LINE__, __FUNCTION__);

#ifndef SCHRO_OPENGL_TYPEDEF
#define SCHRO_OPENGL_TYPEDEF
typedef struct _SchroOpenGL SchroOpenGL;
#endif

typedef struct _SchroOpenGLShaderLibrary SchroOpenGLShaderLibrary;

void schro_opengl_init (void);

SchroOpenGL *schro_opengl_new (void);
void schro_opengl_free (SchroOpenGL *opengl);
int schro_opengl_is_usable (SchroOpenGL *opengl);

void schro_opengl_lock (SchroOpenGL *opengl);
void schro_opengl_unlock (SchroOpenGL *opengl);
void schro_opengl_check_error (const char *file, int line, const char *func);
void schro_opengl_check_framebuffer (const char *file, int line,
    const char *func);
void schro_opengl_set_visible (SchroOpenGL *opengl, int visible);
SchroOpenGLShaderLibrary *schro_opengl_get_library (SchroOpenGL *opengl);
void *schro_opengl_get_tmp (SchroOpenGL *opengl, int size);
GLuint schro_opengl_get_obmc_weight_texture (SchroOpenGL *opengl, int width,
    int height);
void schro_opengl_setup_viewport (int width, int height);
void schro_opengl_render_quad (int x, int y, int width, int height);

SchroMemoryDomain *schro_memory_domain_new_opengl (void);

SCHRO_END_DECLS

#endif

