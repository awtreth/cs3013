#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/cred.h>
#include <linux/slab.h>//kmalloc
#include <asm/current.h>//current
#include <linux/sched.h>//task_struct

unsigned long **sys_call_table;

//asmlinkage long (*ref_sys_cs3013_syscall1)(void);
asmlinkage long (*ref_sys_cs3013_syscall2)(unsigned short *target_pid, unsigned short *target_uid);
asmlinkage long (*ref_sys_cs3013_syscall3)(unsigned short *target_pid, unsigned short *target_uid);


//new system call functions

//Definition of the new system call functions
//~ asmlinkage long new_sys_cs3013_syscall1(void) {
    //~ printk(KERN_INFO "\"’Hello world?!’ More like ’Goodbye, world!’ EXTERMINATE!\" -- Dalek\n");
    //~ return 0;
//~ }

asmlinkage long new_sys_cs3013_syscall2(unsigned short *target_pid, unsigned short *target_uid) {
	
	//Copy parameters from user
	unsigned short ktarget_pid = 0, ktarget_uid = 0;
	
	
	if(	copy_from_user(&ktarget_pid, target_pid, sizeof(unsigned short)) ||
		copy_from_user(&ktarget_uid, target_uid, sizeof(unsigned short)) ) {
			printk(KERN_INFO "ERRO\n");
			return EFAULT;
	}
	
	//printk(KERN_INFO "target_pid: %u;\t pid: %u\n",ktarget_pid, current->pid);
	
    return 0;
}

asmlinkage long new_sys_cs3013_syscall3(unsigned short *target_pid, unsigned short *actual_uid) {
	
	//Copy parameters from user
	unsigned short ktarget_pid = 0, kactual_uid = 0;
	
	if(	copy_from_user(&ktarget_pid, target_pid, sizeof(unsigned short)) |
		copy_from_user(&kactual_uid, actual_uid, sizeof(unsigned short)) ) {
			return EFAULT;
	}
	
	
	
	//Copy actual_id to uinclude/linux/sched.hser
	copy_to_user(actual_uid, &kactual_uid, sizeof(unsigned short));
	
	
    return 0;
}


static unsigned long **find_sys_call_table(void) {
    unsigned long int offset = PAGE_OFFSET;
    unsigned long **sct;
    
    while (offset < ULLONG_MAX) {
        sct = (unsigned long **)offset;
        if (sct[__NR_close] == (unsigned long *) sys_close) {
            printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX\n",
                   (unsigned long) sct);
            return sct;
        }
        offset += sizeof(void *);
    }
    return NULL;
}

static void disable_page_protection(void) {
    /*
    Control Register 0 (cr0) governs how the CPU operates.
    Bit #16, if set, prevents the CPU from writing to memory marked as
    read only. Well, our system call table meets that description.
    But, we can simply turn off this bit in cr0 to allow us to make
    changes. We read in the current value of the register (32 or 64
    bits wide), and AND that with a value where all bits are 0 except
    the 16th bit (using a negation operation), causing the write_cr0
    value to have the 16th bit cleared (with all other bits staying
    the same. We will thus be able to write to the protected memory.
    It’s good to be the kernel!
    */
    write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
    /*
    See the above description for cr0. Here, we use an OR to set the
    16th bit to re-enable write protection on the CPU.
    */
    write_cr0 (read_cr0 () | 0x10000);
}
static int __init interceptor_start(void) {
    /* Find the system call table */
    if(!(sys_call_table = find_sys_call_table())) {
        /* Well, that didn’t work.
        Cancel the module loading step. */
        return -1;
    }
    /* Store a copy of all the existing functions */
    //ref_sys_cs3013_syscall1 = (void *)sys_call_table[__NR_cs3013_syscall1];
    ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];
    ref_sys_cs3013_syscall3 = (void *)sys_call_table[__NR_cs3013_syscall3];
    
    
    /* Replace the existing system calls */
    disable_page_protection();
    //sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)new_sys_cs3013_syscall1;
    sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;
    sys_call_table[__NR_cs3013_syscall3] = (unsigned long *)new_sys_cs3013_syscall3;

    enable_page_protection();
    /* And indicate the load was successful */
    printk(KERN_INFO "Loaded interceptor!\n");
    return 0;
}

static void __exit interceptor_end(void) {
    /* If we don’t know what the syscall table is, don’t bother. */
    if(!sys_call_table)
        return;
    
    /* Revert all system calls to what they were before we began. */
    disable_page_protection();
    
    //sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)ref_sys_cs3013_syscall1;
    sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;
    sys_call_table[__NR_cs3013_syscall3] = (unsigned long *)ref_sys_cs3013_syscall3;
    
    enable_page_protection();
    
    printk(KERN_INFO "Unloaded interceptor!\n");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);
