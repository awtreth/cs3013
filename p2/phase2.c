#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/cred.h>
#include <linux/slab.h>//kmalloc
#include <asm/current.h>//current
#include <linux/sched.h>//task_struct
#include <linux/list.h>//LinkedLists

unsigned long **sys_call_table;

asmlinkage long (*ref_sys_cs3013_syscall2)(unsigned short *target_pid, unsigned short *target_uid);
asmlinkage long (*ref_sys_cs3013_syscall3)(unsigned short *target_pid, unsigned short *target_uid);

/*Search for a process (task_struct) by his pid from a certain start point
 * 
 * @param from: start point process
 * @param pid:	target pid
 * @param result: will save the pointer to the target task_struct (output).
 * 		It must be NULL at the beginning and returns NULL if it doesn't find the target process
 * 
 * It will search for a process with pid "pid" among the children of the process "from"
 * It repeat the process recursevily for all children.
 * On this way, if you provide the "init" task_struct as the 1st parameter, you will look among all the procesess
 */
void look_down(struct task_struct *from, unsigned short pid, struct task_struct **result) {
	
	struct task_struct* task;//used in list_for_each_entry function

	//Basic case
	if(from->pid==pid) {//when pid matches (found it)
		//printk(KERN_DEBUG "result %u\n", from->pid);
		*result = from;//store in the output result
		return;
	}
	
	if(pid > from->pid) {//if pid > from_pid, the target process is not below the current "from"
		
		list_for_each_entry(task, &from->children, sibling) {//iterate through the children
			//printk(KERN_DEBUG "%u\n", task->pid);
			look_down(task,pid,result);//repeat for all children
			if(*result!=NULL) return;//Already found the process, which is unique. So it can finish the searching process 
		}
	}

}

/* Find the init process task_struct
 * 
 * @param from: start point (usually current macro, from <asm/current.h>)
 * 
 * @return: pointer the task_struct of the "init" process
 * 
 */
struct task_struct* find_init(struct task_struct *from) {
	
	//Basic case
	if(from->pid==1)
		return from;//found it
	
	//Check the parent (recursive call)
	return find_init(from->real_parent);
}

/* Look for the process with the provided pid
 * 
 * @param pid: target process id
 * @return: pointer to the target task_struct. It is NULL if it couldn't find the process
 */
struct task_struct* find_process(unsigned short pid) {
	struct task_struct* target_process = NULL;//return pointer
	
	struct task_struct* init = find_init(current);//find the init process (n_parent of everyone)

	look_down(init, pid, &target_process);//Look among all these n-order children

	return target_process;
}

//Possible errors for new_sys_cs3013_syscall2
//PID_NOT_FOUND can be returned by new_sys_cs3013_syscall3 too
#define DO_NOT_OWN_PROCESS  	-2 //current user does not own the target_process and it is not root
#define PID_NOT_FOUND			-3 //there is not a process with the target_pid
#define TARGETUID_NOT_ALLOWED	-4 //current user is not root and target_uid is not DEFAULT_TARGETUID

#define DEFAULT_TARGETUID 1001 //non-privileged secondary-user (with limited-access)


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
asmlinkage long new_sys_cs3013_syscall2(unsigned short *target_pid, unsigned short *target_uid) {
	
	//Copy parameters from user
	unsigned short ktarget_pid = 0, ktarget_uid = 0;
	struct task_struct* target_process;
	
	//Safely copy input variables from user space to kernel space
	if(	copy_from_user(&ktarget_pid, target_pid, sizeof(unsigned short)) ||
		copy_from_user(&ktarget_uid, target_uid, sizeof(unsigned short)) ) {
			printk(KERN_EMERG  "In sys_cs3013_syscall2: Invalid input pointer\n");
			return EFAULT;
	}
	
	target_process = find_process(ktarget_pid);
	
	//CHECKING

	if(target_process==NULL) {//did not find the target process
		printk(KERN_ERR "In sys_cs3013_syscall2: there is no process with pid %u\n", ktarget_pid);
		return PID_NOT_FOUND;
	}
	
	if(current_uid().val != 0) {//current user is not root
		
		if(current_uid().val != task_uid(target_process).val) {//Does the current process own target_process?
			printk(KERN_ERR "In sys_cs3013_syscall2: current user (%d) does not own process %d from user %d\n", current_uid().val, ktarget_pid, task_uid(target_process).val);
			return DO_NOT_OWN_PROCESS;
		}
		
		if(ktarget_uid != DEFAULT_TARGETUID) {//Is target_uid the DEFAULT_TARGETUID?
			printk(KERN_ERR "In sys_cs3013_syscall2: current user is not root. target_uid must be %d\n", DEFAULT_TARGETUID);
			return TARGETUID_NOT_ALLOWED;
		}
	}
	
	target_process->loginuid.val = ktarget_uid;//change the loginuid
	
	printk(KERN_DEBUG "Syscall2: shifted process %d from user %d to user %d\n", ktarget_pid, current_uid().val, ktarget_uid);
	
    return 0;
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
asmlinkage long new_sys_cs3013_syscall3(unsigned short *target_pid, unsigned short *actual_uid) {
	
	//Copy parameters from user
	unsigned short ktarget_pid = 0, kactual_uid = 0;
	struct task_struct* target_process;
	
	//Safely copy input variables from user space to kernel space
	if(	copy_from_user(&ktarget_pid, target_pid, sizeof(unsigned short)) |
		copy_from_user(&kactual_uid, actual_uid, sizeof(unsigned short)) ) {
			printk(KERN_EMERG  "In sys_cs3013_syscall3: Invalid input pointer\n");
			return EFAULT;
	}
	
	target_process = find_process(ktarget_pid);
	
	if(target_process==NULL) {//did not find the target process
		printk(KERN_ERR "In sys_cs3013_syscall2: there is no process with pid %u\n", ktarget_pid);
		return PID_NOT_FOUND;//TODO: error handling
	}
	
	//TODO: check if uid is valid (not necessary)
	
	kactual_uid = (unsigned short) target_process->loginuid.val;
	
	printk(KERN_DEBUG "Syscall3: target_pid = %u\t actual_uid = %u\n", ktarget_pid, kactual_uid);
	
	//Copy kactual_id to user space
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
