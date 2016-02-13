#include <stdio.h>
#include "aux.h"
#include "kitchen.h"
#include <time.h>

#include <mcheck.h>// for memory leakage tracking
#include <stdlib.h>

#include "queue.h"


int main() {

	
	queue_int q;
	queue_init(&q,3);
	
	queue_push(&q, 1);
	queue_push(&q, 3);
	queue_push(&q, 4);
	
	printf("%d\n",queue_pop(&q));
	
	int i = 0;
	
	for(i=0; i < q.size; i++) {
		printf("%d: %d\n", i, queue_get(q,i));
	}
	
	queue_free(&q);
	//queue_int q;
	
	//queue_init(&q, 10, int);
	
	
	return 0;
}
