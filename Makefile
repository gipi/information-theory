CPPFLAGS=-I. -Wall -Wpacked -Wpadded -g

%: %.c
	$(CC) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

ALL = parse test_zig_zag

all: $(ALL)

parse: parse.o
test_zig_zag.o: parse.c
	$(CC) $(CPPFLAGS) -D__TEST_ZIG_ZAG $< -c -o $@

parse.o test_zig_zag.o: jpeg_header.h

clean:
	rm -f $(ALL) *.o core
