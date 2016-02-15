#ifndef ORDER_SEM_H
#define ORDER_SEM_H


#include "queue.h"
#include <pthread.h>
#include <semaphore.h>


/* Same functions of sem_t, however, blocked threads are posted in the order they arrived
*/
typedef struct {
	sem_t main_sem;
	sem_t queue_sem;//mutual exclusion of the queue
	
	//queue parameters
	unsigned int first;//index for the first element in the queue
	unsigned int last;//index for the last element in the queue
	unsigned int capacity;//max number of threads
	unsigned int size;//it will be the number of blocked threads
	
	sem_t *queue;//store blocked threads
} order_sem_t;

/* value: initial value for the semaphore
 * max_threads: maximum threads that can block on this semaphore
 */
int order_sem_init(order_sem_t *sem, unsigned int value, unsigned int max_threads);

//see sem_* linux manual
int order_sem_wait(order_sem_t *sem);

int order_sem_post(order_sem_t *sem);

int order_sem_destroy(order_sem_t *sem);

int order_sem_trywait(order_sem_t *sem);

int order_sem_getvalue(order_sem_t *sem, int *sval);


#endif
