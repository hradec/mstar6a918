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
/// @file   EMAC.h
/// @author MStar Semiconductor Inc.
/// @brief  EMAC Driver Interface
///////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// Linux EMAC.h define start
// -----------------------------------------------------------------------------
#ifndef __DRV_EMAC__
#define __DRV_EMAC__

#include <linux/phy.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/phy.h>

//-------------------------------------------------------------------------------------------------
//  Define Enable or Compiler Switches
//-------------------------------------------------------------------------------------------------
//#define RX_SOFTWARE_DESCRIPTOR
#define SOFTWARE_DESCRIPTOR
#define RX_CHECKSUM
#define INT_JULIAN_D
#define CHIP_FLUSH_READ
#define ISR_BOTTOM_HALF
#define EMAC_INT_MASK (0xdff)

#ifdef RX_SOFTWARE_DESCRIPTOR
//#define RX_BYTE_ALIGN_OFFSET
#endif
// Compiler Switches
#define REG_BIT_MAP
#define URANUS_ETHER_ADDR_CONFIGURABLE	/* MAC address can be changed? */
//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define TRUE                                1
#define FALSE                               0

#define SOFTWARE_DESCRIPTOR_ENABLE          0x0001
#define CHECKSUM_ENABLE                     0x0FE
#define RX_CHECKSUM_ENABLE                  0x000E
#define CONFIG_EMAC_MOA                     1   // System Type
#define EMAC_SPEED_100                      100

#define EMAC_ALLFF                          0xFFFFFFFF
#define EMAC_ABSO_MEM_BASE                  0xA0000000//EMAC_ABSO_MEM_BASE                  0xA0000000
#define INTERNEL_PHY_REG_BASE               0xBF243000
#define EMAC_ABSO_PHY_BASE                  0x80000000//EMAC_ABSO_MEM_BASE
#define EMAC_ABSO_MEM_SIZE                  0x30000//0x16000//0x180000//0x16000//(48 * 1024)     // More than: (32 + 16(0x3FFF)) KB
#define EMAC_MEM_SIZE_SQU                   4    // More than: (32 + 16(0x3FFF)) KB
#define EMAC_BUFFER_MEM_SIZE                0x0004000
#define EMAC_MAX_TX_QUEUE                   30
// Base address here:
#define MIU0_BUS_BASE                       0x00000000
#define REG_ADDR_BASE                       0xBF204000      // The register address base. Depends on system define.
#define RBQP_LENG                           0x40//0x0100  0x40//                // ==?descriptors
#define MAX_RX_DESCR                        RBQP_LENG//32   /* max number of receive buffers */
#define SOFTWARE_DESC_LEN                   0x800
#ifdef SOFTWARE_DESCRIPTOR
#define RX_BUFFER_SEL                       0x0001          // 0x0=2KB,0x1=4KB,0x2=8KB,0x3=16KB, 0x09=1MB
#define RX_BUFFER_SIZE                      (RBQP_LENG*SOFTWARE_DESC_LEN)  //0x10000//0x20000//
#else
#define RX_BUFFER_SEL                       0x0003          // 0x0=2KB,0x1=4KB,0x2=8KB,0x3=16KB, 0x09=1MB
#define RX_BUFFER_SIZE                      (0x2000<<RX_BUFFER_SEL)//0x10000//0x20000//
#endif
#define RBQP_SIZE                           (0x0008*RBQP_LENG)      // ==8bytes*?==?descriptors

#define MAX_INT_COUNTER                     100
#define JULIAN_100_VAL                      (0x0000F0C1)
#define JULIAN_100_VAL_EXT                  (0x0000F0C7)
#define JULIAN_104_VAL                      (0x04020080)

//PHY Register
#define PHY_REG_BASIC	                    (0)
#define PHY_REG_STATUS	                    (1)
//PHY setting
#define PHY_REGADDR_OFFSET                  (18)
#define PHY_ADDR_OFFSET                     (23)

