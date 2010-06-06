CPPFLAGS=-I. -Wall -Wpacked -Wpadded -g

HUFFMAN_LIBRARY_PATH = $(shell dirname $(CURDIR)/../huffman/.)
HUFFMAN_LD_FLAGS = -L $(HUFFMAN_LIBRARY_PATH) \
	      -lhuffman -Wl,-rpath,$(HUFFMAN_LIBRARY_PATH)
HUFFMAN_CPPFLAGS = -I$(HUFFMAN_LIBRARY_PATH)

TEST = test-category-code
ALL = parse $(TEST) test_zig_zag

all: $(ALL)

ljpeg.o: ljpeg.h huffman/huffman.h
test_zig_zag parse: LDFLAGS += $(HUFFMAN_LD_FLAGS)
test_zig_zag parse: ljpeg.o
parse: parse.o

test-category-code: ljpeg.c
	$(CC) $(CPPFLAGS) -D__TEST__ $^ -o $@ $(LDFLAGS) $(HUFFMAN_LD_FLAGS)

parse.o test_zig_zag.o: ljpeg.h


clean:
	rm -f $(ALL) *.o core $(EXTRA)
