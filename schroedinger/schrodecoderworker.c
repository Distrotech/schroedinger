
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#define SCHRO_ARITH_DEFINE_INLINE
#include <schroedinger/schro.h>
#include <liboil/liboil.h>
#include <schroedinger/schrooil.h>
#include <string.h>
#include <stdio.h>

#ifdef SCHRO_GPU
#include <cuda.h>
#include <cuda_runtime_api.h>
#endif

#if 0
/* Used for testing bitstream */
#define MARKER() do{ \
  SCHRO_ASSERT(schro_unpack_decode_uint(&decoder->unpack) == 1234567); \
} while(0)
#else
#define MARKER()
#endif

#define SCHRO_SKIP_TIME_CONSTANT 0.1

#ifdef SCHRO_GPU
// Don't copy frame out
// #define GPU_NOCOPY_OUT

// Load balancing tests -- corrupts output
#define DEBUGGPU


/// Assume subbands and blocks are zero, to prevent expensive filling operations
#define ASSUME_ZERO
#endif

typedef struct _SchroDecoderSubbandContext SchroDecoderSubbandContext;

struct _SchroDecoderSubbandContext {
  int component;
  int position;

  int16_t *data;
  int height;
  int width;
  int stride;

  int16_t *parent_data;
  int parent_stride;
  int parent_width;
  int parent_height;

  int quant_index;
  int subband_length;
  SchroArith *arith;
  SchroUnpack unpack;
  int vert_codeblocks;
  int horiz_codeblocks;
  int have_zero_flags;
  int have_quant_offset;

  int ymin;
  int ymax;
  int xmin;
  int xmax;

  int quant_factor;
  int quant_offset;
  int16_t *line;
};

int _schro_decode_prediction_only;


    
static void schro_decoder_decode_macroblock(SchroDecoderWorker *decoder,
    SchroArith **arith, SchroUnpack *unpack, int i, int j);
static void schro_decoder_decode_prediction_unit(SchroDecoderWorker *decoder,
    SchroArith **arith, SchroUnpack *unpack, SchroMotionVector *motion_vectors, int x, int y);

#ifdef SCHRO_GPU
static void schro_decoder_decode_transform_data_serial (SchroDecoderWorker *decoder, schro_subband_storage *store, SchroGPUFrame *frame);
#endif

static int schro_decoder_decode_subband (SchroDecoderWorker *decoder,
    SchroDecoderSubbandContext *ctx);

#ifndef ASSUME_ZERO
static void schro_decoder_zero_block (SchroDecoderSubbandContext *ctx, int x1, int y1, int x2, int y2);
#endif

static void schro_decoder_error (SchroDecoderWorker *decoder, const char *s);


SchroDecoderWorker *
schro_decoderworker_new (void)
{
  SchroDecoderWorker *decoder;

  decoder = malloc(sizeof(SchroDecoder));
  memset (decoder, 0, sizeof(SchroDecoder));

  decoder->tmpbuf = malloc(SCHRO_LIMIT_WIDTH * 2);
  decoder->tmpbuf2 = malloc(SCHRO_LIMIT_WIDTH * 2);

  decoder->params.video_format = &decoder->settings.video_format;


  return decoder;
}

#ifdef SCHRO_GPU
#define schro_frame_unref schro_gpuframe_unref
#endif

void
schro_decoderworker_free (SchroDecoderWorker *decoder)
{
  if (decoder->frame) {
    schro_frame_unref (decoder->frame);
  }

  if (decoder->mc_tmp_frame) schro_frame_unref (decoder->mc_tmp_frame);
#ifndef SCHRO_GPU
  if (decoder->planar_output_frame) schro_frame_unref (decoder->planar_output_frame);
#endif
  if (decoder->tmpbuf) free (decoder->tmpbuf);
  if (decoder->tmpbuf2) free (decoder->tmpbuf2);
  if (decoder->error_message) free (decoder->error_message);
  
#ifdef SCHRO_GPU
  if (decoder->goutput_frame) schro_gpuframe_unref (decoder->goutput_frame);
  if (decoder->store) schro_subband_storage_free(decoder->store);
  schro_gpumotion_free(decoder->gpumotion);
  cudaStreamDestroy(decoder->stream);
#endif

  free (decoder);
}

#undef schro_frame_unref

int
schro_decoder_is_intra (SchroBuffer *buffer)
{
  uint8_t *data;

  if (buffer->length < 5) return 0;

  data = buffer->data;
  if (data[0] != 'B' || data[1] != 'B' || data[2] != 'C' || data[3] != 'D') {
    return 0;
  }

  if (SCHRO_PARSE_CODE_NUM_REFS(data[4] == 0)) return 1;

  return 1;
}

int
schro_decoder_is_parse_header (SchroBuffer *buffer)
{
  uint8_t *data;

  if (buffer->length < 5) return 0;

  data = buffer->data;
  if (data[0] != 'B' || data[1] != 'B' || data[2] != 'C' || data[3] != 'D') {
    return 0;
  }

  return 1;
}

int
schro_decoder_is_access_unit (SchroBuffer *buffer)
{
  uint8_t *data;

  if (buffer->length < 5) return 0;

  data = buffer->data;
  if (data[0] != 'B' || data[1] != 'B' || data[2] != 'C' || data[3] != 'D') {
    return 0;
  }

  if (data[4] == SCHRO_PARSE_CODE_SEQUENCE_HEADER) return 1;

  return 0;
}

int
schro_decoder_is_picture (SchroBuffer *buffer)
{
  uint8_t *data;

  if (buffer->length < 5) return 0;

  data = buffer->data;
  if (data[0] != 'B' || data[1] != 'B' || data[2] != 'C' || data[3] != 'D') {
    return 0;
  }

  if (SCHRO_PARSE_CODE_IS_PICTURE(data[4])) return 1;

  return 0;
}

int
schro_decoder_is_end_sequence (SchroBuffer *buffer)
{
  uint8_t *data;

  if (buffer->length < 5) return 0;

  data = buffer->data;
  if (data[0] != 'B' || data[1] != 'B' || data[2] != 'C' || data[3] != 'D') {
    return 0;
  }

  if (data[4] == SCHRO_PARSE_CODE_END_OF_SEQUENCE) return 1;

  return 0;
}

#ifdef SCHRO_GPU
#define schro_frame_convert schro_gpuframe_convert
#define schro_frame_add schro_gpuframe_add
#define schro_frame_new_and_alloc schro_gpuframe_new_and_alloc
#endif

/* This must be executed in the GPU thread 
*/
static void
schro_decoderworker_init (SchroDecoderWorker *decoder)
{
  SchroFrameFormat frame_format;
  SchroVideoFormat *video_format = &decoder->settings.video_format;
  int frame_width, frame_height;

  frame_format = schro_params_get_frame_format (16,
      video_format->chroma_format);
  frame_width = ROUND_UP_POW2(video_format->width,
      SCHRO_LIMIT_TRANSFORM_DEPTH + video_format->chroma_h_shift);
  frame_height = ROUND_UP_POW2(video_format->height,
      SCHRO_LIMIT_TRANSFORM_DEPTH + video_format->chroma_v_shift);
  decoder->mc_tmp_frame = schro_frame_new_and_alloc (frame_format,
      frame_width, frame_height);
  decoder->frame = schro_frame_new_and_alloc (frame_format,
      frame_width, frame_height);
  frame_format = schro_params_get_frame_format (8,
      video_format->chroma_format);

#ifdef SCHRO_GPU
  /** We can use the fact that there is only one GPU thread to conserve
      memory, and allocate temporary structures only once.
   */
  if(!decoder->parent->planar_output_frame)
  {
    decoder->parent->planar_output_frame = schro_frame_new_and_alloc (frame_format,
      video_format->width, video_format->height);
    SCHRO_DEBUG("planar output frame %dx%d",
      video_format->width, video_format->height);

    decoder->parent->gupsample_temp = schro_gpuframe_new_and_alloc (frame_format,
        video_format->width*2, video_format->height*2);
  }
#else
  decoder->planar_output_frame = schro_frame_new_and_alloc (frame_format,
      video_format->width, video_format->height);
  SCHRO_DEBUG("planar output frame %dx%d",
      video_format->width, video_format->height);
#endif
#ifdef SCHRO_GPU
  cudaStreamCreate(&decoder->stream);
  decoder->gpumotion = schro_gpumotion_new(decoder->stream);
  
  schro_gpuframe_setstream(decoder->mc_tmp_frame, decoder->stream);
  schro_gpuframe_setstream(decoder->frame, decoder->stream);
#endif
  schro_decoder_skipstate(decoder->parent, decoder, SCHRO_DECODER_INITIALIZED);
}


#ifdef SCHRO_GPU
static void
schro_decoderworker_iterate_init_output (SchroDecoderWorker *decoder)
{
  /* GPU clone output frame. This must be here as we cannot know in advance
     what frame format (interleaved, planar, bit depth) the user expects. 
   */
  if(!decoder->goutput_frame)
  {
      decoder->goutput_frame = schro_gpuframe_new_clone(decoder->output_picture);
      schro_gpuframe_setstream(decoder->goutput_frame, decoder->stream);
  }
  schro_decoder_skipstate(decoder->parent, decoder, SCHRO_DECODER_OUTPUT_INIT);
}
#endif

