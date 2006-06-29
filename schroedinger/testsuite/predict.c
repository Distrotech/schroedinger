
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <schroedinger/schro.h>
#include <stdio.h>
#include <stdlib.h>


static int array[100];

static int
compare (const void *a, const void *b)
{
  return *(const int *)a - *(const int *)b;
}

int
main (int argc, char *argv[])
{
  SchroPredictionList list;
  SchroPredictionVector vec;
  int n;
  int i;

  for(n=1;n<100;n++){
    schro_prediction_list_init (&list);

    for(i=0;i<n;i++){
      array[i] = random() & 0xffff;
      vec.metric = array[i];

      schro_prediction_list_insert (&list, &vec);
    }
    qsort (array, n, sizeof(int), compare);

    for(i=0;i<list.n_vectors;i++){
#if 0
      printf("%d: %d %d %c\n", i, array[i], list.vectors[i].metric,
          (array[i] == list.vectors[i].metric) ? ' ' : '*');
#endif
      SCHRO_ASSERT(array[i] == list.vectors[i].metric);
    }
#if 0
    printf("\n");
#endif
  }

  return 0;
}

