#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/cred.h>
#include <linux/slab.h>//kmalloc

unsigned long **sys_call_table;

//pointer to the old read_function
asmlinkage long (*ref_sys_read)(unsigned int fd, char __user *buf, size_t count);
asmlinkage long (*ref_sys_open)(const char __user *filename, int flags, umode_t mode);
asmlinkage long (*ref_sys_close)(unsigned int fd);

#define TRUE  1
#define FALSE 0

//Adapted from: https://stuff.mit.edu/afs/sipb/project/tcl80/src/tcl8.0/compat/strstr.c
int contain_str(char* substring_, char* string_, size_t size) {
	
	char *string = string_, *substring = substring_;
	char *b = substring;
	size_t i = 0;
    /* First scan quickly through the two strings looking for a
     * single-character match.  When it's found, then compare the
     * rest of the substring.
     */
    
    for (; i < size; i++, string += 1) {
		
		if (*b == *string) {
			b++;
			if(*b=='\0')
				return TRUE;
			continue;
		}

		b = substring;
    }
    
    return FALSE;
}

//New version of sys_open
asmlinkage long new_sys_open(const char __user *filename, int flags, umode_t mode){
	printk(KERN_INFO "User %u is opening file: %s\n", current_uid().val, filename);
	
	return (*ref_sys_open)(filename, flags, mode);
}

//New version of sys_close
asmlinkage long new_sys_close(unsigned int fd) {
	printk(KERN_INFO "User %u is closing file descriptor: %u\n", current_uid().val, fd);
	
	return (*ref_sys_close)(fd);
}

//New version of sys_read
asmlinkage long new_sys_read(unsigned int fd, char __user *buf, size_t count) {
    

	char* kbuf = (char*) kmalloc(count,GFP_KERNEL);

	if(copy_from_user(kbuf,buf,count))
		return EFAULT;

    long n_bytes = (*ref_sys_read)(fd, buf, count);
	
	if(contain_str("VI""RUS", kbuf, count) == TRUE)
		printk(KERN_EMERG "User %u read from file descriptor %u, but that read contained malicious code!\n", current_uid().val, fd);
    
    kfree(kbuf);
    
    return n_bytes;
    return (*ref_sys_read)(fd, buf, count);
}



//Find the pointer of the system call table (first element)
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

//Module_init Function
static int __init interceptor_start(void) {
    /* Find the system call table */
    if(!(sys_call_table = find_sys_call_table())) {
        /* Well, that didn’t work.
        Cancel the module loading step. */
        return -1;
    }
    /* Store a copy of all the existing functions */
    ref_sys_open = (void *)sys_call_table[__NR_open];
    ref_sys_close = (void *)sys_call_table[__NR_close];
    ref_sys_read = (void *)sys_call_table[__NR_read];
    
    
    /* Replace the existing system calls */
    disable_page_protection();
    
    sys_call_table[__NR_open] = (unsigned long *)new_sys_open;
    sys_call_table[__NR_close] = (unsigned long *)new_sys_close;
    sys_call_table[__NR_read] = (unsigned long *)new_sys_read;

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
    
    sys_call_table[__NR_open] = (unsigned long *)ref_sys_open;
    sys_call_table[__NR_close] = (unsigned long *)ref_sys_close;
    sys_call_table[__NR_read] = (unsigned long *)ref_sys_read;
    
    enable_page_protection();
    
    
    printk(KERN_INFO "Unloaded interceptor!\n");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);
