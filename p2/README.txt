Author:				Mateus Amarante Araujo
Date:				feb/01/2015
Project ID:			Project 2
CS Class:			CS 3013 (Operating Systems) - WPI
Programming Language:		C
How to build the program:	make [all]

Files:

Modules:
	phase0.c (simple syscall1 interception)
	phase1.c (open, close and read interceptions)
	phase2.c (syscall2 and syscall2, shift2user and getloginid implementations)

Executables:
	test0 (test phase0)
	shift2user [target_pid] [target_uid]
	getloginuid [target_pid]
	
Syslog files:

	syslog0.txt
	syslog1.txt
	syslog2.txt
	
	They also contain the terminal commands associated with the syslog messages

How to test:	
	> make [all]
	#run "tail -f /var/log/syslog" in a separated terminal to see the syslog in real-time
	#Kepp checking the log during all tests
	> tail -f /var/log/syslog
	
	#Test phase0 module
	> ./test0	
	> sudo insmod phase0.ko
	> ./test0
	> sudo rmmod phase0.ko
	
	#Test phase1 module
	> sudo insmod phase1.ko
	#Test opening and closing files
	#Test reading files/commands with the "VIRUS" string
	> sudo rmmod phase1.ko
	
	#Test phase2 module
	>sudo insmod phase2.ko
	>sleep 1000000 &
	#check pid. I will name it $PID
	>./shift2user $PID 1001
	>./getlogiuid $PID
	>sudo ./shift2user $PID 1000
	>./getlogiuid $PID
	>./shift2user 4468 1000
	>./getlogiuid $PID
	>sudo rmmod phase2.ko	

Known issues:
	The system pop-ups an error when you unload phase1.ko
	It happens when it intercepts the sys_read system call
	You have to restart to be able to load LKM's
	Forum topic:
	[Project 2, Part 1] dmesg: unable to handle kernel paging request	Eli Skeggs	2016-01-30 17:23:28
