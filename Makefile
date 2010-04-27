CPPFLAGS = -I. -Wall -g

BIN = occurence huffman bits

all: $(BIN)

frequency.o: frequency.h
huffman.o: huffman.h frequency.h
huffman: huffman.o frequency.o

bits: bits.o utils/bits.o
utils/bits.o: utils/bits.h
bits.o: utils/bits.h

occurence.o: frequency.h
occurence: occurence.o frequency.o

clean:
	rm -f core $(BIN)
	find . -iname '*.o' -delete
