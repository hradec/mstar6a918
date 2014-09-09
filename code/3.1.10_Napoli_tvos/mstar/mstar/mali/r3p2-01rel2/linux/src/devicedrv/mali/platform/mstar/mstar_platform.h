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

/**
 *         Bus base addr       --> Physical base addr
 *
 * MIU0    MSTAR_MIU0_BUS_BASE --> 0x00000000
 *
 * MIU1    MSTAR_MIU1_BUS_BASE --> (MSTAR_MIU1_BUS_BASE - MSTAR_MIU0_BUS_BASE)
 */
#ifndef MSTAR_MIU1_PHY_BASE
#include "mstar/mstar_chip.h"
#define MSTAR_MIU1_PHY_BASE       (MSTAR_MIU1_BUS_BASE-MSTAR_MIU0_BUS_BASE)
#endif

#define MIU1_PHY_BASE_ADDR_LOW    ( MSTAR_MIU1_PHY_BASE        & 0xffff)
#define MIU1_PHY_BASE_ADDR_HIGH   ((MSTAR_MIU1_PHY_BASE >> 16) & 0xffff)

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

#ifdef __cplusplus
}
#endif
