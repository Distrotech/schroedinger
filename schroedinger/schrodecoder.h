#ifndef __SCHRO_DECODER_H__
#define __SCHRO_DECODER_H__

#include <schroedinger/schropicture.h>
#include <schroedinger/schroasync.h>
#include <pthread.h> /** TODO generalize condition variables in schroasync */

SCHRO_BEGIN_DECLS

/* max number of ready decoder objects, should be larger or
   equal to the number of threads.
*/
#define SCHRO_MAX_DECODERS 16
#define SCHRO_MAX_THREADS 16
#define SCHRO_RETIRE_QUEUE_SIZE 40
#define UQUEUE_SIZE 40 /* Max number of stored ref frames (GPU) */

struct _RetireEntry
{
  int time;
  int frame;
};

struct _SchroDecoderThread
{
  SchroDecoder *parent;
  int id;
  pthread_t thread;
  int gpu; /* is this the gpu thread? */
  int quit;
};

typedef struct _SchroDecoderThread SchroDecoderThread;

#ifdef SCHRO_ENABLE_UNSTABLE_API
struct _SchroDecoder {
  /*< private >*/
  /** Reference queue. The list of reference pictures. NEED LOCKING.
      Managed using two condition variables.
  */
  SchroQueue *reference_queue;

  /** Output queue. A list of frames provided by the app that we'll decode into.
   */
  SchroQueue *output_queue;

  SchroPictureNumber next_frame_number;

  int time; /* monotonically increasing time */
  int worker_count;
  int n_threads;
  
  /** Worker thread management. The lock of this async object also protects
      the fields of this structure marked with NEED LOCKING.
   */
  pthread_mutex_t mutex;
  
  /** Frame queue. Queue of decoded, finished frames, for display. 
      NEED LOCKING
   */
  SchroQueue *frame_queue;
  
  /** List of schrodecoders slave objects.
      NEED LOCKING
   */
  SchroPicture *workers[SCHRO_MAX_DECODERS];
  SchroDecoderThread threads[SCHRO_MAX_THREADS];
  pthread_cond_t worker_statechange;
  pthread_cond_t worker_available;
  
  /** Current decoder settings.
   */
  int major_version;
  int minor_version;
  int profile;
  int level;
  schro_bool interlaced_coding;
  SchroVideoFormat video_format;

  int parse_code;
  int n_refs;
  int next_parse_offset;
  int prev_parse_offset;

  SchroPictureNumber picture_number;
  SchroPictureNumber reference1;
  SchroPictureNumber reference2;
  SchroPictureNumber retired_picture_number;

  SchroPictureNumber earliest_frame;
  
  int have_access_unit;
  int have_frame_number;

  double skip_value;
  double skip_ratio;
  
  int has_md5;
  uint8_t md5_checksum[32];

  /** Working state 
   */
  SchroBuffer *input_buffer;

  /** Retired frame management.
   */
  struct _RetireEntry retired[SCHRO_RETIRE_QUEUE_SIZE];
  int retired_count;
#ifdef SCHRO_GPU
  void *free_stack[UQUEUE_SIZE];
  int free_count;

  SchroFrame *planar_output_frame;
  SchroFrame *gupsample_temp;
#endif
};

struct _SchroPicture {
  /*< private >*/
  /* operations completed on this frame */
  int curstate;
  /* operations in progress */
  int busystate;
  /* persistent state, will be kept for next frame */
  int skipstate;
  
  /* Passed in from parent object */
  int time;
  SchroDecoder *parent;


  SchroBuffer *input_buffer;
  SchroParams params;
  SchroPictureNumber picture_number;
  int n_refs;
  SchroPictureNumber reference1;
  SchroPictureNumber reference2;
  SchroPictureNumber retired_picture_number;

  int subband_length[3][SCHRO_LIMIT_SUBBANDS];
  int subband_quant_index[3][SCHRO_LIMIT_SUBBANDS];
  SchroBuffer *subband_buffer[3][SCHRO_LIMIT_SUBBANDS];
  SchroFrameData subband_data[3][SCHRO_LIMIT_SUBBANDS];

  int has_md5;
  uint8_t md5_checksum[32];

  SchroCUDAStream stream; /* CUDA stream handle */
#ifdef SCHRO_GPU
  int subband_min; /* last band+1 that was transferred to the GPU (updated only by GPU thread) */
  int subband_max; /* last band+1 that was decoded (updated only by CPU threads) */
#endif
  
#ifndef SCHRO_GPU
  SchroUpsampledFrame *ref0;
  SchroUpsampledFrame *ref1;
#else
  SchroUpsampledFrame *ref0;
  SchroUpsampledFrame *ref1;
#endif

  int16_t *tmpbuf;
  int16_t *tmpbuf2;

  int parse_code;
  int next_parse_offset;
  int prev_parse_offset;

  SchroUnpack unpack;

  int zero_residual;

#ifndef SCHRO_GPU
  SchroFrame *frame;
  SchroFrame *mc_tmp_frame;
  SchroFrame *planar_output_frame;
#else
  SchroFrame *frame;
  SchroFrame *mc_tmp_frame;
#endif
  SchroMotion *motion;
  SchroFrame *output_picture;

  int error;
  char *error_message;

#ifdef SCHRO_GPU
  /// Output frame on GPU
  
  SchroFrame *goutput_frame;

  schro_subband_storage *store;  

  SchroGPUMotion *gpumotion;
#endif

};
#endif

SchroDecoder * schro_decoder_new (void);
void schro_decoder_free (SchroDecoder *decoder);
void schro_decoder_reset (SchroDecoder *decoder);
SchroVideoFormat * schro_decoder_get_video_format (SchroDecoder *decoder);
void schro_decoder_push (SchroDecoder *decoder, SchroBuffer *buffer);
SchroFrame *schro_decoder_pull (SchroDecoder *decoder);
int schro_decoder_iterate (SchroDecoder *decoder);

void schro_decoder_set_earliest_frame (SchroDecoder *decoder, SchroPictureNumber earliest_frame);
void schro_decoder_set_skip_ratio (SchroDecoder *decoder, double ratio);
void schro_decoder_add_output_picture (SchroDecoder *decoder, SchroFrame *frame);

void *schro_decoder_reference_getfree (SchroDecoder *decoder);

#ifndef SCHRO_GPU
void schro_decoder_reference_add (SchroDecoder *decoder,
    SchroUpsampledFrame *frame, SchroPictureNumber picture_number);
SchroUpsampledFrame * schro_decoder_reference_get (SchroDecoder *decoder,
    SchroPictureNumber frame_number);
//void schro_decoder_reference_retire (SchroDecoder *decoder,
//    SchroPictureNumber frame_number);
#else
void schro_decoder_reference_add (SchroDecoder *decoder,
    SchroUpsampledFrame *frame, SchroPictureNumber picture_number);
SchroUpsampledFrame * schro_decoder_reference_get (SchroDecoder *decoder,
    SchroPictureNumber frame_number);
//void schro_decoder_reference_retire (SchroDecoder *decoder,
//    SchroPictureNumber frame_number);
#endif
void schro_decoder_add_finished_frame (SchroDecoder *decoder, SchroFrame *frame);
void schro_decoder_skipstate (SchroDecoder *decoder, SchroPicture *w, int state);


SCHRO_END_DECLS

#endif

