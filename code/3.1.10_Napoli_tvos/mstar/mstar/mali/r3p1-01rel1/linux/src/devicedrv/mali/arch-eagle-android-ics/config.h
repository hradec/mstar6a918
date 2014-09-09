/*
 * Copyright (C) 2010-2012 MStar Semiconductor, Inc. All rights reserved.
 * 
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 * 
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __ARCH_CONFIG_H__
#define __ARCH_CONFIG_H__

/* Configuration for Eagle Android ICS platform */

#ifdef MSTAR_RIU_ENABLED
#define MALI_CORE_BASE(offset)  (0xfd280000 + (offset << 1))
#else
#define MALI_CORE_BASE(offset)  (0x1f800000 + (offset))
#endif

static _mali_osk_resource_t arch_configuration [] =
{
	{
		.type = MALI400GP,
		.description = "Mali-400 GP",
		.base = MALI_CORE_BASE(0x0000),
		.irq = MALI_IRQ,
		.mmu_id = 1
	},
	{
		.type = MALI400PP,
		.base = MALI_CORE_BASE(0x8000),
		.irq = MALI_IRQ,
		.description = "Mali-400 PP 0",
		.mmu_id = 2
	},
	{
		.type = MALI400PP,
		.base = MALI_CORE_BASE(0xA000),
		.irq = MALI_IRQ,
		.description = "Mali-400 PP 1",
		.mmu_id = 3
	},
	{
		.type = MMU,
		.base = MALI_CORE_BASE(0x3000),
		.irq = MALI_IRQ,
		.description = "Mali-400 MMU for GP",
		.mmu_id = 1
	},
	{
		.type = MMU,
		.base = MALI_CORE_BASE(0x4000),
		.irq = MALI_IRQ,
		.description = "Mali-400 MMU for PP 0",
		.mmu_id = 2
	},
	{
		.type = MMU,
		.base = MALI_CORE_BASE(0x5000),
		.irq = MALI_IRQ,
		.description = "Mali-400 MMU for PP 1",
		.mmu_id = 3
	},
	{
		.type = MALI400L2,
		.base = MALI_CORE_BASE(0x1000),
		.description = "Mali-400 L2 cache"
	},
	{
		.type = OS_MEMORY,
		.description = "OS Memory",
		.size = 0x20000000,
		.cpu_usage_adjust = PHYS_TO_BUS_ADDRESS_ADJUST,
		.flags = _MALI_CPU_WRITEABLE | _MALI_CPU_READABLE | _MALI_PP_READABLE | _MALI_PP_WRITEABLE |_MALI_GP_READABLE | _MALI_GP_WRITEABLE
	},
	{
		.type = MEM_VALIDATION,
		.description = "Framebuffer",
		.base = 0x00000000,
		.size = 0xf0000000,
		.flags = _MALI_CPU_WRITEABLE | _MALI_CPU_READABLE | _MALI_PP_WRITEABLE | _MALI_PP_READABLE
	}
};

#endif /* __ARCH_CONFIG_H__ */
