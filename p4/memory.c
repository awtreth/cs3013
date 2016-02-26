#include <stdint.h>

#include "memory.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

//CONSTANTS


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

typedef struct {
	uint8_t flags;
	uint16_t addr;
} page_entry_t;

//auxiliar struct
typedef struct{
	uint16_t cursor;//Track the current position for clock and clock with 2nd chance eviction algorithms implementation
	uint8_t size;//auxiliar
	pthread_mutex_t mtx;
	uint8_t* bitmap;
} mem_map_t;

pthread_mutex_t ptable_mtx[PAGE_TABLE_SIZE];

uint8_t ram_bitmap[RAM_SIZE/8+1];
uint8_t ssd_bitmap[SSD_SIZE/8+1];

//mem_map_t ram_map = {0, RAM_SIZE, ram_bitmap, 0};
//mem_map_t ssd_map = {0, SSD_SIZE, ssd_bitmap, 0};

mem_map_t mem_map[2];//indexed by RAM_BIT and SSD_BIT

page_entry_t page_table[PAGE_TABLE_SIZE];//indexed by vAddr

//Memory contents
uint32_t ram_m[RAM_SIZE];
uint32_t ssd_m[SSD_SIZE];
uint32_t hdd_m[HDD_SIZE];

//Auxiliar function (return a random number between low and high)
int uniform_rand(int low, int high) {
	return rand()%(abs(high-low))+low;
}

//Generic bitset function
/*
 * @param number: pointer to the number
 * @param pos: bit position
 * @param bit_value: 0 or !0
 */
void set_bit(uint8_t* number, int pos, int bit_value) {
	if(bit_value) number[pos/8] |= (1 << pos%8);
	else number[pos/8] &= ~(1 << pos%8);
}

//Generic bitget function
/*
 * @param number: pointer to the number
 * @param pos: bit position
 * 
 * @return bit value (1 or 0)
 */
int get_bit(uint8_t* number, int pos){
	return (number[pos/8] & (1 << pos%8)) && 1;
}

/* Select the vAddr placed in mem_bit (RAM or SSD) memory
 */
vAddr evict_select_random(int mem_bit){//assume memory is full
	int addr = uniform_rand(0, PAGE_TABLE_SIZE);//select a memory position
	int i=addr;//start point
	
	while(1) {//go through all pages looking for the selected mem_addr
	
		if(get_bit(&page_table[i].flags, mem_bit)) {//if it's in the specified memory
			pthread_mutex_lock(&ptable_mtx[i]);
			if(get_bit(&page_table[i].flags, mem_bit) == 0){
				pthread_mutex_unlock(&ptable_mtx[i]);
				continue;
			}
			return i;
		}
		i = (i+1)%PAGE_TABLE_SIZE;
	}
	printf("Not supposed to be here. evict_select_random\n");
	return mem_map[mem_bit].cursor;//not supposed to reach this position
}

/* 
 * */
vAddr evict_select_clock(int mem_bit) {
	
	vAddr i = mem_map[mem_bit].cursor;
	
	do {
		if(get_bit(&page_table[i].flags, mem_bit)){
			pthread_mutex_lock(&ptable_mtx[i]);
			if(!get_bit(&page_table[i].flags, mem_bit)){
				pthread_mutex_unlock(&ptable_mtx[i]);
				continue;
			}
			mem_map[mem_bit].cursor = (i+1)%PAGE_TABLE_SIZE;
			return i;
		}
		i = (i+1)%PAGE_TABLE_SIZE;//increment iterator (circular)
	}while(i != mem_map[mem_bit].cursor);//checked all the memory
	
	printf("Not supposed to be here. evict_select_clock\n");
	//not supposed to reach this line
	mem_map[mem_bit].cursor = (i+1)%PAGE_TABLE_SIZE;
	return i;
}

/*
 */
