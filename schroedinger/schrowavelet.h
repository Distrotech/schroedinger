
#ifndef _SCHRO_WAVELET_H_
#define _SCHRO_WAVELET_H_

#include <schroedinger/schroutils.h>
#include <schroedinger/schroframe.h>

SCHRO_BEGIN_DECLS

#ifdef SCHRO_ENABLE_UNSTABLE_API

void schro_wavelet_transform_2d (SchroFrameData *fd, int type, int16_t *tmp);
void schro_wavelet_inverse_transform_2d (SchroFrameData *fd_dest,
    SchroFrameData *fd_src, int type, int16_t *tmp);

void schro_iwt_desl_9_3 (SchroFrameData *fd, int16_t *tmp);
void schro_iwt_5_3 (SchroFrameData *fd, int16_t *tmp);
void schro_iwt_13_5 (SchroFrameData *fd, int16_t *tmp);
void schro_iwt_haar0 (SchroFrameData *fd, int16_t *tmp);
void schro_iwt_haar1 (SchroFrameData *fd, int16_t *tmp);
void schro_iwt_haar2 (SchroFrameData *fd, int16_t *tmp);
void schro_iwt_fidelity (SchroFrameData *fd, int16_t *tmp);
void schro_iwt_daub_9_7 (SchroFrameData *fd, int16_t *tmp);

void schro_iiwt_desl_9_3 (SchroFrameData *dest, SchroFrameData *src,
    int16_t *tmp);
void schro_iiwt_5_3 (SchroFrameData *dest, SchroFrameData *src,
    int16_t *tmp);
void schro_iiwt_13_5 (SchroFrameData *dest, SchroFrameData *src,
    int16_t *tmp);
void schro_iiwt_haar0 (SchroFrameData *dest, SchroFrameData *src,
    int16_t *tmp);
void schro_iiwt_haar1 (SchroFrameData *dest, SchroFrameData *src,
    int16_t *tmp);
void schro_iiwt_fidelity (SchroFrameData *dest, SchroFrameData *src,
    int16_t *tmp);
void schro_iiwt_daub_9_7 (SchroFrameData *dest, SchroFrameData *src,
    int16_t *tmp);

#endif

SCHRO_END_DECLS

#endif

