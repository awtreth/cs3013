#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "memory.h"

#define N_THREADS 5
#define READS_PER_THREAD   30


void* fnc(void* arg) {
	int n = (int) arg;
	int valid = 0, i;
	
	for (i = 0; i < READS_PER_THREAD; i++){
		int op = uniform_rand(0, 4);//four possible operations
		vAddr p;
		uint32_t v;
		int valid;
		
		switch (op)
		{
			case 0:
				p = create_page();
				printf("thread %d created page %d (page == -1 means full memory)\n", n, p);
				break;
			case 1:
				p = uniform_rand(0, PAGE_TABLE_SIZE);
				free_page(p);
				printf("thread %d released page %d\n", n, p);
				break;
			case 2:
				p = uniform_rand(0, PAGE_TABLE_SIZE);
				v = get_value(p, &valid);
				if(valid)
					printf("thread %d got value %d from page %d\n", n, v, p);
				else
					printf("thread %d couldn't get value from page %d\n", n, p);
				break;
			case 3:
				p = uniform_rand(0, PAGE_TABLE_SIZE);
				v = uniform_rand(0, 100);
				store_value(p, &v);
				printf("thread %d stored value %d in page %d (if it's addressed)\n", n, v, p);
				break;
			
			default: break;
				
		}
		int t = uniform_rand(.01*1e6/TIME_FACTOR, 3*1e6/TIME_FACTOR);
		usleep(t);
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
	
	//memoryMaxer(PAGE_TABLE_SIZE);
	//print_memory_state();
	
	pthread_t thread[N_THREADS];

	for (i = 0; i < N_THREADS; i++)
		pthread_create(&thread[i], NULL, fnc, (void*)i);
	
	for (i = 0; i < N_THREADS; i++)
		pthread_join(thread[i], NULL);
	
	print_memory_state();

	destroy_memory();
	
	return 0;
}
