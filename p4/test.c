#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include "memory.h"

void* fnc(void* arg) {
	int n = (int) arg;
	int valid, i;
	
	for (i = 0; i < 10; i++){
		vAddr addr = uniform_rand(0, PAGE_TABLE_SIZE);
		get_value(addr, &valid);
		printf("thread %d read from addr %d. valid = %d\n", n, addr, valid);
	}
	
	return NULL;
}



void memoryMaxer(int n) {
	
	int i;

	vAddr indexes[n];
	
	for (i = 0; i < n; ++i) {
		indexes[i] = create_page();
		uint32_t value = i+1;
		store_value(indexes[i], &value);
	}
}


#define N_THREADS 5

int main() {
	
	int i;
	
	init_memory(RANDOM_EVICT, 2);
	memoryMaxer(PAGE_TABLE_SIZE);
	
	print_page_table();
	print_mem_maps();
	print_memory_all();

	pthread_t thread[N_THREADS];

	for (i = 0; i < N_THREADS; i++)
		pthread_create(&thread[i], NULL, fnc, (void*)i);
	
	for (i = 0; i < N_THREADS; i++)
		pthread_join(thread[i], NULL);
	
	print_page_table();
	print_mem_maps();
	print_memory_all();

	destroy_memory();
	
	return 0;
}
