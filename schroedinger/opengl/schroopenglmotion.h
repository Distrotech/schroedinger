 
#ifndef __SCHRO_OPENGL_MOTION_H__
#define __SCHRO_OPENGL_MOTION_H__

#include <schroedinger/schroframe.h>

SCHRO_BEGIN_DECLS

void schro_opengl_motion_render (SchroMotion *motion, SchroFrame *dest);

SchroOpenGLSpatialWeightBlock *
schro_opengl_spatial_weight_block_new (SchroOpenGL *opengl, int* edges,
    int xbsep, int ybsep, int xblen, int yblen);
void schro_opengl_spatial_weight_block_unref
    (SchroOpenGLSpatialWeightBlock *block);

SchroOpenGLSpatialWeightGrid *
schro_opengl_spatial_weight_grid_new (SchroOpenGL *opengl,
    int width, int height, int x_num_blocks, int y_num_blocks, int xbsep,
    int ybsep, int xblen, int yblen);
void schro_opengl_spatial_weight_grid_unref
    (SchroOpenGLSpatialWeightGrid *grid);

SchroOpenGLSpatialWeightPool *schro_opengl_spatial_weight_pool_new (void);
void schro_opengl_spatial_weight_pool_free (SchroOpenGLSpatialWeightPool* pool);
void schro_opengl_spatial_weight_pool_squeeze
    (SchroOpenGLSpatialWeightPool* pool);

SCHRO_END_DECLS

#endif

