CPPFLAGS += -I.. -I.

BINS = fbits bits-test
OBJS = bits.o xio.o

ALL = $(BINS) $(OBJS)

all: $(ALL)

test-bits: bits.c
	$(CC) $(CPPFLAGS) -D__TEST__ $< -o $@

bits.o: bits.h

fbits: fbits.o bits.o
fbits.o: bits.h

bits-test: bits.o

clean:
	rm -vfr $(OBJS) $(BINS)
