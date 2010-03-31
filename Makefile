CPPFLAGS=-Wall -Wpacked -Wpadded -g

ALL = parse

all: $(ALL)

parse: parse.o

clean:
	rm -f $(ALL) *.o core
