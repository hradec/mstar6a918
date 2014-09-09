////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   devEMAC.c
/// @brief  EMAC Driver
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/init.h>
#include <linux/autoconf.h>
#include <linux/mii.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/pci.h>
#include <linux/crc32.h>
#include <linux/ethtool.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

#if defined(CONFIG_MIPS)
#include <asm/mips-boards/prom.h>
#include "mhal_chiptop_reg.h"
#elif defined(CONFIG_ARM)
#include <asm/arm-boards/prom.h>
#include <asm/mach/map.h>
#endif
#include "mdrv_types.h"
#include "mst_platform.h"
#include "mdrv_system.h"
#include "chip_int.h"
#include "mhal_emac.h"
#include "mdrv_emac.h"
#ifdef CHIP_FLUSH_READ
#include "chip_setup.h"
#endif
#ifdef CONFIG_EMAC_SUPPLY_RNG
#include <linux/input.h>
#include <random.h>
#include "mhal_rng_reg.h"
#endif
//--------------------------------------------------------------------------------------------------
//  Forward declaration
//--------------------------------------------------------------------------------------------------
#define EMAC_RX_TMR         (0)
#define EMAC_LINK_TMR       (1)

#define EMAC_CHECK_LINK_TIME    	(HZ)
#define IER_FOR_INT_JULIAN_D   		(0x0000E4B5)
#define EMAC_CHECK_CNT              (500000)
#define ALBANY_OUI_MSB              (0)
#define RTL_8210                    (0x1C)
#define PACKET_THRESHOLD 260
#define TXCOUNT_THRESHOLD 10

//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------
struct sk_buff *pseudo_packet;

//-------------------------------------------------------------------------------------------------
//  EMAC Function
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_tx (struct sk_buff *skb, struct net_device *dev);
static void MDev_EMAC_timer_callback( unsigned long value );
static int MDev_EMAC_SwReset(struct net_device *dev);
static void MDev_EMAC_Send_PausePkt(struct net_device* dev);
static unsigned long getCurMs(void)
{
	struct timeval tv;
	unsigned long curMs;

	do_gettimeofday(&tv);
    curMs = tv.tv_usec/1000;
	curMs += tv.tv_sec * 1000;
    return curMs;
}

//-------------------------------------------------------------------------------------------------
// PHY MANAGEMENT
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Access the PHY to determine the current Link speed and Mode, and update the
// MAC accordingly.
// If no link or auto-negotiation is busy, then no changes are made.
// Returns:  0 : OK
//              -1 : No link
//              -2 : AutoNegotiation still in progress
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_update_linkspeed (struct net_device *ndev)
{
    u32 bmsr, bmcr, LocPtrA;
    u32 speed, duplex;
	struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);

    // Link status is latched, so read twice to get current value //
    MHal_EMAC_read_phy (ndev, LocPtr->phy.addr, MII_BMSR, &bmsr);
    MHal_EMAC_read_phy (ndev, LocPtr->phy.addr, MII_BMSR, &bmsr);
    if (!(bmsr & BMSR_LSTATUS)){
        return -1;          //no link //
    }
    MHal_EMAC_read_phy (ndev, LocPtr->phy.addr, MII_BMCR, &bmcr);

    if (bmcr & BMCR_ANENABLE)
    {               //AutoNegotiation is enabled //
        if (!(bmsr & BMSR_ANEGCOMPLETE))
        {
            EMAC_DBG(LocPtr->pdev->id ,"==> AutoNegotiation still in progress\n");
            return -2;
        }

        MHal_EMAC_read_phy (ndev, LocPtr->phy.addr, MII_LPA, &LocPtrA);
        if ((LocPtrA & LPA_100FULL) || (LocPtrA & LPA_100HALF))
        {
            speed = SPEED_100;
        }
        else
        {
            speed = SPEED_10;
        }
        if ((LocPtrA & LPA_100FULL) || (LocPtrA & LPA_10FULL))
            duplex = DUPLEX_FULL;
        else
            duplex = DUPLEX_HALF;
    }
    else
    {
        speed = (bmcr & BMCR_SPEED100) ? SPEED_100 : SPEED_10;
        duplex = (bmcr & BMCR_FULLDPLX) ? DUPLEX_FULL : DUPLEX_HALF;
    }

    // Update the MAC //
    MHal_EMAC_update_speed_duplex(ndev, speed, duplex);
    return 0;
}

static int MDev_EMAC_get_info(struct net_device *ndev)
{
    u32 bmsr, bmcr, LocPtrA;
    u32 uRegStatus =0;
	struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);

    // Link status is latched, so read twice to get current value //
    MHal_EMAC_read_phy (ndev, LocPtr->phy.addr, MII_BMSR, &bmsr);
    MHal_EMAC_read_phy (ndev, LocPtr->phy.addr, MII_BMSR, &bmsr);
    if (!(bmsr & BMSR_LSTATUS)){
        uRegStatus &= ~ETHERNET_TEST_RESET_STATE;
        uRegStatus |= ETHERNET_TEST_NO_LINK; //no link //
    }
    MHal_EMAC_read_phy (ndev, LocPtr->phy.addr, MII_BMCR, &bmcr);

    if (bmcr & BMCR_ANENABLE)
    {
        //AutoNegotiation is enabled //
        if (!(bmsr & BMSR_ANEGCOMPLETE))
        {
            uRegStatus &= ~ETHERNET_TEST_RESET_STATE;
            uRegStatus |= ETHERNET_TEST_AUTO_NEGOTIATION; //AutoNegotiation //
        }
        else
        {
            uRegStatus &= ~ETHERNET_TEST_RESET_STATE;
            uRegStatus |= ETHERNET_TEST_LINK_SUCCESS; //link success //
        }

        MHal_EMAC_read_phy (ndev, LocPtr->phy.addr, MII_LPA, &LocPtrA);
        if ((LocPtrA & LPA_100FULL) || (LocPtrA & LPA_100HALF))
        {
            uRegStatus |= ETHERNET_TEST_SPEED_100M; //SPEED_100//
        }
        else
        {
            uRegStatus &= ~ETHERNET_TEST_SPEED_100M; //SPEED_10//
        }

        if ((LocPtrA & LPA_100FULL) || (LocPtrA & LPA_10FULL))
        {
            uRegStatus |= ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_FULL//
        }
        else
        {
            uRegStatus &= ~ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_HALF//
        }
    }
    else
    {
        if(bmcr & BMCR_SPEED100)
        {
            uRegStatus |= ETHERNET_TEST_SPEED_100M; //SPEED_100//
        }
        else
        {
            uRegStatus &= ~ETHERNET_TEST_SPEED_100M; //SPEED_10//
        }

        if(bmcr & BMCR_FULLDPLX)
        {
            uRegStatus |= ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_FULL//
        }
        else
        {
            uRegStatus &= ~ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_HALF//
        }
    }

    return uRegStatus;
}

//-------------------------------------------------------------------------------------------------
//Program the hardware MAC address from dev->dev_addr.
//-------------------------------------------------------------------------------------------------
void MDev_EMAC_update_mac_address (struct net_device *ndev)
{
    u32 value;
	
    value = (ndev->dev_addr[3] << 24) | (ndev->dev_addr[2] << 16) | (ndev->dev_addr[1] << 8) |(ndev->dev_addr[0]);
    MHal_EMAC_Write_SA1L(ndev,value);
    value = (ndev->dev_addr[5] << 8) | (ndev->dev_addr[4]);
    MHal_EMAC_Write_SA1H(ndev, value);
}

//-------------------------------------------------------------------------------------------------
// ADDRESS MANAGEMENT
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Set the ethernet MAC address in dev->dev_addr
//-------------------------------------------------------------------------------------------------
static void MDev_EMAC_get_mac_address (struct net_device *ndev)
{
    char addr[6];
    u32 HiAddr, LoAddr;

    // Check if bootloader set address in Specific-Address 1 //
    HiAddr = MHal_EMAC_get_SA1H_addr(ndev);
    LoAddr = MHal_EMAC_get_SA1L_addr(ndev);

    addr[0] = (LoAddr & 0xff);
    addr[1] = (LoAddr & 0xff00) >> 8;
    addr[2] = (LoAddr & 0xff0000) >> 16;
    addr[3] = (LoAddr & 0xff000000) >> 24;
    addr[4] = (HiAddr & 0xff);
    addr[5] = (HiAddr & 0xff00) >> 8;

    if (is_valid_ether_addr (addr))
    {
        memcpy (ndev->dev_addr, &addr, 6);
        return;
    }
    // Check if bootloader set address in Specific-Address 2 //
    HiAddr = MHal_EMAC_get_SA2H_addr(ndev);
    LoAddr = MHal_EMAC_get_SA2L_addr(ndev);
    addr[0] = (LoAddr & 0xff);
    addr[1] = (LoAddr & 0xff00) >> 8;
    addr[2] = (LoAddr & 0xff0000) >> 16;
    addr[3] = (LoAddr & 0xff000000) >> 24;
    addr[4] = (HiAddr & 0xff);
    addr[5] = (HiAddr & 0xff00) >> 8;

    if (is_valid_ether_addr (addr))
    {
        memcpy (ndev->dev_addr, &addr, 6);
        return;
    }
}

#ifdef URANUS_ETHER_ADDR_CONFIGURABLE
//-------------------------------------------------------------------------------------------------
// Store the new hardware address in dev->dev_addr, and update the MAC.
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_set_mac_address (struct net_device *ndev, void *addr)
{
    struct sockaddr *address = addr;
    if (!is_valid_ether_addr (address->sa_data))
        return -EADDRNOTAVAIL;

    memcpy (ndev->dev_addr, address->sa_data, ndev->addr_len);
    MDev_EMAC_update_mac_address (ndev);
    return 0;
}
#endif

