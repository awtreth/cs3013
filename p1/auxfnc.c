#include "auxfnc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>
#include <sys/resource.h>

#include <sys/types.h>
#include <sys/wait.h>


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

/* Free the memory of the arguments from args_from_str function
 * (assume it will have at most 32 arguments)
 * 
 * @return number of arguments
 */
int free_args(char* args[33]) {
	int i = 0;
	
	while(args[i] != NULL) {//terminate condition
		free(args[i]);
		i++;
	}
	
	return i;
}

/* Return the difference between init and end in miliseconds
 * 
 * @param init: initial time (older)
 * @param end:  end time (more recent)
 * 
 * @return: difference (interval time) in milisseconds
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
	printf("Page Reclaims:\t\t%ld\n\n", data.ru_minflt);
}

/* Calculate the differences between 2 rusages structs
 * 
 * @current recent rusage status (for all children. From getrusage function)
 * @previous previous rusage status (for all children. From getrusage function)
 * 
 * @return rusage resulting from the difference between current and previous (just of the main parameters. Not all)
 */
struct rusage diff_rusage(struct rusage current, struct rusage previous) {
	struct rusage result;
	
	timersub(&current.ru_utime,&previous.ru_utime,&result.ru_utime);
	timersub(&current.ru_stime,&previous.ru_stime,&result.ru_stime);
	result.ru_nivcsw = current.ru_nivcsw - previous.ru_nivcsw;
	result.ru_nvcsw = current.ru_nvcsw - previous.ru_nvcsw;
	result.ru_majflt = current.ru_majflt - previous.ru_majflt;
	result.ru_minflt = current.ru_minflt - previous.ru_minflt;
	
	return result;
}

void print_report(int time, struct rusage usage, int status) {
	//Print report
	if(WEXITSTATUS(status) == EXIT_FAILURE) return;
	printf("***REPORT***\n");
	printf("Wall-clock:\t\t%d\n",time);
	print_rusage(usage);
}
