/*
 *  linux/arch/arm/arm-boards/plat-mstar/sched-clock.c
 *
 *  Copyright (C) 1999 - 2003 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/io.h>
#include <linux/sched.h>

#include <asm/sched_clock.h>
#include <plat/sched_clock.h>


#if defined(CONFIG_MP_CA7_QUAD_CORE_PATCH)


#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/mach/time.h>
#include <mach/hardware.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>

#include <mach/io.h>
#include <mach/timex.h>
#include <asm/irq.h>
#include <linux/timer.h>
#include <plat/localtimer.h>
#include "chip_int.h"

static inline cycle_t arch_counter_get_cntpct(void)
{
	u32 cvall, cvalh;

	asm volatile("mrrc p15, 0, %0, %1, c14" : "=r" (cvall), "=r" (cvalh));

	return ((cycle_t) cvalh << 32) | cvall;
}


static cycle_t arch_counter_read(void)
{
	return arch_counter_get_cntpct();
}
#endif

static DEFINE_CLOCK_DATA(cd);
static void __iomem *ctr;

//clocks_calc_mult_shift(m, s, 450MHz, NSEC_PER_SEC, 8)
#if 0
#define SC_MULT		2386092942u
#define SC_SHIFT	30
#else
u32 SC_MULT=0;
u32 SC_SHIFT=0;
#endif

DEFINE_SCHED_CLOCK_FUNC(mstar_sched_clock)
{

#if defined(CONFIG_MP_CA7_QUAD_CORE_PATCH)
        u32 cyc = arch_counter_read();

        return cyc_to_fixed_sched_clock(&cd, cyc, (u32)~0,
						SC_MULT, SC_SHIFT);

#else        
        
        if (ctr){	
		u32 cyc = readl(ctr);
		return cyc_to_fixed_sched_clock(&cd, cyc, (u32)~0,
						SC_MULT, SC_SHIFT);
	} else
		return 0;
#endif
}

static void notrace mstar_update_sched_clock(void)
{
#if defined(CONFIG_MP_CA7_QUAD_CORE_PATCH)
        u32 cyc = arch_counter_read();
	update_sched_clock(&cd, cyc, (u32)~0);
#else        
	u32 cyc = readl(ctr);
	update_sched_clock(&cd, cyc, (u32)~0);
#endif
}

void __init mstar_sched_clock_init(void __iomem *reg, unsigned long rate)
{
//printk("SC_MULT  = %u  ,SC_SHIFT  = %u \n",SC_MULT,SC_SHIFT);
	ctr = reg;
	init_fixed_arch_sched_clock(&cd, mstar_update_sched_clock,
				    mstar_sched_clock,
				    32, rate, SC_MULT, SC_SHIFT);
}
