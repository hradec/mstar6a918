/*
 *  linux/arch/arm/mach-vexpress/platsmp.c
 *
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/smp.h>
#include <linux/io.h>

#include <asm/smp_scu.h>
#include <asm/hardware/gic.h>

#include <asm/unified.h>


#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/smp.h>

#include <asm/cacheflush.h>
#include <asm/smp_plat.h>
#include <asm/hardware/gic.h>


#include <linux/init.h>
#include <linux/errno.h>
#include <linux/smp.h>
#include <linux/io.h>
#include <linux/of_fdt.h>
#include <linux/vexpress.h>

#include <asm/smp_scu.h>
#include <asm/hardware/gic.h>
#include <asm/mach/map.h>

#include <mach/motherboard.h>

#include <chip_setup.h>

#define SECOND_MAGIC_NUMBER_ADRESS (0xFD000000 + 0x101D00 * 2 + 0x6D * 4)
#define SECOND_START_ADDR (0xFD000000 + (0x101D00 * 2) + (0x7B * 4))

extern void vexpress_secondary_startup(void);
void __init chip_smp_init_cpus(void);
void __init chip_smp_prepare_cpus(unsigned int max_cpus);
void __cpuinit chip_secondary_init(unsigned int cpu);
int __cpuinit chip_boot_secondary(unsigned int cpu, struct task_struct *idle);

#if 1
/*
 * Initialise the CPU possible map early - this describes the CPUs
 * which may be present or become present in the system.
 */
void __init chip_smp_init_cpus(void)
{
	unsigned int i, ncores;
	ncores = scu_get_core_count((void __iomem *)(PERI_ADDRESS(0x16000000)));

	/* sanity check */
	if (ncores == 0) {
		printk(KERN_ERR
		       "vexpress: strange CM count of 0? Default to 1\n");

		ncores = 1;
	}

	if (ncores > NR_CPUS) {
		printk(KERN_WARNING
		       "vexpress: no. of cores (%d) greater than configured "
		       "maximum of %d - clipping\n",
		       ncores, NR_CPUS);
		ncores = NR_CPUS;
	}

	for (i = 0; i < ncores; i++)
		set_cpu_possible(i, true);
		
	set_smp_cross_call(gic_raise_softirq);
}
#endif

void __init chip_smp_prepare_cpus(unsigned int max_cpus)
{
	unsigned int i;

	/*
	 * Initialise the present map, which describes the set of CPUs
	 * actually populated at the present time.
	 */
	for (i = 0; i < max_cpus; i++)
		set_cpu_present(i, true);

		scu_enable((void __iomem *)(PERI_ADDRESS(0x16000000))); // SCU PA = 0x16000000 

		/*
		 * Write the address of secondary startup into the
		 * system-wide flags register. The boot monitor waits
		 * until it receives a soft interrupt, and then the
		 * secondary CPU branches to this address.
		 */
		
		printk("_Secondary_startup physical address = 0x%08x\n",BSYM(virt_to_phys(vexpress_secondary_startup)));
		//writew((BSYM(virt_to_phys(vexpress_secondary_startup))>>16), (void*)SECOND_START_ADDR_HI);
		//writew(BSYM(virt_to_phys(vexpress_secondary_startup)), (void*)SECOND_START_ADDR_LO);
    		writel(0xbabe, SECOND_MAGIC_NUMBER_ADRESS);
		writel(BSYM(virt_to_phys(vexpress_secondary_startup)) & 0xFFFF, SECOND_START_ADDR);
		writel(BSYM(virt_to_phys(vexpress_secondary_startup)) >> 16, SECOND_START_ADDR + 4);
}

/*
 * Write pen_release in a way that is guaranteed to be visible to all
 * observers, irrespective of whether they're taking part in coherency
 * or not.  This is necessary for the hotplug code to work reliably.
 */
static void __cpuinit write_pen_release(int val)
{
	pen_release = val;
	smp_wmb();
#if 1
	Chip_Flush_Cache_Range((unsigned long)&pen_release, sizeof(pen_release));
#else
	__cpuc_flush_dcache_area((void *)&pen_release, sizeof(pen_release));
	outer_clean_range(__pa(&pen_release), __pa(&pen_release + 1));
#endif
}

static DEFINE_SPINLOCK(boot_lock);

void __cpuinit chip_secondary_init(unsigned int cpu)
{
	/*
	 * if any interrupts are already enabled for the primary
	 * core (e.g. timer irq), then they will not have been enabled
	 * for us: do so
	 */
	gic_secondary_init(0);

	/*
	 * let the primary processor know we're out of the
	 * pen, then head off into the C entry point
	 */
	write_pen_release(-1);

	/*
	 * Synchronise with the boot thread.
	 */
	spin_lock(&boot_lock);
	spin_unlock(&boot_lock);
}

#if 1
int __cpuinit chip_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	unsigned long timeout;

	/*
	 * Set synchronisation state between this boot processor
	 * and the secondary one
	 */
	spin_lock(&boot_lock);

	/*
	 * This is really belt and braces; we hold unintended secondary
	 * CPUs in the holding pen until we're ready for them.  However,
	 * since we haven't sent them a soft interrupt, they shouldn't
	 * be there.
	 */
	write_pen_release(cpu_logical_map(cpu));

	#if 0  //debug
	unsigned int *ptr=&pen_release;
	//printk("pen_release = 0x%08x, addr= 0x%08x, pen_release ptr = 0x%08x\n ",pen_release,&pen_release,*ptr);
	#endif

	/*
	 * Send the secondary CPU a soft interrupt, thereby causing
	 * the boot monitor to read the system wide flags register,
	 * and branch to the address found there.
	 */
	gic_raise_softirq(cpumask_of(cpu), 0);

	timeout = jiffies + (1 * HZ);
	while (time_before(jiffies, timeout)) {
		smp_rmb();
		if (pen_release == -1)
			break;

		udelay(10);
	}

	/*
	 * now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	spin_unlock(&boot_lock);

	return pen_release != -1 ? -ENOSYS : 0;
}
#endif


struct smp_operations __initdata chip_smp_ops = {
	.smp_init_cpus		= chip_smp_init_cpus,
	.smp_prepare_cpus	= chip_smp_prepare_cpus,
	.smp_secondary_init	= chip_secondary_init,
	.smp_boot_secondary	= chip_boot_secondary,
};
