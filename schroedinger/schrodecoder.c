#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#define SCHRO_ARITH_DEFINE_INLINE
#include <schroedinger/schro.h>
#include <schroedinger/schropicture.h>

#include <liboil/liboil.h>
#include <schroedinger/schrooil.h>
#include <string.h>
#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

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

#ifdef SCHRO_GPU
#define schro_frame_convert schro_gpuframe_convert
#define schro_frame_add schro_gpuframe_add
#define schro_frame_new_and_alloc schro_gpuframe_new_and_alloc
#endif

typedef struct _SchroPictureSubbandContext SchroPictureSubbandContext;

struct _SchroPictureSubbandContext {
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

static void schro_decoder_decode_macroblock(SchroPicture *picture,
    SchroArith **arith, SchroUnpack *unpack, int i, int j);
static void schro_decoder_decode_prediction_unit(SchroPicture *picture,
    SchroArith **arith, SchroUnpack *unpack, SchroMotionVector *motion_vectors, int x, int y);

#ifdef SCHRO_GPU
static void schro_decoder_decode_transform_data_serial (SchroPicture *picture, schro_subband_storage *store, SchroGPUFrame *frame);
#endif

static int schro_decoder_decode_subband (SchroPicture *picture,
    SchroPictureSubbandContext *ctx);

#ifndef ASSUME_ZERO
static void schro_decoder_zero_block (SchroPictureSubbandContext *ctx, int x1, int y1, int x2, int y2);
#endif

static void schro_decoder_error (SchroPicture *picture, const char *s);


static void schro_decoder_init(SchroDecoder *decoder);
#ifdef SCHRO_GPU
static void schro_decoder_gpu_cleanup(SchroDecoder *decoder);
#endif

/** NFA-based parallel scheduler

    - lock
    - check global quit flag to signify if thread should die
      - if so, unlock and break out
    - thread looks for work (scheduling)
      - go over all picture structures
      - find which one is most important
        - prefer reference frames, especially those that we are waiting for.
      - given curstate, find out what needs/can be done
        - iterate over schro_decoder_ops
          - if state not yet in curstate|busystate
            - see if reqstate bits are in curstate
            - if so, mark it
      - for each bit still set, take into account additional constraints, 
        like availability of reference frames
      - if we found work, break out, otherwise continue searching
      - if we found no work after searching all frames, go into sleep mode,
        wait for wake-up signal (worker_statechange)
    - mark bit in busystate
    - unlock
    - thread executes work
    - lock
    - clear bit in busystate, mark bit in curstate
    - if the final state (SCHRO_DECODER_FINISHED) is set, the decoder 
      can revert back to SCHRO_DECODER_EMPTY. In the case of GPU, 
      there should be a last final state that waits for the GPU to complete
      asynchronous processing.  After this, the structure can be re-used
      for a new frame.
      - notify main thread that a decoder completed its work and is ready
        to accept a new frame (worker_available)
    - unlock
    - wake up threads that a state changed (worker_statechange)
    - repeat from top

TODO port to real schroasync once we know what we're doing and what they're doing
*/


static void* schro_decoder_main(void *arg)
{
  SchroDecoderThread *thread = (SchroDecoderThread*)arg;
  SchroDecoder *decoder = thread->parent;
  int x,y;
  
  SCHRO_DEBUG("Starting thread %i (gpu=%i)", thread->id, thread->gpu);

  while(1)
  {
    SchroDecoderOp *op = NULL;
    SchroPicture *op_w = NULL;    
    int priority = INT_MIN;
#ifdef SCHRO_GPU
#if 0
    if(thread->gpu)
    {
      /* CUDA memory stats */
      unsigned int mfree = 0, mtotal = 0;
      cuMemGetInfo(&mfree, &mtotal);
      SCHRO_DEBUG("Memory free %.1fMB total %.1fMB", mfree/(1024.0*1024.0), mtotal/(1024.0*1024.0));
    }
#endif
#endif
    pthread_mutex_lock (&decoder->mutex);
#if 0
    SCHRO_DEBUG("Reference queue has %i, freestack has %i", decoder->reference_queue->n, decoder->free_count);
#endif
    
    /* Find work to do for this thread. */
    while(!thread->quit)
    {
      for(x=0; x<decoder->worker_count; ++x)
      {
        SchroPicture *w = decoder->workers[x];
        int curstate = w->curstate;
        int exclude = w->busystate | w->curstate;
        int w_pri;
        SchroDecoderOp *ops = schro_get_decoder_ops();
        for(y=0; ops[y].state; ++y)
        {
          /* Check if we 
             a) gpu operations in the gpu thread, cpu operations in the cpu thread
             b) haven't done this operation yet, or aren't working on it
             c) the required state exists for doing it
          */
          if(thread->gpu >= ops[y].gpu && !(exclude & ops[y].state) && (curstate & ops[y].reqstate) == ops[y].reqstate)
          {
            /* Call the specific check function to see if the decoder is ready
               for this operation. This checks for reference frames, for example. */
            if(!ops[y].check || ops[y].check(w))
              break;
          }
        }
        if(ops[y].state == 0)
          /* Continue if we found nothing to do for this frame */
          continue;
        /* Determine priority of this decoder, and compare with the
           priority of what we found up to now, so the most important
           piece of work can be picked.
           
           TODO A decoder is most important if it is a reference frame, or if
           the frame is most near to be shown.
           
           For now, just pick the one with the lowest position in the stream.
        */
        w_pri = -w->time;
        if(w_pri > priority)
        {
          priority = w_pri;
          op = &ops[y];
          op_w = w;
        }
      }
      /* Break out if we found at least one operation to perform */
      if(op != NULL)
      {
        /* Update state that we're working on this */
        op_w->busystate |= op->state;
        break;
      }
      /* If we found nothing to do, wait for a state change */
      SCHRO_DEBUG("Thread %i idle", thread->id);
      pthread_cond_wait (&decoder->worker_statechange, &decoder->mutex);
    }
    pthread_mutex_unlock (&decoder->mutex);
    
    /* Break out of main loop if quit flag is set */
    if(thread->quit)
      break;
#ifdef SCHRO_GPU
    if(thread->gpu)
    {
      /* Set up asynchronous subband transfers */
      for(x=0; x<decoder->worker_count; ++x)
      {
        schro_decoder_async_transfer(decoder->workers[x]);
      }
    }
#endif
    SCHRO_DEBUG("Thread %i working on frame %p, will do operation %04x", thread->id, op_w, op->state);

    op->exec(op_w);

    /* Update state that we've finished working on this */
    pthread_mutex_lock (&decoder->mutex);
    op_w->curstate |= op->state;
    op_w->busystate &= ~op->state;
    
    SCHRO_DEBUG("Thread %i finished on frame %p, new state is %04x", thread->id, op_w, op_w->curstate);
    
    /* Check for final state */
    if(op_w->curstate & SCHRO_DECODER_FINAL)
    {
      /* If reached, reset frame to initial state, and send a signal that
         a decoder is now available. 
       */
      SCHRO_ASSERT(op_w->busystate == 0);
      op_w->curstate = op_w->skipstate | SCHRO_DECODER_INITIAL;
      SCHRO_DEBUG("Thread %i reached final state on %p, new state is %04x", thread->id, op_w, op_w->curstate);
      /* Reset subband fifo */
#ifdef SCHRO_GPU
      op_w->subband_min = op_w->subband_max = 0;
#endif
      pthread_cond_signal (&decoder->worker_available);
    }
    
    pthread_mutex_unlock (&decoder->mutex);
    pthread_cond_broadcast (&decoder->worker_statechange);
  } 
   
#ifdef SCHRO_GPU
  /** If this is the GPU thread, make sure we have freed all GPU structures before
      exiting, otherwise there is no chance of ever doing this again.
  */
  if(thread->gpu)
  {
    schro_decoder_gpu_cleanup(decoder);
  }
#endif
  SCHRO_DEBUG("Quitting thread %i", thread->id);
  return NULL;
}

SchroDecoder *schro_decoder_new()
{
  SchroDecoder *decoder;
  int x;
  int n_threads = 0;

  decoder = malloc(sizeof(SchroDecoder));
  memset (decoder, 0, sizeof(SchroDecoder));

  pthread_mutex_init(&decoder->mutex, NULL);
  //pthread_cond_init(&decoder->reference_notfull, NULL);
  //pthread_cond_init(&decoder->reference_newframe, NULL);
  pthread_cond_init(&decoder->worker_statechange, NULL);
  pthread_cond_init(&decoder->worker_available, NULL);

#ifdef SCHRO_GPU
  decoder->reference_queue = schro_queue_new (SCHRO_LIMIT_REFERENCE_FRAMES, NULL);
//      (SchroQueueFreeFunc)schro_upsampled_gpuframe_free);
//  schro_queue_alloc_freestack(decoder->reference_queue, UQUEUE_SIZE);
#else
  decoder->reference_queue = schro_queue_new (SCHRO_LIMIT_REFERENCE_FRAMES,
      (SchroQueueFreeFunc)schro_upsampled_frame_free);
#endif

  decoder->frame_queue = schro_queue_new (SCHRO_LIMIT_REFERENCE_FRAMES*2,
      (SchroQueueFreeFunc)schro_frame_unref);
  decoder->output_queue = schro_queue_new (SCHRO_LIMIT_REFERENCE_FRAMES,
      (SchroQueueFreeFunc)schro_frame_unref);
  
  if (n_threads == 0) {
    char *s;

    s = getenv ("SCHRO_THREADS");
    if (s && s[0]) {
      char *end;
      int n;
      n = strtoul (s, &end, 0);
      if (end[0] == 0) {
        n_threads = n;
      }
    }
    if (n_threads == 0) {
      n_threads = sysconf(_SC_NPROCESSORS_CONF);
    }
    if (n_threads == 0) {
      n_threads = 1;
    }
  }

  if(n_threads>SCHRO_MAX_THREADS)
    n_threads = SCHRO_MAX_THREADS;
 
  decoder->n_threads = n_threads;
  
  /** Number of worker structures (not threads!) */
  decoder->worker_count = 10;
  
  /** Create decoder structures */
  for(x=0; x<decoder->worker_count; ++x)
  {
    SCHRO_LOG("creating decoder %i", x);
    decoder->workers[x] = schro_picture_new(decoder);
  }
  
  /** Create worker threads */
  for(x=0; x<decoder->n_threads; ++x)
  {
    decoder->threads[x].parent = decoder;
    decoder->threads[x].id = x;
#ifdef SCHRO_GPU 
    decoder->threads[x].gpu = (x==0);
#else
    decoder->threads[x].gpu = FALSE;
#endif
    decoder->threads[x].quit = 0;
    pthread_create (&decoder->threads[x].thread, NULL, schro_decoder_main, &decoder->threads[x]);
  }

  return decoder;
}

void schro_decoder_free(SchroDecoder *decoder)
{
  int x;
  
  /* Send all threads the command to quit, by decreasing
     order of id, so that the GPU thread can clean up things last
     (if there is a GPU thread)
   */
  for(x=decoder->n_threads-1; x>=0; --x)
  {
    void *ignore;
    
    SCHRO_LOG("stopping thread %i", x);

    decoder->threads[x].quit = TRUE;
    pthread_cond_broadcast (&decoder->worker_statechange);
    pthread_join (decoder->threads[x].thread, &ignore);
  }
  
#ifndef SCHRO_GPU
  for(x=0; x<decoder->worker_count; ++x)
  {
    SCHRO_LOG("freeing decoder %i", x);
    schro_picture_free(decoder->workers[x]);
  }
#endif

  schro_queue_free (decoder->output_queue);
  schro_queue_free (decoder->reference_queue);
  schro_queue_free (decoder->frame_queue);
  
  /** Destroy mutexes and conditions */
  pthread_mutex_destroy(&decoder->mutex);
  //pthread_cond_destroy(&decoder->reference_notfull);
  //pthread_cond_destroy(&decoder->reference_newframe);
  pthread_cond_destroy(&decoder->worker_statechange);
  pthread_cond_destroy(&decoder->worker_available);

  free(decoder);
}

void
schro_decoder_reset (SchroDecoder *decoder)
{
  pthread_mutex_lock (&decoder->mutex);

  schro_queue_clear (decoder->frame_queue);
  schro_queue_clear (decoder->reference_queue);
  schro_queue_clear (decoder->output_queue);

  decoder->have_access_unit = FALSE;
  decoder->next_frame_number = 0;
  decoder->have_frame_number = FALSE;

  pthread_mutex_unlock (&decoder->mutex);
}

SchroVideoFormat *
schro_decoder_get_video_format (SchroDecoder *decoder)
{
  SchroVideoFormat *format;

  format = malloc(sizeof(SchroVideoFormat));
  memcpy (format, &decoder->video_format, sizeof(SchroVideoFormat));

  return format;
}

void
schro_decoder_set_earliest_frame (SchroDecoder *decoder,
    SchroPictureNumber earliest_frame)
{
  decoder->earliest_frame = earliest_frame;
}

void
schro_decoder_set_skip_ratio (SchroDecoder *decoder, double ratio)
{
  if (ratio > 1.0) ratio = 1.0;
  if (ratio < 0.0) ratio = 0.0;
  decoder->skip_ratio = ratio;
}

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

SchroFrame *
schro_decoder_pull (SchroDecoder *decoder)
{
  SchroFrame *ret;
  pthread_mutex_lock (&decoder->mutex);

  SCHRO_DEBUG("searching for frame %d", decoder->next_frame_number);
  ret = schro_queue_remove (decoder->frame_queue, decoder->next_frame_number);
  if (ret) {
    decoder->next_frame_number++;
  }
  
  pthread_mutex_unlock (&decoder->mutex);
  return ret;
}

void
schro_decoder_push (SchroDecoder *decoder, SchroBuffer *buffer)
{
  /* no locking needed, only accessed by main thread */
  SCHRO_ASSERT(decoder->input_buffer == NULL);

  decoder->input_buffer = buffer;
}

static SchroPicture *get_free_worker(SchroDecoder *decoder)
{
  int x;
  for(x=0; x<decoder->worker_count; ++x)
  {
    if(!(decoder->workers[x]->curstate & SCHRO_DECODER_START) && decoder->workers[x]->busystate == 0)
      return decoder->workers[x];
  }
  return NULL;
}

static int schro_decoder_mintime(SchroDecoder *decoder)
{
  int x;
  int mintime = INT_MAX;
  /* minimum time of active workers */
  for(x=0; x<decoder->worker_count; ++x)
  {
    if((decoder->workers[x]->curstate & SCHRO_DECODER_START) || decoder->workers[x]->busystate != 0)
    {
      if(decoder->workers[x]->time < mintime)
        mintime = decoder->workers[x]->time;
    }
  }
  return mintime;
}

int
schro_decoder_iterate (SchroDecoder *decoder)
{
  SchroUnpack unpack;
  SchroPicture *w;
  
  if (decoder->input_buffer == NULL) {
    return SCHRO_DECODER_NEED_BITS;
  }

  schro_unpack_init_with_data (&unpack, decoder->input_buffer->data,
      decoder->input_buffer->length, 1);

  schro_decoder_decode_parse_header(decoder, &unpack);

  if (decoder->parse_code == SCHRO_PARSE_CODE_SEQUENCE_HEADER) {
    SCHRO_INFO ("decoding access unit");
    schro_decoder_decode_access_unit(decoder, &unpack);

    schro_buffer_unref (decoder->input_buffer);
    decoder->input_buffer = NULL;

    if (decoder->have_access_unit) {
      return SCHRO_DECODER_OK;
    }
    schro_decoder_init (decoder);
    decoder->have_access_unit = TRUE;
    return SCHRO_DECODER_FIRST_ACCESS_UNIT;
  }

  if (decoder->parse_code == SCHRO_PARSE_CODE_AUXILIARY_DATA) {
    int code;

    code = schro_unpack_decode_bits (&unpack, 8);

    if (code == SCHRO_AUX_DATA_MD5_CHECKSUM) {
      int i;
      for(i=0;i<16;i++){
        decoder->md5_checksum[i] = schro_unpack_decode_bits (&unpack, 8);
      }
      decoder->has_md5 = TRUE;
    }

    schro_buffer_unref (decoder->input_buffer);
    decoder->input_buffer = NULL;
    
    return SCHRO_DECODER_OK;
  }

  if (decoder->parse_code == SCHRO_PARSE_CODE_PADDING) {
    return SCHRO_DECODER_OK;
  }

  if (schro_decoder_is_end_sequence (decoder->input_buffer)) {
    SCHRO_INFO ("decoding end sequence");
    schro_buffer_unref (decoder->input_buffer);
    decoder->input_buffer = NULL;
    return SCHRO_DECODER_EOS;
  }

  if (!decoder->have_access_unit) {
    SCHRO_INFO ("no access unit -- dropping frame");
    schro_buffer_unref (decoder->input_buffer);
    decoder->input_buffer = NULL;
    return SCHRO_DECODER_OK;
  }

  if (schro_queue_is_empty (decoder->output_queue)) {
    return SCHRO_DECODER_NEED_FRAME;
  }
  
  schro_decoder_decode_picture_header(decoder, &unpack);

  if (!decoder->have_frame_number) {
    if (SCHRO_PARSE_CODE_NUM_REFS (decoder->parse_code) > 0) {
      SCHRO_ERROR("expected I frame after access unit header");
    }
    decoder->next_frame_number = decoder->picture_number;
    decoder->have_frame_number = TRUE;
    SCHRO_INFO("next frame number after seek %d", decoder->next_frame_number);
  }

  /** Retiring */
  if (SCHRO_PARSE_CODE_IS_REFERENCE (decoder->parse_code)) {
    //schro_decoder_reference_retire (decoder, decoder->retired_picture_number);
    /** Mark reference for retirement.
        Should only be done if _all_ the frames up to now have been completed.
        Record frames that are currently being processed
        Only when these are finished, retire the frame
       Record current 'time'
       at every retire check, look at the min time of running threads
    */
    if(decoder->retired_picture_number < decoder->picture_number)
    {
        SCHRO_INFO("Need to retire frame %i when mintime >= %i", decoder->retired_picture_number, decoder->time);
        /** record decoder->time, decoder->retired_picture_number */
        pthread_mutex_lock (&decoder->mutex);
        SCHRO_ASSERT(decoder->retired_count < SCHRO_RETIRE_QUEUE_SIZE);
        decoder->retired[decoder->retired_count].time = decoder->time;
        decoder->retired[decoder->retired_count].frame = decoder->retired_picture_number;
        ++decoder->retired_count;
        pthread_mutex_unlock (&decoder->mutex);
    }
  }

#if 0
  /** Skipping */
  if (SCHRO_PARSE_CODE_IS_NON_REFERENCE (decoder->parse_code) &&
       decoder->skip_value > decoder->skip_ratio) {

    decoder->skip_value = (1-SCHRO_SKIP_TIME_CONSTANT) * decoder->skip_value;
    SCHRO_INFO("skipping frame %d", decoder->picture_number);
    SCHRO_DEBUG("skip value %g ratio %g", decoder->skip_value, decoder->skip_ratio);

    schro_buffer_unref (decoder->input_buffer);
    decoder->input_buffer = NULL;

    output_picture = schro_frame_new ();
    output_picture->frame_number = decoder->picture_number;

    SCHRO_DEBUG("adding %d to queue (skipped)", output_picture->frame_number);
    schro_queue_add (decoder->frame_queue, output_picture,
        decoder->picture_number);

    return SCHRO_DECODER_OK;
  }


  decoder->skip_value = (1-SCHRO_SKIP_TIME_CONSTANT) * decoder->skip_value +
    SCHRO_SKIP_TIME_CONSTANT;
SCHRO_DEBUG("skip value %g ratio %g", decoder->skip_value, decoder->skip_ratio);
#endif

  /** Get a free decoder structure */
  SCHRO_DEBUG("Looking for free decoder");
  pthread_mutex_lock (&decoder->mutex);
  while((w = get_free_worker(decoder)) == NULL) {
    /* If current frame is available, unlock and return */
    if(schro_queue_find (decoder->frame_queue, decoder->next_frame_number))
    {
      SCHRO_DEBUG("Stopped looking, current frame is available");
      pthread_mutex_unlock (&decoder->mutex);
      return SCHRO_DECODER_OK;
    }
    pthread_cond_wait (&decoder->worker_available, &decoder->mutex);
  }
  /* Pending SCHRO_DECODER_START */
  w->busystate |= SCHRO_DECODER_START;
  pthread_mutex_unlock (&decoder->mutex);
  
  /** Propagate our settings */
  w->unpack = unpack;

  w->parse_code = decoder->parse_code;
  w->n_refs = decoder->n_refs;
  w->next_parse_offset = decoder->next_parse_offset;
  w->prev_parse_offset = decoder->prev_parse_offset;

  w->picture_number = decoder->picture_number;
  w->reference1 = decoder->reference1;
  w->reference2 = decoder->reference2;
  w->retired_picture_number = decoder->retired_picture_number;

  w->input_buffer = decoder->input_buffer;
  w->output_picture = schro_queue_pull (decoder->output_queue);
  w->has_md5 = decoder->has_md5;
  memcpy(w->md5_checksum, decoder->md5_checksum, 32);
  w->time = decoder->time;
  ++decoder->time;
  
  SCHRO_DEBUG("decode picture here -- %i with decoder %p", decoder->picture_number, w);

  /* Wake up worker threads */
  pthread_mutex_lock (&decoder->mutex);
  w->curstate |= SCHRO_DECODER_START;
  w->busystate &= ~SCHRO_DECODER_START;
  pthread_mutex_unlock (&decoder->mutex);
  pthread_cond_broadcast (&decoder->worker_statechange);
  
  /* We consumed the input buffer */
  decoder->input_buffer = NULL;  

  return SCHRO_DECODER_OK;
}

/* This must be executed in the GPU thread 
*/
static void
schro_picture_init (SchroPicture *picture)
{
  SchroFrameFormat frame_format;
  SchroVideoFormat *video_format = &picture->parent->video_format;
  int frame_width, frame_height;

  frame_format = schro_params_get_frame_format (16,
      video_format->chroma_format);
  frame_width = ROUND_UP_POW2(video_format->width,
      SCHRO_LIMIT_TRANSFORM_DEPTH + video_format->chroma_h_shift);
  frame_height = ROUND_UP_POW2(video_format->height,
      SCHRO_LIMIT_TRANSFORM_DEPTH + video_format->chroma_v_shift);
  picture->mc_tmp_frame = schro_frame_new_and_alloc (frame_format,
      frame_width, frame_height);
  picture->frame = schro_frame_new_and_alloc (frame_format,
      frame_width, frame_height);
  frame_format = schro_params_get_frame_format (8,
      video_format->chroma_format);

#ifdef SCHRO_GPU
  /** We can use the fact that there is only one GPU thread to conserve
      memory, and allocate temporary structures only once.
   */
  if(!picture->parent->planar_output_frame)
  {
    picture->parent->planar_output_frame = schro_frame_new_and_alloc (frame_format,
      video_format->width, video_format->height);
    SCHRO_DEBUG("planar output frame %dx%d",
      video_format->width, video_format->height);

    picture->parent->gupsample_temp = schro_gpuframe_new_and_alloc (frame_format,
        video_format->width*2, video_format->height*2);
  }
#else
  picture->planar_output_frame = schro_frame_new_and_alloc (frame_format,
      video_format->width, video_format->height);
  SCHRO_DEBUG("planar output frame %dx%d",
      video_format->width, video_format->height);
#endif
#ifdef SCHRO_GPU
  cudaStreamCreate(&picture->stream);
  picture->gpumotion = schro_gpumotion_new(picture->stream);
  
  schro_gpuframe_setstream(picture->mc_tmp_frame, picture->stream);
  schro_gpuframe_setstream(picture->frame, picture->stream);
#endif
  schro_decoder_skipstate(picture->parent, picture, SCHRO_DECODER_INITIALIZED);
}


    

SchroPicture *
schro_picture_new (SchroDecoder *decoder)
{
  SchroPicture *picture;

  picture = malloc(sizeof(SchroDecoder));
  memset (picture, 0, sizeof(SchroDecoder));

  picture->parent = decoder;
  picture->tmpbuf = malloc(SCHRO_LIMIT_WIDTH * 2);
  picture->tmpbuf2 = malloc(SCHRO_LIMIT_WIDTH * 2);

  picture->params.video_format = &picture->parent->video_format;

  return picture;
}

#ifdef SCHRO_GPU
#define schro_frame_unref schro_gpuframe_unref
#endif

void
schro_picture_free (SchroPicture *picture)
{
  if (picture->frame) {
    schro_frame_unref (picture->frame);
  }

  if (picture->mc_tmp_frame) schro_frame_unref (picture->mc_tmp_frame);
#ifndef SCHRO_GPU
  if (picture->planar_output_frame) schro_frame_unref (picture->planar_output_frame);
#endif
  if (picture->tmpbuf) free (picture->tmpbuf);
  if (picture->tmpbuf2) free (picture->tmpbuf2);
  if (picture->error_message) free (picture->error_message);
  
#ifdef SCHRO_GPU
  if (picture->goutput_frame) schro_gpuframe_unref (picture->goutput_frame);
  if (picture->store) schro_subband_storage_free(picture->store);
  schro_gpumotion_free(picture->gpumotion);
  cudaStreamDestroy(picture->stream);
#endif

  free (picture);
}

#undef schro_frame_unref




#ifdef SCHRO_GPU
static void
schro_picture_iterate_init_output (SchroPicture *picture)
{
  /* GPU clone output frame. This must be here as we cannot know in advance
     what frame format (interleaved, planar, bit depth) the user expects. 
   */
  if(!picture->goutput_frame)
  {
      picture->goutput_frame = schro_gpuframe_new_clone(picture->output_picture);
      schro_gpuframe_setstream(picture->goutput_frame, picture->stream);
  }
  schro_decoder_skipstate(picture->parent, picture, SCHRO_DECODER_OUTPUT_INIT);
}
#endif

static void
schro_picture_iterate_motion_decode_params (SchroPicture *picture)
{
  SchroParams *params = &picture->params;

  /* General worker state initialisation */
  picture->ref0 = NULL;
  picture->ref1 = NULL;
  params->num_refs = SCHRO_PARSE_CODE_NUM_REFS(picture->parse_code);
  params->is_lowdelay = SCHRO_PARSE_CODE_IS_LOW_DELAY(picture->parse_code);
  params->is_noarith = !SCHRO_PARSE_CODE_USING_AC(picture->parse_code);

  /* Check for motion comp data */
  if (picture->n_refs > 0) {

    SCHRO_DEBUG("inter");

    schro_unpack_byte_sync (&picture->unpack);
    schro_decoder_decode_picture_prediction_parameters (picture);
    schro_params_calculate_mc_sizes (params);

    picture->motion = schro_motion_new (params, NULL, NULL);
  }
}

#ifdef SCHRO_GPU
static void
schro_picture_iterate_motion_init (SchroPicture *picture)
{
  if (picture->n_refs > 0)
  {
    schro_gpumotion_init (picture->gpumotion, picture->motion);
    schro_decoder_skipstate(picture->parent, picture, SCHRO_DECODER_MOTION_INIT);
  }
}
#endif

static void
schro_picture_iterate_motion_decode_vectors (SchroPicture *picture)
{
  if (picture->n_refs > 0) {
    schro_unpack_byte_sync (&picture->unpack);
    schro_decoder_decode_block_data (picture);
#ifdef SCHRO_GPU
    schro_gpumotion_copy (picture->gpumotion, picture->motion);
#endif
  }
}

static void
schro_picture_iterate_wavelet_decode_params (SchroPicture *picture)  
{
  SchroParams *params = &picture->params;
  
  /* Decode transform data */
  schro_unpack_byte_sync (&picture->unpack);
  picture->zero_residual = FALSE;
  if (params->num_refs > 0) {
    picture->zero_residual = schro_unpack_decode_bit (&picture->unpack);

    SCHRO_DEBUG ("zero residual %d", picture->zero_residual);
  }

  if (!picture->zero_residual) {
    schro_decoder_decode_transform_parameters (picture);
    schro_params_calculate_iwt_sizes (params);
  }
}

#ifdef SCHRO_GPU
void
schro_picture_iterate_wavelet_init (SchroPicture *picture)
{
  if (!picture->zero_residual) {
    if(!picture->store)
        picture->store = schro_subband_storage_new(&picture->params, picture->stream);
    schro_decoder_skipstate(picture->parent, picture, SCHRO_DECODER_WAVELET_INIT);
  }
}
#endif         

static void
schro_picture_iterate_wavelet_decode_image (SchroPicture *picture)
{
  SchroParams *params = &picture->params;
  if (!picture->zero_residual) {
    schro_unpack_byte_sync (&picture->unpack);
#ifdef SCHRO_GPU
    SCHRO_ASSERT(!params->is_lowdelay);
    schro_decoder_decode_transform_data_serial(picture, picture->store, picture->frame);
#else
    if (params->is_lowdelay) {
      schro_decoder_decode_lowdelay_transform_data (picture);
    } else {
      schro_decoder_decode_transform_data (picture);
    }
#endif
  }

  /* Input buffer can be released now stream decoding is complete. */
  schro_buffer_unref (picture->input_buffer);
  picture->input_buffer = NULL;
}



static void
schro_picture_iterate_wavelet_transform (SchroPicture *picture)
{
  if (!picture->zero_residual) {
#ifdef SCHRO_GPU
    SCHRO_ASSERT(picture->subband_min == picture->subband_max);
    SCHRO_ASSERT(picture->subband_min == 3*(1+3*picture->params.transform_depth));
    schro_gpuframe_inverse_iwt_transform (picture->frame, &picture->params);
#else
    schro_frame_inverse_iwt_transform (picture->frame, &picture->params,
        picture->tmpbuf);
#endif

  }
}

static int 
schro_picture_check_refs (SchroPicture *picture)
{
  int rv;

  /** Find reference frames, if we didn't yet. We should check for frames
      that are "stuck" due to their reference frames never appearing. 
   */
  if(picture->n_refs > 0 && !picture->ref0)
    picture->ref0 = schro_decoder_reference_get (picture->parent, picture->reference1);
  if(picture->n_refs > 1 && !picture->ref1)
    picture->ref1 = schro_decoder_reference_get (picture->parent, picture->reference2);

  /** Count number of available reference frames, and compare
      to what we need. 
   */
  rv = (picture->ref0 != NULL) + (picture->ref1 != NULL);
  return rv == picture->n_refs;
}

static void
schro_picture_iterate_motion_transform (SchroPicture *picture)
{
  if (picture->n_refs > 0) {
#if 0
    /* Moved this to finish frame stage because of race conditions.
       It should really be here, though. */
    if (params->mv_precision > 0) {
      schro_upsampled_frame_upsample (picture->ref0);
      if (picture->ref1) {
        schro_upsampled_frame_upsample (picture->ref1);
      }
    }
#endif
    picture->motion->src1 = (SchroUpsampledFrame*)picture->ref0;
    picture->motion->src2 = (SchroUpsampledFrame*)picture->ref1;

#ifndef SCHRO_GPU
    schro_motion_render (picture->motion, picture->mc_tmp_frame);
#else
    schro_gpumotion_render (picture->gpumotion, picture->motion, picture->mc_tmp_frame);
#endif

    schro_motion_free (picture->motion);  
    picture->motion = NULL;
  }
}

#ifdef SCHRO_GPU
#define planar_output_frame parent->planar_output_frame
#endif

static void
schro_picture_iterate_finish (SchroPicture *picture)
{
#ifdef SCHRO_GPU
  SchroFrame *cpu_output_picture = picture->output_picture;
  SchroGPUFrame *output_picture = picture->goutput_frame;
  
  schro_gpuframe_setstream(picture->planar_output_frame, picture->stream);
#else
  SchroParams *params = &picture->params;
  SchroFrame *output_picture = picture->output_picture;
  int i;
#endif
  if (picture->zero_residual) {
    if (SCHRO_FRAME_IS_PACKED(output_picture->format)) {
      schro_frame_convert (picture->planar_output_frame, picture->mc_tmp_frame);
      schro_frame_convert (output_picture, picture->planar_output_frame);
    } else {
      schro_frame_convert (output_picture, picture->mc_tmp_frame);
    }
  } else if (!_schro_decode_prediction_only) {
    if (SCHRO_PARSE_CODE_IS_INTER(picture->parse_code)) {
      schro_frame_add (picture->frame, picture->mc_tmp_frame);
    }

    if (SCHRO_FRAME_IS_PACKED(output_picture->format)) {
      schro_frame_convert (picture->planar_output_frame, picture->frame);
      schro_frame_convert (output_picture, picture->planar_output_frame);
    } else {
      schro_frame_convert (output_picture, picture->frame);
    }
  } else {
#ifndef SCHRO_GPU
    SchroFrame *frame;
#else
    SchroGPUFrame *frame;
#endif
    if (SCHRO_PARSE_CODE_IS_INTER(picture->parse_code)) {
      frame = picture->mc_tmp_frame;
    } else {
      frame = picture->frame;
    }
    if (SCHRO_FRAME_IS_PACKED(output_picture->format)) {
      schro_frame_convert (picture->planar_output_frame, frame);
      schro_frame_convert (output_picture, picture->planar_output_frame);
    } else {
      schro_frame_convert (output_picture, frame);
    }

    if (SCHRO_PARSE_CODE_IS_INTER(picture->parse_code)) {
      schro_frame_add (picture->frame, picture->mc_tmp_frame);
    }
  }

  output_picture->frame_number = picture->picture_number;

  if (SCHRO_PARSE_CODE_IS_REFERENCE(picture->parse_code)) {
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
        picture->parent->video_format.width, picture->parent->video_format.height);
    schro_frame_convert (ref, picture->frame);
    ref->frame_number = picture->picture_number;
    
    upsampler = schro_upsampled_frame_new(ref);
    
    /* This upsampling should ideally not happen here, but only on
       demand. */
    schro_upsampled_frame_upsample (upsampler);
    
    schro_decoder_reference_add (picture->parent, upsampler, picture->picture_number);
#else
    SchroGPUFrame *ref;
    SchroUpsampledGPUFrame *rv;
    if(output_picture->format == picture->planar_output_frame->format)
    {
        /* We can skip an extra conversion if we have the output already in the format
           we want
         */
        ref = output_picture;
    } else {
        ref = picture->planar_output_frame;
        schro_gpuframe_convert (picture->planar_output_frame, picture->frame);
    }
    /* Try to re-use a frame from freestack */
    rv = schro_decoder_reference_getfree(picture->parent);
    if(rv == NULL)
        rv = schro_upsampled_gpuframe_new(&picture->parent->video_format);
    
    schro_gpuframe_setstream(picture->parent->gupsample_temp, picture->stream);
    schro_upsampled_gpuframe_upsample(rv, picture->parent->gupsample_temp, ref, &picture->parent->video_format);
    
    schro_decoder_reference_add (picture->parent, rv, picture->picture_number);
#endif
  }


#ifndef SCHRO_GPU
  if (picture->has_md5) {
    uint32_t state[4];

    schro_frame_md5 (output_picture, state);
    if (memcmp (state, picture->md5_checksum, 16) != 0) {
      char a[65];
      char b[65];
      for(i=0;i<16;i++){
        sprintf(a+2*i, "%02x", ((uint8_t *)state)[i]);
        sprintf(b+2*i, "%02x", picture->md5_checksum[i]);
      }
      SCHRO_ERROR("MD5 checksum mismatch (%s should be %s)", a, b);
    }

    picture->has_md5 = FALSE;
  }
#endif