static void
schro_decoderworker_iterate_motion_decode_params (SchroDecoderWorker *decoder)
{
  SchroParams *params = &decoder->params;

  /* General worker state initialisation */
  decoder->ref0 = NULL;
  decoder->ref1 = NULL;
  params->num_refs = SCHRO_PARSE_CODE_NUM_REFS(decoder->header.parse_code);
  params->is_lowdelay = SCHRO_PARSE_CODE_IS_LOW_DELAY(decoder->header.parse_code);
  params->is_noarith = !SCHRO_PARSE_CODE_USING_AC(decoder->header.parse_code);

  /* Check for motion comp data */
  if (decoder->header.n_refs > 0) {

    SCHRO_DEBUG("inter");

    schro_unpack_byte_sync (&decoder->unpack);
    schro_decoder_decode_picture_prediction_parameters (decoder);
    schro_params_calculate_mc_sizes (params);

    decoder->motion = schro_motion_new (params, NULL, NULL);
  }
}

#ifdef SCHRO_GPU
static void
schro_decoderworker_iterate_motion_init (SchroDecoderWorker *decoder)
{
  if (decoder->header.n_refs > 0)
  {
    schro_gpumotion_init (decoder->gpumotion, decoder->motion);
    schro_decoder_skipstate(decoder->parent, decoder, SCHRO_DECODER_MOTION_INIT);
  }
}
#endif

static void
schro_decoderworker_iterate_motion_decode_vectors (SchroDecoderWorker *decoder)
{
  if (decoder->header.n_refs > 0) {
    schro_unpack_byte_sync (&decoder->unpack);
    schro_decoder_decode_block_data (decoder);
#ifdef SCHRO_GPU
    schro_gpumotion_copy (decoder->gpumotion, decoder->motion);
#endif
  }
}

static void
schro_decoderworker_iterate_wavelet_decode_params (SchroDecoderWorker *decoder)  
{
  SchroParams *params = &decoder->params;
  
  /* Decode transform data */
  schro_unpack_byte_sync (&decoder->unpack);
  decoder->zero_residual = FALSE;
  if (params->num_refs > 0) {
    decoder->zero_residual = schro_unpack_decode_bit (&decoder->unpack);

    SCHRO_DEBUG ("zero residual %d", decoder->zero_residual);
  }

  if (!decoder->zero_residual) {
    schro_decoder_decode_transform_parameters (decoder);
    schro_params_calculate_iwt_sizes (params);
  }
}

#ifdef SCHRO_GPU
void
schro_decoderworker_iterate_wavelet_init (SchroDecoderWorker *decoder)
{
  if (!decoder->zero_residual) {
    if(!decoder->store)
        decoder->store = schro_subband_storage_new(&decoder->params, decoder->stream);
    schro_decoder_skipstate(decoder->parent, decoder, SCHRO_DECODER_WAVELET_INIT);
  }
}
#endif         

static void
schro_decoderworker_iterate_wavelet_decode_image (SchroDecoderWorker *decoder)
{
  SchroParams *params = &decoder->params;
  if (!decoder->zero_residual) {
    schro_unpack_byte_sync (&decoder->unpack);
#ifdef SCHRO_GPU
    SCHRO_ASSERT(!params->is_lowdelay);
    schro_decoder_decode_transform_data_serial(decoder, decoder->store, decoder->frame);
#else
    if (params->is_lowdelay) {
      schro_decoder_decode_lowdelay_transform_data (decoder);
    } else {
      schro_decoder_decode_transform_data (decoder);
    }
#endif
  }

  /* Input buffer can be released now stream decoding is complete. */
  schro_buffer_unref (decoder->input_buffer);
  decoder->input_buffer = NULL;
}



static void
schro_decoderworker_iterate_wavelet_transform (SchroDecoderWorker *decoder)
{
  if (!decoder->zero_residual) {
#ifdef SCHRO_GPU
    SCHRO_ASSERT(decoder->subband_min == decoder->subband_max);
    SCHRO_ASSERT(decoder->subband_min == 3*(1+3*decoder->params.transform_depth));
    schro_gpuframe_inverse_iwt_transform (decoder->frame, &decoder->params);
#else
    schro_frame_inverse_iwt_transform (decoder->frame, &decoder->params,
        decoder->tmpbuf);
#endif

  }
}

static int 
schro_decoderworker_check_refs (SchroDecoderWorker *decoder)
{
  int rv;

  /** Find reference frames, if we didn't yet. We should check for frames
      that are "stuck" due to their reference frames never appearing. 
   */
  if(decoder->header.n_refs > 0 && !decoder->ref0)
    decoder->ref0 = schro_decoder_reference_get (decoder->parent, decoder->pichdr.reference1);
  if(decoder->header.n_refs > 1 && !decoder->ref1)
    decoder->ref1 = schro_decoder_reference_get (decoder->parent, decoder->pichdr.reference2);

  /** Count number of available reference frames, and compare
      to what we need. 
   */
  rv = (decoder->ref0 != NULL) + (decoder->ref1 != NULL);
  return rv == decoder->header.n_refs;
}

static void
schro_decoderworker_iterate_motion_transform (SchroDecoderWorker *decoder)
{
  if (decoder->header.n_refs > 0) {
#if 0
    /* Moved this to finish frame stage because of race conditions.
       It should really be here, though. */
    if (params->mv_precision > 0) {
      schro_upsampled_frame_upsample (decoder->ref0);
      if (decoder->ref1) {
        schro_upsampled_frame_upsample (decoder->ref1);
      }
    }
#endif
    decoder->motion->src1 = (SchroUpsampledFrame*)decoder->ref0;
    decoder->motion->src2 = (SchroUpsampledFrame*)decoder->ref1;

#ifndef SCHRO_GPU
    schro_motion_render (decoder->motion, decoder->mc_tmp_frame);
#else
    schro_gpumotion_render (decoder->gpumotion, decoder->motion, decoder->mc_tmp_frame);
#endif

    schro_motion_free (decoder->motion);  
    decoder->motion = NULL;
  }
}

#ifdef SCHRO_GPU
#define planar_output_frame parent->planar_output_frame
#endif

