CPPFLAGS = -I. -Wall -g

BIN = occurence huffman

all: $(BIN)

frequency.o: frequency.h
huffman.o: huffman.h frequency.h
huffman: huffman.o frequency.o

occurence.o: frequency.h
occurence: occurence.o frequency.o

clean:
	rm -f *.o core $(BIN)
