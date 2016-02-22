#include <stdint.h>

#include "memory.h"

#define RAM_SIZE 25
#define SSD_SIZE 100
#define HDD_SIZE 1000

#define PAGE_TABLE_SIZE HDD_SIZE

uint32_t ram_m[RAM_SIZE];
uint32_t ssd_m[SSD_SIZE];
uint32_t hdd_m[HDD_SIZE];

#define M_BIT 0
#define R_BIT 1

#define RAM_BIT 0
#define SSD_BIT 1
#define HDD_BIT 2

typedef struct {
	uint8_t flags;//Modified-Referenced
	uint8_t where;//R-S-H bits
	uint8_t ram_addr;
	uint8_t ssd_addr;
	//we don't need for hdd
} page_entry_t;

uint32_t ram_bitmap;//32 bits (use the first 25)
uint32_t ssd_bitmap[4];//128 bits (use the first 100)
//we don't need for hhd


//GENERIC BIT OPERATIONS
//bitmap can be a sigle integer or an array
int find_empty(uint32_t* bitmap, int from, int size){
	
	int i = from;
	
	do {
		if( !(bitmap[i/size] & (1 << i%size)) ){
			//manage page table and allocate memory
			return i;
		}
		i = (i+1)%size;
	}while(i!=from);
	
	return -1;
}

void set_bit(uint32_t* number, int pos, int bit_value, int size) {
	if(bit_value) number[pos/size] |= (1 << pos%size);
	else number[pos/size] &= ~(1 << pos%size);
}

int get_bit(uint32_t* number, int pos, int size){
	return number[pos/size] & (1 << pos%size);
}

page_entry_t page_table[PAGE_TABLE_SIZE];

vAddr create_page() {
	static int i = -1;
	i = (i+1)%PAGE_TABLE_SIZE;
	
	int end = (i+PAGE_TABLE_SIZE-1)%PAGE_TABLE_SIZE;
	
	while(i!=end){
		if(!page_table[i].where){
			//manage page table and allocate memory
			return i;
		}
		i = (i+1)%PAGE_TABLE_SIZE;
	}
	
	return -1;//memory is full
}

uint32_t* get_value(vAddr address) {
	return 0;
}

void store_value(vAddr address, uint32_t *value) {
	
}

void free_page(vAddr address) {
	page_table[address].flags = 0;
	page_table[address].where = 0;
	page_table[address].ram_addr = 0;
	page_table[address].ssd_addr = 0;
}
