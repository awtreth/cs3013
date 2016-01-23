#ifndef AUXFNC_H
#define AUXFNC_H

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

/* Return the difference between init and end in miliseconds
 * 
 * @param init: initial time (older)
 * @param end:  end time (more recent)
 * 
 * @return: difference (interval time) in milisseconds
 * 
 * the arguments can be gotten by gettimeofday function
 */
int diff_time(struct timeval init, struct timeval end);

/* Convert timeval interval to miliseconds
 * 
 * adequate to be used after diff_time
 */
int timeval_to_millisec(struct timeval tval);

/* Print the main info in rusage struct
 * 
 * use getrusage to get this info 
 */
void print_rusage(struct rusage data);

/* Calculate the differences between 2 rusages structs
 * 
 * @current recent rusage status (for all children. From getrusage function)
 * @previous previous rusage status (for all children. From getrusage function)
 * 
 * @return rusage resulting from the difference between current and previous (just of the main parameters. Not all)
 */
struct rusage diff_rusage(struct rusage current, struct rusage previous);


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
int args_from_str(char str[129], char* args[33]);

/* Free the memory of the arguments from args_from_str function
 * (assume it will have at most 32 arguments)
 * 
 * @return number of arguments
 */
int free_args(char* args[33]);


void print_report(int time, struct rusage usage, int status);



#endif
