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
} order_sem_t;

int order_sem_init(order_sem_t *sem, unsigned int value, unsigned int max_threads);

int order_sem_wait(order_sem_t *sem);

int order_sem_post(order_sem_t *sem);

int order_sem_destroy(order_sem_t *sem);

int order_sem_trywait(order_sem_t *sem);

int order_sem_getvalue(order_sem_t *sem, int *sval);


#endif
