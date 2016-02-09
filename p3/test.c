#include <stdio.h>
#include "aux.h"
#include "recipe.h"
#include <time.h>

#include <mcheck.h>// for memory leakage tracking
#include <stdlib.h>

int main() {

	
	recipe_t recipes[5];
	
	load_recipes(recipes, 5, "recipes.txt");
	
	int i = 0;
	
	for (i=0; i < 5; i++) {
		print_recipe(recipes[i]);	
	}
	
	
	return 0;
}
