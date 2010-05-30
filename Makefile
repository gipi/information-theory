DIRS = data_structure utils huffman jpeg
VPATH = $(DIRS)

include default.mk

CPPFLAGS = -I. -Wall -g

SRC=$(wildcard *.c *.h)

# order is important
#LIB = libhuffman.so
CPPFLAGS += $(addprefix -I ,$(DIRS))

ALL = $(BIN) $(LIB) $(TEST) tags

all: $(ALL) $(DIRS)


.PHONY: $(DIRS)

$(DIRS):
	$(MAKE) -C $@ VPATH=$(CURDIR)

tags: $(SRC)
	ctags -R *

clean:
	rm -f core $(ALL)
	find . -iname '*.o' -delete
	for i in $(DIRS); do $(MAKE) -C $$i clean; done