#ifdef TX_QUEUE_4
#define TX_RING_SIZE						(8)  //effected size = TX_RING_SIZE - 1
#else
#define TX_RING_SIZE						(2)  //effected size = TX_RING_SIZE - 1
#endif

/* Phy type */
#define PHY_INTERNAL                        0x00000001
#define PHY_EXTERNAL		                0x00000010


//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
struct rbf_t
{
  unsigned int  addr;
  unsigned long size;
};

struct recv_desc_bufs
{
    struct rbf_t descriptors[MAX_RX_DESCR];       /* must be on sizeof (rbf_t) boundary */
    char recv_buf[RX_BUFFER_SIZE];                /* must be on MAX_RBUFF_SZ boundary */
};

struct tx_ring
{
	u8 used;
    struct sk_buff *skb;                /* holds skb until xmit interrupt completes */
    dma_addr_t skb_physaddr;            /* phys addr from pci_map_single */
};

#define EP_FLAG_OPEND   0X00000001
#define EP_FLAG_SUSPENDING   0X00000002
#define ROUND_SUP_4(x) (((x)+3)&~3)

struct eth_drv_sgX
{
    u32 buf;
    u32 len;
};

struct _BasicConfigEMAC
{
    u8 connected;          // 0:No, 1:Yes    <== (20070515) Wait for Julian's reply
    u8 speed;               // 10:10Mbps, 100:100Mbps
    // ETH_CTL Register:
    u8 wes;             // 0:Disable, 1:Enable (WR_ENABLE_STATISTICS_REGS)
    // ETH_CFG Register:
    u8 duplex;              // 1:Half-duplex, 2:Full-duplex
    u8 cam;                // 0:No CAM, 1:Yes
    u8 rcv_bcast;       // 0:No, 1:Yes
    u8 rlf;                // 0:No, 1:Yes receive long frame(1522)
    // MAC Address:
    u8 sa1[6];              // Specific Addr 1 (MAC Address)
    u8 sa2[6];              // Specific Addr 2
    u8 sa3[6];              // Specific Addr 3
    u8 sa4[6];              // Specific Addr 4
};
typedef struct _BasicConfigEMAC BasicConfigEMAC;

struct _UtilityVarsEMAC
{
    u32 cntChkINTCounter;
    u32 readIdxRBQP;        // Reset = 0x00000000
    u32 rxOneFrameAddr;     // Reset = 0x00000000 (Store the Addr of "ReadONE_RX_Frame")
    // Statistics Counters : (accumulated)
    u32 cntREG_ETH_FRA;
    u32 cntREG_ETH_SCOL;
    u32 cntREG_ETH_MCOL;
    u32 cntREG_ETH_OK;
    u32 cntREG_ETH_SEQE;
    u32 cntREG_ETH_ALE;
    u32 cntREG_ETH_DTE;
    u32 cntREG_ETH_LCOL;
    u32 cntREG_ETH_ECOL;
    u32 cntREG_ETH_TUE;
    u32 cntREG_ETH_CSE;
    u32 cntREG_ETH_RE;
    u32 cntREG_ETH_ROVR;
    u32 cntREG_ETH_SE;
    u32 cntREG_ETH_ELR;
    u32 cntREG_ETH_RJB;
    u32 cntREG_ETH_USF;
    u32 cntREG_ETH_SQEE;
    // Interrupt Counter :
    u32 cntHRESP;           // Reset = 0x0000
    u32 cntROVR;            // Reset = 0x0000
    u32 cntLINK;            // Reset = 0x0000
    u32 cntTIDLE;           // Reset = 0x0000
    u32 cntTCOM;            // Reset = 0x0000
    u32 cntTBRE;            // Reset = 0x0000
    u32 cntRTRY;            // Reset = 0x0000
    u32 cntTUND;            // Reset = 0x0000
    u32 cntTOVR;            // Reset = 0x0000
    u32 cntRBNA;            // Reset = 0x0000
    u32 cntRCOM;            // Reset = 0x0000
    u32 cntDONE;            // Reset = 0x0000
    // Flags:
    u8 flagMacTxPermit;    // 0:No,1:Permitted.  Initialize as "permitted"
    u8 flagISR_INT_RCOM;
    u8 flagISR_INT_RBNA;
    u8 flagISR_INT_DONE;
    u8 flagPowerOn;        // 0:Poweroff, 1:Poweron
    u8 initedEMAC;         // 0:Not initialized, 1:Initialized.
    u8 flagRBNA;
    // Misc Counter:
    u32 cntRxFrames;        // Reset = 0x00000000 (Counter of RX frames,no matter it's me or not)
    u32 cntReadONE_RX;      // Counter for ReadONE_RX_Frame
    u32 cntCase20070806;
    u32 cntChkToTransmit;
    u32 cntChkCableConnect;
    // Misc Variables:
    u32 mainThreadTasks;    // (20071029_CHARLES) b0=Poweroff,b1=Poweron
};
typedef struct _UtilityVarsEMAC UtilityVarsEMAC;