vAddr evict_select_clock2(int mem_bit) {
	
	vAddr i = mem_map[mem_bit].cursor;

	do {//first chance
		//TODO: ptable_mutex_lock
		if(!pthread_mutex_trylock(&ptable_mtx[i])) {
			if(get_bit(&page_table[i].flags, mem_bit)) {
				if(get_bit(&page_table[i].flags, R_BIT)) {//if R_BIT is set
					set_bit(&page_table[i].flags, R_BIT, 0);//set it to zero
				}else {//not referenced
					mem_map[mem_bit].cursor = (i+1)%PAGE_TABLE_SIZE;//update cursor
					set_bit(&page_table[i].flags, R_BIT, 1);//this page will potentially go to ssd
					return i;
				}
			}
			pthread_mutex_unlock(&ptable_mtx[i]);
		}
		i = (i+1)%PAGE_TABLE_SIZE;//increment iterator (circular)
	}while(i != mem_map[mem_bit].cursor);//checked all the memory

	//this page will potentially go to ssd
	
	i = evict_select_clock(mem_bit);//second loop
	
	set_bit(&page_table[i].flags, R_BIT, 1);//this page will potentially go to ssd
	return i;
}

vAddr (*evict_select)(int);

void init_memory(int eviction_method, int seed) {
	
	switch (eviction_method) {
		case RANDOM_EVICT: evict_select = evict_select_random; break;
		case CLOCK_EVICT: evict_select = evict_select_clock;   break;
		case CLOCK2_EVICT: evict_select = evict_select_clock2; break;
		default: break;
	}
	
	mem_map[RAM_BIT].bitmap = ram_bitmap;
	mem_map[SSD_BIT].bitmap = ssd_bitmap;
	mem_map[RAM_BIT].size = RAM_SIZE;
	mem_map[SSD_BIT].size = SSD_SIZE;
	
	pthread_mutex_init(&mem_map[RAM_BIT].mtx, NULL);
	pthread_mutex_init(&mem_map[SSD_BIT].mtx, NULL);
	
	int i;
	for (i = 0; i < PAGE_TABLE_SIZE; i++)
		pthread_mutex_init(&ptable_mtx[i], NULL);
	
	srand(seed);
}

void destroy_memory(){
	pthread_mutex_destroy(&mem_map[RAM_BIT].mtx);
	pthread_mutex_destroy(&mem_map[SSD_BIT].mtx);
		
	int i;
	for (i = 0; i < PAGE_TABLE_SIZE; i++)
		pthread_mutex_destroy(&ptable_mtx[i]);
}

/* Find empty spot
 */
 
int find_empty(int mem_bit){
	
	static int last_empty[2] = {0,0};
	pthread_mutex_lock(&mem_map[mem_bit].mtx);
	int i = last_empty[mem_bit];
	
	do {//TODO: mutual exclusion
		if(get_bit(mem_map[mem_bit].bitmap, i)==0) {//if it's zero
			set_bit(mem_map[mem_bit].bitmap, i, 1);//set it to one
			last_empty[mem_bit] = (i+1)%mem_map[mem_bit].size;//update cursor
			pthread_mutex_unlock(&mem_map[mem_bit].mtx);
			return i;//return the memory position
		}
		i = (i+1)%mem_map[mem_bit].size;//increment iterator (circular way)
	}while(i != last_empty[mem_bit]);//checked all the memory
	
	pthread_mutex_unlock(&mem_map[mem_bit].mtx);
	return -1;//didn't find
}

//Evict the page in ram_addr position in RAM memory
void evict(vAddr ram_evicted, int release) {//user has already locked ram_evicted
	
	int ssd_addr = find_empty(SSD_BIT);//find a place in ssd to put the evicted page
	int ram_addr = page_table[ram_evicted].addr;
	
	if(ssd_addr < 0){//didn't find empty spot
		vAddr ssd_evicted = evict_select(SSD_BIT);//find page to be evicted from ssd
		//user has already locked ssd_evicted
		
		int hdd_addr = ssd_evicted;
		ssd_addr = page_table[ssd_evicted].addr;
		page_table[ssd_evicted].addr = hdd_addr;
		
		printf("evicting from ssd_addr %d to hdd_addr %d\n", ssd_addr, hdd_addr);
		set_bit(&page_table[ssd_evicted].flags, SSD_BIT, 0);
		set_bit(&page_table[ssd_evicted].flags, HDD_BIT, 1);
		
		
		usleep(HDD_TIME*1e6 + SSD_TIME*1e6);//access time
		hdd_m[hdd_addr] = ssd_m[ssd_addr];
		pthread_mutex_unlock(&ptable_mtx[ssd_evicted]);
	}
	
	printf("evicting from ram_addr %d to ssd_addr %d\n", ram_addr, ssd_addr);
	set_bit(&page_table[ram_evicted].flags, RAM_BIT, 0);
	set_bit(&page_table[ram_evicted].flags, SSD_BIT, 1);
	page_table[ram_evicted].addr = ssd_addr;
	
	usleep(RAM_TIME*1e6 + SSD_TIME*1e6);//access time
	ssd_m[ssd_addr] = ram_m[ram_addr];
	
	if(release)pthread_mutex_unlock(&ptable_mtx[ram_evicted]);
	
}


