#include "my_syscalls.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>


int main(int argc, char **argv) {
	
	if(argc != 3){printf("Expect exactly 2 arguments: 1st-target pid number; 2nd-target uid number\n");return -1;}
	
	unsigned short uid = atoi(argv[2]);
	unsigned short pid = atoi(argv[1]);
	
	long status = cs3013_syscall3(&pid, &uid);
	
	if(status!=0) {
		print_error(status);
		return -1;
	}
	
	printf("DONE\n");
	
	return 0;
}
