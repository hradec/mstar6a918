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
/// file    drvMSPI.c
/// @brief  Master SPI Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/string.h>
#include <linux/kernel.h>
// Common Definition

//#include "MsCommon.h"
//#include "MsVersion.h"
#include "drvMSPI.h"
//#include "MsOS.h"

// Internal Definition
#include "regMSPI.h"
#include "halMSPI.h"
//#include "drvMMIO.h"

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// Local & Global Variables
////////////////////////////////////////////////////////////////////////////////
MS_U8 gu8MSPIConfig;
MS_U8 gu8MSPICurConfig[MSPI_CMD_TYPE];
MS_BOOL gbInitFlag = FALSE;
MS_U8 _u8MSPIDbgLevel;

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/// Description : Set Chip Select
/// @return E_MSPI_OK : 
/// @return >1 : failed to set Chip select
//------------------------------------------------------------------------------ 
static MSPI_ErrorNo _MDrv_ChipSelectSetting(MS_U8 u8Cs)
{
    MS_U8 u8CSMax;
    MSPI_ErrorNo errnum = E_MSPI_OK;

    u8CSMax = HAL_MSPI_ChipSelectMax();
    if(u8Cs > u8CSMax)
        errnum = E_MSPI_PARAM_OVERFLOW;
	else
        HAL_MSPI_SetChipSelect(u8Cs);
	return errnum;
}

//------------------------------------------------------------------------------
/// Description : Set TrStart timing of DC config
/// @return E_MSPI_OK : 
/// @return >1 : failed to set TrStart timing
//------------------------------------------------------------------------------ 
static MSPI_ErrorNo _MDrv_DC_TrStartSetting(MS_U8 TrStart)
{
    MS_U8 u8TrStartMax;
	MSPI_ErrorNo errnum = E_MSPI_OK;

    u8TrStartMax = HAL_MSPI_DCConfigMax(E_MSPI_TRSTART);
    if(TrStart > u8TrStartMax)
        errnum = E_MSPI_PARAM_OVERFLOW;
	else
        HAL_MSPI_SetDcTiming(E_MSPI_TRSTART ,TrStart);
    return errnum;
}

//------------------------------------------------------------------------------
/// Description : Set TrEnd timing of DC config
/// @return E_MSPI_OK : 
/// @return >1 : failed to set TrEnd timing
//------------------------------------------------------------------------------ 
static MSPI_ErrorNo _MDrv_DC_TrEndSetting(MS_U8 TrEnd)
{
    MS_U8 u8TrEndMax;
	MSPI_ErrorNo errnum = E_MSPI_OK;

    u8TrEndMax = HAL_MSPI_DCConfigMax(E_MSPI_TREND);
    if(TrEnd > u8TrEndMax)
        errnum = E_MSPI_PARAM_OVERFLOW;
    else
        HAL_MSPI_SetDcTiming(E_MSPI_TREND ,TrEnd);
    return errnum;

}

//------------------------------------------------------------------------------
/// Description : Set TB timing of DC config 
/// @return E_MSPI_OK :
/// @return >1 : failed to set TB timing
//------------------------------------------------------------------------------ 
static MSPI_ErrorNo _MDrv_DC_TBSetting(MS_U8 TB)
{
    MS_U8 u8TBMax;
	MSPI_ErrorNo errnum = E_MSPI_OK;

    u8TBMax = HAL_MSPI_DCConfigMax(E_MSPI_TB);
    if(TB > u8TBMax)
        errnum = E_MSPI_PARAM_OVERFLOW;
    else
        HAL_MSPI_SetDcTiming(E_MSPI_TB ,TB);
    return errnum;

}

//------------------------------------------------------------------------------
/// Description : Set TRW timing of DC config
/// @return E_MSPI_OK : 
/// @return >1 : failed to set TRW timging
//------------------------------------------------------------------------------ 
static MSPI_ErrorNo _MDrv_DC_TRWSetting(MS_U8 TRW)
{
    MS_U8 u8TRWMax;
	MSPI_ErrorNo errnum = E_MSPI_OK;

    u8TRWMax = HAL_MSPI_DCConfigMax(E_MSPI_TRW);
    if(TRW > u8TRWMax)
        errnum = E_MSPI_PARAM_OVERFLOW;
    else
        HAL_MSPI_SetDcTiming(E_MSPI_TRW ,TRW);
    return errnum;

}

