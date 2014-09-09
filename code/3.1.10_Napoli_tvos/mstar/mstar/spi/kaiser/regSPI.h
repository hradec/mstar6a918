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
/// file    regSPI.h
/// @brief  SPI Register Definition
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _REG_SPI_H_
#define _REG_SPI_H_

//-------------------------------------------------------------------------------------------------
// Include File
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
#define BMASK(_bits_)               (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))

// BASEADDR & BK
#define BASEADDR_RIU                   0xFD000000
#define BASEADDR_NONPM_RIU             0xFD200000
// BASEADDR & BK
#define BK_SPI                         0x800*2
#define BK_CLK0                        0x900*2
#define BK_PM                          0xE00*2

//PM
// PM_SLEEP CMD.
#define REG_PM_CKG_SPI              0x20 // Ref spec. before using these setting.
    #define PM_SPI_CLK_SEL_MASK         BMASK(13:10)
    #define PM_SPI_CLK_XTALI                BITS(13:10, 0)
    #define PM_SPI_CLK_27MHZ                BITS(13:10, 1)
    #define PM_SPI_CLK_36MHZ                BITS(13:10, 2)
    #define PM_SPI_CLK_43MHZ                BITS(13:10, 3)
    #define PM_SPI_CLK_54MHZ                BITS(13:10, 4)
    #define PM_SPI_CLK_72MHZ                BITS(13:10, 5)
    #define PM_SPI_CLK_86MHZ                BITS(13:10, 6)
    #define PM_SPI_CLK_108MHZ               BITS(13:10, 7)
    #define PM_SPI_CLK_24MHZ                BITS(13:10, 15)
    #define PM_SPI_CLK_SWITCH_MASK      BMASK(14:14)
    #define PM_SPI_CLK_SWITCH_OFF           BITS(14:14, 0)
    #define PM_SPI_CLK_SWITCH_ON            BITS(14:14, 1)
// For Power Consumption	
#define REG_PM_EXTRA_GPIO_OEN		0x1E
	#define PM_EXTRA_GPIO_MASK			BMASK(7:4)
	#define PM_EXTRA_GPIO_IN				BITS(7:4, 0xF)
	#define PM_EXTRA_GPIO_OUT				BITS(7:4, 0x0)
#define REG_PM_SPI_IS_GPIO			0x35
	#define PM_SPI_GPIO_MASK			BMASK(2:0)
	#define PM_SPI_IS_GPIO					BITS(2:0, 0x7)
	#define PM_SPI_NOT_GPIO					BITS(2:0, 0x0)

// CLK_GEN0
#define REG_CLK0_CKG_SPI            0x20
    #define CLK0_CKG_SPI_MASK           BMASK(4:2)
    #define CLK0_CKG_SPI_XTALI              BITS(4:2, 0)
    #define CLK0_CKG_SPI_27MHZ              BITS(4:2, 1)
    #define CLK0_CKG_SPI_36MHZ              BITS(4:2, 2)
    #define CLK0_CKG_SPI_43MHZ              BITS(4:2, 3)
    #define CLK0_CKG_SPI_54MHZ              BITS(4:2, 4)
    #define CLK0_CKG_SPI_72MHZ              BITS(4:2, 5)
    #define CLK0_CKG_SPI_86MHZ              BITS(4:2, 6)
    #define CLK0_CKG_SPI_108MHZ             BITS(4:2, 7)
    #define CLK0_CLK_SWITCH_MASK        BMASK(5:5)
    #define CLK0_CLK_SWITCH_OFF           BITS(5:5, 0)
    #define CLK0_CLK_SWITCH_ON            BITS(5:5, 1)

// SPI RIU
#define SPI_REG_PWD                    0x00
#define SPI_REG_WDATA                  0x04
#define SPI_REG_RDATA                  0x05
#define SPI_REG_CLKDIV                 0x06
    #define SPI_CLKDIV2 	           BIT(0)
    #define SPI_CLKDIV4                BIT(2)
    #define SPI_CLKDIV8                BIT(6)
    #define SPI_CLKDIV16               BIT(7)
    #define SPI_CLKDIV32               BIT(8)
    #define SPI_CLKDIV64               BIT(9)
    #define SPI_CLKDIV128              BIT(10)

#define SPI_REG_CECLR                  0x08
#define SPI_REG_RDREQ                  0x0C
    #define ISP_SPI_RDREQ              0x01

#define SPI_REG_RD_DATA_RDY            0x15
    #define SPI_RD_DATARDY_MASK        0x01
    #define SPI_RD_DATARDY             0x01
#define SPI_REG_WR_DATA_RDY            0x16
    #define SPI_WR_DATARDY_MASK        0x01
#define SPI_WR_DATARDY                 0x01
#define SPI_REG_WR_CMD_RDY             0x17
    #define SPI_WR_CMDRDY_MASK         0x01
    #define SPI_WR_CMDRDY              0x01

#define SPI_REG_TRG_MODE               0x2A
#define SPI_REG_POLARITY               0x37

#endif // _REG_SPI_H_

