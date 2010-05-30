CPPFLAGS += -I.. -I.
BIN = occurence
ALL = hm libhuffman.so $(BIN)

all: $(ALL)

frequency.o: frequency.h
occurence.o: frequency.h
occurence: occurence.o frequency.o

huffman.o: CPPFLAGS += -Wpacked
huffman.o: huffman.h frequency.h utils/bits.h

libhuffman.so: huffman.o utils/bits.o utils/xio.o data_structure/tree.o frequency.o
	$(LIBRARY_QUIET)$(CC) $(CPPFLAGS) -shared -Wl,-soname,$@ \
		-o $@ $^

hm.o: frequency.h huffman.h
hm: hm.o huffman.o frequency.o utils/bits.o data_structure/tree.o

clean:
	rm -vfr *.o *.so $(ALL)
