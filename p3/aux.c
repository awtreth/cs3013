#include "aux.h"
#include <stdlib.h>
#include <stdio.h>

int uniform_rand(int low, int high) {
	return rand()%(abs(high-low)+1)+low;
}
