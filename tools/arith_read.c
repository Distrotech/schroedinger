#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <schroedinger/schrobuffer.h>
#include <schroedinger/schroarith.h>

SchroBuffer* read_buffer(char *);

int main(int argc, char **argv) {
  int i;
  SchroBuffer *buf = read_buffer("/home/bart/src/out.arith");
  SchroArith *ar = schro_arith_new();
  schro_arith_decode_init(ar, buf);
  for(i = 0; i < 100; i++) {
    printf("%d\n", schro_arith_decode_sint(ar, 0, 0, 0));
  }
  schro_buffer_unref(buf);
  return 0;
}


SchroBuffer *read_buffer(char *f) {
  int fd = open(f, O_RDONLY);
  struct stat stat;
  SchroBuffer *buf;
  fstat(fd, &stat);
  buf = schro_buffer_new_and_alloc(stat.st_size + 1);
  read(fd, buf->data, stat.st_size);
  close(fd);
  return buf;
}
