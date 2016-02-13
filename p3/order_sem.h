#ifndef ORDER_SEM_H
#define ORDER_SEM_H


#include "queue.h"
#include <pthread.h>
#include <semaphore.h>

typedef struct {
	sem_t main_sem;
	sem_t queue_sem;
	
	unsigned int first;
	unsigned int last;
	unsigned int capacity;
	unsigned int size;
	
	sem_t *queue;
} order_sem;

int order_sem_init(order_sem *sem, unsigned int value, unsigned int max_threads);

int order_sem_wait(order_sem *sem);

int order_sem_post(order_sem *sem);

int order_sem_destroy(order_sem *sem);

int order_sem_trywait(order_sem *sem);

#endif
