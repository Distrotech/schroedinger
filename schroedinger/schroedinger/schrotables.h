
#ifndef __SCHRO_SCHRO_TABLES_H__
#define __SCHRO_SCHRO_TABLES_H__

#include <schroedinger/schro-stdint.h>
#include <schroedinger/schroutils.h>

extern int16_t schro_table_offset[61] SCHRO_INTERNAL;
extern int16_t schro_table_quant[61] SCHRO_INTERNAL;
extern uint32_t schro_table_inverse_quant[61] SCHRO_INTERNAL;
extern uint32_t schro_table_division_factor[256] SCHRO_INTERNAL;

#endif

