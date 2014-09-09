/*
 *  linux/arch/arm/kernel/smp_twd.c
 *
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/smp.h>
#include <linux/jiffies.h>
#include <linux/clockchips.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/percpu.h>

#include <asm/smp_twd.h>
#include <asm/hardware/gic.h>


#if defined(CONFIG_MP_CA7_QUAD_CORE_PATCH)

#include <generated/autoconf.h>

extern int query_frequency(void);

#define ARCH_TIMER_CTRL_ENABLE		(1 << 0)
#define ARCH_TIMER_CTRL_IT_MASK		(1 << 1)
#define ARCH_TIMER_CTRL_IT_STAT		(1 << 2)

#define ARCH_TIMER_REG_CTRL		    0
#define ARCH_TIMER_REG_FREQ		    1
#define ARCH_TIMER_REG_TVAL		    2

#define CA7_TIMER_FREQ	            ((query_frequency() * 1000000) >> 1)


static inline cycle_t arch_counter_get_cntpct(void)
{
	u32 cvall, cvalh;

	asm volatile("mrrc p15, 0, %0, %1, c14" : "=r" (cvall), "=r" (cvalh));

	return ((cycle_t) cvalh << 32) | cvall;
}

static inline cycle_t arch_counter_get_cntvct(void)
{
	u32 cvall, cvalh;

	asm volatile("mrrc p15, 1, %0, %1, c14" : "=r" (cvall), "=r" (cvalh));

	return ((cycle_t) cvalh << 32) | cvall;
}

static inline void arch_counter_set_cntpcval(void)
{
       u32 cvall=0xffffffff, cvalh=0xffffffff;

       asm volatile("mcrr p15, 2, %0, %1, c14" : : "r" (cvall), "r" (cvalh) );

}

#if 0  //no used for current CA7 porting
static u32 notrace arch_counter_get_cntvct32(void)
{
	cycle_t cntvct = arch_counter_get_cntvct();

	/*
	 * The sched_clock infrastructure only knows about counters
	 * with at most 32bits. Forget about the upper 24 bits for the
	 * time being...
	 */
	return (u32)(cntvct & (u32)~0);
}
static cycle_t arch_counter_read(struct clocksource *cs)
{
	return arch_counter_get_cntpct();
}
#endif

static void arch_timer_reg_write(int reg, u32 val)
{
	switch (reg) {
	case ARCH_TIMER_REG_CTRL:
		asm volatile("mcr p15, 0, %0, c14, c2, 1" : : "r" (val));
		break;
	case ARCH_TIMER_REG_TVAL:
		asm volatile("mcr p15, 0, %0, c14, c2, 0" : : "r" (val));
		break;
	}

	isb();
}

static u32 arch_timer_reg_read(int reg)
{
	u32 val;

	switch (reg) {
	case ARCH_TIMER_REG_CTRL:
		asm volatile("mrc p15, 0, %0, c14, c2, 1" : "=r" (val));
		break;
	case ARCH_TIMER_REG_FREQ:
		asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r" (val));
		break;
	case ARCH_TIMER_REG_TVAL:
		asm volatile("mrc p15, 0, %0, c14, c2, 0" : "=r" (val));
		break;
	default:
		BUG();
	}

	return val;
}

static void arch_timer_disable(void)
{
	unsigned long ctrl;

	ctrl = arch_timer_reg_read(ARCH_TIMER_REG_CTRL);
	ctrl &= ~ARCH_TIMER_CTRL_ENABLE;
	arch_timer_reg_write(ARCH_TIMER_REG_CTRL, ctrl);
}

#endif


/* set up by the platform code */
void __iomem *twd_base;

static struct clk *twd_clk;
static unsigned long twd_timer_rate;
static DEFINE_PER_CPU(struct clock_event_device *, twd_ce);

