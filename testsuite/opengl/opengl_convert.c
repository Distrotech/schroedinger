
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <schroedinger/schro.h>
#include <schroedinger/opengl/schroopengl.h>
#include <schroedinger/opengl/schroopenglframe.h>
#include <string.h>
#include "../common.h"
#include "opengl_util.h"

void
opengl_test_convert (SchroFrameFormat dest_format, SchroFrameFormat src_format,
    int dest_width, int dest_height, int src_width, int src_height, int todo,
    int custom_pattern)
{
  char dest_format_name[64];
  char src_format_name[64];
  SchroFrame *cpu_dest_ref_frame;
  SchroFrame *cpu_src_ref_frame;
  SchroFrame *cpu_dest_test_frame;
  SchroFrame *opengl_dest_frame;
  SchroFrame *opengl_src_frame;
  char pattern_name[TEST_PATTERN_NAME_SIZE];
  int i, r;
  int ok;
  int frames = 0;
  int repeats = _benchmark ? 64 : 1;
  int total_length;
  double start_cpu_convert, start_opengl_convert;
  double elapsed_cpu_convert = 0, elapsed_opengl_convert = 0;

  printf ("==========================================================\n");

  if (!opengl_format_name (dest_format, dest_format_name, 64)) {
    printf ("opengl_test_convert: %ix%i -> %ix%i\n", src_width, src_height,
        dest_width, dest_height);
    printf ("  unhandled dest_format 0x%x\n", dest_format);
    printf ("==========================================================\n");

    opengl_test_failed ();
    return;
  }

  if (!opengl_format_name (src_format, src_format_name, 64)) {
    printf ("opengl_test_convert: %ix%i -> %ix%i\n", src_width, src_height,
        dest_width, dest_height);
    printf ("  unhandled src_format 0x%x\n", src_format);
    printf ("==========================================================\n");

    opengl_test_failed ();
    return;
  }

  printf ("opengl_test_convert: %ix%i -> %ix%i (%s -> %s)\n", src_width,
      src_height, dest_width, dest_height, src_format_name, dest_format_name);

  cpu_dest_ref_frame = schro_frame_new_and_alloc (_cpu_domain, dest_format,
      dest_width, dest_height);
  cpu_src_ref_frame = schro_frame_new_and_alloc (_cpu_domain, src_format,
      src_width, src_height);
  cpu_dest_test_frame = schro_frame_new_and_alloc (_cpu_domain, dest_format,
      dest_width, dest_height);
  opengl_dest_frame = schro_opengl_frame_new (_opengl, _opengl_domain,
      dest_format, dest_width, dest_height);
  opengl_src_frame = schro_opengl_frame_new (_opengl, _opengl_domain,
      src_format, src_width, src_height);

  printf ("  patterns\n");

  for (i = 0; i < todo; ++i) {
    opengl_custom_pattern_generate (cpu_src_ref_frame, custom_pattern, i,
        pattern_name);

    start_cpu_convert = schro_utils_get_time ();

    for (r = 0; r < repeats; ++r) {
      schro_frame_convert (cpu_dest_ref_frame, cpu_src_ref_frame);
    }

    elapsed_cpu_convert += schro_utils_get_time () - start_cpu_convert;

    schro_opengl_lock_context (_opengl); // FIXME: remove

    schro_opengl_frame_push (opengl_src_frame, cpu_src_ref_frame);

    start_opengl_convert = schro_utils_get_time ();

    for (r = 0; r < repeats; ++r) {
      schro_opengl_frame_convert (opengl_dest_frame, opengl_src_frame);
    }

    elapsed_opengl_convert += schro_utils_get_time () - start_opengl_convert;

    schro_opengl_frame_pull (cpu_dest_test_frame, opengl_dest_frame);

    schro_opengl_unlock_context (_opengl); // FIXME: remove

    ++frames;

    ok = frame_compare (cpu_dest_ref_frame, cpu_dest_test_frame);

    printf ("    %s: %s\n", pattern_name, ok ? "OK" : "broken");

    if (!ok) {
      if (dest_width <= 32 && dest_height <= 32 && src_width <= 32
          && src_height <= 32) {
        printf ("dest ref frame\n");
        frame_dump (cpu_dest_ref_frame, cpu_dest_ref_frame);

        printf ("src ref frame\n");
        frame_dump (cpu_src_ref_frame, cpu_src_ref_frame);

        printf ("dest test frame <-> dest ref frame\n");
        frame_dump (cpu_dest_test_frame, cpu_dest_ref_frame);
      }

      opengl_test_failed ();
    }
  }

  if (_benchmark) {
    total_length = (cpu_src_ref_frame->components[0].length
        + cpu_src_ref_frame->components[1].length
        + cpu_src_ref_frame->components[2].length) * frames;

    printf ("  results\n");
    printf ("    %i frames converted via cpu/opengl with %i repeats: %.2f "
        "mbyte each\n", frames, repeats,
        ((double)total_length * repeats) / (1024 * 1024));
    printf ("    total %f/%f sec, %.2f/%.2f mbyte/sec\n", elapsed_cpu_convert,
        elapsed_opengl_convert,
        ((double)total_length * repeats) / elapsed_cpu_convert / (1024 * 1024),
        ((double)total_length * repeats) / elapsed_opengl_convert
        / (1024 * 1024));
    printf ("    avg   %.8f/%.8f sec\n",
        elapsed_cpu_convert / repeats / frames,
        elapsed_opengl_convert / repeats / frames);
  }

  schro_frame_unref (cpu_dest_ref_frame);
  schro_frame_unref (cpu_src_ref_frame);
  schro_frame_unref (cpu_dest_test_frame);
  schro_frame_unref (opengl_dest_frame);
  schro_frame_unref (opengl_src_frame);

  printf ("==========================================================\n");
}

