#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

recipe_t recipes[5];


void* order(void* arg) {
	
	
	
	msleep(uniform_rand(1000,5000));
	//
	
	
	
	
	return NULL;
}


int main (int argc, char **argv) {
	
	srand(10);
	
	
	
	
	
	
	
	pthread_t order_thread;
	
	pthread_create(&order_thread, NULL, order, NULL);

	pthread_join(order_thread,NULL);
	
	return 0;
}