  SCHRO_DEBUG("adding %d to queue", output_picture->frame_number);
#ifndef SCHRO_GPU
  schro_decoder_add_finished_frame (picture->parent, output_picture);
#else
#ifndef GPU_NOCOPY_OUT
  schro_gpuframe_to_cpu(cpu_output_picture, output_picture);
#else
  cpu_output_picture->frame_number = output_picture->frame_number;
#endif
  schro_decoder_add_finished_frame (picture->parent, cpu_output_picture);
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
 schro_picture_init
},
/* Start of frame decoding */
{SCHRO_DECODER_MOTION_DECODE_PARAMS,
 SCHRO_DECODER_START|SCHRO_DECODER_INITIALIZED, NULL, 
 schro_picture_iterate_motion_decode_params, 
},
{SCHRO_DECODER_MOTION_DECODE_VECTORS, 
 SCHRO_DECODER_MOTION_DECODE_PARAMS, NULL, 
 schro_picture_iterate_motion_decode_vectors, 
},
{SCHRO_DECODER_WAVELET_DECODE_PARAMS, 
 SCHRO_DECODER_MOTION_DECODE_VECTORS, NULL, 
 schro_picture_iterate_wavelet_decode_params, 
},
{SCHRO_DECODER_WAVELET_DECODE_IMAGE, 
 SCHRO_DECODER_WAVELET_DECODE_PARAMS, NULL, 
 schro_picture_iterate_wavelet_decode_image, 
},

