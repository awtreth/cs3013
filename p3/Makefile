CFLAGS=-Wall -g
LDLIBS+=-lpthread

all: main main2

queue.o:

order_sem.o:

kitchen.o:

main: order_sem.o kitchen.o

main2: order_sem.o kitchen.o

clean:
	rm main *.o *~ main2

.PHONY: clean all
