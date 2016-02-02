#ifndef MY_SYSCALLS_H
#define MY_SYSCALLS_H

long cs3013_syscall2 (unsigned short *target_pid, unsigned short *target_uid);

long cs3013_syscall3 (unsigned short *target_pid, unsigned short *actual_uid);

#endif