{SCHRO_DECODER_MOTION_RENDER, 
 SCHRO_DECODER_MOTION_DECODE_VECTORS, schro_picture_check_refs, 
 schro_picture_iterate_motion_transform, 
},
{SCHRO_DECODER_WAVELET_TRANSFORM, 
 SCHRO_DECODER_WAVELET_DECODE_IMAGE, NULL, 
 schro_picture_iterate_wavelet_transform, 
},
{SCHRO_DECODER_FINISHED, 
 SCHRO_DECODER_MOTION_RENDER|SCHRO_DECODER_WAVELET_TRANSFORM, NULL, 
 schro_picture_iterate_finish, 
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
 schro_picture_init,
 TRUE
},
/* Start of frame decoding */
{SCHRO_DECODER_OUTPUT_INIT,
 SCHRO_DECODER_START|SCHRO_DECODER_INITIALIZED, NULL, 
 schro_picture_iterate_init_output, 
 TRUE
},
{SCHRO_DECODER_MOTION_DECODE_PARAMS,
 SCHRO_DECODER_START|SCHRO_DECODER_INITIALIZED, NULL, 
 schro_picture_iterate_motion_decode_params, 
 FALSE
},
{SCHRO_DECODER_MOTION_INIT, 
 SCHRO_DECODER_MOTION_DECODE_PARAMS, NULL, 
 schro_picture_iterate_motion_init, 
 TRUE
},
{SCHRO_DECODER_MOTION_DECODE_VECTORS, 
 SCHRO_DECODER_MOTION_INIT|SCHRO_DECODER_MOTION_DECODE_PARAMS, NULL, 
 schro_picture_iterate_motion_decode_vectors, 
 FALSE
},
{SCHRO_DECODER_WAVELET_DECODE_PARAMS, 
 SCHRO_DECODER_MOTION_DECODE_VECTORS, NULL, 
 schro_picture_iterate_wavelet_decode_params, 
 FALSE
},
{SCHRO_DECODER_WAVELET_INIT, 
 SCHRO_DECODER_WAVELET_DECODE_PARAMS, NULL, 
 schro_picture_iterate_wavelet_init, 
 TRUE
},
{SCHRO_DECODER_WAVELET_DECODE_IMAGE, 
 SCHRO_DECODER_WAVELET_INIT|SCHRO_DECODER_WAVELET_DECODE_PARAMS, NULL, 
 schro_picture_iterate_wavelet_decode_image, 
 FALSE
},

