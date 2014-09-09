/*
 *
 * mstarsemi HBA 3.0Gbps SATA device driver
 *
 * Author: mstarsemi.com.tw
 *
 * Copyright (c) 2006-2007 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <scsi/scsi_host.h>
#include <scsi/scsi_cmnd.h>
#include <linux/libata.h>
#include <asm/io.h>
#include <mdrv_sata_host.h>

//#define SATA_DEBUG
#ifdef SATA_DEBUG
#define sata_debug(fmt, args...) printk("[%s] " fmt, __FUNCTION__, ##args)
#else
#define sata_debug(fmt, args...) do {} while(0)
#endif

static void mstar_sata_reg_write(u32 data, u32 reg_addr)
{
    iowrite16(data&0xFFFF, reg_addr);
    iowrite16((data >> 16)&0xFFFF,(reg_addr + 0x04));
}

static u32 mstar_sata_reg_read(u32 reg_addr)
{
    u32 data;
    data = (ioread16(reg_addr + 0x04)<<16) + ioread16(reg_addr);
    return data;
}

static void read_cmd_fis(void *rx_fis, u32 u32cmd_offset ,hal_cmd_h2dfis *pfis, u32 misc_base)
{
    void *cmd_address = rx_fis + u32cmd_offset;

#if (SATA_CMD_TYPE == TYPE_XIU)
    unsigned long u32MiscAddr = misc_base;

    writew(0x00,u32MiscAddr+SATA_MISC_ACCESS_MODE);
#endif

    memcpy(pfis, cmd_address, sizeof(hal_cmd_h2dfis));

#if (SATA_CMD_TYPE == TYPE_XIU)
    writew(0x01,u32MiscAddr+SATA_MISC_ACCESS_MODE);
#endif
}

static void build_cmd_fis(void *cmd_tbl ,hal_cmd_h2dfis *pfis, u32 misc_base)
{
#if (SATA_CMD_TYPE == TYPE_XIU)
    unsigned long u32MiscAddr = misc_base;

    writew(0x00,u32MiscAddr+SATA_MISC_ACCESS_MODE);
#endif

    memcpy(cmd_tbl, pfis,sizeof(hal_cmd_h2dfis));

#if (SATA_CMD_TYPE == TYPE_XIU)
    writew(0x01,u32MiscAddr+SATA_MISC_ACCESS_MODE);
#endif
}

static void build_cmd_prdt(void *base_address ,u32 *pprdt, u32 misc_base, u32 prdt_num)
{
    void *cmd_address = base_address + SATA_KA9_CMD_DESC_OFFSET_TO_PRDT;

#if (SATA_CMD_TYPE == TYPE_XIU)
    unsigned long u32MiscAddr = misc_base;


    writew(0x00,u32MiscAddr+SATA_MISC_ACCESS_MODE);
#endif

    memcpy(cmd_address,pprdt,sizeof(u32)*prdt_num*4);

#if (SATA_CMD_TYPE == TYPE_XIU)
    writew(0x01,u32MiscAddr+SATA_MISC_ACCESS_MODE);
#endif
}


static void build_cmd_header(void *cmd_slot, u32 u32offset_address,u32 *pcmdheader, u32 misc_base)
{
    void *cmd_address = cmd_slot + u32offset_address;

#if (SATA_CMD_TYPE == TYPE_XIU)
    unsigned long u32MiscAddr = misc_base;

    writew(0x00,u32MiscAddr+SATA_MISC_ACCESS_MODE);
#endif

    memcpy(cmd_address, pcmdheader, SATA_KA9_CMD_HDR_SIZE);

#if (SATA_CMD_TYPE == TYPE_XIU)
    writew(0x01,u32MiscAddr+SATA_MISC_ACCESS_MODE);
#endif
}

static inline unsigned int sata_mstar_tag(unsigned int tag)
{
    /* all non NCQ/queued commands should have tag#0 */
    if (ata_tag_internal(tag)) {
        return 0;
    }

    if (unlikely(tag >= SATA_KA9_QUEUE_DEPTH)) {
        DPRINTK("tag %d invalid : out of range\n", tag);
        printk("[%s][%d]\n",__FUNCTION__,__LINE__);
        return 0;
    }
    return tag;
}

