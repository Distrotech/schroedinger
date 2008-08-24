
#ifndef __SCHRO_OPENGL_CANVAS_H__
#define __SCHRO_OPENGL_CANVAS_H__

#include <schroedinger/schroframe.h>
#include <schroedinger/opengl/schroopengltypes.h>
#include <GL/glew.h>

SCHRO_BEGIN_DECLS

#define SCHRO_OPNEGL_CANVAS_FROM_FRAMEDATA(_framedata) \
    ((SchroOpenGLCanvas *) (_framedata)->data)

#define SCHRO_OPNEGL_CANVAS_TO_FRAMEDATA(_framedata, _canvas) \
    (_framedata)->data = _canvas

typedef enum _SchroOpenGLPixelbufferType {
  SCHRO_OPENGL_PIXELBUFFER_TYPE_PUSH = 0,
  SCHRO_OPENGL_PIXELBUFFER_TYPE_PULL = 1
} SchroOpenGLPixelbufferType;

#define SCHRO_OPENGL_PIXELBUFFER_BLOCKS 4

struct _SchroOpenGLPixelbuffer {
  SchroOpenGL *opengl;
  SchroOpenGLPixelbufferType type;
  int refcount;
  int uselessness;

  /* frame */
  int width;
  int height;
  int stride;

  /* blocks */
  GLuint handles[SCHRO_OPENGL_PIXELBUFFER_BLOCKS];
  int heights[SCHRO_OPENGL_PIXELBUFFER_BLOCKS];
};

typedef enum _SchroOpenGLCanvasType {
  /* a primary canvas is used to store framedata. each primary canvas is only
     assigned to a single framedata object at the same time. it has a secondary
     canvas and the configured set of pixelbuffers assigned */
  SCHRO_OPENGL_CANVAS_TYPE_PRIMARAY = 0,
  /* a secondary canvas is used as temorary rendertarget to store intermediate
     results while working with a primary canvas, each secondary canvas may be
     assigned to multiple primary canvases at the same time. it has no secondary
     canvas and no pixelbuffers assigned */
  SCHRO_OPENGL_CANVAS_TYPE_SECONDARY = 1,
  /* a spatial weight canvas is used to store a spatial weight block. like the
     primary canvas, each spatial weight canvas is only assigned to a single
     spatial weight block at the same time. like the secondary canvas, it has no
     secondary canvas and no pixelbuffers assigned */
  SCHRO_OPENGL_CANVAS_TYPE_SPATIAL_WEIGHT = 2
} SchroOpenGLCanvasType;

struct _SchroOpenGLCanvas {
  SchroOpenGL *opengl;
  SchroOpenGLCanvasType type;
  int refcount;
  int uselessness;

  /* frame */
  SchroFrameFormat format;
  int width;
  int height;

  /* texture */
  GLuint texture;
  GLenum internal_format;
  GLenum pixel_format;
  GLenum storage_type;
  int channels;

  /* framebuffer */
  GLuint framebuffer;

  /* secondary canvas, only for primary canvas */
  SchroOpenGLCanvas* secondary;

  /* push pixelbuffer, only for primary canvas */
  GLenum push_type;
  int push_stride;
  SchroOpenGLPixelbuffer* push_pixelbuffer;

  /* pull pixelbuffer, only for primary canvas */
  GLenum pull_type;
  int pull_stride;
  SchroOpenGLPixelbuffer* pull_pixelbuffer;
};

#define SCHRO_OPENGL_CANVAS_POOL_LIMIT 1024

// FIXME: add a mechanism to drop long time unused canvases from the pool
struct _SchroOpenGLCanvasPool {
  SchroOpenGL *opengl;

  SchroOpenGLCanvas *canvases[3][SCHRO_OPENGL_CANVAS_POOL_LIMIT];
  int canvas_count[3];

  SchroOpenGLPixelbuffer *pixelbuffers[2][SCHRO_OPENGL_CANVAS_POOL_LIMIT];
  int pixelbuffer_count[2];
};

