
#include "order_sem.h"

int order_sem_init(order_sem_t *sem, unsigned int value, unsigned int max_threads){
	
	//Initialize queue parameters
	sem->size = 0;
	sem->capacity = max_threads;
	sem->first = 0;
	sem->last = 0;
	sem->queue = malloc(sizeof(sem_t)*sem->capacity);
	
	int i;
	//Initialize queue semaphores
	for (i = 0; i < sem->capacity; i++)
		sem_init(&sem->queue[i], 0, 0);
	
	sem_init(&sem->queue_sem, 0, 1);
	return sem_init(&sem->main_sem, 0, value);
}

int order_sem_wait(order_sem_t *sem) {
	
	sem_wait(&sem->queue_sem);//queue access control
	if(sem_trywait(&sem->main_sem) < 0){//if it would block
		//Push into the queue
		sem->size++;
		int pos = sem->last++;
		sem->last= sem->last%sem->capacity;//correct last (circular implementation)
		sem_post(&sem->queue_sem);//queue access control
		return sem_wait(&sem->queue[pos]);//wait on the last semaphore in the queue
	}
	sem_post(&sem->queue_sem);//queue access control
	
	return 0;
}

int order_sem_post(order_sem_t *sem){
	
	sem_wait(&sem->queue_sem);//queue access control
	
	if(sem->size == 0) {//there are no blocked threads
		sem_post(&sem->main_sem);//simply post the main semaphore
	}else {
		sem->size--;//
		sem_post(&sem->queue[sem->first++]);//unblock the first of the queue
		sem->first= sem->first%sem->capacity;
	}
	
	sem_post(&sem->queue_sem);//queue access control
	
	return 0;
}

int order_sem_destroy(order_sem_t *sem) {
	
	int i;
	
	for (i = 0; i < sem->capacity; i++)
		sem_destroy(&sem->queue[i]);
	
	sem->size = 0;
	sem->capacity = 0;
	sem->first = 0;
	sem->last = 0;
	
	if(sem->queue!=NULL) free(sem->queue);
	
	sem_destroy(&sem->queue_sem);
	return sem_destroy(&sem->main_sem);
}

inline int order_sem_trywait(order_sem_t *sem) {
	return sem_trywait(&sem->main_sem);
}

int order_sem_getvalue(order_sem_t *sem, int *sval) {
	int ret = sem_getvalue(&sem->main_sem, sval);
	*sval = *sval - sem->size;
	return ret;
}