//-------------------------------------------------------------------------------------------------
//Enable/Disable promiscuous and multicast modes.
//-------------------------------------------------------------------------------------------------
static void MDev_EMAC_set_rx_mode (struct net_device *ndev)
{
    u32 uRegVal;
    uRegVal  = MHal_EMAC_Read_CFG(ndev);
	
    if (ndev->flags & IFF_PROMISC)
    {   // Enable promiscuous mode //
        uRegVal |= EMAC_CAF;
    }
    else if (ndev->flags & (~IFF_PROMISC))
    {   // Disable promiscuous mode //
        uRegVal &= ~EMAC_CAF;
    }
    MHal_EMAC_Write_CFG(ndev, uRegVal);

    if (ndev->flags & IFF_ALLMULTI)
    {   // Enable all multicast mode //
        MHal_EMAC_update_HSH(ndev, -1, -1);
        uRegVal |= EMAC_MTI;
    }
    else if (ndev->flags & IFF_MULTICAST)
    {   // Enable specific multicasts//
        //MDev_EMAC_sethashtable (dev);
        MHal_EMAC_update_HSH(ndev, -1, -1);
        uRegVal |= EMAC_MTI;
    }
    else if (ndev->flags & ~(IFF_ALLMULTI | IFF_MULTICAST))
    {   // Disable all multicast mode//
        MHal_EMAC_update_HSH(ndev, 0, 0);
        uRegVal &= ~EMAC_MTI;
    }
	
    MHal_EMAC_Write_CFG(ndev, uRegVal);
}
//-------------------------------------------------------------------------------------------------
// IOCTL
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Enable/Disable MDIO
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_mdio_read (struct net_device *ndev, int phy_id, int location)
{
    u32 value;
    MHal_EMAC_read_phy (ndev, phy_id, location, &value);
    return value;
}

static void MDev_EMAC_mdio_write (struct net_device *ndev, int phy_id, int location, int value)
{
    MHal_EMAC_write_phy (ndev, phy_id, location, value);
}

//-------------------------------------------------------------------------------------------------
//ethtool support.
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_ethtool_ioctl (struct net_device *ndev, void *useraddr)
{
    struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);
    u32 ethcmd;
    int res = 0;

    if (copy_from_user (&ethcmd, useraddr, sizeof (ethcmd)))
        return -EFAULT;

    spin_lock_irq (LocPtr->lock);

    switch (ethcmd)
    {
        case ETHTOOL_GSET:
        {
            struct ethtool_cmd ecmd = { ETHTOOL_GSET };
            res = mii_ethtool_gset (&LocPtr->mii, &ecmd);
            if (LocPtr->phy_media == PORT_FIBRE)
            {   //override media type since mii.c doesn't know //
                ecmd.supported = SUPPORTED_FIBRE;
                ecmd.port = PORT_FIBRE;
            }
            if (copy_to_user (useraddr, &ecmd, sizeof (ecmd)))
                res = -EFAULT;
            break;
        }
        case ETHTOOL_SSET:
        {
            struct ethtool_cmd ecmd;
            if (copy_from_user (&ecmd, useraddr, sizeof (ecmd)))
                res = -EFAULT;
            else
                res = mii_ethtool_sset (&LocPtr->mii, &ecmd);
            break;
        }
        case ETHTOOL_NWAY_RST:
        {
            res = mii_nway_restart (&LocPtr->mii);
            break;
        }
        case ETHTOOL_GLINK:
        {
            struct ethtool_value edata = { ETHTOOL_GLINK };
            edata.data = mii_link_ok (&LocPtr->mii);
            if (copy_to_user (useraddr, &edata, sizeof (edata)))
                res = -EFAULT;
            break;
        }
        default:
            res = -EOPNOTSUPP;
    }
    spin_unlock_irq (LocPtr->lock);
    return res;
}

//-------------------------------------------------------------------------------------------------
// User-space ioctl interface.
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_ioctl (struct net_device *ndev, struct ifreq *rq, int cmd)
{
    struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);
    struct mii_ioctl_data *data = if_mii(rq);
	u32 bmsr;

    if (!netif_running(ndev))
    {
        rq->ifr_metric = ETHERNET_TEST_INIT_FAIL;
    }

    switch (cmd)
    {
        case SIOCGMIIPHY:
            data->phy_id = (LocPtr->phy.addr & 0x1F);
            return 0;

        case SIOCDEVPRIVATE:
            rq->ifr_metric = (MDev_EMAC_get_info(ndev)|LocPtr->initstate);
            return 0;

        case SIOCDEVON:
            MHal_EMAC_Power_On_Clk(ndev);
            return 0;

        case SIOCDEVOFF:
            MHal_EMAC_Power_Off_Clk(ndev);
            return 0;

        case SIOCGMIIREG:
            // check PHY's register 1.
            if((data->reg_num & 0x1f) == 0x1) 
            {
                // PHY's register 1 value is set by timer callback function.
                spin_lock_irq(LocPtr->lock);
                MHal_EMAC_read_phy (ndev, LocPtr->phy.addr, MII_BMSR, &bmsr);
                MHal_EMAC_read_phy (ndev, LocPtr->phy.addr, MII_BMSR, &bmsr);
				data->val_out = bmsr;
                spin_unlock_irq(LocPtr->lock);
            }
            else
            {
                MHal_EMAC_read_phy(ndev, (LocPtr->phy.addr & 0x1F), (data->reg_num & 0x1f), (u32 *)&(data->val_out));
            }
            return 0;

        case SIOCSMIIREG:
            if (!capable(CAP_NET_ADMIN))
                return -EPERM;
            MHal_EMAC_write_phy(ndev, (LocPtr->phy.addr & 0x1F), (data->reg_num & 0x1f), data->val_in);
            return 0;

        case SIOCETHTOOL:
            return MDev_EMAC_ethtool_ioctl (ndev, (void *) rq->ifr_data);

        default:
            return -EOPNOTSUPP;
    }
}
//-------------------------------------------------------------------------------------------------
// MAC
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//Initialize and start the Receiver and Transmit subsystems
//-------------------------------------------------------------------------------------------------
static void MDev_EMAC_start (struct net_device *ndev)
{
    struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);
    struct recv_desc_bufs *dlist, *dlist_phys;
#ifndef SOFTWARE_DESCRIPTOR
    int i;
#endif
    u32 uRegVal;

    dlist = LocPtr->dlist;
    dlist_phys = LocPtr->dlist_phys;
#ifdef SOFTWARE_DESCRIPTOR
    dlist->descriptors[MAX_RX_DESCR - 1].addr |= EMAC_DESC_WRAP;
#else
    for(i = 0; i < MAX_RX_DESCR; i++)
    {
        dlist->descriptors[i].addr = 0;
        dlist->descriptors[i].size = 0;
    }
    // Set the Wrap bit on the last descriptor //
    dlist->descriptors[MAX_RX_DESCR - 1].addr = EMAC_DESC_WRAP;
#endif //#ifndef SOFTWARE_DESCRIPTOR
    // set offset of read and write pointers in the receive circular buffer //
    uRegVal = MHal_EMAC_Read_BUFF(ndev);
    uRegVal = (LocPtr->RX_BUFFER_BASE|RX_BUFFER_SEL) - MIU0_BUS_BASE;
    MHal_EMAC_Write_BUFF(ndev, uRegVal);
    MHal_EMAC_Write_RDPTR(ndev, 0);
    MHal_EMAC_Write_WRPTR(ndev, 0);

    // Program address of descriptor list in Rx Buffer Queue register //
    uRegVal = ((EMAC_REG) & dlist_phys->descriptors)- LocPtr->RAM_VA_PA_OFFSET - MIU0_BUS_BASE;
    MHal_EMAC_Write_RBQP(ndev,uRegVal);

    //Reset buffer index//
    LocPtr->rxBuffIndex = 0;

    // Enable Receive and Transmit //
    uRegVal = MHal_EMAC_Read_CTL(ndev);
    uRegVal |= (EMAC_RE | EMAC_TE);
    MHal_EMAC_Write_CTL(ndev, uRegVal);
}

//-------------------------------------------------------------------------------------------------
// Open the ethernet interface
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_open (struct net_device *ndev)
{
    struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);
    u32 uRegVal;
    int ret;

    spin_lock_irq (LocPtr->lock);
    ret = MDev_EMAC_update_linkspeed(ndev);
	
    spin_unlock_irq (LocPtr->lock);
	 
    if (!is_valid_ether_addr (ndev->dev_addr))
       return -EADDRNOTAVAIL;

    //ato  EMAC_SYS->PMC_PCER = 1 << EMAC_ID_EMAC;   //Re-enable Peripheral clock //
    MHal_EMAC_Power_On_Clk(ndev);
    uRegVal = MHal_EMAC_Read_CTL(ndev);
    uRegVal |= EMAC_CSR;
    MHal_EMAC_Write_CTL(ndev, uRegVal);

    // Enable MAC interrupts //
    MHal_EMAC_Write_IER(ndev, IER_FOR_INT_JULIAN_D);

    LocPtr->ep_flag |= EP_FLAG_OPEND;
	LocPtr->tvalue = EMAC_LINK_TMR;


    MDev_EMAC_start (ndev);
    netif_start_queue (ndev);

    init_timer( &LocPtr->link_timer);
    LocPtr->link_timer.data = (unsigned long)ndev;
    LocPtr->link_timer.function = MDev_EMAC_timer_callback;
    LocPtr->link_timer.expires = jiffies + EMAC_CHECK_LINK_TIME;
    add_timer(&LocPtr->link_timer);

    /* check if network linked */
    if (-1 == ret)
        netif_carrier_off(ndev);
    else if(0 == ret)
        netif_carrier_on(ndev);

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Close the interface
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_close (struct net_device *ndev)
{
    u32 uRegVal;
    struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);
	
    //Disable Receiver and Transmitter //
    uRegVal = MHal_EMAC_Read_CTL(ndev);
    uRegVal &= ~(EMAC_TE | EMAC_RE);
    MHal_EMAC_Write_CTL(ndev, uRegVal);

    //Disable MAC interrupts //
    MHal_EMAC_Write_IDR(ndev, IER_FOR_INT_JULIAN_D);

    netif_stop_queue (ndev);
    netif_carrier_off(ndev);
    del_timer(&LocPtr->link_timer);
    LocPtr->CurBCE.connected = 0;
    LocPtr->ep_flag &= (~EP_FLAG_OPEND);

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Update the current statistics from the internal statistics registers.
//-------------------------------------------------------------------------------------------------
static struct net_device_stats * MDev_EMAC_stats (struct net_device *ndev)
{
    struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);
    int ale, lenerr, seqe, lcol, ecol;
    if (netif_running (ndev))
    {
        LocPtr->stats.rx_packets += MHal_EMAC_Read_OK(ndev);            /* Good frames received */
        ale = MHal_EMAC_Read_ALE(ndev);
        LocPtr->stats.rx_frame_errors += ale;                       /* Alignment errors */
        lenerr = MHal_EMAC_Read_ELR(ndev);
        LocPtr->stats.rx_length_errors += lenerr;                   /* Excessive Length or Undersize Frame error */
        seqe = MHal_EMAC_Read_SEQE(ndev);
        LocPtr->stats.rx_crc_errors += seqe;                        /* CRC error */
        LocPtr->stats.rx_fifo_errors += MHal_EMAC_Read_ROVR(ndev);
        LocPtr->stats.rx_errors += ale + lenerr + seqe + MHal_EMAC_Read_SE(ndev) + MHal_EMAC_Read_RJB(ndev);
        LocPtr->stats.tx_packets += MHal_EMAC_Read_FRA(ndev);           /* Frames successfully transmitted */
        LocPtr->stats.tx_fifo_errors += MHal_EMAC_Read_TUE(ndev);       /* Transmit FIFO underruns */
        LocPtr->stats.tx_carrier_errors += MHal_EMAC_Read_CSE(ndev);    /* Carrier Sense errors */
        LocPtr->stats.tx_heartbeat_errors += MHal_EMAC_Read_SQEE(ndev); /* Heartbeat error */
        lcol = MHal_EMAC_Read_LCOL(ndev);
        ecol = MHal_EMAC_Read_ECOL(ndev);
        LocPtr->stats.tx_window_errors += lcol;                     /* Late collisions */
        LocPtr->stats.tx_aborted_errors += ecol;                    /* 16 collisions */
        LocPtr->stats.collisions += MHal_EMAC_Read_SCOL(ndev) + MHal_EMAC_Read_MCOL(ndev) + lcol + ecol;
    }
    return &LocPtr->stats;
}

