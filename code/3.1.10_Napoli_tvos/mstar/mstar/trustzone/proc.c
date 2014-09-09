/*
 * linux/driver/mstar/trustzone/trustzone.c
 *
 * Copyright (C) 1992, 1998-2004 Linus Torvalds, Ingo Molnar
 *
 * This file contains the /proc/irq/ handling code.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include "proc.h"
#include "mdrv_trustzone.h"
#include <linux/kthread.h>

#define MAX_NAMELEN 10

static struct proc_dir_entry *proc_entry;
static struct proc_dir_entry *root_tz_dir;
static struct mutex tz_lock;

/*
#define L_PTE_PRESENT           (1 << 0)
#define L_PTE_YOUNG             (1 << 1)
#define L_PTE_FILE              (1 << 2)        
#define L_PTE_DIRTY             (1 << 6)
#define L_PTE_WRITE             (1 << 7)
#define L_PTE_USER              (1 << 8)
#define L_PTE_EXEC              (1 << 9)
#define L_PTE_SHARED            (1 << 10)       

struct mem_type {
        unsigned int prot_pte;
        unsigned int prot_l1;
        unsigned int prot_sect;
        unsigned int domain;
};

#define CPU_MODE_FIQ 0x11
#define CPU_MODE_IRQ 0x12
#define CPU_MODE_SVC 0x13
#define CPU_MODE_ABT 0x17
#define CPU_MODE_UND 0x1b
#define CPU_MODE_SYS 0x1f
#define CPU_MODE_MON 0x16
*/
/*
struct tz_struct{

    volatile unsigned int cmd1;
    volatile unsigned int cmd2;

    volatile unsigned int* private_data;

};

void __smc(struct tz_struct *tz)
{
    volatile unsigned int u32cpsr;
    volatile unsigned int arg1=0xaa, arg2=0xbb;

    __asm__ volatile (
        "mrs %0, cpsr\n\t"
        "push {r0, r4-r12, lr}\n\t"
	"mov r1, %1\n\t"
	"mov r2, %2\n\t"
        "mov  r3, sp  \n\t"
        "smc  0       \n\t"
        "mov sp, r3\n\t"
        "pop {r0, r4-r12, lr}\n\t"
        "mov %1, r1\n\t"
        "mov %2, r2\n\t"
        "msr cpsr, %0\n\t"
        : "+&r" (u32cpsr), "+&r" (arg1), "+&r" (arg2)
        :
        : "r0", "r1", "r2", "r3", "memory");

    tz->cmd1 = arg1;
    tz->cmd2 = arg2;

    printk("XX cmd1 = %x, cmd2 = %x, arg1 = %x, arg2 = %x\n", tz->cmd1, tz->cmd2, arg1, arg2);
}
*/
/*
static DECLARE_WAIT_QUEUE_HEAD(tz_wait);
static unsigned int tz_flag;

void __smc_wrapper()
{
    unsigned int cpu = smp_processor_id();
    int i = 0;

    struct tz_struct *tz;

    tz = kmalloc(sizeof(struct tz_struct), GFP_KERNEL);

    tz->cmd1 = 0xaa;
    tz->cmd2 = 0xbb;

    printk("[TZ] cpu = %d before enter secure world cmd1 = %x, cmd2 = %x\n", cpu, tz->cmd1, tz->cmd2);

    if(cpu ==1)
    {
        //for(i=0;i<100000000;i++);
        {
            printk("SMC test loop = %d\n", i);
            smc_call(tz);
        }
    }
    else 
    {
       printk("[[Error SMC call CPU = %d!!\n", cpu);
    }

    printk("[TZ] cpu = %d before leave secure world \ncmd1 = %x, cmd2 = %x\n", cpu, tz->cmd1, tz->cmd2);

    kfree(tz);

}
*/
/*
void __smc_thread()
{
    printk("SMC thread start!!\n\n");


    while(1)
    {
       printk("SEC heart beat!!\n");

       wait_event_interruptible(tz_wait, tz_flag != 0);
       tz_flag = 0;
        
       __smc_wrapper();
    }

}

static struct task_struct *task;
*/
int tz_write( struct file *filp, const char __user *buff,
                        unsigned long len, void *data )
{
  unsigned int cpu = smp_processor_id();
 
  mutex_lock(&tz_lock);

  printk("tz_write\n");
  
  tz_call(NULL);
  //tz_flag = 1;
  //wake_up_interruptible(&tz_wait);
  //while(task_is_dead(task))
  //   kthread_stop(task);

  mutex_unlock(&tz_lock);

  return len;
}

int tz_read( char *page, char **start, off_t off,
                   int count, int *eof, void *data )
{

  printk("tz_read\n");
  return 1;
}
extern void iotable_init(struct map_desc *, int);
int __init init_tz_proc( void )
{
    int ret = 0;
    char name [MAX_NAMELEN];

    mutex_init(&tz_lock); 
  
    printk("Init tz proc\n");
    memset(name, 0, MAX_NAMELEN);
    sprintf(name, "%s", "prompt");

    root_tz_dir = proc_mkdir("tz", NULL);
  
    proc_entry = create_proc_entry( "prompt", 0644, root_tz_dir );
    if (proc_entry == NULL) {
      ret = -ENOMEM;
      printk(KERN_INFO "tz_proc: Couldn't create proc entry\n");
      
    } 
    else 
    {
      proc_entry->read_proc = tz_read;
      proc_entry->write_proc = tz_write;
      //proc_entry->owner = THIS_MODULE;
      printk(KERN_INFO "tz: Module loaded.\n");
    }
/*
    task = kthread_create(__smc_thread, NULL, "SMC handle");
 
    kthread_bind(task, 1); //bind the thread on processor 1


    if (!IS_ERR(task))
        wake_up_process(task);
    else 
        printk("smc fail !!\n");
*/
    return ret;
}

int __exit exit_tz_proc(void)
{
    printk("exit!\n");
    return 1;
}

MODULE_DESCRIPTION("TZ driver support");
MODULE_AUTHOR("MStar");
MODULE_LICENSE("GPL");
module_init(init_tz_proc);
module_exit(exit_tz_proc);


