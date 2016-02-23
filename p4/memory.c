#include <stdint.h>

#include "memory.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define RAM_SIZE 3
#define SSD_SIZE 6
#define HDD_SIZE 15

#define PAGE_TABLE_SIZE HDD_SIZE

uint32_t ram_m[RAM_SIZE];
uint32_t ssd_m[SSD_SIZE];
uint32_t hdd_m[HDD_SIZE];

#define M_BIT 0
#define R_BIT 1

#define RAM_BIT 0
#define SSD_BIT 1
#define HDD_BIT 2

//Access time (in seconds)
#define TIME_FACTOR 100
#define RAM_TIME .01/TIME_FACTOR
#define SSD_TIME .1/TIME_FACTOR
#define HDD_TIME 2.5/TIME_FACTOR

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




int ptable_cursor = 0;
int  ram_cursor = 0;
int  ssd_cursor = 0;

//GENERIC BIT OPERATIONS
//bitmap can be a sigle integer or an array
int find_empty(uint32_t* bitmap, int* cursor, int size){
	
	int i = *cursor;
	
	do {
		if( !(bitmap[i/size] & (1 << i%size)) ){
			//manage page table and allocate memory
			*cursor = (i+1)%size;
			return i;
		}
		i = (i+1)%size;
	}while(i != *cursor);
	
	return -1;
}

void set_bit(uint32_t* number, int pos, int bit_value, int size) {
	if(bit_value) number[pos/size] |= (1 << pos%size);
	else number[pos/size] &= ~(1 << pos%size);
}

int get_bit(uint32_t* number, int pos, int size){
	return (number[pos/size] & (1 << pos%size))&&1;
}

void print_bitmap(uint32_t* bitmap, int size) {
	int i;
	for (i = 0; i < size; i++)
		printf("%d", get_bit(bitmap, i, size));
}

void print_page_entry(page_entry_t entry) {
	print_bitmap((uint32_t*)&entry.flags, 2);
	printf(" | ");
	print_bitmap((uint32_t*)&entry.where, 3);
	printf(" | %d | %d\n", entry.ram_addr, entry.ssd_addr);
}

page_entry_t page_table[PAGE_TABLE_SIZE];

//Auxiliar function (return a random number between low and high)
int uniform_rand(int low, int high) {
	return rand()%(abs(high-low)+1)+low;
}

int find_evict_first(int mem_bit, int mem_size) {
	int i = ptable_cursor;//start point
	
	do {
		if( get_bit((uint32_t*) &page_table[i].where, mem_bit, 3) ){
			//ptable_cursor = (i+1)%PAGE_TABLE_SIZE;
			return i;
		}
		i = (i+1)%PAGE_TABLE_SIZE;
	}while(i!=ptable_cursor);
	
	return -1;//not supposed to reach this position
}


int evict() {
	int ram_addr, ssd_addr;
	
	//Virtual address of the process to be evicted from ram
	ram_addr = find_evict_first(RAM_BIT, RAM_SIZE); 
	
	printf("evicted %d from ram\n", ram_addr);
	
	//vAddr in ssd. Potencially can be evicted from ssd
	ssd_addr = find_empty(&ssd_bitmap[0], &ssd_cursor, SSD_SIZE);
	
	if(ssd_addr < 0) {//full
		ssd_addr = find_evict_first(SSD_BIT, SSD_SIZE); 
		printf("evicted %d from ssd\n", ssd_addr);
		//Copy from SSD to HDD
		
		//set bits in page table
		set_bit((uint32_t*) &page_table[ssd_addr].where, SSD_BIT, 0, 3);//take out from ssd
		set_bit((uint32_t*) &page_table[ssd_addr].where, HDD_BIT, 1, 3);//put into hdd
		
		//access time
		usleep(HDD_TIME*1e6 + SSD_TIME*1e6);
		
		//copy the values
		hdd_m[ssd_addr] = ssd_m[page_table[ssd_addr].ssd_addr];
	}else {
		set_bit(&ssd_bitmap[0], ssd_addr, 1, SSD_SIZE);
	}
	print_bitmap(&ssd_bitmap[0], SSD_SIZE);
	printf("\n");
	//Copy from RAM to SSD
	
	//set bits in page table
	set_bit((uint32_t*) &page_table[ram_addr].where, RAM_BIT, 0, 3);
	set_bit((uint32_t*) &page_table[ram_addr].where, SSD_BIT, 1, 3);
	
	//access time
	usleep(RAM_TIME*1e6 + SSD_TIME*1e6);
	
	//copy the values
	ssd_m[page_table[ram_addr].ssd_addr] = ram_m[page_table[ram_addr].ram_addr];
	page_table[ram_addr].ssd_addr = ssd_addr;

	return page_table[ram_addr].ram_addr;//return evicted vAddr
}

vAddr create_page() {
	int i = ptable_cursor;
	
	do{
		if(!page_table[i].where){//not used page-table
			//update page table
			ptable_cursor = (i+1)%PAGE_TABLE_SIZE;
			
			set_bit((uint32_t*) &page_table[i].where, RAM_BIT, 1, 3);			
			
			int ram_addr = find_empty(&ram_bitmap, &ram_cursor, RAM_SIZE);
			
			if (ram_addr < 0)
				ram_addr = evict();//page fault
			else
				set_bit(&ram_bitmap, ram_addr, 1, RAM_SIZE); 
				
			page_table[i].ram_addr = ram_addr;
			
			int j;
			
			for (j = 0; j < PAGE_TABLE_SIZE; j++)
				print_page_entry(page_table[j]);
			printf("------------\n");
			
			return i;
		}
		i = (i+1)%PAGE_TABLE_SIZE;
	
	}while(i != ptable_cursor);
	
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
