#ifndef AUXFNC_H
#define AUXFNC_H

#include <sys/time.h>
#include <sys/resource.h>

char** args_from_cmdline(int argc, char **argv);

void free_args(char** args);

int diff_time(struct timeval init, struct timeval end);

int timeval_to_millisec(struct timeval tval);

void print_rusage(struct rusage data);

char** args_from_str(char* str);


#endif