static void sata_mstar_setup_cmd_hdr_entry(struct sata_mstar_port_priv *pp,
                     unsigned int tag, u32 data_xfer_len, u8 num_prde,
                     u8 fis_len, u32 misc_base)
{
    dma_addr_t cmd_descriptor_address;
    hal_cmd_header cmd_header = {0};
    void *cmd_slot = pp->cmd_slot;

    cmd_descriptor_address = pp->cmd_tbl_dma + tag * SATA_KA9_CMD_DESC_SIZE;

    cmd_header.cmd_fis_len = fis_len;
    cmd_header.PRDTlength = num_prde;
    cmd_header.isclearok = 0;
    cmd_header.PRDBytes = data_xfer_len;
    cmd_header.ctba_hbase = 0;
    cmd_header.ctba_lbase = cmd_descriptor_address;

    build_cmd_header(cmd_slot, tag * SATA_KA9_CMD_HDR_SIZE,(u32 *)&cmd_header, misc_base);
}

static unsigned int sata_mstar_fill_sg(struct ata_queued_cmd *qc,
                     u32 *ttl, void *cmd_tbl, u32 misc_base)
{
    struct scatterlist *sg;
    u32 ttl_dwords = 0;
    u32 prdt[SATA_KA9_USED_PRD * 4] = {0};
    unsigned int si;

    for_each_sg(qc->sg, sg, qc->n_elem, si) {
        dma_addr_t sg_addr = sg_dma_address(sg);
        u32 sg_len = sg_dma_len(sg);

        if (si == (SATA_KA9_USED_PRD - 1) && ((sg_next(sg)) != NULL))
        {
            printk("setting indirect prde , prdt not enough \n");
        }

        ttl_dwords += sg_len;

        prdt[si*4 + 0] =  cpu_to_le32(sg_addr) - 0x20000000;
        prdt[si*4 + 1] = 0;
        prdt[si*4 + 2] = 0xFFFFFFFF;
        prdt[si*4 + 3] = (cpu_to_le32(sg_len) - 1);
    }

    build_cmd_prdt(cmd_tbl,&prdt[0], misc_base, si);
    *ttl = ttl_dwords;
    return si;
}

static u32 mstar_sata_wait_reg(u32 reg_addr, u32 mask, u32 val, unsigned long interval, unsigned long timeout)
{
    u32 temp;
    unsigned long timeout_vale = 0;

    temp = mstar_sata_reg_read(reg_addr);

    while((temp & mask) == val)
    {
        msleep(interval);
        timeout_vale += interval;
        if (timeout_vale > timeout)
            break;
        temp = mstar_sata_reg_read(reg_addr);
    }

    return temp;
}

static int mstar_ahci_stop_engine(u32 port_base)
{
    u32 temp;

    temp = mstar_sata_reg_read(PORT_CMD + port_base);

    /* check if the HBA is idle */
    if ((temp & (PORT_CMD_START | PORT_CMD_LIST_ON)) == 0)
        return 0;

    /* setting HBA to idle */
    temp &= ~PORT_CMD_START;
    mstar_sata_reg_write(temp, PORT_CMD + port_base);

    temp = mstar_sata_wait_reg(PORT_CMD + port_base, PORT_CMD_LIST_ON, PORT_CMD_LIST_ON, 1, 500);

    if (temp & PORT_CMD_LIST_ON)
        return -EIO;

    return 0;
}

static void mstar_ahci_start_engine(u32 port_base)
{
    u32 temp;

    /* Start Port DMA */
    temp = mstar_sata_reg_read(PORT_CMD + port_base);
    temp |= PORT_CMD_START;
    mstar_sata_reg_write(temp, PORT_CMD + port_base);
    mstar_sata_reg_read(PORT_CMD + port_base); /* Flush */
}

static void mstar_ahci_start_fis_rx(struct ata_port *ap)
{
    struct sata_mstar_port_priv *pp = ap->private_data;
    struct sata_mstar_host_priv *host_priv = ap->host->private_data;
    u32 port_base = host_priv->port_base;
    u32 tmp;

    // set FIS registers
    tmp = pp->cmd_slot_dma;
    mstar_sata_reg_write(tmp, PORT_LST_ADDR + port_base);

    tmp = pp->rx_fis_dma;
    mstar_sata_reg_write(tmp , PORT_FIS_ADDR + port_base);

    // enable FIS reception
    tmp = mstar_sata_reg_read(PORT_CMD + port_base);
    tmp |= PORT_CMD_FIS_RX;
    mstar_sata_reg_write(tmp, PORT_CMD + port_base);

    // flush
    tmp= mstar_sata_reg_read(PORT_CMD + port_base);
}

