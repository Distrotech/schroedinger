
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
#include <schrogpuframe.h>
#include <schrogpusubbandstorage.h>
#include <schrogpumotion.h>
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

#define UQUEUE_SIZE 4 /* Max number of cached ref frames */

typedef enum {
  SCHRO_DECODER_FREE, /* Free for use */
  SCHRO_DECODER_PREPARING, /* Preparing for use */
  SCHRO_DECODER_BUSY, /* CPU work in progress */
  SCHRO_DECODER_QUIT, /* Quit this thread */
} SchroDecoderState;

struct _SchroDecoderWorker {
  SchroDecoderState state;
  /* Data received at GPU thread */
  int wait_motion; /* waiting for motion compensation data */
  int wait_subbands; /* waiting for subbands */
  int wait_ref; /* waiting for reference frames */

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
  
  /*< private >*/
#ifndef SCHRO_GPU
  SchroFrame *frame;
  SchroFrame *mc_tmp_frame;
  SchroFrame *planar_output_frame;
#else
  SchroGPUFrame *frame;
  SchroGPUFrame *mc_tmp_frame;
  SchroGPUFrame *planar_output_frame;
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
  
  SchroGPUFrame *gplanar_output_frame;
  SchroGPUFrame *goutput_frame;
  SchroGPUFrame *gupsample_temp;

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

void schro_decoderworker_init (SchroDecoderWorker *decoder);

void schro_decoderworker_iterate (SchroDecoderWorker *decoder);

int schro_decoder_is_parse_header (SchroBuffer *buffer);
int schro_decoder_is_access_unit (SchroBuffer *buffer);
int schro_decoder_is_intra (SchroBuffer *buffer);
int schro_decoder_is_picture (SchroBuffer *buffer);
int schro_decoder_is_end_sequence (SchroBuffer *buffer);

#ifdef SCHRO_ENABLE_UNSTABLE_API

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

