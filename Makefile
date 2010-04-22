CPPFLAGS = -Wall -g

BIN = occurence

all: $(BIN)

clean:
	rm -f *.o core $(BIN)
