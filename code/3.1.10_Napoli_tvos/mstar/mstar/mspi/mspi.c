/*
 * Driver for mstar mspi controller
 *
 * Copyright (C) 2006 Mstar Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>
#include <linux/slab.h>

#include "drvMSPI.h"
#include "halMSPI.h"
/* SPI register offsets */
#define SPI_CR					0x0000
#define SPI_MR					0x0004
#define SPI_RDR					0x0008
#define SPI_TDR					0x000c
#define SPI_SR					0x0010
#define SPI_IER					0x0014
#define SPI_IDR					0x0018
#define SPI_IMR					0x001c
#define SPI_CSR0				0x0030
#define SPI_CSR1				0x0034
#define SPI_CSR2				0x0038
#define SPI_CSR3				0x003c
#define SPI_RPR					0x0100
#define SPI_RCR					0x0104
#define SPI_TPR					0x0108
#define SPI_TCR					0x010c
#define SPI_RNPR				0x0110
#define SPI_RNCR				0x0114
#define SPI_TNPR				0x0118
#define SPI_TNCR				0x011c
#define SPI_PTCR				0x0120
#define SPI_PTSR				0x0124

/* Bitfields in CR */
#define SPI_SPIEN_OFFSET			0
#define SPI_SPIEN_SIZE				1
#define SPI_SPIDIS_OFFSET			1
#define SPI_SPIDIS_SIZE				1
#define SPI_SWRST_OFFSET			7
#define SPI_SWRST_SIZE				1
#define SPI_LASTXFER_OFFSET			24
#define SPI_LASTXFER_SIZE			1

/* Bitfields in MR */
#define SPI_MSTR_OFFSET				0
#define SPI_MSTR_SIZE				1
#define SPI_PS_OFFSET				1
#define SPI_PS_SIZE				1
#define SPI_PCSDEC_OFFSET			2
#define SPI_PCSDEC_SIZE				1
#define SPI_FDIV_OFFSET				3
#define SPI_FDIV_SIZE				1
#define SPI_MODFDIS_OFFSET			4
#define SPI_MODFDIS_SIZE			1
#define SPI_LLB_OFFSET				7
#define SPI_LLB_SIZE				1
#define SPI_PCS_OFFSET				16
#define SPI_PCS_SIZE				4
#define SPI_DLYBCS_OFFSET			24
#define SPI_DLYBCS_SIZE				8

/* Bitfields in RDR */
#define SPI_RD_OFFSET				0
#define SPI_RD_SIZE				16

/* Bitfields in TDR */
#define SPI_TD_OFFSET				0
#define SPI_TD_SIZE				16

/* Bitfields in SR */
#define SPI_RDRF_OFFSET				0
#define SPI_RDRF_SIZE				1
#define SPI_TDRE_OFFSET				1
#define SPI_TDRE_SIZE				1
#define SPI_MODF_OFFSET				2
#define SPI_MODF_SIZE				1
#define SPI_OVRES_OFFSET			3
#define SPI_OVRES_SIZE				1
#define SPI_ENDRX_OFFSET			4
#define SPI_ENDRX_SIZE				1
#define SPI_ENDTX_OFFSET			5
#define SPI_ENDTX_SIZE				1
#define SPI_RXBUFF_OFFSET			6
#define SPI_RXBUFF_SIZE				1
#define SPI_TXBUFE_OFFSET			7
#define SPI_TXBUFE_SIZE				1
#define SPI_NSSR_OFFSET				8
#define SPI_NSSR_SIZE				1
#define SPI_TXEMPTY_OFFSET			9
#define SPI_TXEMPTY_SIZE			1
#define SPI_SPIENS_OFFSET			16
#define SPI_SPIENS_SIZE				1

/* Bitfields in CSR0 */
#define SPI_CPOL_OFFSET				0
#define SPI_CPOL_SIZE				1
#define SPI_NCPHA_OFFSET			1
#define SPI_NCPHA_SIZE				1
#define SPI_CSAAT_OFFSET			3
#define SPI_CSAAT_SIZE				1
#define SPI_BITS_OFFSET				4
#define SPI_BITS_SIZE				4
#define SPI_SCBR_OFFSET				8
#define SPI_SCBR_SIZE				8
#define SPI_DLYBS_OFFSET			16
#define SPI_DLYBS_SIZE				8
#define SPI_DLYBCT_OFFSET			24
#define SPI_DLYBCT_SIZE				8

