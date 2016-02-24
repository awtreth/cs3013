#include <stdio.h>
#include <stdint.h>

#include "memory.h"

void memoryMaxer(int n) {
	
	int i;
	int valid;
	
	vAddr indexes[n];
	
	for (i = 0; i < n; ++i) {
		indexes[i] = create_page();
		uint32_t value = get_value(indexes[i], &valid);
		printf("valid=%d value=%d\n", valid, value);
		value = (i * 3);
		store_value(indexes[i], &value);
		print_page_table();
		print_mem_maps();
		print_memory_all();
	}
	
	for (i = 0; i < n; ++i) {
		free_page(indexes[i]);
	}
	
	print_page_table();
	print_mem_maps();
	print_memory_all();
}

int main() {
	
	memoryMaxer(20);
	
	
	
	return 0;
}
