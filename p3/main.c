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

sem_t intention_sem[N_CHEFS];


queue_int chef_queue;

//SEMAPHORES
sem_t available_orders; //increased by order and decreased by chefs
sem_t remaining_orders; //decreased by chefs
sem_t order_queue_sem;  //order_queue control acces semaphore
sem_t chef_queue_sem;

int check_dead_lock_aux(station_id target, station_id from, queue_int remaining_chefs) {
	int i, j;
	
	printf("target: %d; from = %d;\n", target, from);
	
	for (i = 0; i < remaining_chefs.size; i++) {
		for (j = 0; j < N_STATIONS; j++) {
			if(intention[queue_get(remaining_chefs, i)].link[from][j]){
				if(j == target) {
					printf("found deadlock with chef %d, from %d to %d\n", i, from, j);
					return 1;
				}else{
					queue_int new_queue;
					queue_init(&new_queue, N_CHEFS);
					queue_copy(&remaining_chefs, &new_queue);
					queue_rem(&new_queue, i);
					if(check_dead_lock_aux(target, j, new_queue)){
						queue_free(&new_queue);
						return 1;
					}
					queue_free(&new_queue);
				}
			}
		}
	}
	return 0;
} 

int check_dead_lock(int chef_id, station_id target, station_id next) {
	
	printf("check_dead_lock chef %d\n", chef_id);
	
	int i = 0, ret;
	
	queue_int remaining_chefs;
	queue_init(&remaining_chefs, N_CHEFS);
	
	sem_wait(&chef_queue_sem);
	
	do{
		int chef = queue_get(chef_queue,i);
		i++;
		if(chef==chef_id) break;
		queue_push(&remaining_chefs, chef);
	}while(1);
	
	sem_post(&chef_queue_sem);
	
	for(i = 0; i < remaining_chefs.size; i++){
		sem_wait(&intention_sem[queue_get(remaining_chefs, i)]);
		printf("%d ", queue_get(remaining_chefs, i));
	}printf("\n");
	
	ret = check_dead_lock_aux(target, next, remaining_chefs);
	
	for(i = 0; i < remaining_chefs.size; i++){
		sem_post(&intention_sem[queue_get(remaining_chefs, i)]);
	}
	queue_free(&remaining_chefs);
	
	return ret;
}


