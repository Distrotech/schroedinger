
#include <stdio.h>
#include <math.h>

int
get_quant (int i)
{
  unsigned long long base;
  base = 1<<(i/4);
  switch(i&3) {
    case 0:
      return 4*base;
    case 1:
      return (78892 * base + 8292) / 16585;
    case 2:
      return (228486 * base + 20195) / 40391;
    case 3:
      return (440253 * base + 32722) / 65444;
  }
}

int
get_offset (int i)
{
  return (get_quant(i) * 3 + 4)/8;
}

unsigned int
get_inv_quant (int i)
{
  int q = get_quant(i);
  return (1ULL<<32)/q;
}

int
get_factor (int i)
{
  if (i<2) return 0;
  return (0x10000 + i/2)/i;
}

int
main (int argc, char *argv[])
{
  int i;

  printf("\n");
  printf("#include <schroedinger/schrotables.h>\n");
  printf("\n");

  /* schro_table_offset */
  printf("uint32_t schro_table_offset[61] = {\n");
  for(i=0;i<60;i+=4) {
    printf("  %7d, %7d, %7d, %7d,\n",
        get_offset(i),
        get_offset(i+1),
        get_offset(i+2),
        get_offset(i+3));
  }
  printf("  %7d\n", get_offset(i));
  printf("};\n");
  printf("\n");

  /* schro_table_quant */
  printf("uint32_t schro_table_quant[61] = {\n");
  for(i=0;i<60;i+=4) {
    printf("  %7d, %7d, %7d, %7d,\n",
        get_quant(i),
        get_quant(i+1),
        get_quant(i+2),
        get_quant(i+3));
  }
  printf("  %7d\n", get_quant(i));
  printf("};\n");
  printf("\n");

  /* schro_table_quant */
  printf("uint32_t schro_table_inverse_quant[61] = {\n");
  for(i=0;i<60;i+=4) {
    printf("  %10uu, %10uu, %10uu, %10uu,\n",
        get_inv_quant(i),
        get_inv_quant(i+1),
        get_inv_quant(i+2),
        get_inv_quant(i+3));
  }
  printf("  %10uu\n", get_inv_quant(i));
  printf("};\n");
  printf("\n");

  /* schro_table_quant */
  printf("uint16_t schro_table_division_factor[257] = {\n");
  for(i=0;i<256;i+=4) {
    printf("  %5u, %5u, %5u, %5u,\n",
        get_factor(i),
        get_factor(i+1),
        get_factor(i+2),
        get_factor(i+3));
  }
  printf("  %5u\n", get_factor(i));
  printf("};\n");

  return 0;
}
