#include <stdint.h>

#include "memory.h"

#define RAM_SIZE 25
#define SSD_SIZE 100
#define HDD_SIZE 1000

#define PAGE_TABLE_SIZE HDD_SIZE

uint32_t ram_m[RAM_SIZE];
uint32_t ssd_m[SSD_SIZE];
uint32_t hdd_m[HDD_SIZE];



vAddr create_page() {
	return 0;
}

uint32_t* get_value(vAddr address) {
	return 0;
}

void store_value(vAddr address, uint32_t *value) {
	
}

void free_page(vAddr address) {

}
