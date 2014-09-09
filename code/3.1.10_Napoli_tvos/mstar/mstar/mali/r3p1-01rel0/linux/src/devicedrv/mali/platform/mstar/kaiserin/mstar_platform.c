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
 * MStar platform specific driver functions for Kaiserin
 */
#include "mstar/mstar_platform.h"
#ifdef MSOS
#include "mali_osk.h"
#else
#include <linux/delay.h>
#endif


/* RIU */
#define RIU_MAP             0xbf200000
#define RIU                 ((volatile unsigned short*)RIU_MAP)

#define REG_CLKGEN1_BASE    (0x3300)
#define CLKGEN1_REG(addr)   RIU[((addr) << 1) + REG_CLKGEN1_BASE]

#define REG_G3D_BASE        (0x20300)
#define G3D_REG(addr)       RIU[((addr) << 1) + REG_G3D_BASE]

#define REG_MIU_BASE        (0x1200)
#define MIU_REG(addr)       RIU[((addr) << 1) + REG_MIU_BASE]

#define REG_MIU2_BASE        (0x600)
#define MIU2_REG(addr)       RIU[((addr) << 1) + REG_MIU2_BASE]

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
        /* invalid power mode */
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
    _mali_osk_time_ubusydelay(100);

    /* reg_g3d_rreq_thrd=0 */
    G3D_REG(0x62) &= ~0xF;

    /* reg_g3d_wreq_thrd=0x0 */
    G3D_REG(0x63) &= ~0xF;
    _mali_osk_time_ubusydelay(100);

    /* enable RIU access */
#ifdef MSTAR_RIU_ENABLED
    G3D_REG(0x6A) = 0x1;
    _mali_osk_time_ubusydelay(100);
#endif

    /* disable clock gating: reg_ckg_gpu=0 */
    CLKGEN1_REG(0x31) = 0;
    _mali_osk_time_ubusydelay(100);

    /* power on mali */
    G3D_REG(0x44) = (0x0080);
    G3D_REG(0x45) = (0x00C0);   /* 1108_45[1] = GPU_POWER_DOWN */
    _mali_osk_time_ubusydelay(100);

    /* reset mali */
    G3D_REG(0x0) = 0x0;
    G3D_REG(0x0) = 0x1;
    G3D_REG(0x0) = 0x0;
    _mali_osk_time_ubusydelay(100);
}

void mstar_platform_power_off(void)
{
    /* enable clock gating: reg_ckg_gpu=1 */
    CLKGEN1_REG(0x31) = 1;
    _mali_osk_time_ubusydelay(100);
}

void mstar_platform_light_sleep(void)
{
    mstar_platform_power_off(); /* just power off */
}

void mstar_platform_deep_sleep(void)
{
    mstar_platform_power_off(); /* just power off */
}
