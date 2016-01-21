CFLAGS=-Wall -g

all: runCommand shell test

auxfnc.o:

bgprocess.o:

runCommand: auxfnc.o

shell: auxfnc.o

shell2: auxfnc.o bgprocess.o

test: auxfnc.o

clean:
	rm runCommand shell test mtrace.txt *.o *~

.PHONY: clean all
