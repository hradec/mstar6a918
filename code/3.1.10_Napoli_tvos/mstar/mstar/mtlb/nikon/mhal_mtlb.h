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
/// @file   Mhal_mtlb.h
/// @author MStar Semiconductor Inc.
/// @brief  MTLB Driver Interface
///////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// Linux Mhal_mtlb.h define start
// -----------------------------------------------------------------------------
#ifndef __DRV_MTLB__
#define __DRV_MTLB__

#include <linux/pci.h>

//-------------------------------------------------------------------------------------------------
//  Define Enable or Compiler Switches
//-------------------------------------------------------------------------------------------------
#define MTLB_DBG(fmt, args...)              {printk("Mstar_mtlb: "); printk(fmt, ##args);}

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define TRUE                                1
#define FALSE                               0

#define MTLB_RIU_REG_BASE                   0xBF000000
#define MTLB_RIU_BANK                       0x1615
#define L2_RIU_BANK                         0x1018

/*8 bits address*/
#define REG_TLB_W_ID0                       0x40
#define REG_TLB_CTL                         0x44
#define REG_TLB_DEB_ADDR                    0x46
#define REG_TLB_DEB_OUT                     0x48
#define REG_TLB_L_BOUND                     0x4A
#define REG_TLB_H_BOUND                     0x4C
#define REG_TLB_W_HEADER                    0x4E
#define REG_TLB_W_ID0                       0x40
#define REG_TLB_W_CTL                       0x50
#define REG_W_PACK                          0x98

#define TLB_W_HEADER                        0x0700       // (MTLB) TLB write header.
#define TLB_W_ENABLE                        ( 0x1 << 7 ) // (MTLB) TLB write enable.
#define TLB_ID0_ENABLE                      ( 0x1 << 7 ) // (MTLB) TLB ID0 enable.
#define TLB_GE_ID                           0x21         // (MTLB) GE client ID
#define TLB_MIPS_ID                         0x04         // (MTLB) MIPS client ID
#define TLB_MIU0_NO_PACK                    ( 0x1 )      // (MTLB) Miu0 write always no pack.
#define TLB_DEBUG_ENABLE                    0x0070       // (MTLB) TLB enable sram dump. 
#define TLB_DEBUG_DISABLE                   ~( 0x0070 )  // (MTLB) TLB disable sram dump. 

void MHal_MTLB_Init( void );
bool MHal_MTLB_Mapping(u8 u8miu, u32 u32va, u32 u32pa, u32 u32size);
bool MHal_MTLB_Dump(u8 u8miu, u32 *u32va, u32 *u32pa);

#endif
// -----------------------------------------------------------------------------
// Linux Mhal_mtlb.h End
// -----------------------------------------------------------------------------


