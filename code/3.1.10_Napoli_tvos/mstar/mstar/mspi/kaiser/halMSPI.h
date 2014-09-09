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
// Copyright (c) 2009-2012 MStar Semiconductor, Inc.
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
#ifndef _HAL_MSPI_H_
#define _HAL_MSPI_H_

#include "drvMSPI.h"
//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define MSPI_READ_INDEX 			   0x0
#define MSPI_WRITE_INDEX			   0x1
/* check if chip support MSPI*/
#define HAL_MSPI_HW_Support()          TRUE
#define DEBUG_MSPI(debug_level, x)     do { if (_u8MSPIDbgLevel >= (debug_level)) (x); } while(0)

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

typedef enum _HAL_DC_Config
{
    E_MSPI_TRSTART,
    E_MSPI_TREND,
    E_MSPI_TB,
    E_MSPI_TRW
}eDC_config;

typedef enum _HAL_CLK_Config
{
    E_MSPI_POL,
    E_MSPI_PHA,
    E_MSPI_CLK
}eCLK_config;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
extern MS_U8 _u8MSPIDbgLevel;

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Description : MSPI initial
/// @return void:
//------------------------------------------------------------------------------ 
void HAL_MSPI_Init(void);

//------------------------------------------------------------------------------
/// Description : MSPI	interrupt enable
/// @param bEnable \b OUT: enable or disable mspi interrupt
/// @return void:
//------------------------------------------------------------------------------ 
void HAL_MSPI_IntEnable(MS_BOOL bEnable);

//------------------------------------------------------------------------------
/// Description : Set MSPI chip select
/// @param u8CS \u8 OUT: MSPI chip select
/// @return void:
//------------------------------------------------------------------------------ 
void HAL_MSPI_SetChipSelect(MS_U8 u8CS);

//------------------------------------------------------------------------------
/// Description : Config MSP MMIO base address
/// @param u32PMBankBaseAddr \b IN :base address of MMIO (PM domain)
/// @param u32NONPMRegBaseAddr \b IN :base address of MMIO
/// @param u8DeviceIndex \b IN: index of HW IP
//------------------------------------------------------------------------------ 
void HAL_MSPI_MMIOConfig(MS_U32 u32PMBankBaseAddr, MS_U32 u32NONPMRegBaseAddr, MS_U8 u8DeviceIndex);

//-------------------------------------------------------------------------------------------------
/// Description : read data from MSPI
/// @param pData \b IN :pointer to receive data from MSPI read buffer 
/// @param u16Size \ b OTU : read data size
/// @return TRUE  : read data success
/// @return FALSE : read data fail
//-------------------------------------------------------------------------------------------------
MS_BOOL HAL_MSPI_Read(MS_U8 *pData, MS_U16 u16Size);

//------------------------------------------------------------------------------
/// Description : read data from MSPI
/// @param pData \b OUT :pointer to write  data to MSPI write buffer 
/// @param u16Size \ b OTU : write data size
/// @return TRUE  : write data success
/// @return FALSE : wirte data fail
//------------------------------------------------------------------------------
MS_BOOL HAL_MSPI_Wirte(MS_U8 *pData, MS_U16 u16Size);

//------------------------------------------------------------------------------
/// Description : config spi transfer timing
/// @param eDCField    \enum OUT  type of DC timing
/// @param u8DCtiming \u8 OUT timing of eDCField
//------------------------------------------------------------------------------
void HAL_MSPI_SetDcTiming (eDC_config eDCField, MS_U8 u8DCtiming);

//------------------------------------------------------------------------------
/// Description : config spi clock setting
/// @param eCLKField    \enum OUT  type of Clock setting
/// @param u8DCtiming \u8 OUT setting of eCLKField
//------------------------------------------------------------------------------
void HAL_MSPI_SetCLKTiming(eCLK_config eCLKField, MS_U8 u8CLKVal);

//------------------------------------------------------------------------------
/// Description : config spi per-buffer size
/// @param bDirect              \b direction of operation(read/write)
/// @param u8BufOffset       \u8 offset of per-buffer size register 
/// @param u8PerFrameSize \u8 setting of per-buffer size
//------------------------------------------------------------------------------
void HAL_MSPI_SetPerFrameSize(MS_BOOL bDirect, MS_U8 u8BufOffset, MS_U8 u8PerFrameSize);

//------------------------------------------------------------------------------
/// Description : Reset  DC register setting of MSPI
/// @param NONE
/// @return TRUE  : reset complete
//------------------------------------------------------------------------------
MS_BOOL HAL_MSPI_Reset_DCConfig(void);

//------------------------------------------------------------------------------
/// Description : Reset  Frame register setting of MSPI
/// @param NONE
/// @return TRUE  : reset complete
//------------------------------------------------------------------------------
MS_BOOL HAL_MSPI_Reset_FrameConfig(void);

//------------------------------------------------------------------------------
/// Description : Reset  CLK register setting of MSPI
/// @param NONE
/// @return TRUE  : reset complete
//------------------------------------------------------------------------------
MS_BOOL HAL_MSPI_Reset_CLKConfig(void);

//------------------------------------------------------------------------------
/// Description : get read/write buffer size
/// @param bDirection    \b OUT   specify to get read/write buffer size
/// @return buffer sizel
//------------------------------------------------------------------------------
MS_U8 HAL_MSPI_GetBufSize(MS_BOOL bDirection);

//------------------------------------------------------------------------------
/// Description : Trigger MSPI operation
/// @return TRUE  : operation success
/// @return FALSE : operation timeout
//------------------------------------------------------------------------------
MS_BOOL HAL_MSPI_Trigger(void);


void HAL_MSPI_SlaveEnable(MS_BOOL Enable);

MS_U8 HAL_MSPI_DCConfigMax(eDC_config eDCField);

MS_U8 HAL_MSPI_CLKConfigMax(eCLK_config eCLKField);

MS_U8 HAL_MSPI_FrameConfigMax(void);

MS_U8 HAL_MSPI_ChipSelectMax(void);

void HAL_MSPI_Disable(void);

MS_U32 HAL_MSPI_Get_Clk(void);

void HAL_MSPI_Set_Clk(MS_U32 u32MaxClk);

#endif

