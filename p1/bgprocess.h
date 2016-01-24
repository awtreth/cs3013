#ifndef BGPROCESS_H
#define BGPROCESS_H

#include <unistd.h>

#include <sys/time.h>
#include <sys/resource.h>

/* Store useful background process information
 */
typedef struct bgprocess{
	pid_t pid;
	struct timeval init_time;
	char* name;//TODO: make it static (much simpler, althought we need to assume input state)
	struct bgprocess *next;
	int number;
} bgprocess;

/* bgprocess Linked List
 */
typedef struct bgprocessLL{
	bgprocess* first;
	int size;
} bgprocessLL;


/*Create a bgprocess instance
 * 
 * It allocates memory for name dinamically. Then, you need to call free_bgprocess_name
 *
 * @see bgprocessLL
 * @see free_bgprocess_name
 */
 //TODO: make it simpler, without dynamic allocation
bgprocess init_bgprocess(pid_t pid, struct timeval init_time, char* name, int number, bgprocess* next);


/* Free name pointer of bgprocess variable
 * 
 * TODO: make "name" static (simpler)
 */
void free_bgprocess_name(bgprocess* bgp);

/*Initialize bgprocessLL variable with zero and NULL values
 */
bgprocessLL init_bgprocessLL();

/* Add bgprocess "bgp" into a bgprocessLL "bgpLL"
 * 
 * Add bgp on the beginning of the list and pops from the beggining as well, like a stack
 * */
int add2bgprocessLL(bgprocessLL* bgpLL, bgprocess bgp);

/* Remove bgprocess "bgp" from a bgprocessLL "bgpLL" based on the pid
 * 
 * Returns NULL when it doesn't find it
 * */
bgprocess remove_bgprocess(bgprocessLL* bgpLL, pid_t pid);

/* Print bgprocess content + custom complement (TODO)
 * 
 * [number] pid name complement
 */
void print_bgprocess(bgprocess proc, const char* complement);

/* Print the contents in the linkedlist (in the oposite order. Stack) TODO: make it a queue
 */
void print_bgprocessLL(bgprocessLL bgpLL);

/* "Wait" for background processes in bgpLL
 * 
 * It returns imediatally if no one is done
 * It also print the report of the finished bgprocess 
 */
void check_background_processes(bgprocessLL * bgpLL, int wait_option);

/* Wait all wait4bgprocess in bgpLL finish
 */
void wait4bgprocess(bgprocessLL *bgpLL);

#endif
