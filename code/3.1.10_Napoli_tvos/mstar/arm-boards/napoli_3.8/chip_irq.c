#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/types.h>

#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <mach/io.h>
#include <mach/platform.h>
#include <asm/mach-types.h>
#include <asm/setup.h>

#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>
#include <asm/hardware/gic.h>
#include <mstar/mpatch_macro.h>
#include "chip_int.h"

#if (MP_PLATFORM_INT_1_to_1_SPI == 1)
/*
 * 2 bits/per interrupt
 * b'00: level
 * b'10: edge
 */
unsigned int interrupt_configs[MSTAR_CHIP_INT_END/16] = {
    0x00000000, /*   0~ 15 sgi, don't care */
    0x00000000, /*  16~ 31 ppi, don't care */
    0x00000000, /*  32~ 47 spi, set level for mstar irq */
    0x00000000, /*  64~ 79 spi, set level for mstar irq */
    0x00000000, /*  64~ 79 spi, set level for mstar irq */
    0x00000000, /*  80~ 95 spi, set level for mstar irq */
    0xAAAAAAAA, /*  96~111 spi, set edge for mstar fiq */
    0xAAAAAAAA, /* 112~127 spi, set edge for mstar fiq */
    0xAAAAAAAA, /* 128~143 spi, set edge for mstar fiq */
    0xAAAAAAAA, /* 144~159 spi, set edge for mstar fiq */
                /* set the rest by init_chip_spi_config() */
};

#define BIT_PER_IRQ 2
#define IRQ_PER_UINT 16
#define LEVEL 0
#define EDGE 2
static inline void set_edge(unsigned int irq)
{
    interrupt_configs[irq/IRQ_PER_UINT] |= EDGE
        << ((irq % IRQ_PER_UINT) * BIT_PER_IRQ);
}

void init_chip_spi_config(void)
{
    set_edge(170); /* clock switch interrupt */
    set_edge(171); /* riu/xiu timerout interrupt */
    set_edge(172); /* scu event abort interrupt */

    /* neon/fpu exception flag */
    set_edge(174);
    set_edge(175);
    set_edge(176);
    set_edge(177);
    set_edge(178);
    set_edge(179);
    set_edge(180);

    /* neon/fpu exception flag */
    set_edge(183);
    set_edge(184);
    set_edge(185);
    set_edge(186);
    set_edge(187);
    set_edge(188);
    set_edge(189);

    /* neon/fpu exception flag */
    set_edge(192);
    set_edge(193);
    set_edge(194);
    set_edge(195);
    set_edge(196);
    set_edge(197);
    set_edge(198);

    /* neon/fpu exception flag */
    set_edge(201);
    set_edge(202);
    set_edge(203);
    set_edge(204);
    set_edge(205);
    set_edge(206);
    set_edge(207);
}
#endif/*MP_PLATFORM_INT_1_to_1_SPI*/

/*------------------------------------------------------------------------------
    Local Function Prototypes
-------------------------------------------------------------------------------*/
void chip_irq_ack(unsigned int irq);
void chip_irq_mask(unsigned int irq);
void chip_irq_unmask(unsigned int irq);
void __init chip_init_irq(void);

