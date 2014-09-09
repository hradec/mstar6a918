////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    mdrv_trustzone.c
/// @brief  Trustzone Control Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <linux/autoconf.h>
//#include <linux/undefconf.h> //unused header file now 
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>  //added
#include <linux/timer.h> //added
#include <linux/device.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/types.h>
#include <asm/cacheflush.h>


#include "mdrv_types.h"
#include "mst_platform.h"
#include "mdrv_system.h"
#include "chip_setup.h"
#include <linux/kthread.h>
/*
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include "mdrv_trustzone.h"
//#include "trustzone_llv_control.h"
#include <linux/kdev_t.h>
#include "mst_devid.h"
*/

#include "mst_devid.h"
#include "mdrv_trustzone.h"
#include "mdrv_smc.h"

#define TZ_DRV_NW_VER_MAIN 1
#define TZ_DRV_NW_VER_SUB  0
#define MOD_TZ_NAME             "tz_mod"
#define MOD_TZ_DEVICE_COUNT 1
#define MAX_REGS								15

#define TZ_DPRINTK(fmt, args...) printk(KERN_WARNING"[TrustZone] %s:%d " fmt,__FUNCTION__,__LINE__,## args)
//#define TZ_DPRINTK(fmt, args...)

static int tz_ver_main = TZ_DRV_NW_VER_MAIN;
static int tz_ver_sub  = TZ_DRV_NW_VER_SUB;

static struct class *tz_class;
static struct task_struct *tz_task;

static DECLARE_WAIT_QUEUE_HEAD(tz_wait);
struct smc_struct *smc;

/*
struct tz_drv_struct {
    TZ_SMC_CLASS	tz_op_class;
    void		*tz_op;
    int			tz_op_number; 
    char[128]		tz_class_name;
};
*/

typedef  struct
{
	  unsigned int u32NW_Ver_Main;
	  unsigned int u32NW_Ver_Sub;
	  //unsigned int u32Cpu_Regs[TZ_LLV_REGS_MAX];
	  void *   hook;
	  
}TZ_FileData;

typedef struct
{
    int                         s32TZMajor;
    int                         s32TZMinor;
    void*                       dmaBuf;
    struct cdev                 cDevice;
    struct file_operations      TZFop;
} TZModHandle;

static int                      _MDrv_TZ_Open (struct inode *inode, struct file *filp);
static int                      _MDrv_TZ_Release(struct inode *inode, struct file *filp);
static int                      _MDrv_TZ_MMap(struct file *filp, struct vm_area_struct *vma);
static long                     _MDrv_TZ_Ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

static TZModHandle TZDev=
{
    .s32TZMajor=               MDRV_MAJOR_TZ,
    .s32TZMinor=               MDRV_MINOR_TZ,
    .cDevice=
    {
        .kobj=                  {.name= MOD_TZ_NAME, },
        .owner  =               THIS_MODULE,
    },
    .TZFop=
    {
        .open=                  _MDrv_TZ_Open,
        .release=               _MDrv_TZ_Release,
        .mmap=                  _MDrv_TZ_MMap,
        .unlocked_ioctl =  	_MDrv_TZ_Ioctl,
    },
};


static int _MDrv_TZ_Open(struct inode *inode, struct file *filp)
{
    TZ_FileData *tzData;
    
    tzData = kzalloc(sizeof(*tzData), GFP_KERNEL);
    if (tzData == NULL)
          return -ENOMEM;

    filp->private_data = tzData;
    
    return 0;
}

static int _MDrv_TZ_Release(struct inode *inode, struct file *filp)
{
    TZ_FileData *tzData = filp->private_data ;
    kfree(tzData);

    return 0;
}

static int _MDrv_TZ_MMap(struct file *filp, struct vm_area_struct *vma)
{
    // Implement it later
	  return 0;
}

