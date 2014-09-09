#include "mdrv_smc.h"
#include <linux/kernel.h>


#define TZ_LLV_DPRINTK(fmt, arg...) //printk(KERN_WARNING"%s:%d " fmt,__FUNCTION__,__LINE__,## args)

void smc_call(struct smc_struct *smc)
{
    volatile unsigned int u32cpsr;
    volatile unsigned int arg1, arg2;

    arg1 = smc->cmd1;
    arg2 = smc->cmd2;

    printk("XX cmd1 = %x, cmd2 = %x, arg1 = %x, arg2 = %x\n", smc->cmd1, smc->cmd2, arg1, arg2);
 
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

    smc->cmd1 = arg1;
    smc->cmd2 = arg2;

    printk("XX cmd1 = %x, cmd2 = %x, arg1 = %x, arg2 = %x\n", smc->cmd1, smc->cmd2, arg1, arg2);
}
