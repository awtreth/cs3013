#ifndef AUXFNC_H
#define AUXFNC_H

#include <sys/time.h>
#include <sys/resource.h>

int diff_time(struct timeval init, struct timeval end);

int timeval_to_millisec(struct timeval tval);

void print_rusage(struct rusage data);

//current - last
struct rusage diff_rusage(struct rusage current, struct rusage previous);

int args_from_str(char str[129], char* args[33]);

int free_args(char* args[33]);


#endif