static int mstar_ahci_stop_fis_rx(struct ata_port *ap)
{
    struct sata_mstar_host_priv *host_priv = ap->host->private_data;
    u32 port_base = host_priv->port_base;
    u32 tmp;

    // Disable FIS reception
    tmp = mstar_sata_reg_read(PORT_CMD + port_base);
    tmp &= ~PORT_CMD_FIS_RX;
    mstar_sata_reg_write(tmp, PORT_CMD + port_base);

    // Wait FIS reception Stop for 1000ms
    tmp = mstar_sata_wait_reg(PORT_CMD + port_base, PORT_CMD_FIS_ON, PORT_CMD_FIS_ON, 10, 1000);

    if (tmp & PORT_CMD_FIS_ON)
        return -EBUSY;

    return 0;
}

static void sata_mstar_qc_prep(struct ata_queued_cmd *qc)
{
    struct ata_port *ap = qc->ap;
    struct sata_mstar_port_priv *pp = ap->private_data;
    struct sata_mstar_host_priv *host_priv = ap->host->private_data;
    u32 misc_base = host_priv->misc_base;
    unsigned int tag = sata_mstar_tag(qc->tag);
    hal_cmd_h2dfis h2dfis;
    u32 num_prde = 0;
    u32 ttl_dwords = 0;
    void *cmd_tbl;

    cmd_tbl = pp->cmd_tbl + (tag * SATA_KA9_CMD_DESC_SIZE);

    ata_tf_to_fis(&qc->tf, qc->dev->link->pmp, 1, (u8 *)&h2dfis);

    build_cmd_fis(cmd_tbl ,&h2dfis, misc_base);

    if (qc->flags & ATA_QCFLAG_DMAMAP)
    {
        num_prde = sata_mstar_fill_sg(qc, &ttl_dwords, cmd_tbl, misc_base);
    }

    sata_mstar_setup_cmd_hdr_entry(pp, tag, ttl_dwords,
                     num_prde, 5, misc_base);

}

static unsigned int sata_mstar_qc_issue(struct ata_queued_cmd *qc)
{
    struct sata_mstar_host_priv *host_priv = qc->ap->host->private_data;
    u32 port_base = host_priv->port_base;
    unsigned int tag = sata_mstar_tag(qc->tag);

    if (qc->tf.protocol == ATA_PROT_NCQ)
    {
        mstar_sata_reg_write(1 << qc->tag, PORT_SCR_ACT + port_base);
    }

    mstar_sata_reg_write(1 << tag, PORT_CMD_ISSUE + port_base);

    return 0;
}

static bool sata_mstar_qc_fill_rtf(struct ata_queued_cmd *qc)
{
    struct sata_mstar_port_priv *pp = qc->ap->private_data;
    struct sata_mstar_host_priv *host_priv = qc->ap->host->private_data;
    unsigned int tag = sata_mstar_tag(qc->tag);
    hal_cmd_h2dfis cd;
    u32 misc_base = host_priv->misc_base;
    void *rx_fis;

    rx_fis = pp->rx_fis;

    read_cmd_fis(rx_fis,(tag * SATA_KA9_CMD_DESC_SIZE),&cd, misc_base);

    ata_tf_from_fis((const u8 *)&cd, &qc->result_tf);
    return true;
}

static int sata_mstar_scr_offset(struct ata_port *ap, unsigned int sc_reg)
{
    static const int offset[] = {
        [SCR_STATUS]        = PORT_SCR_STAT,
        [SCR_CONTROL]       = PORT_SCR_CTL,
        [SCR_ERROR]     = PORT_SCR_ERR,
        [SCR_ACTIVE]        = PORT_SCR_ACT,
        [SCR_NOTIFICATION]  = PORT_SCR_NTF,
    };

    if (sc_reg < ARRAY_SIZE(offset) && (sc_reg != SCR_NOTIFICATION))
        return offset[sc_reg];

    return 0;
}

