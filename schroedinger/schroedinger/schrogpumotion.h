#ifndef __SCHRO_GPUMOTION_H__
#define __SCHRO_GPUMOTION_H__

#include <schroedinger/schromotion.h>
#include <schroedinger/schrogpuframe.h>

SCHRO_BEGIN_DECLS

typedef struct _SchroGPUMotion SchroGPUMotion;

SchroGPUMotion *schro_gpumotion_new();
void schro_gpumotion_free(SchroGPUMotion *rv);

void schro_gpumotion_render (SchroGPUMotion *self, SchroMotion *motion, SchroGPUFrame *gdest);

SCHRO_END_DECLS

#endif
