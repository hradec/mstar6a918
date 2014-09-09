/*-----------------------------------------------------------------------------
    Include Files
------------------------------------------------------------------------------*/
#include <linux/suspend.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/rtc.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/atomic.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#include <asm/mach/time.h>
#include <asm/mach/irq.h>
#include <asm/mach-types.h>
#include <mach/pioneer_regs.h>
#include <mach/pm.h>
#include <mach/io.h>
#include <asm/cputype.h>
#include "chip_int.h"
#include "sleep_helper.h"

extern void Chip_Flush_Cache_All(void);

#define WAKEUP_SAVE_ADDR 0xC0000000
#define INT_MASK_REG_BASE IO_ADDRESS(REG_INT_BASE_PA)

extern void sleep_save_cpu_registers(void * pbuffer);
extern void sleep_restore_cpu_registers(void * pbuffer);
extern void sleep_set_wakeup_save_addr_phy(unsigned long phy_addr, void *virt_addr);
extern void sleep_clear_wakeup_save_addr_phy(unsigned long phy_addr, void *virt_addr);
extern void sleep_prepare_last(unsigned long wakeup_addr_phy);
extern void sleep_wakeup_first(unsigned long boot_start,void *exit_addr_virt);
extern void sleep_save_neon_regs(void * pbuffer);
extern void sleep_restore_neon_regs(void * pbuffer);
extern void  MDrv_MBX_NotifyPMtoSetPowerOff(void);
extern void  MDrv_MBX_NotifyPMPassword(unsigned char passwd[16]);
extern void SerPrintf(char *fmt,...);
extern void SerPrintfAtomic(char *fmt,...);

extern void __iomem *gic_cpu_base_addr;
extern void __iomem *gic_dist_base_addr;
static u32 MStar_Suspend_Buffer[SLEEPDATA_SIZE];
static u32 MStar_IntMaskSave[8];
DEFINE_SPINLOCK(ser_printf_lock);

#if (defined(CONFIG_MSTAR_STR_COUNT_MAX)&&(CONFIG_MSTAR_STR_COUNT_MAX>0))||defined(CONFIG_MSTAR_STR_ACOFF_ON_ERR)
static unsigned char pass_wd[16]={0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,
                                   0x11,0x00,0xFF,0xEE,0xDD,0xCC,0xBB,0xAA};
#endif

#if defined(CONFIG_MSTAR_STR_COUNT_MAX)&&(CONFIG_MSTAR_STR_COUNT_MAX>0)
static u32 mstr_cnt=0;
#endif

void SerPrintChar(char ch)
{
    unsigned int tmpch=ch;
    if(tmpch=='\n')
    {
        tmpch='\r';
        __asm__ volatile (
        "ldr r4, %0\n"
        "ldr r5, =0xFD201300\n"
        "strb r4,[r5]\n"
        ::"m"(tmpch):"r4","r5","cc","memory"
        );
        udelay(500);
        tmpch='\n';
    }
    __asm__ volatile (
        "ldr r4, %0\n"
        "ldr r5, =0xFD201300\n"
        "strb r4,[r5]\n"
        ::"m"(tmpch):"r4","r5","cc","memory"
        );
    udelay(500);
}
void SerPrintStr(char *p)
{
    int nLen=strlen(p);
    int i;
    for(i=0;i<nLen;i++)
    {
        SerPrintChar(p[i]);
    }
}
void SerPrintStrAtomic(char *p)
{
    u_long flag;
    spin_lock_irqsave(&ser_printf_lock,flag);
    SerPrintStr(p);
    spin_unlock_irqrestore(&ser_printf_lock,flag);
}
void SerPrintf(char *fmt,...)
{
    char tmpbuf[500];
    int nLen;
    va_list args;
    va_start(args, fmt);
    nLen=vscnprintf(tmpbuf, 500, fmt, args);
    va_end(args);
    if(nLen<=0)
    {
        nLen=0;
    }
    else if(nLen>=500)
    {
        nLen=500-1;
    }
    tmpbuf[nLen]=0;
    SerPrintStr(tmpbuf);
}
void SerPrintfAtomic(char *fmt,...)
{
    char tmpbuf[500];
    int nLen;
    va_list args;
    va_start(args, fmt);
    nLen=vscnprintf(tmpbuf, 500, fmt, args);
    va_end(args);
    if(nLen<=0)
    {
        nLen=0;
    }
    else if(nLen>=500)
    {
        nLen=500-1;
    }
    tmpbuf[nLen]=0;
    SerPrintStrAtomic(tmpbuf);
}
int vSerPrintf(const char *fmt, va_list args)
{
    char tmpbuf[500];
    int nLen;
    nLen=vscnprintf(tmpbuf, 500, fmt, args);
    if(nLen<=0)
    {
        nLen=0;
    }
    else if(nLen>=500)
    {
        nLen=500-1;
    }
    tmpbuf[nLen]=0;
    SerPrintStr(tmpbuf);
    return nLen;
}
int vSerPrintfAtomic(const char *fmt, va_list args)
{
    char tmpbuf[500];
    int nLen;
    nLen=vscnprintf(tmpbuf, 500, fmt, args);
    if(nLen<=0)
    {
        nLen=0;
    }
    else if(nLen>=500)
    {
        nLen=500-1;
    }
    tmpbuf[nLen]=0;
    SerPrintStrAtomic(tmpbuf);
    return nLen;
}

