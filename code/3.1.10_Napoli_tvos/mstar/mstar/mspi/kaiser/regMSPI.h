//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all 
// or part of MStar Software is expressly prohibited, unless prior written 
// permission has been granted by MStar. 
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.  
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software. 
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s 
//    confidential information in strictest confidence and not disclose to any
//    third party.  
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.  
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or 
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.  
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    regMSPI.h
/// @brief  Master SPI Register Definition
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _REG_MSPI_H_
#define _REG_MSPI_H_

//-------------------------------------------------------------------------------------------------
// Include File
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
// BASEADDR & BK
#define BASEADDR_RIU                   0xFD000000
#define BASEADDR_NONPM_RIU             0xFD200000
// BASEADDR & BK
#define BK_MSP                         0x900*2
#define BK_CLK0                        0xE00*2

#define MSPI_REG_BASE                  0x900
#define MSPI_WRITE_BUF_OFFSET          0x40
#define MSPI_READ_BUF_OFFSET           0x44
#define MSPI_WBF_SIZE_OFFSET           0x48
#define MSPI_RBF_SIZE_OFFSET           0x48
    // read/ write buffer size
    #define MSPI_RWSIZE_MASK               0xFF
    #define MSPI_RSIZE_BIT_OFFSET          0x8
    #define MAX_READ_BUF_SIZE              0x7
    #define MAX_WRITE_BUF_SIZE             0x7
// CLK config 
#define MSPI_CTRL_OFFSET               0x49
#define MSPI_CLK_CLOCK_OFFSET          0x49
    #define MSPI_CLK_CLOCK_BIT_OFFSET      0x08
    #define MSPI_CLK_CLOCK_MASK            0xFF
    #define MSPI_CLK_PHASE_MASK            0x40
    #define MSPI_CLK_PHASE_BIT_OFFSET      0x06
    #define MSPI_CLK_POLARITY_MASK         0x80
	#define MSPI_CLK_POLARITY_BIT_OFFSET   0x07
    #define MSPI_CLK_PHASE_MAX             0x1
    #define MSPI_CLK_POLARITY_MAX          0x1
    #define MSPI_CLK_CLOCK_MAX             0x7
// DC config 
#define MSPI_DC_MASK                   0xFF
#define MSPI_DC_BIT_OFFSET             0x08
#define MSPI_DC_TR_START_OFFSET        0x4A
    #define MSPI_DC_TRSTART_MAX            0xFF
#define MSPI_DC_TR_END_OFFSET          0x4A
    #define MSPI_DC_TREND_MAX              0xFF   
#define MSPI_DC_TB_OFFSET              0x4B
    #define MSPI_DC_TB_MAX                 0xFF
#define MSPI_DC_TRW_OFFSET             0x4B
    #define MSPI_DC_TRW_MAX                0xFF
// Frame Config 
#define MSPI_FRAME_WBIT_OFFSET         0x4C
#define MSPI_FRAME_RBIT_OFFSET         0x4E
    #define MSPI_FRAME_BIT_MAX             0x07
    #define MSPI_FRAME_BIT_MASK            0x07
    #define MSPI_FRAME_BIT_FIELD           0x03
#define MSPI_LSB_FIRST_OFFSET          0x50
#define MSPI_TRIGGER_OFFSET            0x5A
#define MSPI_DONE_OFFSET               0x5B
#define MSPI_DONE_CLEAR_OFFSET         0x5C
#define MSPI_CHIP_SELECT_OFFSET        0x5F
//chip select bit map
    #define MSPI_CHIP_SELECT_MAX           0x07    
// control bit
#define MSPI_DONE_FLAG                 0x01
#define MSPI_TRIGGER                   0x01
#define MSPI_CLEAR_DONE                0x01
#define MSPI_INT_ENABLE                0x04
#define MSPI_RESET                     0x02
#define MSPI_ENABLE                    0x01

// Clock setting 

#define MSPI_SRC_CLK_OFFSET            0x25
    #define MSPI_SRC_CLK_MASK          0x0F
	#define MSPI_SRC_CLK_SHIFT         0x02
    #define MSPI_SRC_CLK_DEFAULT       0x04
#endif // _REG_MSPI_H_

