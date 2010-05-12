
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include <schroedinger/schro.h>
#include <schroedinger/schroutils.h>


#define SIZE 256

/* 1.0/log(2.0) */
#define INV_LOG_2 1.44269504088896338700

int entropy (int i)
{
  double x;

  x = i / (double)SIZE;

  return 65536 * (-log(1-x) * INV_LOG_2);
}

int generate_table (void)
{
  int i;

  printf("\n");
  printf("#include <schroedinger/schrotables.h>\n");
  printf("\n");

  printf("const int entropy_lut[%d] = {\n", SIZE);
  for(i=0;i<SIZE;i++){
    if ((i&0x7) == 0) printf("  ");
    printf("%d", entropy(i));
    if (i < SIZE - 1) printf(", ");
    if ((i&0x7) == 0x7) printf("\n");
  }
  printf("};\n");

  return 0;
}


int
main (int argc, char *argv[])
{

  schro_init();

  generate_table();

  return 0;
}