typedef volatile unsigned int EMAC_REG;

struct EMAC_private
{
    struct platform_device	*pdev;
    struct net_device_stats stats;
    struct mii_if_info	 mii;			/* ethtool support */
    struct napi_struct	napi;
    struct timer_list emac_timer, link_timer;
    struct phy_device phy;
    spinlock_t *lock;					/* lock pointer for MDI interface */
	spinlock_t plock;					/* lock body for MDI interface */
   
    unsigned char *emac_bufs;			/* Tx bounce buffer region. */
    short phy_media;					/* media interface type */
    int initialized;
    int tvalue;
   
    /* Transmit */
    struct tx_ring tx_fifo[TX_RING_SIZE];
    unsigned char tx_rdidx; 			/* FIFO read index */
    unsigned char tx_wridx; 			/* FIFO write index */
    struct sk_buff *skb;				/* holds skb until xmit interrupt completes */
   
    /* hw view of rx & tx dma ring */
    dma_addr_t emac_desc_dma;
    int skb_length; 					/* saved skb length for pci_unmap_single */
    unsigned char retx_count;			/* resend count of tx */
    unsigned int txpkt; 				/* previous tx packet pointer */
   
    /* Receive */
    int rxBuffIndex;					/* index into receive descriptor list */
    struct recv_desc_bufs *dlist;		/* descriptor list address */
    struct recv_desc_bufs *dlist_phys;	/* descriptor list physical address */
   
    /* Status */
    BasicConfigEMAC CurBCE;
    UtilityVarsEMAC CurUVE;
   
    struct sk_buff *rx_skb[MAX_RX_DESCR];
    struct sk_buff *rx_skb_dummy;
    unsigned long oldTime;
    u32 rx_abso_addr_dummy;
    u32 contiROVR;
    u32 initstate;
    u32 rx_abso_addr[MAX_RX_DESCR];
	u32 reg_phy_address;

#ifdef INT_JULIAN_D
    u32 xoffsetValue;
    u32 xReceiveNum;
#endif

    /* Tx/Rx buffer base */
    u32     RAM_VA_BASE;               //= 0x00000000;     // After init, RAM_ADDR_BASE = EMAC_ABSO_MEM_BASE
    u32     RAM_PA_BASE;
    u32     RAM_VA_PA_OFFSET;
    u32     RX_BUFFER_BASE;            //= 0x00000000;     // IMPORTANT: lowest 14 bits as zero.
    u32     RBQP_BASE;                 //= RX_BUFFER_SIZE;//0x00004000;     // IMPORTANT: lowest 13 bits as zero.
    u32     TX_BUFFER_BASE;            //= (RX_BUFFER_SIZE+RBQP_SIZE);//0x00006000;
    u32     TX_SKB_BASE;               //= (RX_BUFFER_SIZE+RBQP_SIZE+0x600);//0x00006100;
    u32     TX_PTK_BASE;
	
