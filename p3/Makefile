CFLAGS=-Wall -g
LDLIBS+=-lpthread

all: main test

queue.o:

aux.o:

order_sem.o:

kitchen.o:

main: aux.o order_sem.o kitchen.o

test: aux.o order_sem.o kitchen.o

clean:
	rm main test *.o *~

.PHONY: clean all
