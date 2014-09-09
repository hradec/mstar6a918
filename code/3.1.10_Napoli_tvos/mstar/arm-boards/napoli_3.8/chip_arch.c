/*------------------------------------------------------------------------------
	Copyright (c) 2008 MStar Semiconductor, Inc.  All rights reserved.
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
    PROJECT: chip

	FILE NAME: arch/arm/arm-boards/einstein_3.8/chip_arch.c

    DESCRIPTION:
          Power Management Driver

    HISTORY:
         <Date>     <Author>    <Modification Description>
        2008/07/18  Fred Cheng  Add IO tables for ITCM and DTCM
        2008/07/22  Evan Chang  Add SD card init

------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    Include Files
------------------------------------------------------------------------------*/
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/memory.h>
#include <mach/io.h>
#include <asm/mach/map.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <asm/irq.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/hardware/gic.h>
#include <asm/cacheflush.h>

#include "chip_int.h"
/*------------------------------------------------------------------------------
    External Function Prototypes
-------------------------------------------------------------------------------*/
/* initialize chip's IRQ */
extern void chip_init_irq(void);
/* Add SD Card driver */
extern void __init chip_add_device_mmc(short mmc_id);
extern void __init chip_add_device_memstick(s16 memstick_id);
//extern int Mstar_ehc_platform_init(void);
/*------------------------------------------------------------------------------
    Local Function Prototypes
-------------------------------------------------------------------------------*/
/*static*/ void __init chip_map_io(void);
/*static*/ void __init serial_init(void);
static void __init chip_l2c_init(void);
inline void _chip_flush_miu_pipe(void);
void Chip_Flush_Cache_Range(unsigned long u32Addr, unsigned long u32Size);
void Chip_Clean_Cache_Range(unsigned long u32Addr, unsigned long u32Size);
void Chip_Inv_Cache_Range(unsigned long u32Addr, unsigned long u32Size);
void Chip_Flush_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size);
void Chip_Clean_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size);
void Chip_Inv_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size);
void Chip_Flush_Cache_All(void);
void Chip_L2_cache_wback_inv( unsigned long addr, unsigned long size);
void Chip_L2_cache_wback( unsigned long addr, unsigned long size);
void Chip_L2_cache_inv( unsigned long addr, unsigned long size);
void Chip_Flush_Memory_Range(unsigned long pAddress , unsigned long  size);
void Chip_Flush_Memory(void);
void Chip_Read_Memory_Range(unsigned long pAddress , unsigned long  size);
void Chip_Read_Memory(void);
void Chip_Query_L2_Config(void);
unsigned int Chip_Query_Rev(void);

/*------------------------------------------------------------------------------
    External Global Variable
-------------------------------------------------------------------------------*/
/* chip system timer */
extern struct sys_timer chip_timer;
extern struct sys_timer chip_ptimer;
extern struct smp_operations chip_smp_ops;
/*------------------------------------------------------------------------------
    Global Variable
-------------------------------------------------------------------------------*/
/* platform device for chip_board_init() */
struct platform_device chip_device_usbgadget =
{
	.name		  = "einstein-usbgadget",
	.id		  = -1,
};

#if defined(CONFIG_OPROFILE) 
/* resource for pmu irq */
static struct resource pmu_resources[] =
{
	    [0] = {
		        .start  = CHIP_IRQ_PMU0,
		        .end    = CHIP_IRQ_PMU0,
		        .flags  = IORESOURCE_IRQ,
		    },
	    [1] = {
		        .start  = CHIP_IRQ_PMU1,
		        .end    = CHIP_IRQ_PMU1,
		        .flags  = IORESOURCE_IRQ,
		    },
	    [2] = {
		        .start  = CHIP_IRQ_PMU2,
		        .end    = CHIP_IRQ_PMU2,
		        .flags  = IORESOURCE_IRQ,
		    },
	    [3] = {
		        .start  = CHIP_IRQ_PMU3,
		        .end    = CHIP_IRQ_PMU3,
		        .flags  = IORESOURCE_IRQ,
		    },
};

static struct platform_device chip_device_pmu =
{
        .name      = "arm-pmu",
        .id        = -1,
        .num_resources = ARRAY_SIZE(pmu_resources),
        .resource = pmu_resources,
};
#endif

static struct platform_device Mstar_emac_device = {
    .name       = "Mstar-emac",
    .id     = 0,
};
static struct platform_device Mstar_ir_device = {
	.name		= "Mstar-ir",
	.id		= 0,
};

