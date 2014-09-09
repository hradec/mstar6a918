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
 * @file mali_platform.c
 * Platform specific Mali driver functions for a MStar platform
 */
#ifdef __KERNEL__
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/pm.h>
#ifdef CONFIG_PM_RUNTIME
#include <linux/pm_runtime.h>
#endif
#include <asm/io.h>
#include <linux/mali/mali_utgard.h>
#endif

#ifdef MSOS
#include <msos/mali/mali_utgard.h>
#endif

#include "mali_kernel_common.h"
#include "mali_osk.h"
#include "mali_platform.h"

#include "mstar/mstar_platform.h"

#if USING_GPU_UTILIZATION
unsigned long mali_gpu_utilization = 0;
#endif

_mali_osk_errcode_t mali_platform_init(void)
{
	mstar_platform_init();

	MALI_SUCCESS;
}

_mali_osk_errcode_t mali_platform_deinit(void)
{
	mstar_platform_deinit();

	MALI_SUCCESS;
}

_mali_osk_errcode_t mali_platform_power_mode_change(mali_power_mode power_mode)
{
	MALI_DEBUG_PRINT(4, ("Power mode change to = %d\n", power_mode));

	mstar_platform_power_mode_change(power_mode);

	MALI_SUCCESS;
}

#if USING_GPU_UTILIZATION
void mali_gpu_utilization_handler(unsigned int utilization)
{
	mali_gpu_utilization = utilization;
}
#endif

static struct resource mali_gpu_resources[] =
{
	/* NOTE: all cores share the same IRQ */
#if defined(MALI400)
	#if NUM_PP == 1
		MALI_GPU_RESOURCES(GPU_BASE_ADDRESS, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ)
	#elif NUM_PP == 2
		MALI_GPU_RESOURCES(GPU_BASE_ADDRESS, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ)
	#elif NUM_PP == 3
		MALI_GPU_RESOURCES(GPU_BASE_ADDRESS, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ)
	#elif NUM_PP == 4
		MALI_GPU_RESOURCES(GPU_BASE_ADDRESS, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ)
	#else
		#error invalid NUM_PP for MALI400
	#endif
#elif defined(MALI450)
	#if NUM_PP == 1
		MALI_GPU_RESOURCES(GPU_BASE_ADDRESS, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ)
	#elif NUM_PP == 2
		MALI_GPU_RESOURCES(GPU_BASE_ADDRESS, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ)
	#elif NUM_PP == 4
		MALI_GPU_RESOURCES(GPU_BASE_ADDRESS, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ)
	#elif NUM_PP == 6
		MALI_GPU_RESOURCES(GPU_BASE_ADDRESS, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ)
	#elif NUM_PP == 8
		MALI_GPU_RESOURCES(GPU_BASE_ADDRESS, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ)
	#else
		#error invalid NUM_PP for MALI450
	#endif
#elif defined(MALI200)
	#if NUM_PP != 1
		#error MALI200 has only 1 PP
	#elif USING_PMU
		#error MALI200 has no PMU
	#else
		MALI_GPU_RESOURCES(GPU_BASE_ADDRESS, MALI_IRQ, MALI_IRQ, MALI_IRQ)
	#endif
#elif defined(MALI300)
	#if NUM_PP != 1
		#error MALI300 has only 1 PP
	#else
		MALI_GPU_RESOURCES(GPU_BASE_ADDRESS, MALI_IRQ, MALI_IRQ, MALI_IRQ, MALI_IRQ)
	#endif
#else
	#error unknown GPU_HW
#endif
};

static struct mali_gpu_device_data mali_gpu_data =
{
#if defined(RESOURCE_OS_MEMORY)
	.shared_mem_size = RESOURCE_OS_MEMORY,
#endif
#if defined(RESOURCE_MEMORY_START)
	.dedicated_mem_start = RESOURCE_MEMORY_START,
#endif
#if defined(RESOURCE_MEMORY_SIZE)
	.dedicated_mem_size = RESOURCE_MEMORY_SIZE,
#endif
#if defined(RESOURCE_MEM_VALIDATION_START)
	.fb_start = RESOURCE_MEM_VALIDATION_START,
#endif
#if defined(RESOURCE_MEM_VALIDATION_SIZE)
	.fb_size = RESOURCE_MEM_VALIDATION_SIZE,
#endif
#if USING_GPU_UTILIZATION
    .utilization_handler = mali_gpu_utilization_handler,
#endif
};

#ifndef __KERNEL__

const struct resource* mali_platform_device_get_resource(u32* num_resources)
{
	if (num_resources) *num_resources = sizeof(mali_gpu_resources)/sizeof(mali_gpu_resources[0]);
	return mali_gpu_resources;
}

const struct mali_gpu_device_data* mali_platform_device_get_data()
{
	return &mali_gpu_data;
}

int mali_platform_device_set_data(const struct mali_gpu_device_data* data)
{
	if (data) memcpy(&mali_gpu_data, data, sizeof(struct mali_gpu_device_data));
	return 0;
}

#else /*  __KERNEL__ */

static void mali_platform_device_release(struct device *device);

static struct platform_device mali_gpu_device =
{
	.name = MALI_GPU_NAME_UTGARD,
	.id = 0,
	.dev.release = mali_platform_device_release,
};

int mali_platform_device_register(void)
{
	int err = -1;

	MALI_DEBUG_PRINT(4, ("mali_platform_device_register() called\n"));

	mali_platform_init();

    err = platform_device_add_resources(&mali_gpu_device, mali_gpu_resources, sizeof(mali_gpu_resources) / sizeof(mali_gpu_resources[0]));

	if (0 == err)
	{
		err = platform_device_add_data(&mali_gpu_device, &mali_gpu_data, sizeof(mali_gpu_data));
		if (0 == err)
		{
			/* Register the platform device */
			err = platform_device_register(&mali_gpu_device);
			if (0 == err)
			{
#ifdef CONFIG_PM_RUNTIME
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
				pm_runtime_set_autosuspend_delay(&(mali_gpu_device.dev), 1000);
				pm_runtime_use_autosuspend(&(mali_gpu_device.dev));
#endif
				pm_runtime_enable(&(mali_gpu_device.dev));
#endif

				return 0;
			}
		}

		platform_device_unregister(&mali_gpu_device);
	}

	return err;
}

void mali_platform_device_unregister(void)
{
	MALI_DEBUG_PRINT(4, ("mali_platform_device_unregister() called\n"));

	platform_device_unregister(&mali_gpu_device);
	mali_platform_deinit();
}

static void mali_platform_device_release(struct device *device)
{
	MALI_DEBUG_PRINT(4, ("mali_platform_device_release() called\n"));
}

#endif
