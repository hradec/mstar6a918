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
#include <mach/pm.h>
#include <linux/timer.h>
//#include <plat/localtimer.h>
#include "chip_int.h"
#if defined(CONFIG_MP_KERNEL_COMPAT_FROM_2_6_35_11_TO_3_1_10) 
#include <plat/sched_clock.h>
#endif

//------------------------------------------------------------------------------
//  Macros
//------------------------------------------------------------------------------

//Mstar PIU Timer
#define TIMER_ENABLE			(0x1)
#define TIMER_TRIG				(0x2)
#define TIMER_INTERRUPT		    (0x100)
#define TIMER_CLEAR			    (0x4)
#define TIMER_CAPTURE       	(0x8)
#define ADDR_TIMER_MAX_LOW 	    (0x2<<2)
#define ADDR_TIMER_MAX_HIGH 	(0x3<<2)
#define PIU_TIMER_FREQ_KHZ	    (12000)

//ARM Global Timer
static int GLB_TIMER_FREQ_KHZ;  // PERICLK = CPUCLK/2
static unsigned long interval;



int query_frequency(void)
{
    return (12 * (reg_readw(0x1F000000 + (0x110C26 << 1)) & 0x00FF)); // 1 = 1(Mhz)
	//return (12*reg_readw(0x1F22184c));// 1 = 1(Mhz)
}
EXPORT_SYMBOL(query_frequency);


#if defined(CONFIG_MSTAR_ARM_FPGA) || \
	defined(CONFIG_MSTAR_EAGLE_FPGA)
#define CLOCK_TICK_RATE         (12*1000*1000)
#endif

#if defined(CONFIG_MP_KERNEL_COMPAT_FROM_2_6_35_11_TO_3_1_10) 
#define USE_GLOBAL_TIMER 1
#else
#define USE_GLOBAL_TIMER 0
#endif


static unsigned long long 	src_timer_cnt;
static unsigned int clkevt_base;

static cycle_t timer_read(struct clocksource *cs)
{
	src_timer_cnt=PERI_R(GT_LOADER_UP);
	src_timer_cnt=(src_timer_cnt<<32)+PERI_R(GT_LOADER_LOW);
	return src_timer_cnt;
}