/* Bitfields in RCR */
#define SPI_RXCTR_OFFSET			0
#define SPI_RXCTR_SIZE				16

/* Bitfields in TCR */
#define SPI_TXCTR_OFFSET			0
#define SPI_TXCTR_SIZE				16

/* Bitfields in RNCR */
#define SPI_RXNCR_OFFSET			0
#define SPI_RXNCR_SIZE				16

/* Bitfields in TNCR */
#define SPI_TXNCR_OFFSET			0
#define SPI_TXNCR_SIZE				16

/* Bitfields in PTCR */
#define SPI_RXTEN_OFFSET			0
#define SPI_RXTEN_SIZE				1
#define SPI_RXTDIS_OFFSET			1
#define SPI_RXTDIS_SIZE				1
#define SPI_TXTEN_OFFSET			8
#define SPI_TXTEN_SIZE				1
#define SPI_TXTDIS_OFFSET			9
#define SPI_TXTDIS_SIZE				1

/* Constants for BITS */
#define SPI_BITS_8_BPT				0
#define SPI_BITS_9_BPT				1
#define SPI_BITS_10_BPT				2
#define SPI_BITS_11_BPT				3
#define SPI_BITS_12_BPT				4
#define SPI_BITS_13_BPT				5
#define SPI_BITS_14_BPT				6
#define SPI_BITS_15_BPT				7
#define SPI_BITS_16_BPT				8