int page_fault(vAddr address, vAddr ram_evicted, int ram_addr){//assume the specified page is not in RAM
	int  ssd_addr;
	
	if(get_bit(&page_table[address].flags, SSD_BIT)){//if it is on SSD
		
		ssd_addr = page_table[address].addr;
		usleep( SSD_TIME*1e6 );
		uint32_t tmp = ssd_m[ssd_addr];
		//evict after freeing ssd
		
		if(ram_evicted!=NULL_VADDR) {//it needs to be evicted
			set_bit(&page_table[ram_evicted].flags, RAM_BIT, 0);
			set_bit(&page_table[ram_evicted].flags, SSD_BIT, 1);
			page_table[ram_evicted].addr = ssd_addr;
			
			usleep(RAM_TIME*1e6 + SSD_TIME*1e6);//access time
			ssd_m[ssd_addr] = ram_m[ram_addr];
		}else{
			ssd_m[ssd_addr] = 0;//not necessary (here just for visibility)
			pthread_mutex_lock(&mem_map[SSD_BIT].mtx);
			set_bit(ssd_bitmap, ssd_addr, 0);
			pthread_mutex_unlock(&mem_map[SSD_BIT].mtx);
		}
		
		printf("page_fault of page %d from ssd_addr %d to ram_addr %d\n", address, ssd_addr, ram_addr);
		usleep( RAM_TIME*1e6 );
		ram_m[ram_addr] = tmp;
		
		set_bit(&page_table[address].flags, SSD_BIT, 0);//unset page_table HDD_BIT
		
	}else if(get_bit(&page_table[address].flags, HDD_BIT)){ //assume HDD_BIT is set
		int hdd_addr = address;
		//int ram_addr = find_empty(RAM_BIT);
				
		if (ram_evicted!=NULL_VADDR)
			evict(ram_evicted,0);
		
		printf("page_fault of page %d from hhd_addr %d to ram_addr %d\n", address, hdd_addr, ram_addr);
		usleep( (RAM_TIME+HDD_TIME)*1e6 );
		set_bit(&page_table[address].flags, HDD_BIT, 0);//unset page_table HDD_BIT
		ram_m[ram_addr] = hdd_m[hdd_addr];
		hdd_m[hdd_addr] = 0;//not necessary (here just for visibility)
	}
		
	//update page table
	page_table[address].addr = ram_addr;//update ram_addr in page_table
	set_bit(&page_table[address].flags, RAM_BIT, 1);//set RAM_BIT in page_table
	//if(ram_evicted!=NULL_VADDR) pthread_mutex_unlock(&ptable_mtx[ram_evicted]);
	
	return ram_addr;
}


//API functions

