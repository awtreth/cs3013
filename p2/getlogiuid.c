#include "my_syscalls.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	
	if(argc == 1){printf("Not enough arguments\n");return -1;}
	
	unsigned short uid = 0;
	unsigned short pid = (unsigned short) atoi(argv[1]);
	
	printf("look for %u\n", pid);
	
	printf("%ld\n", cs3013_syscall3(&pid, &uid));
	printf("UID=%u\n",uid);
	
	return 0;
}
