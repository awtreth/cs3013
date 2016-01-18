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


//PROTOTYPES


char** extract_args(int argc, char **argv);

void free_args(char** args);

int diff_time(struct timeval init, struct timeval end);

int timeval_to_millisec(struct timeval tval);

void print_rusage(struct rusage data);


//MAIN
int main(int argc, char **argv) {
	
	//Check the number of arguments to runCommand
	if (argc <= 1) {
		perror("No provided command");
		exit(EXIT_FAILURE);
	}
	
	pid_t pid = fork(); //create a new process
	
	if (pid == 0) { //child
		char** cmd_args = extract_args(argc, argv);
		
		if(execvp(cmd_args[0], cmd_args) == -1){
			perror(NULL);
			free_args(cmd_args);
			exit(EXIT_FAILURE);
		}
		free_args(cmd_args);

	}else if (pid > 0) { //parent
		struct timeval init, end;
		int status = 0;
		
		
		gettimeofday(&init,NULL);
		wait(&status);//wait for child execution
		gettimeofday(&end,NULL);
		
		if(WEXITSTATUS(status) == EXIT_FAILURE)
			exit(EXIT_FAILURE);
		
		printf("\n***REPORT***\n");
	
		struct rusage rep;
		getrusage(RUSAGE_CHILDREN,&rep);
		
		printf("wall-clock:\t\t%d\n",diff_time(init,end));
		print_rusage(rep);
	
	}else { //error
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	
	return 0;
}

//FUNCTIONS

char** extract_args(int argc, char **argv) {
	int n_args = argc-1;
	char** args = (char**) malloc(n_args+1);
	int i = 0;
	
	for (i = 0; i < n_args; i++)
	{
		args[i] = (char*) malloc(strlen(argv[i+1])+1);
		strcpy(args[i],argv[i+1]);
	}
	
	args[n_args] = (char*) NULL;

	return args;
}

void free_args(char** args) {
	int i = 0;
	
	while(args[i] != NULL) {
		free(args[i]);
		i++;
	}
	
	free(args);
}

int diff_time(struct timeval init, struct timeval end) {
	struct timeval res;
	
	timersub(&end,&init,&res);
	
	return timeval_to_millisec(res);
}

int timeval_to_millisec(struct timeval tval) {
	return tval.tv_sec*1000 + tval.tv_usec/1000;
}

void print_rusage(struct rusage data) {
	
	printf("User CPU time:\t\t%d\n", timeval_to_millisec(data.ru_utime));
	printf("System CPU time:\t%d\n", timeval_to_millisec(data.ru_stime));
	printf("Involuntary Switches:\t%ld\n", data.ru_nivcsw);
	printf("Voluntary Switches:\t%ld\n", data.ru_nvcsw);
	printf("Page Faults:\t\t%ld\n", data.ru_majflt);
	printf("Page Reclaims:\t\t%ld\n", data.ru_minflt);
}
