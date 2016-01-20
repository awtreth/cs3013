#include "auxfnc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>
#include <sys/resource.h>

//FUNCTIONS

/* Extract arguments from input string to be used in execpv function
 * 
 * Map: char*->char**
 * 
 * @str input str (assume it has less or equal 128 characters)
 * @args output array of strings, separating the arguments of the input string 
 * (assume it will have at most 32 arguments)
 * 
 * @return number of arguments
 * 
 * Example:
 * in str: "ls /home"
 * out args: {"ls","/home", NULL}
 * return: 2
 */
int args_from_str(char str[129], char* args[33]) {
	int n_args = 0;
	
	char* token = strtok(str," \n");
	
	while(token!=NULL) {
		//~ if(n_args >= 32) printf("Number of arguments exceeded\n");
		args[n_args] = (char*) malloc((strlen(token)+1)*sizeof(char));
		strcpy(args[n_args], token);
		n_args++;
		token = strtok(NULL," \n");
	}
	
	args[n_args] = NULL;

	return n_args;
}

/* Free the memory of the arguments from args_from_cmdline and args_from_str fucntions
 */
int free_args(char* args[33]) {
	int i = 0;
	
	while(args[i] != NULL) {
		free(args[i]);
		i++;
	}
	
	return i;
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