static void
schro_decoderworker_iterate_finish (SchroDecoderWorker *decoder)
{
#ifdef SCHRO_GPU
  SchroFrame *cpu_output_picture = decoder->output_picture;
  SchroGPUFrame *output_picture = decoder->goutput_frame;
  
  schro_gpuframe_setstream(decoder->planar_output_frame, decoder->stream);
#else
  SchroParams *params = &decoder->params;
  SchroFrame *output_picture = decoder->output_picture;
  int i;
#endif
  if (decoder->zero_residual) {
    if (SCHRO_FRAME_IS_PACKED(output_picture->format)) {
      schro_frame_convert (decoder->planar_output_frame, decoder->mc_tmp_frame);
      schro_frame_convert (output_picture, decoder->planar_output_frame);
    } else {
      schro_frame_convert (output_picture, decoder->mc_tmp_frame);
    }
  } else if (!_schro_decode_prediction_only) {
    if (SCHRO_PARSE_CODE_IS_INTER(decoder->header.parse_code)) {
      schro_frame_add (decoder->frame, decoder->mc_tmp_frame);
    }

    if (SCHRO_FRAME_IS_PACKED(output_picture->format)) {
      schro_frame_convert (decoder->planar_output_frame, decoder->frame);
      schro_frame_convert (output_picture, decoder->planar_output_frame);
    } else {
      schro_frame_convert (output_picture, decoder->frame);
    }
  } else {
#ifndef SCHRO_GPU
    SchroFrame *frame;
#else
    SchroGPUFrame *frame;
#endif
    if (SCHRO_PARSE_CODE_IS_INTER(decoder->header.parse_code)) {
      frame = decoder->mc_tmp_frame;
    } else {
      frame = decoder->frame;
    }
    if (SCHRO_FRAME_IS_PACKED(output_picture->format)) {
      schro_frame_convert (decoder->planar_output_frame, frame);
      schro_frame_convert (output_picture, decoder->planar_output_frame);
    } else {
      schro_frame_convert (output_picture, frame);
    }

    if (SCHRO_PARSE_CODE_IS_INTER(decoder->header.parse_code)) {
      schro_frame_add (decoder->frame, decoder->mc_tmp_frame);
    }
  }

  output_picture->frame_number = decoder->pichdr.picture_number;

  if (SCHRO_PARSE_CODE_IS_REFERENCE(decoder->header.parse_code)) {
#ifndef SCHRO_GPU
    SchroFrame *ref;
    SchroFrameFormat frame_format;
    SchroUpsampledFrame *upsampler;

    switch (params->video_format->chroma_format) {
      case SCHRO_CHROMA_420:
        frame_format = SCHRO_FRAME_FORMAT_U8_420;
        break;
      case SCHRO_CHROMA_422:
        frame_format = SCHRO_FRAME_FORMAT_U8_422;
        break;
      case SCHRO_CHROMA_444:
        frame_format = SCHRO_FRAME_FORMAT_U8_444;
        break;
      default:
        SCHRO_ASSERT(0);
    }

    ref = schro_frame_new_and_alloc (frame_format,
        decoder->settings.video_format.width, decoder->settings.video_format.height);
    schro_frame_convert (ref, decoder->frame);
    ref->frame_number = decoder->pichdr.picture_number;
    
    upsampler = schro_upsampled_frame_new(ref);
    
    /* This upsampling should ideally not happen here, but only on
       demand. */
    schro_upsampled_frame_upsample (upsampler);
    
    schro_decoder_reference_add (decoder->parent, upsampler, decoder->pichdr.picture_number);
#else
    SchroGPUFrame *ref;
    SchroUpsampledGPUFrame *rv;
    if(output_picture->format == decoder->planar_output_frame->format)
    {
        /* We can skip an extra conversion if we have the output already in the format
           we want
         */
        ref = output_picture;
    } else {
        ref = decoder->planar_output_frame;
        schro_gpuframe_convert (decoder->planar_output_frame, decoder->frame);
    }
    /* Try to re-use a frame from freestack */
    rv = schro_decoder_reference_getfree(decoder->parent);
    if(rv == NULL)
        rv = schro_upsampled_gpuframe_new(&decoder->settings.video_format);
    
    schro_gpuframe_setstream(decoder->parent->gupsample_temp, decoder->stream);
    schro_upsampled_gpuframe_upsample(rv, decoder->parent->gupsample_temp, ref, &decoder->settings.video_format);
    
    schro_decoder_reference_add (decoder->parent, rv, decoder->pichdr.picture_number);
#endif
  }


#ifndef SCHRO_GPU
  if (decoder->has_md5) {
    uint32_t state[4];

    schro_frame_md5 (output_picture, state);
    if (memcmp (state, decoder->md5_checksum, 16) != 0) {
      char a[65];
      char b[65];
      for(i=0;i<16;i++){
        sprintf(a+2*i, "%02x", ((uint8_t *)state)[i]);
        sprintf(b+2*i, "%02x", decoder->md5_checksum[i]);
      }
      SCHRO_ERROR("MD5 checksum mismatch (%s should be %s)", a, b);
    }

    decoder->has_md5 = FALSE;
  }
#endif

  SCHRO_DEBUG("adding %d to queue", output_picture->frame_number);
#ifndef SCHRO_GPU
  schro_decoder_add_finished_frame (decoder->parent, output_picture);
#else
#ifndef GPU_NOCOPY_OUT
  schro_gpuframe_to_cpu(cpu_output_picture, output_picture);
#else
  cpu_output_picture->frame_number = output_picture->frame_number;
#endif
  schro_decoder_add_finished_frame (decoder->parent, cpu_output_picture);
#endif
}
#undef planar_output_frame
/**
 The "script", this describes the state transitions for the 
 NFA of the decoding process of a frame.

 state is the state bit to set after this operation
 reqstate is a bitfield signifying which states should be completed
 check is a function that does additional requirement checks for this action
 exec is a function that does the work to achieve this state
 gpu signifies this must be executed in the GPU thread (if in GPU mode)
 
 state, 
 reqstate, check,
 exec,
 [gpu,]
*/
#ifndef SCHRO_GPU
/* CPU version */
static SchroDecoderOp schro_decoder_ops[] = {
/* One-time initialisation */
{SCHRO_DECODER_INITIALIZED,
 SCHRO_DECODER_HAVE_ACCESS_UNIT, NULL,
 schro_decoderworker_init
},
/* Start of frame decoding */
{SCHRO_DECODER_MOTION_DECODE_PARAMS,
 SCHRO_DECODER_START|SCHRO_DECODER_INITIALIZED, NULL, 
 schro_decoderworker_iterate_motion_decode_params, 
},
{SCHRO_DECODER_MOTION_DECODE_VECTORS, 
 SCHRO_DECODER_MOTION_DECODE_PARAMS, NULL, 
 schro_decoderworker_iterate_motion_decode_vectors, 
},
{SCHRO_DECODER_WAVELET_DECODE_PARAMS, 
 SCHRO_DECODER_MOTION_DECODE_VECTORS, NULL, 
 schro_decoderworker_iterate_wavelet_decode_params, 
},
{SCHRO_DECODER_WAVELET_DECODE_IMAGE, 
 SCHRO_DECODER_WAVELET_DECODE_PARAMS, NULL, 
 schro_decoderworker_iterate_wavelet_decode_image, 
},

{SCHRO_DECODER_MOTION_RENDER, 
 SCHRO_DECODER_MOTION_DECODE_VECTORS, schro_decoderworker_check_refs, 
 schro_decoderworker_iterate_motion_transform, 
},
{SCHRO_DECODER_WAVELET_TRANSFORM, 
 SCHRO_DECODER_WAVELET_DECODE_IMAGE, NULL, 
 schro_decoderworker_iterate_wavelet_transform, 
},
{SCHRO_DECODER_FINISHED, 
 SCHRO_DECODER_MOTION_RENDER|SCHRO_DECODER_WAVELET_TRANSFORM, NULL, 
 schro_decoderworker_iterate_finish, 
},
/* End of frame decoding */
{SCHRO_DECODER_EMPTY}
};
#else
/* GPU version */
static SchroDecoderOp schro_decoder_ops[] = {
/* One-time initialisation */
{SCHRO_DECODER_INITIALIZED,
 SCHRO_DECODER_HAVE_ACCESS_UNIT, NULL,
 schro_decoderworker_init,
 TRUE
},
/* Start of frame decoding */
{SCHRO_DECODER_OUTPUT_INIT,
 SCHRO_DECODER_START|SCHRO_DECODER_INITIALIZED, NULL, 
 schro_decoderworker_iterate_init_output, 
 TRUE
},
{SCHRO_DECODER_MOTION_DECODE_PARAMS,
 SCHRO_DECODER_START|SCHRO_DECODER_INITIALIZED, NULL, 
 schro_decoderworker_iterate_motion_decode_params, 
 FALSE
},
{SCHRO_DECODER_MOTION_INIT, 
 SCHRO_DECODER_MOTION_DECODE_PARAMS, NULL, 
 schro_decoderworker_iterate_motion_init, 
 TRUE
},
{SCHRO_DECODER_MOTION_DECODE_VECTORS, 
 SCHRO_DECODER_MOTION_INIT|SCHRO_DECODER_MOTION_DECODE_PARAMS, NULL, 
 schro_decoderworker_iterate_motion_decode_vectors, 
 FALSE
},
{SCHRO_DECODER_WAVELET_DECODE_PARAMS, 
 SCHRO_DECODER_MOTION_DECODE_VECTORS, NULL, 
 schro_decoderworker_iterate_wavelet_decode_params, 
 FALSE
},
{SCHRO_DECODER_WAVELET_INIT, 
 SCHRO_DECODER_WAVELET_DECODE_PARAMS, NULL, 
 schro_decoderworker_iterate_wavelet_init, 
 TRUE
},
{SCHRO_DECODER_WAVELET_DECODE_IMAGE, 
 SCHRO_DECODER_WAVELET_INIT|SCHRO_DECODER_WAVELET_DECODE_PARAMS, NULL, 
 schro_decoderworker_iterate_wavelet_decode_image, 
 FALSE
},

{SCHRO_DECODER_MOTION_RENDER, 
 SCHRO_DECODER_MOTION_DECODE_VECTORS, schro_decoderworker_check_refs, 
 schro_decoderworker_iterate_motion_transform, 
 TRUE
},
{SCHRO_DECODER_WAVELET_TRANSFORM, 
 SCHRO_DECODER_WAVELET_DECODE_IMAGE, NULL, 
 schro_decoderworker_iterate_wavelet_transform, 
 TRUE
},
{SCHRO_DECODER_FINISHED, 
 SCHRO_DECODER_OUTPUT_INIT|SCHRO_DECODER_MOTION_RENDER|SCHRO_DECODER_WAVELET_TRANSFORM, NULL, 
 schro_decoderworker_iterate_finish, 
 TRUE
},
/* End of frame decoding */
{SCHRO_DECODER_EMPTY}
};

#endif


SchroDecoderOp *schro_get_decoder_ops()
{
  return schro_decoder_ops;
}

#undef schro_frame_convert
#undef schro_frame_add
#undef schro_frame_new_and_alloc
void
schro_decoder_decode_parse_header (SchroDecoderParseHeader *hdr, SchroUnpack *unpack)
{
  int v1, v2, v3, v4;
  
  v1 = schro_unpack_decode_bits (unpack, 8);
  v2 = schro_unpack_decode_bits (unpack, 8);
  v3 = schro_unpack_decode_bits (unpack, 8);
  v4 = schro_unpack_decode_bits (unpack, 8);
  SCHRO_DEBUG ("parse header %02x %02x %02x %02x", v1, v2, v3, v4);
  if (v1 != 'B' || v2 != 'B' || v3 != 'C' || v4 != 'D') {
    SCHRO_ERROR ("expected parse header");
    return;
  }

  hdr->parse_code = schro_unpack_decode_bits (unpack, 8);
  SCHRO_DEBUG ("parse code %02x", hdr->parse_code);

  hdr->n_refs = SCHRO_PARSE_CODE_NUM_REFS(hdr->parse_code);
  SCHRO_DEBUG("n_refs %d", hdr->n_refs);

  hdr->next_parse_offset = schro_unpack_decode_bits (unpack, 32);
  SCHRO_DEBUG ("next_parse_offset %d", hdr->next_parse_offset);
  hdr->prev_parse_offset = schro_unpack_decode_bits (unpack, 32);
  SCHRO_DEBUG ("prev_parse_offset %d", hdr->prev_parse_offset);
}

