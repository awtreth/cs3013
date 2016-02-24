#include <stdint.h>

#include "memory.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

//CONSTANTS

#define RAM_SIZE 3
#define SSD_SIZE 6
#define HDD_SIZE 15

#define PAGE_TABLE_SIZE HDD_SIZE

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


//STRUCTURE

typedef struct {
	uint8_t flags;//Modified-Referenced
	uint8_t where;//R-S-H bits
	uint8_t ram_addr;
	uint8_t ssd_addr;
	//we don't need for hdd (vAddr directly maps to hdd_m)
} page_entry_t;

page_entry_t page_table[PAGE_TABLE_SIZE];//indexed by vAddr
vAddr ram_map[RAM_SIZE];
vAddr ssd_map[SSD_SIZE];


//Memory contents
uint32_t ram_m[RAM_SIZE];
uint32_t ssd_m[SSD_SIZE];
uint32_t hdd_m[HDD_SIZE];



uint32_t ram_bitmap;//32 bits (use the first 25)
uint32_t ssd_bitmap[4];//128 bits (use the first 100)
//we don't need for hhd (vAddr directly maps to hdd_m)

//Track the current position for FIFO and second chance eviction algorithm implementation
int  ptable_cursor = 0;//cursor along page_table for create_page_table
int  ram_cursor = 0;//cursor along ram_bitmap
int  ssd_cursor = 0;//cursor along ssd_bitmap

//GENERIC BIT OPERATIONS
//bitmap can be a sigle integer or an array
int find_empty(uint32_t* bitmap, int* cursor, int size){
	
	int i = *cursor;//get the current value of the cursor
	
	do {
		if( !(bitmap[i/size] & (1 << i%size)) ){//if it's zero
			*cursor = (i+1)%size;//update cursor
			return i;//return the memory position
		}
		i = (i+1)%size;//increment iterator (circular way)
	}while(i != *cursor);//checked all the memory
	
	return -1;
}

//Generic bitset function
/*
 * @param number: pointer to the number (or array) - start point
 * @param pos: bit position from the start point
 * @param bit_value: 0 or !0
 * @param size: size of the provided bitmap (number)
 */
void set_bit(uint32_t* number, int pos, int bit_value, int size) {
	if(bit_value) number[pos/size] |= (1 << pos%size);
	else number[pos/size] &= ~(1 << pos%size);
}

//Generic bitget function
/*
 * @param number: pointer to the number (or array) - start point
 * @param pos: bit position from the start point
 * @param size: size of the provided bitmap (number)  in bits
 * 
 * @return bit value (1 or 0)
 */
int get_bit(uint32_t* number, int pos, int size){
	return (number[pos/size] & (1 << pos%size))&&1;
}

//Auxiliar function (return a random number between low and high)
int uniform_rand(int low, int high) {
	return rand()%(abs(high-low)+1)+low;
}



//AUXILIAR MEMORY MANAGEMENT FUNCTIONS

/* Find a page to be evicted using FIFO algorithm
 * 
 * @param mem_bit:  where you want to evict from (RAM_BIT or SSD_BIT)
 * @param mem_size: RAM_SIZE or SSD_SIZE (based on mem_bit)
 * 
 * @return the evicted vAddr (page table is properly updated)
 */
vAddr find_evict_first(int mem_bit, int mem_size) {
	int i = ptable_cursor;
	
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
	int ssd_maddr;
	vAddr ram_evicted_address, ssd_evicted_address;
	
	//Virtual address of the process to be evicted from ram
	ram_evicted_address = find_evict_first(RAM_BIT, RAM_SIZE); 
	
	printf("evicted %d from ram\n", ram_evicted_address);
	
	//vAddr in ssd. Potencially can be evicted from ssd
	ssd_maddr = find_empty(&ssd_bitmap[0], &ssd_cursor, SSD_SIZE);
	
	if(ssd_maddr < 0) {//full
		ssd_evicted_address = find_evict_first(SSD_BIT, SSD_SIZE);
		ssd_maddr =  page_table[ssd_evicted_address].ssd_addr;
		
		printf("evicted %d from ssd\n", ssd_evicted_address);
		//Copy from SSD to HDD
		
		//set bits in page table
		set_bit((uint32_t*) &page_table[ssd_evicted_address].where, SSD_BIT, 0, 3);//take out from ssd
		set_bit((uint32_t*) &page_table[ssd_evicted_address].where, HDD_BIT, 1, 3);//put into hdd
		
		//access time
		usleep(HDD_TIME*1e6 + SSD_TIME*1e6);
		
		//copy the values
		hdd_m[ssd_evicted_address] = ssd_m[ssd_maddr];
	}else {
		set_bit(&ssd_bitmap[0], ssd_maddr, 1, SSD_SIZE);
	}
	//print_bitmap(&ssd_bitmap[0], SSD_SIZE);
	//printf("\n");
	//Copy from RAM to SSD
	
	//set bits in page table
	set_bit((uint32_t*) &page_table[ram_evicted_address].where, RAM_BIT, 0, 3);
	set_bit((uint32_t*) &page_table[ram_evicted_address].where, SSD_BIT, 1, 3);
	
	//access time
	usleep(RAM_TIME*1e6 + SSD_TIME*1e6);
	
	//copy the values
	page_table[ram_evicted_address].ssd_addr = ssd_maddr;
	ssd_m[ssd_maddr] = ram_m[page_table[ram_evicted_address].ram_addr];

	return page_table[ram_evicted_address].ram_addr;//return evicted ram_addr
}