static int sata_mstar_scr_read(struct ata_link *link,
                 unsigned int sc_reg, u32 *val)
{
    int offset = sata_mstar_scr_offset(link->ap, sc_reg);
    struct sata_mstar_host_priv *host_priv = link->ap->host->private_data;
    u32 port_base = host_priv->port_base;

    if (offset) {
        *val = mstar_sata_reg_read(offset + port_base);
        return 0;
    }

    return -1;
}

static int sata_mstar_scr_write(struct ata_link *link,
                  unsigned int sc_reg_in, u32 val)
{
    int offset = sata_mstar_scr_offset(link->ap, sc_reg_in);
    struct sata_mstar_host_priv *host_priv = link->ap->host->private_data;
    u32 port_base = host_priv->port_base;

    if (offset)
    {
        mstar_sata_reg_write(val, offset + port_base);
        return 0;
    }

    return -1;
}

static void sata_mstar_freeze(struct ata_port *ap)
{
    struct sata_mstar_host_priv *host_priv = ap->host->private_data;
    u32 port_base = host_priv->port_base;

    mstar_sata_reg_write(0, PORT_IRQ_MASK + port_base);
}

static void sata_mstar_thaw(struct ata_port *ap)
{
    struct sata_mstar_host_priv *host_priv = ap->host->private_data;
    u32 hba_base = host_priv->hba_base;
    u32 port_base = host_priv->port_base;
    u32 u32Temp = 0;

    // clear IRQ
    u32Temp = mstar_sata_reg_read(PORT_IRQ_STAT + port_base);
    mstar_sata_reg_write(u32Temp, PORT_IRQ_STAT + port_base);

    // Clear Port 0 IRQ on HBA
    u32Temp = mstar_sata_reg_read(HOST_IRQ_STAT + hba_base);
    mstar_sata_reg_write(u32Temp, HOST_IRQ_STAT + hba_base);

    // Enable Host Interrupt
    u32Temp = mstar_sata_reg_read(HOST_CTL + hba_base);
    u32Temp |= HOST_IRQ_EN;
    mstar_sata_reg_write(u32Temp, HOST_CTL + hba_base);

    // Enable Port Interrupt
    mstar_sata_reg_write(DEF_PORT_IRQ, PORT_IRQ_MASK + port_base);
    mstar_sata_reg_read(PORT_IRQ_MASK + port_base);

}

static unsigned int sata_mstar_dev_classify(struct ata_port *ap)
{
    struct ata_taskfile tf;
    u32 temp = 0;
    struct sata_mstar_host_priv *host_priv = ap->host->private_data;

    temp = mstar_sata_reg_read(PORT_SIG + host_priv->port_base);

    tf.lbah = (temp >> 24) & 0xff;
    tf.lbam = (temp >> 16) & 0xff;
    tf.lbal = (temp >> 8) & 0xff;
    tf.nsect = temp & 0xff;

    return ata_dev_classify(&tf);
}

int mstar_ahci_check_ready(struct ata_link *link)
{
    struct ata_port *ap = link->ap;
    struct sata_mstar_host_priv *host_priv = ap->host->private_data;
    u32 port_base = host_priv->port_base;
    u8 status = mstar_sata_reg_read(PORT_TFDATA + port_base) & 0xFF;

    return ata_check_ready(status);
}

static int sata_mstar_hardreset(struct ata_link *link, unsigned int *class,
                    unsigned long deadline)
{
    struct ata_port *ap = link->ap;
    struct sata_mstar_host_priv *host_priv = ap->host->private_data;
    const unsigned long *timing = sata_ehc_deb_timing(&link->eh_context);
    u32 port_base = host_priv->port_base;
    bool online;
    int rc;

    mstar_ahci_stop_engine(port_base);

    rc = sata_link_hardreset(link, timing, deadline, &online,
                 mstar_ahci_check_ready);

    mstar_ahci_start_engine(port_base);

    if (online)
        *class = sata_mstar_dev_classify(ap);

    return rc;
}

static int sata_mstar_softreset(struct ata_link *link, unsigned int *class,
                    unsigned long deadline)
{
    // Unused Function
    return 0;
}

static void sata_mstar_error_handler(struct ata_port *ap)
{
    sata_pmp_error_handler(ap);
}

