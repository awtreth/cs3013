#include "my_syscalls.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


int main() {
	
	
	pid_t pid = getpid();
	unsigned short a = 1;
	printf("%d", pid);
	cs3013_syscall2 ((unsigned short*)&pid, &a);
	
	
	return 0;
}