/* Clear FIQ (Clear is not supported for IRQ) */
void chip_irq_ack(unsigned int irq) {
    unsigned short tmp;
//printk(KERN_WARNING "chip_irq_ack(irq=%d)\n",irq);
        if(irq <16)
	{
	  tmp = (unsigned short)(0x01 << irq);
        reg_writew(tmp, REG_INT_BASE_PA + 0x2c*4);
	}
	else if((irq >= 16) && (irq < 32))
	{
	  tmp = (unsigned short)(0x01 << (irq-16));
        reg_writew(tmp, REG_INT_BASE_PA + 0x2d*4);
	}
	else if( (irq >= 32) && (irq < 48))
	{
	  tmp = (unsigned short)((0x01) << (irq - 32));
        reg_writew(tmp, REG_INT_BASE_PA + 0x2e*4);
	}
	else if( (irq >= 48) && (irq < 64))
	{
          tmp = (unsigned short)((0x01) << (irq - 48));
        reg_writew(tmp, REG_INT_BASE_PA + 0x2f*4);
        }
	/*
	else if( (irq >= 64) && (irq < 80))
	{
        tmp = reg_readw(REG_INT_BASE_PA + 0x3c*4);
    	tmp |= (0x01) << (irq - 64);
        reg_writew(tmp, REG_INT_BASE_PA + 0x3c*4);
        }
	else if((irq >= 80) && (irq < 96))
	{
        tmp = reg_readw(REG_INT_BASE_PA + 0x3d*4);
	    tmp |= 0x01 << (irq - 81);
        reg_writew(tmp, REG_INT_BASE_PA + 0x3d*4);
	}
	else if( (irq >= 96) && (irq < 112))
	{
        tmp = reg_readw(REG_INT_BASE_PA + 0x3e*4);
		tmp |= (0x01) << (irq - 97);
        reg_writew(tmp, REG_INT_BASE_PA + 0x3e*4);
	}
	else if( (irq >= 112) && (irq < 128))
	{
        tmp = reg_readw(REG_INT_BASE_PA + 0x3f*4);
    	tmp |= (0x01) << (irq - 112);
        reg_writew(tmp, REG_INT_BASE_PA + 0x3f*4);
	}
	*/
}

/* Mask IRQ/FIQ */
void chip_irq_mask(unsigned int irq) {
    unsigned short tmp;
//printk(KERN_WARNING "chip_irq_mask(irq=%d)\n",irq);
	if((irq <16) && (irq != 4) && (irq != 14))
	{
        tmp = reg_readw(REG_INT_BASE_PA + 0x24*4);
        tmp |= (unsigned short)((0x01)<<irq);
        reg_writew(tmp, REG_INT_BASE_PA + 0x24*4);
	}
	else if((irq >= 16) && (irq < 32))
	{
        tmp = reg_readw(REG_INT_BASE_PA + 0x25*4);
        tmp |= (unsigned short)((0x01) << (irq - 16));
        reg_writew(tmp, REG_INT_BASE_PA + 0x25*4);
	}
	else if( (irq >= 32) && (irq < 48))
	{
        tmp = reg_readw(REG_INT_BASE_PA + 0x26*4);
        tmp |= (unsigned short)((0x01) << (irq - 32));
        reg_writew(tmp, REG_INT_BASE_PA + 0x26*4); 
	}
	else if( (irq >= 48) && (irq < 64))
	{    
        tmp = reg_readw(REG_INT_BASE_PA + 0x27*4);
        tmp |= (unsigned short)((0x01) << (irq - 48));
        reg_writew(tmp, REG_INT_BASE_PA + 0x27*4);
	}
	else if((irq >= 64) && (irq < 80))
	{
        tmp = reg_readw(REG_INT_BASE_PA + 0x34*4);
        tmp |= (unsigned short)((0x01) << (irq - 64));
        reg_writew(tmp, REG_INT_BASE_PA + 0x34*4);
	}
	else if((irq >= 80) && (irq < 96))
	{
        tmp = reg_readw(REG_INT_BASE_PA + 0x35*4);
        tmp |= (unsigned short)((0x01) << (irq - 80));
        reg_writew(tmp, REG_INT_BASE_PA + 0x35*4);
	}
	else if( (irq >= 96) && (irq < 112))
	{
        tmp = reg_readw(REG_INT_BASE_PA + 0x36*4);
        tmp |= (unsigned short)((0x01) << (irq - 96));
        reg_writew(tmp, REG_INT_BASE_PA + 0x36*4);
	}
	else if( (irq >= 112) && (irq < 128))
	{
        tmp = reg_readw(REG_INT_BASE_PA + 0x37*4);
        tmp |= (unsigned short)((0x01) << (irq - 112));
        reg_writew(tmp, REG_INT_BASE_PA + 0x37*4);
	}

}

