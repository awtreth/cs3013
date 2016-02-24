#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

typedef signed short vAddr;

//USER API

vAddr create_page();

uint32_t get_value(vAddr address, int* valid);

void store_value(vAddr address, uint32_t *value);

void free_page(vAddr address);

void print_memory_all();

void print_page_table();

void print_bitmaps();

#endif
