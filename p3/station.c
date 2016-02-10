#include "station.h"
#include "queue.h"

void init_station_state(station_state_t *state, int n_chefs){
	queue_init(&state->chef_queue, n_chefs);
	queue_init(&state->future_chef_queue, n_chefs);
	state->current_chef = -1;
}

void free_station_state(station_state_t *state){
	queue_free(&state->chef_queue);
	queue_free(&state->future_chef_queue);
	state->current_chef = -1;
}

void init_station_array(station_state_t states[], int n_chefs) {
	int i = 0;
	for(; i < sizeof(states)/sizeof(station_state_t); i++) {
		init_station_state(&states[i], n_chefs);
	}
}

void free_station_array(station_state_t states[]) {
	int i = 0;
	for(; i < sizeof(states)/sizeof(station_state_t); i++) {
		free_station_state(&states[i]);
	}
}
