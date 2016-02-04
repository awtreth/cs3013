#include "my_syscalls.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	
	if(argc != 2){printf("Expect exactly 1 argument: the target pid number\n");return -1;}
	
	unsigned short uid = 0;
	unsigned short pid = (unsigned short) atoi(argv[1]);
	
	printf("look for %u\n", pid);
	
	long status = cs3013_syscall3(&pid, &uid);
	
	if(status!=0) {
		print_error(status);
		return -1;
	}
	
	printf("actual_uid=%u\n",uid);
	
	return 0;
}
