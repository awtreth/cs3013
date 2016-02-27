#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

#include "memory.h"


int main(int argc, char** argv) {
	
	int i, eviction_algorithm, seed;
	
	//Parse input
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
	
	printf("STORE VALUES IN ORDER value = vAddr+1 (press enter to continue):\n");
	memoryMaxer(PAGE_TABLE_SIZE+2);//beyond the limit (the last 2 page creations are ignored)
	print_memory_state();
	getchar();
	
	printf("GET_VALUE FROM ALL ADDRESSES (press enter to continue):\n");
	for(i = 0; i < PAGE_TABLE_SIZE; i++){
		if(get_value(i, NULL) != i+1){
			printf("wrong answer for page %d\n", i);
			return -1;//not supposed to happen
		}
	}
	print_memory_state();
	getchar();
	
	printf("FREE RANDOM pages AND TRY TO GET_VALUE. AND CREATE SOME PAGES (press enter to continue):\n");
	for (i = 0; i < 20; i++){
		int addr = uniform_rand(0, PAGE_TABLE_SIZE);
		printf("free %d\n", addr);
		free_page(addr);
		int valid;
		get_value(addr, &valid);
		if(valid == 0) printf("Couldn't read from addr %d\n", addr);
	}
	for (i = 0; i < 10; i++) {
		vAddr addr = create_page();
		uint32_t v = uniform_rand(0, PAGE_TABLE_SIZE);
		store_value(addr, &v);
		printf("created page %d with value %d\n", addr, v);
	}
	
	
	print_memory_state();
	getchar();
	
	printf("ALTERNATE STORE AND GET (press enter to continue)\n");
	for (i = 0; i < 20; i++){
		int addr = uniform_rand(0, PAGE_TABLE_SIZE);
		uint32_t v = uniform_rand(0, 100);
		store_value(addr, &v);
		printf("store %d in vAddr %d\n", v, addr);
		int valid;
		addr = uniform_rand(0, PAGE_TABLE_SIZE);
		v = get_value(addr, &valid);
		if(valid) printf("got %d from addr %d\n", v, addr);
		else printf("Couldn't read from addr %d\n", addr);
	}
	
	
	print_memory_state();
	getchar();
	
	destroy_memory();
	
	return 0;
}
