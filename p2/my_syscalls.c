#include "my_syscalls.h"

#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>

// These values MUST match the unistd_32.h modifications:
//#define __NR_cs3013_syscall1 355
#define __NR_cs3013_syscall2 356
#define __NR_cs3013_syscall3 357

long cs3013_syscall2 (unsigned short *target_pid, unsigned short *target_uid) {
	return (long) syscall(__NR_cs3013_syscall2, target_pid, target_uid);
}

long cs3013_syscall3 (unsigned short *target_pid, unsigned short *actual_uid) {
	return (long) syscall(__NR_cs3013_syscall3, target_pid, actual_uid);
}