static struct platform_device Mstar_gmac_device = {
    .name       = "Mstar-gmac",
    .id     = 0,
};

static struct platform_device Mstar_mbx_device = {
	.name		= "Mstar-mbx",
	.id		= 0,
};

static struct platform_device Mstar_gflip_device = {
	.name		= "Mstar-gflip",
	.id		= 0,
};

static struct platform_device Mstar_alsa_device = {
	.name		= "Mstar-alsa",
	.id		= 0,
};
#ifdef CONFIG_MSTAR_IIC_MODULE
static struct platform_device Mstar_iic_device = {
	.name		= "Mstar-iic",
	.id		= 0,
};
#endif
#ifdef CONFIG_MSTAR_GPIO_MODULE
static struct platform_device Mstar_gpio_device = {
	.name		= "Mstar-gpio",
	.id		= 0,
};
#endif

/* platform device array for chip_board_init() */
static struct platform_device *chip_devices[] __initdata =
{
	&chip_device_usbgadget,
	#if (MP_PLATFORM_ARM_PMU == 1)
	&chip_device_pmu,
	#endif/*MP_PLATFORM_ARM_PMU*/
    &Mstar_emac_device,
	&Mstar_ir_device,
    &Mstar_gmac_device,
    &Mstar_mbx_device,
    &Mstar_gflip_device,
    &Mstar_alsa_device,
#ifdef CONFIG_MSTAR_IIC_MODULE
    &Mstar_iic_device,
#endif
#ifdef CONFIG_MSTAR_GPIO_MODULE
    &Mstar_gpio_device,
#endif
};

/* IO tables for Registers, ITCM and DTCM */
static struct map_desc chip_io_desc[] __initdata =
{
    /* Define Registers' physcial and virtual addresses */
    {
	    .virtual        = IO_VIRT,
	    .pfn            = __phys_to_pfn(IO_PHYS),
	    .length         = IO_SIZE,
	    .type           = MT_DEVICE
    },

    /* Define periphral physcial and virtual addresses */
    {
	    .virtual        = PERI_VIRT,
	    .pfn            = __phys_to_pfn(PERI_PHYS),
	    .length         = PERI_SIZE,
	    .type           = MT_DEVICE
    },	 	

    {
	    .virtual	    = L2_CACHE_VIRT,
	    .pfn	    = __phys_to_pfn(L2_CACHE_PHYS),
	    .length	    = L2_CACHE_SIZE,
	    .type	    = MT_DEVICE,
    },
};

/*------------------------------------------------------------------------------
    Local Function
-------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Function: chip_map_io

    Description:
        init  chip IO tables
    Input: (The arguments were used by caller to input data.)
        None.
    Output: (The arguments were used by caller to receive data.)
        None.
    Return:
        None.
    Remark:
        None.
-------------------------------------------------------------------------------*/
void __init chip_map_io(void)
{
    iotable_init(chip_io_desc, ARRAY_SIZE(chip_io_desc) );
	serial_init();
#ifdef CONFIG_CACHE_L2X0
	chip_l2c_init();
#endif
}

