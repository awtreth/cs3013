#ifndef MY_SYSCALLS_H
#define MY_SYSCALLS_H

//Possible errors for new_sys_cs3013_syscall2
//PID_NOT_FOUND can be returned by new_sys_cs3013_syscall3 too
#define DO_NOT_OWN_PROCESS  	-2 //current user does not own the target_process and it is not root
#define PID_NOT_FOUND			-3 //there is not a process with the target_pid
#define TARGETUID_NOT_ALLOWED	-4 //current user is not root and target_uid is not DEFAULT_TARGETUID
//EFAULT: invalid input pointer. defined in <errno.h>

#include <errno.h>

#define DEFAULT_TARGETUID 1001 //non-privileged secondary-user (with limited-access). Must match with definition in phase2.c

//You can check debug messages in /var/log/syslog

/*shift2user
 * 
 * Try to shift the loginuid value of process with pid *target_pid to *target_uid.
 * 
 * Restrictions when the current user is not root:
 * 		- It must own the target process
 * 		- target_uid must contain DEFAULT_TARGETUID
 * 
 * @param target_pid: pointer to the process id number of the process that you want to change uid
 * @param target_uid: pointer to the user id number that you want to switch to
 * 
 * @return: 0 when it's OK
 * 			can return DO_NOT_OWN_PROCESS, PID_NOT_FOUND, TARGETUID_NOT_ALLOWED or EFAULT (described above) for errors
 * 
 */
long cs3013_syscall2 (unsigned short *target_pid, unsigned short *target_uid);

/*getloginuid
 * 
 * Read the loginuid value of process with pid *target_pid. Store result in target_uid pointer

 * 
 * @param target_pid: pointer to the process id number of the process that you read the loginuid from
 * @param target_uid: where you store the result (must be pre-allocated by theuser)
 * 
 * @return: 0 when it's OK
 * 			can PID_NOT_FOUND or EFAULT (described above) for errors
 * 
 */
long cs3013_syscall3 (unsigned short *target_pid, unsigned short *actual_uid);

/* Print error status for the values returned by cs3013_syscall2 and cs3013_syscall3
 */
void print_error(long error);



#endif
