#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#include "recipe.h"
#include "aux.h"
#include "station.h"

#define N_RECIPES	5
#define MAX_ORDERS	5
#define N_CHEFS		3

//GLOBAL VARIABLES
recipe_t recipes[N_RECIPES];

queue_recipe_t order_queue;

station_state_t state[N_STATIONS];

//SEMAPHORES
sem_t available_orders; //increased by order and decreased by chefs
sem_t remaining_orders; //decreased by chefs
//sem_t order_queue_sem;  //order_queue control acces semaphore

//Take care of order generation and queueing
void* order(void* arg) {
	
	int order = 0;
	
	for (;order < MAX_ORDERS; order++) {
		
		//Generate Order
		usleep(uniform_rand(1000000,5000000));    //wait for a random time
		int recipe = uniform_rand(0, N_RECIPES-1);//pick one recipe randomly
		
		//Push the recipe to the order_queue
		//sem_wait(&order_queue_sem);
		queue_push(&order_queue, recipes[recipe]);
		//sem_post(&order_queue_sem);
		
		//Notify that there are available orders
		sem_post(&available_orders);
		
		printf("Order %d (Recipe %d) has arrived.\n", order+1, recipe+1);
		//print_recipe_queue(order_queue);
	}/**/

	return NULL;
}

//Thread for each chef
void* chef(void* arg) {
	
	int chef_number = (int)arg;
	printf("Initialized chef %d\n", chef_number);
	
	recipe_t current_recipe;
	
	while(sem_trywait(&remaining_orders)!=-1) {
		sem_wait(&available_orders);
		//Push the recipe to the order_queue
		//sem_wait(&order_queue_sem);
		current_recipe = queue_pop(&order_queue);
		//sem_post(&order_queue_sem);
		printf("Chef %d received recipe\n", chef_number);
		usleep(current_recipe.steps[0].duration*1000000);
		printf("Chef %d finished order\n", chef_number);
	}
	
	printf("Chef %d noticed that there is not more remaining orders. Then, he went back to his home\n", chef_number);/**/
	
	return NULL;
}

int main (int argc, char **argv) {
	
	int i = 0;
	
	//Define random seed for all functions that use "rand" function
	srand(10);

	//Load Global variables
	load_recipes(recipes, N_RECIPES, "recipes.txt");
	
	queue_init(&order_queue, MAX_ORDERS);
	
	init_station_array(state, N_CHEFS);
	
	
	//Initialize global semaphores
	sem_init(&available_orders, 0, 0);
	sem_init(&remaining_orders, 0, MAX_ORDERS);
	//sem_init(&order_queue_sem, 0, 1);
	
	//Create threads
	pthread_t order_thread;
	pthread_t chef_thread[N_CHEFS];
	
	pthread_create(&order_thread, NULL, order, NULL);
	
	for (i = 0; i < N_CHEFS; i++) {
		pthread_create(&chef_thread[i], NULL, chef, (void*)i);
	}
	
	//Wait for all the threads' termination
	pthread_join(order_thread,NULL);
	for (i = 0; i < N_CHEFS; i++)
		pthread_join(chef_thread[i],NULL);
	
	//Free all dinamically allocated memory
	queue_free(&order_queue);
	for (i = 0; i < N_RECIPES; i++) {
		recipe_free(&recipes[i]);
	}
	sem_destroy(&available_orders);
	sem_destroy(&remaining_orders);
	//sem_destroy(&order_queue_sem);
	
	free_station_array(state);
	
	return 0;
}