void __init serial_init(void)
{

#ifdef CONFIG_SERIAL_8250
    struct uart_port uart0, uart1, uart2, fuart;

	struct uart_port uart3, uart4;

    memset(&uart0, 0, sizeof(uart0));
    uart0.type = PORT_16550;
    uart0.iobase = 0xFD201300; //virtual RIU_BASE = 0xFD000000   
    uart0.irq = E_IRQ_UART0;

#ifdef CONFIG_MSTAR_ARM_BD_FPGA
    uart0.uartclk = 12000000; //FPGA
#elif  CONFIG_MSTAR_ARM_BD_GENERIC  
    uart0.uartclk = 123000000; //real chip
#endif
    uart0.iotype = 0;
    uart0.regshift = 0;
    uart0.fifosize = 16 ; // use the 8 byte depth FIFO well
    uart0.line = 0;

    if (early_serial_setup(&uart0) != 0) {
        printk(KERN_ERR "Serial(0) setup failed!\n");
    }
	memset(&uart1, 0, sizeof(uart1));
	uart1.type = PORT_16550;
	uart1.iobase = 0xFD220C00;
	uart1.irq = E_IRQEXPL_UART1;
	//uart1.uartclk = 108000000;
	uart1.uartclk = 123000000;
	uart1.iotype = 0;
	uart1.regshift = 0;
	uart1.fifosize = 16 ; // use the 8 byte depth FIFO well
	uart1.line = 1;
	
	if (early_serial_setup(&uart1) != 0) {
		printk(KERN_ERR "Serial(1) setup failed!\n");
	}

	memset(&uart2, 0, sizeof(uart2));
	uart2.type = PORT_16550;
	uart2.iobase = 0xFD220C80;
	uart2.irq = E_IRQEXPL_UART2 ;
	uart2.uartclk = 123000000;
	uart2.iotype = 0;
	uart2.regshift = 0;
	uart2.fifosize = 16 ; // use the 8 byte depth FIFO well
	uart2.line = 3;

	if (early_serial_setup(&uart2) != 0) {
		printk(KERN_ERR "Serial(2) setup failed!\n");
	}
	

	memset(&uart3, 0, sizeof(uart3));
	uart3.type = PORT_16550;
	uart3.iobase = 0xFD201280;
	uart3.irq = E_IRQEXPL_UART3 ;
	uart3.uartclk = 123000000;
	uart3.iotype = 0;
	uart3.regshift = 0;
	uart3.fifosize = 16 ; // use the 8 byte depth FIFO well
	uart3.line = 4;

	if (early_serial_setup(&uart3) != 0) {
		printk(KERN_ERR "Serial(3) setup failed!\n");
	}

	memset(&uart4, 0, sizeof(uart4));
	uart4.type = PORT_16550;
	uart4.iobase = 0xFD220D80;
	uart4.irq = E_IRQ_UART4;
	uart4.uartclk = 123000000;
	uart4.iotype = 0;
	uart4.regshift = 0;
	uart4.fifosize = 16 ; // use the 8 byte depth FIFO well
	uart4.line = 5;

	if (early_serial_setup(&uart4) != 0) {
		printk(KERN_ERR "Serial(4) setup failed!\n");
	}
	
	memset(&fuart, 0, sizeof(fuart));
	fuart.type = PORT_16550;
	fuart.iobase = 0xFD220D00;
	fuart.irq = E_IRQEXPH_UART2MCU;
	fuart.uartclk = 123000000;
	fuart.iotype = 0;
	fuart.regshift = 0;
	fuart.fifosize = 16 ; // use the 8 byte depth FIFO well
	fuart.line = 2;

	if (early_serial_setup(&fuart) != 0) {
		printk(KERN_ERR "Serial(f) setup failed!\n");
	}

#endif
}

extern void back_trace(void);

static irqreturn_t chip_xiu_timeout_interrupt(int irq, void *dev_id)
{
  

    #ifdef CONFIG_HANG_ISSUE_DEBUG
    char * ptr= NULL;
    #endif

    printk("XIU Time Out Occurred!\n");
    printk("Address is 0x%x\n",*(volatile  short*)(0xfd200224));  
   
    #ifdef CONFIG_HANG_ISSUE_DEBUG
    back_trace(); 
    #endif
     
    #ifdef CONFIG_HANG_ISSUE_DEBUG
    *ptr = 1; 
    #endif

    return IRQ_HANDLED;
} 

static struct irqaction chip_xiu_timeout_irq = {
    .name = "Einstein Timeout IRQ",
    .flags = IRQF_TIMER | IRQF_IRQPOLL | IRQF_DISABLED,
    .handler = chip_xiu_timeout_interrupt,

};

void __init chip_l2c_init(void)
{
#ifdef CONFIG_CACHE_L2X0
    void __iomem * l2x0_base = (void __iomem *)(L2_CACHE_ADDRESS(L2_CACHE_PHYS));
    unsigned int val = 0;
#if L2_LINEFILL
    val = L2_CACHE_read( L2_CACHE_PHYS + PREFETCH_CTL_REG );
    L2_CACHE_write(( val | DOUBLE_LINEFILL_ENABLE | LINEFILL_WRAP_DISABLE ), L2_CACHE_PHYS + PREFETCH_CTL_REG);
#endif

#if L2_PREFETCH
    val = L2_CACHE_read( L2_CACHE_PHYS + PREFETCH_CTL_REG );
    L2_CACHE_write(( val | I_PREFETCH_ENABLE | D_PREFETCH_ENABLE | PREFETCH_OFFSET ), L2_CACHE_PHYS + PREFETCH_CTL_REG );
#endif

    /* set RAM latencies to 2 cycle for this core tile. */
    L2_CACHE_write(0x111, L2_CACHE_PHYS + L2X0_TAG_LATENCY_CTRL);
    L2_CACHE_write(0x121, L2_CACHE_PHYS + L2X0_DATA_LATENCY_CTRL);
//#endif    /* end of CONFIG_ARM_TRUSTZONE */

    l2x0_init(l2x0_base, 0x00400000, 0xfe0fffff);
#endif
}

