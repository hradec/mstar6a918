#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <asm/smp_twd.h>
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
#include <plat/sched_clock.h>

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
static unsigned int GLB_TIMER_FREQ_KHZ;  // PERICLK = CPUCLK/2
static unsigned long interval;

/*------------------------------------------------------------------------------
    Local Function Prototypes
-------------------------------------------------------------------------------*/
void __init chip_clocksource_init(unsigned int base);
void __init chip_clockevents_init(unsigned int base,unsigned int irq);
void __init chip_init_timer(void);

unsigned int query_frequency(void)
{
	unsigned int reg60 = 0;
	unsigned int reg61 = 0;

	reg60 = reg_readw(0x1F000000 + 0x110C00 * 2 + 0x60 * 4);
	reg61 = reg_readw(0x1F000000 + 0x110C00 * 2 + 0x61 * 4);
	return 0xA1FFFF40 / ((reg61 << 16) + reg60);	
}
EXPORT_SYMBOL(query_frequency);

#if defined(CONFIG_MSTAR_ARM_FPGA)
#define CLOCK_TICK_RATE         (12*1000*1000)
#endif


#ifndef CONFIG_GENERIC_CLOCKEVENTS
static irqreturn_t chip_timer_interrupt(int irq, void *dev_id)
{

    unsigned short tmp;
    timer_tick();

    //stop timer
    CLRREG16(chip_BASE_REG_TIMER0_PA, TIMER_TRIG);
    //set interval
    //interval = (CLOCK_TICK_RATE / HZ);
    OUTREG16(chip_BASE_REG_TIMER0_PA + ADDR_TIMER_MAX_LOW, (interval &0xffff));
    OUTREG16(chip_BASE_REG_TIMER0_PA + ADDR_TIMER_MAX_HIGH, (interval >>16));

    //trig timer0
    SETREG16(chip_BASE_REG_TIMER0_PA, TIMER_TRIG);

    return IRQ_HANDLED;
}

static struct irqaction chip_timer_irq = {
    .name = "Einstein Timer Tick",
    .flags = IRQF_TIMER | IRQF_IRQPOLL | IRQF_DISABLED,
    .handler = chip_timer_interrupt,
};
#endif


#ifdef CONFIG_GENERIC_CLOCKEVENTS

#define USE_GLOBAL_TIMER 1
#if USE_GLOBAL_TIMER
static unsigned long long	src_timer_cnt;
#else
static unsigned int src_timer_cnt;
#endif

static unsigned int evt_timer_cnt;
static unsigned int clksrc_base;
static unsigned int clkevt_base;

static cycle_t timer_read(struct clocksource *cs)
{
#if USE_GLOBAL_TIMER	
	src_timer_cnt=PERI_R(GT_LOADER_UP);
	src_timer_cnt=(src_timer_cnt<<32)+PERI_R(GT_LOADER_LOW);
#else
	src_timer_cnt=INREG16(clksrc_base+(0x5<<2));
	src_timer_cnt=(src_timer_cnt<<16)+INREG16(clksrc_base+(0x4<<2));
#endif	
	return src_timer_cnt;
}

struct clocksource clocksource_timer = {
	.name		= "timer1",
	.rating		= 200,
	.read		= timer_read,
	.mask		= CLOCKSOURCE_MASK(32),
	.shift		= 20,   
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};
EXPORT_SYMBOL(clocksource_timer);

void __init chip_clocksource_init(unsigned int base)
{

	struct clocksource *cs = &clocksource_timer;
	clksrc_base = base;
#if USE_GLOBAL_TIMER
	PERI_W(GT_CONTROL,0x1); //Enable	

	//calculate the value of mult    //cycle= ( time(ns) *mult ) >> shift
	cs->mult = clocksource_khz2mult((u32)GLB_TIMER_FREQ_KHZ, cs->shift);//PERICLK = CPUCLK/2
#else	
	/* setup timer 1 as free-running clocksource */
	//make sure timer 1 is disable
	CLRREG16(clksrc_base, TIMER_ENABLE);

	//set max period	
	OUTREG16(clksrc_base+(0x2<<2),0xffff);
	OUTREG16(clksrc_base+(0x3<<2),0xffff);

	//enable timer 1
	SETREG16(clksrc_base, TIMER_ENABLE);

		// TODO: need to double check  
	//calculate the value of mult    //cycle= ( time(ns) *mult ) >> shift
	cs->mult = clocksource_khz2mult(GLB_TIMER_FREQ_KHZ, cs->shift);  //Mstar timer =>12Mhz, 
#endif                  
	
	clocksource_register(cs);
}

/*
 * IRQ handler for the timer
 */
static irqreturn_t timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = dev_id;

	/* clear the interrupt */
	evt_timer_cnt=INREG16(clkevt_base+(0x3<<2));
	OUTREG16(clkevt_base+(0x3<<2),evt_timer_cnt);

	evt->event_handler(evt);

	return IRQ_HANDLED;
}


