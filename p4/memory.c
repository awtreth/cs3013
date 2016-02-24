#include <stdint.h>

#include "memory.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

//CONSTANTS

#define RAM_SIZE 3
#define SSD_SIZE 6
#define HDD_SIZE 15

#define PAGE_TABLE_SIZE HDD_SIZE

//Access time (in seconds)
#define TIME_FACTOR 100
#define RAM_TIME .01/TIME_FACTOR
#define SSD_TIME .1/TIME_FACTOR
#define HDD_TIME 2.5/TIME_FACTOR


//STRUCTURE
#define RAM_BIT 0
#define SSD_BIT 1
#define HDD_BIT 2
#define R_BIT	3 //Reference bit (set whenever the user read or write on a page)
#define N_FLAGS 4

#define NULL_VADDR -1

#define RANDOM_STRATEGY			0
#define FIFO_STRATEGY			1
#define SECOND_CHANCE_STRATEGY	2

#define STRATEGY SECOND_CHANCE_STRATEGY


typedef struct {
	uint8_t flags;
	uint8_t ram_addr;
	uint8_t ssd_addr;
	//we don't need for hdd (vAddr directly maps to hdd_m)
} page_entry_t;

typedef struct{
	uint8_t cursor;//Track the current position for FIFO and second chance eviction algorithm implementation
	uint8_t size;
	vAddr *map;
} mem_map_t;

vAddr ram_map_[] = {[0 ... RAM_SIZE]=NULL_VADDR};
vAddr ssd_map_[] = {[0 ... SSD_SIZE]=NULL_VADDR};

mem_map_t ram_map = {0, RAM_SIZE, ram_map_ };
mem_map_t ssd_map = {0, SSD_SIZE, ssd_map_ };
//we don't need for hhd (vAddr directly maps to hdd_m)


page_entry_t page_table[PAGE_TABLE_SIZE];//indexed by vAddr

//Memory contents
uint32_t ram_m[RAM_SIZE];
uint32_t ssd_m[SSD_SIZE];
uint32_t hdd_m[HDD_SIZE];

#if STRATEGY == RANDOM_STRATEGY
//Auxiliar function (return a random number between low and high)
int uniform_rand(int low, int high) {
	static int flag=1;
	if(flag) {srand(time(NULL)); flag = 0;}
	return rand()%(abs(high-low))+low;
}
#endif


//Generic bitset function
/*
 * @param number: pointer to the number
 * @param pos: bit position
 * @param bit_value: 0 or !0
 */
void set_bit(uint8_t* number, int pos, int bit_value) {
	if(bit_value) *number |= (1 << pos);
	else *number &= ~(1 << pos);
}

//Generic bitget function
/*
 * @param number: pointer to the number
 * @param pos: bit position
 * 
 * @return bit value (1 or 0)
 */
int get_bit(uint8_t number, int pos){
	return (number & (1 << pos)) && 1;
}

int find_slot(mem_map_t* mem_map){
	
	int i = mem_map->cursor;//get the current value of the cursor
	
	do {
		if( mem_map->map[i] == NULL_VADDR){//not addressed
			mem_map->cursor = (i+1)%mem_map->size;//update cursor
			return i;//return the memory position
		}
		i = (i+1)%mem_map->size;//increment iterator (circular way)
	}while(i != mem_map->cursor);//checked all the memory
	

#if STRATEGY == RANDOM_STRATEGY
	return uniform_rand(0, mem_map->size);	
	
#elif STRATEGY == FIFO_STRATEGY
	//FIFO
	//select vAddress to be evicted
	mem_map->cursor = (i+1)%mem_map->size;
	return i;
#elif STRATEGY == SECOND_CHANCE_STRATEGY
	do {
		if(get_bit(page_table[mem_map->map[i]].flags, R_BIT)){//referenced
			set_bit(&page_table[mem_map->map[i]].flags, R_BIT, 0);
		}else {
			mem_map->cursor = (i+1)%mem_map->size;//update cursor
			set_bit(&page_table[mem_map->map[i]].flags, R_BIT, 1);//set to one to mark it as a new page in ssd
			return i;//return the memory position
		}
		i = (i+1)%mem_map->size;//increment iterator (circular way)
	}while(i != mem_map->cursor);//checked all the memory
	
	//this page will potentially go to ssd
	set_bit(&page_table[mem_map->map[i]].flags, R_BIT, 1);//set to one to mark it as a new page in ssd
	mem_map->cursor = (i+1)%mem_map->size;
	return i;
#endif

}



