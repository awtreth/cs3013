#include <stdio.h>
#include <stdint.h>

#include "memory.h"

void memoryMaxer() {
	
	int i;
	
	vAddr indexes[1000];
	
	for (i = 0; i < 1000; ++i) {
		indexes[i] = create_page();
		uint32_t *value = get_value(indexes[i]);
		*value = (i * 3);
		store_value(indexes[i], value);
	}
	
	for (i = 0; i < 1000; ++i) {
		free_page(indexes[i]);
	}
}

int main() {
	
	int i;
	
	for (i = 0; i < 15; i++)
		create_page();
	
	return 0;
}