void mstar_save_int_mask(void)
{
    volatile unsigned long *int_mask_base=(volatile unsigned long *)INT_MASK_REG_BASE;
    MStar_IntMaskSave[0]=int_mask_base[0x24];
    MStar_IntMaskSave[1]=int_mask_base[0x25];
    MStar_IntMaskSave[2]=int_mask_base[0x26];
    MStar_IntMaskSave[3]=int_mask_base[0x27];
    MStar_IntMaskSave[4]=int_mask_base[0x34];
    MStar_IntMaskSave[5]=int_mask_base[0x35];
    MStar_IntMaskSave[6]=int_mask_base[0x36];
    MStar_IntMaskSave[7]=int_mask_base[0x37];
}
void mstar_restore_int_mask(void)
{
    volatile unsigned long *int_mask_base=(volatile unsigned long *)INT_MASK_REG_BASE;
    int_mask_base[0x24]=MStar_IntMaskSave[0];
    int_mask_base[0x25]=MStar_IntMaskSave[1];
    int_mask_base[0x26]=MStar_IntMaskSave[2];
    int_mask_base[0x27]=MStar_IntMaskSave[3];
    int_mask_base[0x34]=MStar_IntMaskSave[4];
    int_mask_base[0x35]=MStar_IntMaskSave[5];
    int_mask_base[0x36]=MStar_IntMaskSave[6];
    int_mask_base[0x37]=MStar_IntMaskSave[7];
}
void mstar_clear_int(void)
{
    volatile unsigned long *int_mask_base=(volatile unsigned long *)INT_MASK_REG_BASE;
    int_mask_base[0x2c]=0xFFFF;int_mask_base[0x2c]=0;
    int_mask_base[0x2d]=0xFFFF;int_mask_base[0x2d]=0;
    int_mask_base[0x2e]=0xFFFF;int_mask_base[0x2e]=0;
    int_mask_base[0x2f]=0xFFFF;int_mask_base[0x2f]=0;
    int_mask_base[0x3c]=0xFFFF;int_mask_base[0x3c]=0;
    int_mask_base[0x3d]=0xFFFF;int_mask_base[0x3d]=0;
    int_mask_base[0x3e]=0xFFFF;int_mask_base[0x3e]=0;
    int_mask_base[0x3f]=0xFFFF;int_mask_base[0x3f]=0;
}
unsigned long mstar_virt_to_phy(void* virtaddr)
{
    unsigned long rest=0;
    rest=virt_to_phys(virtaddr);
    return rest;
}

void* mstar_phy_to_virt(unsigned long phyaddr )
{
    void *rest=0;
    rest=phys_to_virt(phyaddr);
    return rest;
}

void mstar_sleep_flush_all(void)
{
    Chip_Flush_Cache_All();
}

