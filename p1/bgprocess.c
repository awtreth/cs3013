#include "bgprocess.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>
#include <sys/resource.h>

bgprocess init_bgprocess(pid_t pid, struct timeval init_time, char* name) {
	bgprocess bgp;
	
	bgp.pid = pid;
	bgp.init_time = init_time;
	
	if(name!=NULL) {
		bgp.name = (char*) malloc ((strlen(name)+1)*sizeof(char));
		strcpy(bgp.name, name);
	}else
		bgp.name = NULL;
	
	bgp.next = NULL;
	
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
	bg.size = 0;
	
	return bg;
}

int add2bgprocessLL(bgprocessLL* bgpLL, bgprocess* bgp) {
	
	bgp->next = bgpLL->first;
	bgpLL->first = bgp;
	
	bgpLL->size++;
	
	return 0;
}

bgprocess* remove_bgprocess(bgprocessLL* bgpLL, pid_t pid) {
	
	bgprocess* node = bgpLL->first;
	
	while(node!=NULL) {
		if (node->pid == pid) {
			bgpLL->size--;
			break;
		}
		node = node->next;
	}
	
	return node;
}

void print_bgprocessLL(bgprocessLL bgpLL) {
	
	bgprocess* node = bgpLL.first;
	
	while(node!=NULL) {
		printf("[%d] %s\n", node->pid, node->name);
		node = node->next;
	}
}