/*------------------------------------------------------------------------------
    Function: chip_board_init

    Description:
        chip board init function
    Input: (The arguments were used by caller to input data.)
        None.
    Output: (The arguments were used by caller to receive data.)
        None.
    Return:
        None.
    Remark:
        None.
-------------------------------------------------------------------------------*/
static void __init chip_board_init(void)
{

	platform_add_devices(chip_devices, ARRAY_SIZE(chip_devices));

#if defined(CONFIG_USB_EHCI_HCD) || defined(CONFIG_USB_EHCI_HCD_MODULE)
	Mstar_ehc_platform_init();
#endif

#if defined(CONFIG_MEMSTICK_CHIP_MS)
	chip_add_device_memstick(0);
#endif


	*(volatile unsigned short *)(0xfd200200) = 0x1 ;
	*(volatile unsigned short *)(0xfd200250) = 0xFFFF ;
	*(volatile unsigned short *)(0xfd200254) = 0xFFFF ;

    setup_irq(E_FIQ_XIU_TIMEOUT , &chip_xiu_timeout_irq);
}


/*************************************
*		Mstar chip flush function
*************************************/
#define _BIT(x)                      (1<<(x))
static DEFINE_SPINLOCK(l2prefetch_lock);

inline void _chip_flush_miu_pipe(void)
{
	unsigned long   dwLockFlag = 0;    
	unsigned short dwReadData = 0;

	spin_lock_irqsave(&l2prefetch_lock, dwLockFlag);
	//toggle the flush miu pipe fire bit 
	*(volatile unsigned short *)(0xfd203114) = 0x0;
	*(volatile unsigned short *)(0xfd203114) = 0x1;

	do
	{
		dwReadData = *(volatile unsigned short *)(0xfd203140);
		dwReadData &= _BIT(12);  //Check Status of Flush Pipe Finish

	} while(dwReadData == 0);

	spin_unlock_irqrestore(&l2prefetch_lock, dwLockFlag);

}

#ifndef CONFIG_OUTER_CACHE
struct outer_cache_fns outer_cache;
#endif

void Chip_Flush_Cache_Range(unsigned long u32Addr, unsigned long u32Size)
{
	if(  u32Addr == (unsigned long)NULL )
        {
                printk("u32Addr is invalid\n");
                return;
        }        	
	//Clean L1 & Inv L1   
	dmac_flush_range((void *)u32Addr,(void *)(u32Addr + u32Size));
       
#ifdef CONFIG_OUTER_CACHE
	if (outer_cache.is_enable()) //check if L2 is enabled
	{
		if(!virt_addr_valid(u32Addr) || !virt_addr_valid(u32Addr+ u32Size - 1))
			//Clean&Inv L2 by Way
			outer_cache.flush_all();
		else 
			//Clean&Inv L2 by Range
			outer_cache.flush_range(__pa(u32Addr) ,__pa(u32Addr)+ u32Size);
	}
#endif

#ifndef CONFIG_OUTER_CACHE //flush miu pipe for L2 disabled case
	_chip_flush_miu_pipe();
#endif

}

void Chip_Clean_Cache_Range(unsigned long u32Addr, unsigned long u32Size)
{
        if(  u32Addr == (unsigned long)NULL )
        {
                printk("u32Addr is invalid\n");
                return;
        }
	//Clean L1
	dmac_map_area((void *)u32Addr,u32Size,1);

#ifdef CONFIG_OUTER_CACHE
	if (outer_cache.is_enable()) //check if L2 is enabled
	{
		if(!virt_addr_valid(u32Addr) || !virt_addr_valid(u32Addr+ u32Size - 1))
			//Clean L2 by Way
			outer_cache.clean_all();
		else 
			//Clean L2 by Range 
			outer_cache.clean_range( __pa(u32Addr),__pa(u32Addr) + u32Size);
	}
#endif

#ifndef CONFIG_OUTER_CACHE //flush miu pipe for L2 disabled case
	_chip_flush_miu_pipe();
#endif
}


