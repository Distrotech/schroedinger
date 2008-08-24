
#ifndef __SCHRO_OPENGL_H__
#define __SCHRO_OPENGL_H__

#include <schroedinger/schrodomain.h>
#include <schroedinger/schroutils.h>
#include <schroedinger/opengl/schroopengltypes.h>

SCHRO_BEGIN_DECLS

#define SCHRO_OPENGL_LOCK_CONTEXT(_opengl) \
    schro_opengl_lock_context (_opengl, __FILE__, __LINE__, __FUNCTION__)

#define SCHRO_OPENGL_UNLOCK_CONTEXT(_opengl) \
    schro_opengl_unlock_context (_opengl, __FILE__, __LINE__, __FUNCTION__)

#define SCHRO_OPENGL_CHECK_ERROR \
    schro_opengl_check_error (__FILE__, __LINE__, __FUNCTION__);

#define SCHRO_OPENGL_CHECK_FRAMEBUFFER \
    schro_opengl_check_framebuffer (__FILE__, __LINE__, __FUNCTION__);

#if 1
#define SCHRO_OPENGL_FLUSH \
    glFlush ();
#else
#define SCHRO_OPENGL_FLUSH
#endif

/*typedef enum _SchroOpenGLPipelineType {
  SCHRO_OPENGL_PIPELINE_TYPE_FIXED_POINT = 0,
  //SCHRO_OPENGL_PIPELINE_TYPE_FLOAT, // FIXME: investigate this
  SCHRO_OPENGL_PIPELINE_TYPE_INTEGER
} SchroOpenGLPipelineType;*/ // FIXME

void schro_opengl_init (void);

SchroOpenGL *schro_opengl_new (void);
void schro_opengl_free (SchroOpenGL *opengl);
int schro_opengl_is_usable (SchroOpenGL *opengl);

void schro_opengl_lock_context (SchroOpenGL *opengl, const char *file, int line,
    const char *function);
void schro_opengl_unlock_context (SchroOpenGL *opengl, const char *file,
    int line, const char *function);
void schro_opengl_lock_canvas_pool (SchroOpenGL *opengl);
void schro_opengl_unlock_canvas_pool (SchroOpenGL *opengl);
void schro_opengl_check_error (const char *file, int line,
    const char *function);
void schro_opengl_check_framebuffer (const char *file, int line,
    const char *function);
void schro_opengl_set_visible (SchroOpenGL *opengl, int visible);
void schro_opengl_setup_viewport (int width, int height);
void schro_opengl_render_quad (int x, int y, int width, int height);

//SchroOpenGLPipelineType *schro_opengl_get_pipeline_type (SchroOpenGL *opengl); // FIXME
void *schro_opengl_get_tmp (SchroOpenGL *opengl, int size);
SchroOpenGLShaderLibrary *schro_opengl_get_shader_library (SchroOpenGL *opengl);
SchroOpenGLCanvasPool *schro_opengl_get_canvas_pool (SchroOpenGL *opengl);
SchroOpenGLSpatialWeightPool *schro_opengl_get_spatial_weight_pool
    (SchroOpenGL *opengl);
void schro_opengl_squeeze_pools (SchroOpenGL *opengl);

SchroMemoryDomain *schro_memory_domain_new_opengl (void);

SCHRO_END_DECLS

#endif

