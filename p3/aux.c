#include "aux.h"

//Station enumeration
typedef enum {PREP, OVEN, STOVE, SINK, N_STATIONS} station_t;

//Recipe Step
typedef struct {
	station_t station;
	unsigned int time;
} recipe_step_t;

//Construct a Recipe Step
recipe_step_t recipe_step_init(station_t station, unsigned int time) {
	recipe_step_t ret = {station,time};
	return ret;
}

//Sequence of recipe steps. Behaves like a bag data type
typedef struct {
	int nsteps;
	recipe_step_t *steps;
} recipe_t;

//Initialize empty recipe_t
recipe_t recipe_init() {
	recipe_t recipe = {0,NULL};
	return recipe;
}

//Add step to the specified recipe (in order)
int add_recipe_step(recipe_t *recipe, recipe_step_t step) {
	recipe->steps = (recipe_step_t*) realloc(recipe->steps, ++recipe->nsteps);
	recipe->steps[recipe->nsteps-1] = step;
	return 0;
}

//Free recipe content
void free_recipe(recipe_t *recipe) {
	return free(recipe->steps);
}

//Fixed size queue of Recipes
typedef struct {
	unsigned int size;
	unsigned int init;//position of the first element
	unsigned int end;//position of the last element
	unsigned int capacity;//max fixed capacity
	recipe_t *array;
}recipe_queue_t;

//Initialize the queue with fixed capacity
recipe_queue_t recipe_queue_init(unsigned int capacity) {
	recipe_queue_t queue;
	queue.size = 0;
	queue.init = 1;
	queue.end =  0;
	queue.capacity = capacity;
	queue.array = (recipe_t*) malloc(sizeof(recipe_t)*queue.capacity);
	return queue;
}

//Free queue content
void recipe_queue_destroy(recipe_queue_t* queue) {
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
