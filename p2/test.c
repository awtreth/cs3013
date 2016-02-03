#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "my_syscalls.h"

int main(int argc, char** argv) {
	
	//~ pid_t pid = fork();
	//~ 
	//~ 
	//~ if(pid == 0) {
		//~ pid_t child = fork();
		//~ 
		//~ if(child == 0) {
			//~ sleep(10);
			//~ return 0;
		//~ }
		//~ 
		//~ wait(NULL);
		//~ return 0;
	//~ }
	//~ 
	//~ printf("parent %d; child %d\n", getpid(), pid);
	//~ 
	//~ sleep(1);
	//~ 
	unsigned short a = 1066, b;
	
	cs3013_syscall2(&a,&b);
	
	//~ wait(NULL);
	
	return 0;
}