void
schro_decoder_decode_access_unit (SchroDecoderSettings *hdr, SchroUnpack *unpack)
{
  int bit;
  int index;
  SchroVideoFormat *format = &hdr->video_format;

  SCHRO_DEBUG("decoding access unit");

  /* parse parameters */
  hdr->major_version = schro_unpack_decode_uint (unpack);
  SCHRO_DEBUG("major_version = %d", hdr->major_version);
  hdr->minor_version = schro_unpack_decode_uint (unpack);
  SCHRO_DEBUG("minor_version = %d", hdr->minor_version);
  hdr->profile = schro_unpack_decode_uint (unpack);
  SCHRO_DEBUG("profile = %d", hdr->profile);
  hdr->level = schro_unpack_decode_uint (unpack);
  SCHRO_DEBUG("level = %d", hdr->level);

  if (hdr->major_version != 0 || hdr->minor_version != 20071203) {
    SCHRO_ERROR("Expecting version number 0.20071203, got %d.%d",
        hdr->major_version, hdr->minor_version);
    //SCHRO_MILD_ASSERT(0);
  }
  if (hdr->profile != 0 || hdr->level != 0) {
    SCHRO_ERROR("Expecting profile/level 0,0, got %d,%d",
        hdr->profile, hdr->level);
    SCHRO_MILD_ASSERT(0);
  }

  /* base video format */
  index = schro_unpack_decode_uint (unpack);
  schro_video_format_set_std_video_format (format, index);

  /* source parameters */
  /* frame dimensions */
  bit = schro_unpack_decode_bit (unpack);
  if (bit) {
    format->width = schro_unpack_decode_uint (unpack);
    format->height = schro_unpack_decode_uint (unpack);
  }
  SCHRO_DEBUG("size = %d x %d", format->width, format->height);

  /* chroma format */
  bit = schro_unpack_decode_bit (unpack);
  if (bit) {
    format->chroma_format = schro_unpack_decode_uint (unpack);
  }
  SCHRO_DEBUG("chroma_format %d", format->chroma_format);

  /* scan format */
  bit = schro_unpack_decode_bit (unpack);
  if (bit) {
    format->interlaced = schro_unpack_decode_bit (unpack);
    if (format->interlaced) {
      format->top_field_first = schro_unpack_decode_bit (unpack);
    }
  }
  SCHRO_DEBUG("interlaced %d top_field_first %d",
      format->interlaced, format->top_field_first);

  MARKER();

  /* frame rate */
  bit = schro_unpack_decode_bit (unpack);
  if (bit) {
    int index;
    index = schro_unpack_decode_uint (unpack);
    if (index == 0) {
      format->frame_rate_numerator = schro_unpack_decode_uint (unpack);
      format->frame_rate_denominator = schro_unpack_decode_uint (unpack);
    } else {
      schro_video_format_set_std_frame_rate (format, index);
    }
  }
  SCHRO_DEBUG("frame rate %d/%d", format->frame_rate_numerator,
      format->frame_rate_denominator);

  MARKER();

  /* aspect ratio */
  bit = schro_unpack_decode_bit (unpack);
  if (bit) {
    int index;
    index = schro_unpack_decode_uint (unpack);
    if (index == 0) {
      format->aspect_ratio_numerator =
        schro_unpack_decode_uint (unpack);
      format->aspect_ratio_denominator =
        schro_unpack_decode_uint (unpack);
    } else {
      schro_video_format_set_std_aspect_ratio (format, index);
    }
  }
  SCHRO_DEBUG("aspect ratio %d/%d", format->aspect_ratio_numerator,
      format->aspect_ratio_denominator);

  MARKER();

  /* clean area */
  bit = schro_unpack_decode_bit (unpack);
  if (bit) {
    format->clean_width = schro_unpack_decode_uint (unpack);
    format->clean_height = schro_unpack_decode_uint (unpack);
    format->left_offset = schro_unpack_decode_uint (unpack);
    format->top_offset = schro_unpack_decode_uint (unpack);
  }
  SCHRO_DEBUG("clean offset %d %d", format->left_offset,
      format->top_offset);
  SCHRO_DEBUG("clean size %d %d", format->clean_width,
      format->clean_height);

  MARKER();

  /* signal range */
  bit = schro_unpack_decode_bit (unpack);
  if (bit) {
    int index;
    index = schro_unpack_decode_uint (unpack);
    if (index == 0) {
      format->luma_offset = schro_unpack_decode_uint (unpack);
      format->luma_excursion = schro_unpack_decode_uint (unpack);
      format->chroma_offset = schro_unpack_decode_uint (unpack);
      format->chroma_excursion =
        schro_unpack_decode_uint (unpack);
    } else {
      if (index <= SCHRO_SIGNAL_RANGE_12BIT_VIDEO) {
        schro_video_format_set_std_signal_range (format, index);
      } else {
        // XXX schro_decoder_error (decoder, "signal range index out of range");
      }
    }
  }
  SCHRO_DEBUG("luma offset %d excursion %d", format->luma_offset,
      format->luma_excursion);
  SCHRO_DEBUG("chroma offset %d excursion %d", format->chroma_offset,
      format->chroma_excursion);

  MARKER();

  /* colour spec */
  bit = schro_unpack_decode_bit (unpack);
  if (bit) {
    int index;
    index = schro_unpack_decode_uint (unpack);
    if (index <= SCHRO_COLOUR_SPEC_CINEMA) {
      schro_video_format_set_std_colour_spec (format, index);
    } else {
      // XXX schro_decoder_error (decoder, "colour spec index out of range");
    }
    if (index == 0) {
      /* colour primaries */
      bit = schro_unpack_decode_bit (unpack);
      if (bit) {
        format->colour_primaries = schro_unpack_decode_uint (unpack);
      }
      /* colour matrix */
      bit = schro_unpack_decode_bit (unpack);
      if (bit) {
        format->colour_matrix = schro_unpack_decode_uint (unpack);
      }
      /* transfer function */
      bit = schro_unpack_decode_bit (unpack);
      if (bit) {
        format->transfer_function = schro_unpack_decode_uint (unpack);
      }
    }
  }

  hdr->interlaced_coding = schro_unpack_decode_uint (unpack);

  MARKER();

  schro_video_format_validate (format);
}

void
schro_decoder_decode_picture_header (SchroDecoderPictureHeader *hdr, SchroUnpack *unpack, SchroDecoderParseHeader *phdr)
{
  schro_unpack_byte_sync(unpack);

  hdr->picture_number = schro_unpack_decode_bits (unpack, 32);
  SCHRO_DEBUG("picture number %d", hdr->picture_number);

  if (phdr->n_refs > 0) {
    hdr->reference1 = hdr->picture_number +
      schro_unpack_decode_sint (unpack);
    SCHRO_DEBUG("ref1 %d", hdr->reference1);
  }

  if (phdr->n_refs > 1) {
    hdr->reference2 = hdr->picture_number +
      schro_unpack_decode_sint (unpack);
    SCHRO_DEBUG("ref2 %d", hdr->reference2);
  }

  if (SCHRO_PARSE_CODE_IS_REFERENCE(phdr->parse_code)) {
    hdr->retired_picture_number = hdr->picture_number +
      schro_unpack_decode_sint (unpack);
  }
}

