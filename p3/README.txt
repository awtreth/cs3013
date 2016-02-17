Author:				Mateus Amarante Araujo
Project ID:			Project 3
CS Class:			CS 3013 (Operating Systems) - WPI
Programming Language:		C
How to build the program:	make [all]

Files:
	queue_sem.h: a set of macros that implements general type queue
	order_sem.*: a semaphore that releases threads following the order they were blocked
	kitchen.*: all problem specific functionso and structures

	main.c: executable file which uses semaphores
	main2.c: executable file which uses mutexes + condition variables

	OBS: in kitchen.*, kitchen_t uses semaphores and kitchen2_t uses mutexes+condition variables

	script.sh: a big set of tests for both implementations

	recipes.txt: store information about recipes (please does not change it)
	
	problem_explanation.pdf: algorithm explanation


How to test (same for main2):

	./main 			(run simulation with seed "time(NULL)")

	./main [seed] 		(run simulation with the provided seed)

	sh script.sh		(run heavy test)



Custom Parameters:

	At the beginning of both main.c and main2.c there are some defined constants that can be changed


	#define MAX_ORDERS	30   //numbers of orders to be ran
	#define MIN_ORDER_TIME 3     //minimum time between order arrivals
	#define MAX_ORDER_TIME 10    //maximum time between order arrivals
	#define TIME_UNIT 1 	     //time unit in milliseconds


Output: I recommend to read problem_explanation.pdf first

	huge_log.txt:  output of script.sh

	small_log.txt:  output for ./main 1   (with MAX_ORDERS=10; MIN_ORDER_TIME=5; MAX_ORDER_TIME=15; TIME_UNIT=1)


#define TIME_UNIT 1 //in milliseconds )
	small_log2.txt: output for ./main2 1  (with MAX_ORDERS=10; MIN_ORDER_TIME=5; MAX_ORDER_TIME=15; TIME_UNIT=1)

	You can notice that results for main and main2 are very similar.
	
	In the small_log you can see:
		- an example of "special dead-lock" avoidance
		- some cases that has 3 chefs at the same time (e.g.: lines 116 and 123 of small_log2.txt)
		- some cases of 2-dead-lock avoidance (e.g.: line 189 of small_log.txt)
		- some cases of 3-dead-lock avoidance (e.g.: line 77 of small_log.txt)

	You also can see that a chef only enters a region when the chef that is occuping it moves to his next station
	
	To follow the log, I recommend to draw diagrams along the reading (similar to the graphs presented in problem_explanation.pdf)




