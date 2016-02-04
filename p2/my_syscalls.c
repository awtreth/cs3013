#include "my_syscalls.h"

#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>

// These values MUST match the unistd_32.h modifications:
//#define __NR_cs3013_syscall1 355
#define __NR_cs3013_syscall2 356
#define __NR_cs3013_syscall3 357

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
long cs3013_syscall2 (unsigned short *target_pid, unsigned short *target_uid) {
	return (long) syscall(__NR_cs3013_syscall2, target_pid, target_uid);
}

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
long cs3013_syscall3 (unsigned short *target_pid, unsigned short *actual_uid) {
	return (long) syscall(__NR_cs3013_syscall3, target_pid, actual_uid);
}

/* Print error status for the values returned by cs3013_syscall2 and cs3013_syscall3
 */
void print_error(long error) {
	switch (error)
	{
		case DO_NOT_OWN_PROCESS: printf("Current user does not own the target_process and it is not root\n"); break;
		case PID_NOT_FOUND: printf("There is not a process with the target_pid\n"); break;
		case TARGETUID_NOT_ALLOWED: printf("Current user is not root and target_uid is not DEFAULT_TARGETUID\n"); break;
		case EFAULT: printf("Invalid input pointer. defined in <errno.h>\n"); break;
		case 0: printf("No error\n"); break;
		
		default: printf("Not defined error\n");
	}
}
