#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
//#define SCHRO_ARITH_DEFINE_INLINE
#include <schroedinger/schro.h>
#include <schroedinger/schrodecoderworker.h>

#include <liboil/liboil.h>
#include <schroedinger/schrooil.h>
#include <string.h>
#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#ifdef SCHRO_GPU
#include <cuda.h>
#endif

static void schro_decoder_init(SchroDecoder *decoder);
#ifdef SCHRO_GPU
static void schro_decoder_gpu_cleanup(SchroDecoder *decoder);
#endif

/** NFA-based parallel scheduler

    - lock
    - check global quit flag to signify if thread should die
      - if so, unlock and break out
    - thread looks for work (scheduling)
      - go over all decoderworker structures
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
    SchroDecoderWorker *op_w = NULL;    
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
        SchroDecoderWorker *w = decoder->workers[x];
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
          if(thread->gpu == ops[y].gpu && !(exclude & ops[y].state) && (curstate & ops[y].reqstate) == ops[y].reqstate)
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
  pthread_cond_init(&decoder->reference_notfull, NULL);
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
#ifdef SCHRO_GPU 
  n_threads += 1; /* add special gpu-only thread */
#endif
  if(n_threads>SCHRO_MAX_THREADS)
    n_threads = SCHRO_MAX_THREADS;
 
  decoder->n_threads = n_threads;
  
  /** Number of worker structures (not threads!) */
  decoder->worker_count = 10;
  
  /** Create decoder structures */
  for(x=0; x<decoder->worker_count; ++x)
  {
    SCHRO_LOG("creating decoder %i", x);
    decoder->workers[x] = schro_decoderworker_new();
    decoder->workers[x]->parent = decoder;
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
    schro_decoderworker_free(decoder->workers[x]);
  }
#endif

  schro_queue_free (decoder->output_queue);
  schro_queue_free (decoder->reference_queue);
  schro_queue_free (decoder->frame_queue);
  
  /** Destroy mutexes and conditions */
  pthread_mutex_destroy(&decoder->mutex);
  pthread_cond_destroy(&decoder->reference_notfull);
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
  memcpy (format, &decoder->settings.video_format, sizeof(SchroVideoFormat));

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

static SchroDecoderWorker *get_free_worker(SchroDecoder *decoder)
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
  SchroDecoderParseHeader hdr;
  SchroDecoderPictureHeader pichdr;
  SchroDecoderWorker *w;
  
  if (decoder->input_buffer == NULL) {
    return SCHRO_DECODER_NEED_BITS;
  }

  schro_unpack_init_with_data (&unpack, decoder->input_buffer->data,
      decoder->input_buffer->length, 1);

  schro_decoder_decode_parse_header(&hdr, &unpack);

  if (hdr.parse_code == SCHRO_PARSE_CODE_SEQUENCE_HEADER) {
    SCHRO_INFO ("decoding access unit");
    schro_decoder_decode_access_unit(&decoder->settings, &unpack);

    schro_buffer_unref (decoder->input_buffer);
    decoder->input_buffer = NULL;

    if (decoder->have_access_unit) {
      return SCHRO_DECODER_OK;
    }
    schro_decoder_init (decoder);
    decoder->have_access_unit = TRUE;
    return SCHRO_DECODER_FIRST_ACCESS_UNIT;
  }

  if (hdr.parse_code == SCHRO_PARSE_CODE_AUXILIARY_DATA) {
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

  if (hdr.parse_code == SCHRO_PARSE_CODE_PADDING) {
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
  
  schro_decoder_decode_picture_header(&pichdr, &unpack, &hdr);

  if (!decoder->have_frame_number) {
    if (SCHRO_PARSE_CODE_NUM_REFS (hdr.parse_code) > 0) {
      SCHRO_ERROR("expected I frame after access unit header");
    }
    decoder->next_frame_number = pichdr.picture_number;
    decoder->have_frame_number = TRUE;
    SCHRO_INFO("next frame number after seek %d", decoder->next_frame_number);
  }

  /** Retiring */
  if (SCHRO_PARSE_CODE_IS_REFERENCE (hdr.parse_code)) {
    //schro_decoder_reference_retire (decoder, decoder->retired_picture_number);
    /** Mark reference for retirement.
        Should only be done if _all_ the frames up to now have been completed.
        Record frames that are currently being processed
        Only when these are finished, retire the frame
       Record current 'time'
       at every retire check, look at the min time of running threads
    */
    if(pichdr.retired_picture_number < pichdr.picture_number)
    {
        SCHRO_INFO("Need to retire frame %i when mintime >= %i", pichdr.retired_picture_number, decoder->time);
        /** record decoder->time, pichdr.retired_picture_number */
        pthread_mutex_lock (&decoder->mutex);
        SCHRO_ASSERT(decoder->retired_count < SCHRO_RETIRE_QUEUE_SIZE);
        decoder->retired[decoder->retired_count].time = decoder->time;
        decoder->retired[decoder->retired_count].frame = pichdr.retired_picture_number;
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
  w->settings = decoder->settings;
  w->header = hdr;
  w->pichdr = pichdr;
  w->input_buffer = decoder->input_buffer;
  w->output_picture = schro_queue_pull (decoder->output_queue);
  w->has_md5 = decoder->has_md5;
  memcpy(w->md5_checksum, decoder->md5_checksum, 32);
  w->time = decoder->time;
  ++decoder->time;
  
  SCHRO_DEBUG("decode picture here -- %i with decoder %p", pichdr.picture_number, w);

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
  
  /* Block while queue is full */
  while(schro_queue_is_full(decoder->reference_queue)) {
    pthread_cond_wait (&decoder->reference_notfull, &decoder->mutex);
  }
  schro_queue_add (decoder->reference_queue, frame, picture_number);
  pthread_mutex_unlock (&decoder->mutex);
#if 0
  /** Broadcast signal that a new frame is waiting */
  pthread_cond_broadcast(&decoder->reference_newframe);
#else
  /** Wake up worker threads that might be waiting for this reference to appear */
  //pthread_cond_broadcast (&decoder->worker_statechange);
#endif
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
  if(x != 0)
    pthread_cond_signal(&decoder->reference_notfull);
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

void schro_decoder_skipstate (SchroDecoder *decoder, SchroDecoderWorker *w, int state)
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
      decoder->workers[x]->settings = decoder->settings;
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
    schro_decoderworker_free(decoder->workers[x]);
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