static struct clocksource clocksource_timer = {
	.name		= "timer1",
	.rating		= 200,
	.read		= timer_read,
	.mask		= CLOCKSOURCE_MASK(32),
	.shift		= 20,
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

//void __init chip_clocksource_init(unsigned int base)
void __init chip_clocksource_init(void)
{

	struct clocksource *cs = &clocksource_timer;
	PERI_W(GT_CONTROL,0x1); //Enable

	//calculate the value of mult    //cycle= ( time(ns) *mult ) >> shift
	cs->mult = clocksource_khz2mult(GLB_TIMER_FREQ_KHZ, cs->shift);//PERICLK = CPUCLK/2

	clocksource_register(cs);
}

/*unsigned long long notrace sched_clock(void)
{
    // use 96bit data to prevent overflow
    cycle_t cycles = clocksource_timer.read(&clocksource_timer);
    u32 mult = clocksource_timer.mult;
    u32 shift = clocksource_timer.shift;

    u64 cyc_up=(cycles>>32);
    u64 cyc_low=(cycles&0xFFFFFFFF);
    u64 cyc_tmp;
    cyc_up  *= mult;
    cyc_low *= mult;
    cyc_tmp = cyc_low;
    cyc_low = (cyc_low + (cyc_up <<32));
    cyc_up  = ((cyc_up + (cyc_tmp>>32))>>32);
	return (shift>=64)?0:((cyc_low>>shift)|(cyc_up<<(64-shift)));
}*/

/*
 * IRQ handler for the timer
 */
static irqreturn_t timer_interrupt(int irq, void *dev_id)
{
    struct clock_event_device *evt = dev_id;
#if defined(CONFIG_MSTAR_EAGLE)
    int cpu = smp_processor_id();
    /* local_timer call pmu_handle_irq instead of PMU interrupt can't raise by itself. */
        extern u32 armpmu_enable_flag[];
    if (armpmu_enable_flag[cpu]) {
        extern irqreturn_t armpmu_handle_irq(int irq_num, void *dev);
        armpmu_handle_irq(cpu, NULL);
    }
#endif

#if USE_GLOBAL_TIMER
	{
		static unsigned int evt_timer_cnt;
		/* clear the interrupt */
		evt_timer_cnt=INREG16(clkevt_base+(0x3<<2));
		OUTREG16(clkevt_base+(0x3<<2),evt_timer_cnt);
	}
#else
	//clear timer event flag
	PERI_W(PT_STATUS, FLAG_EVENT);
#endif

	evt->event_handler(evt);

	return IRQ_HANDLED;
}


static void timer_set_mode(enum clock_event_mode mode,
	struct clock_event_device *evt)
{
#if defined(CONFIG_MP_KERNEL_COMPAT_FROM_2_6_35_11_TO_3_1_10) 
	unsigned short ctl = TIMER_INTERRUPT;
#else
	unsigned long ctl = FLAG_TIMER_PRESCALAR;
#endif
	switch (mode) {
		case CLOCK_EVT_MODE_PERIODIC:
#if USE_GLOBAL_TIMER
			interval = (PIU_TIMER_FREQ_KHZ*1000 / HZ)  ;
			OUTREG16(clkevt_base + ADDR_TIMER_MAX_LOW, (interval &0xffff));
			OUTREG16(clkevt_base + ADDR_TIMER_MAX_HIGH, (interval >>16));
			ctl|=TIMER_ENABLE;
			SETREG16(clkevt_base, ctl);
#else
			interval = (GLB_TIMER_FREQ_KHZ * 1000 / HZ);
			PERI_W(PT_LOADER, interval);
			ctl |= FLAG_IT_ENABLE | FLAG_AUTO_RELOAD | FLAG_TIMER_ENABLE;
#endif
			break;

		case CLOCK_EVT_MODE_ONESHOT:
#if USE_GLOBAL_TIMER
			/* period set, and timer enabled in 'next_event' hook */
			ctl|=TIMER_TRIG;
			SETREG16(clkevt_base, ctl);
#else
			PERI_W(PT_COUNTER, 0);
			ctl |= FLAG_IT_ENABLE | FLAG_TIMER_ENABLE;
#endif
			break;

		case CLOCK_EVT_MODE_UNUSED:
		case CLOCK_EVT_MODE_SHUTDOWN:
		default:
			break;
	}
#if defined(CONFIG_MP_KERNEL_COMPAT_FROM_2_6_35_11_TO_3_1_10) 
#else
	PERI_W(PT_CONTROL, ctl);
#endif
}

static int timer_set_next_event(unsigned long next, struct clock_event_device *evt)
{
#if USE_GLOBAL_TIMER 
	//stop timer
	//OUTREG16(clkevt_base, 0x0);

	//set period
	OUTREG16(clkevt_base + ADDR_TIMER_MAX_LOW, (next &0xffff));
	OUTREG16(clkevt_base + ADDR_TIMER_MAX_HIGH, (next >>16));

	//enable timer
	SETREG16(clkevt_base, TIMER_TRIG|TIMER_INTERRUPT);//default
#else
	PERI_W(PT_COUNTER, next);
#endif

	return 0;
}



static struct clock_event_device clockevent_timer = {
	.name		= "timer0",
	.shift		= 32,
	.features       = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode	= timer_set_mode,
	.set_next_event	= timer_set_next_event,
	.rating		= 300,
	.cpumask	= cpu_all_mask,
};

static struct irqaction timer_irq = {
	.name		= "timer",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= timer_interrupt,
	.dev_id		= &clockevent_timer,
};

void __init chip_clockevents_init(unsigned int base,unsigned int irq)
{
	struct clock_event_device *evt = &clockevent_timer;
	printk("\n%s, %d, cpu%X, 0x%X, 0x%X\n", __FUNCTION__, __LINE__, smp_processor_id(), base, irq); // bob.fu

	clkevt_base = base; // if not use global timer, don't care...by bob.fu

	evt->irq = irq;
#if USE_GLOBAL_TIMER
	evt->mult = div_sc(PIU_TIMER_FREQ_KHZ, NSEC_PER_MSEC, evt->shift); //PIU Timer FRE = 12Mhz
#else
	evt->mult = div_sc(GLB_TIMER_FREQ_KHZ, NSEC_PER_MSEC, evt->shift);
#endif
	evt->max_delta_ns = clockevent_delta2ns(0xffffffff, evt);
	evt->min_delta_ns = clockevent_delta2ns(0xf, evt);

#if USE_GLOBAL_TIMER
#else
	{
		unsigned long temp;
		//clear timer event flag
		PERI_W(PT_STATUS, FLAG_EVENT);
		//Interrupt Set Enable Register
		temp = PERI_R(GIC_DIST_SET_EANBLE);
		temp = temp | (0x1 << irq);
		PERI_W(GIC_DIST_SET_EANBLE, temp);
	}
#endif

    setup_irq(irq, &timer_irq);
    clockevents_register_device(evt);
}

extern u32 SC_MULT;
extern u32 SC_SHIFT;
void __init chip_init_timer(void){
#if defined(CONFIG_MSTAR_ARM_BD_FPGA) || \
	defined(CONFIG_MSTAR_EAGLE_BD_FPGA)
    GLB_TIMER_FREQ_KHZ= 24*1000 ;              // PERIPHCLK = CPU Clock / 2,
                                           // div 2 later,when CONFIG_GENERIC_CLOCKEVENTS
                                           // clock event will handle this value
#elif defined(CONFIG_MSTAR_ARM_BD_GENERIC) || \
	  defined(CONFIG_MSTAR_EAGLE_BD_GENERIC) 
    GLB_TIMER_FREQ_KHZ=(query_frequency()*1000/2); // PERIPHCLK = CPU Clock / 2
                                             // div 2 later,when CONFIG_GENERIC_CLOCKEVENTS
                                             // clock event will handle this value
#endif

    printk("Global Timer Frequency = %d MHz\n",GLB_TIMER_FREQ_KHZ/1000);
    printk("CPU Clock Frequency = %d MHz\n",query_frequency());


#ifdef CONFIG_HAVE_SCHED_CLOCK

//calculate mult and shift for sched_clock
#if 1 
	{
		u32 shift,mult;
		clocks_calc_mult_shift(&mult,&shift,(GLB_TIMER_FREQ_KHZ*1000),NSEC_PER_SEC,0);
		printk("fre = %d, mult= %d, shift= %d\n",(GLB_TIMER_FREQ_KHZ*1000),mult,shift);
		SC_SHIFT=shift;
		SC_MULT=mult;
	}
#endif
	mstar_sched_clock_init((void __iomem *)(PERI_VIRT+0x200),(GLB_TIMER_FREQ_KHZ*1000));
#endif

	//mstar_local_timer_init(((void __iomem *)PERI_ADDRESS(PERI_PHYS+0x600)));  //private_timer base
	chip_clocksource_init();
#if USE_GLOBAL_TIMER
	chip_clockevents_init(chip_BASE_REG_TIMER0_PA,E_FIQ_EXTIMER0);
#else
	chip_clockevents_init(chip_BASE_REG_TIMER0_PA,29);
#endif
}

struct sys_timer chip_timer = {
    .init = chip_init_timer,
};
