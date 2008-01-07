
#ifndef __SCHRO_PICTURE_H__
#define __SCHRO_PICTURE_H__

#include <schroedinger/schrobuffer.h>
#include <schroedinger/schroparams.h>
#include <schroedinger/schroframe.h>
#include <schroedinger/schromotion.h>
#include <schroedinger/schrounpack.h>
#include <schroedinger/schrobitstream.h>
#include <schroedinger/schroqueue.h>

#ifdef SCHRO_GPU
#include <schroedinger/schrogpuframe.h>
#include <schroedinger/schrogpusubbandstorage.h>
#include <schroedinger/schrogpumotion.h>
#endif

SCHRO_BEGIN_DECLS

typedef struct _SchroDecoder SchroDecoder;
typedef struct _SchroPicture SchroPicture;

#ifdef SCHRO_ENABLE_UNSTABLE_API

/** 
  State bits. Bits in curstate signify which parts
  of the decoding process have been completed, and bits in 
  busystate signify which are currently in progress.
  
  SCHRO_DECODER_MOTION_RENDER/SCHRO_DECODER_WAVELET_TRANSFORM
    can be executed in arbitrary order, even in parallel.
  
  Would it be possible to do the same for SCHRO_DECODER_MOTION_DECODE/
  SCHRO_DECODER_WAVELET_DECODE ? (are the offsets known in advance?)
 */
typedef enum {
  /* Newly allocated frame buffer.
   */
  SCHRO_DECODER_EMPTY = 0x00, 
  /* First access unit has been there.
   */
  SCHRO_DECODER_HAVE_ACCESS_UNIT = 0x01, 
  /* Initialized frame buffer, and allocated memory for buffers.
   */
  SCHRO_DECODER_INITIALIZED = 0x02, 
  /* Input buffer loaded, stream decoding can start.
   */
  SCHRO_DECODER_START = 0x04, 
  /* Do some initialisation based on desired output format. 
     GPU: this must be executed in the GPU thread.
   */
  SCHRO_DECODER_OUTPUT_INIT = 0x08,
  /* Decode motion params.
   */
  SCHRO_DECODER_MOTION_DECODE_PARAMS = 0x10, 
  /* Initialize motion (GPU only)
   */
  SCHRO_DECODER_MOTION_INIT = 0x20, 
  /* Decode motion vectors.
   */
  SCHRO_DECODER_MOTION_DECODE_VECTORS = 0x40, 
  /* Decode wavelet parameters.
   */
  SCHRO_DECODER_WAVELET_DECODE_PARAMS = 0x80, 
  /* Initialize wavelet (GPU only)
   */
  SCHRO_DECODER_WAVELET_INIT = 0x100, 
  /* Decode wavelet data.
     The input buffer is released after this.
   */
  SCHRO_DECODER_WAVELET_DECODE_IMAGE = 0x200, 
  /* Render motion transform.
     GPU: this must be executed in the GPU thread.
   */
  SCHRO_DECODER_MOTION_RENDER = 0x400,
  /* Inverse wavelet transform.
     GPU: this must be executed in the GPU thread.
   */
  SCHRO_DECODER_WAVELET_TRANSFORM = 0x800,
  /* The frame is finished up.
     GPU: this must be executed in the GPU thread.
   */
  SCHRO_DECODER_FINISHED = 0x1000,
  /* Final and initial state for a frame */
  SCHRO_DECODER_INITIAL = 0,
  SCHRO_DECODER_FINAL = SCHRO_DECODER_FINISHED
} SchroDecoderFrameState;

struct _SchroDecoderOp
{
  int state;
  int reqstate;
  int (*check)(SchroPicture *decoder);
  void (*exec)(SchroPicture *decoder);
  int gpu;
};
typedef struct _SchroDecoderOp SchroDecoderOp;

#endif

enum {
  SCHRO_DECODER_OK,
  SCHRO_DECODER_ERROR,
  SCHRO_DECODER_EOS,
  SCHRO_DECODER_FIRST_ACCESS_UNIT,
  SCHRO_DECODER_NEED_BITS,
  SCHRO_DECODER_NEED_FRAME
};

SchroPicture * schro_picture_new (SchroDecoder *decoder);
void schro_picture_free (SchroPicture *decoder);

int schro_decoder_is_parse_header (SchroBuffer *buffer);
int schro_decoder_is_access_unit (SchroBuffer *buffer);
int schro_decoder_is_intra (SchroBuffer *buffer);
int schro_decoder_is_picture (SchroBuffer *buffer);
int schro_decoder_is_end_sequence (SchroBuffer *buffer);

#ifdef SCHRO_ENABLE_UNSTABLE_API
SchroDecoderOp *schro_get_decoder_ops();
void schro_decoder_async_transfer(SchroPicture *decoder);

void schro_decoder_decode_parse_header (SchroDecoder *decoder, SchroUnpack *unpack);
void schro_decoder_decode_access_unit (SchroDecoder *decoder, SchroUnpack *unpack);
void schro_decoder_decode_picture_header (SchroDecoder *decoder, SchroUnpack *unpack);

void schro_decoder_parse_transform_data (SchroPicture *decoder);

void schro_decoder_decode_picture_prediction_parameters (SchroPicture *decoder);
void schro_decoder_decode_block_data (SchroPicture *decoder);
void schro_decoder_decode_transform_parameters (SchroPicture *decoder);
void schro_decoder_decode_transform_data (SchroPicture *decoder);
void schro_decoder_decode_lowdelay_transform_data (SchroPicture *decoder);
void schro_decoder_iwt_transform (SchroPicture *decoder, int component);
void schro_decoder_copy_from_frame_buffer (SchroPicture *decoder, SchroBuffer *buffer);

void schro_decoder_subband_dc_predict (SchroFrameData *fd);

void schro_decoder_decode_lowdelay_transform_data_2 (SchroPicture *decoder);

#endif

SCHRO_END_DECLS

#endif

