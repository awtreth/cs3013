#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>

long cs3013_syscall2(unsigned short *target pid, unsigned short *target uid);
long cs3013_syscall3(unsigned short *target pid, unsigned short *actual uid);
