
EXTRA_DIST = drc-to-png

<<<<<<< HEAD:tools/Makefile.am
noinst_PROGRAMS = dump_packets dump_gop drc-cut parse_header
=======
noinst_PROGRAMS = dump_packets dump_gop drc-cut drc-strip
>>>>>>> bart:tools/Makefile.am


AM_CFLAGS = $(SCHRO_CFLAGS) $(ERROR_CFLAGS) $(LIBOIL_CFLAGS) \
	    -DSCHRO_ENABLE_UNSTABLE_API
LDADD = $(SCHRO_LIBS)

parse_header_SOURCES = \
	dirac_parse.c \
	dirac_parse.h \
	parse_header.c