void Chip_Inv_Cache_Range(unsigned long u32Addr, unsigned long u32Size)
{
        if(  u32Addr == (unsigned long)NULL )
        {
                printk("u32Addr is invalid\n");
                return;
        }
#ifdef CONFIG_OUTER_CACHE
	if (outer_cache.is_enable()) //check if L2 is enabled
	{

		if(!virt_addr_valid(u32Addr) || !virt_addr_valid(u32Addr+ u32Size - 1))
			printk(KERN_DEBUG "Input VA can't be converted to PA\n");   
		else
			//Inv L2 by range 
			outer_cache.inv_range(__pa(u32Addr) , __pa(u32Addr) + u32Size);
	}
#endif
	//Inv L1   
	dmac_map_area((void *)u32Addr,u32Size,2); 
}

void Chip_Flush_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size)
{
        if(  u32VAddr == (unsigned long)NULL )
        {
                printk("u32VAddr is invalid\n");
                return;
        }
	//Clean & Invalid L1
	dmac_flush_range((void *)u32VAddr, (void *)(u32VAddr + u32Size));

#ifdef CONFIG_OUTER_CACHE
	if (outer_cache.is_enable()) //check if L2 is enabled
	{
		//Clean&Inv L2 by range
		outer_cache.flush_range(u32PAddr,u32PAddr + u32Size);
	}
#endif

#ifndef CONFIG_OUTER_CACHE
	_chip_flush_miu_pipe();
#endif
}

void Chip_Clean_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size)
{
        if(  u32VAddr == (unsigned long)NULL )
        {
                printk("u32VAddr is invalid\n");
                return;
        }
	//Clean L1
	dmac_map_area((void *)u32VAddr,u32Size,1);

#ifdef CONFIG_OUTER_CACHE
	if (outer_cache.is_enable()) //check if L2 is enabled
	{
		//Clean L2 by Way
		outer_cache.clean_range(u32PAddr,u32PAddr + u32Size);
	}
#endif

#ifndef CONFIG_OUTER_CACHE
	_chip_flush_miu_pipe();
#endif
}

void Chip_Inv_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size)
{
        if(  u32VAddr == (unsigned long)NULL )
        {
                printk("u32VAddr is invalid\n");
                return;
        }

#ifdef CONFIG_OUTER_CACHE
	if (outer_cache.is_enable()) //check if L2 is enabled
	{
		//Inv L2 by range
		outer_cache.inv_range( u32PAddr ,u32PAddr + u32Size );
	}
#endif
	//Inv L1
	dmac_map_area((void *)u32VAddr,u32Size,2);
}

void Chip_Flush_Cache_All(void)
{
	unsigned long flags;

	local_irq_save(flags);

	//Clean & Inv All L1
	__cpuc_flush_kern_all();

	local_irq_restore(flags);

	smp_call_function((smp_call_func_t)__cpuc_flush_kern_all, NULL, 1);

	local_irq_save(flags);

#ifdef CONFIG_OUTER_CACHE
	if (outer_cache.is_enable()) //check if L2 is enabled
	{
		//Clean&Inv L2 by Way
		outer_cache.flush_all();
	}
#endif
	//Clean L1  & Inv L1
	//dmac_flush_range(u32Addr,u32Addr + u32Size );

#ifndef CONFIG_OUTER_CACHE
	_chip_flush_miu_pipe();
#endif 
	
	local_irq_restore(flags);
}

//need to be modified
void Chip_L2_cache_wback_inv( unsigned long addr, unsigned long size)
{
	outer_cache.flush_all();
}

//need to be modified
void Chip_L2_cache_wback( unsigned long addr, unsigned long size)
{
	//Flush L2 by Way, change to by Addr later
	outer_cache.clean_all();
}

//need to be modified
void Chip_L2_cache_inv( unsigned long addr, unsigned long size)
{
	//Inv L2 by Way, change to by Addr later
	outer_cache.inv_all();
}

void Chip_Flush_Memory_Range(unsigned long pAddress , unsigned long  size)
{
#ifdef CONFIG_OUTER_CACHE_SYNC
	//2012/01/12 Modify by MStar: Add outer_cache.sync for enabling the non-cache bufferable mode
	if(outer_cache.sync)
		outer_cache.sync();
	else
		_chip_flush_miu_pipe();
#else
	_chip_flush_miu_pipe();
#endif
}