	u32     txidx;
	u32     txcount;

    /* suspend/resume */
    unsigned long ep_flag;

	/*phy mode*/
	unsigned long phytype;
	
	struct net_device *dev;
    struct work_struct task;
    unsigned int xReceiveFlag;
};


//-------------------------------------------------------------------------------------------------
//  Bit Define
//-------------------------------------------------------------------------------------------------
#define EMAC_MIU_RW       (0x3 <<  10)   //EMAC power on clk

// -------- EMAC_CTL : (EMAC Offset: 0x0)  --------
#define EMAC_LB           ( 0x1 <<  0) // (EMAC) Loopback. Optional. When set, loopback signal is at high level.
#define EMAC_LBL          ( 0x1 <<  1) // (EMAC) Loopback local.
#define EMAC_RE           ( 0x1 <<  2) // (EMAC) Receive enable.
#define EMAC_TE           ( 0x1 <<  3) // (EMAC) Transmit enable.
#define EMAC_MPE          ( 0x1 <<  4) // (EMAC) Management port enable.
#define EMAC_CSR          ( 0x1 <<  5) // (EMAC) Clear statistics registers.
#define EMAC_ISR          ( 0x1 <<  6) // (EMAC) Increment statistics registers.
#define EMAC_WES          ( 0x1 <<  7) // (EMAC) Write enable for statistics registers.
#define EMAC_BP           ( 0x1 <<  8) // (EMAC) Back pressure.
// -------- EMAC_CFG : (EMAC Offset: 0x4) Network Configuration Register --------
#define EMAC_SPD          ( 0x1 <<  0) // (EMAC) Speed.
#define EMAC_FD           ( 0x1 <<  1) // (EMAC) Full duplex.
#define EMAC_BR           ( 0x1 <<  2) // (EMAC) Bit rate.
#define EMAC_CAF          ( 0x1 <<  4) // (EMAC) Copy all frames.
#define EMAC_NBC          ( 0x1 <<  5) // (EMAC) No broadcast.
#define EMAC_MTI          ( 0x1 <<  6) // (EMAC) Multicast hash enable
#define EMAC_UNI          ( 0x1 <<  7) // (EMAC) Unicast hash enable.
#define EMAC_RLF          ( 0x1 <<  8) // (EMAC) Receive Long Frame.
#define EMAC_EAE          ( 0x1 <<  9) // (EMAC) External address match enable.
#define EMAC_CLK          ( 0x3 << 10) // (EMAC)
#define EMAC_CLK_HCLK_8   ( 0x0 << 10) // (EMAC) HCLK divided by 8
#define EMAC_CLK_HCLK_16  ( 0x1 << 10) // (EMAC) HCLK divided by 16
#define EMAC_CLK_HCLK_32  ( 0x2 << 10) // (EMAC) HCLK divided by 32
#define EMAC_CLK_HCLK_64  ( 0x3 << 10) // (EMAC) HCLK divided by 64
#define EMAC_RT           ( 0x1 << 12) // (EMAC) Retry test
#define EMAC_CAMMEG       ( 0x1 << 13) // (EMAC)
// -------- EMAC_SR : (EMAC Offset: 0x8) Network Status Register --------
#define EMAC_MDIO         ( 0x1 <<  1) // (EMAC)
#define EMAC_IDLE         ( 0x1 <<  2) // (EMAC)
// -------- EMAC_TCR : (EMAC Offset: 0x10) Transmit Control Register --------
#define EMAC_LEN          ( 0x7FF <<  0) // (EMAC)
#define EMAC_NCRC         ( 0x1 << 15) // (EMAC)
// -------- EMAC_TSR : (EMAC Offset: 0x14) Transmit Control Register --------
#define EMAC_OVR          ( 0x1 <<  0) // (EMAC)
#define EMAC_COL          ( 0x1 <<  1) // (EMAC)
#define EMAC_RLE          ( 0x1 <<  2) // (EMAC)
//#define EMAC_TXIDLE     ( 0x1 <<  3) // (EMAC)
#define EMAC_IDLETSR      ( 0x1 <<  3) // (EMAC)
#define EMAC_BNQ          ( 0x1 <<  4) // (EMAC)
#define EMAC_COMP         ( 0x1 <<  5) // (EMAC)
#define EMAC_UND          ( 0x1 <<  6) // (EMAC)
// -------- EMAC_RSR : (EMAC Offset: 0x20) Receive Status Register --------
#define EMAC_DNA          ( 0x1 <<  0) // (EMAC)
#define EMAC_REC          ( 0x1 <<  1) // (EMAC)
#define EMAC_RSROVR       ( 0x1 <<  2) // (EMAC)
#define EMAC_BNA          ( 0x1 <<  3) // (EMAC)
// -------- EMAC_ISR : (EMAC Offset: 0x24) Interrupt Status Register --------
#define EMAC_INT_DONE     ( 0x1 <<  0) // (EMAC)
#define EMAC_INT_RCOM     ( 0x1 <<  1) // (EMAC)
#define EMAC_INT_RBNA     ( 0x1 <<  2) // (EMAC)
#define EMAC_INT_TOVR     ( 0x1 <<  3) // (EMAC)
#define EMAC_INT_TUND     ( 0x1 <<  4) // (EMAC)
#define EMAC_INT_RTRY     ( 0x1 <<  5) // (EMAC)
#define EMAC_INT_TBRE     ( 0x1 <<  6) // (EMAC)
#define EMAC_INT_TCOM     ( 0x1 <<  7) // (EMAC)
#define EMAC_INT_TIDLE    ( 0x1 <<  8) // (EMAC)
#define EMAC_INT_LINK     ( 0x1 <<  9) // (EMAC)
#define EMAC_INT_ROVR     ( 0x1 << 10) // (EMAC)
#define EMAC_INT_HRESP    ( 0x1 << 11) // (EMAC)
// -------- EMAC_IER : (EMAC Offset: 0x28) Interrupt Enable Register --------
// -------- EMAC_IDR : (EMAC Offset: 0x2c) Interrupt Disable Register --------
// -------- EMAC_IMR : (EMAC Offset: 0x30) Interrupt Mask Register --------
// -------- EMAC_MAN : (EMAC Offset: 0x34) PHY Maintenance Register --------
#define EMAC_DATA         ( 0xFFFF <<  0) // (EMAC)
#define EMAC_CODE         ( 0x3 << 16) // (EMAC)
#define EMAC_CODE_802_3   ( 0x2 << 16) // (EMAC) Write Operation
#define EMAC_REGA         ( 0x1F << 18) // (EMAC)
#define EMAC_PHYA         ( 0x1F << 23) // (EMAC)
#define EMAC_RW           ( 0x3 << 28) // (EMAC)
#define EMAC_RW_R         ( 0x2 << 28) // (EMAC) Read Operation
#define EMAC_RW_W         ( 0x1 << 28) // (EMAC) Write Operation
#define EMAC_HIGH         ( 0x1 << 30) // (EMAC)
#define EMAC_LOW          ( 0x1 << 31) // (EMAC)
// -------- EMAC_RBRP: (EMAC Offset: 0x38) Receive Buffer First full pointer--------
#define EMAC_WRAP_R       ( 0x1 << 14) // Wrap bit
// -------- EMAC_RBWP: (EMAC Offset: 0x3C) Receive Buffer Current pointer--------
#define EMAC_WRAP_W       ( 0x1 << 14) // Wrap bit
// ........................................................................ //
//URANUS PHY //
#define MII_URANUS_ID                   0x01111//Test value
//URANUS specific registers //
#define MII_USCR_REG	                16
#define MII_USCSR_REG                   17
#define MII_USINTR_REG                  21
/* ........................................................................ */
#define EMAC_DESC_DONE                  0x00000001  /* bit for if DMA is done */
#define EMAC_DESC_WRAP                  0x00000002  /* bit for wrap */
#define EMAC_BROADCAST                  0x80000000  /* broadcast address */
#define EMAC_MULTICAST                  0x40000000  /* multicast address */
#define EMAC_UNICAST                    0x20000000  /* unicast address */

