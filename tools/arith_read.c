#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <schroedinger/schrobuffer.h>
#include <schroedinger/schroarith.h>


/* Tool to arithmetic-decode a file.
 * There's a bug with the end of the file,
 * so don't use it to replace bzip2 */

SchroBuffer* read_buffer(char *); 

int main(int argc, char **argv) {
  int i;
  if(argc < 2) {
    fputs("Usage: arith_read <infile>", stderr);
  }
  SchroBuffer *buf = read_buffer(argv[1]);
  SchroArith *ar = schro_arith_new();
  schro_arith_decode_init(ar, buf);
  while(ar->buffer->length - ar->offset > 0) {
    char c = 0;
    for(i = 0; i < 8; i++) {
      c = (c << 1) | schro_arith_decode_bit(ar,i); 
    }
    putchar(c);
  }
  putchar('\n');
  schro_buffer_unref(buf);
  schro_arith_free(ar);
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
