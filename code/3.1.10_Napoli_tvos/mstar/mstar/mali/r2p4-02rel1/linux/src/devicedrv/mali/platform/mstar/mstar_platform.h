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
 * @file mstar_platform.h
 * MStar platform specific driver functions
 */

#include "mali_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief MStar platform specific setup and initialisation function
 *
 * @return _MALI_OSK_ERR_OK on success otherwise, a suitable _mali_osk_errcode_t error.
 */
void mstar_platform_init(void);

/** @brief MStar platform specific deinitialisation function
 *
 * @return _MALI_OSK_ERR_OK on success otherwise, a suitable _mali_osk_errcode_t error.
 */
void mstar_platform_deinit(void);

/** @brief MStar platform specific power mode changing function
 *
 */
void mstar_platform_power_mode_change(mali_power_mode power_mode);

/** @brief MStar platform power on function
 *
 */
void mstar_platform_power_on(void);

/** @brief MStar platform power off function
 *
 */
void mstar_platform_power_off(void);

/** @brief MStar platform light sleep function
 *
 */
void mstar_platform_light_sleep(void);

/** @brief MStar platform deep sleep function
 *
 */
void mstar_platform_deep_sleep(void);

#ifdef MSTAR_GLADIUS
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

/** @brief MStar platform GPU clock function
 *
 */
void mstar_pm_set_gpu_clock(mstar_pm_gpu_clk clock)

#endif /* CONFIG_PM */
#endif /* USING_MALI_PMM */
#endif /* MSTAR_GLADIUS */

#ifdef __cplusplus
}
#endif
