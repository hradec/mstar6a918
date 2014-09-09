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
 * @file mstar_platform.h
 * MStar platform specific driver functions
 */

#include "mali_osk.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief MStar platform specific setup and initialisation
 *
 * @return _MALI_OSK_ERR_OK on success otherwise, a suitable _mali_osk_errcode_t error.
 */
void mstar_platform_init(void);

/** @brief MStar platform specific deinitialisation
 *
 * @return _MALI_OSK_ERR_OK on success otherwise, a suitable _mali_osk_errcode_t error.
 */
void mstar_platform_deinit(void);

#if USING_MALI_PMM
#ifdef CONFIG_PM

/** @brief GPU clock
 *
 */
typedef enum mstar_pm_gpu_clk
{
	MSTAR_PM_GPU_CLK_8M,
	MSTAR_PM_GPU_CLK_64M,
	MSTAR_PM_GPU_CLK_128M,
	MSTAR_PM_GPU_CLK_256M,
} mstar_pm_gpu_clk;

/** @brief description of power change reasons
 *
 * this enumeration is the same as mali_power_mode
 */
typedef enum mstar_power_mode
{
	MSTAR_POWER_MODE_ON,
	MSTAR_POWER_MODE_LIGHT_SLEEP,
	MSTAR_POWER_MODE_DEEP_SLEEP,
} mstar_power_mode;

/** @brief MStar platform specific suspend function
 *
 */
void mstar_pm_suspend(void);

/** @brief MStar platform specific resume function
 *
 */
void mstar_pm_resume(void);

/** @brief Setting the GPU clock
 *
 */
void mstar_pm_set_gpu_clock(mstar_pm_gpu_clk clock);

/** @brief MStar platform specific power mode changing function
 *
 */
void mstar_pm_power_mode_change(mstar_power_mode power_mode);

#endif /* CONFIG_PM */
#endif /* USING_MALI_PMM */

#ifdef __cplusplus
}
#endif
