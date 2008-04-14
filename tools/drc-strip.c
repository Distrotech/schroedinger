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


typedef struct SchroWriter {
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

static void write_uint32_lit(char *b, uint32_t u)
{
  b[0] = (u>>24) & 0xff;
  b[1] = (u>>16) & 0xff;
  b[2] = (u>>8)  & 0xff;
  b[3] = (u)     & 0xff;
}

typedef struct SchroRawPicture {
  
  
} SchroRawPicture;

static uint32_t read_uint32_lit(char *b)
{
  return (b[0]<<24) | ((b[1]<<16)&0xff0000) |
    ((b[2]<<8)&0xff00) | (b[3]&0xff);
}


static void schro_read_packet(SchroStream *r, SchroPacket *p)
{
  char h[13];
  SCHRO_ASSERT(read(r->f, h, 13) == 13);
  SCHRO_ASSERT(memcmp(h, "BBCD", 4) == 0);
  p->c = h[4];
  SCHRO_ASSERT(read_uint32_lit(h+9) ==  r->l);
  r->l = read_uint32_lit(h+5);
  r->l = (r->l ? r->l : 13);
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
  write_uint32_lit(b+5,n);
  write_uint32_lit(b+9,n);
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


unsigned schro_stream_eos(SchroStream *r)
{
  static int i = 104;
  return (i-- < 0);
}

/* we will not use this data, but we need to skip it. */

static void skip_global_parameters(SchroParseCode c, SchroUnpack *u)
{
  unsigned i;
  for(i = 0; i < SCHRO_PARSE_CODE_NUM_REFS(c); i++) {
    /* pan tilt */
    if(schro_unpack_decode_bit(u)) {
      schro_unpack_decode_sint(u);
      schro_unpack_decode_sint(u);
    }
    /* zoom rotation shear */
    if(schro_unpack_decode_bit(u)) {
      schro_unpack_decode_uint(u);
      schro_unpack_decode_sint(u);
      schro_unpack_decode_sint(u);
      schro_unpack_decode_sint(u);
      schro_unpack_decode_sint(u);
    }
    /* perspective */
    if(schro_unpack_decode_bit(u)) {
      schro_unpack_decode_uint(u);
      schro_unpack_decode_sint(u);
      schro_unpack_decode_sint(u);
    }
  }
}

struct SchroRawPicture {
  uint32_t n;
  uint32_t r[2];
  uint32_t lxl,lyl,lxs,lys; 
  uint32_t mvp; /* motion vector precision */
  uint8_t g;
  uint32_t wi,wd,cbm; /* wavelet index, dwt depth, codeblock mode */
};

static void preset_block_params(uint32_t i, struct SchroRawPicture *p)
{
  p->lxl = p->lyl = (i+1)*8;
  p->lxs = p->lys = i*8;
}

void strip_picture(SchroPacket *p)
{
  SchroUnpack u;
  struct SchroRawPicture r; 
  schro_unpack_init_with_data(&u,p->b.p, p->b.s, 1);
  r.n = schro_unpack_decode_bits(&u,32);
  printf("Picture number: %d\n", r.n);
  if(SCHRO_PARSE_CODE_IS_INTER(p->c)) {
    r.r[0] = r.n +  schro_unpack_decode_sint(&u);
    printf("Reference picture 1: %d\n", r.r[0]);
    if(SCHRO_PARSE_CODE_NUM_REFS(p->c) == 2) {
      r.r[1] = r.n + schro_unpack_decode_sint(&u);  
      printf("Reference picture 1: %d\n", r.r[1]);
    }
    
  }
  if(SCHRO_PARSE_CODE_IS_REFERENCE(p->c)) {
    /* retire */
    puts("Intra frame");
    r.r[0] = r.n + schro_unpack_decode_sint(&u); 
    if(r.r[0]) {
      printf("Retire: %d\n", r.r[0]);
    } else {
      puts("Retire: none");
    }
  }
  if(SCHRO_PARSE_CODE_NUM_REFS(p->c) > 0) {
    printf("Inter frame\n");
    uint32_t i;
    schro_unpack_byte_sync(&u);
    i = schro_unpack_decode_uint(&u);
    if(i == 0) { /* we need this data */
      puts("Custom block parameters");
      r.lxl = schro_unpack_decode_uint(&u);  
      r.lyl = schro_unpack_decode_uint(&u);  
      r.lxs = schro_unpack_decode_uint(&u);  
      r.lys = schro_unpack_decode_uint(&u);  

    } else {
      preset_block_params(i,&r);
    }
    r.mvp = schro_unpack_decode_uint(&u);
    printf("motion vector precision %d\n", r.mvp);
    r.g = schro_unpack_decode_bit(&u);
    if(r.g) {
      puts("Global motion");
      skip_global_parameters(p->c, &u);
    }
    printf("picture prediction mode %d\n",schro_unpack_decode_uint(&u));
    /* reference picture weight data */
    if(schro_unpack_decode_bit(&u)) {
      schro_unpack_decode_uint(&u);
      schro_unpack_decode_sint(&u);
      if(SCHRO_PARSE_CODE_NUM_REFS(p->c) == 2)
	schro_unpack_decode_sint(&u);
    }
  }
  schro_unpack_byte_sync(&u);
  /* wavelet transform parameters */
  if(!SCHRO_PARSE_CODE_IS_INTER(p->c) ||
     schro_unpack_decode_bit(&u)) {
    r.wi = schro_unpack_decode_uint(&u);
    printf("Wavelet index: %d\n", r.wi);
    r.wd = schro_unpack_decode_uint(&u);
    printf("Wavelet depth: %d\n", r.wd);
    /* codeblock parameters */
    if(!SCHRO_PARSE_CODE_IS_LOW_DELAY(p->c)) {
      puts("Codeblock parameters");
      if(schro_unpack_decode_bit(&u)) {
	uint32_t i;
	for(i = 0; i < r.wd; i++) {
	  schro_unpack_decode_uint(&u);
	  schro_unpack_decode_uint(&u);
	}
	schro_unpack_decode_uint(&u);
      }
    } else { /* lowdelay syntax */
      /* slice parameters */
      schro_unpack_decode_uint(&u);
      schro_unpack_decode_uint(&u);
      schro_unpack_decode_uint(&u);
      schro_unpack_decode_uint(&u);
      /* quantisation matrix */
      if(schro_unpack_decode_bit(&u)) {
	uint32_t i;
	schro_unpack_decode_uint(&u);
	for(i = 1; i < r.wd; i++) {
	  schro_unpack_decode_uint(&u);
	  schro_unpack_decode_uint(&u);
	  schro_unpack_decode_uint(&u);
	}
      }
    }
  }
  putchar('\n');
}


int main(int argc, char **argv) {
  SchroStream r,w;
  SchroPacket p = {0, {NULL, 0}};
  SCHRO_ASSERT(argc > 1);
  r = schro_stream_open(argv[1], O_RDONLY);  
  w = schro_stream_open("output.drc", O_WRONLY|O_CREAT|O_TRUNC);
  while(!schro_stream_eos(&r)) {
    schro_read_packet(&r,&p);
    if(SCHRO_PARSE_CODE_IS_PICTURE(p.c)) {
      strip_picture(&p);
    }
    schro_write_packet(&w,&p);
  }
  if(p.c != SCHRO_PARSE_CODE_END_OF_SEQUENCE)
    schro_write_eos(&w);
  schro_stream_close(&r);
  schro_stream_close(&w);
  exit(0);
}
