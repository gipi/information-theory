CPPFLAGS=-I. -I.. -Wall -Wpacked -Wpadded -g

HUFFMAN_LIBRARY_PATH = $(shell dirname $(CURDIR)/../huffman/.)
HUFFMAN_LD_FLAGS = -L $(HUFFMAN_LIBRARY_PATH) \
	      -lhuffman -Wl,-rpath,$(HUFFMAN_LIBRARY_PATH)
HUFFMAN_CPPFLAGS = -I$(HUFFMAN_LIBRARY_PATH)

TEST = test-category-code test_zig_zag
EXTRA = images pure_wave.jpg
ALL = parse $(TEST) $(EXTRA)

all: $(ALL)

ljpeg.o: ljpeg.h huffman/huffman.h
test_zig_zag parse: LDFLAGS += $(HUFFMAN_LD_FLAGS)
test_zig_zag parse: ljpeg.o
parse: parse.o

test-category-code: ljpeg.c
	$(CC) $(CPPFLAGS) -D__TEST__ $^ -o $@ $(LDFLAGS) $(HUFFMAN_LD_FLAGS)

parse.o test_zig_zag.o: ljpeg.h

pure_wave.jpg:
	convert -size 8x8  xc:black \
          -draw 'fill gray50  color 4,4 point' \
          -draw 'fill gray25  color 3,4 point' \
          -draw 'fill gray25  color 5,4 point' \
          generated_magnitude.png
	convert generated_magnitude.png \
          -auto-level -evaluate log 3  generated_spectrum.png
	convert -size 8x8 xc:gray50  generated_phase.png
	convert generated_magnitude.png generated_phase.png \
           -quality 100 -ift $@

images:
	convert -size 8x8 xc:red -sampling-factor 2x2,1x1,1x1 8x8-2x2-1x1-1x1.jpg
	convert -size 16x8 xc:red -sampling-factor 2x2,1x1,1x1 16x8-2x2-1x1-1x1.jpg
	convert -size 8x16 xc:red -sampling-factor 2x2,1x1,1x1 8x16-2x2-1x1-1x1.jpg
	convert -size 16x16 xc:red -sampling-factor 2x2,1x1,1x1 16x16-2x2-1x1-1x1.jpg


clean:
	rm -f $(ALL) *.o core $(EXTRA) *.jpg *.png
