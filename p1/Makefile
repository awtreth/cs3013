CFLAGS=-Wall -g

all: runCommand shell test shell2

bgprocess.o:

auxfnc.o:

runCommand: auxfnc.o

shell: auxfnc.o

shell2: auxfnc.o bgprocess.o

test: auxfnc.o

clean:
	rm runCommand shell test mtrace.txt *.o *~

.PHONY: clean all
