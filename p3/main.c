#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#include "kitchen.h"
#include "aux.h"

#define N_RECIPES	5
#define MAX_ORDERS	5
#define N_CHEFS		3

#define TIME_UNIT 1000 //in milliseconds 

//GLOBAL VARIABLES
recipe_t recipes[N_RECIPES];

define_queue(order_t);//queue_order_t
queue_order_t order_queue;

kitchen_t kitchen;

intention_t intention[N_CHEFS];//automatically initialized with zeros because it's global

//SEMAPHORES
sem_t available_orders; //increased by order and decreased by chefs
sem_t remaining_orders; //decreased by chefs
sem_t order_queue_sem;  //order_queue control acces semaphore


//Take care of order generation and queueing
void* order(void* arg) {
	
	order_t current_order;
	current_order.number = 0;
	int recipe_number = 0;
	
	for (;current_order.number < MAX_ORDERS; current_order.number++) {
		
		//Generate Order
		usleep(uniform_rand(1000*TIME_UNIT,5000*TIME_UNIT));    //wait for a random time
		
		recipe_number = uniform_rand(0, N_RECIPES-1);//pick one recipe randomly
		
		current_order.recipe = recipes[recipe_number];
		
		//Push the recipe to the order_queue
		sem_wait(&order_queue_sem);
		queue_push(&order_queue, current_order);
		sem_post(&order_queue_sem);
		
		//Notify that there are available orders
		sem_post(&available_orders);
		
		printf("Order %d (Recipe %d) has arrived.\n", current_order.number, recipe_number);
	}/**/

	return NULL;
}

//Thread for each chef
void* chef(void* arg) {
	
	chef_t chef;
	chef.id = (int)arg;
	
	printf("Initialized chef %d\n", chef.id);
	
	while(sem_trywait(&remaining_orders)!=-1) {
		sem_wait(&available_orders);
		//Push the recipe to the order_queue
		sem_wait(&order_queue_sem);
		chef.current_order = queue_pop(&order_queue);
		sem_post(&order_queue_sem);
		printf("Chef %d received order %d (recipe %d)\n", chef.id, chef.current_order.number, chef.current_order.recipe.id);
		
		usleep(chef.current_order.recipe.steps[0].duration*1000*TIME_UNIT);
		printf("Chef %d finished order\n", chef.id);
	}
	
	printf("Chef %d noticed that there is not more remaining orders. Then, he went back to his home\n", chef.id);/**/
	
	return NULL;
}

int main (int argc, char **argv) {
	
	int i = 0;
	
	//Define random seed for all functions that use "rand" function
	srand(10);

	//Load Global variables
	load_recipes(recipes, N_RECIPES, "recipes.txt");
	
	queue_init(&order_queue, MAX_ORDERS);

	//Initialize global semaphores
	sem_init(&available_orders, 0, 0);
	sem_init(&remaining_orders, 0, MAX_ORDERS);
	sem_init(&order_queue_sem, 0, 1);
	
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
	sem_destroy(&order_queue_sem);
	
	return 0;
}