/* Un-Mask IRQ/FIQ */
void chip_irq_unmask(unsigned int irq) {
   unsigned short tmp;

    //printk(KERN_WARNING "chip_irq_unmask(irq=%d)\n",irq);

    if(irq < 16)
    {
        tmp = reg_readw(REG_INT_BASE_PA + 0x24*4);
        tmp &= (unsigned short)(~((0x01) << irq));
        reg_writew(tmp, REG_INT_BASE_PA + 0x24*4);
    }
    
    else if((irq >= 16) && (irq < 32))
    {
        tmp = reg_readw(REG_INT_BASE_PA + 0x25*4);
        tmp &= (unsigned short)(~((0x01) << (irq -16)));
        reg_writew(tmp, REG_INT_BASE_PA + 0x25*4);
    }
    else if((irq >= 32) && (irq < 48))
    {
        tmp = reg_readw(REG_INT_BASE_PA + 0x26*4);
        tmp &= (unsigned short)(~((0x01) << (irq-32)));
        reg_writew(tmp, REG_INT_BASE_PA + 0x26*4);
    }
    else if((irq >= 48) && (irq < 64))
    {
        tmp = reg_readw(REG_INT_BASE_PA + 0x27*4);
        tmp &= (unsigned short)(~((0x01) << (irq -48)));
        reg_writew(tmp, REG_INT_BASE_PA + 0x27*4);
    }
    	else if((irq >= 64) && (irq < 80))
	{
        tmp = reg_readw(REG_INT_BASE_PA + 0x34*4);
        tmp &= (unsigned short)(~((0x01) << (irq - 64)));
        reg_writew(tmp, REG_INT_BASE_PA + 0x34*4);
	}
	else if((irq >= 80) && (irq < 96))
	{
        tmp = reg_readw(REG_INT_BASE_PA + 0x35*4);
        tmp &= (unsigned short)(~((0x01) << (irq - 80)));
        reg_writew(tmp, REG_INT_BASE_PA + 0x35*4);
	}
	else if( (irq >= 96) && (irq < 112))
	{
        tmp = reg_readw(REG_INT_BASE_PA + 0x36*4);
        tmp &= (unsigned short)(~((0x01) << (irq - 96)));
        reg_writew(tmp, REG_INT_BASE_PA + 0x36*4);
	}
	else if( (irq >= 112) && (irq < 128))
	{
        tmp = reg_readw(REG_INT_BASE_PA + 0x37*4);
        tmp &= (unsigned short)(~((0x01) << (irq - 112)));
        reg_writew(tmp, REG_INT_BASE_PA + 0x37*4);
    }
}

void __iomem *_gic_cpu_base_addr=(void __iomem *)(0xFC000000 + 0x0100);
void __iomem *_gic_dist_base_addr=(void __iomem *)(0xFC000000 + 0x1000);

extern void gic_dist_init(unsigned int gic_nr, void __iomem *base, unsigned int irq_start);
extern void gic_cpu_init(unsigned int gic_nr, void __iomem *base);
extern void arm_interrupt_chain_setup(int chain_num);

#if (MP_PLATFORM_INT_1_to_1_SPI != 1)
// switch FIQ/IRQ merge bit
/*static*/ int __init init_irq_fiq_merge(void)
{
	unsigned short tmp = 0;

	tmp = reg_readw(0x1f2472c8);
	tmp &= 0xFFDF;
	tmp |= 0x0050;
	reg_writew(tmp, 0x1f2472c8);

	return 0;
}
#endif/*MP_PLATFORM_INT_1_to_1_SPI*/

void __init chip_init_irq(void)
{
	gic_init(0,29,_gic_dist_base_addr,_gic_cpu_base_addr);

#if (MP_PLATFORM_INT_1_to_1_SPI != 1)
	init_irq_fiq_merge();
	arm_interrupt_chain_setup(INT_PPI_IRQ);
#endif/*MP_PLATFORM_INT_1_to_1_SPI*/
}