static void sata_mstar_post_internal_cmd(struct ata_queued_cmd *qc)
{
    if (qc->flags & ATA_QCFLAG_FAILED)
        qc->err_mask |= AC_ERR_OTHER;

    if (qc->err_mask) {
        /* make DMA engine forget about the failed command */
    }
}

static int sata_mstar_port_start(struct ata_port *ap)
{
    struct sata_mstar_port_priv *pp;
    struct sata_mstar_host_priv *host_priv = ap->host->private_data;
    u32 temp;
    u32 port_base = host_priv->port_base;
#if (SATA_CMD_TYPE == TYPE_DRAM)
    struct device *dev = ap->host->dev;
    void *mem;
    dma_addr_t mem_dma;
    size_t dma_sz;
#endif

    // Allocate SATA Port Private Data
    pp = kzalloc(sizeof(*pp), GFP_KERNEL);
    if (!pp)
    {
        printk("[%s][Error] SATA Allocate Port Private Data Fail\n", __func__);
        return -ENOMEM;
    }

#if (SATA_CMD_TYPE == TYPE_XIU)
    pp->cmd_slot = (void *)(SATA_SDMAP_RIU_BASE + (AHCI_P0CLB & 0xfff));
    pp->rx_fis = (void *)(SATA_SDMAP_RIU_BASE + (AHCI_P0FB & 0xfff));
    pp->cmd_tbl = (void *)(SATA_SDMAP_RIU_BASE + (AHCI_CTBA0 & 0xfff));

    pp->cmd_slot_dma = AHCI_P0CLB;
    pp->rx_fis_dma = AHCI_P0FB;
    pp->cmd_tbl_dma = AHCI_CTBA0;

#elif (SATA_CMD_TYPE == TYPE_DRAM)
    dma_sz = 0x10000;
    mem = dmam_alloc_coherent(dev, dma_sz, &mem_dma, GFP_KERNEL);
    if (!mem)
        return -ENOMEM;
    memset(mem, 0, dma_sz);

    pp->cmd_slot = mem;
    pp->cmd_slot_dma = mem_dma;

    mem += SATA_CMD_HEADER_SIZE;
    mem_dma += SATA_CMD_HEADER_SIZE;

    pp->rx_fis = mem;
    pp->rx_fis_dma = mem_dma;

    mem += SATA_FIS_SIZE;
    mem_dma += SATA_FIS_SIZE;

    pp->cmd_tbl = mem;
    pp->cmd_tbl_dma = mem_dma;
#else
#error "SATA_CMD_TYPE Unknown"
#endif

    sata_debug("cmd_slot = 0x%x ; cmd_slot_dma = 0x%x\n", (u32)pp->cmd_slot, (u32)pp->cmd_slot_dma);
    sata_debug("rx_fis = 0x%x ; rx_fis_dma = 0x%x\n", (u32)pp->rx_fis, (u32)pp->rx_fis_dma);
    sata_debug("cmd_tbl = 0x%x ; cmd_tbl_dma = 0x%x\n", (u32)pp->cmd_tbl, (u32)pp->cmd_tbl_dma);

    ap->private_data = pp;

    temp = mstar_sata_reg_read(PORT_CMD + port_base) & ~PORT_CMD_ICC_MASK;

    // spin up device
    temp |= PORT_CMD_SPIN_UP;
    mstar_sata_reg_write(temp, PORT_CMD + port_base);

    // wake up link
    mstar_sata_reg_write((temp | PORT_CMD_ICC_ACTIVE), PORT_CMD + port_base);

    // start FIS RX
    mstar_ahci_start_fis_rx(ap);

    // Clear IS , Interrupt Status
    mstar_sata_reg_write(0xFFFFFFFF, PORT_IRQ_STAT + port_base);
    mstar_sata_reg_write(0xFFFFFFFF, PORT_SCR_ERR + port_base);

    // set to speed limit with gen 1, gen 2 or auto
    temp = mstar_sata_reg_read(PORT_SCR_CTL + port_base);
    temp = temp & (~E_PORT_SPEED_MASK); //Auto
    //temp = temp | E_PORT_SPEED_GEN1;  //Gen1 Limit
    //temp = temp | E_PORT_SPEED_GEN2;  // Gen2 Limit
    mstar_sata_reg_write(temp, PORT_SCR_CTL + port_base);

    // Start DMA Engine
    mstar_ahci_start_engine(port_base);

    return 0;
}

