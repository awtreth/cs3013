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

//#include <mcheck.h>// for memory leakage tracking

//MAIN
int main(int argc, char **argv) {
	
	//mtrace();//to check memory leakage problems
	
	//Initialize prev_rusage with 0 values
	struct rusage prev_rusage;
	getrusage(RUSAGE_CHILDREN,&prev_rusage);
	
	
	char str[129];//input string
	char* cmd_args[32]; //vector of strings (arguments for the shell)
	
	while(1) {
		printf(">");//prompt character
		
		if(fgets(str,129,stdin)==NULL)//EOF or error (to be able to pipe input files)
			break;

		//TODO: check string size
		
		int n_args = args_from_str(str, cmd_args);
		
		//Special cases
		if(n_args==0) {free_args(cmd_args); continue;}//no argument
		if(strcmp(cmd_args[0],"exit") == 0 && n_args == 1) {free_args(cmd_args); break;}//exit command
		if(strcmp(cmd_args[0],"cd") == 0) {//change directory built in command
			if(n_args==1)chdir("/home");//default value (in this case, the current directory)
			else chdir(cmd_args[1]);
			free_args(cmd_args);
			continue;
		}
		
		struct timeval init, end; //checkpoint to measure wall-clock time
		
		gettimeofday(&init,NULL);//get time right before creating the child
		pid_t pid = fork(); //create a new process
		
		//CHILD
		if (pid == 0) { 
			
			if(execvp(cmd_args[0], cmd_args) == -1){ //error with de command
				perror(NULL);//print error message
				//free_args(cmd_args);//free memory allocaded in args_from_cmdline function
				exit(EXIT_FAILURE);
			}

		}else if (pid > 0) { //PARENT
			
			int status = 0;//return from wait function
			
			wait(&status);//wait for child execution
			gettimeofday(&end,NULL);
		
			free_args(cmd_args);//free memory allocaded in args_from_str function
			
			//if the child returned EXIT_FAILURE (when something goes wrong)
			if(WEXITSTATUS(status) == EXIT_FAILURE) {
				//the child print the error
				continue;
				//exit(EXIT_FAILURE);
			}
			//get CHILDREN execution statistics
			struct rusage current_rusage;
			getrusage(RUSAGE_CHILDREN,&current_rusage);
			
			//Print report
			printf("\n***REPORT***\n");
			printf("wall-clock:\t\t%d\n",diff_time(init,end));
			//use the difference to get the measurements of the most recent child
			print_rusage(diff_rusage(current_rusage, prev_rusage));
			
			prev_rusage = current_rusage;//update previous accumulated rusage
		
		}else { //error
			perror(NULL);//error in fork
			free_args(cmd_args);//free memory allocaded in args_from_str function
			exit(EXIT_FAILURE);
		}
		
	}
	
	//free_args(cmd_args);//free memory allocaded in args_from_str function
	return 0;
}
