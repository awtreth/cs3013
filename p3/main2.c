#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <mcheck.h>

#include "kitchen.h"
#include "order_sem.h"
#include "queue.h"

//PROBLEM PARAMETERS
#define N_RECIPES	5
#define MAX_ORDERS	30
#define N_CHEFS		3
#define MIN_ORDER_TIME 5
#define MAX_ORDER_TIME 15

#define TIME_UNIT 1 //in milliseconds 

//GLOBAL VARIABLES
//store the constant recipes
recipe_t recipes[N_RECIPES];//it will be constant along all the program

//queue of order_t (defined in kitchen.h)
queue_order_t order_queue;

//store the chef ids that has an assigned order
queue_int chef_queue;//the first element has the oldest order

//has information about the stations (include synchronization tools)
kitchen2_t kitchen;

//Store the intention of movement for each chef
intention_t intention[N_CHEFS];//automatically initialized with zeros because it's global

//MUTEXES AND CONDITION VARIABLES
int available_orders = 0;
int remaining_orders = MAX_ORDERS;
pthread_mutex_t available_orders_mtx;  //order_queue control acces mtx
pthread_cond_t available_orders_cv;
pthread_mutex_t remaining_orders_mtx; //mutual exclusion of chef_queue

pthread_mutex_t order_queue_mtx;  //order_queue control acces mtx
pthread_mutex_t chef_queue_mtx; //mutual exclusion of chef_queue
pthread_mutex_t intention_mtx[N_CHEFS]; //mutual exclusion of intention (for each chef)


//Auxiliar function (return a random number between low and high)
int uniform_rand(int low, int high) {
	return rand()%(abs(high-low)+1)+low;
}


/* Auxiliar function for check_dead_lock
 * 
 * It basically recursively checks if the there is another chef that is going to
 * move from the next station (from) to the target. If it's true with distinct chefs,
 * then there is a deadlock. (in a dead-lock, there is one link (move arrow) for each chef)
 * 
 * Sorry, this function is hard to explain without figures
 * 
 * @param target: same of check_dead_lock function
 * @param from:		next station in the path in analysis
 * @param remaining_chefs: store the chefs_id that was not analized yet
 */
int check_dead_lock_aux(station_id target, station_id from, queue_int remaining_chefs);

/*Check if the next movevement of a certain chef will generate a deadlock
 * @param chef_id: chef who wants to make the movement
 * @param target:	target station
 * @param next: next station after the target one
 * 
 * @return 1-TRUE; 0-FALSE
 */
int check_dead_lock(int chef_id, station_id target, station_id next, int step);


//OrderThread: Take care of order generation and queueing
void* order(void* arg);

//ChefThread: one for each thread
void* chef(void* arg);

//MAIN
int main (int argc, char **argv) {
	
	//mtrace();//just for debug
	
	int i = 0;//auxiliar variable
	
	//Define random seed for all functions that use "rand" function
	if(argc>1) srand(atoi(argv[1]));
	else srand(time(NULL));

	//Load Global variables
	load_recipes(recipes, N_RECIPES, "recipes.txt");
	queue_init(&order_queue, MAX_ORDERS);
	queue_init(&chef_queue, N_CHEFS);
	init_kitchen2(&kitchen);

	//Initialize global semaphores
	pthread_mutex_init(&available_orders_mtx, NULL);
	pthread_mutex_init(&remaining_orders_mtx, NULL);
	pthread_cond_init(&available_orders_cv, NULL);
	
	
	pthread_mutex_init(&order_queue_mtx, NULL);
	pthread_mutex_init(&chef_queue_mtx, NULL);
	for (i = 0; i < N_CHEFS; i++)
		pthread_mutex_init(&intention_mtx[i], NULL);
	
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
	free_kitchen2(&kitchen);
	for (i = 0; i < N_RECIPES; i++)
		recipe_free(&recipes[i]);
	
	//Destroy all semaphores
	pthread_mutex_destroy(&available_orders_mtx);
	pthread_mutex_destroy(&remaining_orders_mtx);
	
	pthread_cond_destroy(&available_orders_cv);
	
	pthread_mutex_destroy(&order_queue_mtx);
	pthread_mutex_destroy(&chef_queue_mtx);
	
	for (i = 0; i < N_CHEFS; i++)
		pthread_mutex_destroy(&intention_mtx[i]);//MUTEX
	
	
	return 0;
}