static void sata_mstar_port_stop(struct ata_port *ap)
{
    struct sata_mstar_host_priv *host_priv = ap->host->private_data;
    u32 port_base = host_priv->port_base;
    int ret;

    // Stop DMA Engine
    ret = mstar_ahci_stop_engine(port_base);
    if (ret)
    {
        printk("[%s][Error] Fail to Stop SATA Port\n", __func__);
    }

    // Disable FIS reception
    ret = mstar_ahci_stop_fis_rx(ap);
    if (ret) {
        printk("[%s][Error] Fail to Stop FIS RX\n", __func__);
    }
}

irqreturn_t sata_mstar_interrupt(int irq, void *dev_instance)
{
    struct ata_host *host = dev_instance;
    struct ata_port *ap = host->ports[0];
    struct sata_mstar_host_priv *host_priv = host->private_data;
    u32 port_base = host_priv->port_base;
    u32 hba_base = host_priv->hba_base;
    u32 host_status,port_status;
    u32 qc_active;

    host_status = mstar_sata_reg_read(HOST_IRQ_STAT + hba_base);

    spin_lock(&host->lock);

    port_status = mstar_sata_reg_read(PORT_IRQ_STAT + port_base);
    mstar_sata_reg_write(port_status, PORT_IRQ_STAT + port_base);

    qc_active = mstar_sata_reg_read(PORT_SCR_ACT + port_base);
    qc_active |= mstar_sata_reg_read(PORT_CMD_ISSUE + port_base);

    ata_qc_complete_multiple(ap, qc_active);

    mstar_sata_reg_write(host_status, HOST_IRQ_STAT + hba_base);

    spin_unlock(&host->lock);

    return IRQ_RETVAL(1);
}

