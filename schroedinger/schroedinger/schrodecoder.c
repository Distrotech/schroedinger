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

static void schro_decoder_init(SchroDecoder *decoder);

static void* schro_decoder_main(void *arg)
{
  SchroDecoderWorker *decoder = arg;
  int newstate;
  SCHRO_DEBUG("Starting thread %p", decoder);
  while(1)
  {
    pthread_mutex_lock (&decoder->parent->mutex);
    while(decoder->state == SCHRO_DECODER_FREE || decoder->state == SCHRO_DECODER_PREPARING)
      pthread_cond_wait (&decoder->parent->worker_statechange, &decoder->parent->mutex);
    newstate = decoder->state;
    pthread_mutex_unlock (&decoder->parent->mutex);
    
    SCHRO_DEBUG("Thread %p got new state %i", decoder, newstate);

    if(newstate == SCHRO_DECODER_QUIT)
      break;
      
    schro_decoderworker_iterate(decoder);
    SCHRO_DEBUG("Thread %p finished", decoder);
    schro_decoder_set_worker_state(decoder->parent, decoder, SCHRO_DECODER_FREE);
  }
  
  SCHRO_DEBUG("Quitting thread %p", decoder);
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
  pthread_cond_init(&decoder->reference_newframe, NULL);
  pthread_cond_init(&decoder->worker_statechange, NULL);

#ifdef SCHRO_GPU
  decoder->reference_queue = schro_queue_new (SCHRO_LIMIT_REFERENCE_FRAMES,
      (SchroQueueFreeFunc)schro_upsampled_gpuframe_free);
  schro_queue_alloc_freestack(decoder->reference_queue, UQUEUE_SIZE);
#else
  decoder->reference_queue = schro_queue_new (SCHRO_LIMIT_REFERENCE_FRAMES,
      (SchroQueueFreeFunc)schro_upsampled_frame_free);
#endif
      
  decoder->frame_queue = schro_queue_new (SCHRO_LIMIT_REFERENCE_FRAMES,
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
  if(n_threads>SCHRO_MAX_DECODERS)
    n_threads = SCHRO_MAX_DECODERS;
  
  decoder->worker_count = n_threads;
  
  for(x=0; x<decoder->worker_count; ++x)
  {
    SCHRO_LOG("creating decoder %i", x);
    decoder->workers[x] = schro_decoderworker_new();
    decoder->workers[x]->parent = decoder;
    pthread_create (&decoder->worker_threads[x], NULL, schro_decoder_main, decoder->workers[x]);
  }

  return decoder;
}

void schro_decoder_free(SchroDecoder *decoder)
{
  int x;
  
  /** Send all threads the command to quit */
  for(x=0; x<decoder->worker_count; ++x)
  {
    void *ignore;
    
    SCHRO_LOG("stopping decoder %p", decoder->workers[x]);
    
    pthread_mutex_lock (&decoder->mutex);
    while(decoder->workers[x]->state != SCHRO_DECODER_FREE)
        pthread_cond_wait (&decoder->worker_statechange, &decoder->mutex);
    decoder->workers[x]->state = SCHRO_DECODER_QUIT;    
    pthread_mutex_unlock (&decoder->mutex);

    pthread_cond_broadcast (&decoder->worker_statechange);
    
    pthread_join (decoder->worker_threads[x], &ignore);

    SCHRO_LOG("freeing decoder %i", x);
    schro_decoderworker_free(decoder->workers[x]);
  }

  schro_queue_free (decoder->output_queue);
  schro_queue_free (decoder->reference_queue);
  schro_queue_free (decoder->frame_queue);
  
  /** Destroy mutexes and conditions */
  pthread_mutex_destroy(&decoder->mutex);
  pthread_cond_destroy(&decoder->reference_notfull);
  pthread_cond_destroy(&decoder->reference_newframe);
  pthread_cond_destroy(&decoder->worker_statechange);

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
    if(decoder->workers[x]->state == SCHRO_DECODER_FREE)
      return decoder->workers[x];
  }
  return NULL;
}

static int schro_decoder_mintime(SchroDecoder *decoder)
{
  int x;
  int mintime = INT_MAX;

  for(x=0; x<decoder->worker_count; ++x)
  {
    if(decoder->workers[x]->state == SCHRO_DECODER_BUSY)
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

  /** Get free worker thread */
  SCHRO_DEBUG("Looking for free worker thread");
  pthread_mutex_lock (&decoder->mutex);
  while((w = get_free_worker(decoder)) == NULL) {
    pthread_cond_wait (&decoder->worker_statechange, &decoder->mutex);
  }
  w->state = SCHRO_DECODER_PREPARING;
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

  /* Fire off thread */
  schro_decoder_set_worker_state(decoder, w, SCHRO_DECODER_BUSY);
  
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
  /** Broadcast signal that a new frame is waiting */
  pthread_cond_broadcast(&decoder->reference_newframe);
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
  
  pthread_mutex_lock (&decoder->mutex);
  
  SCHRO_DEBUG("getting %d", picture_number);
  /** Block if frame is not found */
  while((ret = schro_queue_find (decoder->reference_queue, picture_number)) == NULL) {
    //SCHRO_ERROR("%d Not found -- waiting", picture_number);
    pthread_cond_wait (&decoder->reference_newframe, &decoder->mutex);
  }
  
  pthread_mutex_unlock (&decoder->mutex);
  
  return ret;
}

#if 0
void
schro_decoder_reference_retire (SchroDecoder *decoder,
    SchroPictureNumber picture_number)
{
  SCHRO_DEBUG("retiring %d", picture_number);
  pthread_mutex_lock (&decoder->mutex);
  schro_queue_delete (decoder->reference_queue, picture_number);
  pthread_mutex_unlock (&decoder->mutex);
  /** Signal that queue is no longer full */
  pthread_cond_signal(&decoder->reference_notfull);
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

static void schro_decoder_init (SchroDecoder *decoder)
{
  /* We could send a message to subthreads to initialize here,
     as the video format is known now.
  */
  int x;
  for(x=0; x<decoder->worker_count; ++x)
  {
      SCHRO_LOG("initializing decoder %i", x);
      decoder->workers[x]->settings = decoder->settings;
      schro_decoderworker_init(decoder->workers[x]);
  }
}

void schro_decoder_set_worker_state(SchroDecoder *decoder, SchroDecoderWorker *worker, SchroDecoderState state)
{
  pthread_mutex_lock (&decoder->mutex);
  worker->state = state;
  pthread_mutex_unlock (&decoder->mutex);
  pthread_cond_broadcast (&decoder->worker_statechange);
}