int page_fault(vAddr address){
	int ram_addr = find_empty(&ram_bitmap, &ram_cursor, RAM_SIZE);
	if(ram_addr<0)
		ram_addr = evict();
	else
		set_bit(&ram_bitmap, ram_addr, 1, RAM_SIZE);
	
	page_table[address].ram_addr = ram_addr;
	
	set_bit((uint32_t*) &page_table[address].where, RAM_BIT, 1, 3);
	
	if(get_bit((uint32_t*) &page_table[address].where, SSD_BIT, 3)) {
		//set bits in page table
		set_bit((uint32_t*) &page_table[address].where, SSD_BIT, 0, 3);
		set_bit(&ssd_bitmap[0], page_table[address].ssd_addr, 0, SSD_SIZE);
		usleep( (RAM_TIME+SSD_TIME)*1e6 );
		ram_m[ram_addr] = ssd_m[page_table[address].ssd_addr];
	}else {//assume HHD_BIT = 1
		usleep( (RAM_TIME+HDD_TIME)*1e6 );
		ram_m[ram_addr] = hdd_m[address];
	}
	
	return ram_addr;
}


//API functions

vAddr create_page() {
	int i = ptable_cursor;
	
	do{
		if(!page_table[i].where){//not used page-table
			//update page table
			ptable_cursor = (i+1)%PAGE_TABLE_SIZE;
			
			set_bit((uint32_t*) &page_table[i].where, RAM_BIT, 1, 3);			
			
			int ram_addr = find_empty(&ram_bitmap, &ram_cursor, RAM_SIZE);
			
			if (ram_addr < 0)
				ram_addr = evict();
			else
				set_bit(&ram_bitmap, ram_addr, 1, RAM_SIZE); 
				
			page_table[i].ram_addr = ram_addr;
			
			return i;
		}
		i = (i+1)%PAGE_TABLE_SIZE;
	
	}while(i != ptable_cursor);
	
	return -1;//memory is full
}





uint32_t get_value(vAddr address, int* valid) {
	int ram_addr;
	if(page_table[address].where) {
		if(get_bit((uint32_t*) &page_table[address].where, RAM_BIT, 3)){
			ram_addr = page_table[address].ram_addr;
		}else{
			ram_addr = page_fault(address);
		}
		
		*valid = 1;
		usleep(RAM_TIME*1e6);
		return ram_m[ram_addr];
	}
	
	*valid = 0;
	return 0;
}

void store_value(vAddr address, uint32_t *value) {
	int ram_addr;
	if(page_table[address].where) {
		if(get_bit((uint32_t*) &page_table[address].where, RAM_BIT, 3)){
			ram_addr = page_table[address].ram_addr;
		}else{
			ram_addr = page_fault(address);
		}
		
		usleep(RAM_TIME*1e6);
		ram_m[ram_addr] = *value;
	}
}

void free_page(vAddr address) {
	
	if(get_bit((uint32_t*) &page_table[address].where, RAM_BIT, 3))
		set_bit(&ram_bitmap, page_table[address].ram_addr, 0, RAM_SIZE);
	
	if(get_bit((uint32_t*) &page_table[address].where, SSD_BIT, 3))
		set_bit(&ssd_bitmap[0], page_table[address].ssd_addr, 0, SSD_SIZE);
	
	page_table[address].flags = 0;
	page_table[address].where = 0;
	page_table[address].ram_addr = 0;
	page_table[address].ssd_addr = 0;
}


//PRINTING FUNCTIONS
//size of the provided bitmap (number)  in bits
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

void print_page_table() {
	int j;
			
	for (j = 0; j < PAGE_TABLE_SIZE; j++)
		print_page_entry(page_table[j]);
}

void print_bitmaps() {
	print_bitmap(&ram_bitmap, RAM_SIZE);
	printf("\n");
	print_bitmap(&ssd_bitmap[0], SSD_SIZE);
	printf("\n");
}



//print memory content (specify which memory: ram_m, ssd_m or hdd_m)
void print_memory(uint32_t* mem, int size){
	int i = 0;
	for (i = 0; i < size; i++)
		printf("%d ", mem[i]);
	printf("\n");
}

//print all memory content (ram, sdd and hdd)
void print_memory_all() {
	print_memory(ram_m, RAM_SIZE);
	print_memory(ssd_m, SSD_SIZE);
	print_memory(hdd_m, HDD_SIZE);
}
