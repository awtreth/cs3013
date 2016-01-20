CFLAGS=-Wall -g

all: runCommand shell test

auxfnc.o:

runCommand: auxfnc.o

shell: auxfnc.o

test: auxfnc.o

clean:
	rm runCommand shell test mtrace.txt *.o *~

.PHONY: clean all
