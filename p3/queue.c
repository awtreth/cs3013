#include "queue.h"

#include <stdlib.h>


//Initialize the queue with fixed capacity
void int_queue_init(int_queue_t* queue, unsigned int capacity) {
	queue->size = 0;
	queue->init = 1;
	queue->end =  0;
	queue->capacity = capacity;
	queue->array = (int*) malloc(sizeof(int)*capacity);
}

//Free queue content
void int_queue_free(int_queue_t* queue) {
	if(queue->array != NULL) free(queue->array);
	queue->size = 0;
	queue->init = 1;
	queue->end =  0;
	queue->capacity = 0;
}

//Push at the begining of the recipe queue
int int_queue_push(int_queue_t* queue, int number) {
	
	if(queue->size >= queue->capacity) {
		printf("Someone tried to push on a full queue\n");
		return -1;
	}
	
	queue->init = (queue->init + queue->capacity - 1)%queue->capacity;
	queue->array[queue->init] = number;
	queue->size++;
	return size;
}

//Pop the last element from the recipe queue
int_queue_t int_queue_pop(int_queue_t* queue) {
	if(queue->size <= 0) {
		printf("Someone tried to pop from an empty queue\n");
		return -1;
	}
	unsigned int old_end = queue->end;
	queue->end = (queue->end + queue->capacity - 1)%queue->capacity;
	queue->size--;
	return queue->array[old_end];
}

//Return the recipe in relative position
int_queue_t int_queue_get(int_queue_t queue, unsigned int pos);
