/*
 * Copyright (C) 2010-2011 ARM Limited. All rights reserved.
 * 
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 * 
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file mali_platform.c
 * Platform specific Mali driver functions for a default platform
 */
#include "mali_kernel_common.h"
#include "mali_osk.h"
#include "mali_platform.h"

#ifdef MSTAR
#include "mstar/mstar_platform.h"
#endif


_mali_osk_errcode_t mali_platform_init(void)
{
    /* MStar platform initialization */
#ifdef MSTAR
    mstar_platform_init();
#endif

    MALI_SUCCESS;
}

_mali_osk_errcode_t mali_platform_deinit(void)
{
    /* MStar platform deinitialization */
#ifdef MSTAR
    mstar_platform_deinit();
#endif

    MALI_SUCCESS;
}

_mali_osk_errcode_t mali_platform_power_mode_change(mali_power_mode power_mode)
{
	MALI_DEBUG_PRINT(4, ("Power mode change to = %d\n", power_mode));

#ifdef MSTAR
#if USING_MALI_PMM
#ifdef CONFIG_PM
	mstar_pm_power_mode_change(power_mode);
#endif
#endif
#endif

    MALI_SUCCESS;
}

void mali_gpu_utilization_handler(u32 utilization)
{
}

void set_mali_parent_power_domain(void* dev)
{
}


