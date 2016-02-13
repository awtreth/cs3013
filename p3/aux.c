#include "aux.h"
#include <stdlib.h>

int uniform_rand(int low, int high) {
	return rand()%(abs(high-low)+1)+low;
}