//Evict the page in ram_addr position in RAM memory
int evict(int ram_addr) {
	
	int ssd_addr;
	
	ssd_addr = find_slot(&ssd_map);//find a place in ssd to put the evicted page
	printf("evicted from ram_addr %d to ssd_addr %d\n", ram_addr, ssd_addr);
	
	if (ssd_map.map[ssd_addr] != NULL_VADDR) {//if this address is not free
		printf("evicted from ssd_addr %d to hdd_addr %d\n", ssd_addr, ssd_map.map[ssd_addr]);
		//EVICT FROM SSD TO HDD
		//update page table
		set_bit(&page_table[ssd_map.map[ssd_addr]].flags, SSD_BIT, 0);
		set_bit(&page_table[ssd_map.map[ssd_addr]].flags, HDD_BIT, 1);
		//update the memories itself
		usleep(HDD_TIME*1e6 + SSD_TIME*1e6);//access time
		hdd_m[ssd_map.map[ssd_addr]] = ssd_m[ssd_addr];
	}
	
	//EVICT FROM RAM TO SSD
	//update page table
	set_bit(&page_table[ram_map.map[ram_addr]].flags, RAM_BIT, 0);//unset RAM_BIT
	set_bit(&page_table[ram_map.map[ram_addr]].flags, SSD_BIT, 1);//set SSD_BIT
	page_table[ram_map.map[ram_addr]].ssd_addr = ssd_addr;//update ssd_addr in page_table
	
	//update mem_maps
	ssd_map.map[ssd_addr] = ram_map.map[ram_addr];//copy vAddr
	ram_map.map[ram_addr] = NULL_VADDR;//clean ram vAddr
	
	//update the memories itself
	usleep(RAM_TIME*1e6 + SSD_TIME*1e6);//access time
	ssd_m[ssd_addr] = ram_m[ram_addr];

	return 0;
}

int page_fault(vAddr address){//assume the specified page is not in RAM
	int ram_addr;
	
	
	if(get_bit(page_table[address].flags, SSD_BIT)){//if it is on SSD
		
		set_bit(&page_table[address].flags, SSD_BIT, 0);//unset page_table SSD_BIT
		ssd_map.map[page_table[address].ssd_addr] = NULL_VADDR;//update ssd_map 
		
		usleep( SSD_TIME*1e6 );
		uint32_t tmp = ssd_m[page_table[address].ssd_addr];
		
		//evict after freeing ssd
		ram_addr = find_slot(&ram_map);//find a place in RAM
		if(ram_map.map[ram_addr] != NULL_VADDR)//not necesasry
			evict(ram_addr);
		
		printf("page_fault of page %d from ssd_addr %d to ram_addr %d\n", address, page_table[address].ssd_addr, ram_addr);
		usleep( RAM_TIME*1e6 );
		ram_m[ram_addr] = tmp;
	}else { //assume HDD_BIT is set
		ram_addr = find_slot(&ram_map);//find a place in ssd to put the evicted page;
		if(ram_map.map[ram_addr] != NULL_VADDR)//not necessary
			evict(ram_addr);
		//ram_addr is free
		
		printf("page_fault of page %d from hhd_addr %d to ram_addr %d\n", address, address, ram_addr);
		usleep( (RAM_TIME+HDD_TIME)*1e6 );
		set_bit(&page_table[address].flags, HDD_BIT, 0);//unset page_table HDD_BIT
		ram_m[ram_addr] = hdd_m[address];
	}
	
	
	ram_map.map[ram_addr] = address;//update ram_map
	
	//update page table
	page_table[address].ram_addr = ram_addr;//update ram_addr in page_table
	set_bit(&page_table[address].flags, RAM_BIT, 1);//set RAM_BIT in page_table
	
	return ram_addr;
}


