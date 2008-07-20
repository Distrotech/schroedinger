#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <schroedinger/schrobuffer.h>
#include <schroedinger/schroarith.h>
#include <schroedinger/schrounpack.h>

/* Tool to arittmetic-encode a file. */
/* Output file extension is sundea:
 * schroedinger unique new dirac arithmetic encoder 
 * or dynamic for dirac */

void arith_encode(int, int);
SchroBuffer* read_buffer(int); 

int main(int argc, char **argv) {
  int in, out;
  char *ofn;
  if(argc < 2) {
    fputs("Usage: arith_file <infile> [<outfile>]\n", stderr);
    exit(1);
  }
  in = open(argv[1], O_RDONLY);
  if(in < 0) {
    fputs("Error: could not open input file", stderr);
    exit(1);
  }
  if(argc > 2) {
    ofn = argv[2];
  } else {
    ofn = malloc(strlen(argv[1]) + 8);
    strcpy(ofn, argv[1]);
    strcat(ofn, ".sundae"); 
  }
  out = open(ofn, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
  arith_encode(in, out);
  close(in);
  close(out);
  return 0;
}


SchroBuffer *read_buffer(int f) {
  SchroBuffer *buf;
  struct stat stat;
  fstat(f, &stat);
  buf = schro_buffer_new_and_alloc(stat.st_size + 1);
  read(f, buf->data, stat.st_size);
  return buf;
}

void arith_encode(int in, int out) {
  int c = 0;
  SchroUnpack u; 
  SchroBuffer *inbuf = read_buffer(in);
  SchroBuffer *outbuf = schro_buffer_new_and_alloc(inbuf->length);
  SchroArith *a = schro_arith_new();
  schro_unpack_init_with_data(&u, inbuf->data, inbuf->length, 1);
  schro_arith_encode_init(a, outbuf);
  while(schro_unpack_get_bits_remaining(&u) > 0) {
    int bit = schro_unpack_decode_bit(&u);
    schro_arith_encode_bit(a, c&7 ,bit);
    c++;
  }
  schro_arith_flush(a);
  write(out, outbuf->data, a->offset + 1);
  //  schro_buffer_unref(inbuf);
  //  schro_buffer_unref(outbuf);
  //  schro_arith_free(a);
}
