
#ifndef __SCHRO_GPUFRAME_H__
#define __SCHRO_GPUFRAME_H__

#include <schroedinger/schro-stdint.h>
#include <schroedinger/schroframe.h>

SCHRO_BEGIN_DECLS

typedef SchroFrame SchroGPUFrame;
typedef SchroFrameData SchroGPUFrameComponent;
typedef SchroUpsampledFrame SchroUpsampledGPUFrame;

typedef void (*SchroGPUFrameFreeFunc)(SchroFrame *frame, void *priv);


SchroGPUFrame * schro_gpuframe_new (void);
SchroGPUFrame * schro_gpuframe_new_and_alloc (SchroFrameFormat format, int width, int height);
SchroGPUFrame * schro_gpuframe_new_from_data_I420 (void *data, int width, int height);
SchroGPUFrame * schro_gpuframe_new_from_data_YUY2 (void *data, int width, int height);
SchroGPUFrame * schro_gpuframe_new_from_data_AYUV (void *data, int width, int height);
SchroGPUFrame * schro_gpuframe_new_clone (SchroFrame *src);

void schro_gpuframe_setstream(SchroGPUFrame *frame, SchroCUDAStream stream);

void schro_gpuframe_to_cpu (SchroFrame *dest, SchroGPUFrame *src);
void schro_frame_to_gpu (SchroGPUFrame *dest, SchroFrame *src);

void schro_gpuframe_set_free_callback (SchroGPUFrame *frame, SchroGPUFrameFreeFunc free_func, void *priv);
void schro_gpuframe_unref (SchroGPUFrame *frame);
SchroGPUFrame *schro_gpuframe_ref (SchroGPUFrame *frame);
void schro_gpuframe_convert (SchroGPUFrame *dest, SchroGPUFrame *src);
void schro_gpuframe_add (SchroGPUFrame *dest, SchroGPUFrame *src);
void schro_gpuframe_subtract (SchroGPUFrame *dest, SchroGPUFrame *src);
//void schro_gpuframe_shift_left (SchroGPUFrame *frame, int shift);
//void schro_gpuframe_shift_right (SchroGPUFrame *frame, int shift);
//void schro_gpuframe_edge_extend (SchroGPUFrame *frame, int width, int height);
//void schro_gpuframe_zero_extend (SchroGPUFrame *frame, int width, int height);

void schro_gpuframe_iwt_transform (SchroGPUFrame *frame, SchroParams *params);
void schro_gpuframe_inverse_iwt_transform (SchroGPUFrame *frame, SchroParams *params);

//void schro_gpuframe_downsample (SchroGPUFrame *dest, SchroGPUFrame *src);
//void schro_gpuframe_upsample_horiz (SchroGPUFrame *dest, SchroGPUFrame *src);
//void schro_gpuframe_upsample_vert (SchroGPUFrame *dest, SchroGPUFrame *src);
//int schro_gpuframe_calculate_average_luma (SchroGPUFrame *frame);

//SchroGPUFrame * schro_gpuframe_convert_to_444 (SchroGPUFrame *frame);
void schro_gpuframe_compare (SchroGPUFrame *a, SchroFrame *b);

void schro_gpuframe_upsample(SchroGPUFrame *dst, SchroGPUFrame *src);

SchroUpsampledGPUFrame *schro_upsampled_gpuframe_new(SchroVideoFormat *fmt);
void schro_upsampled_gpuframe_upsample(SchroUpsampledGPUFrame *rv, SchroGPUFrame *temp_f, SchroGPUFrame *src, SchroVideoFormat *fmt);
void schro_upsampled_gpuframe_free(SchroUpsampledGPUFrame *x);


void schro_gpuframe_zero (SchroGPUFrame *dest);

/// Special frame for transferring to GPU
SchroFrame *schro_frame_new_and_alloc_locked (SchroFrameFormat format, int width, int height);

SCHRO_END_DECLS

#endif

