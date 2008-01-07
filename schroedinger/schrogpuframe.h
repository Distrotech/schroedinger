
#ifndef __SCHRO_GPUFRAME_H__
#define __SCHRO_GPUFRAME_H__

#include <schroedinger/schro-stdint.h>
#include <schroedinger/schroframe.h>

SCHRO_BEGIN_DECLS

SchroFrame * schro_gpuframe_new (void);
SchroFrame * schro_gpuframe_new_and_alloc (SchroFrameFormat format, int width, int height);
SchroFrame * schro_gpuframe_new_clone (SchroFrame *src);

void schro_gpuframe_setstream(SchroFrame *frame, SchroCUDAStream stream);

void schro_gpuframe_to_cpu (SchroFrame *dest, SchroFrame *src);
void schro_frame_to_gpu (SchroFrame *dest, SchroFrame *src);

void schro_gpuframe_set_free_callback (SchroFrame *frame, SchroFrameFreeFunc free_func, void *priv);
void schro_gpuframe_unref (SchroFrame *frame);
SchroFrame *schro_gpuframe_ref (SchroFrame *frame);
void schro_gpuframe_convert (SchroFrame *dest, SchroFrame *src);
void schro_gpuframe_add (SchroFrame *dest, SchroFrame *src);
void schro_gpuframe_subtract (SchroFrame *dest, SchroFrame *src);
//void schro_gpuframe_shift_left (SchroFrame *frame, int shift);
//void schro_gpuframe_shift_right (SchroFrame *frame, int shift);
//void schro_gpuframe_edge_extend (SchroFrame *frame, int width, int height);
//void schro_gpuframe_zero_extend (SchroFrame *frame, int width, int height);

void schro_gpuframe_iwt_transform (SchroFrame *frame, SchroParams *params);
void schro_gpuframe_inverse_iwt_transform (SchroFrame *frame, SchroParams *params);

//void schro_gpuframe_downsample (SchroFrame *dest, SchroFrame *src);
//void schro_gpuframe_upsample_horiz (SchroFrame *dest, SchroFrame *src);
//void schro_gpuframe_upsample_vert (SchroFrame *dest, SchroFrame *src);
//int schro_gpuframe_calculate_average_luma (SchroFrame *frame);

//SchroFrame * schro_gpuframe_convert_to_444 (SchroFrame *frame);
void schro_gpuframe_compare (SchroFrame *a, SchroFrame *b);

void schro_gpuframe_upsample(SchroFrame *dst, SchroFrame *src);

SchroUpsampledFrame *schro_upsampled_gpuframe_new(SchroVideoFormat *fmt);
void schro_upsampled_gpuframe_upsample(SchroUpsampledFrame *rv, SchroFrame *temp_f, SchroFrame *src, SchroVideoFormat *fmt);
void schro_upsampled_gpuframe_free(SchroUpsampledFrame *x);


void schro_gpuframe_zero (SchroFrame *dest);

/// Special frame for transferring to GPU
SchroFrame *schro_frame_new_and_alloc_locked (SchroFrameFormat format, int width, int height);

SCHRO_END_DECLS

#endif

