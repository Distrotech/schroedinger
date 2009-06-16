
/* autogenerated from schro.orc */

#ifndef _ORC_OUT_H_
#define _ORC_OUT_H_

void orc_add2_rshift_add_s16_22 (int16_t *d1, int16_t *s1, int16_t *s2, int n);
void orc_add2_rshift_sub_s16_22 (int16_t *d1, int16_t *s1, int16_t *s2, int n);
void orc_add2_rshift_add_s16_11 (int16_t *d1, int16_t *s1, int16_t *s2, int n);
void orc_add2_rshift_sub_s16_11 (int16_t *d1, int16_t *s1, int16_t *s2, int n);
void orc_add_const_rshift_s16_11 (int16_t *d1, int16_t *s1, int n);
void orc_add_const_rshift_s16 (int16_t *d1, int p1, int p2, int n);
void orc_add_s16 (int16_t *d1, int16_t *s1, int16_t *s2, int n);
void orc_addc_rshift_s16 (int16_t *d1, int16_t *s1, int16_t *s2, int p1, int n);
void orc_lshift1_s16 (int16_t *d1, int16_t *s1, int n);
void orc_lshift2_s16 (int16_t *d1, int16_t *s1, int n);
void orc_lshift_s16_ip (int16_t *d1, int p1, int n);
void orc_mas2_add_s16_ip (int16_t *d1, int16_t *s1, int16_t *s2, int p1, int p2, int p3, int n);
void orc_mas2_sub_s16_ip (int16_t *d1, int16_t *s1, int16_t *s2, int p1, int p2, int p3, int n);
void orc_mas4_across_add_s16_1991_ip (int16_t *d1, int16_t *s1, int16_t *s2, int16_t *s3, int16_t *s4, int p1, int p2, int n);
void orc_mas4_across_sub_s16_1991_ip (int16_t *d1, int16_t *s1, int16_t *s2, int16_t *s3, int16_t *s4, int p1, int p2, int n);
void orc_subtract_s16 (int16_t *d1, int16_t *s1, int16_t *s2, int n);
void orc_memcpy (void *d1, void *s1, int n);
void orc_add_s16_u8 (int16_t *d1, int16_t *s1, uint8_t *s2, int n);
void orc_convert_s16_u8 (uint16_t *d1, uint8_t *s1, int n);
void orc_convert_u8_s16 (uint8_t *d1, int16_t *s1, int n);
void orc_offsetconvert_u8_s16 (uint8_t *d1, int16_t *s1, int n);
void orc_offsetconvert_s16_u8 (int16_t *d1, uint8_t *s1, int n);
void orc_subtract_s16_u8 (int16_t *d1, int16_t *s1, uint8_t *s2, int n);
void orc_multiply_and_add_s16_u8 (int16_t *d1, int16_t *s1, uint8_t *s2, int n);
void orc_splat_s16_ns (int16_t *d1, int p1, int n);
void orc_splat_u8_ns (uint8_t *d1, int p1, int n);
void orc_average_u8 (uint8_t *d1, uint8_t *s1, uint8_t *s2, int n);
void orc_rrshift6_s16_ip (int16_t *d1, int n);
void orc_unpack_yuyv_y (uint8_t *d1, uint16_t *s1, int n);
void orc_unpack_yuyv_u (uint8_t *d1, uint32_t *s1, int n);
void orc_unpack_yuyv_v (uint8_t *d1, uint32_t *s1, int n);
void orc_packyuyv (uint32_t *d1, uint8_t *s1, uint8_t *s2, uint8_t *s3, int n);
void orc_interleave2_s16 (int16_t *d1, int16_t *s1, int16_t *s2, int n);
void orc_deinterleave2_s16 (int16_t *d1, int16_t *d2, int16_t *s1, int n);
void orc_haar_sub_s16 (int16_t *d1, int16_t *s1, int n);
void orc_haar_add_half_s16 (int16_t *d1, int16_t *s1, int n);
void orc_haar_add_s16 (int16_t *d1, int16_t *s1, int n);
void orc_haar_sub_half_s16 (int16_t *d1, int16_t *s1, int n);
void orc_sum_u8 (int32_t *a1, uint8_t *s1, int n);
void orc_sum_s16 (int32_t *a1, int16_t *s1, int n);
void orc_sum_square_diff_u8 (int32_t *a1, uint8_t *s1, uint8_t *s2, int n);

#endif

