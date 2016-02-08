#include <stdio.h>
#include "aux.h"
#include "recipe.h"
#include <time.h>

#include <mcheck.h>// for memory leakage tracking
#include <stdlib.h>

int main() {

	
	
	mtrace();
	
	recipe_t recipe = recipe_init();
	recipe_t recipe2 = recipe_init();
	
	
	
	
	
	
	
	recipe_add_step(&recipe, recipe_step_init(PREP,1));
	recipe_add_step(&recipe, recipe_step_init(STOVE,4));
	recipe_add_step(&recipe, recipe_step_init(OVEN,5));

	recipe_add_step(&recipe2, recipe_step_init(OVEN,5));
	recipe_add_step(&recipe2, recipe_step_init(STOVE,4));
	recipe_add_step(&recipe2, recipe_step_init(PREP,1));
	
	recipe_queue_t queue = recipe_queue_init(10);
	
	recipe_queue_push(&queue, recipe);
	recipe_queue_push(&queue, recipe);
	recipe_queue_push(&queue, recipe2);
	recipe_queue_push(&queue, recipe2);
	
	print_recipe_queue(queue);
	
	recipe_queue_pop(&queue);
	
	print_recipe_queue(queue);
	
	recipe_queue_free(&queue);
	recipe_free(&recipe);
	
	return 0;
}
