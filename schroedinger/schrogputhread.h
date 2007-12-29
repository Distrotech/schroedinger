#ifndef __SCHRO_GPUTHREAD_H__
#define __SCHRO_GPUTHREAD_H__

#include <schroedinger/schrodecoder.h>

SCHRO_BEGIN_DECLS

typedef enum {
   /** Initialize a schrodecoder, first time init of GPU
       structures.
    */
   SCHRO_GCMD_INIT,
   /** Initialize output frame.
    */
   SCHRO_GCMD_READY_OUTPUT,
   /** Transfer motion compensation data from DMA buffer, and start motion
       compensation kernel as soon as both reference frames are available.
    */
   SCHRO_GCMD_MOTION,
   /** Subband transfer. Fire off copying of one subband from the DMA buffer
       to the GPU. Parameters: component, position, offset
    */
   SCHRO_GCMD_SUBBAND, 
   /** After the last subband, initiate inverse wavelet transform.
    */
   SCHRO_GCMD_IWT,
   /** Finish frame. Signify that CPU work is done, put frame into reference 
       and frame queue. Needs SCHRO_GCMD_MOTION and SCHRO_GCMD_IWT to be complete.
    */
   SCHRO_GCMD_FINISH,
} SchroGpuCommandId;

/** schrodecoder should have an event to signify completion of the GPU commands,
    GPU thread must also monitor finishing of events, to signal when the decoder object
    can be delegated to a thread again.
*/
/**    
    Keep a queue of spare schrodecoders that is shared between the worker threads. To
    decode a frame, the thread picks a free schrodecoder and goes to work using the
    input buffer it is given.
    Any decoding settings that changed until the worker frame should also be 
    propagated to the schrodecoder.
 */
 
/** Decoder main: 
    - process input buffers,
    - remember current decode settings (track state),
      schro_decoder_decode_access_unit
      access units have state changes
    - if input buffer holds frame, 
      - get a free schrodecoder structure, mark it as busy
        - if no free schrodecoder structure available, wait
          for GPU thread
      - schrodecoder holds input buffer
    - push schrodecoder into work queue
    
    Worker thread:
    - watch work queue
    - get buffer from work queue
    - decode input buffer into schrodecoder (CPU work)
    - unref input buffer
    - signal GPU thread the we finished GPU work
    - start over, watch work queue
    
    GPU thread
    - wait for commands from worker threads 
    - pass to GPU
    - check for completion events on schrodecoder structures,
      re-add them to 'free' list if this is the case

    Global:
    - list of schrodecoder structures
    - mutex that protects this list, at least the state
      of free objects
    
 */

/** Max number of parameters for GPU thread commands
 */
#define SCHRO_GPU_COMMAND_NPARAMS 10

/** FIFO command.
 */
struct _SchroGPUCommand
{
    SchroGpuCommandId cmd;
    struct _SchroDecoder *dec;
    int params[SCHRO_GPU_COMMAND_NPARAMS];
};
typedef struct _SchroGPUCommand SchroGPUCommand;
typedef struct _SchroGPUThread SchroGPUThread;

SchroGPUThread *schro_gputhread_new();
void schro_gputhread_command(SchroGPUThread *self, SchroGPUCommand *cmd);
void schro_gputhread_free(SchroGPUThread *self);

SCHRO_END_DECLS

#endif

