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

#include <mcheck.h>

//MAIN
int main(int argc, char **argv) {
	
	mtrace();
	
	char str[129];
	char* cmd_args[32];
	
	while(1) {
		printf(">");
		fgets(str,129,stdin);
		//TODO: check string size and pop up error if it's invalid or continue if it's zero
		int n_args = args_from_str(str, cmd_args);
		
		if(strcmp(cmd_args[0],"exit") == 0 && n_args == 1) break;
		if(strcmp(cmd_args[0],"cd") == 0 && n_args == 2) {
			
		}
		
		struct timeval init, end; //checkpoint to measure wall-clock time
		gettimeofday(&init,NULL);
		pid_t pid = fork(); //create a new process
		
		//CHILD
		if (pid == 0) { 
			
			if(execvp(cmd_args[0], cmd_args) == -1){ //error with de command
				perror(NULL);//print error message
				free_args(cmd_args);//free memory allocaded in args_from_cmdline function
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
		
		free_args(cmd_args);//free memory allocaded in args_from_str function
	}
	
	free_args(cmd_args);//free memory allocaded in args_from_str function
	return 0;
}