//OrderThread: Take care of order generation and queueing
void* order(void* arg) {
	
	order_t current_order;
	current_order.number = 0;
	int recipe_number = 0;
	
	//for MAX_ORDERS times
	for (;current_order.number < MAX_ORDERS; current_order.number++) {
		
		//Generate Order
		usleep(uniform_rand(MIN_ORDER_TIME*TIME_UNIT*1000,MAX_ORDER_TIME*TIME_UNIT*1000));    //wait for a random time
		
		recipe_number = uniform_rand(0, N_RECIPES-1);//pick one recipe randomly
		
		current_order.recipe_id = recipe_number;
		
		//Push the recipe to the order_queue
		pthread_mutex_lock(&order_queue_mtx);
		queue_push(&order_queue, current_order);
		pthread_mutex_unlock(&order_queue_mtx);
		
		//Notify that there are available orders
		pthread_mutex_lock(&available_orders_mtx);
		if(available_orders++ == 0)
			pthread_cond_signal(&available_orders_cv);
		pthread_mutex_unlock(&available_orders_mtx);
		
		printf("Order %d (Recipe %d) has arrived.\n", current_order.number, recipe_number);
	}/**/

	return NULL;
}

//ChefThread: one for each thread
void* chef(void* arg) {
	
	chef_t chef;//store information about the chef
	chef.id = (int)arg;
	
	printf("Initialized chef %d\n", chef.id);
	
	//auxiliar variables
	int i;
	int target_station, next_station;
	
	//while there are orders to be received
	while(1) {
		pthread_mutex_lock(&remaining_orders_mtx);
		if(remaining_orders == 0) {
			pthread_mutex_unlock(&remaining_orders_mtx);
			break;
		}
		remaining_orders--;
		pthread_mutex_unlock(&remaining_orders_mtx);
		
		pthread_mutex_lock(&available_orders_mtx);
		if(available_orders==0)
			pthread_cond_wait(&available_orders_cv, &available_orders_mtx);
		available_orders--;
		pthread_mutex_unlock(&available_orders_mtx);
		
		//Push the recipe to the order_queue
		pthread_mutex_lock(&order_queue_mtx);
		chef.order = queue_pop(&order_queue);
		pthread_mutex_unlock(&order_queue_mtx);
printf("Chef %d received order %d (recipe %d)\n", chef.id, chef.order.number, chef.order.recipe_id);
print_recipe(recipes[chef.order.recipe_id]);
		
		//update the movement intentions
		for(i = 1; i < recipes[chef.order.recipe_id].nsteps; i++) 
			add_intention(&intention[chef.id], recipes[chef.order.recipe_id].steps[i-1].station, recipes[chef.order.recipe_id].steps[i].station);
		
		//Put yourself at the end of the queue of the chefs with assigned orders
		pthread_mutex_lock(&chef_queue_mtx);
		queue_push(&chef_queue, chef.id);
		pthread_mutex_unlock(&chef_queue_mtx);
		
		//ALREADY SET, NOW IT WILL PREPARE FOR MOVES
		for (i = 1; i <= recipes[chef.order.recipe_id].nsteps; i++) {
			target_station = recipes[chef.order.recipe_id].steps[i-1].station;//the station I want to go in the current step
			
printf("Chef %d is waiting for station %d\n", chef.id, target_station);
			pthread_mutex_lock(&kitchen.sleep_mtx[target_station]);
			//if(kitchen.sleepers[target_station])
				//pthread_cond_wait(&kitchen.sleep_cv[target_station], &kitchen.sleep_mtx[target_station]);
			
			//order_sem_wait(&kitchen.station_sem[target_station]);//wait the station to be freed
//printf("Chef %d passed semaphore of station %d\n", chef.id, target_station);
		
			if(i < recipes[chef.order.recipe_id].nsteps) {//not last step
				next_station = recipes[chef.order.recipe_id].steps[i].station;//the station of the next step
			
				//Check if the next movevement of a certain chef will generate a deadlock
				if(check_dead_lock(chef.id, target_station, next_station, i-1)){//==TRUE
//printf("Chef %d identified deadlock when it tried to enter station %d\n", chef.id, target_station);
printf("Chef %d slept trying to enter station %d\n", chef.id, target_station);
					pthread_cond_wait(&kitchen.sleep_cv[target_station], &kitchen.sleep_mtx[target_station]);
					pthread_mutex_unlock(&kitchen.sleep_mtx[target_station]);
printf("Chef %d was woken up\n", chef.id);
					i--;//to repeat the loop on the same step
					continue;
				}//ELSE
			}
			//NOW, the chef has aquired the desired station
			
			if(i >= 2) {//not the first step
				//remove the intention from the old station to the new one
				pthread_mutex_lock(&intention_mtx[chef.id]);
				rem_intention(&intention[chef.id], recipes[chef.order.recipe_id].steps[i-2].station, target_station);
				pthread_mutex_unlock(&intention_mtx[chef.id]);
			}
printf("Chef %d is safe to enter to enter station %d\n", chef.id, target_station);
			
			//update kitchen
			
			int chef_station = chef.station;
			chef.station = target_station;//update chef station
			kitchen.chef[target_station] = chef;//update the new station
			
			if(i > 1) {//not the first step
				kitchen.chef[chef_station].id = -1;//the old station is now free
				print_kitchen2(kitchen);
				//free sleeping chefs
				//kitchen.sleepers[chef.station]=0;
				pthread_cond_broadcast(&kitchen.sleep_cv[chef_station]);
				pthread_mutex_unlock(&kitchen.sleep_mtx[chef_station]); //allow the next
			}else
				print_kitchen2(kitchen);
			
			//Use the station
			usleep(TIME_UNIT*recipes[chef.order.recipe_id].steps[i-1].duration*1000);
printf("Chef %d has just finished to use station %d\n", chef.id, target_station);
			
		}
		
		//special routines for the last step termination
		
		//remove itself from the chef_queue (it doesn't have orders anymore)
		pthread_mutex_lock(&chef_queue_mtx);
		int idx = queue_find(chef_queue, chef.id);
		queue_rem(&chef_queue, idx);
		pthread_mutex_unlock(&chef_queue_mtx);
		
		int chef_station = chef.station;
		chef.station = -1;//update chef station
		kitchen.chef[chef_station].id = -1;
		print_kitchen2(kitchen);
		
		//free sleeping chefs of the last station
		//kitchen.sleepers[chef.station]=0;
		pthread_cond_broadcast(&kitchen.sleep_cv[chef_station]);
		pthread_mutex_unlock(&kitchen.sleep_mtx[chef_station]); //allow the next
		
printf("Chef %d finished order %d\n", chef.id, chef.order.number);
	}
	
printf("Chef %d noticed that there is not more remaining orders. Then, he went back to his home\n", chef.id);/**/
	
	return NULL;
}


