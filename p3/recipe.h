#ifndef RECIPE_H
#define RECIPE_H

//Station enumeration
typedef enum {PREP, OVEN, STOVE, SINK, N_STATIONS} station_t;

//Recipe Step
typedef struct {
	station_t station;
	unsigned int duration;
} recipe_step_t;



//Construct a Recipe Step
recipe_step_t recipe_step_init(station_t station, unsigned int time);

//Sequence of recipe steps. Behaves like a bag data type
typedef struct {
	int nsteps;
	recipe_step_t *steps;
} recipe_t;


//Initialize empty recipe_t
recipe_t recipe_init();

//Fixed size queue of Recipes
typedef struct {
	unsigned int size;
	unsigned int init;//position of the first element
	unsigned int end;//position of the last element
	unsigned int capacity;//max fixed capacity
	recipe_t *array;
}recipe_queue_t;

//Add step to the specified recipe (in order)
int recipe_add_step(recipe_t *recipe, recipe_step_t step);

//Free recipe content
void recipe_free(recipe_t *recipe);

//Initialize the queue with fixed capacity
recipe_queue_t recipe_queue_init(unsigned int capacity);

//Free queue content
void recipe_queue_free(recipe_queue_t* queue);

//Push at the begining of the recipe queue
void recipe_queue_push(recipe_queue_t* queue, recipe_t recipe);

//Pop the last element from the recipe queue
recipe_t recipe_queue_pop(recipe_queue_t* queue);

//Return the recipe in relative position
recipe_t recipe_queue_get(recipe_queue_t queue, unsigned int pos);

//Print recipe content
void print_recipe(recipe_t recipe);

//Print recipe queue content
void print_recipe_queue(recipe_queue_t queue);

void load_recipe(recipe_t * recipe, const char* filename);

#endif
