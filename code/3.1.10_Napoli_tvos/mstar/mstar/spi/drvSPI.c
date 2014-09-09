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
/// file    drvSPI.c
/// @brief  Master SPI Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/string.h>
#include <linux/kernel.h>
// Common Definition

#include "drvSPI.h"
//#include "MsOS.h"

// Internal Definition
#include "regSPI.h"
#include "halSPI.h"


//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// Local & Global Variables
////////////////////////////////////////////////////////////////////////////////
MS_U8 _u8SPIDbgLevel;

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SPI_Read(MS_U32 u32Size, MS_U8 *pu8buffer)
{
     printk("MDrv_SPI_Read %d \r\n",u32Size);
     HAL_SPI_Read(u32Size, pu8buffer);
}

MS_BOOL MDrv_SPI_Write(MS_U32 u32Size, MS_U8 *pu8buffer)
{
     printk("MDrv_SPI_Write %d %x %x \r\n",u32Size, pu8buffer[0], pu8buffer[1]);
     HAL_SPI_Write(u32Size, pu8buffer);
}

void MDrv_SPI_PolaritySet(MS_U8 u8Polarity)
{
    HAL_SPI_PolaritySet(u8Polarity);
}

void MDrv_SPI_ChipSelect(MS_BOOL Enable)
{
    HAL_SPI_ChipSelect(Enable);
}

void MDrv_SPI_SetMaxClk(MS_U32 u32MaxClk)
{
	HAL_SPI_SetMaxClk(u32MaxClk);
}

void MDrv_SPI_Disable(void)
{
    HAL_ISP_Disable();
}

void MDrv_SPI_Enable(void)
{
    HAL_ISP_Enable();
}