//Take care of order generation and queueing
void* order(void* arg) {
	
	order_t current_order;
	current_order.number = 0;
	int recipe_number = 0;
	
	for (;current_order.number < MAX_ORDERS; current_order.number++) {
		
		//Generate Order
		usleep(uniform_rand(100*TIME_UNIT,500*TIME_UNIT));    //wait for a random time
		
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
	
	int i, j;
	int value, target_station, next_station;
	
	while(sem_trywait(&remaining_orders)!=-1) {
		sem_wait(&available_orders);
		//Push the recipe to the order_queue
		sem_wait(&order_queue_sem);
		chef.order = queue_pop(&order_queue);
		sem_post(&order_queue_sem);
		printf("Chef %d received order %d (recipe %d)\n", chef.id, chef.order.number, chef.order.recipe.id);
		print_recipe(chef.order.recipe);
		
		for(i = 1; i < chef.order.recipe.nsteps; i++) 
			add_intention(&intention[chef.id], chef.order.recipe.steps[i-1].station, chef.order.recipe.steps[i].station);
		
		sem_wait(&chef_queue_sem);
		queue_push(&chef_queue, chef.id);
		printf("Chef %d pushed on chef_queue\n", chef.id);
		for (i = 0; i < chef_queue.size; i++)
			printf("%d ", queue_get(chef_queue, i));
		printf("\n");
		sem_post(&chef_queue_sem);
		
		
		
		//ALREADY SET, NOW IT WILL PREPARE FOR MOVES
		for (i = 1; i <= chef.order.recipe.nsteps; i++) {
			target_station = chef.order.recipe.steps[i-1].station;
			
			printf("Chef %d is waiting for station %d\n", chef.id, target_station);
			order_sem_wait(&kitchen.station_sem[target_station]);//wait the station be free
			//printf("Chef %d passed semaphore of station %d\n", chef.id, target_station);
		
			if(i < chef.order.recipe.nsteps) {//not last step
				next_station = chef.order.recipe.steps[i].station;
			
				if(check_dead_lock(chef.id, target_station, next_station)){//==TRUE
					printf("Chef %d identified deadlock when it tried to enter station %d\n", chef.id, target_station);
					
					printf("Chef %d slept trying to enter station %d\n", chef.id, target_station);
					order_sem_post(&kitchen.station_sem[target_station]); //allow the next
					order_sem_wait(&kitchen.sleep_sem[target_station]);
					printf("Chef %d was awaken\n", chef.id);
					i--;
					continue;
				}//ELSE
			}
			if(i >= 2) {
				sem_wait(&intention_sem[chef.id]);
				rem_intention(&intention[chef.id], chef.order.recipe.steps[i-2].station, target_station);
				sem_post(&intention_sem[chef.id]);
			}
			printf("Chef %d is safe to enter to enter station %d\n", chef.id, target_station);
			
			
			kitchen.chef[chef.station].id = -1;
			kitchen.chef[target_station] = chef;
			
			if(i > 1) {//not the first step
				//free sleeping chefs
				for(j = 0; j < kitchen.sleep_sem[chef.station].size; j++)
					order_sem_post(&kitchen.sleep_sem[chef.station]);
				order_sem_post(&kitchen.station_sem[chef.station]); //allow the next
			}
			chef.station = target_station;
			
			//printf("Chef %d has just started to use station %d for %d time units\n", chef.id, target_station, chef.order.recipe.steps[i-1].duration);
			usleep(TIME_UNIT*chef.order.recipe.steps[i-1].duration*100);
			printf("Chef %d has just finished to use station %d\n", chef.id, target_station);
			
		}
		sem_wait(&chef_queue_sem);
		int idx = queue_find(chef_queue, chef.id);
		queue_rem(&chef_queue, idx);
		sem_post(&chef_queue_sem);
		
		for(j = 0; j < kitchen.sleep_sem[chef.station].size; j++)
			order_sem_post(&kitchen.sleep_sem[chef.station]);
		order_sem_post(&kitchen.station_sem[chef.station]); //allow the next
		
		printf("Chef %d finished order\n", chef.id);
	}
	
	printf("Chef %d noticed that there is not more remaining orders. Then, he went back to his home\n", chef.id);/**/
	
	return NULL;
}

int main (int argc, char **argv) {
	
	int i = 0;
	
	//Define random seed for all functions that use "rand" function
	srand(9);

	//Load Global variables
	load_recipes(recipes, N_RECIPES, "recipes.txt");
	
	queue_init(&order_queue, MAX_ORDERS);
	queue_init(&chef_queue, N_CHEFS);

	init_kitchen(&kitchen);

	//Initialize global semaphores
	sem_init(&available_orders, 0, 0);
	sem_init(&remaining_orders, 0, MAX_ORDERS);
	sem_init(&order_queue_sem, 0, 1);
	sem_init(&chef_queue_sem, 0, 1);
	for (i = 0; i < N_CHEFS; i++)
		sem_init(&intention_sem[i],0,1);//MUTEX
	
	
	//Create threads
	pthread_t order_thread;
	pthread_t chef_thread[N_CHEFS];
	
	pthread_create(&order_thread, NULL, order, NULL);
	
	for (i = 0; i < N_CHEFS; i++) 
		pthread_create(&chef_thread[i], NULL, chef, (void*)i);
	
	
	//Wait for all the threads' termination
	pthread_join(order_thread,NULL);
	for (i = 0; i < N_CHEFS; i++)
		pthread_join(chef_thread[i],NULL);
	
	//Free all dinamically allocated memory
	queue_free(&order_queue);
	queue_free(&chef_queue);
	
	for (i = 0; i < N_RECIPES; i++) {
		recipe_free(&recipes[i]);
	}
	sem_destroy(&available_orders);
	sem_destroy(&remaining_orders);
	sem_destroy(&order_queue_sem);
	sem_destroy(&chef_queue_sem);
	for (i = 0; i < N_CHEFS; i++)
		sem_destroy(&intention_sem[i]);//MUTEX
	
	free_kitchen(&kitchen);
	
	return 0;
}