struct ConvertTest {
  SchroFrameFormat dest_format;
  SchroFrameFormat src_format;
  int dest_width, dest_height;
  int src_width, src_height;
  int todo;
  int custom_pattern;
};

struct ConvertTest opengl_test_convert_list[] = {
  /* S16 -> U8 */
  { SCHRO_FRAME_FORMAT_S16_444, SCHRO_FRAME_FORMAT_U8_444, 16, 16, 16, 16, 1,
      OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_S16_444, SCHRO_FRAME_FORMAT_U8_444, 1920, 1080, 1920,
      1080, 1, OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_S16_422, SCHRO_FRAME_FORMAT_U8_422, 1920, 1080, 1280,
      720, 1, OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_S16_420, SCHRO_FRAME_FORMAT_U8_420, 1280, 720, 1920,
      1080, 1, OPENGL_CUSTOM_PATTERN_RANDOM },

  { SCHRO_FRAME_FORMAT_S16_444, SCHRO_FRAME_FORMAT_U8_444, 16, 16, 16, 16, -1,
      OPENGL_CUSTOM_PATTERN_NONE },
  { SCHRO_FRAME_FORMAT_S16_422, SCHRO_FRAME_FORMAT_U8_422, 16, 16, 16, 16, -1,
      OPENGL_CUSTOM_PATTERN_NONE },
  { SCHRO_FRAME_FORMAT_S16_420, SCHRO_FRAME_FORMAT_U8_420, 16, 16, 16, 16, -1,
      OPENGL_CUSTOM_PATTERN_NONE },

  { SCHRO_FRAME_FORMAT_S16_444, SCHRO_FRAME_FORMAT_U8_444, 19, 19, 21, 21, -1,
      OPENGL_CUSTOM_PATTERN_NONE },
  { SCHRO_FRAME_FORMAT_S16_422, SCHRO_FRAME_FORMAT_U8_422, 21, 21, 19, 19, -1,
      OPENGL_CUSTOM_PATTERN_NONE },
  { SCHRO_FRAME_FORMAT_S16_420, SCHRO_FRAME_FORMAT_U8_420, 19, 21, 21, 19, -1,
     OPENGL_CUSTOM_PATTERN_NONE },

