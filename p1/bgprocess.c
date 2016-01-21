#include "bgprocess.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/resource.h>

bgprocess init_bgprocess(pid_t pid, struct timeval init_time, char* name) {
	bgprocess bgp;
	return bgp;
}

void free_bgprocess(bgprocess* bgp) {
	if(bgp->name != NULL) {
		free(bgp->name);
	}
}

bgprocessLL init_bgprocessLL() {
	bgprocessLL bg;
	
	bg.first = NULL;
	bg.last = NULL;
	bg.n = 0;
	
	return bg;
}

int add2bgprocessLL(bgprocessLL* bgpLL, bgprocess bgp) {
	return 0;
}

bgprocess remove_bgprocess(bgprocessLL* bgpLL, pid_t pid) {
	bgprocess bgp;
	return bgp;
}

void print_bgprocessLL(bgprocessLL bgpLL) {
}

