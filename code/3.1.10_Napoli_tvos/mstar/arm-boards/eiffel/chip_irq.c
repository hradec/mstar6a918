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
#include <asm/mach-types.h>
#include <asm/setup.h>

#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>
#include <asm/hardware/gic.h>

#include "chip_int.h"

/* Clear FIQ (Clear is not supported for IRQ) */
void chip_irq_ack(unsigned int irq)
{
    unsigned short tmp;

    if(irq <16)
    {
        tmp = (0x01 << irq);
        reg_writew(tmp, REG_INT_BASE_PA + (0x2c << 2));
    }
    else if((irq >= 16) && (irq < 32))
    {
      tmp = (0x01 << (irq - 16));
        reg_writew(tmp, REG_INT_BASE_PA + (0x2d << 2));
    }
    else if( (irq >= 32) && (irq < 48))
    {
      tmp = (0x01) << (irq - 32);
        reg_writew(tmp, REG_INT_BASE_PA + (0x2e << 2));
    }
    else if( (irq >= 48) && (irq < 64))
    {
        tmp = (0x01) << (irq - 48);
        reg_writew(tmp, REG_INT_BASE_PA + (0x2f << 2));
    }
#if 0
    else if( (irq >= 64) && (irq < 80))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x3c << 2));
        tmp |= (0x01) << (irq - 64);
        reg_writew(tmp, REG_INT_BASE_PA + (0x3c << 2));
        }
    else if((irq >= 80) && (irq < 96))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x3d << 2));
        tmp |= 0x01 << (irq - 81);
        reg_writew(tmp, REG_INT_BASE_PA + (0x3d << 2));
    }
    else if( (irq >= 96) && (irq < 112))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x3e << 2));
        tmp |= (0x01) << (irq - 97);
        reg_writew(tmp, REG_INT_BASE_PA + (0x3e << 2));
    }
    else if( (irq >= 112) && (irq < 128))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x3f << 2));
        tmp |= (0x01) << (irq - 112);
        reg_writew(tmp, REG_INT_BASE_PA + (0x3f << 2));
    }
#endif
}

/* Mask IRQ/FIQ */
void chip_irq_mask(unsigned int irq)
{
    unsigned short tmp;

    if(irq <16)
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x24 << 2));
        tmp |= (0x01) << irq;
        reg_writew(tmp, REG_INT_BASE_PA + (0x24 << 2));
    }
    else if((irq >= 16) && (irq < 32))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x25 << 2));
        tmp |= (0x01) << (irq - 16);
        reg_writew(tmp, REG_INT_BASE_PA + (0x25 << 2));
    }
    else if( (irq >= 32) && (irq < 48))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x26 << 2));
        tmp |= (0x01) << (irq - 32);
        reg_writew(tmp, REG_INT_BASE_PA + (0x26 << 2));
    }
    else if( (irq >= 48) && (irq < 64))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x27 << 2));
        tmp |= (0x01) << (irq - 48);
        reg_writew(tmp, REG_INT_BASE_PA + (0x27 << 2));
    }
    else if((irq >= 64) && (irq < 80))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x34 << 2));
        tmp |= (0x01) << (irq - 64);
        reg_writew(tmp, REG_INT_BASE_PA + (0x34 << 2));
    }
    else if((irq >= 80) && (irq < 96))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x35 << 2));
        tmp |= (0x01) << (irq - 80);
        reg_writew(tmp, REG_INT_BASE_PA + (0x35 << 2));
    }
    else if( (irq >= 96) && (irq < 112))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x36 << 2));
        tmp |= (0x01) << (irq - 96);
        reg_writew(tmp, REG_INT_BASE_PA + (0x36 << 2));
    }
    else if( (irq >= 112) && (irq < 128))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x37 << 2));
        tmp |= (0x01) << (irq - 112);
        reg_writew(tmp, REG_INT_BASE_PA + (0x37 << 2));
    }

}

