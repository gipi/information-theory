# this is verbatim copied into Makefile, so with space keep it as a rule
if IS_WIN32
EXTRALIBS = -lws2_32
endif

bin_PROGRAMS = parse
parse_SOURCES = parse.c ljpeg.c
parse_CPPFLAGS = -I../
parse_LDADD = ../huffman/libhuffman.a $(EXTRALIBS)

images:
	convert -size 8x8 xc:red -quality 85 -sampling-factor 2x2,1x1,1x1 8x8.jpg