void Chip_Flush_Memory(void)
{
#ifdef CONFIG_OUTER_CACHE_SYNC
	//2012/01/12 Modify by MStar: Add outer_cache.sync for enabling the non-cache bufferable mode
	if(outer_cache.sync)
		outer_cache.sync();
	else
		_chip_flush_miu_pipe();
#else
	_chip_flush_miu_pipe();
#endif
}

void Chip_Read_Memory_Range(unsigned long pAddress , unsigned long  size)
{
#ifdef CONFIG_OUTER_CACHE_SYNC
	//2012/01/12 Modify by MStar: Add outer_cache.sync for enabling the non-cache bufferable mode
	if(outer_cache.sync)
		outer_cache.sync();
	else
		_chip_flush_miu_pipe();
#else
	_chip_flush_miu_pipe();
#endif
}

void Chip_Read_Memory(void)
{
#ifdef CONFIG_OUTER_CACHE_SYNC
	//2012/01/12 Modify by MStar: Add outer_cache.sync for enabling the non-cache bufferable mode
	if(outer_cache.sync)
		outer_cache.sync();
	else
		_chip_flush_miu_pipe();
#else
	_chip_flush_miu_pipe();
#endif
}

void Chip_Query_L2_Config(void)
{
}

unsigned int Chip_Query_Rev(void)
{
	unsigned int    dwRevisionId = 0;
	dwRevisionId = *((volatile unsigned int*)(0xFD003D9C));
	dwRevisionId >>= 8;

	return dwRevisionId;
}

#if 0 //modify later
unsigned int Chip_Query_CLK(void)
{
    return (12*reg_readw(0x1F22184c)); // 1 = 12(Hz)

}
#endif


void arch_enable_MsWDT_reset(unsigned long sec)
{
    *(volatile int*)(0xFD000000 + (0x300A << 1)) = 0x02dc;
    *(volatile int*)(0xFD000000 + (0x3008 << 1)) = 0x6700;
}

void arch_disable_MsWDT_reset(void)
{
    *(volatile int*)(0xFD000000+(0x300A<<1)) = 0x0;
    *(volatile int*)(0xFD000000+(0x3008<<1)) = 0x0;  // reg_top_sw_rst   
}


EXPORT_SYMBOL(Chip_Flush_Cache_Range);
EXPORT_SYMBOL(Chip_Flush_Cache_All);
EXPORT_SYMBOL(Chip_Clean_Cache_Range);
EXPORT_SYMBOL(Chip_Inv_Cache_Range);
EXPORT_SYMBOL(Chip_L2_cache_wback_inv);
EXPORT_SYMBOL(Chip_L2_cache_wback);
EXPORT_SYMBOL(Chip_L2_cache_inv);
EXPORT_SYMBOL(Chip_Flush_Memory);
EXPORT_SYMBOL(Chip_Flush_Memory_Range);
EXPORT_SYMBOL(Chip_Read_Memory);
EXPORT_SYMBOL(Chip_Read_Memory_Range);
EXPORT_SYMBOL(Chip_Flush_Cache_Range_VA_PA);
EXPORT_SYMBOL(Chip_Clean_Cache_Range_VA_PA);
EXPORT_SYMBOL(Chip_Inv_Cache_Range_VA_PA);

/**
 ** please refer to include/asm-arm/mach/arch.h for further details
 **
 ** #define MACHINE_START(_type,_name)				\
 ** static const struct machine_desc __mach_desc_##_type	\
 **  __used							\
 **   __attribute__((__section__(".arch.info.init"))) = {	\
 **   	.nr		= MACH_TYPE_##_type,			\
 **   	.name		= _name,
 **
 **
 ** #define MACHINE_END			\
 ** };
 **
 **
 **/

MACHINE_START(EINSTEIN, CONFIG_MSTAR_CHIP_NAME)

//    .atag_offset    = PHYS_OFFSET + 0x00000100,
    .atag_offset    = 0x00000100,
    .map_io = chip_map_io,
    .init_irq = chip_init_irq,
    .init_machine = chip_board_init,
    .handle_irq = gic_handle_irq,
    .smp = smp_ops(chip_smp_ops),
#ifdef CONFIG_GENERIC_CLOCKEVENTS
    .timer = &chip_timer,
#else
    .timer = &chip_ptimer,
#endif
MACHINE_END
