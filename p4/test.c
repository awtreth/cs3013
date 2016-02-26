#include <stdio.h>
#include <stdint.h>

#include "memory.h"

void memoryMaxer(int n) {
	
	int i;

	vAddr indexes[n];
	
	for (i = 0; i < n; ++i) {
		indexes[i] = create_page();
		uint32_t value = i+1;
		store_value(indexes[i], &value);
	}
}

int main() {
	
	init_memory(CLOCK2_EVICT, 2);
	memoryMaxer(PAGE_TABLE_SIZE);
	
	print_page_table();
	print_mem_maps();
	print_memory_all();

	destroy_memory();
	
	return 0;
}