static void timer_set_mode(enum clock_event_mode mode,
	struct clock_event_device *evt)
{
    unsigned short ctl=TIMER_INTERRUPT;
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
	interval = (PIU_TIMER_FREQ_KHZ*1000 / HZ)  ;
	OUTREG16(clkevt_base + ADDR_TIMER_MAX_LOW, (interval &0xffff));
	OUTREG16(clkevt_base + ADDR_TIMER_MAX_HIGH, (interval >>16));
        ctl|=TIMER_ENABLE;
		SETREG16(clkevt_base, ctl);
		break;

	case CLOCK_EVT_MODE_ONESHOT:
		/* period set, and timer enabled in 'next_event' hook */
        ctl|=TIMER_TRIG;
		SETREG16(clkevt_base, ctl);
		break;

	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
		OUTREG16(clkevt_base, 0);
	default:
		break;
	}
}

static int timer_set_next_event(unsigned long next, struct clock_event_device *evt)
{
	//stop timer
	//OUTREG16(clkevt_base, 0x0);

	//set period
	OUTREG16(clkevt_base + ADDR_TIMER_MAX_LOW, (next &0xffff));
	OUTREG16(clkevt_base + ADDR_TIMER_MAX_HIGH, (next >>16));

	//enable timer
	SETREG16(clkevt_base, TIMER_TRIG|TIMER_INTERRUPT);//default

	return 0;
}

static struct clock_event_device clockevent_timer = {
	.name		= "timer0",
	.shift		= 32,
	.features       = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode	= timer_set_mode,
	.set_next_event	= timer_set_next_event,
	.rating		= 300,
//	.cpumask        = cpu_all_mask,
//	.cpumask	= cpumask_of(0),
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

	clkevt_base = base;

	evt->cpumask = cpumask_of(0);
	evt->irq = (int)irq;
	evt->mult = (unsigned int)div_sc(PIU_TIMER_FREQ_KHZ, NSEC_PER_MSEC, (int)evt->shift); //PIU Timer FRE = 12Mhz
	evt->max_delta_ns = clockevent_delta2ns(0xffffffff, evt);
	evt->min_delta_ns = clockevent_delta2ns(0xf, evt);

	setup_irq(irq, &timer_irq);
	clockevents_register_device(evt);
}

#endif

#ifdef CONFIG_HAVE_ARM_TWD
static DEFINE_TWD_LOCAL_TIMER(twd_local_timer, chip_MPCORE_TWD, IRQ_LOCALTIMER);

static void __init chip_twd_init(void)
{
    int err = twd_local_timer_register(&twd_local_timer);
    if (err)
        pr_err("twd_local_timer_register failed %d\n", err);
}
#else
#define chip_twd_init()  do {} while(0)
#endif

static int __init chip_init_timer_clocks(void)
{
    struct clk *fclk;
    struct clk *armperi_clk;

    fclk = clk_register_fixed_rate(NULL, "fclk", NULL, CLK_IS_ROOT, GLB_TIMER_FREQ_KHZ * 1000 * 2);

    armperi_clk = clk_register_fixed_rate(NULL, "arm_peri", "fclk", 0, GLB_TIMER_FREQ_KHZ * 1000);

    /* TODO: use static lookup table? */
    clk_register_clkdev(fclk, "fclk", NULL);
    clk_register_clkdev(armperi_clk, NULL, "smp_twd");

    return 0;
}

extern u32 SC_MULT;
extern u32 SC_SHIFT;
void __init chip_init_timer(void)
{
   	u32 shift,mult;


	GLB_TIMER_FREQ_KHZ=(query_frequency()*1000/2); // PERIPHCLK = CPU Clock / 2  
                                             // div 2 later,when CONFIG_GENERIC_CLOCKEVENTS
                                             // clock event will handle this value

	printk("Global Timer Frequency = %d MHz\n",GLB_TIMER_FREQ_KHZ/1000);
	printk("CPU Clock Frequency = %d MHz\n",query_frequency());
	
	chip_init_timer_clocks();

	clocks_calc_mult_shift(&mult, &shift, (u32)(GLB_TIMER_FREQ_KHZ*1000), NSEC_PER_SEC,0);
	printk("fre = %d, mult= %d, shift= %d\n",(GLB_TIMER_FREQ_KHZ*1000),mult,shift);
	SC_SHIFT=shift;
	SC_MULT=mult;
	mstar_sched_clock_init((void __iomem *)(PERI_VIRT+0x200), (unsigned long)(GLB_TIMER_FREQ_KHZ*1000));

	chip_clocksource_init(chip_BASE_REG_TIMER1_PA);
	chip_clockevents_init(chip_BASE_REG_TIMER0_PA,E_FIQ_EXTIMER0);

	chip_twd_init();
}

struct sys_timer chip_timer = {
    .init = chip_init_timer,
};

