/*
 * Copyright (C) 2011 MStar Semiconductor, Inc. All rights reserved.
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
#include <linux/delay.h>


/* RIU */
#define RIU_MAP             0xbf200000
#define RIU                 ((volatile unsigned short*)RIU_MAP)

#define REG_CLKGEN1_BASE    (0x3300)
#define CLKGEN1_REG(addr)   RIU[((addr) << 1) + REG_CLKGEN1_BASE]

#define REG_G3D_BASE        (0x20300)
#define G3D_REG(addr)       RIU[((addr) << 1) + REG_G3D_BASE]


/* platform functions */
void mstar_platform_init(void)
{
    /* reg_ckg_gpu=0 (disable clock gating) */
    CLKGEN1_REG(0x31) = 0;

    udelay(100);

    /* GPU_CLOCK (MHz): 231, 252, 276, 300, 306, 360, ...
     *
     * reg_gpupll_ctrl1[7:0] must be between 0x2A and 0x64.
     */
#ifndef GPU_CLOCK
    #define GPU_CLOCK 231
#endif

    /* reg_gpupll_ctl[01] (GPU_CLOCK) */
    G3D_REG(0x44) = (0x0080);
    G3D_REG(0x45) = (0x00C0);
    G3D_REG(0x46) = ((GPU_CLOCK >= 306)
                        ? (0x0000 | ((GPU_CLOCK/6) & 0xff))
                        : (0x0400 | ((GPU_CLOCK/3) & 0xff)));
    G3D_REG(0x47) = (0x0000);

    udelay(100);

    /* reg_g3d_[rw]req_thrd=0 */
    G3D_REG(0x62) &= ~0xF;
    G3D_REG(0x63) &= ~0xF;

    /* reset mali */
    G3D_REG(0x0) = 0x0;
    G3D_REG(0x0) = 0x1;
    G3D_REG(0x0) = 0x0;

#ifdef MSTAR_RIU_ENABLED
    G3D_REG(0x6A) = 0x1;
#endif

    udelay(100);
}

void mstar_platform_deinit(void)
{    
    /* reg_ckg_gpu=1 (enable clock gating) */
    CLKGEN1_REG(0x31) = 1;
}
