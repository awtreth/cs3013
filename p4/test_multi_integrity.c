#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

#include "memory.h"

#define N_THREADS 5
#define READS_PER_THREAD   30


void* fnc(void* arg) {
	int n = (int) arg;
	int valid, i;
	
	for (i = 0; i < READS_PER_THREAD; i++){
		vAddr addr = uniform_rand(0, PAGE_TABLE_SIZE);
		printf("thread %d is trying to read vAddr %d\n", n, i);
		get_value(addr, &valid);
	}
	
	return NULL;
}

int main(int argc, char** argv) {
	
	int i, eviction_algorithm, seed;
	
	if(argc == 2){
		eviction_algorithm = atoi(argv[1]);
		seed = time(NULL);
	}else if(argc == 3) {
		eviction_algorithm = atoi(argv[1]);
		seed = atoi(argv[2]);
	}else{
		printf("1st argument: eviction algorithm (0-Random; 1-Clock; 2-Clock2ndChance)\n2nd argument: seed for random algorithm\n");
		return -1;
	}
	
	init_memory(eviction_algorithm, seed);
	
	memoryMaxer(PAGE_TABLE_SIZE);
	print_memory_state();
	
	pthread_t thread[N_THREADS];

	for (i = 0; i < N_THREADS; i++)
		pthread_create(&thread[i], NULL, fnc, (void*)i);
	
	for (i = 0; i < N_THREADS; i++)
		pthread_join(thread[i], NULL);
	
	print_memory_state();

	destroy_memory();
	
	return 0;
}