static int MDev_EMAC_TxReset(struct net_device *ndev)
{
    u32 val = MHal_EMAC_Read_CTL(ndev) & 0x000001FF;

    MHal_EMAC_Write_CTL(ndev, (val & ~EMAC_TE));
    MHal_EMAC_Write_TCR(ndev, 0);
    MHal_EMAC_Write_CTL(ndev, (MHal_EMAC_Read_CTL(ndev) | EMAC_TE));
    return 0;
}

static int MDev_EMAC_CheckTSR(struct net_device *ndev)
{
    u32 check;
    u32 tsrval = 0;
    struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);

#ifdef TX_QUEUE_4
    u8  avlfifo[8] = {0};
    u8  avlfifoidx;
    u8  avlfifoval = 0;

    for (check = 0; check < EMAC_CHECK_CNT; check++)
    {
        tsrval = MHal_EMAC_Read_TSR(ndev);
		
        avlfifo[0] = ((tsrval & EMAC_IDLETSR) != 0)? 1 : 0;
        avlfifo[1] = ((tsrval & EMAC_BNQ)!= 0)? 1 : 0;
        avlfifo[2] = ((tsrval & EMAC_TBNQ) != 0)? 1 : 0;
        avlfifo[3] = ((tsrval & EMAC_FBNQ) != 0)? 1 : 0;
        avlfifo[4] = ((tsrval & EMAC_FIFO1IDLE) !=0)? 1 : 0;
        avlfifo[5] = ((tsrval & EMAC_FIFO2IDLE) != 0)? 1 : 0;
        avlfifo[6] = ((tsrval & EMAC_FIFO3IDLE) != 0)? 1 : 0;
        avlfifo[7] = ((tsrval & EMAC_FIFO4IDLE) != 0)? 1 : 0;

        avlfifoval = 0;

        for(avlfifoidx = 0; avlfifoidx < 8; avlfifoidx++)
        {
            avlfifoval += avlfifo[avlfifoidx];
        }
   
        if (avlfifoval > 4)
            return NETDEV_TX_OK;
    }
#else
	
    for (check = 0; check < EMAC_CHECK_CNT; check++)
    {
        tsrval = MHal_EMAC_Read_TSR(ndev);
        if ((tsrval & EMAC_IDLETSR) || (tsrval & EMAC_BNQ))
            return NETDEV_TX_OK;
    }
    #endif

    EMAC_DBG(LocPtr->pdev->id ,"Err CheckTSR:0x%x\n", tsrval);
    MDev_EMAC_TxReset(ndev);

    return NETDEV_TX_BUSY;
}

static u8 pause_pkt[] =
{
    //DA - multicast
    0x01, 0x80, 0xC2, 0x00, 0x00, 0x01,
    //SA
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //Len-Type
    0x88, 0x08,
    //Ctrl code
    0x00, 0x01,
    //Ctrl para 8192
    0x20, 0x00
};

static dma_addr_t get_tx_addr(struct net_device* ndev)
{
    dma_addr_t addr;
	struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);

    addr = LocPtr->TX_PTK_BASE + (2048*LocPtr->txidx);
    LocPtr->txidx ++;
    LocPtr->txidx = LocPtr->txidx % TX_RING_SIZE;
	
    return addr;
}

void MDrv_EMAC_DumpMem(u32 addr, u32 len)
{
    u8 *ptr = (u8 *)addr;
    u32 i;

    printk("\n ===== Dump %lx =====\n", (long unsigned int)ptr);
    for (i=0; i<len; i++)
    {
        if ((u32)i%0x10 ==0)
            printk("%lx: ", (long unsigned int)ptr);
        if (*ptr < 0x10)
            printk("0%x ", *ptr);
        else
            printk("%x ", *ptr);
        if ((u32)i%0x10 == 0x0f)
            printk("\n");
	ptr++;
    }
    printk("\n");
}

//Background send
static int MDev_EMAC_BGsend(struct net_device* ndev, u32 addr, int len )
{
	dma_addr_t skb_addr;
    struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);

    if (NETDEV_TX_OK != MDev_EMAC_CheckTSR(ndev))
        return NETDEV_TX_BUSY;

    skb_addr = get_tx_addr(ndev);
    memcpy((void*)skb_addr,(void *)addr, len);

    LocPtr->stats.tx_bytes += len;

#ifdef CHIP_FLUSH_READ
    #if defined(CONFIG_MIPS)
    if((unsigned int)skb_addr < 0xC0000000)
    {
        Chip_Flush_Memory_Range((unsigned int)skb_addr&0x0FFFFFFF, len);
    }
    else
    {
        Chip_Flush_Memory_Range(0, 0xFFFFFFFF);
    }
    #elif defined(CONFIG_ARM)
        Chip_Flush_Memory_Range(0, 0xFFFFFFFF);
    #else
	    #ERROR
    #endif
#endif

    //Set address of the data in the Transmit Address register //
    MHal_EMAC_Write_TAR(ndev, skb_addr - LocPtr->RAM_VA_PA_OFFSET - MIU0_BUS_BASE);

    // Set length of the packet in the Transmit Control register //
    MHal_EMAC_Write_TCR(ndev, len);

    return NETDEV_TX_OK;
}

static void MDev_EMAC_Send_PausePkt(struct net_device* ndev)
{
    u32 val = MHal_EMAC_Read_CTL(ndev) & 0x000001FF;

    //Disable Rx
    MHal_EMAC_Write_CTL(ndev, (val & ~EMAC_RE));
    memcpy(&pause_pkt[6], ndev->dev_addr, 6);
    MDev_EMAC_BGsend(ndev, (u32)pause_pkt, sizeof(pause_pkt));
    //Enable Rx
    MHal_EMAC_Write_CTL(ndev, (MHal_EMAC_Read_CTL(ndev) | EMAC_RE));
}

//-------------------------------------------------------------------------------------------------
//Patch for losing small-size packet when running SMARTBIT
//-------------------------------------------------------------------------------------------------
#ifdef CONFIG_MP_ETHERNET_MSTAR_ICMP_ENHANCE
static void MDev_EMAC_Period_Retry(struct sk_buff *skb, struct net_device* ndev)
{
    u32 xval;
    u32 uRegVal;
	struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);

    xval = MHal_EMAC_ReadReg32(ndev ,REG_ETH_CFG);

    if((skb->len <= PACKET_THRESHOLD) && !(xval & EMAC_SPD) && !(xval & EMAC_FD))
    {
        LocPtr->txcount++;
    }
    else
    {
        LocPtr->txcount = 0;
    }

    if(LocPtr->txcount > TXCOUNT_THRESHOLD)
    {
        uRegVal  = MHal_EMAC_Read_CFG(ndev);
        uRegVal  |= 0x00001000;
        MHal_EMAC_Write_CFG(ndev, uRegVal);
    }
    else
    {
        uRegVal = MHal_EMAC_Read_CFG(ndev);
        uRegVal &= ~(0x00001000);
        MHal_EMAC_Write_CFG(ndev, uRegVal);
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//Transmit packet.
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_tx (struct sk_buff *skb, struct net_device *ndev)
{
    struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);
    unsigned long flags;
    dma_addr_t skb_addr;

    spin_lock_irqsave(LocPtr->lock, flags);
    if (skb->len > EMAC_MTU)
    {
        EMAC_DBG(LocPtr->pdev->id, "Wrong Tx len:%u\n", skb->len);
        spin_unlock_irqrestore(LocPtr->lock, flags);
        return NETDEV_TX_BUSY;
    }

    if (NETDEV_TX_OK != MDev_EMAC_CheckTSR(ndev))
    {
	    spin_unlock_irqrestore(LocPtr->lock, flags);
	    return NETDEV_TX_BUSY; //check
    }


    skb_addr = get_tx_addr(ndev);
    memcpy((void*)skb_addr, skb->data, skb->len);

#ifdef CONFIG_MP_ETHERNET_MSTAR_ICMP_ENHANCE
    MDev_EMAC_Period_Retry(skb, ndev);
#endif


    // Store packet information (to free when Tx completed) //
    LocPtr->stats.tx_bytes += skb->len;

#ifdef CHIP_FLUSH_READ
    #if defined(CONFIG_MIPS)
    if((unsigned int)skb_addr < 0xC0000000)
    {
        Chip_Flush_Memory_Range((unsigned int)skb_addr&0x0FFFFFFF, skb->len);
    }
    else
    {
        Chip_Flush_Memory_Range(0, 0xFFFFFFFF);
    }
    #elif defined(CONFIG_ARM)
        Chip_Flush_Memory_Range(0, 0xFFFFFFFF);
    #else
	    #ERROR
    #endif
#endif
    //MDrv_EMAC_DumpMem(tx_fifo->skb_physaddr,skb->len);
    //Set address of the data in the Transmit Address register //
    MHal_EMAC_Write_TAR(ndev, skb_addr - LocPtr->RAM_VA_PA_OFFSET - MIU0_BUS_BASE);

    // Set length of the packet in the Transmit Control register //
    MHal_EMAC_Write_TCR(ndev, skb->len);

    ndev->trans_start = jiffies;
    dev_kfree_skb_irq(skb);
    spin_unlock_irqrestore(LocPtr->lock, flags);
    return NETDEV_TX_OK;
}

