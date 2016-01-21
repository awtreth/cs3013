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
#include "bgprocess.h"

#include <mcheck.h>// for memory leakage tracking


//MAIN
int main(int argc, char **argv) {
	
	//mtrace();//to check memory leakage problems
	
	//Initialize prev_rusage with 0 values
	struct rusage usage;
	int status = 0;//return from wait function
	
	char str[129];//input string
	char* cmd_args[32]; //vector of strings (arguments for the shell)
	struct timeval init, end; //checkpoint to measure wall-clock time
	
	bgprocessLL bgpLL = init_bgprocessLL();
	
	
	while(1) {
		printf(">");//prompt character
		
		if(fgets(str,129,stdin)==NULL)//EOF or error (to be able to pipe input files)
			break;

		//TODO: check string size
		
		int n_args = args_from_str(str, cmd_args);
		
		//Special cases
		if(n_args==0) continue;//no argument
		if(strcmp(cmd_args[0],"exit") == 0 && n_args == 1) break;//exit command
		if(strcmp(cmd_args[0],"cd") == 0) {//change directory built in command
			if(n_args==1)chdir(".");//default value (in this case, the current directory)
			else chdir(cmd_args[1]);
			continue;
		}
		
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
			if(strcmp(cmd_args[n_args-1],"&") != 0) {//false
				int pid = wait3(&status, 0, &usage);
				gettimeofday(&end, NULL);
				print_report(pid, cmd_args[0], diff_time(init,end), usage, status);
			}else {
				bgprocess bgp = init_bgprocess(pid, init, cmd_args[0]);
				add2bgprocessLL(&bgpLL, &bgp);
			}
			
			
			/*while(1) {
				int pid = wait3(&status, WNOHANG, &usage);
				gettimeofday(&end,NULL);
				if(pid <= 0) break; //TODO: treat -1 condition
				else {
					print_report(pid);
				}
			}*/
			
			
			free_args(cmd_args);//free memory allocaded in args_from_str function
		
		}else { //error
			perror(NULL);//error in fork
			free_args(cmd_args);//free memory allocaded in args_from_str function
			exit(EXIT_FAILURE);
		}
		
	}
	
	//free_args(cmd_args);//free memory allocaded in args_from_str function
	return 0;
}