#define EMAC_DESC_IPV4                  (0x1 << 11)
#define EMAC_DESC_UDP                   (0x1 << 18)
#define EMAC_DESC_TCP                   (0x1 << 19)
#define EMAC_DESC_IP_CSUM               (0x1 << 20)
#define EMAC_DESC_TCP_UDP_CSUM          (0x1 << 21)
// Constant: ----------------------------------------------------------------
// Register MAP:
#define REG_ETH_CTL                     0x00000000         // Network control register
#define REG_ETH_CFG                     0x00000004         // Network configuration register
#define REG_ETH_SR                      0x00000008         // Network status register
#define REG_ETH_TAR                     0x0000000C         // Transmit address register
#define REG_ETH_TCR                     0x00000010         // Transmit control register
#define REG_ETH_TSR                     0x00000014         // Transmit status register
#define REG_ETH_RBQP                    0x00000018         // Receive buffer queue pointer
#define REG_ETH_BUFF                    0x0000001C         // Receive Buffer Configuration
#define REG_ETH_RSR                     0x00000020         // Receive status register
#define REG_ETH_ISR                     0x00000024         // Interrupt status register
#define REG_ETH_IER                     0x00000028         // Interrupt enable register
#define REG_ETH_IDR                     0x0000002C         // Interrupt disable register
#define REG_ETH_IMR                     0x00000030         // Interrupt mask register
#define REG_ETH_MAN                     0x00000034         // PHY maintenance register
#define REG_ETH_BUFFRDPTR               0x00000038         // Receive First Full Pointer
#define REG_ETH_BUFFWRPTR               0x0000003C         // Receive Current pointer
#define REG_ETH_FRA                     0x00000040         // Frames transmitted OK
#define REG_ETH_SCOL                    0x00000044         // Single collision frames
#define REG_ETH_MCOL                    0x00000048         // Multiple collision frames
#define REG_ETH_OK                      0x0000004C         // Frames received OK
#define REG_ETH_SEQE                    0x00000050         // Frame check sequence errors
#define REG_ETH_ALE                     0x00000054         // Alignment errors
#define REG_ETH_DTE                     0x00000058         // Deferred transmission frames
#define REG_ETH_LCOL                    0x0000005C         // Late collisions
#define REG_ETH_ECOL                    0x00000060         // Excessive collisions
#define REG_ETH_TUE                     0x00000064         // Transmit under-run errors
#define REG_ETH_CSE                     0x00000068         // Carrier sense errors
#define REG_ETH_RE                      0x0000006C         // Receive resource error
#define REG_ETH_ROVR                    0x00000070         // Received overrun
#define REG_ETH_SE                      0x00000074         // Received symbols error
#define REG_ETH_ELR                     0x00000078         // Excessive length errors
#define REG_ETH_RJB                     0x0000007C         // Receive jabbers
#define REG_ETH_USF                     0x00000080         // Undersize frames
#define REG_ETH_SQEE                    0x00000084         // SQE test errors
#define REG_ETH_HSL                     0x00000090         // Hash register [31:0]
#define REG_ETH_HSH                     0x00000094         // Hash register [63:32]
#define REG_ETH_SA1L                    0x00000098         // Specific address 1 first 4 bytes
#define REG_ETH_SA1H                    0x0000009C         // Specific address 1 last  2 bytes
#define REG_ETH_SA2L                    0x000000A0         // Specific address 2 first 4 bytes
#define REG_ETH_SA2H                    0x000000A4         // Specific address 2 last  2 bytes
#define REG_ETH_SA3L                    0x000000A8         // Specific address 3 first 4 bytes
#define REG_ETH_SA3H                    0x000000AC         // Specific address 3 last  2 bytes
#define REG_ETH_SA4L                    0x000000B0         // Specific address 4 first 4 bytes
#define REG_ETH_SA4H                    0x000000B4         // Specific address 4 last  2 bytes
#define REG_TAG_TYPE                    0x000000B8         // tag type of the frame
#define REG_CAMA0_l                     0x00000200         // 16 LSB of CAM address  0
#define REG_CAMA0_h                     0x00000204         // 32 MSB of CAM address  0
#define REG_CAMA62_l                    0x000003F0         // 16 LSB of CAM address 62
#define REG_CAMA62_h                    0x000003F4         // 32 MSB of CAM address 62

