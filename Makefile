CPPFLAGS = -I. -Wall -g

BIN = occurence huffman

all: $(BIN)

frequency.o: frequency.h
huffman.o: huffman.h
huffman: huffman.o frequency.o
occurence: occurence.o frequency.o

clean:
	rm -f *.o core $(BIN)