{SCHRO_DECODER_MOTION_RENDER, 
 SCHRO_DECODER_MOTION_DECODE_VECTORS, schro_picture_check_refs, 
 schro_picture_iterate_motion_transform, 
 TRUE
},
{SCHRO_DECODER_WAVELET_TRANSFORM, 
 SCHRO_DECODER_WAVELET_DECODE_IMAGE, NULL, 
 schro_picture_iterate_wavelet_transform, 
 TRUE
},
{SCHRO_DECODER_FINISHED, 
 SCHRO_DECODER_OUTPUT_INIT|SCHRO_DECODER_MOTION_RENDER|SCHRO_DECODER_WAVELET_TRANSFORM, NULL, 
 schro_picture_iterate_finish, 
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
schro_decoder_decode_parse_header (SchroDecoder *decoder, SchroUnpack *unpack)
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

  decoder->parse_code = schro_unpack_decode_bits (unpack, 8);
  SCHRO_DEBUG ("parse code %02x", decoder->parse_code);

  decoder->n_refs = SCHRO_PARSE_CODE_NUM_REFS(decoder->parse_code);
  SCHRO_DEBUG("n_refs %d", decoder->n_refs);

  decoder->next_parse_offset = schro_unpack_decode_bits (unpack, 32);
  SCHRO_DEBUG ("next_parse_offset %d", decoder->next_parse_offset);
  decoder->prev_parse_offset = schro_unpack_decode_bits (unpack, 32);
  SCHRO_DEBUG ("prev_parse_offset %d", decoder->prev_parse_offset);
}