#define REG_EMAC_JULIAN_0100            0x00000100
#define REG_EMAC_JULIAN_0104            0x00000104
#define REG_EMAC_JULIAN_0108            0x00000108

u32 MHal_EMAC_ReadReg32(struct net_device *ndev, u32 xoffset );
u32 MHal_EMAC_ReadRam32( u32 uRamAddr, u32 xoffset);
u32 MHal_EMAC_NegotiationPHY( struct net_device *ndev, int phyaddr );
u32 MHal_EMAC_CableConnection( struct net_device *ndev );
u32 MHal_EMAC_get_SA1H_addr( struct net_device *ndev );
u32 MHal_EMAC_get_SA1L_addr( struct net_device *ndev );
u32 MHal_EMAC_get_SA2H_addr( struct net_device *ndev );
u32 MHal_EMAC_get_SA2L_addr( struct net_device *ndev );
u32 MHal_EMAC_Read_CTL(struct net_device *ndev);
u32 MHal_EMAC_Read_CFG(struct net_device *ndev);
u32 MHal_EMAC_Read_ISR(struct net_device *ndev );
u32 MHal_EMAC_Read_IER(struct net_device *ndev);
u32 MHal_EMAC_Read_IMR(struct net_device *ndev);
u32 MHal_EMAC_Read_BUFF(struct net_device *ndev);
u32 MHal_EMAC_Read_MAN(struct net_device *ndev);
u32 MHal_EMAC_Read_FRA(struct net_device *ndev);
u32 MHal_EMAC_Read_SCOL(struct net_device *ndev);
u32 MHal_EMAC_Read_MCOL(struct net_device *ndev);
u32 MHal_EMAC_Read_OK(struct net_device *ndev);
u32 MHal_EMAC_Read_SEQE(struct net_device *ndev);
u32 MHal_EMAC_Read_ALE(struct net_device *ndev);
u32 MHal_EMAC_Read_LCOL(struct net_device *ndev);
u32 MHal_EMAC_Read_ECOL(struct net_device *ndev);
u32 MHal_EMAC_Read_TUE(struct net_device *ndev);
u32 MHal_EMAC_Read_TSR(struct net_device *ndev);
u32 MHal_EMAC_Read_RSR( struct net_device *ndev );
u32 MHal_EMAC_Read_CSE(struct net_device *ndev);
u32 MHal_EMAC_Read_ROVR(struct net_device *ndev);
u32 MHal_EMAC_Read_SE(struct net_device *ndev);
u32 MHal_EMAC_Read_ELR(struct net_device *ndev);
u32 MHal_EMAC_Read_RJB(struct net_device *ndev);
u32 MHal_EMAC_Read_USF(struct net_device *ndev);
u32 MHal_EMAC_Read_SQEE(struct net_device *ndev);
u32 MHal_EMAC_Read_JULIAN_0100( struct net_device *ndev );
u32 MHal_EMAC_Read_JULIAN_0104( struct net_device *ndev );
u32 MHal_EMAC_Read_JULIAN_0108( struct net_device *ndev );

