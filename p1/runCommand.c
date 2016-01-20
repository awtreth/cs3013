/*
 * Mateus Amarante Araujo <mamarantearaujo@wpi.edu>
 */

//HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//implements wait function
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/time.h>
#include <sys/resource.h>

//custom library
#include "auxfnc.h"

//MAIN
int main(int argc, char **argv) {
	
	//Check the number of arguments to runCommand
	if (argc <= 1) {
		printf("No provided command\n");
		exit(EXIT_FAILURE);
	}
	
	argv[argc]=(char*)NULL;
	
	struct timeval init, end; //checkpoint to measure wall-clock time
	gettimeofday(&init,NULL);
	pid_t pid = fork(); //create a new process
	
	//CHILD
	if (pid == 0) { 
		
		if(execvp(argv[1], &argv[1]) == -1){ //error with de command
			perror(NULL);//print error message
			exit(EXIT_FAILURE);
		}
		
	}else if (pid > 0) { //PARENT
		int status = 0;//return from wait function
		
		wait(&status);//wait for child execution
		gettimeofday(&end,NULL);
		
		//if the child returned EXIT_FAILURE (when something goes wrong)
		if(WEXITSTATUS(status) == EXIT_FAILURE)
			exit(EXIT_FAILURE);
		
		//get child execution statistics
		struct rusage rep;
		getrusage(RUSAGE_CHILDREN,&rep);
		
		//Print report
		printf("\n***REPORT***\n");
		printf("wall-clock:\t\t%d\n",diff_time(init,end));
		print_rusage(rep);
	
	}else { //error
		perror(NULL);//error in fork
		exit(EXIT_FAILURE);
	}
	
	return 0;
}
