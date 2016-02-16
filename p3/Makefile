CFLAGS=-Wall -g
LDLIBS+=-lpthread

all: main test main2

queue.o:

aux.o:

order_sem.o:

kitchen.o:

main: aux.o order_sem.o kitchen.o

main2: aux.o order_sem.o kitchen.o

test: aux.o order_sem.o kitchen.o

clean:
	rm main test *.o *~ main2

.PHONY: clean all
