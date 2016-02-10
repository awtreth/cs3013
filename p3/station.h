#ifndef STATION_H
#define STATION_H

#include "queue.h"

//Station enumeration
typedef enum {PREP, OVEN, STOVE, SINK, N_STATIONS} station_t;

define_queue(int);

typedef struct {
	int chef;
	station_t from;
} future_chef_t;

define_queue(future_chef_t);

typedef struct {
	int 				current_chef;
	queue_int 			chef_queue; 
	queue_future_chef_t future_chef_queue; 
} station_state_t;

void init_station_state(station_state_t *state, int n_chefs);

void free_station_state(station_state_t *state);

void init_station_array(station_state_t states[], int n_chefs);

void free_station_array(station_state_t states[]);

#endif
