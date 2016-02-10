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


void print_recipe(recipe_t recipe) {
	int i = 0;
	for(; i < recipe.nsteps; i++) {
		printf("(%d, %d) ", recipe.steps[i].station, recipe.steps[i].duration);
	}
	printf("\n");
}

void print_recipe_queue(queue_recipe_t queue) {
	int i = 0;
	for(; i < queue.size; i++) {
		printf("Recipe %d: ", i);
		print_recipe(queue_get(queue,i));
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

void load_recipes(recipe_t recipes[], unsigned int size, const char* filename) {

	FILE *fd = fopen(filename, "r");
	char station_str[10], duration_str[10];

	int i = 0;

	while(i <= size && fscanf(fd,"%s %s",station_str, duration_str)!=EOF) {//not EOF
		if(station_str[0]=='-'){
			recipes[i++] = recipe_init();
			continue;
		}
		station_t station = parse_station(station_str);
		unsigned int duration = atoi(duration_str);
		recipe_add_step(&recipes[i-1], recipe_step_init(station,duration));
	}

	fclose(fd);

}
