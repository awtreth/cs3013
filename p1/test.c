#include <stdio.h>
#include <stdlib.h>

int main() {
	
	int v = 1, i = 0;
	
	for(i = 0; i < 10000000; i++)
		v+=i;

	return 0;
}
