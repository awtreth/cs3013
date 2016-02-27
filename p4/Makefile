CFLAGS=-Wall -g
LDLIBS+=-lpthread

all: test_single test_multi_integrity test_multi_misc

memory.o:

test_single: memory.o

test_multi_integrity: memory.o

test_multi_misc: memory.o

clean:
	rm test_single *.o *~ test_multi_misc test_multi_integrity

.PHONY: clean all
