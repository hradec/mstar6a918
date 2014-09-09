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
 * MStar platform specific driver functions for Edison
 */
#include "mstar/mstar_platform.h"
#include <linux/delay.h>


/* RIU */
#define RIU_MAP             0xfd200000
#define RIU                 ((volatile unsigned short*)RIU_MAP)

#define REG_CLKGEN1_BASE    (0x3300)
#define CLKGEN1_REG(addr)   RIU[((addr) << 1) + REG_CLKGEN1_BASE]

#define REG_G3D_BASE        (0x10800)
#define G3D_REG(addr)       RIU[((addr) << 1) + REG_G3D_BASE]


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
    /* set GPU clock: must before power on
     *
     * GPU_CLOCK (MHz): 231, 252, 276, 300, 306, 360, ...
     *
     * reg_gpupll_ctrl1[7:0] must be between 0x2A and 0x64.
     */
    G3D_REG(0x46) = (GPU_CLOCK >= 306) ?
                        (0x0000 | ((GPU_CLOCK/6) & 0xff)) :
                        (0x0400 | ((GPU_CLOCK/3) & 0xff));
    G3D_REG(0x47) = (0x0000);
    udelay(100);

    /* enable read by outstanding order: reg_split_2ch_md=1 */
    G3D_REG(0x60) |= 0x2;

    /* reg_g3d_rreq_thrd=0 */
    G3D_REG(0x62) &= ~0xF;

    /* reg_g3d_wreq_thrd=0x0 */
    G3D_REG(0x63) &= ~0xF;

    /* The physical address of MIU1 is 0x60000000
     *
     * reg_miu1_base=0x60000000
     */
    G3D_REG(0x77) = 0x0000;
    G3D_REG(0x78) = 0x6000;
    udelay(100);

    /* enable RIU access */
#ifdef MSTAR_RIU_ENABLED
    G3D_REG(0x6A) = 0x1;
    udelay(100);
#endif

    /* disable clock gating: reg_ckg_gpu=0 */
    CLKGEN1_REG(0x20) = 0;
    udelay(100);

    /* power on mali */
    G3D_REG(0x44) = (0x0080);
    G3D_REG(0x45) = (0x00C0);   /* 1108_45[1] = GPU_POWER_DOWN */
    udelay(100);

    /* reset mali */
    G3D_REG(0x0) = 0x0;
    udelay(100);
    G3D_REG(0x0) = 0x1;
    udelay(100);
    G3D_REG(0x0) = 0x0;
    udelay(100);
}

void mstar_platform_power_off(void)
{
    /* enable clock gating: reg_ckg_gpu=1 */
    CLKGEN1_REG(0x20) = 1;
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
