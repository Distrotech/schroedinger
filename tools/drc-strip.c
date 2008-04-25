#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#define SCHRO_ENABLE_UNSTABLE_API 1
#include <schroedinger/schro.h>
#include <schroedinger/schroutils.h>
#include <schroedinger/schrodebug.h>
#include <schroedinger/schro-stdint.h>
#include <schroedinger/schrobitstream.h>
#include <schroedinger/schrounpack.h>
#include <schroedinger/schroparams.h>


struct Block {
  void *p;
  size_t s;
};

typedef struct SchroPacket {
  SchroParseCode c;
  struct Block b;
} SchroPacket;


typedef struct SchroStream {
  size_t l; /* last packet size */
  struct Block b;
  int f;
} SchroStream;





struct Block block(size_t s)
{
  struct Block b;
  b.p = schro_malloc(s);
  b.s = s;
  return b;
}

static void dispatch_block(struct Block *b)
{
  if(b->p && b->s)
    schro_free(b->p);
  b->p = NULL;
  b->s = 0;
}

void schro_read_packet(SchroStream *r, SchroPacket *p)
{
  char h[13];
  SCHRO_ASSERT(read(r->f, h, 13) == 13);
  SCHRO_ASSERT(memcmp(h, "BBCD", 4) == 0);
  p->c = h[4];
  r->l = (h[5] << 24 | h[6] << 16 | h[7] << 8 | h[8]);
  if(r->l - 13 > p->b.s) { /* doesn't fit in packet block */
    dispatch_block(&p->b);
    p->b = block(r->l - 13);
  } else {
    p->b.s = r->l - 13;
  }
  SCHRO_ASSERT(read(r->f, p->b.p, p->b.s) == p->b.s);
}

static void 
raw_packet_header(char b[13], SchroParseCode c, uint32_t n, uint32_t p)
{
  memcpy(b, "BBCD", 4);
  b[4] = c;
  memcpy(b + 5, &n, 4);
  memcpy(b + 9, &p, 4);
}

static void schro_write_packet(SchroStream *w, SchroPacket *p)
{
  if(w->b.s < p->b.s + 13) {
    free(w->b.p);
    w->b.p = malloc(p->b.s + 13);
    w->b.s = p->b.s + 13;
  }
  raw_packet_header(w->b.p, p->c, p->b.s + 13, w->l);
  memcpy(w->b.p + 13, p->b.p, p->b.s);
  write(w->f, w->b.p, p->b.s + 13);
  w->l = p->b.s + 13;
}


void schro_write_eos(SchroStream *w)
{
  struct SchroPacket p = { SCHRO_PARSE_CODE_END_OF_SEQUENCE,{ NULL, 0}};
  schro_write_packet(w, &p);
}


static void schro_stream_close(SchroStream *p)
{
  dispatch_block(&p->b);
  close(p->f);
}

SchroStream schro_stream_open(char *n, unsigned m)
{
  SchroStream s;
  if(m & O_CREAT) 
    s.f = open(n,m, 00644);
  else
    s.f = open(n,m);
  SCHRO_ASSERT(s.f >= 0);
  if(m & O_WRONLY)
    s.b = block(1024);
  else {
    s.b.p = NULL;
    s.b.s = 0;
  }
  s.l = 0;
  return s;
}



int main(int argc, char **argv) {
  SchroStream r,w;
  SchroPacket p = {0, {NULL, 0}};
  SCHRO_ASSERT(argc > 1);
  r = schro_stream_open(argv[1], O_RDONLY);  
  w = schro_stream_open("output.drc", O_WRONLY|O_CREAT|O_TRUNC);
  schro_read_packet(&r,&p);
  schro_write_packet(&w,&p);
  schro_write_eos(&w);
  schro_stream_close(&r);
  schro_stream_close(&w);
  exit(0);
}
