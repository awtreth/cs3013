#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define RANDOM_EVICT	0
#define CLOCK_EVICT		1
#define CLOCK2_EVICT	2//second chance


typedef signed short vAddr;

//USER API

void init_memory(int eviction_method_, int seed) ;
void destroy_memory();

vAddr create_page();

uint32_t get_value(vAddr address, int* valid);

void store_value(vAddr address, uint32_t *value);

void free_page(vAddr address);

void print_memory_all();

void print_page_table();

void print_mem_maps();

#endif