//-------------------------------------------------------------------------------------------------
// Extract received frame from buffer descriptors and sent to upper layers.
// (Called from interrupt context)
// (Disable RX software discriptor)
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_rx (struct net_device *ndev)
{
    struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);
    struct recv_desc_bufs *dlist;
    unsigned char *p_recv;
    u32 pktlen;
    u32 retval=0;
    struct sk_buff *skb;

    dlist = LocPtr->dlist ;
    // If any Ownership bit is 1, frame received.
    do
    {
#ifdef CHIP_FLUSH_READ
    #if defined(CONFIG_MIPS)
        Chip_Read_Memory_Range((unsigned int)(&(dlist->descriptors[LocPtr->rxBuffIndex].addr)) & 0x0FFFFFFF, sizeof(dlist->descriptors[LocPtr->rxBuffIndex].addr));
    #elif defined(CONFIG_ARM)
        Chip_Inv_Cache_Range_VA_PA((unsigned int)(&(dlist->descriptors[LocPtr->rxBuffIndex].addr)),(unsigned int)(&(dlist->descriptors[LocPtr->rxBuffIndex].addr)) - LocPtr->RAM_VA_PA_OFFSET ,sizeof(dlist->descriptors[LocPtr->rxBuffIndex].addr));
	#else
        #ERROR
    #endif
#endif
        if(!((dlist->descriptors[LocPtr->rxBuffIndex].addr) & EMAC_DESC_DONE))
        {
            break;
        }

        p_recv = (char *) ((((dlist->descriptors[LocPtr->rxBuffIndex].addr) & 0xFFFFFFFF) + LocPtr->RAM_VA_PA_OFFSET + MIU0_BUS_BASE) &~(EMAC_DESC_DONE | EMAC_DESC_WRAP));
        pktlen = dlist->descriptors[LocPtr->rxBuffIndex].size & 0x7ff;    /* Length of frame including FCS */
        skb = alloc_skb (pktlen + 6, GFP_ATOMIC);

        if (skb != NULL)
        {
            skb_reserve (skb, 2);
    #ifdef CHIP_FLUSH_READ
        #if defined(CONFIG_MIPS)
            if((unsigned int)p_recv < 0xC0000000)
            {
                Chip_Read_Memory_Range((unsigned int)(p_recv) & 0x0FFFFFFF, pktlen);
            }
            else
            {
                Chip_Read_Memory_Range(0, 0xFFFFFFFF);
            }
        #elif defined(CONFIG_ARM)
            Chip_Inv_Cache_Range_VA_PA((unsigned int)p_recv, (unsigned int)(p_recv - LocPtr->RAM_VA_PA_OFFSET), pktlen);
        #else
            #ERROR
        #endif
    #endif

#ifdef CONFIG_K3_RX_SWPATCH
            if (((p_recv[4]==ndev->dev_addr[0])&&(p_recv[5]==ndev->dev_addr[1])&&(p_recv[6]==ndev->dev_addr[2])&&(p_recv[7]==ndev->dev_addr[3])&&(p_recv[8]==ndev->dev_addr[4])&&(p_recv[9]==ndev->dev_addr[5])) || ((p_recv[4]==0xFF)&&(p_recv[5]==0xFF)&&(p_recv[6]==0xFF)&&(p_recv[7]==0xFF)&&(p_recv[8]==0xFF)&&(p_recv[9]==0xFF)))
            {
                memcpy(skb_put(skb, pktlen), p_recv+4, pktlen);
            }
            else
            {
                memcpy(skb_put(skb, pktlen), p_recv, pktlen);
            }
#else
            memcpy(skb_put(skb, pktlen), p_recv, pktlen);
#endif
            skb->dev = ndev;
            skb->protocol = eth_type_trans (skb, ndev);
            skb->len = pktlen;
            ndev->last_rx = jiffies;
            LocPtr->stats.rx_bytes += pktlen;

        #ifdef RX_CHECKSUM
            if(((dlist->descriptors[LocPtr->rxBuffIndex].size & EMAC_DESC_TCP ) || (dlist->descriptors[LocPtr->rxBuffIndex].size & EMAC_DESC_UDP )) && \
               (dlist->descriptors[LocPtr->rxBuffIndex].size & EMAC_DESC_IP_CSUM) && \
               (dlist->descriptors[LocPtr->rxBuffIndex].size & EMAC_DESC_TCP_UDP_CSUM) )
            {
                skb->ip_summed = CHECKSUM_UNNECESSARY;
            }
            else
            {
                skb->ip_summed = CHECKSUM_NONE;
            }
        #endif
		
            retval = netif_rx (skb);
        }
        else
        {
            LocPtr->stats.rx_dropped += 1;
        }

        if (dlist->descriptors[LocPtr->rxBuffIndex].size & EMAC_MULTICAST)
        {
            LocPtr->stats.multicast++;
        }
        dlist->descriptors[LocPtr->rxBuffIndex].addr  &= ~EMAC_DESC_DONE;  /* reset ownership bit */
#ifdef CHIP_FLUSH_READ
    #if defined(CONFIG_MIPS)
        Chip_Flush_Memory_Range((unsigned int)(&(dlist->descriptors[LocPtr->rxBuffIndex].addr)) & 0x0FFFFFFF, sizeof(dlist->descriptors[LocPtr->rxBuffIndex].addr));
    #elif defined(CONFIG_ARM)
        Chip_Inv_Cache_Range_VA_PA((unsigned int)(&(dlist->descriptors[LocPtr->rxBuffIndex].addr)),(unsigned int)(&(dlist->descriptors[LocPtr->rxBuffIndex].addr)) - LocPtr->RAM_VA_PA_OFFSET ,sizeof(dlist->descriptors[LocPtr->rxBuffIndex].addr));
    #else
        #ERROR
    #endif
#endif
        //wrap after last buffer //
        LocPtr->rxBuffIndex++;
        if (LocPtr->rxBuffIndex == MAX_RX_DESCR)
        {
            LocPtr->rxBuffIndex = 0;
        }

    #ifdef CONFIG_EMAC_SUPPLY_RNG
        {
            static unsigned long u32LastInputRNGJiff=0;
            unsigned long u32Jiff=jiffies;

            if ( time_after(u32Jiff, u32LastInputRNGJiff+InputRNGJiffThreshold) )
            {
                unsigned int u32Temp;
                unsigned short u16Temp;

                u32LastInputRNGJiff = u32Jiff;
                u16Temp = MIPS_REG(REG_RNG_OUT);
                memcpy((unsigned char *)&u32Temp+0, &u16Temp, 2);
                u16Temp = MIPS_REG(REG_RNG_OUT);
                memcpy((unsigned char *)&u32Temp+2, &u16Temp, 2);
                add_input_randomness(EV_MSC, MSC_SCAN, u32Temp);
            }
        }
    #endif

        if(LocPtr->CurUVE.flagRBNA == 0)
        {
            LocPtr->xReceiveNum--;
            if(LocPtr->xReceiveNum==0)
                  return 0;
        }

    }while(1);

    LocPtr->xReceiveNum=0;
    LocPtr->CurUVE.flagRBNA=0;
    return 0;
}

//-------------------------------------------------------------------------------------------------
//MAC interrupt handler
//(Interrupt delay enable)
//-------------------------------------------------------------------------------------------------

void MDev_EMAC_bottom_task(struct work_struct *work)
{
    struct EMAC_private *LocPtr = container_of(work, struct EMAC_private, task);
    struct net_device *ndev = LocPtr->dev;

    MDev_EMAC_rx(ndev);
}

irqreturn_t MDev_EMAC_interrupt(int irq, void *dev_id)
{
    struct net_device *ndev = (struct net_device *) dev_id;
    struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);
    u32 intstatus=0;
    unsigned long flags;
#ifndef RX_SOFTWARE_DESCRIPTOR
    u32 wp = 0;
#endif

    spin_lock_irqsave(LocPtr->lock, flags);

    //MAC Interrupt Status register indicates what interrupts are pending.
    //It is automatically cleared once read.
    LocPtr->xoffsetValue = MHal_EMAC_Read_JULIAN_0108(ndev) & 0x0000FFFF;
    LocPtr->xReceiveNum += LocPtr->xoffsetValue & 0xFF;

#ifndef RX_SOFTWARE_DESCRIPTOR
    wp = MHal_EMAC_Read_JULIAN_0100(ndev) & 0x00100000;
    if(wp)
    {
        EMAC_DBG(LocPtr->pdev->id, "EMAC HW write invalid address");
    }