#if defined(CONFIG_MSTAR_STR_COUNT_MAX)&&(CONFIG_MSTAR_STR_COUNT_MAX>0)
static void mstar_str_notifypmmaxcnt_off(void)
{
    pass_wd[0x0A]=0xFD;
    MDrv_MBX_NotifyPMPassword(pass_wd);
    while(1);
}
#endif

#if defined(CONFIG_MSTAR_STR_ACOFF_ON_ERR)
void mstar_str_notifypmerror_off(void)
{
    pass_wd[0x0A]=0xFE;
    MDrv_MBX_NotifyPMPassword(pass_wd);
    while(1);
}
#endif

void WriteRegWord(unsigned long addr, unsigned long val)
{
    volatile unsigned long *regaddr=(unsigned long *)addr;
    (*regaddr)=val;
}
unsigned long ReadRegWord(unsigned long addr)
{
    volatile unsigned long *regaddr=(unsigned long *)addr;
    return (*regaddr);
}

/*------------------------------------------------------------------------------
    Function: mstar_pm_enter

    Description:
        Actually enter sleep state
    Input: (The arguments were used by caller to input data.)
        state - suspend state (not used)
    Output: (The arguments were used by caller to receive data.)
        None.
    Return:
        0
    Remark:
        None.
-------------------------------------------------------------------------------*/
static int mstar_pm_enter(suspend_state_t state)
{
    void *pWakeup=0;
    __asm__ volatile (
        "ldr r1, =MSTAR_WAKEUP_ENTRY\n"
        "str r1, %0"
        :"=m"(pWakeup)::"r1"
        );
#if defined(CONFIG_MSTAR_STR_COUNT_MAX)&&(CONFIG_MSTAR_STR_COUNT_MAX>0)
    mstr_cnt++;
#endif

    mstar_save_int_mask();
    save_performance_monitors((appf_u32 *)performance_monitor_save);
    save_a9_timers((appf_u32*)&a9_timer_save, PERI_ADDRESS(PERI_PHYS));
    save_a9_global_timer((appf_u32 *)a9_global_timer_save,PERI_ADDRESS(PERI_PHYS));

    save_gic_interface((appf_u32 *)gic_interface_save,(unsigned)gic_cpu_base_addr,1);
    save_gic_distributor_private((appf_u32 *)gic_distributor_private_save,(unsigned)gic_dist_base_addr,1);

    save_cp15((appf_u32 *)cp15_save);// CSSELR
    save_a9_other((appf_u32 *)a9_other_save,1);

    save_v7_debug((appf_u32 *)&a9_dbg_data_save);

    save_gic_distributor_shared((appf_u32 *)gic_distributor_shared_save,(unsigned)gic_dist_base_addr,1);
    //start add
    save_control_registers(control_data, 1);
    save_mmu(mmu_data);
    //end add
    save_a9_scu((appf_u32 *)a9_scu_save,PERI_ADDRESS(PERI_PHYS));
    save_pl310((appf_u32*)&pl310_context_save,L2_CACHE_ADDRESS(L2_CACHE_PHYS));

    sleep_save_neon_regs(&MStar_Suspend_Buffer[SLEEPSTATE_NEONREG/WORD_SIZE]);
    sleep_save_cpu_registers(MStar_Suspend_Buffer);
    sleep_set_wakeup_save_addr_phy(mstar_virt_to_phy((void*)WAKEUP_SAVE_ADDR),(void*)WAKEUP_SAVE_ADDR);

    sleep_prepare_last(mstar_virt_to_phy(pWakeup));
    write_actlr(read_actlr() & ~A9_SMP_BIT);//add
    SerPrintf("\nMStar STR waiting power off...\n");
    __asm__ volatile (
        "nop\n"
        :::"r0","r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12"
        );
    //__asm__ volatile(
    //    "SUSPEND_WAIT:\n"
    //    "nop\n"
    //    "nop\n"
    //    "b SUSPEND_WAIT\n"
    //    );
#if defined(CONFIG_MSTAR_STR_COUNT_MAX)&&(CONFIG_MSTAR_STR_COUNT_MAX>0)
    // pass different password to do ac onoff
    if(mstr_cnt>=CONFIG_MSTAR_STR_COUNT_MAX)
    {
        mstar_str_notifypmmaxcnt_off();
    }
    else
#endif
    MDrv_MBX_NotifyPMtoSetPowerOff();
    __asm__ volatile(
        "WAIT_SLEEP:\n"
        "nop\n"
        "nop\n"
        "b WAIT_SLEEP\n"
        );

    //////////////////////////////////////////////////////////////
    __asm__ volatile(
        "MSTAR_WAKEUP_ENTRY:\n"
        "bl ensure_environment\n"
        "bl use_tmp_stack\n"
        "mov r0, #'K'\n"
        "bl __PUTCHAR\n"
        "ldr r1, =exit_addr\n"
        "sub r0, pc,#4 \n"
        "b   sleep_wakeup_first\n"          //sleep_wakeup_first();
        "exit_addr: \n"
        "mov r0, #'L'\n"
        "bl PUTCHAR_VIRT\n"
        "ldr r0,=MStar_Suspend_Buffer\n"
        "bl sleep_restore_cpu_registers\n"  //sleep_restore_cpu_registers(MStar_Suspend_Buffer)
        :::"r0","r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12"
    );
    SerPrintf("\nMStar STR Resuming...\n");
    sleep_restore_neon_regs(&MStar_Suspend_Buffer[SLEEPSTATE_NEONREG/WORD_SIZE]);
    restore_a9_scu((appf_u32 *)a9_scu_save,PERI_ADDRESS(PERI_PHYS));
    restore_pl310((appf_u32*)&pl310_context_save,L2_CACHE_ADDRESS(L2_CACHE_PHYS), 0); //means power off
    //start add
    restore_mmu(mmu_data);
    restore_control_registers(control_data, 1);
    //end add
    restore_v7_debug((appf_u32 *)&a9_dbg_data_save);
    restore_gic_distributor_shared((appf_u32 *)gic_distributor_shared_save,(unsigned)gic_dist_base_addr,1);
    gic_distributor_set_enabled(TRUE, (unsigned)gic_dist_base_addr);//add
    restore_gic_distributor_private((appf_u32 *)gic_distributor_private_save,(unsigned)gic_dist_base_addr,1);
    restore_gic_interface((appf_u32 *)gic_interface_save,(unsigned)gic_cpu_base_addr,1);

    restore_a9_other((appf_u32 *)a9_other_save,1);
    restore_cp15((appf_u32 *)cp15_save);

    restore_a9_timers((appf_u32*)&a9_timer_save, PERI_ADDRESS(PERI_PHYS));
    restore_a9_global_timer((appf_u32 *)a9_global_timer_save,PERI_ADDRESS(PERI_PHYS));
    restore_performance_monitors((appf_u32 *)performance_monitor_save);

    mstar_restore_int_mask();

    sleep_clear_wakeup_save_addr_phy(mstar_virt_to_phy((void*)WAKEUP_SAVE_ADDR),(void*)WAKEUP_SAVE_ADDR);
    mstar_sleep_flush_all();

#if 1  //switch FIQ/IRQ merge bit
    {
        //reg_writew(0x0002, 0x1F203114);
        unsigned short tmp;
        tmp = reg_readw(0x1f203adc);
        tmp |= 0x20;  //FPGA-820 ,08/22 sof
        reg_writew(tmp, 0x1f203adc);
    }
#endif

	return 0;
}

static struct platform_suspend_ops mstar_pm_ops =
{
	.enter		= mstar_pm_enter,
	.valid		= suspend_valid_only_mem,
};


/*------------------------------------------------------------------------------
    Function: mstar_pm_init

    Description:
        init function of power management
    Input: (The arguments were used by caller to input data.)
        None.
    Output: (The arguments were used by caller to receive data.)
        None.
    Return:
        0
    Remark:
        None.
-------------------------------------------------------------------------------*/
static int __init mstar_pm_init(void)
{
    /* set operation function of suspend */
	suspend_set_ops(&mstar_pm_ops);
	return 0;
}

__initcall(mstar_pm_init);