static void twd_set_mode(enum clock_event_mode mode,
			struct clock_event_device *clk)
{
	unsigned long ctrl = 0;

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:

#if defined(CONFIG_MP_CA7_QUAD_CORE_PATCH)
		ctrl = arch_timer_reg_read(ARCH_TIMER_REG_CTRL);
		ctrl |= ARCH_TIMER_CTRL_ENABLE;
		arch_timer_reg_write(ARCH_TIMER_REG_TVAL,CA7_TIMER_FREQ/HZ );
#else
		/* timer load already set up */
		ctrl = TWD_TIMER_CONTROL_ENABLE | TWD_TIMER_CONTROL_IT_ENABLE
			| TWD_TIMER_CONTROL_PERIODIC;
		__raw_writel(twd_timer_rate / HZ, twd_base + TWD_TIMER_LOAD);
#endif
		break;
	case CLOCK_EVT_MODE_ONESHOT:
#if defined(CONFIG_MP_CA7_QUAD_CORE_PATCH)
               ctrl = arch_timer_reg_read(ARCH_TIMER_REG_CTRL);
               ctrl |= ARCH_TIMER_CTRL_ENABLE;
#else
		/* period set, and timer enabled in 'next_event' hook */
		ctrl = TWD_TIMER_CONTROL_IT_ENABLE | TWD_TIMER_CONTROL_ONESHOT;
#endif
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
#if defined(CONFIG_MP_CA7_QUAD_CORE_PATCH)
		arch_timer_disable();
                return;
#else
		ctrl = 0;
#endif
	}
#if defined(CONFIG_MP_CA7_QUAD_CORE_PATCH)
        arch_timer_reg_write(ARCH_TIMER_REG_CTRL, ctrl);
#else
	__raw_writel(ctrl, twd_base + TWD_TIMER_CONTROL);
#endif


}

static int twd_set_next_event(unsigned long evt,
			struct clock_event_device *unused)
{

#if defined(CONFIG_MP_CA7_QUAD_CORE_PATCH)
	unsigned long ctrl = arch_timer_reg_read(ARCH_TIMER_REG_CTRL);
	ctrl |= ARCH_TIMER_CTRL_ENABLE;
	ctrl &= ~ARCH_TIMER_CTRL_IT_MASK;

	arch_timer_reg_write(ARCH_TIMER_REG_TVAL, evt);
	arch_timer_reg_write(ARCH_TIMER_REG_CTRL, ctrl);

#else
	unsigned long ctrl = __raw_readl(twd_base + TWD_TIMER_CONTROL);

	ctrl |= TWD_TIMER_CONTROL_ENABLE;

	__raw_writel(evt, twd_base + TWD_TIMER_COUNTER);
	__raw_writel(ctrl, twd_base + TWD_TIMER_CONTROL);
#endif
	return 0;

}

/*
 * local_timer_ack: checks for a local timer interrupt.
 *
 * If a local timer interrupt has occurred, acknowledge and return 1.
 * Otherwise, return 0.
 */
int twd_timer_ack(void)
{


#if defined(CONFIG_MP_CA7_QUAD_CORE_PATCH)
	unsigned long ctrl = arch_timer_reg_read(ARCH_TIMER_REG_CTRL);
	if (ctrl & ARCH_TIMER_CTRL_IT_STAT) {
		arch_timer_disable();
		arch_timer_reg_write(ARCH_TIMER_REG_TVAL,CA7_TIMER_FREQ*2);
		return 1;
	}

#else
	if (__raw_readl(twd_base + TWD_TIMER_INTSTAT)) {
		__raw_writel(1, twd_base + TWD_TIMER_INTSTAT);
		return 1;
	}
#endif


	return 0;
}

/*
 * Updates clockevent frequency when the cpu frequency changes.
 * Called on the cpu that is changing frequency with interrupts disabled.
 */
static void twd_update_frequency(void *data)
{
	twd_timer_rate = clk_get_rate(twd_clk);

	clockevents_update_freq(__get_cpu_var(twd_ce), twd_timer_rate);
}

