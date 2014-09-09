


#include <linux/autoconf.h>
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
#include <linux/time.h>
#include <linux/timer.h>
#include <asm/io.h>
#include <asm/delay.h>

#include "mhal_wdt.h"
#include "mdrv_wdt.h"

void MDrv_WDT_DisableWDT(void)
{
    HAL_WDT_DisableWDT();
}

void MDrv_WDT_SetWDT_MS(S32 s32msec)
{
    HAL_WDT_SetWDT_MS(s32msec);
}

void MDrv_WDT_SetWDTInt(U16 u16Sec)
{
    HAL_WDT_SetWDTInt(u16Sec);
}

void MDrv_WDT_ClearWDT(void)
{
    HAL_WDT_ClearWDT();
}