void MHal_EMAC_WritReg32(struct net_device *ndev, u32 xoffset, u32 xval );
void MHal_EMAC_update_HSH(struct net_device *ndev, u32 mc0, u32 mc1);
void MHal_EMAC_Write_CTL(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_CFG(struct net_device *ndev ,u32 xval);
void MHal_EMAC_Write_RBQP(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_BUFF(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_MAN(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_TAR(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_TCR(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_TSR( struct net_device *ndev, u32 xval );
void MHal_EMAC_Write_RSR( struct net_device *ndev, u32 xval );
void MHal_EMAC_Write_RDPTR(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_WRPTR(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_ISR(struct net_device *ndev, u32 xval );
void MHal_EMAC_Write_IER(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_IDR(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_SA1H(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_SA1L(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_SA2H(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_SA2L(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_JULIAN_0100(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_JULIAN_0104(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_JULIAN_0108(struct net_device *ndev, u32 xval);
void MHal_EMAC_Set_Tx_JULIAN_T(struct net_device *ndev, u32 xval);
u32 MHal_EMAC_Get_Tx_FIFO_Threshold(struct net_device *ndev);
void MHal_EMAC_Set_Rx_FIFO_Enlarge(struct net_device *ndev, u32 xval);
u32 MHal_EMAC_Get_Rx_FIFO_Enlarge(struct net_device *ndev);
void MHal_EMAC_Set_Miu_Priority(struct net_device *ndev, u32 xval);
u32 MHal_EMAC_Get_Miu_Priority(struct net_device *ndev);
void MHal_EMAC_Set_Tx_Hang_Fix_ECO(struct net_device *ndev, u32 xval);
void MHal_EMAC_Set_MIU_Out_Of_Range_Fix(struct net_device *ndev, u32 xval);
void MHal_EMAC_Set_Rx_Tx_Burst16_Mode(struct net_device *ndev, u32 xval);
void MHal_EMAC_Set_Tx_Rx_Req_Priority_Switch(struct net_device *ndev, u32 xval);
void MHal_EMAC_Set_Rx_Byte_Align_Offset(struct net_device *ndev, u32 xval);
void MHal_EMAC_Write_Protect(struct net_device *ndev, u32 start_addr, u32 length);
void MHal_EMAC_HW_init(struct net_device *ndev);
void MHal_EMAC_Power_On_Clk( struct net_device *dev);
void MHal_EMAC_Power_Off_Clk(struct net_device *ndev);
void MHal_EMAC_timer_callback(struct net_device *ndev, unsigned long value);
void MHal_EMAC_WritRam32(u32 uRamAddr, u32 xoffset,u32 xval);
void MHal_EMAC_enable_mdi(struct net_device *ndev);
void MHal_EMAC_disable_mdi(struct net_device *ndev);
void MHal_EMAC_write_phy(struct net_device *ndev, unsigned char phy_addr, unsigned char address, u32 value);
void MHal_EMAC_read_phy(struct net_device *ndev, unsigned char phy_addr, unsigned char address,u32 *value);
void MHal_EMAC_update_speed_duplex(struct net_device *ndev, u32 uspeed, u32 uduplex);
void MHal_EMAC_Write_SA1_MAC_Address(struct net_device *ndev, u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5);
void MHal_EMAC_Write_SA2_MAC_Address(struct net_device *ndev, u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5);
void MHal_EMAC_Write_SA3_MAC_Address(struct net_device *ndev, u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5);
void MHal_EMAC_Write_SA4_MAC_Address(struct net_device *ndev, u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5);
#endif
// -----------------------------------------------------------------------------
// Linux EMAC.c End
// -----------------------------------------------------------------------------


