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
/// @file   drvMSPI.h
/// @brief  Master SPI Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_MSPI_H_
#define _DRV_MSPI_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "MsTypes.h"
//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define DEBUG_SER_FLASH(debug_level, x)     do { if (_u8SPIDbgLevel >= (debug_level)) (x); } while(0)

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_SERFLASH_DBGLV_NONE,    //disable all the debug message
    E_SERFLASH_DBGLV_INFO,    //information
    E_SERFLASH_DBGLV_NOTICE,  //normal but significant condition
    E_SERFLASH_DBGLV_WARNING, //warning conditions
    E_SERFLASH_DBGLV_ERR,     //error conditions
    E_SERFLASH_DBGLV_CRIT,    //critical conditions
    E_SERFLASH_DBGLV_ALERT,   //action must be taken immediately
    E_SERFLASH_DBGLV_EMERG,   //system is unusable
    E_SERFLASH_DBGLV_DEBUG    //debug-level messages
} SERFLASH_DbgLv;

typedef enum _SPI_DrvClkDiv
{
     E_SPI_DIV2
    ,E_SPI_DIV4
    ,E_SPI_DIV8
    ,E_SPI_DIV16
    ,E_SPI_DIV32
    ,E_SPI_DIV64
    ,E_SPI_DIV128
    ,E_SPI_ClkDiv_NOT_SUPPORT
}SPI_DrvClkDiv;

typedef enum _SPI_DrvCKG
{
    E_SPI_XTALI = 0,
    E_SPI_27M,
    E_SPI_36M,
    E_SPI_43M,
    E_SPI_54M,
    E_SPI_72M,
    E_SPI_86M,
    E_SPI_108M,
    E_SPI_24M = 15, // T3 only
    E_SPI_HALCKG_NOT_SUPPORT
}SPI_DrvCKG;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SPI_Read(MS_U32 u32Size, MS_U8 *pu8buffer);
MS_BOOL MDrv_SPI_Write(MS_U32 u32Size, MS_U8 *pu8buffer);
void MDrv_SPI_PolaritySet(MS_U8 u8Polarity);
void MDrv_SPI_ChipSelect(MS_BOOL Enable);
void MDrv_SPI_SetMaxClk(MS_U32 u32MaxClk);
void MDrv_SPI_Disable(void);
void MDrv_SPI_Enable(void);

#ifdef __cplusplus
}
#endif

#endif // _DRV_MSPI_H_