void
schro_decoder_decode_access_unit (SchroDecoder *decoder, SchroUnpack *unpack)
{
  int bit;
  int index;
  SchroVideoFormat *format = &decoder->video_format;

  SCHRO_DEBUG("decoding access unit");

  /* parse parameters */
  decoder->major_version = schro_unpack_decode_uint (unpack);
  SCHRO_DEBUG("major_version = %d", decoder->major_version);
  decoder->minor_version = schro_unpack_decode_uint (unpack);
  SCHRO_DEBUG("minor_version = %d", decoder->minor_version);
  decoder->profile = schro_unpack_decode_uint (unpack);
  SCHRO_DEBUG("profile = %d", decoder->profile);
  decoder->level = schro_unpack_decode_uint (unpack);
  SCHRO_DEBUG("level = %d", decoder->level);

  if (decoder->major_version != 0 || decoder->minor_version != 20071203) {
    SCHRO_ERROR("Expecting version number 0.20071203, got %d.%d",
        decoder->major_version, decoder->minor_version);
    //SCHRO_MILD_ASSERT(0);
  }
  if (decoder->profile != 0 || decoder->level != 0) {
    SCHRO_ERROR("Expecting profile/level 0,0, got %d,%d",
        decoder->profile, decoder->level);
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

  decoder->interlaced_coding = schro_unpack_decode_uint (unpack);

  MARKER();

  schro_video_format_validate (format);
}

void
schro_decoder_decode_picture_header (SchroDecoder *decoder, SchroUnpack *unpack)
{
  schro_unpack_byte_sync(unpack);

  decoder->picture_number = schro_unpack_decode_bits (unpack, 32);
  SCHRO_DEBUG("picture number %d", decoder->picture_number);

  if (decoder->n_refs > 0) {
    decoder->reference1 = decoder->picture_number +
      schro_unpack_decode_sint (unpack);
    SCHRO_DEBUG("ref1 %d", decoder->reference1);
  }

  if (decoder->n_refs > 1) {
    decoder->reference2 = decoder->picture_number +
      schro_unpack_decode_sint (unpack);
    SCHRO_DEBUG("ref2 %d", decoder->reference2);
  }

  if (SCHRO_PARSE_CODE_IS_REFERENCE(decoder->parse_code)) {
    decoder->retired_picture_number = decoder->picture_number +
      schro_unpack_decode_sint (unpack);
  }
}

void
schro_decoder_decode_picture_prediction_parameters (SchroPicture *picture)
{
  SchroParams *params = &picture->params;
  SchroUnpack *unpack = &picture->unpack;
  int bit;
  int index;

  /* block parameters */
  index = schro_unpack_decode_uint (unpack);
  if (index == 0) {
    params->xblen_luma = schro_unpack_decode_uint (unpack);
    params->yblen_luma = schro_unpack_decode_uint (unpack);
    params->xbsep_luma = schro_unpack_decode_uint (unpack);
    params->ybsep_luma = schro_unpack_decode_uint (unpack);
  } else {
    schro_params_set_block_params (params, index);
  }
  SCHRO_DEBUG("blen_luma %d %d bsep_luma %d %d",
      params->xblen_luma, params->yblen_luma,
      params->xbsep_luma, params->ybsep_luma);

  MARKER();

  /* mv precision */
  params->mv_precision = schro_unpack_decode_uint (unpack);
  SCHRO_DEBUG("mv_precision %d", params->mv_precision);

  MARKER();

  /* global motion */
  params->have_global_motion = schro_unpack_decode_bit (unpack);
  if (params->have_global_motion) {
    int i;

    for (i=0;i<params->num_refs;i++) {
      SchroGlobalMotion *gm = params->global_motion + i;

      /* pan/tilt */
      bit = schro_unpack_decode_bit (unpack);
      if (bit) {
        gm->b0 = schro_unpack_decode_sint (unpack);
        gm->b1 = schro_unpack_decode_sint (unpack);
      } else {
        gm->b0 = 0;
        gm->b1 = 0;
      }

      /* zoom/rotate/shear */
      bit = schro_unpack_decode_bit (unpack);
      if (bit) {
        gm->a_exp = schro_unpack_decode_uint (unpack);
        gm->a00 = schro_unpack_decode_sint (unpack);
        gm->a01 = schro_unpack_decode_sint (unpack);
        gm->a10 = schro_unpack_decode_sint (unpack);
        gm->a11 = schro_unpack_decode_sint (unpack);
      } else {
        gm->a_exp = 0;
        gm->a00 = 1;
        gm->a01 = 0;
        gm->a10 = 0;
        gm->a11 = 1;
      }

      /* perspective */
      bit = schro_unpack_decode_bit (unpack);
      if (bit) {
        gm->c_exp = schro_unpack_decode_uint (unpack);
        gm->c0 = schro_unpack_decode_sint (unpack);
        gm->c1 = schro_unpack_decode_sint (unpack);
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
  params->picture_pred_mode = schro_unpack_decode_uint (unpack);
  if (params->picture_pred_mode != 0) {
    schro_decoder_error (picture, "picture prediction mode != 0");
  }

  /* reference picture weights */
  params->picture_weight_bits = 1;
  params->picture_weight_1 = 1;
  params->picture_weight_2 = 1;
  bit = schro_unpack_decode_bit (unpack);
  if (bit) {
    params->picture_weight_bits = schro_unpack_decode_uint (unpack);
    params->picture_weight_1 = schro_unpack_decode_sint (unpack);
    if (params->num_refs > 1) {
      params->picture_weight_2 = schro_unpack_decode_sint (unpack);
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
schro_decoder_decode_block_data (SchroPicture *picture)
{
  SchroParams *params = &picture->params;
  SchroArith *arith[9];
  SchroUnpack unpack[9];
  int i, j;

  memset(picture->motion->motion_vectors, 0,
      sizeof(SchroMotionVector)*params->y_num_blocks*params->x_num_blocks);

  for(i=0;i<9;i++){
    SchroBuffer *buffer;
    int length;

    if (params->num_refs < 2 && (i == SCHRO_DECODER_ARITH_VECTOR_REF2_X ||
          i == SCHRO_DECODER_ARITH_VECTOR_REF2_Y)) {
      arith[i] = NULL;
      continue;
    }
    length = schro_unpack_decode_uint (&picture->unpack);
    schro_unpack_byte_sync (&picture->unpack);
    buffer = schro_buffer_new_subbuffer (picture->input_buffer,
        schro_unpack_get_bits_read (&picture->unpack)/8, length);

    if (!params->is_noarith) {
      arith[i] = schro_arith_new ();
      schro_arith_decode_init (arith[i], buffer);

      schro_unpack_skip_bits (&picture->unpack, length*8);
    } else {
      schro_unpack_copy (unpack + i, &picture->unpack);
      schro_unpack_limit_bits_remaining (unpack + i, length*8);
      schro_unpack_skip_bits (&picture->unpack, length*8);
    }
  }

  for(j=0;j<params->y_num_blocks;j+=4){
    for(i=0;i<params->x_num_blocks;i+=4){
      schro_decoder_decode_macroblock(picture, arith, unpack, i, j);
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
schro_decoder_decode_macroblock(SchroPicture *picture, SchroArith **arith,
    SchroUnpack *unpack, int i, int j)
{
  SchroParams *params = &picture->params;
  SchroMotion *motion = picture->motion;
  SchroMotionVector *mv = &motion->motion_vectors[j*params->x_num_blocks + i];
  int k,l;
  int split_prediction;

  split_prediction = schro_motion_split_prediction (motion, i, j);
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
      schro_decoder_decode_prediction_unit (picture, arith, unpack,
          motion->motion_vectors, i, j);
      mv[1] = mv[0];
      mv[2] = mv[0];
      mv[3] = mv[0];
      memcpy(mv + params->x_num_blocks, mv, 4*sizeof(*mv));
      memcpy(mv + 2*params->x_num_blocks, mv, 4*sizeof(*mv));
      memcpy(mv + 3*params->x_num_blocks, mv, 4*sizeof(*mv));
      break;
    case 1:
      schro_decoder_decode_prediction_unit (picture, arith, unpack,
          motion->motion_vectors, i, j);
      mv[1] = mv[0];
      schro_decoder_decode_prediction_unit (picture, arith, unpack,
          motion->motion_vectors, i + 2, j);
      mv[3] = mv[2];
      memcpy(mv + params->x_num_blocks, mv, 4*sizeof(*mv));

      mv += 2*params->x_num_blocks;
      schro_decoder_decode_prediction_unit (picture, arith, unpack,
          motion->motion_vectors, i, j + 2);
      mv[1] = mv[0];
      schro_decoder_decode_prediction_unit (picture, arith, unpack,
          motion->motion_vectors, i + 2, j + 2);
      mv[3] = mv[2];
      memcpy(mv + params->x_num_blocks, mv, 4*sizeof(*mv));
      break;
    case 2:
      for (l=0;l<4;l++) {
        for (k=0;k<4;k++) {
          schro_decoder_decode_prediction_unit (picture, arith, unpack,
              motion->motion_vectors, i + k, j + l);
        }
      }
      break;
    default:
      SCHRO_ERROR("mv->split == %d, split_prediction %d", mv->split, split_prediction);
      SCHRO_ASSERT(0);
  }
}

void
schro_decoder_decode_prediction_unit(SchroPicture *picture, SchroArith **arith,
    SchroUnpack *unpack, SchroMotionVector *motion_vectors, int x, int y)
{
  SchroParams *params = &picture->params;
  SchroMotion *motion = picture->motion;
  SchroMotionVector *mv = &motion_vectors[y*params->x_num_blocks + x];

  mv->pred_mode = schro_motion_get_mode_prediction (motion,
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

    schro_motion_dc_prediction (motion, x, y, pred);

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
      pred = schro_motion_get_global_prediction (motion, x, y);
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
        schro_motion_vector_prediction (motion, x, y,
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
        schro_motion_vector_prediction (motion, x, y,
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
schro_decoder_decode_transform_parameters (SchroPicture *picture)
{
  int bit;
  int i;
  SchroParams *params = &picture->params;
  SchroUnpack *unpack = &picture->unpack;

  /* transform */
  params->wavelet_filter_index = schro_unpack_decode_uint (unpack);
  SCHRO_DEBUG ("wavelet filter index %d", params->wavelet_filter_index);

  /* transform depth */
  params->transform_depth = schro_unpack_decode_uint (unpack);
  SCHRO_DEBUG ("transform depth %d", params->transform_depth);

  if (!SCHRO_PARSE_CODE_IS_LOW_DELAY(picture->parse_code)) {
    /* codeblock parameters */
    params->codeblock_mode_index = 0;
    for(i=0;i<params->transform_depth + 1;i++) {
      params->horiz_codeblocks[i] = 1;
      params->vert_codeblocks[i] = 1;
    }

    bit = schro_unpack_decode_bit (unpack);
    if (bit) {
      int i;
      for(i=0;i<params->transform_depth + 1;i++) {
        params->horiz_codeblocks[i] = schro_unpack_decode_uint (unpack);
        params->vert_codeblocks[i] = schro_unpack_decode_uint (unpack);
      }
      params->codeblock_mode_index = schro_unpack_decode_uint (unpack);
    }
  } else {
    /* slice parameters */
    params->n_horiz_slices = schro_unpack_decode_uint(unpack);
    params->n_vert_slices = schro_unpack_decode_uint(unpack);

    params->slice_bytes_num = schro_unpack_decode_uint(unpack);
    params->slice_bytes_denom = schro_unpack_decode_uint(unpack);

    /* quant matrix */
    bit = schro_unpack_decode_bit (unpack);
    if (bit) {
      params->quant_matrix[0] = schro_unpack_decode_uint (unpack);
      for(i=0;i<params->transform_depth;i++){
        params->quant_matrix[1+3*i] = schro_unpack_decode_uint (unpack);
        params->quant_matrix[2+3*i] = schro_unpack_decode_uint (unpack);
        params->quant_matrix[3+3*i] = schro_unpack_decode_uint (unpack);
      }
    } else {
      schro_params_set_default_quant_matrix (params);
    }
  }
}

#ifndef SCHRO_GPU
void
schro_decoder_decode_transform_data (SchroPicture *picture)
{
  int i;
  int component;
  SchroParams *params = &picture->params;
  SchroPictureSubbandContext context = { 0 }, *ctx = &context;

  for(component=0;component<3;component++){
    for(i=0;i<1+3*params->transform_depth;i++) {
      ctx->component = component;
      ctx->position = schro_subband_get_position(i);

      schro_subband_get (picture->frame, ctx->component, ctx->position, &picture->params, &ctx->data, &ctx->stride, &ctx->width, &ctx->height);
      if (ctx->position >= 4) {
          schro_subband_get (picture->frame, ctx->component, ctx->position - 4,
            &picture->params, &ctx->parent_data, &ctx->parent_stride, &ctx->parent_width, &ctx->parent_height);
      } else {
          ctx->parent_data = NULL;
          ctx->parent_stride = 0;
      }

      schro_unpack_byte_sync (&picture->unpack);
      if(!schro_decoder_decode_subband (picture, ctx))
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
schro_decoder_decode_transform_data_serial (SchroPicture *picture, schro_subband_storage *store, SchroGPUFrame *frame)
{
  int i;
  int component;
  SchroParams *params = &picture->params;
  SchroPictureSubbandContext context = { 0 }, *ctx = &context;
  int total_length = 0;
  int bandid = 0;

  for(component=0;component<3;component++) {
    for(i=0;i<1+3*params->transform_depth;i++) {
    
      // 64 byte (32 pixel) align up
      total_length = ROUND_UP_POW2(total_length,5);

      ctx->component = component;
      ctx->position = schro_subband_get_position(i);
      ctx->data = &store->data[total_length];

      schro_subband_get_s(ctx->component, ctx->position, &picture->params, &ctx->stride, &ctx->width, &ctx->height);

      int length = ctx->height*ctx->stride/2;

      SCHRO_ASSERT((total_length+length) <= store->maxsize);

      if (ctx->position >= 4) {
          schro_subband_get_s(ctx->component, ctx->position-4, &picture->params, &ctx->parent_stride, &ctx->parent_width, &ctx->parent_height);
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
      schro_unpack_byte_sync (&picture->unpack);
      if(schro_decoder_decode_subband (picture, ctx))
      {
          store->offsets[bandid] = total_length;
          total_length += length;
      }
      else
      {
          /** Empty block */
          store->offsets[bandid] = -1;
      }
      /* SCHRO_ERROR("%p %i (%i %i)", picture, bandid, ctx->component, ctx->position); */
      ++bandid;
      /** Fire off asynchronous transfer to GPU */
      picture->subband_max = bandid;
    }
  }
  store->used = total_length;
  SCHRO_DEBUG("coefficients in buffer: %i (%f%%)", total_length, 100.0*(double)total_length/store->maxsize);
}

void schro_decoder_async_transfer(SchroPicture *picture)
{
  /* Start transfers in FIFO order */
  int x;
  int limit = picture->subband_max;
  int subbands_per_component = 1+3*picture->params.transform_depth;
  /* SCHRO_ERROR("%i %i", picture->subband_min, limit); */
  for(x = picture->subband_min; x < limit; ++x)
  {
    int component, position;
    if(x == 0)
      schro_subband_storage_to_gpuframe_init(picture->store, picture->frame);
    component = x / subbands_per_component;
    position = schro_subband_get_position(x % subbands_per_component);
    /* SCHRO_ERROR("%p %i/%i/%i (%i %i)", picture, x, limit, subbands_per_component*3, component, position); */
    schro_subband_storage_to_gpuframe(picture->store, picture->frame, 
      component, position, picture->store->offsets[x]);
  }
  picture->subband_min = x;
  SCHRO_ASSERT(x == limit);
}
#endif

static void
codeblock_line_decode_generic (SchroPictureSubbandContext *ctx,
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
codeblock_line_decode_noarith (SchroPictureSubbandContext *ctx,
    int16_t *line)
{
  int i;

  for(i=ctx->xmin;i<ctx->xmax;i++){
    line[i] = schro_dequantise (schro_unpack_decode_sint (&ctx->unpack),
        ctx->quant_factor, ctx->quant_offset);
  }
}

#if 0
static void
codeblock_line_decode_deep (SchroPictureSubbandContext *ctx,
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
codeblock_line_decode_deep_parent (SchroPictureSubbandContext *ctx,
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
codeblock_line_decode_p_horiz (SchroPictureSubbandContext *ctx,
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
codeblock_line_decode_p_vert (SchroPictureSubbandContext *ctx,
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
codeblock_line_decode_p_diag (SchroPictureSubbandContext *ctx,
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
schro_decoder_setup_codeblocks (SchroPicture *picture,
    SchroPictureSubbandContext *ctx)
{
  SchroParams *params = &picture->params;

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
schro_decoder_zero_block (SchroPictureSubbandContext *ctx,
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
schro_decoder_decode_codeblock (SchroPicture *picture,
    SchroPictureSubbandContext *ctx)
{
  SchroParams *params = &picture->params;
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
      codeblock_line_decode_noarith (ctx, p);
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
schro_decoder_decode_subband (SchroPicture *picture,
    SchroPictureSubbandContext *ctx)
{
  SchroParams *params = &picture->params;
  SchroUnpack *unpack = &picture->unpack;
  int x,y;
  SchroBuffer *buffer = NULL;

  ctx->subband_length = schro_unpack_decode_uint (unpack);
  SCHRO_DEBUG("subband %d %d length %d", ctx->component, ctx->position,
      ctx->subband_length);

  if (ctx->subband_length == 0) {
    SCHRO_DEBUG("subband is zero");
    schro_unpack_byte_sync (unpack);
    //schro_decoder_zero_block (ctx, 0, 0, ctx->width, ctx->height);
    return 0;
  }

  ctx->quant_index = schro_unpack_decode_uint (unpack);
  SCHRO_DEBUG("quant index %d", ctx->quant_index);

  /* FIXME check quant_index */
  SCHRO_MILD_ASSERT(ctx->quant_index >= 0);
  SCHRO_MILD_ASSERT(ctx->quant_index <= 60);

  schro_unpack_byte_sync (unpack);
  if (!params->is_noarith) {
    buffer = schro_buffer_new_subbuffer (picture->input_buffer,
        schro_unpack_get_bits_read (unpack)/8,
        ctx->subband_length);

    ctx->arith = schro_arith_new ();
    schro_arith_decode_init (ctx->arith, buffer);
  } else {
    schro_unpack_copy (&ctx->unpack, unpack);
    schro_unpack_limit_bits_remaining (&ctx->unpack, ctx->subband_length*8);
  }

  schro_decoder_setup_codeblocks (picture, ctx);

  for(y=0;y<ctx->vert_codeblocks;y++){
    ctx->ymin = (ctx->height*y)/ctx->vert_codeblocks;
    ctx->ymax = (ctx->height*(y+1))/ctx->vert_codeblocks;

    for(x=0;x<ctx->horiz_codeblocks;x++){

      ctx->xmin = (ctx->width*x)/ctx->horiz_codeblocks;
      ctx->xmax = (ctx->width*(x+1))/ctx->horiz_codeblocks;
      
      schro_decoder_decode_codeblock (picture, ctx);
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
  schro_unpack_skip_bits (unpack, ctx->subband_length*8);

  if (ctx->position == 0 && picture->n_refs == 0) {
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
schro_decoder_error (SchroPicture *picture, const char *s)
{
  SCHRO_DEBUG("decoder error");
  picture->error = TRUE;
  if (!picture->error_message) {
    picture->error_message = strdup(s);
  }
}

/* reference pool */

void
schro_decoder_add_output_picture (SchroDecoder *decoder, SchroFrame *frame)
{
  /* no locking needed, only accessed by main thread */
  schro_queue_add (decoder->output_queue, frame, 0);
}


#ifdef SCHRO_GPU
void
schro_decoder_reference_add (SchroDecoder *decoder, SchroUpsampledGPUFrame *frame,
    SchroPictureNumber picture_number)
#else
void
schro_decoder_reference_add (SchroDecoder *decoder, SchroUpsampledFrame *frame,
    SchroPictureNumber picture_number)
#endif
{
  pthread_mutex_lock (&decoder->mutex);
  SCHRO_DEBUG("adding %d", picture_number);
  
  if(schro_queue_is_full(decoder->reference_queue)) {
    SCHRO_ERROR("Reference queue is full -- dropping frame");
#ifdef SCHRO_GPU
    schro_upsampled_gpuframe_free(frame);
#else
    schro_upsampled_frame_free(frame);
#endif
  }
  else
  {
    schro_queue_add (decoder->reference_queue, frame, picture_number);
  }
  pthread_mutex_unlock (&decoder->mutex);
}

#ifdef SCHRO_GPU
SchroUpsampledGPUFrame *
schro_decoder_reference_get (SchroDecoder *decoder,
    SchroPictureNumber picture_number)
#else
SchroUpsampledFrame *
schro_decoder_reference_get (SchroDecoder *decoder,
    SchroPictureNumber picture_number)
#endif
{
  void *ret;

  SCHRO_DEBUG("getting %d", picture_number);
  ret = schro_queue_find (decoder->reference_queue, picture_number); 
  
  return ret;
}

#ifdef SCHRO_GPU
void *schro_decoder_reference_getfree(SchroDecoder *decoder)
{
  void *ret = NULL;
  pthread_mutex_lock (&decoder->mutex);
  if(decoder->free_count)
  {
    --decoder->free_count;
    ret = decoder->free_stack[decoder->free_count];
  }
  SCHRO_DEBUG("getfree %p, %i left", ret, decoder->free_count);
  pthread_mutex_unlock (&decoder->mutex);
  return ret;
}
#endif

static void
schro_retirement_check(SchroDecoder *decoder)
{
  int mintime, x;

  pthread_mutex_lock (&decoder->mutex);
  mintime = schro_decoder_mintime(decoder);
  SCHRO_DEBUG("mintime is %i", mintime);
  
  for(x=0; x<decoder->retired_count && mintime >= decoder->retired[x].time; ++x)
  {
    SCHRO_DEBUG("retiring %i", decoder->retired[x].frame);
#ifdef SCHRO_GPU
    /* Retired frames will not actually be freed, but moved to the free
       stack, otherwise we run into problems, as the delete function for GPU
       frames must be called in the GPU thread. This also gives a slight
       performance gain.
     */
    SCHRO_ASSERT(decoder->free_count < UQUEUE_SIZE);
    decoder->free_stack[decoder->free_count] = schro_queue_find(decoder->reference_queue, decoder->retired[x].frame);
    if(decoder->free_stack[decoder->free_count])
      ++decoder->free_count;
#endif
    schro_queue_delete (decoder->reference_queue, decoder->retired[x].frame);
  }
  
  memmove(&decoder->retired[0], &decoder->retired[x], (decoder->retired_count-x)*sizeof(struct _RetireEntry));
  decoder->retired_count -= x;
  
  pthread_mutex_unlock (&decoder->mutex);
  SCHRO_DEBUG("retired %i of %i in retirement queue", x, x+decoder->retired_count);
}

void 
schro_decoder_add_finished_frame (SchroDecoder *decoder, SchroFrame *output_picture)
{
  pthread_mutex_lock (&decoder->mutex);
  schro_queue_add (decoder->frame_queue, output_picture,
      output_picture->frame_number);
  SCHRO_DEBUG("finished decoding frame %i", output_picture->frame_number);
  pthread_mutex_unlock (&decoder->mutex);

  /* We can retire frames that are marked retired in frame
     <= output_picture->frame_number here 
  */
  schro_retirement_check(decoder);

}

void schro_decoder_skipstate (SchroDecoder *decoder, SchroPicture *w, int state)
{
  pthread_mutex_lock (&decoder->mutex);
  w->skipstate |= state;
  pthread_mutex_unlock (&decoder->mutex);
}

static void schro_decoder_init (SchroDecoder *decoder)
{
  /* Send a message to subthreads to initialize here,
     as the video format is known now.
  */
  int x;
  pthread_mutex_lock (&decoder->mutex);
  for(x=0; x<decoder->worker_count; ++x)
  {
      decoder->workers[x]->curstate |= SCHRO_DECODER_HAVE_ACCESS_UNIT;
      decoder->workers[x]->skipstate |= SCHRO_DECODER_HAVE_ACCESS_UNIT;
  }
  pthread_mutex_unlock (&decoder->mutex);
  pthread_cond_broadcast (&decoder->worker_statechange);
}

#ifdef SCHRO_GPU
static void schro_decoder_gpu_cleanup(SchroDecoder *decoder)
{
  int x;
  SCHRO_DEBUG("Cleaning up GPU structures");
  /* Release workers */
  for(x=0; x<decoder->worker_count; ++x) {
    schro_picture_free(decoder->workers[x]);
  }
  /* Release reference queue entries */
  for(x=0;x<decoder->reference_queue->n;x++){
    schro_upsampled_gpuframe_free(decoder->reference_queue->elements[x].data);
  }
  /* Release freestack entries */
  for(x=0;x<decoder->free_count;x++){
    schro_upsampled_gpuframe_free(decoder->free_stack[x]);
  }
  /* Release temporary frames */
  if(decoder->planar_output_frame)
    schro_gpuframe_unref(decoder->planar_output_frame);
  if(decoder->gupsample_temp)
    schro_gpuframe_unref(decoder->gupsample_temp);
}
#endif
