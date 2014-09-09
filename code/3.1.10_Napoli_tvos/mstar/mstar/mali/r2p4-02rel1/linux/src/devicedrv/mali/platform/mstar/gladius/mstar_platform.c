/*
 * Copyright (C) 2010-2012 MStar Semiconductor, Inc. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file mstar_platform.c
 * MStar platform specific driver functions for Gladius
 */
#include "mstar/mstar_platform.h"
#include <linux/delay.h>
#include <linux/platform_device.h>
#include "hal_cmu_mmp.h"
#include "hal_pmu.h"
#include "drv_clkgen_cmu_api.h"


/* RIU */
#define RIU_MAP             0xE5006800
#define RIU                 ((volatile unsigned int*)RIU_MAP)

#define G3D_REG(addr)       RIU[(addr)]


/* platform functions */
void mstar_platform_init(void)
{
    mstar_platform_power_on();
}

void mstar_platform_deinit(void)
{
    mstar_platform_power_off();
}

void mstar_platform_power_mode_change(mali_power_mode power_mode)
{
    switch (power_mode)
    {
    case MALI_POWER_MODE_ON:
        mstar_platform_power_on();
        break;

    case MALI_POWER_MODE_LIGHT_SLEEP:
        mstar_platform_light_sleep();
        break;

    case MALI_POWER_MODE_DEEP_SLEEP:
        mstar_platform_deep_sleep();
        break;

    default:
        printk(KERN_ERR "invalid power mode\n");
        break;
    }
}

void mstar_platform_power_on(void)
{
    /* turn on GPU power domain */
    if (HalPmuPowerOnOff(PMU_SW_DOMAIN_13_GPU, 1) < 0)
    {
        printk(KERN_ERR "failed to turn on GPU power domain\n");
        return;
    }

    udelay(100);

    if (DrvClkgenSetClk(CMU_GPU_CLK_GPU, CMU_CLK_ON) < 0)
    {
        printk(KERN_ERR "DrvClkgenSetClk() failed\n");
        return;
    }

    if (DrvClkgenSetClk(CMU_TOP_CLK_MIU_GPU, CMU_CLK_ON) < 0)
    {
        printk(KERN_ERR "DrvClkgenSetClk() failed\n");
        return;
    }

    /* switch clock source to 256MHz
     * clock can be one of these: CMU_GPU_CLK_32K, CMU_GPU_CLK_85P3M, CMU_GPU_CLK_109P6M,
     *                            CMU_GPU_CLK_128M, CMU_GPU_CLK_153P6M, CMU_GPU_CLK_170P6M,
     *                            CMU_GPU_CLK_192M, CMU_GPU_CLK_219P4M, CMU_GPU_CLK_256M
     */
    if (DrvClkgenSetClk(CMU_GPU_CLK_GPU, CMU_GPU_CLK_256M) < 0)
    {
        printk(KERN_ERR "DrvClkgenSetClk() failed\n");
        return;
    }

    udelay(100);

    /* set clock */
    mstar_pm_set_gpu_clock(MSTAR_PM_GPU_CLK_256M);

    /* reset mali */
    G3D_REG(0x62) &= ~0x1;
    G3D_REG(0x63) &= ~0x1;
    G3D_REG(0x6A) = 0x1;

    G3D_REG(0) = 0x1;
    G3D_REG(0) = 0x0;

    udelay(100);
}

void mstar_platform_power_off(void)
{
    if (DrvClkgenSetClk(CMU_GPU_CLK_GPU, CMU_CLK_OFF) < 0)
    {
        printk(KERN_ERR "DrvClkgenSetClk() failed\n");
        return;
    }

    if (DrvClkgenSetClk(CMU_TOP_CLK_MIU_GPU, CMU_CLK_OFF) < 0)
    {
        printk(KERN_ERR "DrvClkgenSetClk() failed\n");
        return;
    }

    /* turn off GPU power domain */
    if (HalPmuPowerOnOff(PMU_SW_DOMAIN_13_GPU, 0) < 0)
    {
        printk(KERN_ERR "faled to turn off GPU power domain\n");
        return;
    }
}

void mstar_platform_light_sleep(void)
{
    mstar_platform_power_off(); /* just power off */
}

void mstar_platform_deep_sleep(void)
{
    mstar_platform_power_off(); /* just power off */
}


#if USING_MALI_PMM
#ifdef CONFIG_PM

void mstar_pm_set_gpu_clock(mstar_pm_gpu_clk clock)
{
    switch (clock)
    {
    case MSTAR_PM_GPU_CLK_8M:
        if (DrvClkgenSetClk(CMU_GPU_CLK_GPU, CMU_OP_DIVN(1)) < 0)
        {
            printk(KERN_ERR "DrvClkgenSetClk() failed\n");
            return;
        }
        break;

    case MSTAR_PM_GPU_CLK_64M:
        if (DrvClkgenSetClk(CMU_GPU_CLK_GPU, CMU_OP_DIVN(8)) < 0)
        {
            printk(KERN_ERR "DrvClkgenSetClk() failed\n");
            return;
        }
        break;

    case MSTAR_PM_GPU_CLK_128M:
        if (DrvClkgenSetClk(CMU_GPU_CLK_GPU, CMU_OP_DIVN(16)) < 0)
        {
            printk(KERN_ERR "DrvClkgenSetClk() failed\n");
            return;
        }
        break;

    case MSTAR_PM_GPU_CLK_256M:
        if (DrvClkgenSetClk(CMU_GPU_CLK_GPU, CMU_OP_DIVN(32)) < 0)
        {
            printk(KERN_ERR "DrvClkgenSetClk() failed\n");
            return;
        }
        break;

    default:
        printk(KERN_ERR "unsupported clock\n");
        break;
    }
}

#endif /* CONFIG_PM */
#endif /* USING_MALI_PMM */
