#ifndef BGPROCESS_H
#define BGPROCESS_H

#include <unistd.h>

#include <sys/time.h>
#include <sys/resource.h>

/* Store useful background process information
 */
typedef struct {
	pid_t pid;
	struct timeval init_time;
	char* name;
} bgprocess;

/* bgprocess Linked List
 */
typedef struct {
	bgprocess *first;
	bgprocess *last;
	int n;
} bgprocessLL;

bgprocess init_bgprocess(pid_t pid, struct timeval init_time, char* name);

void free_bgprocess(bgprocess* bgp);

bgprocessLL init_bgprocessLL();

int add2bgprocessLL(bgprocessLL* bgpLL, bgprocess bgp);

bgprocess remove_bgprocess(bgprocessLL* bgpLL, pid_t pid);

void print_bgprocessLL(bgprocessLL bgpLL);

#endif
