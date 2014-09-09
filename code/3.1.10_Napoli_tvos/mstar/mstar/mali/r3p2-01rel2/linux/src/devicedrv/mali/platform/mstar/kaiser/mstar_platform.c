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
 * MStar platform specific driver functions for Amber3
 */
#include "mstar/mstar_platform.h"
#include <linux/delay.h>

/* RIU */
#define RIU_MAP             0xfd200000
#define RIU                 ((volatile unsigned short*)RIU_MAP)

#define REG_CLKGEN0_BASE    0x1e00
#define CLKGEN0_REG(addr)   RIU[((addr) << 1) + REG_CLKGEN0_BASE]

#define REG_CLKGEN1_BASE    0x3300
#define CLKGEN1_REG(addr)   RIU[((addr) << 1) + REG_CLKGEN1_BASE]

#define REG_G3D_BASE        0x21a00
#define G3D_REG(addr)       RIU[((addr) << 1) + REG_G3D_BASE]

/* GPU clock */
extern int mali_gpu_clock;

/* stat_bond2(0x68)[1]                   */
/* 0: GPU single core, 1: GPU dual core  */
unsigned int is_gpu_dual = 1; 


/* platform functions */
void mstar_platform_init(void)
{
#if GPU_CLOCK == 240 || GPU_CLOCK == 192 || GPU_CLOCK == 172 || GPU_CLOCK == 250 || GPU_CLOCK == 216
    mali_gpu_clock = GPU_CLOCK;
#else
    mali_gpu_clock = 288;
#endif
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
    /* reg_gpu_pll_ctrl1 is no used in Kaiser */

    /* enable read by outstanding order: reg_split_2ch_md=1 */
    G3D_REG(0x60) |= 0x2;

    /* reg_g3d_rreq_thrd=0 */
    G3D_REG(0x62) &= ~0xF;

    /* reg_g3d_wreq_thrd=0x0 */
    G3D_REG(0x63) &= ~0xF;

    /* The physical address of MIU1 */
    G3D_REG(0x77) = MIU1_PHY_BASE_ADDR_LOW;
    G3D_REG(0x78) = MIU1_PHY_BASE_ADDR_HIGH;
    udelay(100);

    /* enable RIU access */
#ifdef MSTAR_RIU_ENABLED
    G3D_REG(0x6A) = 0x1;
    udelay(100);
#endif

    /* reg_ckg_gpu[1:0] = 0   ... disable clock gating
     *
     * reg_ckg_gpu[4:2] can switch the clock of gpu
     * reg_ckg_gpu[4:2] = 000 ... 240 Mhz
     *                  = 001 ... 192 Mhz
     *                  = 010 ... 172 Mhz
     *                  = 011 ... miupll
     *                  = 100 ... 250 Mhz
     *                  = 101 ... 216 Mhz
     *                  = 110 ... miupll
     *                  = 111 ... 288 Mhz
     */
#if GPU_CLOCK == 240
    CLKGEN1_REG(0x31) = (0 << 2);
#elif GPU_CLOCK == 192
    CLKGEN1_REG(0x31) = (1 << 2);
#elif GPU_CLOCK == 172
    CLKGEN1_REG(0x31) = (2 << 2);
#elif GPU_CLOCK == 250
    CLKGEN1_REG(0x31) = (4 << 2);
#elif GPU_CLOCK == 216
    CLKGEN1_REG(0x31) = (5 << 2);
#else /* GPU_CLOCK == 288 */
    CLKGEN1_REG(0x31) = (7 << 2);
#endif
    udelay(100);

    /* stat_bond2(0x68)[1]                  */
    /* 0: GPU single core, 1: GPU dual core */
    is_gpu_dual = (CLKGEN0_REG(0x68) >> 1) & 0x1;


    /* power on mali */
    G3D_REG(0x44) = 0x0080;
    G3D_REG(0x45) = 0x00C0;   /* 1108_45[1] = GPU_POWER_DOWN */
    udelay(100);

    /* reset mali */
    G3D_REG(0x0) = 0x0;
    G3D_REG(0x0) = 0x1;
    G3D_REG(0x0) = 0x0;
    udelay(100);
}

void mstar_platform_power_off(void)
{
    /* enable clock gating: reg_ckg_gpu=1 */
    CLKGEN1_REG(0x31) = 1;
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
