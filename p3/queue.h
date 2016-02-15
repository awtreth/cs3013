#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <string.h>

//SET OF MACROS FOR GENERIC QUEUES

/* Define the structure for an especific TYPE
 * e.g.: define_queue(double) defines the queue_double struct (queue of doubles)
 */
#define define_queue(TYPE)\
	typedef struct {\
		unsigned int size;\
		unsigned int init;\
		unsigned int end;\
		unsigned int capacity;\
		TYPE *array;\
	} queue_##TYPE;

define_queue(int);//Default int queue

/*Initialize the queue with fixed capacity
 * queue: pointer to a queue
 * CAPACITY: int
 */
#define queue_init(queue, CAPACITY) \
		(queue)->size = 0;\
		(queue)->init = 1;\
		(queue)->end =  0;\
		(queue)->capacity = CAPACITY;\
		(queue)->array = malloc(sizeof(typeof(*((queue)->array)))*CAPACITY);

/*Free queue content
 * queue: pointer to a queue 
 */
#define queue_free(queue) {\
	if((queue)->array != NULL) free((queue)->array);\
	(queue)->size = 0;\
	(queue)->init = 1;\
	(queue)->end =  0;\
	(queue)->capacity = 0;\
}

/*Push at the begining of the recipe queue
 * queue: pointer to a queue
 * value: input value of the type of the queue
 * return the resulting size
 */
#define queue_push(queue, value)({\
		int ret = 0;\
		if((queue)->size >= (queue)->capacity) {\
			printf("Someone tried to push on a full queue\n");exit(-1);\
			ret = -1;\
		}else{\
			(queue)->init = ((queue)->init + (queue)->capacity - 1)%(queue)->capacity;\
			(queue)->array[(queue)->init] = value;\
			ret = ++(queue)->size;\
		}\
		ret;\
	})

/*return !0 if the queue is empty. 0 if not
 * queue: struct "object" (not pointer)
*/
#define queue_is_empty(queue) ({\
	int ret = (queue).size<=0;\
	ret;\
	})

/*pop from the end of the queue
 * queue: pointer to the queue
 * return the popped object
*/
#define queue_pop(queue)({\
		if((queue)->size<=0){\
			printf("Someone tried to pop from an empty queue\n");\
		}\
		unsigned int old_end = (queue)->end;\
		(queue)->end = ((queue)->end + (queue)->capacity - 1)%(queue)->capacity;\
		--(queue)->size;\
		typeof(*((queue)->array)) ret = (queue)->array[old_end];\
		ret;\
	})

/* get the value of "queue" (not pointer) in the position "pos"
 */
#define queue_get(queue, pos) ({\
		if(pos >= (queue).size || pos < 0) {printf("Someone tried to get a value of an invalid position in a queue\n");exit(-1);}\
		typeof(*((queue).array)) ret =(queue).array[((queue).end+(queue).capacity-pos)%(queue).capacity];\
		ret;\
	})

/* remove element (return value) from "queue"(pointer) of the position "pos"
 */
#define queue_rem(queue, pos) ({\
		typeof(*((queue)->array)) tmp = queue_get(*(queue), pos);\
		int kkk=((queue)->end+(queue)->capacity-pos)%(queue)->capacity;\
		while(kkk != (queue)->init) {\
			(queue)->array[(kkk+(queue)->capacity)%(queue)->capacity]=(queue)->array[(kkk-1+(queue)->capacity)%(queue)->capacity];\
			kkk=(kkk-1+(queue)->capacity)%(queue)->capacity;\
		}\
		(queue)->init = ((queue)->init+1)%(queue)->capacity;\
		(queue)->size = (queue)->size-1;\
		tmp;})
		
/*Copy queue from "from" to "to" (both arguments are pointers
 * the function assumes that "to" is properly allocated
*/
#define queue_copy(from, to) {\
	if((from)->capacity != (to)->capacity) {printf("In queue_copy: queues' capacities don't match\n");}\
	else{\
	(to)->size = (from)->size;\
	(to)->init = (from)->init;\
	(to)->end = (from)->end;\
	memcpy((to)->array, (from)->array, (from)->capacity*sizeof(typeof(*((from)->array))));}}

//check if "queue" (not pointer) has the element "elem"
#define queue_find(queue, elem) ({\
	int kkk = 0;\
	for (kkk = 0; kkk < (queue).size; kkk++) {\
		if(queue_get((queue),kkk)==elem) break;\
	}\
	kkk;})
#endif