//API functions

vAddr create_page() {
	static int ptable_cursor = 0;
	
	int i = ptable_cursor;
	
	do{
		if(!(page_table[i].flags & (0b111)) ){//not used page-table
			//update page table
			
			int ram_addr = find_slot(&ram_map);
			
			if (ram_map.map[ram_addr] != NULL_VADDR)
				evict(ram_addr);
			
			//update page table
			set_bit(&page_table[i].flags, RAM_BIT, 1);			
			ram_map.map[ram_addr] = i; 
			page_table[i].ram_addr = ram_addr;
			
			ptable_cursor = (i+1)%PAGE_TABLE_SIZE;
			return i;
		}
		i = (i+1)%PAGE_TABLE_SIZE;
	
	}while(i != ptable_cursor);
	
	return -1;//memory is full
}


uint32_t get_value(vAddr address, int* valid) {
	if(address < 0 || address >= PAGE_TABLE_SIZE){
		*valid = 0;
		return 0 ;
	}
	
	int ram_addr;
	
	if(page_table[address].flags & (0b111)) {//it's somewhere
		if(get_bit(page_table[address].flags, RAM_BIT)){
			ram_addr = page_table[address].ram_addr;
		}else{
			ram_addr = page_fault(address);
		}
		
		*valid = 1;
		
		set_bit(&page_table[address].flags, R_BIT, 1);
		usleep(RAM_TIME*1e6);
		return ram_m[ram_addr];
	}
	*valid = 0;
	return 0;
}

void store_value(vAddr address, uint32_t *value) {
	if(address < 0 || address >= PAGE_TABLE_SIZE)
		return;
	
	int ram_addr;
	if(page_table[address].flags & (0b111)) {
		if(get_bit(page_table[address].flags, RAM_BIT)){
			ram_addr = page_table[address].ram_addr;
		}else{
			ram_addr = page_fault(address);
		}
		
		set_bit(&page_table[address].flags, R_BIT, 1);
		usleep(RAM_TIME*1e6);
		ram_m[ram_addr] = *value;
	}
}

void free_page(vAddr address) {
	
	if(0 <= address && address < PAGE_TABLE_SIZE) {
		if(get_bit(page_table[address].flags, RAM_BIT))
			ram_map.map[page_table[address].ram_addr] = NULL_VADDR;

		if(get_bit(page_table[address].flags, SSD_BIT))
			ssd_map.map[page_table[address].ssd_addr] = NULL_VADDR;
			
			page_table[address].flags = 0;
			page_table[address].ram_addr = 0;
			page_table[address].ssd_addr = 0;
	}
}

void print_page_entry(page_entry_t entry) {
	int i;
	for (i = 0; i < N_FLAGS; i++)
		printf("%d", get_bit(entry.flags, i));
	printf(" | %d | %d\n", entry.ram_addr, entry.ssd_addr);
}

void print_page_table() {
	int j;
			
	for (j = 0; j < PAGE_TABLE_SIZE; j++){
		printf("%d) ", j);
		print_page_entry(page_table[j]);
	}
}

void print_mem_map(mem_map_t mem_map) {
	int i;
	printf("%d | ", mem_map.cursor);
	for (i = 0; i < mem_map.size; i++)
		printf("%d ", mem_map.map[i]);
	printf("\n");
}

void print_mem_maps() {
	print_mem_map(ram_map);
	print_mem_map(ssd_map);
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
