Author:				Mateus Amarante Araujo
Date:				feb/01/2015
Project ID:			Project 2
CS Class:			CS 3013 (Operating Systems) - WPI
Programming Language:		C
How to build the program:	make [all]

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

Known issues:
	The system pop-ups an error when you unload phase1.ko
	It happens when it intercepts the sys_read system call
	You have to restart to be able to load LKM's