//------------------------------------------------------------------------------
/// Description : Set clock polarity of MSPI
/// @return E_MSPI_OK : 
/// @return >1 : failed to set clock polarity
//------------------------------------------------------------------------------ 
static MSPI_ErrorNo _MDrv_CLK_PolaritySetting(MS_U8 u8Pol)
{
    MS_U8 u8PolarityMax;
	MSPI_ErrorNo errnum = E_MSPI_OK;

    u8PolarityMax = HAL_MSPI_CLKConfigMax(E_MSPI_POL);
    if(u8Pol > u8PolarityMax)
        errnum = E_MSPI_PARAM_OVERFLOW;
    else
        HAL_MSPI_SetCLKTiming(E_MSPI_POL ,u8Pol);
    return errnum;

}

//------------------------------------------------------------------------------
/// Description : Set clock phase of MSPI
/// @return E_MSPI_OK : 
/// @return >1 : failed to set clock phase
//------------------------------------------------------------------------------ 
static MSPI_ErrorNo _MDrv_CLK_PhaseSetting(MS_U8 u8Pha)
{
    MS_U8 u8PhaseMax;
	MSPI_ErrorNo errnum = E_MSPI_OK;

    u8PhaseMax = HAL_MSPI_CLKConfigMax(E_MSPI_PHA);
    if(u8Pha > u8PhaseMax)
        errnum = E_MSPI_PARAM_OVERFLOW;
    else
        HAL_MSPI_SetCLKTiming(E_MSPI_PHA ,u8Pha);
    return errnum;

}

//------------------------------------------------------------------------------
/// Description : Set clock rate of MSPI
/// @return E_MSPI_OK : 
/// @return >1 : failed to set clock rate
//------------------------------------------------------------------------------ 
static MSPI_ErrorNo _MDrv_CLK_ClockSetting(MS_U8 u8Clock)
{
    MS_U8 u8ClockMax;
	MSPI_ErrorNo errnum = E_MSPI_OK;

    u8ClockMax = HAL_MSPI_CLKConfigMax(E_MSPI_CLK);
    if(u8Clock > u8ClockMax)
        errnum = E_MSPI_PARAM_OVERFLOW;
    else
        HAL_MSPI_SetCLKTiming(E_MSPI_CLK ,u8Clock);
    return errnum;

}

