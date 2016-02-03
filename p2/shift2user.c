#include "my_syscalls.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>


int main(int argc, char **argv) {
	
	
	unsigned short uid = atoi(argv[2]);
	unsigned short pid = atoi(argv[1]);
	
	cs3013_syscall2 (&pid, &uid);
	
	return 0;
}
