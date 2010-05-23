CPPFLAGS=-I. -Wall -Wpacked -Wpadded -g

%: %.c
	$(CC) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

ALL = parse test_zig_zag

all: $(ALL)

ljpeg.o: ljpeg.h
test_zig_zag parse: LDFLAGS=-L $(HUFFMAN_LIBRARY_PATH) -lhuffman -Wl,-rpath,$(HUFFMAN_LIBRARY_PATH)
test_zig_zag parse: ljpeg.o
parse: parse.o

parse.o test_zig_zag.o: ljpeg.h


clean:
	rm -f $(ALL) *.o core
