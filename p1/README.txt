Author:						Mateus Amarante Araujo
Date:						jan/25/2015
Project ID:					Project 1
CS Class:					CS 3013 (Operating Systems)
Programming Language:		C
How to build the program:	make all
Problem Description:		see Project1_Description.pdf

How to run:	
	./runCommand [shell commands]
	
	./shell < [input file] (Ex: input_test.txt)
		or
	./shell
		[then enter commands (special built-in commands: 'exit' and 'cd')]
	
	./shell2 < [input file] (Ex: input_test2.txt)
		or			
	./shell2
		[then enter commands (special built-in commands: 'exit', 'cd', 'jobs')]
		#it accepts background processes (put '&' character at the end)



# Program Design for shell2

## Overall
	-parse commands
	-check and run (if it's requested) built-in commands
	-check if it is a background process
	-fork or get next command
	-if the process is not background, wait for it (show completed background process if they eventually finish)
	-if it's background, get next command
	-in all cases, check again if there are completed background process
	
	OBS: as you can see, I used the proposed method in the Project1_Description.pdf to handle with background processes

## Background process structure
	I created a structure called "bgprocess" which store the necessary information that we'll need later (see bgprocess.h)
	It is also structured to be grouped in a LinkedList, handled by the "bgprocessLL" structure (see bgprocess.h)
	bgprocessLL have the pointer to the first element and the LL size. On the same way, bgprocess has a pointer to the next bgprocess in the LinkedList
	To populate the LL, use "add2bgprocessLL" function (put in the first position). To remove them based on the pid, use "remove_bgprocess" function
	Actually, bgprocessLL works like a Map, where the pid is the key
	
	OBS: with command jobs, the processes are showed in reverse order (for convenience)
	

## Testing
	
	### shell
	
	With input_text.txt:
	- No argument program
	- Multiple argument program [test param parsing]
	- cd built-in command (pwd next for checking)
	- longer commands with system calls and I/O (ps -aux, lspci) [check rusage]
	- simple only-CPU process with no I/O operation (./test) [compare rusage]
	- repeat command after a while [check if we are not taking accumulated values]
	- missing argument (sleep)
	- wrong command (dafsjlad)
	
	By hand:
	- similar tests
	- tested exit command
	
	Special cases:
	- no command -> do nothing
	- cd -> go to /home
	
	
	### shell2
	
	Basically, tested cases involving background processes and 'jobs' built-in commands.
	All the tests were made with sleep command, because it's easier to handle with and I didn't
find any reason to use other one
	
	By hand:
	- run a bunch of background processes (Ex: "sleep 10 &") and keep checking with 'jobs' and other commands. It is supposed to:
		. show the running processes (in reverse order. Kept like this to make it simpler)
		. show the information about a bground process that just finished (pid, name and rusage)
		. it has to detect the envetual completion bground processes when it runs any other command, such as ('cd', '\n', bground and non-bground commands)
		. with "exit", it has to wait for the remaining bground processes (it also has to show the results)
	
	With input_text.txt:
	- It is supposed to run exactlly equal to shell
	
	With input_text2.txt:
	- multiple background processes, 'job', built-in and normal commands
	- it is supposed to wait for the remaining bground processes at the end
	
	
## Other comments

	- auxfnc.h (auxiliar functions)
	- bgprocess.h (bgprocess and bgprocessLL structures definitions and related functions)

	- The 'jobs' command show the processes in reverse order just for convenience.
The way the LinkedList was structured it's easier to pop the values from the front. To fix it, I could
store them in a temporary array (even another LL) and print them in the desired order. However it could take some time that I think it's not worth it
since it does not seem to be the focus of the project

	- I debugged the code in respect to memory leaks using the "mtrace" command/library. To use it, just do:
	
		"#include <mcheck.h>" and run "mtrace()" at the beggining of the code (you could limit to an area, calling umtrace)
		
		compile the code with the "-g" flag (Debug mode)
		
		specify where you want to save the log, setting the environment variable "MALLOC_TRACE"
			Ex: export MALLOC_TRACE=/home/mateus/log.txt
		
		Then just run your program.
		
		To see the log in a more friendly way, just run "mtrace log.txt"
		
		http://man7.org/linux/man-pages/man3/mtrace.3.html
