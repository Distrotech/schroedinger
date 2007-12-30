
#ifndef __SCHRO_DECODERWORKER_H__
#define __SCHRO_DECODERWORKER_H__

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
typedef struct _SchroDecoderWorker SchroDecoderWorker;

struct _SchroDecoderSettings 
{
  int major_version;
  int minor_version;
  int profile;
  int level;
  schro_bool interlaced_coding;
  SchroVideoFormat video_format;
};

typedef struct _SchroDecoderSettings SchroDecoderSettings;

struct _SchroDecoderParseHeader
{
  int parse_code;
  int n_refs;
  int next_parse_offset;
  int prev_parse_offset;
};
typedef struct _SchroDecoderParseHeader SchroDecoderParseHeader;

struct _SchroDecoderPictureHeader
{
  SchroPictureNumber picture_number;
  SchroPictureNumber reference1;
  SchroPictureNumber reference2;
  SchroPictureNumber retired_picture_number;
};
typedef struct _SchroDecoderPictureHeader SchroDecoderPictureHeader;

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
  int (*check)(SchroDecoderWorker *decoder);
  void (*exec)(SchroDecoderWorker *decoder);
  int gpu;
};
typedef struct _SchroDecoderOp SchroDecoderOp;

struct _SchroDecoderWorker {
  /* operations completed on this frame */
  int curstate;
  /* operations in progress */
  int busystate;
  /* persistent state, will be kept for next frame */
  int skipstate;
  
  /* Passed in from parent object */
  int time;
  SchroDecoder *parent;
  SchroUnpack unpack;
  SchroDecoderSettings settings;
  SchroDecoderParseHeader header;
  SchroDecoderPictureHeader pichdr;

  int has_md5;
  uint8_t md5_checksum[32];

  SchroBuffer *input_buffer;
  SchroFrame *output_picture;
  
#ifdef SCHRO_GPU
  SchroStream stream; /* CUDA stream handle */
  int subband_min; /* last band+1 that was transferred to the GPU (updated only by GPU thread) */
  int subband_max; /* last band+1 that was decoded (updated only by CPU threads) */
#endif
  
  /*< private >*/
#ifndef SCHRO_GPU
  SchroFrame *frame;
  SchroFrame *mc_tmp_frame;
  SchroFrame *planar_output_frame;
#else
  SchroGPUFrame *frame;
  SchroGPUFrame *mc_tmp_frame;
#endif

#ifndef SCHRO_GPU
  SchroUpsampledFrame *ref0;
  SchroUpsampledFrame *ref1;
#else
  SchroUpsampledGPUFrame *ref0;
  SchroUpsampledGPUFrame *ref1;
#endif

  int16_t *tmpbuf;
  int16_t *tmpbuf2;

  SchroParams params;

  SchroMotion *motion;

  int zero_residual;

  int error;
  char *error_message;

#ifdef SCHRO_GPU
  /// Output frame on GPU
  
  SchroGPUFrame *goutput_frame;

  schro_subband_storage *store;  

  SchroGPUMotion *gpumotion;
#endif

};
#endif

enum {
  SCHRO_DECODER_OK,
  SCHRO_DECODER_ERROR,
  SCHRO_DECODER_EOS,
  SCHRO_DECODER_FIRST_ACCESS_UNIT,
  SCHRO_DECODER_NEED_BITS,
  SCHRO_DECODER_NEED_FRAME
};

SchroDecoderWorker * schro_decoderworker_new (void);
void schro_decoderworker_free (SchroDecoderWorker *decoder);

int schro_decoder_is_parse_header (SchroBuffer *buffer);
int schro_decoder_is_access_unit (SchroBuffer *buffer);
int schro_decoder_is_intra (SchroBuffer *buffer);
int schro_decoder_is_picture (SchroBuffer *buffer);
int schro_decoder_is_end_sequence (SchroBuffer *buffer);

#ifdef SCHRO_ENABLE_UNSTABLE_API
SchroDecoderOp *schro_get_decoder_ops();
void schro_decoder_async_transfer(SchroDecoderWorker *decoder);

void schro_decoder_decode_parse_header (SchroDecoderParseHeader *hdr, SchroUnpack *unpack);
void schro_decoder_decode_access_unit (SchroDecoderSettings *hdr, SchroUnpack *unpack);
void schro_decoder_decode_picture_header (SchroDecoderPictureHeader *hdr, SchroUnpack *unpack, SchroDecoderParseHeader *phdr);

void schro_decoder_decode_picture_prediction_parameters (SchroDecoderWorker *decoder);
void schro_decoder_decode_block_data (SchroDecoderWorker *decoder);
void schro_decoder_decode_transform_parameters (SchroDecoderWorker *decoder);
void schro_decoder_decode_transform_data (SchroDecoderWorker *decoder);
void schro_decoder_decode_lowdelay_transform_data (SchroDecoderWorker *decoder);
void schro_decoder_iwt_transform (SchroDecoderWorker *decoder, int component);
void schro_decoder_copy_from_frame_buffer (SchroDecoderWorker *decoder, SchroBuffer *buffer);

void schro_decoder_subband_dc_predict (SchroFrameData *fd);

void schro_decoder_decode_lowdelay_transform_data_2 (SchroDecoderWorker *decoder);

#endif

SCHRO_END_DECLS

#endif