void
schro_decoder_decode_picture_prediction_parameters (SchroDecoderWorker *decoder)
{
  SchroParams *params = &decoder->params;
  int bit;
  int index;

  /* block parameters */
  index = schro_unpack_decode_uint (&decoder->unpack);
  if (index == 0) {
    params->xblen_luma = schro_unpack_decode_uint (&decoder->unpack);
    params->yblen_luma = schro_unpack_decode_uint (&decoder->unpack);
    params->xbsep_luma = schro_unpack_decode_uint (&decoder->unpack);
    params->ybsep_luma = schro_unpack_decode_uint (&decoder->unpack);
  } else {
    schro_params_set_block_params (params, index);
  }
  SCHRO_DEBUG("blen_luma %d %d bsep_luma %d %d",
      params->xblen_luma, params->yblen_luma,
      params->xbsep_luma, params->ybsep_luma);

  MARKER();

  /* mv precision */
  params->mv_precision = schro_unpack_decode_uint (&decoder->unpack);
  SCHRO_DEBUG("mv_precision %d", params->mv_precision);

  MARKER();

  /* global motion */
  params->have_global_motion = schro_unpack_decode_bit (&decoder->unpack);
  if (params->have_global_motion) {
    int i;

    for (i=0;i<params->num_refs;i++) {
      SchroGlobalMotion *gm = params->global_motion + i;

      /* pan/tilt */
      bit = schro_unpack_decode_bit (&decoder->unpack);
      if (bit) {
        gm->b0 = schro_unpack_decode_sint (&decoder->unpack);
        gm->b1 = schro_unpack_decode_sint (&decoder->unpack);
      } else {
        gm->b0 = 0;
        gm->b1 = 0;
      }

      /* zoom/rotate/shear */
      bit = schro_unpack_decode_bit (&decoder->unpack);
      if (bit) {
        gm->a_exp = schro_unpack_decode_uint (&decoder->unpack);
        gm->a00 = schro_unpack_decode_sint (&decoder->unpack);
        gm->a01 = schro_unpack_decode_sint (&decoder->unpack);
        gm->a10 = schro_unpack_decode_sint (&decoder->unpack);
        gm->a11 = schro_unpack_decode_sint (&decoder->unpack);
      } else {
        gm->a_exp = 0;
        gm->a00 = 1;
        gm->a01 = 0;
        gm->a10 = 0;
        gm->a11 = 1;
      }

      /* perspective */
      bit = schro_unpack_decode_bit (&decoder->unpack);
      if (bit) {
        gm->c_exp = schro_unpack_decode_uint (&decoder->unpack);
        gm->c0 = schro_unpack_decode_sint (&decoder->unpack);
        gm->c1 = schro_unpack_decode_sint (&decoder->unpack);
      } else {
        gm->c_exp = 0;
        gm->c0 = 0;
        gm->c1 = 0;
      }

      SCHRO_DEBUG("ref %d pan %d %d matrix %d %d %d %d perspective %d %d",
          i, gm->b0, gm->b1, gm->a00, gm->a01, gm->a10, gm->a11,
          gm->c0, gm->c1);
    }
  }

  MARKER();

  /* picture prediction mode */
  params->picture_pred_mode = schro_unpack_decode_uint (&decoder->unpack);
  if (params->picture_pred_mode != 0) {
    schro_decoder_error (decoder, "picture prediction mode != 0");
  }

  /* reference picture weights */
  params->picture_weight_bits = 1;
  params->picture_weight_1 = 1;
  params->picture_weight_2 = 1;
  bit = schro_unpack_decode_bit (&decoder->unpack);
  if (bit) {
    params->picture_weight_bits = schro_unpack_decode_uint (&decoder->unpack);
    params->picture_weight_1 = schro_unpack_decode_sint (&decoder->unpack);
    if (params->num_refs > 1) {
      params->picture_weight_2 = schro_unpack_decode_sint (&decoder->unpack);
    }
  }

  MARKER();
}

enum {
  SCHRO_DECODER_ARITH_SUPERBLOCK,
  SCHRO_DECODER_ARITH_PRED_MODE,
  SCHRO_DECODER_ARITH_VECTOR_REF1_X,
  SCHRO_DECODER_ARITH_VECTOR_REF1_Y,
  SCHRO_DECODER_ARITH_VECTOR_REF2_X,
  SCHRO_DECODER_ARITH_VECTOR_REF2_Y,
  SCHRO_DECODER_ARITH_DC_0,
  SCHRO_DECODER_ARITH_DC_1,
  SCHRO_DECODER_ARITH_DC_2
};

void
schro_decoder_decode_block_data (SchroDecoderWorker *decoder)
{
  SchroParams *params = &decoder->params;
  SchroArith *arith[9];
  SchroUnpack unpack[9];
  int i, j;

  memset(decoder->motion->motion_vectors, 0,
      sizeof(SchroMotionVector)*params->y_num_blocks*params->x_num_blocks);

  for(i=0;i<9;i++){
    SchroBuffer *buffer;
    int length;

    if (params->num_refs < 2 && (i == SCHRO_DECODER_ARITH_VECTOR_REF2_X ||
          i == SCHRO_DECODER_ARITH_VECTOR_REF2_Y)) {
      arith[i] = NULL;
      continue;
    }
    length = schro_unpack_decode_uint (&decoder->unpack);
    schro_unpack_byte_sync (&decoder->unpack);
    buffer = schro_buffer_new_subbuffer (decoder->input_buffer,
        schro_unpack_get_bits_read (&decoder->unpack)/8, length);

    if (!params->is_noarith) {
      arith[i] = schro_arith_new ();
      schro_arith_decode_init (arith[i], buffer);

      schro_unpack_skip_bits (&decoder->unpack, length*8);
    } else {
      schro_unpack_copy (unpack + i, &decoder->unpack);
      schro_unpack_limit_bits_remaining (unpack + i, length*8);
      schro_unpack_skip_bits (&decoder->unpack, length*8);
    }
  }

  for(j=0;j<params->y_num_blocks;j+=4){
    for(i=0;i<params->x_num_blocks;i+=4){
      schro_decoder_decode_macroblock(decoder, arith, unpack, i, j);
    }
  }

  for(i=0;i<9;i++) {
    if (!params->is_noarith) {
      if (arith[i] == NULL) continue;

      if (arith[i]->offset < arith[i]->buffer->length) {
        SCHRO_WARNING("arith decoding %d didn't consume buffer (%d < %d)", i,
            arith[i]->offset, arith[i]->buffer->length);
      }
      if (arith[i]->offset > arith[i]->buffer->length + 6) {
        SCHRO_ERROR("arith decoding %d overran buffer (%d > %d)", i,
            arith[i]->offset, arith[i]->buffer->length);
      }
      schro_buffer_unref (arith[i]->buffer);
      schro_arith_free (arith[i]);
    } else {
      /* FIXME complain about buffer over/underrun */
    }
  }
}

static void
schro_decoder_decode_macroblock(SchroDecoderWorker *decoder, SchroArith **arith,
    SchroUnpack *unpack, int i, int j)
{
  SchroParams *params = &decoder->params;
  SchroMotionVector *mv = &decoder->motion->motion_vectors[j*params->x_num_blocks + i];
  int k,l;
  int split_prediction;

  split_prediction = schro_motion_split_prediction (decoder->motion, i, j);
  if (!params->is_noarith) {
    mv->split = (split_prediction +
        _schro_arith_decode_uint (arith[SCHRO_DECODER_ARITH_SUPERBLOCK],
          SCHRO_CTX_SB_F1, SCHRO_CTX_SB_DATA))%3;
  } else {
    mv->split = (split_prediction +
        schro_unpack_decode_uint (unpack + SCHRO_DECODER_ARITH_SUPERBLOCK))%3;
  }

  switch (mv->split) {
    case 0:
      schro_decoder_decode_prediction_unit (decoder, arith, unpack,
          decoder->motion->motion_vectors, i, j);
      mv[1] = mv[0];
      mv[2] = mv[0];
      mv[3] = mv[0];
      memcpy(mv + params->x_num_blocks, mv, 4*sizeof(*mv));
      memcpy(mv + 2*params->x_num_blocks, mv, 4*sizeof(*mv));
      memcpy(mv + 3*params->x_num_blocks, mv, 4*sizeof(*mv));
      break;
    case 1:
      schro_decoder_decode_prediction_unit (decoder, arith, unpack,
          decoder->motion->motion_vectors, i, j);
      mv[1] = mv[0];
      schro_decoder_decode_prediction_unit (decoder, arith, unpack,
          decoder->motion->motion_vectors, i + 2, j);
      mv[3] = mv[2];
      memcpy(mv + params->x_num_blocks, mv, 4*sizeof(*mv));

      mv += 2*params->x_num_blocks;
      schro_decoder_decode_prediction_unit (decoder, arith, unpack,
          decoder->motion->motion_vectors, i, j + 2);
      mv[1] = mv[0];
      schro_decoder_decode_prediction_unit (decoder, arith, unpack,
          decoder->motion->motion_vectors, i + 2, j + 2);
      mv[3] = mv[2];
      memcpy(mv + params->x_num_blocks, mv, 4*sizeof(*mv));
      break;
    case 2:
      for (l=0;l<4;l++) {
        for (k=0;k<4;k++) {
          schro_decoder_decode_prediction_unit (decoder, arith, unpack,
              decoder->motion->motion_vectors, i + k, j + l);
        }
      }
      break;
    default:
      SCHRO_ERROR("mv->split == %d, split_prediction %d", mv->split, split_prediction);
      SCHRO_ASSERT(0);
  }
}