static long _MDrv_TZ_Ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	  int         err= 0;
    int         ret= 0;

    //TZ_FileData *tzData = filp->private_data ;
	  
	  /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if (TZ_IOC_MAGIC!= _IOC_TYPE(cmd))
    {
        return -ENOTTY;
    }

    /*
     * the direction is a bitmask, and VERIFY_WRITE catches R/W
     * transfers. `Type' is user-oriented, while
     * access_ok is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }
    
    if (err)
    {
    	  TZ_DPRINTK("TZ err\n");
        return -EFAULT;
    }
    
    switch(cmd)
    {
        case TZ_IOC_INFO:
        {
            DrvTZ_Info_t i ;
            i.u32NW_Ver_Main = tz_ver_main;
            i.u32NW_Ver_Sub = tz_ver_sub;
            
            ret = copy_to_user( (void __user *)arg, &i, sizeof(i) );   
        }
        break;
	    case TZ_IOC_Register_Class:
	    {
              // Implement it later.
	    }
	    break;
	    case TZ_IOC_Call:
	    {
                DrvTZ_Args_t i;
		int cpu;
 
                cpu = smp_processor_id();
                if(cpu ==0)
                {
                    TZ_DPRINTK("call from cpu0, reject!\n");
                    break;    
                }                     
                ret= copy_from_user( &i, (void __user *)arg, sizeof(i) );
		//Tz_LLV_Ctrl_SMC(&i.u32Args[0], &i.u32Args[1], 
                              // &i.u32Args[2], &i.u32Args[3]);
		ret = copy_to_user((void __user *)arg, &i, sizeof(i));
	     }
	     break;
	     default:
	        TZ_DPRINTK("Unknown ioctl command %d\n", cmd);
		return -ENOTTY;
	     }
    return 0;
    	  
}

static void __smc_wrapper(void)
{
    unsigned int cpu = smp_processor_id();

    TZ_DPRINTK("[TZ] cpu = %d before enter secure world cmd1 = %x, cmd2 = %x\n", cpu, smc->cmd1, smc->cmd2);

    if(cpu ==1)
    {
       smc_call(smc);
    }
    else
    {
       TZ_DPRINTK("[[Error SMC call CPU = %d!!\n", cpu);
    }

    TZ_DPRINTK("[TZ] cpu = %d before leave secure world \ncmd1 = %x, cmd2 = %x\n", cpu, smc->cmd1, smc->cmd2);

}


static int __smc_thread(void * w)
{
    TZ_DPRINTK("SMC thread start!!\n\n");

    while(1)
    {
       TZ_DPRINTK("SEC heart beat!!\n");

       wait_event_interruptible(tz_wait, smc->smc_flag != 0);

       down_write(&smc->smc_lock);
       smc->smc_flag = 0;
       __smc_wrapper();
       up_write(&smc->smc_lock);
    }

    return 0;
}

int tz_call(struct tz_struct *tz)
{
    down_write(&smc->smc_lock);
    smc->smc_flag = 1;
    smc->cmd1 =0xdd;
    smc->cmd2 =0xcc;
    up_write(&smc->smc_lock);
    wake_up_interruptible(&tz_wait);

    return 0;
}

int __init mod_TZ_DRV_Init(void)
{
    int s32Ret;
    dev_t dev;

    TZ_DPRINTK("TZ support Ver.1\n");

    smc = kmalloc(sizeof(struct smc_struct), GFP_KERNEL);
    if(!smc)
    {
        TZ_DPRINTK("smc malloc fail!!\n");
        return -1;
    }

    init_rwsem(&smc->smc_lock);

    tz_class = class_create(THIS_MODULE, "trustzone");

    if (IS_ERR(tz_class))
    {
        return PTR_ERR(tz_class);
    }

    if (TZDev.s32TZMajor)
    {
        dev = MKDEV(TZDev.s32TZMajor, TZDev.s32TZMinor);
        s32Ret = register_chrdev_region(dev, MOD_TZ_DEVICE_COUNT, MOD_TZ_NAME);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, TZDev.s32TZMinor, MOD_TZ_DEVICE_COUNT, MOD_TZ_NAME);
        TZDev.s32TZMajor = MAJOR(dev);
    }

    if ( 0 > s32Ret)
    {
        TZ_DPRINTK("Unable to get major %d\n", TZDev.s32TZMajor);
        class_destroy(tz_class);
        return s32Ret;
    }

    cdev_init(&TZDev.cDevice, &TZDev.TZFop);
    if (0!= (s32Ret= cdev_add(&TZDev.cDevice, dev, MOD_TZ_DEVICE_COUNT)))
    {
        TZ_DPRINTK("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_TZ_DEVICE_COUNT);
        class_destroy(tz_class);
        return s32Ret;
    }

    device_create(tz_class, NULL, dev, NULL, MOD_TZ_NAME);

    down_write(&smc->smc_lock);
    smc->smc_flag = 1;
    smc->cmd1 = 0xdeadbee1;
    smc->cmd2 = 0xdeadbee2;
    up_write(&smc->smc_lock);

    tz_task = kthread_create(__smc_thread, NULL, "SMC handle");

    kthread_bind(tz_task, 1); //bind the thread on processor 1

    if (!IS_ERR(tz_task))
        wake_up_process(tz_task);
    else
    {
        TZ_DPRINTK("smc fail !!\n");
        return -1;
    }
    
    return 0;	
}

void __exit mod_TZ_DRV_Exit(void)
{
    cdev_del(&TZDev.cDevice);
    unregister_chrdev_region(MKDEV(TZDev.s32TZMajor, TZDev.s32TZMinor), MOD_TZ_DEVICE_COUNT);

    device_destroy(tz_class, MKDEV(TZDev.s32TZMajor, TZDev.s32TZMinor));
    class_destroy(tz_class);	
    kfree(smc);
}

module_init(mod_TZ_DRV_Init);
module_exit(mod_TZ_DRV_Exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("TZ driver");
MODULE_LICENSE("GPL");
