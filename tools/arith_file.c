#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <schroedinger/schrobuffer.h>
#include <schroedinger/schroarith.h>

void write_buffer(SchroBuffer *b);

int main(int argc, char **argv) {
  int i;
  SchroArith *a = schro_arith_new();
  SchroBuffer *b = schro_buffer_new_and_alloc(100);
  schro_arith_encode_init(a, b);
  for(i = -50; i < 50; i++) {
    schro_arith_encode_sint(a, 0, 0, 0, i);
  }
  write_buffer(b);
  //schro_buffer_unref(b);
  schro_arith_free(a);
  return 0;
}

void write_buffer(SchroBuffer *b) {
  int fd;
  if(access("../java/out.arith", W_OK) == 0) {
    fd = open("../java/out.arith", O_TRUNC|O_WRONLY);
  } else {
    if(access("../java/out.arith", F_OK) == 0) 
      unlink("../java/out.arith");
    fd  = creat("../java/out.arith", S_IRUSR|S_IWUSR);
  }
  write(fd, b->data, b->length);
  close(fd);
}