static void
schro_decoder_decode_prediction_unit(SchroDecoderWorker *decoder, SchroArith **arith,
    SchroUnpack *unpack, SchroMotionVector *motion_vectors, int x, int y)
{
  SchroParams *params = &decoder->params;
  SchroMotionVector *mv = &motion_vectors[y*params->x_num_blocks + x];

  mv->pred_mode = schro_motion_get_mode_prediction (decoder->motion,
      x, y);
  if (!params->is_noarith) {
    mv->pred_mode ^= 
      _schro_arith_decode_bit (arith[SCHRO_DECODER_ARITH_PRED_MODE],
          SCHRO_CTX_BLOCK_MODE_REF1);
  } else {
    mv->pred_mode ^= 
      schro_unpack_decode_bit (unpack + SCHRO_DECODER_ARITH_PRED_MODE);
  }
  if (params->num_refs > 1) {
    if (!params->is_noarith) {
      mv->pred_mode ^=
        _schro_arith_decode_bit (arith[SCHRO_DECODER_ARITH_PRED_MODE],
            SCHRO_CTX_BLOCK_MODE_REF2) << 1;
    } else {
      mv->pred_mode ^= 
        schro_unpack_decode_bit (unpack + SCHRO_DECODER_ARITH_PRED_MODE) << 1;
    }
  }

  if (mv->pred_mode == 0) {
    int pred[3];
    SchroMotionVectorDC *mvdc = (SchroMotionVectorDC *)mv;

    schro_motion_dc_prediction (decoder->motion, x, y, pred);

    if (!params->is_noarith) {
      mvdc->dc[0] = pred[0] + _schro_arith_decode_sint (
          arith[SCHRO_DECODER_ARITH_DC_0],
          SCHRO_CTX_LUMA_DC_CONT_BIN1, SCHRO_CTX_LUMA_DC_VALUE,
          SCHRO_CTX_LUMA_DC_SIGN);
      mvdc->dc[1] = pred[1] + _schro_arith_decode_sint (
          arith[SCHRO_DECODER_ARITH_DC_1],
          SCHRO_CTX_CHROMA1_DC_CONT_BIN1, SCHRO_CTX_CHROMA1_DC_VALUE,
          SCHRO_CTX_CHROMA1_DC_SIGN);
      mvdc->dc[2] = pred[2] + _schro_arith_decode_sint (
          arith[SCHRO_DECODER_ARITH_DC_2],
          SCHRO_CTX_CHROMA2_DC_CONT_BIN1, SCHRO_CTX_CHROMA2_DC_VALUE,
          SCHRO_CTX_CHROMA2_DC_SIGN);
    } else {
      mvdc->dc[0] = pred[0] +
        schro_unpack_decode_sint (unpack + SCHRO_DECODER_ARITH_DC_0);
      mvdc->dc[1] = pred[1] +
        schro_unpack_decode_sint (unpack + SCHRO_DECODER_ARITH_DC_1);
      mvdc->dc[2] = pred[2] +
        schro_unpack_decode_sint (unpack + SCHRO_DECODER_ARITH_DC_2);
    }
  } else {
    int pred_x, pred_y;

    if (params->have_global_motion) {
      int pred;
      pred = schro_motion_get_global_prediction (decoder->motion, x, y);
      if (!params->is_noarith) {
        mv->using_global = pred ^ _schro_arith_decode_bit (
            arith[SCHRO_DECODER_ARITH_PRED_MODE], SCHRO_CTX_GLOBAL_BLOCK);
      } else {
        mv->using_global = pred ^ schro_unpack_decode_bit (
            unpack + SCHRO_DECODER_ARITH_PRED_MODE);
      }
    } else {
      mv->using_global = FALSE;
    }
    if (!mv->using_global) {
      if (mv->pred_mode & 1) {
        schro_motion_vector_prediction (decoder->motion, x, y,
            &pred_x, &pred_y, 1);

        if (!params->is_noarith) {
          mv->x1 = pred_x + _schro_arith_decode_sint (
                arith[SCHRO_DECODER_ARITH_VECTOR_REF1_X],
                SCHRO_CTX_MV_REF1_H_CONT_BIN1, SCHRO_CTX_MV_REF1_H_VALUE,
                SCHRO_CTX_MV_REF1_H_SIGN);
          mv->y1 = pred_y + _schro_arith_decode_sint (
                arith[SCHRO_DECODER_ARITH_VECTOR_REF1_Y],
                SCHRO_CTX_MV_REF1_V_CONT_BIN1, SCHRO_CTX_MV_REF1_V_VALUE,
                SCHRO_CTX_MV_REF1_V_SIGN);
        } else {
          mv->x1 = pred_x + schro_unpack_decode_sint (
                unpack + SCHRO_DECODER_ARITH_VECTOR_REF1_X);
          mv->y1 = pred_y + schro_unpack_decode_sint (
                unpack + SCHRO_DECODER_ARITH_VECTOR_REF1_Y);
        }
      }
      if (mv->pred_mode & 2) {
        schro_motion_vector_prediction (decoder->motion, x, y,
            &pred_x, &pred_y, 2);

        if (!params->is_noarith) {
          mv->x2 = pred_x + _schro_arith_decode_sint (
                arith[SCHRO_DECODER_ARITH_VECTOR_REF2_X],
                SCHRO_CTX_MV_REF2_H_CONT_BIN1, SCHRO_CTX_MV_REF2_H_VALUE,
                SCHRO_CTX_MV_REF2_H_SIGN);
          mv->y2 = pred_y + _schro_arith_decode_sint (
                arith[SCHRO_DECODER_ARITH_VECTOR_REF2_Y],
                SCHRO_CTX_MV_REF2_V_CONT_BIN1, SCHRO_CTX_MV_REF2_V_VALUE,
                SCHRO_CTX_MV_REF2_V_SIGN);
        } else {
          mv->x2 = pred_x + schro_unpack_decode_sint (
                unpack + SCHRO_DECODER_ARITH_VECTOR_REF2_X);
          mv->y2 = pred_y + schro_unpack_decode_sint (
                unpack + SCHRO_DECODER_ARITH_VECTOR_REF2_Y);
        }
      }
    } else {
      mv->x1 = 0;
      mv->y1 = 0;
      mv->x2 = 0;
      mv->y2 = 0;
    }
  }
}

void
schro_decoder_decode_transform_parameters (SchroDecoderWorker *decoder)
{
  int bit;
  int i;
  SchroParams *params = &decoder->params;

  /* transform */
  params->wavelet_filter_index = schro_unpack_decode_uint (&decoder->unpack);
  SCHRO_DEBUG ("wavelet filter index %d", params->wavelet_filter_index);

  /* transform depth */
  params->transform_depth = schro_unpack_decode_uint (&decoder->unpack);
  SCHRO_DEBUG ("transform depth %d", params->transform_depth);

  if (!SCHRO_PARSE_CODE_IS_LOW_DELAY(decoder->header.parse_code)) {
    /* codeblock parameters */
    params->codeblock_mode_index = 0;
    for(i=0;i<params->transform_depth + 1;i++) {
      params->horiz_codeblocks[i] = 1;
      params->vert_codeblocks[i] = 1;
    }

    bit = schro_unpack_decode_bit (&decoder->unpack);
    if (bit) {
      int i;
      for(i=0;i<params->transform_depth + 1;i++) {
        params->horiz_codeblocks[i] = schro_unpack_decode_uint (&decoder->unpack);
        params->vert_codeblocks[i] = schro_unpack_decode_uint (&decoder->unpack);
      }
      params->codeblock_mode_index = schro_unpack_decode_uint (&decoder->unpack);
    }
  } else {
    /* slice parameters */
    params->n_horiz_slices = schro_unpack_decode_uint(&decoder->unpack);
    params->n_vert_slices = schro_unpack_decode_uint(&decoder->unpack);

    params->slice_bytes_num = schro_unpack_decode_uint(&decoder->unpack);
    params->slice_bytes_denom = schro_unpack_decode_uint(&decoder->unpack);

    /* quant matrix */
    bit = schro_unpack_decode_bit (&decoder->unpack);
    if (bit) {
      params->quant_matrix[0] = schro_unpack_decode_uint (&decoder->unpack);
      for(i=0;i<params->transform_depth;i++){
        params->quant_matrix[1+3*i] = schro_unpack_decode_uint (&decoder->unpack);
        params->quant_matrix[2+3*i] = schro_unpack_decode_uint (&decoder->unpack);
        params->quant_matrix[3+3*i] = schro_unpack_decode_uint (&decoder->unpack);
      }
    } else {
      schro_params_set_default_quant_matrix (params);
    }
  }
}

#ifndef SCHRO_GPU
void
schro_decoder_decode_transform_data (SchroDecoderWorker *decoder)
{
  int i;
  int component;
  SchroParams *params = &decoder->params;
  SchroDecoderSubbandContext context = { 0 }, *ctx = &context;

  for(component=0;component<3;component++){
    for(i=0;i<1+3*params->transform_depth;i++) {
      ctx->component = component;
      ctx->position = schro_subband_get_position(i);

      schro_subband_get (decoder->frame, ctx->component, ctx->position, &decoder->params, &ctx->data, &ctx->stride, &ctx->width, &ctx->height);
      if (ctx->position >= 4) {
          schro_subband_get (decoder->frame, ctx->component, ctx->position - 4,
            &decoder->params, &ctx->parent_data, &ctx->parent_stride, &ctx->parent_width, &ctx->parent_height);
      } else {
          ctx->parent_data = NULL;
          ctx->parent_stride = 0;
      }

      schro_unpack_byte_sync (&decoder->unpack);
      if(!schro_decoder_decode_subband (decoder, ctx))
      {
          schro_decoder_zero_block (ctx, 0, 0, ctx->width, ctx->height);
      }
    }
  }
}
#endif

#ifdef SCHRO_GPU
void
schro_subband_get_s (int component, int position,
    SchroParams *params,
    int *stride, int *width, int *height)
{
  int shift;

  shift = params->transform_depth - SCHRO_SUBBAND_SHIFT(position);

  if (component == 0) {
    *width = params->iwt_luma_width >> shift;
    *height = params->iwt_luma_height >> shift;
  } else {
    *width = params->iwt_chroma_width >> shift;
    *height = params->iwt_chroma_height >> shift;
  }
  *stride = (*width) *2;
}