  { SCHRO_FRAME_FORMAT_S16_444, SCHRO_FRAME_FORMAT_U8_444, 32, 32, 16, 16, -1,
      OPENGL_CUSTOM_PATTERN_NONE },
  { SCHRO_FRAME_FORMAT_S16_422, SCHRO_FRAME_FORMAT_U8_422, 32, 32, 16, 16, -1,
      OPENGL_CUSTOM_PATTERN_NONE },
  { SCHRO_FRAME_FORMAT_S16_420, SCHRO_FRAME_FORMAT_U8_420, 32, 32, 16, 16, -1,
      OPENGL_CUSTOM_PATTERN_NONE },

  /* U8 -> S16 */
  { SCHRO_FRAME_FORMAT_U8_444, SCHRO_FRAME_FORMAT_S16_444, 16, 16, 16, 16, 1,
      OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_U8_444, SCHRO_FRAME_FORMAT_S16_444, 1280, 720, 1920,
      1080, 1, OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_U8_422, SCHRO_FRAME_FORMAT_S16_422, 1920, 1080, 1280,
      720, 1, OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_U8_420, SCHRO_FRAME_FORMAT_S16_420, 1920, 1080, 1920,
      1080, 1, OPENGL_CUSTOM_PATTERN_RANDOM },

  { SCHRO_FRAME_FORMAT_U8_444, SCHRO_FRAME_FORMAT_S16_444, 16, 16, 16, 16, -1,
      OPENGL_CUSTOM_PATTERN_NONE },
  { SCHRO_FRAME_FORMAT_U8_422, SCHRO_FRAME_FORMAT_S16_422, 16, 16, 16, 16, -1,
      OPENGL_CUSTOM_PATTERN_NONE },
  { SCHRO_FRAME_FORMAT_U8_420, SCHRO_FRAME_FORMAT_S16_420, 16, 16, 16, 16, -1,
      OPENGL_CUSTOM_PATTERN_NONE },

  { SCHRO_FRAME_FORMAT_U8_444, SCHRO_FRAME_FORMAT_S16_444, 19, 19, 21, 21, -1,
      OPENGL_CUSTOM_PATTERN_NONE },
  { SCHRO_FRAME_FORMAT_U8_422, SCHRO_FRAME_FORMAT_S16_422, 21, 21, 19, 19, -1,
      OPENGL_CUSTOM_PATTERN_NONE },
  { SCHRO_FRAME_FORMAT_U8_420, SCHRO_FRAME_FORMAT_S16_420, 19, 21, 21, 19, -1,
      OPENGL_CUSTOM_PATTERN_NONE },

  { SCHRO_FRAME_FORMAT_U8_444, SCHRO_FRAME_FORMAT_S16_444, 32, 32, 16, 16, -1,
      OPENGL_CUSTOM_PATTERN_NONE },
  { SCHRO_FRAME_FORMAT_U8_422, SCHRO_FRAME_FORMAT_S16_422, 32, 32, 16, 16, -1,
      OPENGL_CUSTOM_PATTERN_NONE },
  { SCHRO_FRAME_FORMAT_U8_420, SCHRO_FRAME_FORMAT_S16_420, 32, 32, 16, 16, -1,
      OPENGL_CUSTOM_PATTERN_NONE },

  /* U8 -> U8 */
  { SCHRO_FRAME_FORMAT_U8_444, SCHRO_FRAME_FORMAT_U8_444, 16, 16, 16, 16, 1,
      OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_U8_444, SCHRO_FRAME_FORMAT_U8_444, 1280, 720, 1920,
      1080, 1, OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_U8_422, SCHRO_FRAME_FORMAT_U8_422, 1920, 1080, 1280,
      720, 1, OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_U8_420, SCHRO_FRAME_FORMAT_U8_420, 1920, 1080, 1920,
      1080, 1, OPENGL_CUSTOM_PATTERN_RANDOM },