//------------------------------------------------------------------------------
/// Description : Set transfer bit pre frame for read/write buffer
/// @return E_MSPI_OK : 
/// @return >1 : failed to check paramter 
//------------------------------------------------------------------------------ 
static MSPI_ErrorNo _MDrv_Frame_BitSetting(MS_BOOL bDirect, MS_U8 u8Index, MS_U8 u8BitPerFrame)
{
    MS_U8 u8MAxBitPerFrame;
	MSPI_ErrorNo errnum = E_MSPI_OK;

    u8MAxBitPerFrame = HAL_MSPI_FrameConfigMax();
    if(u8BitPerFrame > u8MAxBitPerFrame)
        errnum = E_MSPI_PARAM_OVERFLOW;
    else	
        HAL_MSPI_SetPerFrameSize(bDirect,  u8Index, u8BitPerFrame);
    return errnum;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Description : Set detailed level of MSPI driver debug message
/// @param u8DbgLevel    \b IN  debug level for Serial Flash driver
/// @return TRUE : succeed
/// @return FALSE : failed to set the debug level
//------------------------------------------------------------------------------
MS_BOOL MDrv_MSPI_SetDbgLevel(MS_U8 u8DbgLevel)
{
    _u8MSPIDbgLevel = u8DbgLevel;

    return TRUE;
}
//------------------------------------------------------------------------------
/// Description : MSPI initial Ext
/// @return E_MSPI_OK : 
/// @return >1 : failed to initial 
//------------------------------------------------------------------------------ 
MSPI_ErrorNo MDrv_MSPI_Init_Ext(MS_U8 u8HWNum)
{
    //MS_U32 u32NONPMBank, u32NONPMBankSize;
    //MS_U32 u32PMBank, u32PMBankSize;
	printk(KERN_EMERG"MDrv_MSPI_Init_Ext \r\n");

    MSPI_ErrorNo errorno = E_MSPI_OK;

    _u8MSPIDbgLevel = E_MSPI_DBGLV_NONE;
    if(!MDrv_MSPI_HW_Support())
    {
        gbInitFlag = FALSE;
        return E_MSPI_HW_NOT_SUPPORT;
    }

    //HAL_MSPI_MMIOConfig(u32PMBank, u32NONPMBank, u8HWNum);

    memset(gu8MSPICurConfig, 0, sizeof(gu8MSPICurConfig));
    // default use CS0 for slave device 
    errorno = _MDrv_ChipSelectSetting(0);
    //default use polling mode 
    HAL_MSPI_IntEnable(0);
    HAL_MSPI_Init();
    DEBUG_MSPI(E_MSPI_DBGLV_INFO, printk(" MSPI Init complete\n"));
    gbInitFlag = TRUE;
    //default clock setting
    errorno = _MDrv_CLK_PolaritySetting(1);
    DEBUG_MSPI(E_MSPI_DBGLV_INFO, printk("PolaritySetting complete\n"));
    if(errorno != E_MSPI_OK)
        goto ERROR_HANDLE;
    errorno = _MDrv_CLK_PhaseSetting(0);
    if(errorno != E_MSPI_OK)
        goto ERROR_HANDLE;
    errorno = _MDrv_CLK_ClockSetting(0);
    if(errorno != E_MSPI_OK)
        goto ERROR_HANDLE;
    return E_MSPI_OK;
ERROR_HANDLE:
    errorno |= E_MSPI_INIT_FLOW_ERROR;
	DEBUG_MSPI(E_MSPI_DBGLV_ERR, printk("MSPI CLKconfig error errno =%d\n",errorno));
    return errorno;
}

//------------------------------------------------------------------------------
/// Description : MSPI initial
/// @return E_MSPI_OK : 
/// @return >1 : failed to initial 
//------------------------------------------------------------------------------ 
MSPI_ErrorNo MDrv_MSPI_Init(MSPI_config *tMSPIConfig, MS_U8 u8HWNum)
{
//    MS_U32 u32NONPMBank, u32NONPMBankSize;
//    MS_U32 u32PMBank, u32PMBankSize;
    MSPI_ErrorNo errorno = E_MSPI_OK;
    if(!MDrv_MSPI_HW_Support())
    {
        gbInitFlag = FALSE;
        return E_MSPI_HW_NOT_SUPPORT;
    }
    _u8MSPIDbgLevel = E_MSPI_DBGLV_NONE;
#if 0
    if (!MDrv_MMIO_GetBASE( &u32PMBank, &u32PMBankSize, MS_MODULE_PM))
    {
        DEBUG_MSPI(E_MSPI_DBGLV_ERR, printk("IOMap failure to get DRV_MMIO_NONPM_BANK\n"));
    }

    if (!MDrv_MMIO_GetBASE( &u32NONPMBank, &u32NONPMBankSize, MS_MODULE_HW))
    {
        DEBUG_MSPI(E_MSPI_DBGLV_ERR, printk("IOMap failure to get DRV_MMIO_PM_BANK\n"));
        errorno = E_MSPI_MMIO_ERROR;
    }
#endif
    if(u8HWNum > 2)
        return E_MSPI_PARAM_OVERFLOW;
    //HAL_MSPI_MMIOConfig(u32PMBank, u32NONPMBank, u8HWNum);

    if(tMSPIConfig->BIntEnable)
    {
        //register IRQ handler
    }
	memcpy(gu8MSPICurConfig, tMSPIConfig->U8BitMapofConfig, sizeof(gu8MSPICurConfig));

    errorno = _MDrv_ChipSelectSetting(tMSPIConfig->U8ChipSel);
    if(errorno != E_MSPI_OK)
        return errorno;
    HAL_MSPI_IntEnable(tMSPIConfig->BIntEnable);
    HAL_MSPI_Init();
    gbInitFlag = TRUE;
    return errorno;
}

//-------------------------------------------------------------------------------------------------
/// Description : read data from MSPI
/// @param pData \b IN :pointer to receive data from MSPI read buffer 
/// @param u16Size \ b OTU : read data size
/// @return the errorno of operation
//-------------------------------------------------------------------------------------------------
MSPI_ErrorNo MDrv_MSPI_Read(MS_U8 *pData, MS_U16 u16Size)
{
    MS_U8 u8FlowCheck = 0;
	MSPI_ErrorNo errorno = E_MSPI_OK;

    //check init
    if(!gbInitFlag)
        return E_MSPI_INIT_FLOW_ERROR;

    // check config error
    if((gu8MSPICurConfig[MSPI_READ_OPERATION] != gu8MSPIConfig))
    {
        u8FlowCheck = gu8MSPICurConfig[MSPI_READ_OPERATION] ^ gu8MSPIConfig;
        if(u8FlowCheck & MSPI_DC_CONFIG)
            errorno |= E_MSPI_DCCONFIG_ERROR; 
        if(u8FlowCheck & MSPI_CLK_CONFIG)
            errorno |= E_MSPI_CLKCONFIG_ERROR;
        if(u8FlowCheck & MSPI_FRAME_CONFIG)
            errorno |= E_MSPI_FRAMECONFIG_ERROR;
        // reset config
        DEBUG_MSPI(E_MSPI_DBGLV_ERR, printk("Read Operation MSPI config error %d\n",errorno));
        return errorno;
    }

    HAL_MSPI_Read(pData, u16Size);
    return E_MSPI_OK;
}

//------------------------------------------------------------------------------
/// Description : read data from MSPI
/// @param pData \b OUT :pointer to write  data to MSPI write buffer 
/// @param u16Size \ b OTU : write data size
/// @return the errorno of operation
//------------------------------------------------------------------------------
MSPI_ErrorNo MDrv_MSPI_Write(MS_U8 *pData, MS_U16 u16Size)
{
    MS_U8 u8FlowCheck = 0;
	MSPI_ErrorNo errorno = E_MSPI_OK;

    //check init
    if(!gbInitFlag)
        return E_MSPI_INIT_FLOW_ERROR;

    // check config error
    if((gu8MSPICurConfig[MSPI_WRITE_OPERATION] != gu8MSPIConfig))
    {
        u8FlowCheck = gu8MSPICurConfig[MSPI_WRITE_OPERATION] ^ gu8MSPIConfig;
        if(u8FlowCheck & MSPI_DC_CONFIG)
            errorno |= E_MSPI_DCCONFIG_ERROR; 
        if(u8FlowCheck & MSPI_CLK_CONFIG)
            errorno |= E_MSPI_CLKCONFIG_ERROR;
        if(u8FlowCheck & MSPI_FRAME_CONFIG)
            errorno |= E_MSPI_FRAMECONFIG_ERROR;
        // reset config
        DEBUG_MSPI(E_MSPI_DBGLV_ERR, printk("Write Operation MSPI config error %d\n",errorno));
        return errorno;
    }

	// write operation
    HAL_MSPI_Wirte(pData, u16Size);
    return errorno;
}

//------------------------------------------------------------------------------
/// Description : config spi transfer timing
/// @param ptDCConfig    \b OUT  struct pointer of transfer timing config
/// @return E_MSPI_OK : succeed
/// @return E_MSPI_DCCONFIG_ERROR : failed to config transfer timing
//------------------------------------------------------------------------------
MSPI_ErrorNo MDrv_MSPI_DCConfig(MSPI_DCConfig *ptDCConfig)
{
    MSPI_ErrorNo errnum = E_MSPI_OK;
    //check init
    if(!gbInitFlag)
        return E_MSPI_INIT_FLOW_ERROR;

    if(ptDCConfig == NULL)
    {
        HAL_MSPI_Reset_DCConfig();
        return E_MSPI_OK;
    }
    errnum = _MDrv_DC_TrStartSetting(ptDCConfig->u8TrStart);
    if(errnum != E_MSPI_OK)
        goto ERROR_HANDLE;
    errnum = _MDrv_DC_TrEndSetting(ptDCConfig->u8TrEnd);
    if(errnum != E_MSPI_OK)
        goto ERROR_HANDLE;
    errnum = _MDrv_DC_TBSetting(ptDCConfig->u8TB);
    if(errnum != E_MSPI_OK)
        goto ERROR_HANDLE;
    errnum = _MDrv_DC_TRWSetting(ptDCConfig->u8TRW);
    if(errnum != E_MSPI_OK)
        goto ERROR_HANDLE;
    gu8MSPIConfig |= MSPI_DC_CONFIG;
    return E_MSPI_OK;

ERROR_HANDLE:
    errnum |= E_MSPI_DCCONFIG_ERROR;
	DEBUG_MSPI(E_MSPI_DBGLV_ERR, printk("MSPI DCconfig error errno =%d\n",errnum));
    return errnum;	
}

//------------------------------------------------------------------------------
/// Description : config spi clock setting
/// @param ptCLKConfig    \b OUT  struct pointer of clock config
/// @return E_MSPI_OK : succeed
/// @return E_MSPI_CLKCONFIG_ERROR : failed to config spi clock
//------------------------------------------------------------------------------
MSPI_ErrorNo MDrv_MSPI_CLKConfig(MSPI_CLKConfig *ptCLKConfig)
{
    MSPI_ErrorNo errnum = E_MSPI_OK;
    //check init
    if(!gbInitFlag)
        return E_MSPI_INIT_FLOW_ERROR;

    if(ptCLKConfig == NULL)
    {
        HAL_MSPI_Reset_CLKConfig();
        return E_MSPI_OK;
    }

    errnum = _MDrv_CLK_PolaritySetting(ptCLKConfig->BClkPolarity);
    if(errnum != E_MSPI_OK)
        goto ERROR_HANDLE;
    errnum = _MDrv_CLK_PhaseSetting(ptCLKConfig->BClkPhase);
    if(errnum != E_MSPI_OK)
        goto ERROR_HANDLE;
    errnum = _MDrv_CLK_ClockSetting(ptCLKConfig->U8Clock);
    if(errnum != E_MSPI_OK)
        goto ERROR_HANDLE;
    gu8MSPIConfig |= MSPI_CLK_CONFIG;
    return E_MSPI_OK;

ERROR_HANDLE:
    errnum |= E_MSPI_CLKCONFIG_ERROR;
	DEBUG_MSPI(E_MSPI_DBGLV_ERR, printk("MSPI CLKconfig error errno =%d\n",errnum));
    return errnum;

}


//------------------------------------------------------------------------------
/// Description : config spi transfer timing
/// @param ptDCConfig    \b OUT  struct pointer of bits of buffer tranferred to slave config
/// @return E_MSPI_OK : succeed
/// @return E_MSPI_FRAMECONFIG_ERROR : failed to config transfered bit per buffer
//------------------------------------------------------------------------------
MSPI_ErrorNo MDrv_MSPI_FRAMEConfig(MSPI_FrameConfig *ptFrameConfig)
{
    MSPI_ErrorNo errnum = E_MSPI_OK;
    MS_U8 u8Index = 0;
	MS_U8 u8BufSize;

    //check init
    if(!gbInitFlag)
        return E_MSPI_INIT_FLOW_ERROR;

    if(ptFrameConfig == NULL)
    {
        HAL_MSPI_Reset_FrameConfig();
        return E_MSPI_OK;
    }
    // read buffer bit config 
	u8BufSize = sizeof(ptFrameConfig->u8RBitConfig);
    for(u8Index = 0;u8Index < u8BufSize; u8Index++)
    {
        errnum = _MDrv_Frame_BitSetting(MSPI_READ_INDEX, u8Index, ptFrameConfig->u8RBitConfig[u8Index]);
        if(errnum != E_MSPI_OK)
        {
            errnum |= E_MSPI_FRAMECONFIG_ERROR;
             DEBUG_MSPI(E_MSPI_DBGLV_ERR, printk("MSPI FRAMEconfig error errno =%d\n",errnum));
            return errnum;
        }
    }
    //write buffer bit config
    u8BufSize = sizeof(ptFrameConfig->u8WBitConfig);
    for(u8Index = 0;u8Index < u8BufSize; u8Index++)
    {
        errnum = _MDrv_Frame_BitSetting(MSPI_WRITE_INDEX, u8Index, ptFrameConfig->u8WBitConfig[u8Index]);
        if(errnum != E_MSPI_OK)
        {
            errnum |= E_MSPI_FRAMECONFIG_ERROR;
            DEBUG_MSPI(E_MSPI_DBGLV_ERR, printk("MSPI FRAMEconfig error errno =%d\n",errnum));
            return errnum;
        }
    }

    gu8MSPIConfig |= MSPI_FRAME_CONFIG;
    return errnum;
}

//------------------------------------------------------------------------------
/// Description : Enable Slave device  
//------------------------------------------------------------------------------
void MDrv_MSPI_SlaveEnable(MS_BOOL Enable)
{
    HAL_MSPI_SlaveEnable(Enable);
}

//------------------------------------------------------------------------------
/// Description : 
/// @return TRUE : chip support 
/// @return FALSE: 
//------------------------------------------------------------------------------
MS_BOOL MDrv_MSPI_HW_Support(void)
{
    return HAL_MSPI_HW_Support();
}

void MDrv_MSPI_Disable(void)
{
    HAL_MSPI_Disable();
}

MS_U32 MDrv_MSPI_Get_Clk(void)
{
    return HAL_MSPI_Get_Clk();
}

void MDrv_MSPI_Set_Clk(MS_U32 u32MaxClk)
{
    HAL_MSPI_Set_Clk(u32MaxClk);
}