//Auxiliar function for check_dead_lock
int check_dead_lock_aux(station_id target, station_id from, queue_int remaining_chefs) {
	int i, j;
	
	for (i = 0; i < remaining_chefs.size; i++) {//for each chef with older orders
		for (j = 0; j < N_STATIONS; j++) {//for all station
			if(intention[queue_get(remaining_chefs, i)].link[from][j]){//if someone wants to go from the "from" station to somewhere
				if(j == target) {//if this "somewhere" station is the target
printf("found deadlock with chef %d, from %d to %d\n", queue_get(remaining_chefs, i), from, j);
					return 1;//it's a deadlock
				}else{
					//maybe there is a multi-level dead-lock
					//remove the chef in analysis (in a dead-lock, there is one link (move arrow) for each chef)
					queue_int new_queue;
					queue_init(&new_queue, N_CHEFS);
					queue_copy(&remaining_chefs, &new_queue);
					queue_rem(&new_queue, i);
					
					//check for deadlocks analising the next station
					if(check_dead_lock_aux(target, j, new_queue)){
						queue_free(&new_queue);
						return 1;//repass result
					}
					queue_free(&new_queue);
				}
			}
		}
	}
	//looked everwhere
	return 0;//there is no dead-lock risk
} 

//Check if the next movevement of a certain chef will generate a deadlock
int check_dead_lock(int chef_id, station_id target, station_id next, int step) {
	
printf("check_dead_lock chef %d\n", chef_id);
	
	int i = 0, j, ret = 0;
	
	queue_int remaining_chefs;//chefs with orders that are older than the caller chef order
	queue_init(&remaining_chefs, N_CHEFS);
	
	pthread_mutex_lock(&chef_queue_mtx);//to know about the running orders/chefs
	do{
		int chef = queue_get(chef_queue,i);//get the oldest
		i++;
		if(chef==chef_id) break;//we just want the chefs with older orders then chef_id's chef
		queue_push(&remaining_chefs, chef);
	}while(1);
	
	if(remaining_chefs.size==2 && step == 0) {
		for (i = 0; i < remaining_chefs.size; i++) {//for each chef with older orders
			for (j = 0; j < N_STATIONS; j++) {//for all station
				if(intention[queue_get(remaining_chefs, i)].link[j][target]){
					ret++;
					//break;
				}
			}
			//if(ret)break;
		}
	}//else {
	if(ret<2){
		//aquire access to the intentions
		for(i = 0; i < remaining_chefs.size; i++)
			pthread_mutex_lock(&intention_mtx[queue_get(remaining_chefs, i)]);
		
		//check deadlock
		ret = check_dead_lock_aux(target, next, remaining_chefs);
	
		//post intention semaphores
		for(i = 0; i < remaining_chefs.size; i++)
			pthread_mutex_unlock(&intention_mtx[queue_get(remaining_chefs, i)]);
	}else
		printf("Chef %d found special deadlock\n", chef_id);
	
	pthread_mutex_unlock(&chef_queue_mtx);
	queue_free(&remaining_chefs);
	
	return ret;
}