static int twd_cpufreq_transition(struct notifier_block *nb,
	unsigned long state, void *data)
{
	struct cpufreq_freqs *freqs = data;

	/*
	 * The twd clock events must be reprogrammed to account for the new
	 * frequency.  The timer is local to a cpu, so cross-call to the
	 * changing cpu.
	 */
	if (state == CPUFREQ_POSTCHANGE || state == CPUFREQ_RESUMECHANGE)
		smp_call_function_single(freqs->cpu, twd_update_frequency,
			NULL, 1);

	return NOTIFY_OK;
}

static struct notifier_block twd_cpufreq_nb = {
	.notifier_call = twd_cpufreq_transition,
};

static int twd_cpufreq_init(void)
{
	if (!IS_ERR_OR_NULL(twd_clk))
		return cpufreq_register_notifier(&twd_cpufreq_nb,
			CPUFREQ_TRANSITION_NOTIFIER);

	return 0;
}
core_initcall(twd_cpufreq_init);

static void __cpuinit twd_calibrate_rate(void)
{
	unsigned long count;
	u64 waitjiffies;

	/*
	 * If this is the first time round, we need to work out how fast
	 * the timer ticks
	 */
	if (twd_timer_rate == 0) {
		printk(KERN_INFO "Calibrating local timer... ");

		/* Wait for a tick to start */
		waitjiffies = get_jiffies_64() + 1;

		while (get_jiffies_64() < waitjiffies)
			udelay(10);

		/* OK, now the tick has started, let's get the timer going */
		waitjiffies += 5;

				 /* enable, no interrupt or reload */
		__raw_writel(0x1, twd_base + TWD_TIMER_CONTROL);

				 /* maximum value */
		__raw_writel(0xFFFFFFFFU, twd_base + TWD_TIMER_COUNTER);

		while (get_jiffies_64() < waitjiffies)
			udelay(10);

		count = __raw_readl(twd_base + TWD_TIMER_COUNTER);

		twd_timer_rate = (0xFFFFFFFFU - count) * (HZ / 5);

		printk("%lu.%02luMHz.\n", twd_timer_rate / 1000000,
			(twd_timer_rate / 10000) % 100);
	}
}

static struct clk *twd_get_clock(void)
{
	struct clk *clk;
	int err;

	clk = clk_get_sys("smp_twd", NULL);
	if (IS_ERR(clk)) {
		pr_err("smp_twd: clock not found: %d\n", (int)PTR_ERR(clk));
		return clk;
	}

	err = clk_enable(clk);
	if (err) {
		pr_err("smp_twd: clock failed to enable: %d\n", err);
		clk_put(clk);
		return ERR_PTR(err);
	}

	return clk;
}

/*
 * Setup the local clock events for a CPU.
 */
void __cpuinit twd_timer_setup(struct clock_event_device *clk)
{
	if (!twd_clk)
		twd_clk = twd_get_clock();

	if (!IS_ERR_OR_NULL(twd_clk))
		twd_timer_rate = clk_get_rate(twd_clk);
	else
		twd_calibrate_rate();

#if defined(CONFIG_MP_CA7_QUAD_CORE_PATCH)


	clk->name = "arch_sys_timer";
	clk->features =  CLOCK_EVT_FEAT_ONESHOT ;
	clk->rating = 450;
	clk->set_mode = twd_set_mode;
	clk->set_next_event = twd_set_next_event;
	__get_cpu_var(twd_ce) = clk;
  	clockevents_config_and_register(clk,CA7_TIMER_FREQ,
					0xf, 0xffffffff);
#else

	__raw_writel(0, twd_base + TWD_TIMER_CONTROL);

	clk->name = "local_timer";
	clk->features = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT |
			CLOCK_EVT_FEAT_C3STOP;
	clk->rating = 350;
	clk->set_mode = twd_set_mode;
	clk->set_next_event = twd_set_next_event;

	__get_cpu_var(twd_ce) = clk;

	clockevents_config_and_register(clk, twd_timer_rate,
					0xf, 0xffffffff);
#endif
	/* Make sure our local interrupt controller has this enabled */
	gic_enable_ppi(clk->irq);
}
