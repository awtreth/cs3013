#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define RANDOM_EVICT	0
#define CLOCK_EVICT		1
#define CLOCK2_EVICT	2//second chance

#define RAM_SIZE 25
#define SSD_SIZE 100
#define HDD_SIZE 1000

#define TIME_FACTOR 1 //Divide the default times by this number (RAM: 0.01s, SSD: 0.1s, HDD:2.5s)

#define PAGE_TABLE_SIZE HDD_SIZE

typedef signed short vAddr;

//USER API

void init_memory(int eviction_method_, int seed) ;
void destroy_memory();
int uniform_rand(int low, int high);

vAddr create_page();

uint32_t get_value(vAddr address, int* valid);

void store_value(vAddr address, uint32_t *value);

void free_page(vAddr address);

void print_memory_state();

void memoryMaxer(int n);


#endif
