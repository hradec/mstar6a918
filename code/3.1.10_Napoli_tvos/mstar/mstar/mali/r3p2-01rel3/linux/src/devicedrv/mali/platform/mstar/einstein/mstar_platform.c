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
 * MStar platform specific driver functions for Nike
 */
#include "mstar/mstar_platform.h"
#include <linux/delay.h>

#define REG_CLKGEN1_BASE      0x103300
#define REG_CKG_GPU           0x20
#define DISABLE_CLK_GPU       0x0001

#define REG_G3D_BASE          0x110800
#define REG_SOFT_RESET        0x00
#define BITMASK_SOFT_RESET    0x0001
#define REG_GPUPLL_CTRL0_LOW  0x44
#define REG_GPUPLL_CTRL0_HIGH 0x45
#define REG_GPUPLL_CTRL1_LOW  0x46
#define REG_GPUPLL_CTRL1_HIGH 0x47
#define REG_SPLIT_2CH_MD      0x60
#define READ_OUTSTANDING      0x0002
#define REG_FIFO_DEPTH        0x61
#define SHIFT_FIFO_DEPTH      0x6
#define MASK_FIFO_DEPTH       (0x3 << SHIFT_FIFO_DEPTH)
#define REG_G3D_RREQ          0x62
#define REG_G3D_RREQ_THRD     0x000f
#define REG_G3D_WREQ          0x63
#define REG_G3D_WREQ_THRD     0x000f
#define REG_RIU_APB_EN        0x6a
#define BITMASK_RIU_APB_EN    0x0001
#define REG_MIU1_BASE_LOW     0x77
#define REG_MIU1_BASE_HIGH    0x78

/* RIU */
#define RIU_MAP               0xfd000000
#define RIU                   ((volatile unsigned short*)RIU_MAP)
#define CLKGEN1_REG(addr)     RIU[REG_CLKGEN1_BASE + ((addr) << 1)]
#define G3D_REG(addr)         RIU[REG_G3D_BASE + ((addr) << 1)]

/* GPU clock */
extern int mali_gpu_clock;

/* platform functions */
void mstar_platform_init(void)
{
    mali_gpu_clock = GPU_CLOCK/12*12;
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
    /* set GPU clock: must before power on */
    G3D_REG(REG_GPUPLL_CTRL0_LOW)  = 0x0000;
    G3D_REG(REG_GPUPLL_CTRL0_HIGH) = 0x0000;
    G3D_REG(REG_GPUPLL_CTRL1_LOW)  = GPU_CLOCK/12;
    G3D_REG(REG_GPUPLL_CTRL1_HIGH) = 0x0009;
    udelay(100);

    /* enable read by outstanding order*/
    G3D_REG(REG_SPLIT_2CH_MD) |= READ_OUTSTANDING;

    /* reg_g3d_rreq_thrd = 0x0 */
    G3D_REG(REG_G3D_RREQ) &= ~REG_G3D_RREQ_THRD;

    /* reg_g3d_wreq_thrd = 0x0 */
    G3D_REG(REG_G3D_WREQ) &= ~REG_G3D_WREQ_THRD;

    /* Set MIU1 base address */
    G3D_REG(REG_MIU1_BASE_LOW)  = MIU1_PHY_BASE_ADDR_LOW;
    G3D_REG(REG_MIU1_BASE_HIGH) = MIU1_PHY_BASE_ADDR_HIGH;
    udelay(100);

    /* enable RIU access */
#ifdef MSTAR_RIU_ENABLED
    G3D_REG(REG_RIU_APB_EN) |= BITMASK_RIU_APB_EN;
    udelay(100);
#endif

    /* For improve performance, set FIFO depth to 48                   */
    /* Actually, inside the hardware design, the FIFO depth is only 32 */
    G3D_REG(REG_FIFO_DEPTH) = (G3D_REG(REG_FIFO_DEPTH) & ~MASK_FIFO_DEPTH)
        | ((0x2 << SHIFT_FIFO_DEPTH) & MASK_FIFO_DEPTH);
    udelay(100);

    /* disable GPU clock gating */
    CLKGEN1_REG(REG_CKG_GPU) &= ~DISABLE_CLK_GPU;
    udelay(100);

    /* reset mali */
    G3D_REG(REG_SOFT_RESET) &= ~BITMASK_SOFT_RESET;
    G3D_REG(REG_SOFT_RESET) |= BITMASK_SOFT_RESET;
    udelay(100); /*delay for run-time suspend*/
    G3D_REG(REG_SOFT_RESET) &= ~BITMASK_SOFT_RESET;
    udelay(100);
}

void mstar_platform_power_off(void)
{
    /* enable GPU clock gating */
    CLKGEN1_REG(REG_CKG_GPU) |= DISABLE_CLK_GPU;
    udelay(100);
}

void mstar_platform_light_sleep(void)
{
    mstar_platform_power_off(); /* just power off */
}

void mstar_platform_deep_sleep(void)
{
    mstar_platform_power_off(); /* just power off */
}