static void
schro_decoder_decode_transform_data_serial (SchroDecoderWorker *decoder, schro_subband_storage *store, SchroGPUFrame *frame)
{
  int i;
  int component;
  SchroParams *params = &decoder->params;
  SchroDecoderSubbandContext context = { 0 }, *ctx = &context;
  int total_length = 0;
  int bandid = 0;

  for(component=0;component<3;component++) {
    for(i=0;i<1+3*params->transform_depth;i++) {
    
      // 64 byte (32 pixel) align up
      total_length = ROUND_UP_POW2(total_length,5);

      ctx->component = component;
      ctx->position = schro_subband_get_position(i);
      ctx->data = &store->data[total_length];

      schro_subband_get_s(ctx->component, ctx->position, &decoder->params, &ctx->stride, &ctx->width, &ctx->height);

      int length = ctx->height*ctx->stride/2;

      SCHRO_ASSERT((total_length+length) <= store->maxsize);

      if (ctx->position >= 4) {
          schro_subband_get_s(ctx->component, ctx->position-4, &decoder->params, &ctx->parent_stride, &ctx->parent_width, &ctx->parent_height);
          int parent_offset = store->offsets[bandid-3];
          if(parent_offset != -1)
          {
              ctx->parent_data = &store->data[parent_offset];
          } else {
              /** If the parent block was zero, we must provide a generic block filled with zeroes for reference */
              ctx->parent_data = store->zeroes;
          }
      } else {
          ctx->parent_data = NULL;
          ctx->parent_stride = 0;
      }
#ifdef ASSUME_ZERO
      memset(ctx->data, 0, length*2);
#endif
      schro_unpack_byte_sync (&decoder->unpack);
      if(schro_decoder_decode_subband (decoder, ctx))
      {
          store->offsets[bandid] = total_length;
          total_length += length;
      }
      else
      {
          /** Empty block */
          store->offsets[bandid] = -1;
      }
      /* SCHRO_ERROR("%p %i (%i %i)", decoder, bandid, ctx->component, ctx->position); */
      ++bandid;
      /** Fire off asynchronous transfer to GPU */
      decoder->subband_max = bandid;
    }
  }
  store->used = total_length;
  SCHRO_DEBUG("coefficients in buffer: %i (%f%%)", total_length, 100.0*(double)total_length/store->maxsize);
}

void schro_decoder_async_transfer(SchroDecoderWorker *decoder)
{
  /* Start transfers in FIFO order */
  int x;
  int limit = decoder->subband_max;
  int subbands_per_component = 1+3*decoder->params.transform_depth;
  /* SCHRO_ERROR("%i %i", decoder->subband_min, limit); */
  for(x = decoder->subband_min; x < limit; ++x)
  {
    int component, position;
    if(x == 0)
      schro_subband_storage_to_gpuframe_init(decoder->store, decoder->frame);
    component = x / subbands_per_component;
    position = schro_subband_get_position(x % subbands_per_component);
    /* SCHRO_ERROR("%p %i/%i/%i (%i %i)", decoder, x, limit, subbands_per_component*3, component, position); */
    schro_subband_storage_to_gpuframe(decoder->store, decoder->frame, 
      component, position, decoder->store->offsets[x]);
  }
  decoder->subband_min = x;
  SCHRO_ASSERT(x == limit);
}
#endif

static void
codeblock_line_decode_generic (SchroDecoderSubbandContext *ctx,
    int16_t *line, int j, const int16_t *parent_data, const int16_t *prev)
{
  int i;

  for(i=ctx->xmin;i<ctx->xmax;i++){
    int v;
    int parent;
    int nhood_or;
    int previous_value;

    if (parent_data) {
      parent = parent_data[(i>>1)];
    } else {
      parent = 0;
    }

    nhood_or = 0;
    if (j>0) nhood_or |= prev[i];
    if (i>0) nhood_or |= line[i-1];
    if (i>0 && j>0) nhood_or |= prev[i-1];

    previous_value = 0;
    if (SCHRO_SUBBAND_IS_HORIZONTALLY_ORIENTED(ctx->position)) {
      if (i > 0) previous_value = line[i-1];
    } else if (SCHRO_SUBBAND_IS_VERTICALLY_ORIENTED(ctx->position)) {
      if (j > 0) previous_value = prev[i];
    }

#define STUFF \
  do { \
    int cont_context, sign_context, value_context; \
    \
    if (parent == 0) { \
      cont_context = nhood_or ? SCHRO_CTX_ZPNN_F1 : SCHRO_CTX_ZPZN_F1; \
    } else { \
      cont_context = nhood_or ? SCHRO_CTX_NPNN_F1 : SCHRO_CTX_NPZN_F1; \
    } \
     \
    if (previous_value < 0) { \
      sign_context = SCHRO_CTX_SIGN_NEG; \
    } else { \
      sign_context = (previous_value > 0) ? SCHRO_CTX_SIGN_POS : \
        SCHRO_CTX_SIGN_ZERO; \
    } \
 \
    value_context = SCHRO_CTX_COEFF_DATA; \
 \
    v = _schro_arith_decode_uint (ctx->arith, cont_context, \
        value_context); \
    if (v) { \
      v = (ctx->quant_offset + ctx->quant_factor * v + 2)>>2; \
      if (_schro_arith_decode_bit (ctx->arith, sign_context)) { \
        v = -v; \
      } \
      line[i] = v; \
    } else { \
      line[i] = 0; \
    } \
  } while(0)

    STUFF;
  }
}

static void
codeblock_line_decode_noarith (SchroDecoderSubbandContext *ctx,
    int16_t *line, SchroDecoderWorker *decoder)
{
  int i;

  for(i=ctx->xmin;i<ctx->xmax;i++){
    line[i] = schro_dequantise (schro_unpack_decode_sint (&ctx->unpack),
        ctx->quant_factor, ctx->quant_offset);
  }
}

#if 0
static void
codeblock_line_decode_deep (SchroDecoderSubbandContext *ctx,
    int32_t *line, int j, const int32_t *parent_data, const int32_t *prev)
{
  int i;

  for(i=ctx->xmin;i<ctx->xmax;i++){
    int v;
    int parent;
    int nhood_or;
    int previous_value;

    if (parent_data) {
      parent = parent_data[(i>>1)];
    } else {
      parent = 0;
    }

    nhood_or = 0;
    if (j>0) nhood_or |= prev[i];
    if (i>0) nhood_or |= line[i-1];
    if (i>0 && j>0) nhood_or |= prev[i-1];

    previous_value = 0;
    if (SCHRO_SUBBAND_IS_HORIZONTALLY_ORIENTED(ctx->position)) {
      if (i > 0) previous_value = line[i-1];
    } else if (SCHRO_SUBBAND_IS_VERTICALLY_ORIENTED(ctx->position)) {
      if (j > 0) previous_value = prev[i];
    }

    STUFF;
  }
}

static void
codeblock_line_decode_deep_parent (SchroDecoderSubbandContext *ctx,
    int16_t *line, int j, const int32_t *parent_data, const int16_t *prev)
{
  int i;

  for(i=ctx->xmin;i<ctx->xmax;i++){
    int v;
    int parent;
    int nhood_or;
    int previous_value;

    if (parent_data) {
      parent = parent_data[(i>>1)];
    } else {
      parent = 0;
    }

    nhood_or = 0;
    if (j>0) nhood_or |= prev[i];
    if (i>0) nhood_or |= line[i-1];
    if (i>0 && j>0) nhood_or |= prev[i-1];

    previous_value = 0;
    if (SCHRO_SUBBAND_IS_HORIZONTALLY_ORIENTED(ctx->position)) {
      if (i > 0) previous_value = line[i-1];
    } else if (SCHRO_SUBBAND_IS_VERTICALLY_ORIENTED(ctx->position)) {
      if (j > 0) previous_value = prev[i];
    }

    STUFF;
  }
}
#endif


static void
codeblock_line_decode_p_horiz (SchroDecoderSubbandContext *ctx,
    int16_t *line, int j, const int16_t *parent_data, const int16_t *prev)
{
  int i = ctx->xmin;
  int v;
  int parent;
  int nhood_or;
  int previous_value;

  if (i == 0) {
    parent = parent_data[(i>>1)];
    nhood_or = prev[i];
    previous_value = 0;

    STUFF;
    i++;
  }
  for(;i<ctx->xmax;i++){
    parent = parent_data[(i>>1)];

    nhood_or = prev[i];
    nhood_or |= line[i-1];
    nhood_or |= prev[i-1];

    previous_value = line[i-1];

    STUFF;
  }
}

static void
codeblock_line_decode_p_vert (SchroDecoderSubbandContext *ctx,
    int16_t *line, int j, const int16_t *parent_data, const int16_t *prev)
{
  int i = ctx->xmin;
  int v;
  int parent;
  int nhood_or;
  int previous_value;

  if (i == 0) {
    parent = parent_data[(i>>1)];
    nhood_or = prev[i];
    previous_value = prev[i];

    STUFF;
    i++;
  }
  for(;i<ctx->xmax;i++){
    parent = parent_data[(i>>1)];

    nhood_or = prev[i];
    nhood_or |= line[i-1];
    nhood_or |= prev[i-1];

    previous_value = prev[i];

    STUFF;
  }
}

static void
codeblock_line_decode_p_diag (SchroDecoderSubbandContext *ctx,
    int16_t *line, int j,
    const int16_t *parent_data,
    const int16_t *prev)
{
  int i;
  int v;
  int parent;
  int nhood_or;
  int previous_value;

  i = ctx->xmin;
  if (i == 0) {
    parent = parent_data[(i>>1)];
    nhood_or = prev[i];
    previous_value = 0;

    STUFF;
    i++;
  }
  for(;i<ctx->xmax;i++){
    parent = parent_data[(i>>1)];

    nhood_or = prev[i];
    nhood_or |= line[i-1];
    nhood_or |= prev[i-1];
    previous_value = 0;

    STUFF;
  }
}

