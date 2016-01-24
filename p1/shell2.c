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


#define TRUE  1
#define FALSE 0

//MAIN
int main(int argc, char **argv) {
	
	mtrace();//to check memory leakage problems
	
	//Initialize prev_rusage with 0 values
	struct rusage usage;
	int status = 0;//return from wait function
	int background = FALSE;
	
	char str[129];//input string
	char* cmd_args[32]; //vector of strings (arguments for the shell)
	struct timeval init, end; //checkpoint to measure wall-clock time
	
	bgprocessLL bgpLL = init_bgprocessLL();
	
	
	while(1) {
		printf(">");//prompt character
		
		if(fgets(str,129,stdin)==NULL){//EOF or error (to be able to pipe input files)
			wait4bgprocess(&bgpLL);
			break;
		}

		int n_args = args_from_str(str, cmd_args);
		
		int builtin_effect = treat_builtin_cmds(cmd_args, n_args, &bgpLL);
		
		if(builtin_effect==CONTINUE) continue;
		else if (builtin_effect==BREAK) break;
		//else: continue normally
		
		if(strcmp(cmd_args[n_args-1],"&") == 0) {
			free(cmd_args[n_args-1]);
			cmd_args[n_args-1]=NULL;
			n_args--;
			background = TRUE;
		}else
			background = FALSE;
		
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
			if(background == FALSE) {//false
				while(1) {
					int pid_done = wait3(&status, 0, &usage);
					gettimeofday(&end, NULL);
					
					if(pid_done!=pid) {
						bgprocess bgp = remove_bgprocess(&bgpLL, pid_done);

						print_bgprocess(bgp, "completed");
						print_report(diff_time(bgp.init_time,end), usage, status);
						free_bgprocess_name(&bgp);
					}else {
						print_report(diff_time(init,end), usage, status);
						break;
					}
				}
			}else {
				bgprocess bgp = init_bgprocess(pid, init, cmd_args[0],0,NULL);
				
				add2bgprocessLL(&bgpLL, bgp);
				
				free_bgprocess_name(&bgp);
				
				print_bgprocess(*bgpLL.first, "");
			}
			
			free_args(cmd_args);//free memory allocaded in args_from_str function
		
		}else { //error
			perror(NULL);//error in fork
			free_args(cmd_args);//free memory allocaded in args_from_str function
			exit(EXIT_FAILURE);
		}
		
		check_background_processes(&bgpLL, WNOHANG);
		
		//~ while(1) {
			//~ int pid_done = wait3(&status, WNOHANG, &usage);
			//~ gettimeofday(&end, NULL);
			//~ 
			//~ if(pid_done <= 0) break; //TODO: treat -1 condition
			//~ else {
				//~ bgprocess bgp = remove_bgprocess(&bgpLL, pid_done);
				//~ print_bgprocess(bgp);
				//~ print_report(diff_time(bgp.init_time,end), usage, status);
				//~ free_bgprocess_name(&bgp);
			//~ }
		//~ }
		
	}
	
	//free_args(cmd_args);//free memory allocaded in args_from_str function
	return 0;
}