#endif

    if(LocPtr->xoffsetValue & 0x8000)
    {
        LocPtr->xReceiveFlag = 1;
    }
    LocPtr->CurUVE.flagRBNA = 0;

    LocPtr->oldTime = getCurMs();
    while((LocPtr->xReceiveFlag == 1) || (intstatus = (MHal_EMAC_Read_ISR(ndev) & ~MHal_EMAC_Read_IMR(ndev) & EMAC_INT_MASK )) )
    {
        if (intstatus & EMAC_INT_RBNA)
        {
            LocPtr->stats.rx_dropped ++;
            LocPtr->CurUVE.flagRBNA = 1;
            LocPtr->xReceiveFlag = 1;
            //write 1 clear
            MHal_EMAC_Write_RSR(ndev, EMAC_BNA);
        }

        // Transmit complete //
        if (intstatus & EMAC_INT_TCOM)
        {
            // The TCOM bit is set even if the transmission failed. //
            if (intstatus & (EMAC_INT_TUND | EMAC_INT_RTRY))
            {
                LocPtr->stats.tx_errors += 1;
                if(intstatus & EMAC_INT_TUND)
                {
                    //write 1 clear
                    MHal_EMAC_Write_TSR(ndev, EMAC_UND);
                    //EMAC_DBG (LocPtr->pdev->id,"==> %s: Transmit TUND error\n", dev->name);
                }
            }
            else
            {
                LocPtr->retx_count = 0;
            }

            if (((LocPtr->ep_flag&EP_FLAG_SUSPENDING)==0) && netif_queue_stopped (ndev))
                netif_wake_queue(ndev);
        }

        if(intstatus&EMAC_INT_DONE)
        {
            LocPtr->CurUVE.flagISR_INT_DONE = 0x01;
        }

        //Overrun //
        if(intstatus & EMAC_INT_ROVR)
        {
            LocPtr->stats.rx_dropped++;
            LocPtr->contiROVR++;
            //write 1 clear
            MHal_EMAC_Write_RSR(ndev, EMAC_RSROVR);
            //EMAC_DBG (LocPtr->pdev->id,"==> %s: ROVR error %u times!\n", dev->name, contiROVR);
            if (LocPtr->contiROVR < 3)
            {
                MDev_EMAC_Send_PausePkt(ndev);
            }
            else
            {
                MDev_EMAC_SwReset(ndev);
                LocPtr->xReceiveNum = 0;
            }
        }
        else
        {
            LocPtr->contiROVR = 0;
        }

        if(LocPtr->xReceiveNum != 0)
        {
            LocPtr->xReceiveFlag = 1;
        }

        // Receive complete //
        if(LocPtr->xReceiveFlag == 1)
        {
            LocPtr->xReceiveFlag = 0;
        #ifdef ISR_BOTTOM_HALF
            schedule_work(&LocPtr->task);
        #else
            MDev_EMAC_rx(ndev);
        #endif
        }
    }
    spin_unlock_irqrestore(LocPtr->lock, flags);
    return IRQ_HANDLED;
}

//-------------------------------------------------------------------------------------------------
// EMAC Hardware register set
//-------------------------------------------------------------------------------------------------
void MDev_EMAC_HW_init(struct net_device *ndev)
{
    u32 word_ETH_CTL = 0x00000000;
    u32 word_ETH_CFG = 0x00000800;
    u32 uJulian104Value = 0;
    u32 uNegPhyVal = 0;
#ifdef SOFTWARE_DESCRIPTOR
    u32 idxRBQP = 0;
    u32 RBQP_offset = 0;
#endif
	struct EMAC_private *LocPtr = (struct EMAC_private *) netdev_priv(ndev);


    // (20071026_CHARLES) Disable TX, RX and MDIO:   (If RX still enabled, the RX buffer will be overwrited)
    MHal_EMAC_Write_CTL(ndev, word_ETH_CTL);
    // Init RX --------------------------------------------------------------
    memset((u8*)LocPtr->RAM_VA_PA_OFFSET + LocPtr->RX_BUFFER_BASE, 0x00, RX_BUFFER_SIZE);

    MHal_EMAC_Write_BUFF(ndev, (LocPtr->RX_BUFFER_BASE | RX_BUFFER_SEL) - MIU0_BUS_BASE);
    MHal_EMAC_Write_RDPTR(ndev, 0x00000000);
    MHal_EMAC_Write_WRPTR(ndev, 0x00000000);
	
    // Initialize "Receive Buffer Queue Pointer"
    MHal_EMAC_Write_RBQP(ndev, LocPtr->RBQP_BASE - MIU0_BUS_BASE);
	EMAC_DBG(LocPtr->pdev->id, "RBQP_BASE = %x\n", LocPtr->RBQP_BASE);
	
    // Initialize Receive Buffer Descriptors
    memset((u8*)LocPtr->RAM_VA_PA_OFFSET + LocPtr->RBQP_BASE, 0x00, RBQP_SIZE);        // Clear for max(8*1024)bytes (max:1024 descriptors)
    MHal_EMAC_WritRam32(LocPtr->RAM_VA_PA_OFFSET, (LocPtr->RBQP_BASE + RBQP_SIZE - 0x08), 0x00000002);             // (n-1) : Wrap = 1

    //Reg_rx_frame_cyc[15:8] -0xFF range 1~255
    //Reg_rx_frame_num[7:0]  -0x05 receive frames per INT.
    //0x80 Enable interrupt delay mode.
    //register 0x104 receive counter need to modify smaller for ping
    //Modify bigger(need small than 8) for throughput
    uJulian104Value = JULIAN_104_VAL;//0xFF050080;
    MHal_EMAC_Write_JULIAN_0104(ndev, uJulian104Value);

    // Set MAC address ------------------------------------------------------
    MHal_EMAC_Write_SA1_MAC_Address(ndev, LocPtr->CurBCE.sa1[0], LocPtr->CurBCE.sa1[1], LocPtr->CurBCE.sa1[2], LocPtr->CurBCE.sa1[3], LocPtr->CurBCE.sa1[4], LocPtr->CurBCE.sa1[5]);
    MHal_EMAC_Write_SA2_MAC_Address(ndev, LocPtr->CurBCE.sa2[0], LocPtr->CurBCE.sa2[1], LocPtr->CurBCE.sa2[2], LocPtr->CurBCE.sa2[3], LocPtr->CurBCE.sa2[4], LocPtr->CurBCE.sa2[5]);
    MHal_EMAC_Write_SA3_MAC_Address(ndev, LocPtr->CurBCE.sa3[0], LocPtr->CurBCE.sa3[1], LocPtr->CurBCE.sa3[2], LocPtr->CurBCE.sa3[3], LocPtr->CurBCE.sa3[4], LocPtr->CurBCE.sa3[5]);
    MHal_EMAC_Write_SA4_MAC_Address(ndev, LocPtr->CurBCE.sa4[0], LocPtr->CurBCE.sa4[1], LocPtr->CurBCE.sa4[2], LocPtr->CurBCE.sa4[3], LocPtr->CurBCE.sa4[4], LocPtr->CurBCE.sa4[5]);

#ifdef SOFTWARE_DESCRIPTOR
    #ifdef RX_CHECKSUM
    uJulian104Value=uJulian104Value | (RX_CHECKSUM_ENABLE | SOFTWARE_DESCRIPTOR_ENABLE);
    #else
    uJulian104Value=uJulian104Value | SOFTWARE_DESCRIPTOR_ENABLE;
    #endif

    MHal_EMAC_Write_JULIAN_0104(ndev, uJulian104Value);
    for(idxRBQP = 0; idxRBQP < RBQP_LENG; idxRBQP++)
    {
    #ifdef RX_SOFTWARE_DESCRIPTOR
        rx_skb[idxRBQP] = alloc_skb(SOFTWARE_DESC_LEN, GFP_ATOMIC);

        rx_abso_addr[idxRBQP] = (u32)rx_skb[idxRBQP]->data;
        RBQP_offset = idxRBQP * 8;
        if(idxRBQP < (RBQP_LENG - 1))
        {
            MHal_EMAC_WritRam32(LocPtr->RAM_VA_PA_OFFSET, LocPtr->RBQP_BASE + RBQP_offset, rx_abso_addr[idxRBQP]);
        }
        else
        {
            MHal_EMAC_WritRam32(LocPtr->RAM_VA_PA_OFFSET, LocPtr->RBQP_BASE + RBQP_offset, (rx_abso_addr[idxRBQP] + EMAC_DESC_WRAP));
        }
    #else
            RBQP_offset = idxRBQP * 8;
        if(idxRBQP < (RBQP_LENG - 1))
        {
            MHal_EMAC_WritRam32(LocPtr->RAM_VA_PA_OFFSET, LocPtr->RBQP_BASE + RBQP_offset, (LocPtr->RX_BUFFER_BASE - MIU0_BUS_BASE + SOFTWARE_DESC_LEN * idxRBQP));
        }
        else
        {
            MHal_EMAC_WritRam32(LocPtr->RAM_VA_PA_OFFSET, LocPtr->RBQP_BASE + RBQP_offset, (LocPtr->RX_BUFFER_BASE - MIU0_BUS_BASE + SOFTWARE_DESC_LEN * idxRBQP + EMAC_DESC_WRAP));
        }
    #endif
    }
#ifdef RX_SOFTWARE_DESCRIPTOR
	rx_skb_dummy = 	alloc_skb(SOFTWARE_DESC_LEN, GFP_ATOMIC);
	if(rx_skb_dummy == NULL)
    {
        EMAC_DBG(LocPtr->pdev->id, KERN_INFO "allocate skb dummy failed\n");
    }
	else
    {
	    rx_abso_addr_dummy = (u32)(rx_skb_dummy->data);
    }
#endif

#endif //#ifdef SOFTWARE_DESCRIPTOR

    if (!LocPtr->CurUVE.initedEMAC)
    {
        MHal_EMAC_write_phy(ndev, LocPtr->phy.addr, MII_BMCR, 0x9000);
        MHal_EMAC_write_phy(ndev, LocPtr->phy.addr, MII_BMCR, 0x1000);
        // IMPORTANT: Run NegotiationPHY() before writing REG_ETH_CFG.
        uNegPhyVal = MHal_EMAC_NegotiationPHY(ndev, LocPtr->phy.addr);
        if(uNegPhyVal == 0x01)
        {
            LocPtr->CurUVE.flagMacTxPermit = 0x01;
            LocPtr->CurBCE.duplex = 1;

        }
        else if(uNegPhyVal == 0x02)
        {
            LocPtr->CurUVE.flagMacTxPermit = 0x01;
            LocPtr->CurBCE.duplex = 2;
        }

        // ETH_CFG Register -----------------------------------------------------
        word_ETH_CFG = 0x00000800;        // Init: CLK = 0x2
        // (20070808) IMPORTANT: REG_ETH_CFG:bit1(FD), 1:TX will halt running RX frame, 0:TX will not halt running RX frame.
        // If always set FD=0, no CRC error will occur. But throughput maybe need re-evaluate.
        // IMPORTANT: (20070809) NO_MANUAL_SET_DUPLEX : The real duplex is returned by "negotiation"
        if(LocPtr->CurBCE.speed     == EMAC_SPEED_100) word_ETH_CFG |= 0x00000001;
        if(LocPtr->CurBCE.duplex    == 2)              word_ETH_CFG |= 0x00000002;
        if(LocPtr->CurBCE.cam       == 1)              word_ETH_CFG |= 0x00000200;
        if(LocPtr->CurBCE.rcv_bcast == 0)              word_ETH_CFG |= 0x00000020;
        if(LocPtr->CurBCE.rlf       == 1)              word_ETH_CFG |= 0x00000100;

        MHal_EMAC_Write_CFG(ndev, word_ETH_CFG);
        // ETH_CTL Register -----------------------------------------------------
        word_ETH_CTL = 0x0000000C;                          // Enable transmit and receive : TE + RE = 0x0C (Disable MDIO)
        if(LocPtr->CurBCE.wes == 1) word_ETH_CTL |= 0x00000080;
        MHal_EMAC_Write_CTL(ndev, word_ETH_CTL);

        if (LocPtr->phytype == PHY_INTERNAL)
        {
            MHal_EMAC_Write_JULIAN_0100(ndev, JULIAN_100_VAL);
            MHal_EMAC_Write_JULIAN_0100(ndev, 0x0000F001);
        }
        else if (LocPtr->phytype == PHY_EXTERNAL)
        {
            MHal_EMAC_Write_JULIAN_0100(ndev, JULIAN_100_VAL_EXT);
        }

        LocPtr->CurUVE.flagPowerOn = 1;
        LocPtr->CurUVE.initedEMAC  = 1;
    }

    MHal_EMAC_HW_init(ndev);
}


