#include "recipe.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Construct a Recipe Step
recipe_step_t recipe_step_init(station_t station, unsigned int time) {
	recipe_step_t ret = {station,time};
	return ret;
}

//Initialize empty recipe_t
recipe_t recipe_init() {
	recipe_t recipe = {0,(recipe_step_t*)NULL};
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


//Initialize the queue with fixed capacity
recipe_queue_t recipe_queue_init(unsigned int capacity) {
	recipe_queue_t queue;
	
	queue.size = 0;
	queue.init = 1;
	queue.end =  0;
	queue.capacity = capacity;
	queue.array = (recipe_t*) malloc(sizeof(recipe_t)*capacity);
	
	return queue;
}


//Free queue content
void recipe_queue_free(recipe_queue_t* queue) {
	if(queue->array != NULL) free(queue->array);
	queue->size = 0;
	queue->init = 1;
	queue->end =  0;
	queue->capacity = 0;
}

//Push at the begining of the recipe queue
void recipe_queue_push(recipe_queue_t* queue, recipe_t recipe) {
	queue->init = (queue->init + queue->capacity - 1)%queue->capacity;
	queue->array[queue->init] = recipe;
	queue->size++;
}

//Pop the last element from the recipe queue
recipe_t recipe_queue_pop(recipe_queue_t* queue) {
	unsigned int old_end = queue->end;
	queue->end = (queue->end + queue->capacity - 1)%queue->capacity;
	queue->size--;
	return queue->array[old_end];
} 

recipe_t recipe_queue_get(recipe_queue_t queue, unsigned int pos) {
	return queue.array[(queue.init+pos)%queue.capacity];
}


void print_recipe(recipe_t recipe) {
	int i = 0;
	for(; i < recipe.nsteps; i++) {
		printf("(%d, %d) ", recipe.steps[i].station, recipe.steps[i].duration);
	}
	printf("\n");
}

void print_recipe_queue(recipe_queue_t queue) {
	int i = 0;
	for(; i < queue.size; i++) {
		printf("Recipe %d: ", i);
		print_recipe(recipe_queue_get(queue,i));
	}
}



station_t parse_station(char *name) {
	if(strcmp("prep", name)==0) return PREP;
	else if(strcmp("stove", name)==0) return STOVE;
	else if(strcmp("oven", name)==0) return OVEN;
	else if(strcmp("sink", name)==0) return SINK;
	else printf("invalid input\n");

	return -1;
}

void load_recipe(recipe_t * recipe, const char* filename) {

	FILE *fd = fopen(filename, "r");
	char str[20];

	while(fgets(str, 20, fd)!=NULL) {//not EOF
		if(str[0]=='-')
			continue;
		station_t station = parse_station(strtok (str," "));
		unsigned int duration = atoi(strtok (NULL,"\n"));
		recipe_add_step(recipe, recipe_step_init(station,duration));
	}

	fclose(fd);

}
