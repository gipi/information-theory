CPPFLAGS = -I. -Wall -g

SRC=$(wildcard *.c *.h)

BIN = occurence hm
TEST = test-tree fbits test-bits
DIRS = jpeg
LIB = libhuffman.so

ALL = $(BIN) $(LIB) $(TEST) tags

all: $(ALL)

data_structure/tree.o: data_structure/tree.h

test-tree: data_structure/tree.c
test-bits: utils/bits.c
	$(CC) $(CPPFLAGS) -D__TEST__ $< -o $@

frequency.o: frequency.h

huffman/huffman.o: CPPFLAGS += -Wpacked
huffman/huffman.o: huffman/huffman.h frequency.h utils/bits.h

libhuffman.so: huffman/huffman.o utils/bits.o utils/xio.o data_structure/tree.o frequency.o
	$(LIBRARY_QUIET)$(CC) $(CPPFLAGS) -shared -Wl,-soname,$@ \
		-o $@ $^


hm.o: frequency.h huffman/huffman.h
hm: hm.o huffman/huffman.o frequency.o utils/bits.o data_structure/tree.o

utils/bits.o: utils/bits.h

fbits: fbits.o utils/bits.o
fbits.o: utils/bits.h

bits-test: utils/bits.o

occurence.o: frequency.h
occurence: occurence.o frequency.o

tags: $(SRC)
	ctags -R *

clean:
	rm -f core $(ALL)
	find . -iname '*.o' -delete
