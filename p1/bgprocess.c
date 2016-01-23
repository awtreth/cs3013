#include "bgprocess.h"
#include "auxfnc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>


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
	
	bgp.number = 0;
	
	return bgp;
}

bgprocess init_null_bgprocess() {
	struct timeval t;
	t.tv_sec = 0; t.tv_usec = 0;
	return init_bgprocess(0, t, NULL);
}


bgprocess copy_bgprocess(bgprocess other) {
	return init_bgprocess(other.pid, other.init_time, other.name);
}

bgprocess* dynamic_copy_bgprocess(bgprocess other) {
	bgprocess* b = (bgprocess*)malloc(sizeof(bgprocess));
	free_bgprocess_name(b);
	*b = copy_bgprocess(other);
	
	return b;
}

void free_bgprocess(bgprocess* bgp) {
	free_bgprocess_name(bgp);
	if(bgp != NULL) {
		free(bgp);
	}
}

void free_bgprocess_name(bgprocess* bgp) {
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

int add2bgprocessLL(bgprocessLL* bgpLL, bgprocess bgp) {
	
	
	if(bgpLL->first == NULL){
		bgpLL->first = dynamic_copy_bgprocess(bgp);
		bgpLL->first->next = NULL;
	}else {
		bgprocess* old_first = bgpLL->first;
		bgpLL->first = dynamic_copy_bgprocess(bgp);
		bgpLL->first->next = old_first;
	}
	
	bgpLL->size++;
	
	bgpLL->first->number = bgpLL->size;
	
	return 0;
}

bgprocess remove_bgprocess(bgprocessLL* bgpLL, pid_t pid) {
	
	bgprocess* node = bgpLL->first;
	
	if(node->pid == pid) {
		
		bgprocess b = copy_bgprocess(*bgpLL->first);
		free_bgprocess(bgpLL->first);
		bgpLL->first = NULL;
		
		bgpLL->size--;
		
		return b;
	}else {
		while(node->next!=NULL) {
			if (node->next->pid == pid) {
				bgprocess* old_next = node->next;
				bgprocess b = copy_bgprocess(*old_next);
				node->next = node->next->next;
				free_bgprocess(old_next);
				bgpLL->size--;
				
				return b;
			}
			node = node->next;
		}
	}
	
	return *node;
}

void print_bgprocess(bgprocess proc) {
	printf("[%d] %d %s\n", proc.number, proc.pid, proc.name);
}

void print_bgprocessLL(bgprocessLL bgpLL) {
	
	bgprocess* node = bgpLL.first;
	
	while(node!=NULL) {
		print_bgprocess(*node);
		node = node->next;
	}
}

void check_background_processes(bgprocessLL * bgpLL) {
	int status = 0;
	struct rusage usage;
	struct timeval end;
	
	while(1) {
		int pid_done = wait3(&status, WNOHANG, &usage);
		gettimeofday(&end, NULL);
		
		if(pid_done <= 0) break; //TODO: treat -1 condition
		else {
			bgprocess bgp = remove_bgprocess(bgpLL, pid_done);
			print_bgprocess(bgp);
			print_report(diff_time(bgp.init_time,end), usage, status);
			free_bgprocess_name(&bgp);
		}
	}
}

