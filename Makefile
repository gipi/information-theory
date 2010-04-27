CPPFLAGS = -I. -Wall -g

BIN = occurence hm bits

all: $(BIN)

frequency.o: frequency.h
huffman/huffman.o: huffman/huffman.h frequency.h utils/bits.h

hm.o: frequency.h huffman/huffman.h
hm: hm.o huffman/huffman.o frequency.o utils/bits.o

bits: bits.o utils/bits.o
utils/bits.o: utils/bits.h
bits.o: utils/bits.h

occurence.o: frequency.h
occurence: occurence.o frequency.o

clean:
	rm -f core $(BIN)
	find . -iname '*.o' -delete
