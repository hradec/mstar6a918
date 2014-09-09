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
#include <linux/clocksource.h>

#include <asm/sched_clock.h>
#include <plat/sched_clock.h>

extern struct clocksource clocksource_timer;
//static DEFINE_CLOCK_DATA(cd);
static void __iomem *ctr;

//clocks_calc_mult_shift(m, s, 450MHz, NSEC_PER_SEC, 8)
#if 0
#define SC_MULT		2386092942u
#define SC_SHIFT	30
#else
u32 SC_MULT=0;
u32 SC_SHIFT=0;
#endif

static u32 notrace mstar_sched_clock(void)
{
	return clocksource_cyc2ns(clocksource_timer.read(&clocksource_timer), SC_MULT, SC_SHIFT);
}

void __init mstar_sched_clock_init(void __iomem *reg, unsigned long rate)
{
//printk("SC_MULT  = %u  ,SC_SHIFT  = %u \n",SC_MULT,SC_SHIFT);
	ctr = reg;
	setup_sched_clock(mstar_sched_clock, 32, rate);
}