// FIXME: reduce storage flags to fixed point, float and integer
#define SCHRO_OPENGL_CANVAS_STORE_PACKED_AS_RGBA (1 <<  0)
#define SCHRO_OPENGL_CANVAS_STORE_PACKED_AS_BGRA (1 <<  1)
#define SCHRO_OPENGL_CANVAS_STORE_U8_AS_U8       (1 <<  2)
#define SCHRO_OPENGL_CANVAS_STORE_U8_AS_F16      (1 <<  3)
#define SCHRO_OPENGL_CANVAS_STORE_U8_AS_F32      (1 <<  4)
#define SCHRO_OPENGL_CANVAS_STORE_U8_AS_UI8      (1 <<  5)
#define SCHRO_OPENGL_CANVAS_STORE_S16_AS_S16     (1 <<  6)
#define SCHRO_OPENGL_CANVAS_STORE_S16_AS_U16     (1 <<  7)
#define SCHRO_OPENGL_CANVAS_STORE_S16_AS_F16     (1 <<  8)
#define SCHRO_OPENGL_CANVAS_STORE_S16_AS_F32     (1 <<  9)
#define SCHRO_OPENGL_CANVAS_STORE_S16_AS_I16     (1 << 10)
#define SCHRO_OPENGL_CANVAS_STORE_S16_AS_UI16    (1 << 11)

#define SCHRO_OPENGL_CANVAS_PUSH_SUBIMAGE        (1 << 12)
#define SCHRO_OPENGL_CANVAS_PUSH_RENDER_QUAD     (1 << 13)
#define SCHRO_OPENGL_CANVAS_PUSH_DRAWPIXELS      (1 << 14)
#define SCHRO_OPENGL_CANVAS_PUSH_U8_PIXELBUFFER  (1 << 15)
#define SCHRO_OPENGL_CANVAS_PUSH_S16_PIXELBUFFER (1 << 16)
#define SCHRO_OPENGL_CANVAS_PUSH_U8_AS_U8        (1 << 17)
#define SCHRO_OPENGL_CANVAS_PUSH_U8_AS_F32       (1 << 18)
#define SCHRO_OPENGL_CANVAS_PUSH_S16_AS_S16      (1 << 19)
#define SCHRO_OPENGL_CANVAS_PUSH_S16_AS_U16      (1 << 20)
#define SCHRO_OPENGL_CANVAS_PUSH_S16_AS_F32      (1 << 21)

#define SCHRO_OPENGL_CANVAS_PULL_U8_PIXELBUFFER  (1 << 22)
#define SCHRO_OPENGL_CANVAS_PULL_S16_PIXELBUFFER (1 << 23)
#define SCHRO_OPENGL_CANVAS_PULL_U8_AS_U8        (1 << 24)
#define SCHRO_OPENGL_CANVAS_PULL_U8_AS_F32       (1 << 25)
#define SCHRO_OPENGL_CANVAS_PULL_S16_AS_S16      (1 << 26)
#define SCHRO_OPENGL_CANVAS_PULL_S16_AS_U16      (1 << 27)
#define SCHRO_OPENGL_CANVAS_PULL_S16_AS_F32      (1 << 28)

#define SCHRO_OPENGL_CANVAS_FLAG_COUNT                 29

extern uint32_t _schro_opengl_canvas_flags;

#define SCHRO_OPENGL_CANVAS_IS_FLAG_SET(_flag) \
    (_schro_opengl_canvas_flags & SCHRO_OPENGL_CANVAS_##_flag)
#define SCHRO_OPENGL_CANVAS_SET_FLAG(_flag) \
    (_schro_opengl_canvas_flags |= SCHRO_OPENGL_CANVAS_##_flag)
#define SCHRO_OPENGL_CANVAS_CLEAR_FLAG(_flag) \
    (_schro_opengl_canvas_flags &= ~SCHRO_OPENGL_CANVAS_##_flag)

void schro_opengl_canvas_setup_flags (void);
void schro_opengl_canvas_check_flags (void);
void schro_opengl_canvas_print_flags (const char* indent);

SchroOpenGLCanvas *schro_opengl_canvas_new (SchroOpenGL *opengl,
    SchroOpenGLCanvasType type, SchroFrameFormat format, int width, int height);
void schro_opengl_canvas_unref (SchroOpenGLCanvas *canvas);
void schro_opengl_canvas_push (SchroOpenGLCanvas *dest,
    SchroFrameData *src); // CPU -> GPU
void schro_opengl_canvas_pull (SchroFrameData *dest,
    SchroOpenGLCanvas *src); // CPU <- GPU

SchroOpenGLPixelbuffer *schro_opengl_pixelbuffer_new (SchroOpenGL *opengl,
    SchroOpenGLPixelbufferType type, int width, int height, int stride);
void schro_opengl_pixelbuffer_unref (SchroOpenGLPixelbuffer *pixelbuffer);

SchroOpenGLCanvasPool *schro_opengl_canvas_pool_new (SchroOpenGL *opengl);
void schro_opengl_canvas_pool_free (SchroOpenGLCanvasPool* pool);
void schro_opengl_canvas_pool_squeeze (SchroOpenGLCanvasPool* pool);

SCHRO_END_DECLS

#endif

