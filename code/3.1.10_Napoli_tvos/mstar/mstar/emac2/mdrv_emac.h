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

#ifndef __DRV_EMAC_H_
#define __DRV_EMAC_H_

#define EMAC_DBG(ndevid ,fmt, args...)     {printk("Mstar_emac%d: ", ndevid); printk(fmt, ##args);}

#define EMAC_INFO                           {printk("Line:%u\n", __LINE__);}

//-------------------------------------------------------------------------------------------------
//  Define Enable or Compiler Switches
//-------------------------------------------------------------------------------------------------
#define USE_TASK                            1            // 1:Yes, 0:No
#define EMAC_MTU                            (1524)

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define ETHERNET_TEST_NO_LINK               0x00000000
#define ETHERNET_TEST_AUTO_NEGOTIATION      0x00000001
#define ETHERNET_TEST_LINK_SUCCESS          0x00000002
#define ETHERNET_TEST_RESET_STATE           0x00000003
#define ETHERNET_TEST_SPEED_100M            0x00000004
#define ETHERNET_TEST_DUPLEX_FULL           0x00000008
#define ETHERNET_TEST_INIT_FAIL             0x00000010


u8 MY_DEV[16]   = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
u8 MY_MAC[2][6] = { { 0x00, 0x30, 0x1B, 0xBA, 0x02, 0xDB },
	                { 0x00, 0x30, 0x1B, 0xBA, 0x02, 0xDC } };
u8 PC_MAC[6]    = { 0x00, 0x1A, 0x4B, 0x5C, 0x39, 0xDF };

struct sk_buff *rx_skb[MAX_RX_DESCR];
u32 rx_abso_addr[MAX_RX_DESCR];
struct sk_buff * rx_skb_dummy;
u32 	rx_abso_addr_dummy;

// -----------------------------------------------------------------------------
// Linux EMAC.h End
// -----------------------------------------------------------------------------
#endif

