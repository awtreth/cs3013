#ifndef MY_SYSCALLS_H
#define MY_SYSCALLS_H

#define UID_NOT_EXIST  	-1 //target_uid does not exist
#define NOT_ALLOWED  	-2 //caller process does not own the target_process and it is not of root
#define PID_NOT_FOUND	-3 //there is not a process with the target_pid
//EFAULT: invalid input

//shift2user
long cs3013_syscall2 (unsigned short *target_pid, unsigned short *target_uid);

//getlogiuid
long cs3013_syscall3 (unsigned short *target_pid, unsigned short *actual_uid);

#endif
