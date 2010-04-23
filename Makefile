CPPFLAGS = -I. -Wall -g

BIN = occurence

all: $(BIN)

frequency.o: frequency.h
occurence: occurence.o frequency.o

clean:
	rm -f *.o core $(BIN)