  /* S16 -> S16 */
  { SCHRO_FRAME_FORMAT_S16_444, SCHRO_FRAME_FORMAT_S16_444, 16, 16, 16, 16, 1,
      OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_S16_444, SCHRO_FRAME_FORMAT_S16_444, 1280, 720, 1920,
      1080, 1, OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_S16_422, SCHRO_FRAME_FORMAT_S16_422, 1920, 1080, 1280,
      720, 1, OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_S16_420, SCHRO_FRAME_FORMAT_S16_420, 1920, 1080, 1920,
      1080, 1, OPENGL_CUSTOM_PATTERN_RANDOM },

  /* YUYV -> U8 422 */
  { SCHRO_FRAME_FORMAT_U8_422, SCHRO_FRAME_FORMAT_YUYV, 16, 16, 32, 16, 1,
      OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_U8_422, SCHRO_FRAME_FORMAT_YUYV, 1920 / 2, 1080, 1920,
      1080, 1, OPENGL_CUSTOM_PATTERN_RANDOM },

  /* UYVY -> U8 422 */
  { SCHRO_FRAME_FORMAT_U8_422, SCHRO_FRAME_FORMAT_UYVY, 16, 16, 32, 16, 1,
      OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_U8_422, SCHRO_FRAME_FORMAT_UYVY, 1920 / 2, 1080, 1920,
      1080, 1, OPENGL_CUSTOM_PATTERN_RANDOM },

  /* AYUV -> U8 444 */
  { SCHRO_FRAME_FORMAT_U8_444, SCHRO_FRAME_FORMAT_AYUV, 16, 16, 64, 16, 1,
      OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_U8_444, SCHRO_FRAME_FORMAT_AYUV, 1920 / 4, 1080, 1920,
      1080, 1, OPENGL_CUSTOM_PATTERN_RANDOM },

  /* U8 422 -> YUYV */
  { SCHRO_FRAME_FORMAT_YUYV, SCHRO_FRAME_FORMAT_U8_422, 32, 16, 16, 16, 1,
      OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_YUYV, SCHRO_FRAME_FORMAT_U8_422, 1920, 1080, 1920 / 2,
      1080, 1, OPENGL_CUSTOM_PATTERN_RANDOM },

  /* U8 422 -> UYVY */
  { SCHRO_FRAME_FORMAT_UYVY, SCHRO_FRAME_FORMAT_U8_422, 32, 16, 16, 16, 1,
      OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_UYVY, SCHRO_FRAME_FORMAT_U8_422, 1920, 1080, 1920 / 2,
      1080, 1, OPENGL_CUSTOM_PATTERN_RANDOM },

  /* U8 444 -> AYUV */
  { SCHRO_FRAME_FORMAT_AYUV, SCHRO_FRAME_FORMAT_U8_444, 64, 16, 16, 16, 1,
      OPENGL_CUSTOM_PATTERN_RANDOM },
  { SCHRO_FRAME_FORMAT_AYUV, SCHRO_FRAME_FORMAT_U8_444, 1920, 1080, 1920 / 4,
      1080, 1, OPENGL_CUSTOM_PATTERN_RANDOM }
};

void
opengl_test_convert_run ()
{
  int i;

  /* convert */
  for (i = 0; i < ARRAY_SIZE (opengl_test_convert_list); ++i) {
    opengl_test_convert (opengl_test_convert_list[i].dest_format,
        opengl_test_convert_list[i].src_format,
        opengl_test_convert_list[i].dest_width,
        opengl_test_convert_list[i].dest_height,
        opengl_test_convert_list[i].src_width,
        opengl_test_convert_list[i].src_height,
        opengl_test_convert_list[i].todo < 1 ? _generators
        : opengl_test_convert_list[i].todo,
        opengl_test_convert_list[i].custom_pattern);
  }
}

