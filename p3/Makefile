CFLAGS=-Wall -g
LDLIBS+=-lpthread

all: main test

queue.o:

aux.o:

recipe.o:

station.o:

main: aux.o recipe.o station.o

test: aux.o recipe.o

clean:
	rm main test *.o *~

.PHONY: clean all