void
schro_decoder_subband_dc_predict (SchroFrameData *fd)
{
  int16_t *prev_line;
  int16_t *line;
  int i,j;
  int pred_value;

  line = SCHRO_FRAME_DATA_GET_LINE(fd, 0);
  for(i=1;i<fd->width;i++){
    pred_value = line[i-1];
    line[i] += pred_value;
  }
  
  for(j=1;j<fd->height;j++){
    line = SCHRO_FRAME_DATA_GET_LINE(fd, j);
    prev_line = SCHRO_FRAME_DATA_GET_LINE(fd, j-1);

    pred_value = prev_line[0];
    line[0] += pred_value;

    for(i=1;i<fd->width;i++){
      pred_value = schro_divide(line[i-1] + prev_line[i] +
          prev_line[i-1] + 1,3);
      line[i] += pred_value;
    }
  }

}

static void
schro_decoder_setup_codeblocks (SchroDecoderWorker *decoder,
    SchroDecoderSubbandContext *ctx)
{
  SchroParams *params = &decoder->params;

  if (ctx->position == 0) {
    ctx->vert_codeblocks = params->vert_codeblocks[0];
    ctx->horiz_codeblocks = params->horiz_codeblocks[0];
  } else {
    ctx->vert_codeblocks = params->vert_codeblocks[SCHRO_SUBBAND_SHIFT(ctx->position)+1];
    ctx->horiz_codeblocks = params->horiz_codeblocks[SCHRO_SUBBAND_SHIFT(ctx->position)+1];
  }
  if ((ctx->horiz_codeblocks > 1 || ctx->vert_codeblocks > 1) && ctx->position > 0) {
    ctx->have_zero_flags = TRUE;
  } else {
    ctx->have_zero_flags = FALSE;
  }
  if (ctx->horiz_codeblocks > 1 || ctx->vert_codeblocks > 1) {
    if (params->codeblock_mode_index == 1) {
      ctx->have_quant_offset = TRUE;
    } else {
      ctx->have_quant_offset = FALSE;
    }
  } else {
    ctx->have_quant_offset = FALSE;
  }
}

#ifndef ASSUME_ZERO
static void
schro_decoder_zero_block (SchroDecoderSubbandContext *ctx,
    int x1, int y1, int x2, int y2)
{
  int j;
  int16_t *line;

  SCHRO_DEBUG("subband is zero");
  for(j=y1;j<y2;j++){
    line = OFFSET(ctx->data, j*ctx->stride);
    oil_splat_s16_ns (line + x1, schro_zero, x2 - x1);
  }
}
#endif

static void
schro_decoder_decode_codeblock (SchroDecoderWorker *decoder,
    SchroDecoderSubbandContext *ctx)
{
  SchroParams *params = &decoder->params;
  int j;

  if (ctx->have_zero_flags) {
    int bit;

    /* zero codeblock */
    if (params->is_noarith) {
      bit = schro_unpack_decode_bit (&ctx->unpack);
    } else {
      bit = _schro_arith_decode_bit (ctx->arith, SCHRO_CTX_ZERO_CODEBLOCK);
    }
    if (bit) {
#ifndef ASSUME_ZERO
      schro_decoder_zero_block (ctx, ctx->xmin, ctx->ymin,
          ctx->xmax, ctx->ymax);
#endif
      return;
    }
  }

  if (ctx->have_quant_offset) {
    if (params->is_noarith) {
      ctx->quant_index += schro_unpack_decode_sint (&ctx->unpack);
    } else {
      ctx->quant_index += _schro_arith_decode_sint (ctx->arith,
          SCHRO_CTX_QUANTISER_CONT, SCHRO_CTX_QUANTISER_VALUE,
          SCHRO_CTX_QUANTISER_SIGN);
    }

    /* FIXME check quant_index */
    SCHRO_MILD_ASSERT(ctx->quant_index >= 0);
    SCHRO_MILD_ASSERT(ctx->quant_index <= 60);
  }

  ctx->quant_factor = schro_table_quant[ctx->quant_index];
  if (params->num_refs > 0) {
    ctx->quant_offset = schro_table_offset_3_8[ctx->quant_index];
  } else {
    ctx->quant_offset = schro_table_offset_1_2[ctx->quant_index];
  }

  for(j=ctx->ymin;j<ctx->ymax;j++){
    int16_t *p = OFFSET(ctx->data,j*ctx->stride);
    const int16_t *parent_line;
    const int16_t *prev_line;
    if (ctx->position >= 4) {
      parent_line = OFFSET(ctx->parent_data, (j>>1)*ctx->parent_stride);
    } else {
      parent_line = NULL;
    }
    if (j==0) {
      prev_line = schro_zero;
    } else {
      prev_line = OFFSET(ctx->data, (j-1)*ctx->stride);
    }
    if (params->is_noarith) {
      codeblock_line_decode_noarith (ctx, p, decoder);
    } else if (ctx->position >= 4) {
      if (SCHRO_SUBBAND_IS_HORIZONTALLY_ORIENTED(ctx->position)) {
        codeblock_line_decode_p_horiz (ctx, p, j, parent_line, prev_line);
      } else if (SCHRO_SUBBAND_IS_VERTICALLY_ORIENTED(ctx->position)) {
        codeblock_line_decode_p_vert (ctx, p, j, parent_line, prev_line);
      } else {
        codeblock_line_decode_p_diag (ctx, p, j, parent_line, prev_line);
      }
    } else {
      codeblock_line_decode_generic (ctx, p, j, parent_line, prev_line);
    }
  }
}

int
schro_decoder_decode_subband (SchroDecoderWorker *decoder,
    SchroDecoderSubbandContext *ctx)
{
  SchroParams *params = &decoder->params;
  int x,y;
  SchroBuffer *buffer = NULL;

  ctx->subband_length = schro_unpack_decode_uint (&decoder->unpack);
  SCHRO_DEBUG("subband %d %d length %d", ctx->component, ctx->position,
      ctx->subband_length);

  if (ctx->subband_length == 0) {
    SCHRO_DEBUG("subband is zero");
    schro_unpack_byte_sync (&decoder->unpack);
    //schro_decoder_zero_block (ctx, 0, 0, ctx->width, ctx->height);
    return 0;
  }

  ctx->quant_index = schro_unpack_decode_uint (&decoder->unpack);
  SCHRO_DEBUG("quant index %d", ctx->quant_index);

  /* FIXME check quant_index */
  SCHRO_MILD_ASSERT(ctx->quant_index >= 0);
  SCHRO_MILD_ASSERT(ctx->quant_index <= 60);

  schro_unpack_byte_sync (&decoder->unpack);
  if (!params->is_noarith) {
    buffer = schro_buffer_new_subbuffer (decoder->input_buffer,
        schro_unpack_get_bits_read (&decoder->unpack)/8,
        ctx->subband_length);

    ctx->arith = schro_arith_new ();
    schro_arith_decode_init (ctx->arith, buffer);
  } else {
    schro_unpack_copy (&ctx->unpack, &decoder->unpack);
    schro_unpack_limit_bits_remaining (&ctx->unpack, ctx->subband_length*8);
  }

  schro_decoder_setup_codeblocks (decoder, ctx);

  for(y=0;y<ctx->vert_codeblocks;y++){
    ctx->ymin = (ctx->height*y)/ctx->vert_codeblocks;
    ctx->ymax = (ctx->height*(y+1))/ctx->vert_codeblocks;

    for(x=0;x<ctx->horiz_codeblocks;x++){

      ctx->xmin = (ctx->width*x)/ctx->horiz_codeblocks;
      ctx->xmax = (ctx->width*(x+1))/ctx->horiz_codeblocks;
      
      schro_decoder_decode_codeblock (decoder, ctx);
    }
  }
  if (!params->is_noarith) {
    schro_arith_decode_flush (ctx->arith);
    if (ctx->arith->offset < buffer->length) {
      SCHRO_ERROR("arith decoding didn't consume buffer (%d < %d)",
          ctx->arith->offset, buffer->length);
    }
    if (ctx->arith->offset > buffer->length + 4) {
      SCHRO_ERROR("arith decoding overran buffer (%d > %d)",
          ctx->arith->offset, buffer->length);
    }
    schro_arith_free (ctx->arith);
    schro_buffer_unref (buffer);
  }
  schro_unpack_skip_bits (&decoder->unpack, ctx->subband_length*8);

  if (ctx->position == 0 && decoder->header.n_refs == 0) {
    SchroFrameData fd;
    fd.data = ctx->data;
    fd.stride = ctx->stride;
    fd.height = ctx->height;
    fd.width = ctx->width;
    schro_decoder_subband_dc_predict (&fd);
  }
  return 1;
}

static void
schro_decoder_error (SchroDecoderWorker *decoder, const char *s)
{
  SCHRO_DEBUG("decoder error");
  decoder->error = TRUE;
  if (!decoder->error_message) {
    decoder->error_message = strdup(s);
  }
}