vAddr create_page() {
	static int ptable_cursor = 0;
	
	int i = ptable_cursor;
	
	do{
		if(!pthread_mutex_trylock(&ptable_mtx[i])) {
			if(!(page_table[i].flags & (0b1111)) ){//not used page-table
				//update page table
				
				int ram_addr = find_empty(RAM_BIT);
				
				if (ram_addr < 0){
					vAddr ram_evicted = evict_select(RAM_BIT);
					ram_addr = page_table[ram_evicted].addr;
					evict(ram_evicted, 1);
				}
				
				//update page table
				set_bit(&page_table[i].flags, RAM_BIT, 1);			
				//set_bit(&page_table[i].flags, R_BIT, 1);			
				page_table[i].addr = ram_addr;
				
				
				ptable_cursor = (i+1)%PAGE_TABLE_SIZE;
				pthread_mutex_unlock(&ptable_mtx[i]);
				return i;
			}//else
			pthread_mutex_unlock(&ptable_mtx[i]);
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
	pthread_mutex_lock(&ptable_mtx[address]);
	if(page_table[address].flags & (0b111)) {//it's somewhere
		if(get_bit(&page_table[address].flags, RAM_BIT)){
			ram_addr = page_table[address].addr;
		}else{
			pthread_mutex_unlock(&ptable_mtx[address]);
			vAddr ram_evicted = NULL_VADDR;
	
			ram_addr = find_empty(RAM_BIT);//find a place in RAM
			if(ram_addr < 0){
				ram_evicted = evict_select(RAM_BIT);
				ram_addr = page_table[ram_evicted].addr;
			}
			pthread_mutex_lock(&ptable_mtx[address]);
			page_fault(address, ram_evicted, ram_addr);
			if(ram_evicted!=NULL_VADDR) pthread_mutex_unlock(&ptable_mtx[ram_evicted]);
			
		}
		
		*valid = 1;
		
		set_bit(&page_table[address].flags, R_BIT, 1);
		usleep(RAM_TIME*1e6);
		uint32_t value = ram_m[ram_addr];
		pthread_mutex_unlock(&ptable_mtx[address]);
		return value;
	}
	*valid = 0;
	return 0;
}

void store_value(vAddr address, uint32_t *value) {
	if(address < 0 || address >= PAGE_TABLE_SIZE)
		return;
	
	int ram_addr;
	
	pthread_mutex_lock(&ptable_mtx[address]);
	
	if(page_table[address].flags & (0b111)) {
		if(get_bit(&page_table[address].flags, RAM_BIT)){
			ram_addr = page_table[address].addr;
		}else{
			pthread_mutex_unlock(&ptable_mtx[address]);
			vAddr ram_evicted = NULL_VADDR;
	
			ram_addr = find_empty(RAM_BIT);//find a place in RAM
			if(ram_addr < 0){
				ram_evicted = evict_select(RAM_BIT);
				ram_addr = page_table[ram_evicted].addr;
			}
			pthread_mutex_lock(&ptable_mtx[address]);
			ram_addr = page_fault(address, ram_evicted, ram_addr);
			if(ram_evicted!=NULL_VADDR) pthread_mutex_unlock(&ptable_mtx[ram_evicted]);
		}
		
		set_bit(&page_table[address].flags, R_BIT, 1);
		usleep(RAM_TIME*1e6);
		ram_m[ram_addr] = *value;
		pthread_mutex_unlock(&ptable_mtx[address]);
	}
}

void free_page(vAddr address) {
	
	if(0 <= address && address < PAGE_TABLE_SIZE) {
		pthread_mutex_lock(&ptable_mtx[address]);
			
		if(get_bit(&page_table[address].flags, RAM_BIT)){
			ram_m[page_table[address].addr] = 0;
			pthread_mutex_lock(&mem_map[RAM_BIT].mtx);
			set_bit(mem_map[RAM_BIT].bitmap, page_table[address].addr, 0);
			pthread_mutex_unlock(&mem_map[RAM_BIT].mtx);
		}else if(get_bit(&page_table[address].flags, SSD_BIT)){
			ssd_m[page_table[address].addr] = 0;
			pthread_mutex_lock(&mem_map[SSD_BIT].mtx);
			set_bit(mem_map[SSD_BIT].bitmap, page_table[address].addr, 0);
			pthread_mutex_unlock(&mem_map[SSD_BIT].mtx);
		}else{
			hdd_m[address] = 0;
		}
			page_table[address].flags = 0;
			page_table[address].addr = 0;
		pthread_mutex_unlock(&ptable_mtx[address]);
	}
}

void print_page_entry(page_entry_t entry) {
	int i;
	for (i = 0; i < N_FLAGS; i++){
		printf("%d", get_bit(&entry.flags, i));
	}
	printf(" | %d | ", entry.addr);
	if(get_bit(&entry.flags, RAM_BIT))
		printf("%d\n", ram_m[entry.addr]);
	if(get_bit(&entry.flags, SSD_BIT))
		printf("%d\n", ssd_m[entry.addr]);
	if(get_bit(&entry.flags, HDD_BIT))
		printf("%d\n", hdd_m[entry.addr]);

}

void print_page_table() {
	int j;
			
	for (j = 0; j < PAGE_TABLE_SIZE; j++){
		printf("%d) ", j);
		print_page_entry(page_table[j]);
	}
}

void print_mem_map(int mem_bit) {
	int i;
	printf("%d | ", mem_map[mem_bit].cursor);
	for (i = 0; i < mem_map[mem_bit].size; i++)
		printf("%d ", get_bit(mem_map[mem_bit].bitmap, i));
	printf("\n");
}

void print_mem_maps() {
	print_mem_map(RAM_BIT);
	print_mem_map(SSD_BIT);
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