//-------------------------------------------------------------------------------------------------
// EMAC init Variable
//-------------------------------------------------------------------------------------------------
static u32 MDev_EMAC_VarInit(struct net_device *ndev)
{
    u32 alloRAM_PA_BASE;
    u32 alloRAM_SIZE;
    char addr[6];
    u32 HiAddr, LoAddr;
    u32 *alloRAM_VA_BASE;
	struct EMAC_private *LocPtr = (struct EMAC_private *) netdev_priv(ndev);

    get_boot_mem_info(EMAC_MEM, &alloRAM_PA_BASE, &alloRAM_SIZE);

    if(alloRAM_SIZE < 0x100000)
    {
        EMAC_DBG(LocPtr->pdev->id, "EMAC_MEM should be large than or equal to 1MB");
        return 0;
    }
        
    if (LocPtr->pdev->id == 0)
    {
        alloRAM_PA_BASE =  alloRAM_PA_BASE;
		alloRAM_SIZE = 0x80000;
    }
	else
    {
        alloRAM_PA_BASE =  alloRAM_PA_BASE + 0x80000;
        alloRAM_SIZE = 0x80000;
    }
	
    alloRAM_VA_BASE = (u32 *)ioremap(alloRAM_PA_BASE, alloRAM_SIZE); //map buncing buffer from PA to VA

    EMAC_DBG(LocPtr->pdev->id, "alloRAM_VA_BASE = 0x%X alloRAM_PA_BASE= 0x%X  alloRAM_SIZE= 0x%X\n", (u32)alloRAM_VA_BASE, alloRAM_PA_BASE, alloRAM_SIZE);

#ifndef RX_SOFTWARE_DESCRIPTOR
    //Add Write Protect
    MHal_EMAC_Write_Protect(ndev, alloRAM_PA_BASE, alloRAM_SIZE);
#endif
    memset((u32 *)alloRAM_VA_BASE,0x00,alloRAM_SIZE);

    LocPtr->RAM_VA_BASE = ((u32)alloRAM_VA_BASE + sizeof(struct EMAC_private) + 0x3FFF) & ~0x3FFF;   // IMPORTANT: Let lowest 14 bits as zero.
    LocPtr->RAM_PA_BASE = ((u32)alloRAM_PA_BASE + sizeof(struct EMAC_private) + 0x3FFF) & ~0x3FFF;   // IMPORTANT: Let lowest 14 bits as zero.
	LocPtr->RX_BUFFER_BASE = LocPtr->RAM_PA_BASE + RBQP_SIZE;
	LocPtr->RBQP_BASE = LocPtr->RAM_PA_BASE; 
	LocPtr->TX_BUFFER_BASE = LocPtr->RAM_PA_BASE + (RX_BUFFER_SIZE + RBQP_SIZE);
    LocPtr->RAM_VA_PA_OFFSET =  LocPtr->RAM_VA_BASE - LocPtr->RAM_PA_BASE;
	LocPtr->TX_SKB_BASE = LocPtr->TX_BUFFER_BASE + MAX_RX_DESCR * sizeof(struct rbf_t);
	LocPtr->TX_PTK_BASE  = LocPtr->TX_SKB_BASE + LocPtr->RAM_VA_PA_OFFSET;

    memset(&LocPtr->CurBCE,0x00,sizeof(BasicConfigEMAC));
    memset(&LocPtr->CurUVE,0x00,sizeof(UtilityVarsEMAC));

    LocPtr->CurBCE.wes          = 0;             		// 0:Disable, 1:Enable (WR_ENABLE_STATISTICS_REGS)
    LocPtr->CurBCE.duplex       = 2;                    // 1:Half-duplex, 2:Full-duplex
    LocPtr->CurBCE.cam			= 0;                 	// 0:No CAM, 1:Yes
    LocPtr->CurBCE.rcv_bcast	= 0;                  	// 0:No, 1:Yes
    LocPtr->CurBCE.rlf  		= 0;                 	// 0:No, 1:Yes receive long frame(1522)
    LocPtr->CurBCE.rcv_bcast    = 1;
    LocPtr->CurBCE.speed        = EMAC_SPEED_100;

    // Check if bootloader set address in Specific-Address 1 //
    HiAddr = MHal_EMAC_get_SA1H_addr(ndev);
    LoAddr = MHal_EMAC_get_SA1L_addr(ndev);

    addr[0] = (LoAddr & 0xff);
    addr[1] = (LoAddr & 0xff00) >> 8;
    addr[2] = (LoAddr & 0xff0000) >> 16;
    addr[3] = (LoAddr & 0xff000000) >> 24;
    addr[4] = (HiAddr & 0xff);
    addr[5] = (HiAddr & 0xff00) >> 8;

    if (is_valid_ether_addr (addr))
    {
        memcpy (LocPtr->CurBCE.sa1, &addr, 6);
    }
    else
    {
        // Check if bootloader set address in Specific-Address 2 //
        HiAddr = MHal_EMAC_get_SA2H_addr(ndev);
        LoAddr = MHal_EMAC_get_SA2L_addr(ndev);
        addr[0] = (LoAddr & 0xff);
        addr[1] = (LoAddr & 0xff00) >> 8;
        addr[2] = (LoAddr & 0xff0000) >> 16;
        addr[3] = (LoAddr & 0xff000000) >> 24;
        addr[4] = (HiAddr & 0xff);
        addr[5] = (HiAddr & 0xff00) >> 8;

        if (is_valid_ether_addr (addr))
        {
            memcpy (LocPtr->CurBCE.sa1, &addr, 6);
        }
        else
        {    
            /*Set mac address by netdev id*/
            if (LocPtr->pdev->id == 0) 
            {
                LocPtr->CurBCE.sa1[0] = MY_MAC[0][0];
                LocPtr->CurBCE.sa1[1] = MY_MAC[0][1];
                LocPtr->CurBCE.sa1[2] = MY_MAC[0][2];
                LocPtr->CurBCE.sa1[3] = MY_MAC[0][3];
                LocPtr->CurBCE.sa1[4] = MY_MAC[0][4];
                LocPtr->CurBCE.sa1[5] = MY_MAC[0][5];
            }
            else
            {
                LocPtr->CurBCE.sa1[0] = MY_MAC[1][0];
                LocPtr->CurBCE.sa1[1] = MY_MAC[1][1];
                LocPtr->CurBCE.sa1[2] = MY_MAC[1][2];
                LocPtr->CurBCE.sa1[3] = MY_MAC[1][3];
                LocPtr->CurBCE.sa1[4] = MY_MAC[1][4];
                LocPtr->CurBCE.sa1[5] = MY_MAC[1][5];
            }	
        }
    }
    LocPtr->CurBCE.connected = 0;

    return (u32)alloRAM_VA_BASE;
}

//-------------------------------------------------------------------------------------------------
// Initialize the ethernet interface
// @return TRUE : Yes
// @return FALSE : FALSE
//-------------------------------------------------------------------------------------------------

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
static const struct net_device_ops mstar_lan_netdev_ops = {
        .ndo_open               = MDev_EMAC_open,
        .ndo_stop               = MDev_EMAC_close,
        .ndo_start_xmit         = MDev_EMAC_tx,
        .ndo_set_mac_address    = MDev_EMAC_set_mac_address,
        .ndo_set_multicast_list = MDev_EMAC_set_rx_mode,
        .ndo_do_ioctl           = MDev_EMAC_ioctl,
        .ndo_get_stats          = MDev_EMAC_stats,
};

