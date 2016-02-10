#ifndef RECIPE_H
#define RECIPE_H

#include "queue.h"

#include "station.h"

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


define_queue(recipe_t);

//Initialize empty recipe_t
recipe_t recipe_init();

//Add step to the specified recipe (in order)
int recipe_add_step(recipe_t *recipe, recipe_step_t step);

//Free recipe content
void recipe_free(recipe_t *recipe);

//Print recipe content
void print_recipe(recipe_t recipe);

//Print recipe queue content
void print_recipe_queue(queue_recipe_t queue);

void load_recipes(recipe_t recipes[], unsigned int size, const char* filename);

#endif