/* Bit manipulation macros */
#define SPI_BIT(name) \
	(1 << SPI_##name##_OFFSET)
#define SPI_BF(name,value) \
	(((value) & ((1 << SPI_##name##_SIZE) - 1)) << SPI_##name##_OFFSET)
#define SPI_BFEXT(name,value) \
	(((value) >> SPI_##name##_OFFSET) & ((1 << SPI_##name##_SIZE) - 1))
#define SPI_BFINS(name,value,old) \
	( ((old) & ~(((1 << SPI_##name##_SIZE) - 1) << SPI_##name##_OFFSET)) \
	  | SPI_BF(name,value))

/* Register access macros */
#if 0
#define spi_readl(port,reg) \
     __raw_readl((port)->regs + SPI_##reg)
#define spi_writel(port,reg,value) \
     __raw_writel((value), (port)->regs + SPI_##reg)
#endif

/*
 * The core SPI transfer engine just talks to a register bank to set up
 * DMA transfers; transfer queue progress is driven by IRQs.  The clock
 * framework provides the base clock, subdivided for each spi_device.
 */
struct mspi_spi {
    spinlock_t    lock;

//    void __iomem  *regs;
//    int           irq;
//    struct clk    *clk;
    struct platform_device  *pdev;
    struct spi_device   *stay;
    MSPI_DCConfig        *dc;
    MSPI_FrameConfig     *frame;
    MSPI_CLKConfig       *clk;
    u8     stopping;
    struct list_head    queue;
    struct spi_transfer *current_transfer;
    unsigned long       current_remaining_bytes;
    struct spi_transfer *next_transfer;
    unsigned long       next_remaining_bytes;
    void            *buffer;
};

/* Controller-specific per-slave state */
struct mspi_spi_device 
{
    unsigned int  npcs_pin;
    u32           csr;
};

#define BUFFER_SIZE         PAGE_SIZE
#define INVALID_DMA_ADDRESS 0xffffffff

static void cs_activate(struct mspi_spi *ms, struct spi_device *spi)
{
    HAL_MSPI_SlaveEnable(TRUE);
}

static void cs_deactivate(struct mspi_spi *ms, struct spi_device *spi)
{
//    struct mspi_spi_device *msd = spi->controller_state;
//    unsigned active = spi->mode & SPI_CS_HIGH;
    HAL_MSPI_SlaveEnable(FALSE);
}

static inline int mspi_spi_xfer_is_last(struct spi_message *msg,
                                           struct spi_transfer *xfer)
{
    return msg->transfers.prev == &xfer->transfer_list;
}

static inline int mspi_spi_xfer_can_be_chained(struct spi_transfer *xfer)
{
    return xfer->delay_usecs == 0 && !xfer->cs_change;
}

static void mspi_spi_next_xfer_data(struct spi_master *master,
                struct spi_transfer *xfer,
                dma_addr_t *tx_dma,
                dma_addr_t *rx_dma,
                u32 *plen)
{
    struct mspi_spi     *ms = spi_master_get_devdata(master);
    u32                 len = *plen;

    /* use scratch buffer only when rx or tx data is unspecified */
    if (xfer->rx_buf)
        *rx_dma = xfer->rx_dma + xfer->len - *plen;
    else
    {
        if (len > BUFFER_SIZE)
            len = BUFFER_SIZE;
    }
    if (xfer->tx_buf)
        *tx_dma = xfer->tx_dma + xfer->len - *plen;
    else
    {
        if (len > BUFFER_SIZE)
            len = BUFFER_SIZE;
        memset(ms->buffer, 0, len);
    }
    *plen = len;
}

/*
 * Submit next transfer for DMA.
 * lock is held, spi irq is blocked
 */
static void mspi_spi_next_xfer(struct spi_master *master,
            struct spi_message *msg)
{
    struct mspi_spi     *ms = spi_master_get_devdata(master);
    struct spi_transfer *xfer;
    u32                 len, remaining;
    u32                 ieval;
    dma_addr_t		tx_dma, rx_dma;

    if (!ms->current_transfer)
        xfer = list_entry(msg->transfers.next,
        struct spi_transfer, transfer_list);
    else if (!ms->next_transfer)
        xfer = list_entry(ms->current_transfer->transfer_list.next,
                struct spi_transfer, transfer_list);
    else
        xfer = NULL;

    if (xfer)
    {
        len = xfer->len;
        mspi_spi_next_xfer_data(master, xfer, &tx_dma, &rx_dma, &len);
        remaining = xfer->len - len;

        if (msg->spi->bits_per_word > 8)
            len >>= 1;


        dev_dbg(&msg->spi->dev,
            "  start xfer %p: len %u tx %p/%08x rx %p/%08x\n",
            xfer, xfer->len, xfer->tx_buf, xfer->tx_dma,
            xfer->rx_buf, xfer->rx_dma);
    }
    else
    {
        xfer = ms->next_transfer;
        remaining = ms->next_remaining_bytes;
    }

    ms->current_transfer = xfer;
    ms->current_remaining_bytes = remaining;

    if (remaining > 0)
        len = remaining;
    else if (!mspi_spi_xfer_is_last(msg, xfer)
             && mspi_spi_xfer_can_be_chained(xfer))
    {
        xfer = list_entry(xfer->transfer_list.next,
               struct spi_transfer, transfer_list);
        len = xfer->len;
    }
    else
        xfer = NULL;

    ms->next_transfer = xfer;

    if (xfer)
    {
        u32 total;

        total = len;
        mspi_spi_next_xfer_data(master, xfer, &tx_dma, &rx_dma, &len);
        ms->next_remaining_bytes = total - len;

        if (msg->spi->bits_per_word > 8)
            len >>= 1;


        dev_dbg(&msg->spi->dev,
        "  next xfer %p: len %u tx %p/%08x rx %p/%08x\n",
        xfer, xfer->len, xfer->tx_buf, xfer->tx_dma,
        xfer->rx_buf, xfer->rx_dma);
        ieval = SPI_BIT(ENDRX) | SPI_BIT(OVRES);
    }
    else
    {
        ieval = SPI_BIT(RXBUFF) | SPI_BIT(ENDRX) | SPI_BIT(OVRES);
    }
}

static void mspi_spi_next_message(struct spi_master *master)
{
    struct mspi_spi     *ms = spi_master_get_devdata(master);
    struct spi_message  *msg;
    struct spi_device   *spi;

    BUG_ON(ms->current_transfer);

    msg = list_entry(ms->queue.next, struct spi_message, queue);
    spi = msg->spi;

    dev_dbg(master->dev.parent, "start message %p for %s\n",
            msg, dev_name(&spi->dev));

    /* select chip if it's not still active */
    if (ms->stay)
    {
        if (ms->stay != spi)
        {
            cs_deactivate(ms, ms->stay);
            cs_activate(ms, spi);
        }
        ms->stay = NULL;
    }
    else
        cs_activate(ms, spi);

    mspi_spi_next_xfer(master, msg);
}

/*
 * For DMA, tx_buf/tx_dma have the same relationship as rx_buf/rx_dma:
 *  - The buffer is either valid for CPU access, else NULL
 *  - If the buffer is valid, so is its DMA address
 *
 * This driver manages the dma address unless message->is_dma_mapped.
 */
#if 0
static int
mspi_spi_dma_map_xfer(struct mspi_spi *ms, struct spi_transfer *xfer)
{
    struct device   *dev = &ms->pdev->dev;

    xfer->tx_dma = xfer->rx_dma = INVALID_DMA_ADDRESS;
    if (xfer->tx_buf)
    {
         /* tx_buf is a const void* where we need a void * for the dma
               * mapping */
         void *nonconst_tx = (void *)xfer->tx_buf;

         xfer->tx_dma = dma_map_single(dev,
                nonconst_tx, xfer->len,
                DMA_TO_DEVICE);
         if (dma_mapping_error(dev, xfer->tx_dma))
             return -ENOMEM;

        if (xfer->rx_buf)
        {
            xfer->rx_dma = dma_map_single(dev,
                xfer->rx_buf, xfer->len,
                DMA_FROM_DEVICE);
            if (dma_mapping_error(dev, xfer->rx_dma))
            {
                if (xfer->tx_buf)
                        dma_unmap_single(dev,
                        xfer->tx_dma, xfer->len,
                        DMA_TO_DEVICE);
                    return -ENOMEM;
            }
        }
       return 0;

}

static void mspi_spi_dma_unmap_xfer(struct spi_master *master,
                    struct spi_transfer *xfer)
{
     if (xfer->tx_dma != INVALID_DMA_ADDRESS)
        dma_unmap_single(master->dev.parent, xfer->tx_dma,
                 xfer->len, DMA_TO_DEVICE);
     if (xfer->rx_dma != INVALID_DMA_ADDRESS)
        dma_unmap_single(master->dev.parent, xfer->rx_dma,
                 xfer->len, DMA_FROM_DEVICE);
}
#endif

static void mspi_spi_msg_done(struct spi_master *master, struct mspi_spi *ms,
        struct spi_message *msg, int status, int stay)
{
    if (!stay || status < 0)
        cs_deactivate(ms, msg->spi);
    else
        ms->stay = msg->spi;

    list_del(&msg->queue);
    msg->status = status;

    dev_dbg(master->dev.parent,
        "xfer complete: %u bytes transferred\n",
        msg->actual_length);

    spin_unlock(&ms->lock);
    msg->complete(msg->context);
    spin_lock(&ms->lock);

    ms->current_transfer = NULL;
    ms->next_transfer = NULL;

    /* continue if needed */
    if (!(list_empty(&ms->queue) || ms->stopping))
       mspi_spi_next_message(master);
}
#if 0
static irqreturn_t
mspi_spi_interrupt(int irq, void *dev_id)
{
    struct spi_master	*master = dev_id;
    struct mspi_spi	*ms = spi_master_get_devdata(master);
    struct spi_message	*msg;
    struct spi_transfer	*xfer;
    u32    status, pending, imr;
    int    ret = IRQ_NONE;

    spin_lock(&as->lock);

    xfer = as->current_transfer;
    msg = list_entry(as->queue.next, struct spi_message, queue);

    imr = spi_readl(as, IMR);
    status = spi_readl(as, SR);
    pending = status & imr;

    if (pending & SPI_BIT(OVRES)) {
        int timeout;

        ret = IRQ_HANDLED;

        spi_writel(as, IDR, (SPI_BIT(RXBUFF) | SPI_BIT(ENDRX)
                  | SPI_BIT(OVRES)));

		/*
		 * When we get an overrun, we disregard the current
		 * transfer. Data will not be copied back from any
		 * bounce buffer and msg->actual_len will not be
		 * updated with the last xfer.
		 *
		 * We will also not process any remaning transfers in
		 * the message.
		 *
		 * First, stop the transfer and unmap the DMA buffers.
		 */
		spi_writel(as, PTCR, SPI_BIT(RXTDIS) | SPI_BIT(TXTDIS));
		if (!msg->is_dma_mapped)
			atmel_spi_dma_unmap_xfer(master, xfer);

		/* REVISIT: udelay in irq is unfriendly */
		if (xfer->delay_usecs)
			udelay(xfer->delay_usecs);

		dev_warn(master->dev.parent, "overrun (%u/%u remaining)\n",
			 spi_readl(as, TCR), spi_readl(as, RCR));

		/*
		 * Clean up DMA registers and make sure the data
		 * registers are empty.
		 */
		spi_writel(as, RNCR, 0);
		spi_writel(as, TNCR, 0);
		spi_writel(as, RCR, 0);
		spi_writel(as, TCR, 0);
		for (timeout = 1000; timeout; timeout--)
			if (spi_readl(as, SR) & SPI_BIT(TXEMPTY))
				break;
		if (!timeout)
			dev_warn(master->dev.parent,
				 "timeout waiting for TXEMPTY");
		while (spi_readl(as, SR) & SPI_BIT(RDRF))
			spi_readl(as, RDR);

		/* Clear any overrun happening while cleaning up */
		spi_readl(as, SR);

		atmel_spi_msg_done(master, as, msg, -EIO, 0);
	} else if (pending & (SPI_BIT(RXBUFF) | SPI_BIT(ENDRX))) {
		ret = IRQ_HANDLED;

		spi_writel(as, IDR, pending);

		if (as->current_remaining_bytes == 0) {
			msg->actual_length += xfer->len;

			if (!msg->is_dma_mapped)
				atmel_spi_dma_unmap_xfer(master, xfer);

			/* REVISIT: udelay in irq is unfriendly */
			if (xfer->delay_usecs)
				udelay(xfer->delay_usecs);

			if (atmel_spi_xfer_is_last(msg, xfer)) {
				/* report completed message */
				atmel_spi_msg_done(master, as, msg, 0,
						xfer->cs_change);
			} else {
				if (xfer->cs_change) {
					cs_deactivate(as, msg->spi);
					udelay(1);
					cs_activate(as, msg->spi);
				}

				/*
				 * Not done yet. Submit the next transfer.
				 *
				 * FIXME handle protocol options for xfer
				 */
				atmel_spi_next_xfer(master, msg);
			}
		} else {
			/*
			 * Keep going, we still have data to send in
			 * the current transfer.
			 */
			atmel_spi_next_xfer(master, msg);
		}
	}

	spin_unlock(&as->lock);

	return ret;
}
#endif
static int mspi_spi_setup(struct spi_device *spi)
{
    struct mspi_spi         *ms;
    struct mspi_spi_device  *msd;
    u32                      scbr, csr;
    unsigned int             bits = spi->bits_per_word;
//    unsigned long            bus_hz;
    unsigned int             npcs_pin;
//    int                      ret;


    ms = spi_master_get_devdata(spi->master);

    if (ms->stopping)
        return -ESHUTDOWN;

    if (spi->chip_select > spi->master->num_chipselect) {
        dev_dbg(&spi->dev,
                "setup: invalid chipselect %u (%u defined)\n",
                spi->chip_select, spi->master->num_chipselect);
        return -EINVAL;
    }
    //set chip select number
    HAL_MSPI_SetChipSelect(0);
    if (bits < 1 || bits > 8) {
        dev_dbg(&spi->dev,
                "setup: invalid bits_per_word %u (1 to 8)\n",
                bits);
        return -EINVAL;
    }

    /* see notes above re chipselect */
    if (spi->chip_select == 0 && (spi->mode & SPI_CS_HIGH))
    {
        dev_dbg(&spi->dev, "setup: can't be active-high\n");
        return -EINVAL;
    }

    /* chips start out at half the peripheral bus speed. */
    #if 0
    bus_hz = clk_get_rate(as->clk);
    bus_hz /= 2;

    if (spi->max_speed_hz) {
        /*
               * Calculate the lowest divider that satisfies the
               * constraint, assuming div32/fdiv/mbz == 0.
               */
        scbr = DIV_ROUND_UP(bus_hz, spi->max_speed_hz);

        /*
               * If the resulting divider doesn't fit into the
               * register bitfield, we can't satisfy the constraint.
               */
        if (scbr >= (1 << SPI_SCBR_SIZE)) {
            dev_dbg(&spi->dev,
                "setup: %d Hz too slow, scbr %u; min %ld Hz\n",
                spi->max_speed_hz, scbr, bus_hz/255);
            return -EINVAL;
        }
    } else
        /* speed zero means "as slow as possible" */
        scbr = 0xff;
    #endif
    csr = SPI_BF(SCBR, scbr) | SPI_BF(BITS, bits - 8);

    if (spi->mode & SPI_CPOL)
    {
        if(!ms->clk)
            ms->clk = kzalloc(sizeof(MSPI_CLKConfig), GFP_KERNEL);
        ms->clk->BClkPolarity = 1;
    }
    if (!(spi->mode & SPI_CPHA))
    {
        if(!ms->clk)
            ms->clk = kzalloc(sizeof(MSPI_CLKConfig), GFP_KERNEL);
        ms->clk->BClkPhase = 1;
    }
    /* DLYBS is mostly irrelevant since we manage chipselect using GPIOs.
        *
        * DLYBCT would add delays between words, slowing down transfers.
        * It could potentially be useful to cope with DMA bottlenecks, but
        * in those cases it's probably best to just use a lower bitrate.
        */
    csr |= SPI_BF(DLYBS, 0);
    csr |= SPI_BF(DLYBCT, 0);

    /* chipselect must have been muxed as GPIO (e.g. in board setup) */
    npcs_pin = (unsigned int)spi->controller_data;
    msd = spi->controller_state;
    if (!msd)
    {
        msd = kzalloc(sizeof(struct mspi_spi_device), GFP_KERNEL);
        if (!msd)
            return -ENOMEM;
#if 0  // no need cs control by mspi riu register
        ret = gpio_request(npcs_pin, dev_name(&spi->dev));
        if (ret) {
            kfree(asd);
            return ret;
        }

        msd->npcs_pin = npcs_pin;
        spi->controller_state = msd;
        gpio_direction_output(npcs_pin, !(spi->mode & SPI_CS_HIGH));
#endif
    }
    else
    {
        unsigned long     flags;

        spin_lock_irqsave(&ms->lock, flags);
        if (ms->stay == spi)
            ms->stay = NULL;
        cs_deactivate(ms, spi);
        spin_unlock_irqrestore(&ms->lock, flags);
     }

     msd->csr = csr;
    return 0;
}

static int mspi_spi_transfer(struct spi_device *spi, struct spi_message *msg)
{
    struct mspi_spi         *ms;
    struct spi_transfer     *xfer;
    unsigned long           flags;
    struct device           *controller = spi->master->dev.parent;
    u8                      bits;
    struct mspi_spi_device  *msd;

    ms = spi_master_get_devdata(spi->master);

    dev_dbg(controller, "new message %p submitted for %s\n",
            msg, dev_name(&spi->dev));

    if (unlikely(list_empty(&msg->transfers)))
        return -EINVAL;

    if (ms->stopping)
        return -ESHUTDOWN;

    list_for_each_entry(xfer, &msg->transfers, transfer_list)
    {
        if (!(xfer->tx_buf || xfer->rx_buf) && xfer->len)
        {
            dev_dbg(&spi->dev, "missing rx or tx buf\n");
            return -EINVAL;
        }

        if (xfer->bits_per_word)
        {
            msd = spi->controller_state;
            bits = (msd->csr >> 4) & 0xf;
            if (bits != xfer->bits_per_word - 8)
            {
                dev_dbg(&spi->dev, "you can't yet change "
                    "bits_per_word in transfers\n");
                return -ENOPROTOOPT;
            }
        }
#if 0
        /* FIXME implement these protocol options!! */
        if (xfer->speed_hz)
        {
            dev_dbg(&spi->dev, "no protocol options yet\n");
            return -ENOPROTOOPT;
        }

        /*
                * DMA map early, for performance (empties dcache ASAP) and
                * better fault reporting.  This is a DMA-only driver.
                *
                * NOTE that if dma_unmap_single() ever starts to do work on
                * platforms supported by this driver, we would need to clean
                * up mappings for previously-mapped transfers.
                */
        if (!msg->is_dma_mapped) {
            if (atmel_spi_dma_map_xfer(as, xfer) < 0)
                return -ENOMEM;
        }
#endif

    }
 

#ifdef VERBOSE
    list_for_each_entry(xfer, &msg->transfers, transfer_list)
    {
        dev_dbg(controller,
            "  xfer %p: len %u tx %p/%08x rx %p/%08x\n",
            xfer, xfer->len,
            xfer->tx_buf, xfer->tx_dma,
            xfer->rx_buf, xfer->rx_dma);
    }
#endif

    msg->status = -EINPROGRESS;
    msg->actual_length = 0;

    spin_lock_irqsave(&ms->lock, flags);
    list_add_tail(&msg->queue, &ms->queue);
    if (!ms->current_transfer)
        mspi_spi_next_message(spi->master);
    spin_unlock_irqrestore(&ms->lock, flags);

    return 0;
}

static void mspi_spi_cleanup(struct spi_device *spi)
{
    struct mspi_spi *ms = spi_master_get_devdata(spi->master);
    struct mspi_spi_device *msd = spi->controller_state;
    unsigned long   flags;

    if (!msd)
        return;

    spin_lock_irqsave(&ms->lock, flags);
    if ( ms->stay == spi)
    {
        ms->stay = NULL;
        cs_deactivate(ms, spi);
    }
    spin_unlock_irqrestore(&ms->lock, flags);

    spi->controller_state = NULL;
    kfree(msd);
}

/*-------------------------------------------------------------------------*/

static int __init mspi_spi_probe(struct platform_device *pdev)
{
    struct resource   *regs;
    int               irq;
    struct clk        *clk;
    int               ret;
    struct spi_master *master;
    struct mspi_spi   *ms;

#if 0
    regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!regs)
        return -ENXIO;

    irq = platform_get_irq(pdev, 0);
    if (irq < 0)
        return irq;

    clk = clk_get(&pdev->dev, "spi_clk");

    if (IS_ERR(clk))
        return PTR_ERR(clk);
#endif

    /* setup spi core then atmel-specific driver state */
    ret = -ENOMEM;
    master = spi_alloc_master(&pdev->dev, sizeof *ms);
    if (!master)
        goto out_free;

    /* the spi->mode bits understood by this driver: */
    master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH;

    master->bus_num = pdev->id;
    master->num_chipselect = 1;
    master->setup = mspi_spi_setup;
    master->transfer = mspi_spi_transfer;
    master->cleanup = mspi_spi_cleanup;
    platform_set_drvdata(pdev, master);

    ms = spi_master_get_devdata(master);

    /*
         * Scratch buffer is used for throwaway rx and tx data.
         * It's coherent to minimize dcache pollution.
         */
    if (!ms->buffer)
        goto out_free;

    spin_lock_init(&ms->lock);
    INIT_LIST_HEAD(&ms->queue);
    ms->pdev = pdev;
#if 0
    as->regs = ioremap(regs->start, resource_size(regs));
    if (!as->regs)
        goto out_free_buffer;
    as->irq = irq;
    as->clk = clk;

    ret = request_irq(irq, atmel_spi_interrupt, 0,
              dev_name(&pdev->dev), master);
    if (ret)
        goto out_unmap_regs;
#endif
    /* Initialize the hardware */
    HAL_MSPI_Init();

    /* go! */
    dev_info(&pdev->dev, "MSPI SPI Controller at 0x%08lx (irq %d)\n",
        (unsigned long)regs->start, irq);

    ret = spi_register_master(master);
    if (ret)
        goto out_reset_hw;

    return 0;

out_reset_hw:

out_free:
    spi_master_put(master);
    return ret;
}

static int __exit mspi_spi_remove(struct platform_device *pdev)
{
    struct spi_master     *master = platform_get_drvdata(pdev);
    struct mspi_spi *ms = spi_master_get_devdata(master);
    struct spi_message  *msg;

    /* reset the hardware and block queue progress */
    spin_lock_irq(&ms->lock);
    ms->stopping = 1;
    spin_unlock_irq(&ms->lock);

    /* Terminate remaining queued transfers */
    list_for_each_entry(msg, &ms->queue, queue)
    {
        /* REVISIT unmapping the dma is a NOP on ARM and AVR32
                * but we shouldn't depend on that...
                */
        msg->status = -ESHUTDOWN;
        msg->complete(msg->context);
    }
    spi_unregister_master(master);

    return 0;
}

static struct platform_driver mspi_spi_driver = {
    .driver     = {
        .name   = "mspi_spi",
        .owner  = THIS_MODULE,
    },
    .suspend    = NULL,
    .resume     = NULL,
    .remove     = __exit_p(mspi_spi_remove),
};

static int __init mspi_spi_init(void)
{
	return platform_driver_probe(&mspi_spi_driver, mspi_spi_probe);
}
module_init(mspi_spi_init);

static void __exit mspi_spi_exit(void)
{
    platform_driver_unregister(&mspi_spi_driver);
}
module_exit(mspi_spi_exit);

MODULE_DESCRIPTION("Mstar MSPI SPI Controller driver");
MODULE_AUTHOR("AAAA");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:mspi_spi");