static int mstar_sata_hardware_init(struct sata_mstar_host_priv *hpriv)
{
    u32 i;
    u32 u32Temp = 0;
    u32 misc_base = hpriv->misc_base;
    u32 port_base = hpriv->port_base;
    u32 hba_base = hpriv->hba_base;
    u16 u16Temp;
    u8 u8Temp;

    // Open SATA PHY Clock
    writew(0x0030, 0xfd220000 + (0x28a2 << 1));

    // Set MIU1 Access for SATA
    u16Temp = readw(0xfd200000 + (0x06f6 << 1));
    u16Temp |= 0x8000;
    writew(u16Temp, 0xfd200000 + (0x06f6 << 1));

    writew(0x0001, misc_base + SATA_MISC_HOST_SWRST);
    writew(0x0000, misc_base + SATA_MISC_HOST_SWRST);
    writew(0x0000, misc_base + SATA_MISC_AMBA_MUXRST);
    writew(0x008C, misc_base + SATA_MISC_AMBA_ARBIT);

    // AHB Data FIFO  Setting
    writew(0x0000, misc_base + SATA_MISC_HBA_HADDR);
    writew(0x0000, misc_base + SATA_MISC_HBA_LADDR);
    writew(0x1800, misc_base + SATA_MISC_CMD_HADDR);
    writew(0x1000, misc_base + SATA_MISC_CMD_LADDR);
    writew(0x0000, misc_base + SATA_MISC_DATA_ADDR);
    writew(0x0001, misc_base + SATA_MISC_ENRELOAD);
    writew(0x0001, misc_base + SATA_MISC_ACCESS_MODE);

    // SATA Phy Setting
    writew(0x0000, 0xfd240000 + (0x2322 << 1));
    writew(0x2514, 0xfd240000 + (0x2324 << 1));
    writew(0x0302, 0xfd240000 + (0x230c << 1));
    writew(0x2200, 0xfd240000 + (0x230e << 1));
    writew(0x0300, 0xfd240000 + (0x2310 << 1));
    writew(0x2190, 0xfd240000 + (0x2368 << 1));
    writew(0x0000, 0xfd240000 + (0x237e << 1));
    writew(0x0a3d, 0xfd240000 + (0x2360 << 1));
    writew(0x0017, 0xfd240000 + (0x2362 << 1));
    writew(0x8087, 0xfd240000 + (0x2302 << 1));
    writew(0x8086, 0xfd240000 + (0x2302 << 1));

    // rterm setup
    u16Temp = readw(0xfd240000 + (0x2330 << 1));
    u16Temp &= ~0xffc0;
    u16Temp |= 0x83c0;
    writew(u16Temp, 0xfd240000 + (0x2330 << 1));
    writew(0x6c29, 0xfd240000 + (0x232c << 1));

    // cominit comwake time setup
    u8Temp = readb(0xfd240000 + ((0x2303 << 1)-1));
    u8Temp &= ~0x02;
    u8Temp |= 0x02;
    writeb(u8Temp, 0xfd240000 + ((0x2303 << 1)-1));
    u8Temp = readb(0xfd240000 + ((0x2303 << 1)-1));
    u8Temp &= ~0x10;
    u8Temp |= 0x10;
    writeb(u8Temp, 0xfd240000 + ((0x2303 << 1)-1));
    writeb(0x2a, 0xfd240000 + (0x2306 << 1));
    writeb(0x0b, 0xfd240000 + ((0x2307 << 1)-1));
    writeb(0x1c, 0xfd240000 + (0x2308 << 1));
    writeb(0x30, 0xfd240000 + ((0x2309 << 1)-1));
    writeb(0x5a, 0xfd240000 + (0x230a << 1));

    // enable SSC
    u8Temp = readb(0xfd240000 + ((0x2365 << 1)-1));
    u8Temp &= ~0x20;
    u8Temp |= 0x20;
    writeb(u8Temp, 0xfd240000 + ((0x2365 << 1)-1));
    writeb(0x17, 0xfd240000 + (0x2362 << 1));
    writew(0x0a3d, 0xfd240000 + (0x2360 << 1));
    u8Temp = readb(0xfd240000 + ((0x2367 << 1)-1));
    u8Temp &= ~0x7f;
    u8Temp |= 0x04;
    writeb(u8Temp, 0xfd240000 + ((0x2367 << 1)-1));
    writeb(0x70, 0xfd240000 + (0x2366 << 1));
    u8Temp = readb(0xfd240000 + ((0x2365 << 1)-1));
    u8Temp &= ~0x07;
    u8Temp |= 0x00;
    writeb(u8Temp, 0xfd240000 + ((0x2365 << 1)-1));
    writeb(0x02, 0xfd240000 + (0x2364 << 1));

    writew(HOST_RESET, hba_base + (HOST_CTL));

    u32Temp = mstar_sata_wait_reg(HOST_CTL + hba_base, HOST_RESET, HOST_RESET, 1, 500);

    if (u32Temp & HOST_RESET)
        return -1;

    //  Turn on AHCI_EN
    u32Temp = mstar_sata_reg_read(HOST_CTL + hba_base);
    if (u32Temp & HOST_AHCI_EN)
        goto FW_INITIAL;

    // Try AHCI_EN Trurn on for a few time
    for (i = 0; i < 5; i++) {
        u32Temp |= HOST_AHCI_EN;
        mstar_sata_reg_write(u32Temp, HOST_CTL + hba_base);
        u32Temp = mstar_sata_reg_read(HOST_CTL + hba_base);
        if (u32Temp & HOST_AHCI_EN)
            break;
        msleep(10);
    }

FW_INITIAL:

    //  Init FW to trigger controller
    writew(0x0000, hba_base + (HOST_CAP));
    writew(0x0000, hba_base + (HOST_CAP + 0x4));

    // Port Implement
    writew(0x0001, hba_base + (HOST_PORTS_IMPL));
    writew(0x0000, port_base + (PORT_CMD));
    writew(0x0000, port_base + (PORT_CMD + 0x4));

    return 0;
}

static struct scsi_host_template mstar_sata_sht = {
    ATA_NCQ_SHT("mstar_sata"),
    .can_queue = SATA_KA9_QUEUE_DEPTH,
    .sg_tablesize = SATA_KA9_USED_PRD,
    .dma_boundary = ATA_DMA_BOUNDARY,
};

static struct ata_port_operations mstar_sata_ops = {
    .inherits       = &sata_pmp_port_ops,

    .qc_defer = ata_std_qc_defer,
    .qc_prep = sata_mstar_qc_prep,
    .qc_issue = sata_mstar_qc_issue,
    .qc_fill_rtf = sata_mstar_qc_fill_rtf,

    .scr_read = sata_mstar_scr_read,
    .scr_write = sata_mstar_scr_write,

    .freeze = sata_mstar_freeze,
    .thaw = sata_mstar_thaw,

