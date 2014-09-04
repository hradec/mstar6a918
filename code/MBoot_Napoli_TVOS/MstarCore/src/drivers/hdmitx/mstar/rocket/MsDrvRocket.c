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
// Copyright (c) 2006-2009 MStar Semiconductor, Inc.
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
/// file    mhal_lth.cpp
/// @author MStar Semiconductor Inc.
/// @brief  HDMITx HDCP HAL
///////////////////////////////////////////////////////////////////////////////////////////////////
#define _MHAL_LTH_C_

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

#include <common.h>
#include <command.h>
#include "MsCommon.h"
#include <MsDebug.h>
#include <apilth.h>
#include <apiSWI2C.h>
#include <apiHDMITx.h>

#define ROCKET_I2C_WR_SLAVE_ID 0x00A2

static E_LTH_HDMITX_OUTPUT_TIMING_TYPE getRocket_output_timing(HDMITX_VIDEO_TIMING outputTiming)
{
    E_LTH_HDMITX_OUTPUT_TIMING_TYPE rocketTiming=E_LTH_HDMITX_1080_60P;
    
    switch(outputTiming)
    {
        case HDMITX_RES_720x480i:
            rocketTiming=E_LTH_HDMITX_480_60I;
            break;
        case HDMITX_RES_720x576i:
            rocketTiming=E_LTH_HDMITX_576_50I;
            break;
        case HDMITX_RES_720x480p:
            rocketTiming=E_LTH_HDMITX_480_60P;
            break;
        case HDMITX_RES_720x576p:
            rocketTiming=E_LTH_HDMITX_576_50P;
            break;
        case HDMITX_RES_1280x720p_50Hz:
            rocketTiming=E_LTH_HDMITX_720_50P;
            break;
        case HDMITX_RES_1280x720p_60Hz:
            rocketTiming=E_LTH_HDMITX_720_60P;
            break;
        case HDMITX_RES_1920x1080i_50Hz:
            rocketTiming=E_LTH_HDMITX_1080_50I;
            break;
        case HDMITX_RES_1920x1080i_60Hz:
            rocketTiming=E_LTH_HDMITX_1080_60I;
            break;
        case HDMITX_RES_1920x1080p_24Hz:
            rocketTiming=E_LTH_HDMITX_1080_24P;
            break;
        case HDMITX_RES_1920x1080p_25Hz:
            rocketTiming=E_LTH_HDMITX_1080_25P;
            break;
        case HDMITX_RES_1920x1080p_30Hz:
            rocketTiming=E_LTH_HDMITX_1080_30P;
            break;
        case HDMITX_RES_1920x1080p_50Hz:
            rocketTiming=E_LTH_HDMITX_1080_50P;
            break;
        case HDMITX_RES_1920x1080p_60Hz:
            rocketTiming=E_LTH_HDMITX_1080_60P;
            break;
        case HDMITX_RES_4K2Kp_30Hz:
            rocketTiming=E_LTH_HDMITX_4K2K_30P;
            break;
        case HDMITX_RES_1280x1470p_50Hz:
            rocketTiming=E_LTH_HDMITX_1470_50P;
            break;
        case HDMITX_RES_1280x1470p_60Hz:
            rocketTiming=E_LTH_HDMITX_1470_60P;
            break;
        case HDMITX_RES_1280x1470p_24Hz:
            rocketTiming=E_LTH_HDMITX_1470_24P;
            break;
        case HDMITX_RES_1280x1470p_30Hz:
            rocketTiming=E_LTH_HDMITX_1470_30P;
            break;
        case HDMITX_RES_1920x2205p_24Hz:
            rocketTiming=E_LTH_HDMITX_2250_24P;
            break;
        case HDMITX_RES_1920x2205p_30Hz:
            rocketTiming=E_LTH_HDMITX_2250_30P;
            break;
        case HDMITX_RES_MAX:
            rocketTiming=E_LTH_HDMITX_TIMING_MAX;
            break;
    
        default:        
            UBOOT_ERROR("getRocket_output_timing: Type Error !!!\n");        
            break;
    }
    
    UBOOT_DEBUG(" SWITCH HDMI TIMING TABLE [%d]--> ROCKET[%d] \n",outputTiming,rocketTiming);
    return rocketTiming;

}

MS_BOOL Rocket_IIC_WriteBytes(MS_U16 u16SlaveCfg, MS_U32 uAddrCnt, MS_U8 *pRegAddr, MS_U32 uSize, MS_U8 *pData)
{
    MS_BOOL bResult = TRUE;
    if(MApi_SWI2C_WriteBytes(u16SlaveCfg ,uAddrCnt, pRegAddr, uSize, pData) == FALSE)
    {
        UBOOT_ERROR("IIC Write fail\n");
        bResult = MApi_SWI2C_WriteBytes(ROCKET_I2C_WR_SLAVE_ID ,0, NULL, 4, pData);
    }
    return bResult;

}

MS_BOOL Rocket_IIC_ReadBytes(MS_U16 u16SlaveCfg, MS_U32 uAddrCnt, MS_U8 *pRegAddr, MS_U32 uSize, MS_U8 *pData)
{
    MS_BOOL bResult = TRUE;
    if(MApi_SWI2C_ReadBytes(u16SlaveCfg ,uAddrCnt, pRegAddr, uSize, pData) == FALSE)
    {
        UBOOT_ERROR("IIC Read fail 2\n");
        bResult = (MApi_SWI2C_WriteBytes(u16SlaveCfg ,uAddrCnt, pRegAddr, uSize, pData));
    }

    return bResult;
}

MS_BOOL msHdmitx_Rocket_Init(HDMITX_VIDEO_TIMING outPutType )
{
    UBOOT_TRACE("IN\n");
    LTH_IIC_CB_CONFIG stIIC_Cfg;
   
    stIIC_Cfg.IIC_WriteBytes = Rocket_IIC_WriteBytes;
    stIIC_Cfg.IIC_ReadBytes = Rocket_IIC_ReadBytes; 
    if(MApi_Lth_Init_IIC_CB(stIIC_Cfg) == FALSE)     
    {                                                
        UBOOT_ERROR("Init IIC Fail \n");      
        return FALSE;                                
    }
   
    if(MApi_Lth_Init() == FALSE)                                                          
    {                                   
        UBOOT_ERROR("MApi_Lth_Init Fail \n");
        return FALSE;                   
    }      
      
    MApi_Lth_Set_Dump_All_Table_Setting(TRUE);
      
    MApi_Lth_Load_OuputTimingSetting(getRocket_output_timing(outPutType), E_LTH_HDMITX_CD_8BITS);   
    MApi_Lth_Set_Audio(E_LTH_HDMITX_AUDIO_I2S);

    UBOOT_TRACE("OUT\n");
    return TRUE;
}
