#include "kitchen.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "order_sem.h"

//Construct a Recipe Step
recipe_step_t recipe_step_init(station_id station, unsigned int time) {
	recipe_step_t ret = {station,time};
	return ret;
}

//Initialize empty recipe_t
recipe_t recipe_init() {
	recipe_t recipe = {0,0,(recipe_step_t*)NULL};
	return recipe;
}

//Initialize empty recipe_t
recipe_t recipe_init2(int id) {
	recipe_t recipe = {id,0,(recipe_step_t*)NULL};
	return recipe;
}

//Add step to the specified recipe (in order)
int recipe_add_step(recipe_t *recipe, recipe_step_t step) {
	recipe->steps = (recipe_step_t*) realloc(recipe->steps, sizeof(recipe_step_t)*++recipe->nsteps);
	recipe->steps[recipe->nsteps-1] = step;
	return 0;
}

//Free recipe content
void recipe_free(recipe_t *recipe) {
	return free(recipe->steps);
}


void print_recipe(recipe_t recipe) {
	int i = 0;
	printf("Recipe %d: ", recipe.id);
	for(; i < recipe.nsteps; i++) {
		printf("(%d, %d) ", recipe.steps[i].station, recipe.steps[i].duration);
	}
	printf("\n");
}

station_id parse_station(char *name) {
	if(strcmp("prep", name)==0) return PREP;
	else if(strcmp("stove", name)==0) return STOVE;
	else if(strcmp("oven", name)==0) return OVEN;
	else if(strcmp("sink", name)==0) return SINK;
	else printf("invalid input\n");

	return -1;
}

int load_recipes(recipe_t recipes[], unsigned int size, const char* filename) {

	FILE *fd = fopen(filename, "r");
	char station_str[10], duration_str[10];

	int i = 0;

	while(i <= size && fscanf(fd,"%s %s",station_str, duration_str)!=EOF) {//not EOF
		if(station_str[0]=='-'){
			recipes[i] = recipe_init2(i);
			i++;
			continue;
		}
		station_id station = parse_station(station_str);
		unsigned int duration = atoi(duration_str);
		recipe_add_step(&recipes[i-1], recipe_step_init(station,duration));
	}

	fclose(fd);

	return i;
}

void add_intention(intention_t* intention, station_id from, station_id to) {
	intention->link[from][to]++;
}

void rem_intention(intention_t* intention, station_id from, station_id to) {
	intention->link[from][to]--;
}

void print_intention(intention_t intent, int chef) {
	int i, j;
	printf("Intention of %d:\n", chef);
	for(i = 0; i < N_STATIONS; i++) {
		for (j = 0; j < N_STATIONS; j++) {
			printf("%d ", intent.link[i][j]);
		}
		printf("\n");
	}
}

void init_kitchen(kitchen_t *kitchen) {
	int i;
	
	for (i = 0; i < N_STATIONS; i++) {
		order_sem_init(&kitchen->station_sem[i], 1, N_STATIONS);
		order_sem_init(&kitchen->sleep_sem[i], 0, 3);
	}
}

void free_kitchen(kitchen_t *kitchen) {
	int i;
	
	for (i = 0; i < N_STATIONS; i++) {
		order_sem_destroy(&kitchen->station_sem[i]);
		order_sem_destroy(&kitchen->sleep_sem[i]);
	}
}

void print_kitchen(kitchen_t kitchen){
	int i;
	printf("KITCHEN\n");
	for (i = 0; i < N_STATIONS; i++) {
		printf("Station %d; chef %d\n", i, kitchen.chef[i].id);
	}
}



void init_kitchen2(kitchen2_t *kitchen) {
	int i;
	
	for (i = 0; i < N_STATIONS; i++) {
		//pthread_mutex_init(&kitchen->station_mtx[i], NULL);
		pthread_mutex_init(&kitchen->sleep_mtx[i], NULL);
		pthread_cond_init(&kitchen->sleep_cv[i], NULL);
	}
}

void free_kitchen2(kitchen2_t *kitchen) {
	int i;
	
	for (i = 0; i < N_STATIONS; i++) {
		//pthread_mutex_destroy(&kitchen->station_mtx[i]);
		pthread_mutex_destroy(&kitchen->sleep_mtx[i]);
		pthread_cond_destroy(&kitchen->sleep_cv[i]);
	}
}
