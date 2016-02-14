#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <string.h>

#define define_queue(TYPE)\
	typedef struct {\
		unsigned int size;\
		unsigned int init;\
		unsigned int end;\
		unsigned int capacity;\
		TYPE *array;\
	} queue_##TYPE;

define_queue(int);//Default int queue

//Initialize the queue with fixed capacity
#define queue_init(queue, CAPACITY) \
		(queue)->size = 0;\
		(queue)->init = 1;\
		(queue)->end =  0;\
		(queue)->capacity = CAPACITY;\
		(queue)->array = malloc(sizeof(typeof(*((queue)->array)))*CAPACITY);

//Free queue content
#define queue_free(queue) {\
	if((queue)->array != NULL) free((queue)->array);\
	(queue)->size = 0;\
	(queue)->init = 1;\
	(queue)->end =  0;\
	(queue)->capacity = 0;\
}

//Push at the begining of the recipe queue
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
	
#define queue_is_empty(queue) ({\
	int ret = (queue).size<=0;\
	ret;\
	})

//Push at the begining of the recipe queue
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


#define queue_get(queue, pos) ({\
		if(pos >= (queue).size || pos < 0) {printf("Someone tried to get a value of an invalid position in a queue\n");exit(-1);}\
		typeof(*((queue).array)) ret =(queue).array[((queue).end+(queue).capacity-pos)%(queue).capacity];\
		ret;\
	})


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
		
		
#define queue_copy(from, to) {\
	if((from)->capacity != (to)->capacity) {printf("In queue_copy: queues' capacities don't match\n");}\
	else{\
	(to)->size = (from)->size;\
	(to)->init = (from)->init;\
	(to)->end = (from)->end;\
	memcpy((to)->array, (from)->array, (from)->capacity*sizeof(typeof(*((from)->array))));}}


#define queue_find(queue, elem) ({\
	int kkk = 0;\
	for (kkk = 0; kkk < (queue).size; kkk++) {\
		if(queue_get((queue),kkk)==elem) break;\
	}\
	kkk;})
#endif
