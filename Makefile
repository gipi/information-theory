CPPFLAGS=-I. -Wall -Wpacked -Wpadded -g

ALL = parse

all: $(ALL)

parse: parse.o

parse.o: jpeg_header.h

clean:
	rm -f $(ALL) *.o core