    .softreset = sata_mstar_softreset,
    .hardreset = sata_mstar_hardreset,

    .pmp_softreset = sata_mstar_softreset,
    .error_handler = sata_mstar_error_handler,
    .post_internal_cmd = sata_mstar_post_internal_cmd,

    .port_start = sata_mstar_port_start,
    .port_stop = sata_mstar_port_stop,

};

static const struct ata_port_info mstar_sata_port_info[] = {
    {
     .flags = SATA_KA9_HOST_FLAGS,
     .pio_mask = ATA_PIO6,
     .udma_mask = ATA_UDMA6,
     .port_ops = &mstar_sata_ops,
     },
};

static int mstar_sata_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct ata_port_info pi = mstar_sata_port_info[0];
    const struct ata_port_info *ppi[] = { &pi, NULL };
    struct sata_mstar_host_priv *hpriv;
    struct ata_host *host;
    struct resource *port_mem;
    struct resource *misc_mem;
    struct resource *hba_mem;
    int irq = 0;
    int ret = 0;

    printk("MStar SATA Host Controller Probe\n");

    hba_mem  = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    port_mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    misc_mem = platform_get_resource(pdev, IORESOURCE_MEM, 2);

    if (!hba_mem) {
        printk("[%s][Error] SATA Get HBA Resource Fail\n", __func__);
        return -EINVAL;
    }
    if (!port_mem) {
        printk("[%s][Error] SATA Get Port Resource Fail\n", __func__);
        return -EINVAL;
    }
    if (!misc_mem) {
        printk("[%s][Error] SATA Get MISC Resource Fail\n", __func__);
        return -EINVAL;
    }

    irq = platform_get_irq(pdev, 0);
    if (irq <= 0) {
        printk("[%s][Error] SATA Get IRQ Fail\n", __func__);
        return -EINVAL;
    }

    sata_debug("irq_number = %d\n", irq);

    // Allocate Host Private Data
    hpriv = devm_kzalloc(dev, sizeof(*hpriv), GFP_KERNEL);
    if (!hpriv)
    {
        printk("[%s][Error] SATA Allocate Host Private Data Fail\n", __func__);
        return -ENOMEM;
    }

    hpriv->hba_base = (u32)hba_mem->start;
    hpriv->port_base = (u32)port_mem->start;
    hpriv->misc_base = (u32)misc_mem->start;

    sata_debug("hba_base = 0x%x ; port_base = 0x%x ; misc_base = 0x%x\n",
        hpriv->hba_base, hpriv->port_base, hpriv->misc_base);

    // Initial SATA Hardware
    if (mstar_sata_hardware_init(hpriv))
    {
        printk("[%s][Error] SATA Hardware Initial Failed\n", __func__);
        return -EINVAL;
    }

    host = ata_host_alloc_pinfo(dev, ppi, SATA_PORT_NUM);
    if (!host) {
        ret = -ENOMEM;
        printk("[%s][Error] SATA Allocate ATA Host Fail\n", __func__);
        goto out_devm_kzalloc_hpriv;
    }

    host->private_data = hpriv;

    return ata_host_activate(host, irq, sata_mstar_interrupt, IRQF_SHARED, &mstar_sata_sht);

out_devm_kzalloc_hpriv:
    devm_kfree(dev, hpriv);

    return ret;
}

static int mstar_sata_remove(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct ata_host *host = dev_get_drvdata(dev);
    struct sata_mstar_host_priv *host_priv = host->private_data;

    ata_host_detach(host);

    devm_kfree(dev, host_priv);

    return 0;
}

static struct platform_driver mstar_sata_driver = {
    .probe          = mstar_sata_probe,
    .remove         = mstar_sata_remove,
    .driver         = {
        .name       = "Mstar-sata",
    }
};

static int __init mstar_sata_drv_init(void)
{
    int ret = 0;

    ret= platform_driver_register(&mstar_sata_driver);
    return ret;
}

static void __exit mstar_sata_drv_exit(void)
{
    platform_driver_unregister(&mstar_sata_driver);
}

MODULE_AUTHOR("Mstar Semiconductor");
MODULE_DESCRIPTION("Mstar 3.0Gbps SATA controller low level driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.00");

module_init(mstar_sata_drv_init);
module_exit(mstar_sata_drv_exit);
