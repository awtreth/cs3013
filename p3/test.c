#include <stdio.h>
#include "aux.h"
#include "kitchen.h"
#include <time.h>

#include <mcheck.h>// for memory leakage tracking
#include <stdlib.h>

#include "queue.h"

#include "order_sem.h"
#include <pthread.h>
#include <unistd.h>

#define N_THREADS 10

order_sem_t sem;


void* fnc(void* arg) {
	int n = (int) arg;
	
	order_sem_wait(&sem);
	printf("thread %d has passed\n", n);
	
	return NULL;
}



int main() {

	int value;
	
	queue_int q1, q2;
	
	queue_init(&q1, 5);
	queue_init(&q2, 5);
	
	queue_push(&q1, 10);
	queue_push(&q1, 3);
	queue_push(&q1, 5);
	queue_push(&q1, 6);
	queue_push(&q1, 1);
	
	//printf("%d\n", queue_get(q1,2));
	value = queue_find(q1,3);
	printf("%d\n", value);
	printf("%d\n", queue_rem(&q1, 0));
	printf("%d\n", queue_find(q1,5));
	
	
	
	
	/*pthread_t thread[N_THREADS];
	
	order_sem_init(&sem, 0, N_THREADS);
	
	order_sem_getvalue(&sem, &value);
	printf("initial value = %d\n", value);
	
	for (i = 0; i < N_THREADS; i++)
		pthread_create(&thread[i], NULL, fnc, (void*)i);
	
	sleep(1);
	
	for (i = 0; i < N_THREADS; i++){
		order_sem_post(&sem);
		order_sem_getvalue(&sem, &value);
		printf("value = %d\n", value);
		sleep(1);
	}
	
	
	for (i = 0; i < N_THREADS; i++)
		pthread_join(thread[i], NULL);
	
	order_sem_destroy(&sem);*/

	return 0;
}
