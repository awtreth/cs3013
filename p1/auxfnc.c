#include "auxfnc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>
#include <sys/resource.h>

//FUNCTIONS

/* Extract arguments from command line to be used in execpv function
 * 
 * call void free_args(char** args) function when you are done with this function output
 * 
 * Example:
 * "./runCommand ls /home"
 * 
 * argc = 3; argv = {"./runCommand", "ls", "/home"}
 * 
 * returns: {"./runCommand", "ls", "/home", NULL}
 */
char** args_from_cmdline(int argc, char **argv) {
	int n_args = argc-1;//first str is always "./runCommand"
	char** args = (char**) malloc((n_args+1)*sizeof(char*));//last one for NULL (requirement of execpv function)
	int i = 0;
	
	for (i = 0; i < n_args; i++)
	{
		args[i] = (char*) malloc((strlen(argv[i+1])+1)*sizeof(char));//+1 for \0 caracter
		strcpy(args[i],argv[i+1]);
	}
	
	args[n_args] = (char*) NULL;//force last to be NULL (requirement of execpv function)

	return args;
}

char** args_from_str(char* str) {
	char** args = (char**)malloc(33*sizeof(char*));
	int n_args = 0;
	
	char* token = strtok(str," \n");
	
	while(token!=NULL) {
		//~ if(n_args >= 32) printf("Number of arguments exceeded\n");
		args[n_args] = (char*) malloc((strlen(token)+1)*sizeof(char));
		strcpy(args[n_args], token);
		n_args++;
		token = strtok(NULL," \n");
	}
	args = (char**) realloc(args, (n_args+1)*sizeof(char*));
	args[n_args] = NULL;

	return args;
}

/* Free the memory of the arguments from args_from_cmdline and args_from_str fucntions
 */
void free_args(char** args) {
	int i = 0;
	
	while(args[i] != NULL) {
		free(args[i]);
		i++;
	}
	
	free(args[i]);
	free(args);
}

/* return the difference between init and end in miliseconds
 * 
 * the arguments can be gotten by gettimeofday function
 */
int diff_time(struct timeval init, struct timeval end) {
	struct timeval res;
	
	timersub(&end,&init,&res);
	
	return timeval_to_millisec(res);
}

/* Convert timeval interval to miliseconds
 * 
 * adequate to be used after diff_time
 */
int timeval_to_millisec(struct timeval tval) {
	return tval.tv_sec*1000 + tval.tv_usec/1000;
}

/* Print the main info in rusage struct
 * 
 * use getrusage to get this info 
 */
void print_rusage(struct rusage data) {
	
	printf("User CPU time:\t\t%d\n", timeval_to_millisec(data.ru_utime));
	printf("System CPU time:\t%d\n", timeval_to_millisec(data.ru_stime));
	printf("Involuntary Switches:\t%ld\n", data.ru_nivcsw);
	printf("Voluntary Switches:\t%ld\n", data.ru_nvcsw);
	printf("Page Faults:\t\t%ld\n", data.ru_majflt);
	printf("Page Reclaims:\t\t%ld\n", data.ru_minflt);
}
