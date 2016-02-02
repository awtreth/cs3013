obj-m := phase0.o phase1.o phase2.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: test0 my_syscalls.o
	$(MAKE) -C $(KDIR) M=$(PWD) modules

test0:

my_syscalls.o:

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm test0 *~