static int MDev_EMAC_get_settings(struct net_device *ndev, struct ethtool_cmd *cmd)
{
    struct EMAC_private *LocPtr =(struct EMAC_private *) netdev_priv(ndev);
	
    mii_ethtool_gset (&LocPtr->mii, cmd);

	if (LocPtr->phy_media == PORT_FIBRE)
	{	
	    //override media type since mii.c doesn't know //
		cmd->supported = SUPPORTED_FIBRE;
		cmd->port = PORT_FIBRE;
	}

	return 0;
}

static int MDev_EMAC_set_settings(struct net_device *ndev, struct ethtool_cmd *cmd)
{
    struct EMAC_private *LocPtr =(struct EMAC_private *) netdev_priv(ndev);
	
    mii_ethtool_sset (&LocPtr->mii, cmd);
	
    return 0;
}

static int MDev_EMAC_nway_reset(struct net_device *ndev)
{
    struct EMAC_private *LocPtr =(struct EMAC_private *) netdev_priv(ndev);
	
    mii_nway_restart (&LocPtr->mii);
		
    return 0;
}

static u32 MDev_EMAC_get_link(struct net_device *ndev)
{
    u32	u32data;
    struct EMAC_private *LocPtr =(struct EMAC_private *) netdev_priv(ndev);
	
    u32data = mii_link_ok (&LocPtr->mii);

    return u32data;
}

static const struct ethtool_ops ethtool_ops = {
    .get_settings = MDev_EMAC_get_settings,
    .set_settings = MDev_EMAC_set_settings,
    .nway_reset   = MDev_EMAC_nway_reset,
    .get_link     = MDev_EMAC_get_link,    
};

#endif
static int MDev_EMAC_setup (struct net_device *ndev, unsigned long phy_type)
{
    struct EMAC_private *LocPtr = (struct EMAC_private *) netdev_priv(ndev);
    dma_addr_t dmaaddr;
    u32 val;
    u32 RetAddr;
#ifdef CONFIG_MSTAR_HW_TX_CHECKSUM
    u32 retval;
#endif
    if (LocPtr->initialized)
    {
        EMAC_DBG(LocPtr->pdev->id, "Mstar emac has initilaized\n");
        return FALSE;
    }

    /*Init the bottom half ISR task*/
    INIT_WORK(&LocPtr->task, MDev_EMAC_bottom_task);

    LocPtr->dev = ndev;
    RetAddr = MDev_EMAC_VarInit(ndev);
    if(!RetAddr)
    {
        EMAC_DBG(LocPtr->pdev->id, "Var init fail!!\n");
        return FALSE;
    }

    if (LocPtr == NULL)
    {
        free_irq (ndev->irq, ndev);
        EMAC_DBG(LocPtr->pdev->id, "LocPtr fail\n");
        return -ENOMEM;
    }

    MDev_EMAC_HW_init(ndev);

    // Allocate memory for DMA Receive descriptors //
    LocPtr->dlist_phys = LocPtr->dlist = (struct recv_desc_bufs *) (LocPtr->RBQP_BASE + LocPtr->RAM_VA_PA_OFFSET);

    if (LocPtr->dlist == NULL)
    {
        dma_free_noncoherent((void *)LocPtr, EMAC_ABSO_MEM_SIZE,&dmaaddr,0);//kfree (dev->priv);
        free_irq (ndev->irq, ndev);
        return -ENOMEM;
    }

    LocPtr->lock = &LocPtr->plock;
    spin_lock_init (LocPtr->lock);
    ether_setup (ndev);
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,28)
    ndev->open = MDev_EMAC_open;
    ndev->stop = MDev_EMAC_close;
    ndev->hard_start_xmit = MDev_EMAC_tx;
    ndev->get_stats = MDev_EMAC_stats;
    ndev->set_multicast_list = MDev_EMAC_set_rx_mode;
    ndev->do_ioctl = MDev_EMAC_ioctl;
    ndev->set_mac_address = MDev_EMAC_set_mac_address;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
    ndev->netdev_ops = &mstar_lan_netdev_ops;
#endif
    ndev->tx_queue_len = EMAC_MAX_TX_QUEUE;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
    SET_ETHTOOL_OPS(ndev, &ethtool_ops);
#endif
    MDev_EMAC_get_mac_address (ndev);    // Get ethernet address and store it in dev->dev_addr //
    MDev_EMAC_update_mac_address (ndev); // Program ethernet address into MAC //
    spin_lock_irq (LocPtr->lock);
    MHal_EMAC_enable_mdi (ndev);
    MHal_EMAC_read_phy (ndev, LocPtr->phy.addr, MII_USCR_REG, &val);
    if ((val & (1 << 10)) == 0)   // DSCR bit 10 is 0 -- fiber mode //
        LocPtr->phy_media = PORT_FIBRE;

    spin_unlock_irq (LocPtr->lock);

    //Support for ethtool //
    LocPtr->mii.dev = ndev;
    LocPtr->mii.mdio_read = MDev_EMAC_mdio_read;
    LocPtr->mii.mdio_write = MDev_EMAC_mdio_write;
    LocPtr->initialized = 1;
#ifdef CONFIG_MSTAR_HW_TX_CHECKSUM
    retval = MHal_EMAC_Read_JULIAN_0104(ndev) | TX_CHECKSUM_ENABLE;
    MHal_EMAC_Write_JULIAN_0104(ndev, retval);
    ndev->features |= NETIF_F_IP_CSUM;
#endif

    //Install the interrupt handler //
    //Notes: Modify linux/kernel/irq/manage.c  /* interrupt.h */
    if (request_irq(ndev->irq, MDev_EMAC_interrupt, SA_INTERRUPT, ndev->name, ndev))
        return -EBUSY;

    //Determine current link speed //
    spin_lock_irq (LocPtr->lock);
    MDev_EMAC_update_linkspeed (ndev);
    spin_unlock_irq (LocPtr->lock);

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Restar the ethernet interface
// @return TRUE : Yes
// @return FALSE : FALSE
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_SwReset(struct net_device *ndev)
{
    struct EMAC_private *LocPtr = (struct EMAC_private *) netdev_priv(ndev);
    u32 oldCFG, oldCTL;
    u32 retval;

    MDev_EMAC_get_mac_address (ndev);
    oldCFG = MHal_EMAC_Read_CFG(ndev);
    oldCTL = MHal_EMAC_Read_CTL(ndev) & ~(EMAC_TE | EMAC_RE);

    //free tx skb
    if (LocPtr->retx_count)
    {
        if (LocPtr->skb)
        {
            dev_kfree_skb_irq(LocPtr->skb );
            LocPtr->skb = NULL;
        }
        if (netif_queue_stopped (ndev))
            netif_wake_queue (ndev);
    }

    netif_stop_queue (ndev);

    retval = MHal_EMAC_Read_JULIAN_0100(ndev);
    MHal_EMAC_Write_JULIAN_0100(ndev, retval & 0x00000FFF);
    MHal_EMAC_Write_JULIAN_0100(ndev, retval);

    MDev_EMAC_HW_init(ndev);
    MHal_EMAC_Write_CFG(ndev, oldCFG);
    MHal_EMAC_Write_CTL(ndev, oldCTL);
    MHal_EMAC_enable_mdi (ndev);
    MDev_EMAC_update_mac_address (ndev); // Program ethernet address into MAC //
    MDev_EMAC_update_linkspeed (ndev);
    MHal_EMAC_Write_IER(ndev, IER_FOR_INT_JULIAN_D);
    MDev_EMAC_start (ndev);
    MDev_EMAC_set_rx_mode(ndev);
    netif_start_queue (ndev);
    LocPtr->contiROVR = 0;
    LocPtr->retx_count = 0;
#ifdef CONFIG_MSTAR_HW_TX_CHECKSUM
    retval = MHal_EMAC_Read_JULIAN_0104(ndev) | TX_CHECKSUM_ENABLE;
    MHal_EMAC_Write_JULIAN_0104(ndev, retval);
#endif
    EMAC_DBG(LocPtr->pdev->id, "=> Take %lu ms to reset EMAC!\n", (getCurMs() - LocPtr->oldTime));
    return 0;
}

//-------------------------------------------------------------------------------------------------
// Detect MAC and PHY and perform initialization
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_probe (struct net_device *dev)
{
    int detected = -1;
    /* Read the PHY ID registers - try all addresses */
    detected = MDev_EMAC_setup(dev, MII_URANUS_ID);
    return detected;
}

//-------------------------------------------------------------------------------------------------
// EMAC Timer set for Receive function
//-------------------------------------------------------------------------------------------------
static void MDev_EMAC_timer_callback(unsigned long value)
{
    int ret = 0;
	struct net_device *dev = (void *)value;
    struct EMAC_private *LocPtr = (struct EMAC_private *) netdev_priv(dev);

    spin_lock_irq (LocPtr->lock);
    ret = MDev_EMAC_update_linkspeed(dev);
    spin_unlock_irq (LocPtr->lock);
    if (0 == ret)
    {
        if (!LocPtr->CurBCE.connected)
        {
            LocPtr->CurBCE.connected = 1;
            netif_carrier_on(dev);
        }
    }
    else if ((-1 == ret) && (LocPtr->CurBCE.connected))    //no link
    {
        LocPtr->CurBCE.connected = 0;
        netif_carrier_off(dev);
    }

	LocPtr->link_timer.expires = jiffies + EMAC_CHECK_LINK_TIME;
    add_timer(&LocPtr->link_timer);
}

