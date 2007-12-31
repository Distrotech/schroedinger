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

/** Thread management -- global object */
struct _SchroDecoder
{
  int time; /* monotonically increasing time */
  int worker_count;
  int n_threads;
  
  /** Worker thread management. The lock of this async object also protects
      the fields of this structure marked with NEED LOCKING.
   */
  //SchroAsync *async;
  pthread_mutex_t mutex;
  
  /** Reference queue. The list of reference pictures. NEED LOCKING.
      Managed using two condition variables.
  */
  SchroQueue *reference_queue;
  pthread_cond_t reference_notfull;//, reference_newframe;

  /** Output queue. A list of frames provided by the app that we'll decode into.
   */
  SchroQueue *output_queue;

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
  
  /** Current decoder settings. Can change for each access unit. 
   */
  SchroDecoderSettings settings;

  SchroPictureNumber next_frame_number;
  SchroPictureNumber earliest_frame;
  
  double skip_value;
  double skip_ratio;
  
  /** Working state 
   */
  SchroBuffer *input_buffer;

  int have_access_unit;
  int have_frame_number;
  int has_md5;
  uint8_t md5_checksum[32];

  /** Retired frame management.
   */
  struct _RetireEntry retired[SCHRO_RETIRE_QUEUE_SIZE];
  int retired_count;
#ifdef SCHRO_GPU
  void *free_stack[UQUEUE_SIZE];
  int free_count;

  SchroGPUFrame *planar_output_frame;
  SchroGPUFrame *gupsample_temp;
#endif
};

SchroDecoder *schro_decoder_new();
void schro_decoder_reset (SchroDecoder *decoder);
SchroVideoFormat *schro_decoder_get_video_format (SchroDecoder *decoder);

void schro_decoder_set_earliest_frame (SchroDecoder *decoder, SchroPictureNumber earliest_frame);
void schro_decoder_set_skip_ratio (SchroDecoder *decoder, double ratio);
void schro_decoder_add_output_picture (SchroDecoder *decoder, SchroFrame *frame);

void schro_decoder_push (SchroDecoder *decoder, SchroBuffer *buffer);
SchroFrame *schro_decoder_pull (SchroDecoder *decoder);
int schro_decoder_iterate (SchroDecoder *decoder);

void *schro_decoder_reference_getfree(SchroDecoder *decoder);

#ifndef SCHRO_GPU
void schro_decoder_reference_add (SchroDecoder *decoder,
    SchroUpsampledFrame *frame, SchroPictureNumber picture_number);
SchroUpsampledFrame * schro_decoder_reference_get (SchroDecoder *decoder,
    SchroPictureNumber frame_number);
//void schro_decoder_reference_retire (SchroDecoder *decoder,
//    SchroPictureNumber frame_number);
#else
void schro_decoder_reference_add (SchroDecoder *decoder,
    SchroUpsampledGPUFrame *frame, SchroPictureNumber picture_number);
SchroUpsampledGPUFrame * schro_decoder_reference_get (SchroDecoder *decoder,
    SchroPictureNumber frame_number);
//void schro_decoder_reference_retire (SchroDecoder *decoder,
//    SchroPictureNumber frame_number);
#endif
void schro_decoder_add_finished_frame (SchroDecoder *decoder, SchroFrame *frame);
void schro_decoder_skipstate (SchroDecoder *decoder, SchroPicture *w, int state);

void schro_decoder_free(SchroDecoder *self);

SCHRO_END_DECLS

#endif

