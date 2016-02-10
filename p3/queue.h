#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <semaphore.h>

#define define_queue(TYPE)\
	typedef struct {\
		unsigned int size;\
		unsigned int init;\
		unsigned int end;\
		unsigned int capacity;\
		sem_t *sem;\
		TYPE *array;\
	} queue_##TYPE;

//Initialize the queue with fixed capacity
#define queue_init(queue, CAPACITY) \
		(queue)->size = 0;\
		(queue)->init = 1;\
		(queue)->end =  0;\
		(queue)->capacity = CAPACITY;\
		(queue)->sem = (sem_t*) malloc(sizeof(sem_t));\
		sem_init((queue)->sem, 0, 1);\
		(queue)->array = malloc(sizeof(typeof(*((queue)->array)))*CAPACITY);
	

//Free queue content
#define queue_free(queue) {\
	if((queue)->array != NULL) free((queue)->array);\
	if((queue)->sem != NULL){sem_destroy((queue)->sem);free((queue)->sem);}\
	(queue)->size = 0;\
	(queue)->init = 1;\
	(queue)->end =  0;\
	(queue)->capacity = 0;\
}

//Push at the begining of the recipe queue
#define queue_push(queue, value)({\
		int ret = 0;\
		sem_wait((queue)->sem);\
		if((queue)->size >= (queue)->capacity) {\
			sem_post((queue)->sem);\
			printf("Someone tried to push on a full queue\n");\
			ret = -1;\
		}else{\
			(queue)->init = ((queue)->init + (queue)->capacity - 1)%(queue)->capacity;\
			(queue)->array[(queue)->init] = value;\
			ret = ++(queue)->size;\
			sem_post((queue)->sem);\
		}\
		ret;\
	})
	
#define queue_is_empty(queue) ({\
	sem_wait((queue).sem);\
	int ret = (queue).size<=0;\
	sem_post((queue).sem);\
	ret;\
	})

//Push at the begining of the recipe queue
#define queue_pop(queue)({\
		sem_wait((queue)->sem);\
		if((queue)->size<=0){\
			printf("Someone tried to pop from an empty queue\n");\
		}\
		unsigned int old_end = (queue)->end;\
		(queue)->end = ((queue)->end + (queue)->capacity - 1)%(queue)->capacity;\
		--(queue)->size;\
		typeof(*((queue)->array)) ret = (queue)->array[old_end];\
		sem_post((queue)->sem);\
		ret;\
	})


#define queue_get(queue, pos) ({\
		sem_wait((queue).sem);\
		if(pos >= (queue).size || pos < 0) {printf("Someone tried to get a value of an invalid position in a queue\n");}\
		typeof(*((queue).array)) ret =(queue).array[((queue).end+queue.capacity-pos)%(queue).capacity];\
		sem_post((queue).sem);\
		ret;\
	})


#endif