//-------------------------------------------------------------------------------------------------
// EMAC init module
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_ScanPhyAddr(struct net_device *ndev)
{
    unsigned char addr = 0;
    u32 value = 0;
	struct EMAC_private *LocPtr;

	LocPtr = (struct EMAC_private*) netdev_priv(ndev);
	
    if (LocPtr->phytype == PHY_INTERNAL)
    {
        MHal_EMAC_Write_JULIAN_0100(ndev, 0x0000F001);
    }
    else
    {	
        MHal_EMAC_Write_JULIAN_0100(ndev, 0x0000F007);
    }

    MHal_EMAC_enable_mdi(ndev);
    do
    {
        MHal_EMAC_read_phy(ndev, addr, MII_BMSR, &value);
        if (0 != value && 0x0000FFFF != value)
        {
            EMAC_DBG(LocPtr->pdev->id, "[ PHY Addr ] ==> :%u\n", addr);
            break;
        }
    }while(++addr && addr < 32);
    MHal_EMAC_disable_mdi(ndev);
	LocPtr->phy.addr = addr;

	if (LocPtr->phy.addr >= 32)
	{
		EMAC_DBG(LocPtr->pdev->id, "Wrong PHY Addr and reset to 0\n");
		LocPtr->phy.addr = 0;
		return -1;
	}
	return 0;
}

static void Rtl_Patch(struct net_device *ndev)
{
    u32 val;
    struct EMAC_private *LocPtr = (struct EMAC_private *) netdev_priv(ndev);

    MHal_EMAC_read_phy(ndev, LocPtr->phy.addr, 25, &val);
    MHal_EMAC_write_phy(ndev, LocPtr->phy.addr, 25, 0x400);
    MHal_EMAC_read_phy(ndev, LocPtr->phy.addr, 25, &val);
}

static void MDev_EMAC_Patch_PHY(struct net_device *ndev)
{
    u32 val;
    struct EMAC_private *LocPtr = (struct EMAC_private *) netdev_priv(ndev);

    MHal_EMAC_read_phy(ndev, LocPtr->phy.addr, 2, &val);
    if (RTL_8210 == val)
        Rtl_Patch(ndev);
}

struct net_device * MDev_EMAC_init(struct platform_device *pdev)
{
    struct EMAC_private *LocPtr;
	struct resource *platform_res = NULL;
	struct net_device *ndev;
	
    /* alloc ethernet dev */
    ndev = alloc_etherdev(sizeof(struct EMAC_private));
	LocPtr = netdev_priv(ndev);

	/*Init global data in each net device*/
	LocPtr->initialized = 0;
	LocPtr->txidx = 0;
	LocPtr->txcount = 0;
		
    if (!ndev)
    {
        EMAC_DBG(LocPtr->pdev->id, "Unable to alloc new net device\n");
        goto end;
    }

	SET_NETDEV_DEV(ndev, &pdev->dev);
	LocPtr->pdev = pdev;

    /* setup register base */
	platform_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!platform_res)
	{
		EMAC_DBG(LocPtr->pdev->id, "unable to get platform iomem resource\n");
	    goto end;
	}
	ndev->base_addr = platform_res->start;
	EMAC_DBG(LocPtr->pdev->id, "emac_dev->base_addr = %08lx\n", ndev->base_addr)

    /* setup IRQ */
	ndev->irq = platform_get_irq(pdev, 0);
	if (ndev->irq < 0)
	{
	    EMAC_DBG(LocPtr->pdev->id, "unable get IRQ number");
	    goto end;
	}
	EMAC_DBG(LocPtr->pdev->id, "emac_dev->irq = %d\n", ndev->irq);

	/* setup ephy register base */
	platform_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!platform_res)
	{
        LocPtr->phytype = PHY_EXTERNAL;
	    EMAC_DBG(LocPtr->pdev->id, "use external phy\n");
	}
	else
    {
        LocPtr->phytype = PHY_INTERNAL;
	LocPtr->reg_phy_address = platform_res->start;
        EMAC_DBG(LocPtr->pdev->id, "use internel phy\n");

    }

	/* enable clock */
    MHal_EMAC_Power_On_Clk(ndev);

    init_timer(&LocPtr->emac_timer);

    LocPtr->emac_timer.data = (unsigned long)ndev;
    LocPtr->emac_timer.function = MDev_EMAC_timer_callback;
    LocPtr->emac_timer.expires = jiffies;

    if (LocPtr->phytype == PHY_EXTERNAL)
    {
        MHal_EMAC_Write_JULIAN_0100(ndev ,JULIAN_100_VAL_EXT);
    }
	else
    {
        MHal_EMAC_Write_JULIAN_0100(ndev ,JULIAN_100_VAL);
	}


    if (0 > MDev_EMAC_ScanPhyAddr(ndev))
        goto end;

	MDev_EMAC_Patch_PHY(ndev);	
    if (!MDev_EMAC_probe (ndev))
    {
        register_netdev (ndev);
    }
	
	return ndev;

end:
    free_netdev(ndev);
    ndev = 0;
    LocPtr->initstate = ETHERNET_TEST_INIT_FAIL;
    EMAC_DBG(LocPtr->pdev->id, KERN_ERR "Init EMAC error!\n" );
    return NULL;
}
//-------------------------------------------------------------------------------------------------
// EMAC exit module
//-------------------------------------------------------------------------------------------------
static void MDev_EMAC_exit(struct net_device *ndev)
{
	struct EMAC_private *LocPtr = (struct EMAC_private*) netdev_priv(ndev);

    if (ndev)
    {
        unregister_netdev(ndev);
        free_netdev(ndev);
    }
}

static int mstar_emac_drv_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct net_device *ndev=(struct net_device*)pdev->dev.platform_data;
    struct EMAC_private *LocPtr;
    u32 uRegVal;
    printk(KERN_INFO "mstar_emac_drv_suspend\n");
    if(!ndev)
    {
        return -1;
    }

    LocPtr = (struct EMAC_private*) netdev_priv(ndev);
    LocPtr->ep_flag |= EP_FLAG_SUSPENDING;
    netif_stop_queue (ndev);

    disable_irq(ndev->irq);
    del_timer(&LocPtr->link_timer);

    //MHal_EMAC_Power_On_Clk(ndev);

    //Disable Receiver and Transmitter //
    uRegVal = MHal_EMAC_Read_CTL(ndev);
    uRegVal &= ~(EMAC_TE | EMAC_RE);
    MHal_EMAC_Write_CTL(ndev, uRegVal);

    //Disable MAC interrupts //
    MHal_EMAC_Write_IDR(ndev, IER_FOR_INT_JULIAN_D);

    //MHal_EMAC_Power_Off_Clk(ndev);
    
    return 0;
}
static int mstar_emac_drv_resume(struct platform_device *pdev)
{
    struct net_device *ndev=(struct net_device*)pdev->dev.platform_data;
    struct EMAC_private *LocPtr;
    u32 alloRAM_PA_BASE;
    u32 alloRAM_SIZE;
    u32 retval;
    printk(KERN_INFO "mstar_emac_drv_resume\n");
    if(!ndev)
    {
        return -1;
    }
    LocPtr = (struct EMAC_private*) netdev_priv(ndev);;
    LocPtr->ep_flag &= ~EP_FLAG_SUSPENDING;

    MHal_EMAC_Power_On_Clk(ndev);

    MHal_EMAC_Write_JULIAN_0100(ndev, JULIAN_100_VAL);

    if (0 > MDev_EMAC_ScanPhyAddr(ndev))
        return -1;

    MDev_EMAC_Patch_PHY(ndev);

    get_boot_mem_info(EMAC_MEM, &alloRAM_PA_BASE, &alloRAM_SIZE);
#ifndef RX_SOFTWARE_DESCRIPTOR
    //Add Write Protect
    MHal_EMAC_Write_Protect(ndev, alloRAM_PA_BASE, alloRAM_SIZE);
#endif

    LocPtr->CurUVE.initedEMAC = 0;
    MDev_EMAC_HW_init(ndev);

    MDev_EMAC_update_mac_address (ndev); // Program ethernet address into MAC //
    spin_lock_irq (LocPtr->lock);
    MHal_EMAC_enable_mdi(ndev);
    MHal_EMAC_read_phy (ndev, LocPtr->phy.addr, MII_USCR_REG, &retval);
    if ((retval & (1 << 10)) == 0)   // DSCR bit 10 is 0 -- fiber mode //
        LocPtr->phy_media = PORT_FIBRE;

    spin_unlock_irq (LocPtr->lock);

#ifdef CONFIG_MSTAR_HW_TX_CHECKSUM
    retval = MHal_EMAC_Read_JULIAN_0104(ndev) | TX_CHECKSUM_ENABLE;
    MHal_EMAC_Write_JULIAN_0104(ndev, retval);
#endif

    enable_irq(ndev->irq);
    if(LocPtr->ep_flag & EP_FLAG_OPEND)
    {
        if(0>MDev_EMAC_open(ndev))
        {
            printk(KERN_WARNING "Driver Emac: open failed after resume\n");
        }
    }
    return 0;
}

static int mstar_emac_drv_probe(struct platform_device *pdev)
{
    int retval=0;
	struct net_device *ndev;

    if( !(pdev->name) || strcmp(pdev->name,"Mstar-emac2") )
    {
        retval = -ENXIO;
    }

    ndev = MDev_EMAC_init(pdev);
    if(!retval)
    {
        pdev->dev.platform_data=ndev;
    }
	return retval;
}

static int mstar_emac_drv_remove(struct platform_device *pdev)
{
    struct net_device *ndev=(struct net_device*)pdev->dev.platform_data;
	
    if( !(pdev->name) || strcmp(pdev->name,"Mstar-emac2") )
    {
        return -1;
    }
	
    MDev_EMAC_exit(ndev);
    pdev->dev.platform_data=NULL;
    return 0;
}



static struct platform_driver Mstar_emac_driver = {
	.probe 		= mstar_emac_drv_probe,
	.remove 	= mstar_emac_drv_remove,
    .suspend    = mstar_emac_drv_suspend,
    .resume     = mstar_emac_drv_resume,

	.driver = {
		.name	= "Mstar-emac2",
        .owner  = THIS_MODULE,
	}
};

static int __init mstar_emac_drv_init_module(void)
{
    int retval=0;
	
    retval = platform_driver_register(&Mstar_emac_driver);
    return retval;
}

static void __exit mstar_emac_drv_exit_module(void)
{
    platform_driver_unregister(&Mstar_emac_driver);
}



module_init(mstar_emac_drv_init_module);
module_exit(mstar_emac_drv_exit_module);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("EMAC Ethernet driver");
MODULE_LICENSE("GPL");
