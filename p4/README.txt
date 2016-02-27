Author:				Mateus Amarante Araujo
Project ID:			Project 4
CS Class:			CS 3013 (Operating Systems) - WPI
Programming Language:		C
How to build the program:	make [all]

Files:
-memory.*: virtual memory routines (including API)
-test* : suggested tests
-output*: output of the related tests (indicated by eviction algorithm)

Eviction Algorithms:
1) Random
2) Clock
3) Clock with 2nd chance (history recorded by reference bit)

Compiling and Testing:
in memory.h you can setup the following constants (define)
#define RAM_SIZE 25
#define SSD_SIZE 100
#define HDD_SIZE 1000
#define TIME_FACTOR 1 //Divide the default times by this number (RAM: 0.01s, SSD: 0.1s, HDD:2.5s)

OBS: I recommend to use RAM_SIZE = 3; SSD = 12 and HDD = 50 for stress tests

int test_multi* you can set the following constants

#define N_THREADS 5 		//number of threads
#define READS_PER_THREAD   30   //number of operations per thread

In all tests you can provide up to 2 command line arguments

arg1: eviction algorithm (0-Random; 2-Clock; 3-Clock with 2nd Chance)
arg2: seed for random number generations (it is set to time(NULL) if it is not provided)

OBS: test_single wait for user input (there are "enter to continue" messages)

More details about tests in test_methodology.txt



Synchronization problem:
- The current version implements multi-thread memory access without dead-lock problems, but only with mutex-trylock (kind of busy-waiting). For this reason the eviction algorithm analysis
in multi-thread systems is very hard.

- I think I have a conceptually good strategy, but I couldn't finish its implementation in time:
	In my code I have one mutex for each page entry and other 2 to protect auxiliar strucutures (bitmask, cursor for clock algorithm implementation)	
	
	In my code there is one mutex for each page entry and other 2 to protect auxiliary structures (bitmask, cursor for clock algorithm implementation).

	The algorithm relies on guarantee that every function locks the mutexes on the same direction. That is, every function has to lock a page entry in RAM, followed by SSD and HDD (if necessary). Also, each operation can only lock one page per memory. 
	This restriction is fairly reasonable, since page faults and evictions start checking from RAM, going to SSD and may also access the HDD.
	On this way, one thread that is waiting for a page in SSD can trust that the thread that is locking it is not waiting for the ram address that the first thread has already locked. The second thread already own his RAM page to be in SSD.

	While trying to implement this strategy, I faced problems in dealing with page faults. To identify that a page fault is necessary, I have to lock the corresponding page entry first (which is not in RAM), unlock it, select a page in RAM to be evicted (lock it) and get access to the original page (lock in SSD or HDD). I also had problems with mutual access to the auxiliar strucutures.
	I have some ideas to solve these problems (started testing it), but I didn't have time to try them.


