CPPFLAGS += -I.. -I.

BIN = test-tree
OBJS = tree.o

ALL = $(BIN) $(OBJS)

all: $(ALL)
tree.o: tree.h

test-tree: tree.c
	$(CC) $(CPPFLAGS) -D__TEST__ $< -o $@

clean:
	rm -vfr *.o core $(ALL)