/* Un-Mask IRQ/FIQ */
void chip_irq_unmask(unsigned int irq)
{
   unsigned short tmp;

    //printk(KERN_WARNING "chip_irq_unmask(irq=%d)\n",irq);

    if(irq < 16)
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x24 << 2));
        tmp &= ~((0x01) << irq);
        reg_writew(tmp, REG_INT_BASE_PA + (0x24 << 2));
    }

    else if((irq >= 16) && (irq < 32))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x25 << 2));
        tmp &= ~((0x01) << (irq - 16));
        reg_writew(tmp, REG_INT_BASE_PA + (0x25 << 2));
    }
    else if((irq >= 32) && (irq < 48))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x26 << 2));
        tmp &= ~((0x01) << (irq - 32));
        reg_writew(tmp, REG_INT_BASE_PA + (0x26 << 2));
    }
    else if((irq >= 48) && (irq < 64))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x27 << 2));
        tmp &= ~((0x01) << (irq - 48));
        reg_writew(tmp, REG_INT_BASE_PA + (0x27 << 2));
    }
        else if((irq >= 64) && (irq < 80))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x34 << 2));
        tmp &= ~((0x01) << (irq - 64));
        reg_writew(tmp, REG_INT_BASE_PA + (0x34 << 2));
    }
    else if((irq >= 80) && (irq < 96))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x35 << 2));
        tmp &= ~((0x01) << (irq - 80));
        reg_writew(tmp, REG_INT_BASE_PA + (0x35 << 2));
    }
    else if( (irq >= 96) && (irq < 112))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x36 << 2));
        tmp &= ~((0x01) << (irq - 96));
        reg_writew(tmp, REG_INT_BASE_PA + (0x36 << 2));
    }
    else if( (irq >= 112) && (irq < 128))
    {
        tmp = reg_readw(REG_INT_BASE_PA + (0x37 << 2));
        tmp &= ~((0x01) << (irq - 112));
        reg_writew(tmp, REG_INT_BASE_PA + (0x37 << 2));
    }
}


void __iomem *_gic_cpu_base_addr = (void __iomem *) (0xFC000000 + 0x0100);
void __iomem *_gic_dist_base_addr = (void __iomem *) (0xFC000000 + 0x1000);

extern void gic_dist_init(unsigned int gic_nr, void __iomem *base, unsigned int irq_start);
extern void gic_cpu_init(unsigned int gic_nr, void __iomem *base);
extern void arm_interrupt_chain_setup(int chain_num);

void __init chip_init_irq(void)
{
#ifndef CONFIG_MP_PLATFORM_MSTAR_LEGANCY_INTR
    unsigned long temp;
#endif

#if 1
    gic_init(0, 29, _gic_dist_base_addr, _gic_cpu_base_addr);
#else
    gic_dist_init(0, gic_dist_base_addr, 29);
    gic_cpu_init(0, gic_cpu_base_addr);
#endif

#if defined(CONFIG_MP_PLATFORM_MSTAR_LEGANCY_INTR)
    arm_interrupt_chain_setup(INT_PPI_IRQ);
#else
    //GIC Interrupt Set Enable Register for MSTAR controller
    temp=PERI_R(GIC_DIST_SET_EANBLE);
    temp= temp | (0x01 << INT_PPI_IRQ );
    PERI_W(GIC_DIST_SET_EANBLE,temp);
#endif /* CONFIG_MP_PLATFORM_MSTAR_LEGANCY_INTR */

}

// switch FIQ/IRQ merge bit
int __init init_irq_fiq_merge(void)
{
#if 0 //When ARM been switch to non-secure mode, interrupt setting can't be touch, so we remove it.
    unsigned short tmp;

    tmp = reg_readw(0x1f000000 + (0x123964 << 1));
    tmp &= 0xFFDF;
    tmp |= 0x0050;
    reg_writew(tmp, (0x1f000000 + (0x123964 << 1)));
#endif
    return 0;
}
