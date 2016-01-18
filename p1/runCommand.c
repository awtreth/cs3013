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

//PROTOTYPES


char** extract_args(int argc, char **argv);

void free_args(char** args);

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
		
		execvp(cmd_args[0], cmd_args);
		
		free_args(cmd_args);

	}else if (pid > 0) { //parent
		wait(NULL);//wait for child execution
	
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
