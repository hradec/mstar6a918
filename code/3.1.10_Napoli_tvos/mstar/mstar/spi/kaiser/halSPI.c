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
/// file    halMPI.c
/// @brief  RIU SPI Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/spinlock.h>
#include "MsTypes.h"
#include "halSPI.h"
#include "regSPI.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define READ_BYTE(_reg)             (*(volatile MS_U8*)(_reg))
#define READ_WORD(_reg)             (*(volatile MS_U16*)(_reg))
#define READ_LONG(_reg)             (*(volatile MS_U32*)(_reg))
#define WRITE_BYTE(_reg, _val)      { (*((volatile MS_U8*)(_reg))) = (MS_U8)(_val); }
#define WRITE_WORD(_reg, _val)      { (*((volatile MS_U16*)(_reg))) = (MS_U16)(_val); }
#define WRITE_LONG(_reg, _val)      { (*((volatile MS_U32*)(_reg))) = (MS_U32)(_val); }
#define WRITE_WORD_MASK(_reg, _val, _mask)  { (*((volatile MS_U16*)(_reg))) = ((*((volatile MS_U16*)(_reg))) & ~(_mask)) | ((MS_U16)(_val) & (_mask)); }

#define SPI_READ(addr)                      READ_WORD(_hal_msp.u32SpiBaseAddr + ((addr)<<2))
#define SPI_WRITE(addr, val)                WRITE_WORD(_hal_msp.u32SpiBaseAddr + ((addr)<<2), (val))
#define CLK_READ(addr)                      READ_WORD(_hal_msp.u32Clk0BaseAddr + ((addr)<<2))
#define CLK_WRITE(addr, val)                WRITE_WORD(_hal_msp.u32Clk0BaseAddr + ((addr)<<2), (val))
#define CLK_WRITE_MASK(addr, val, mask)    WRITE_WORD_MASK(_hal_msp.u32Clk0BaseAddr + ((addr)<<2), (val), (mask))

// PM_SLEEP CMD.
#define PM_READ(addr)                      READ_WORD(_hal_msp.u32PMBaseAddr+ ((addr)<<2))
#define PM_WRITE(addr, val)                WRITE_WORD(_hal_msp.u32PMBaseAddr+ ((addr)<<2), (val))
#define PM_WRITE_MASK(addr, val, mask)     WRITE_WORD_MASK(_hal_msp.u32PMBaseAddr+ ((addr)<<2), (val), (mask))

#define MAX_CHECK_CNT                       2000
#define DEBUG_HAL_SPI(debug_level, x)     do { if (_u8HalMSPIDbgLevel >= (debug_level)) (x); } while(0)

#define SERFLASH_SAFETY_FACTOR      3000
#define SER_FLASH_TIME(_stamp)                 (do_gettimeofday(&_stamp))
#define SER_FLASH_EXPIRE(_stamp,_msec)         (_Hal_GetMsTime(_stamp, _msec))


//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef struct
{
    MS_U32 u32SpiBaseAddr;
    MS_U32 u32Clk0BaseAddr;
	MS_U32 u32PMBaseAddr;

} hal_msp_t;

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static hal_msp_t _hal_msp =
{
    .u32SpiBaseAddr = BASEADDR_RIU + BK_SPI,
    .u32Clk0BaseAddr = BASEADDR_NONPM_RIU + BK_CLK0,
    .u32PMBaseAddr  = BASEADDR_RIU + BK_PM,
};
//
//  Spi  Clk Table (List)
//
static MS_U16 _hal_ckg_spi_pm[] = {
     PM_SPI_CLK_XTALI
    ,PM_SPI_CLK_27MHZ
    ,PM_SPI_CLK_36MHZ
    ,PM_SPI_CLK_43MHZ
    ,PM_SPI_CLK_54MHZ
    ,PM_SPI_CLK_72MHZ
    ,PM_SPI_CLK_86MHZ
    ,PM_SPI_CLK_108MHZ
    ,PM_SPI_CLK_24MHZ
};

static MS_U16 _hal_ckg_spi_nonpm[] = {
     CLK0_CKG_SPI_XTALI
    ,CLK0_CKG_SPI_27MHZ
    ,CLK0_CKG_SPI_36MHZ
    ,CLK0_CKG_SPI_43MHZ
    ,CLK0_CKG_SPI_54MHZ
    ,CLK0_CKG_SPI_72MHZ
    ,CLK0_CKG_SPI_86MHZ
    ,CLK0_CKG_SPI_108MHZ
};

static MS_U8 guChipSelect = 0;
//MS_U8 _u8SPIDbgLevel;
static MS_BOOL _gbMspiIntEnable;

//-------------------------------------------------------------------------------------------------
// Local Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Enable RIU ISP engine
// @return TRUE : succeed
// @return FALSE : fail
// @note : If Enable ISP engine, the XIU mode does not work
//-------------------------------------------------------------------------------------------------
void HAL_ISP_Enable(void)
{
    SPI_WRITE(SPI_REG_PWD, 0xAAAA);
	PM_WRITE(0x39,0x40);
}

void HAL_ISP_Disable(void)
{
    SPI_WRITE(SPI_REG_PWD, 0x5555);
    //_HAL_SPI_Rest();
}

static MS_BOOL _Hal_GetMsTime( struct timeval tPreTime, MS_U32 u32Fac)
{
    MS_U32 u32NsTime = 0;
    struct timeval time_st;
    do_gettimeofday(&time_st);
    u32NsTime = ((time_st.tv_sec - tPreTime.tv_sec) * 1000) + ((time_st.tv_usec - tPreTime.tv_usec)/1000);
    if(u32NsTime > u32Fac)
        return TRUE;
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
// Wait for SPI Read Data Ready
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static MS_BOOL _HAL_SPI_WaitReadDataRdy(void)
{
    MS_BOOL bRet = FALSE;

    struct timeval time_st;
    SER_FLASH_TIME(time_st);
    do
    {
        if ( (SPI_READ(SPI_REG_RD_DATA_RDY) & SPI_RD_DATARDY_MASK) == SPI_RD_DATARDY )
        {
            bRet = TRUE;
            break;
        }
    } while (!SER_FLASH_EXPIRE(time_st, SERFLASH_SAFETY_FACTOR * 30));



    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("Wait for SPI Read Data Ready fails!\n"));
    }

    return bRet;
}

//-------------------------------------------------------------------------------------------------
// Wait for Write/Erase to be done
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static MS_BOOL _HAL_SPI_WaitWriteDataRdy(void)
{
    MS_BOOL bRet = FALSE;
    struct timeval time_st;
    SER_FLASH_TIME(time_st);

    do
    {
        if ( (SPI_READ(SPI_REG_WR_DATA_RDY) & SPI_WR_DATARDY_MASK) == SPI_WR_DATARDY )
        {
            bRet = TRUE;
            break;
        }

    } while (!SER_FLASH_EXPIRE(time_st, SERFLASH_SAFETY_FACTOR * 30));

    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("Wait for SPI Write Data Ready fails!\n"));
    }

    return bRet;
}

//-------------------------------------------------------------------------------------------------
// Wait for SPI Write Cmd Ready
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static MS_BOOL _HAL_SPI_WaitWriteCmdRdy(void)
{
    MS_BOOL bRet = FALSE;

    struct timeval time_st;
    SER_FLASH_TIME(time_st);

    do
    {
        if ( (SPI_READ(SPI_REG_WR_CMD_RDY) & SPI_WR_CMDRDY_MASK) == SPI_WR_CMDRDY )
        {
            bRet = TRUE;
            break;
        }
    } while (!SER_FLASH_EXPIRE(time_st, SERFLASH_SAFETY_FACTOR * 30));

    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("Wait for SPI Write Cmd Ready fails!\n"));
    }

    return bRet;
}

MS_BOOL HAL_SPI_Read(MS_U32 u32Size, MS_U8 *pu8buffer)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32Idx = 0;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    if ( _HAL_SPI_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SPI_Read_return;
    }

    for(u32Idx = 0; u32Idx < u32Size; u32Idx++)
    {
        SPI_WRITE(SPI_REG_RDREQ, 0x01); // SPI read request

        if ( _HAL_SPI_WaitReadDataRdy() == FALSE )
        {
            goto HAL_SPI_Read_return;
        }
  
        *(pu8buffer + u32Idx) = SPI_READ(SPI_REG_RDATA);
        //printk("HAL_SPI_Read %x \r\n",*(pu8buffer + u32Idx));
    }
    bRet = TRUE;
    return bRet;
HAL_SPI_Read_return:
	printk("HAL_SPI_Read Fail!!! \r\n");
	SPI_WRITE(SPI_REG_CECLR, 1); // SPI CEB dis
	
    SPI_WRITE(SPI_REG_TRG_MODE, 0x2222);     // disable trigger mode

    HAL_ISP_Disable();

    return bRet;
}

MS_BOOL HAL_SPI_Write(MS_U32 u32Size, MS_U8 *pu8buffer)
{
    MS_BOOL bRet = FALSE;
	MS_U32 u32Idx = 0;

    if ( _HAL_SPI_WaitWriteCmdRdy() == FALSE )
    {
            goto HAL_SPI_Write_return;
    }

    for(u32Idx = 0; u32Idx < u32Size; u32Idx++)
    {
        SPI_WRITE(SPI_REG_WDATA, *(pu8buffer + u32Idx));
        if ( _HAL_SPI_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SPI_Write_return;
        }
    }
    printk("HAL_SPI_Write Done!!!\r\n");
    bRet = TRUE;
    return bRet;
HAL_SPI_Write_return:
    printk("HAL_SPI_Write Fail!!!\r\n");
    SPI_WRITE(SPI_REG_CECLR, 1); // SPI CEB dis

    SPI_WRITE(SPI_REG_TRG_MODE, 0x5555);     // disable trigger mode

    HAL_ISP_Disable();

    return bRet;
}

//------------------------------------------------------------------------------
/// Description : Set SPI Polarity
/// @param u8Polarity \b OUT: write riu register
//------------------------------------------------------------------------------
void HAL_SPI_PolaritySet(MS_U8 u8Polarity)
{
    SPI_WRITE(SPI_REG_POLARITY, u8Polarity);
}

//------------------------------------------------------------------------------
/// Description : SPI chip select enable and disable
/// @param Enable \b OUT: enable or disable chip select
//------------------------------------------------------------------------------
void HAL_SPI_ChipSelect(MS_BOOL Enable)
{
    if(Enable)
    {
        SPI_WRITE(SPI_REG_TRG_MODE, 0x3333);      // Enable trigger mode CS state from HIGH to LOW
    }
	else
	{	
	    SPI_WRITE(SPI_REG_CECLR, 1); // SPI CEB dis
	   // printk("gen a high pulse \r\n");
		//SPI_WRITE(SPI_REG_TRG_MODE, 0x2222);	 // disable trigger mode CS state from LOW to HIGH
	}
    return;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SERFLASH_ClkDiv()
/// @brief \b Function \b Description: This function is used to set clock div dynamically
/// @param <IN>        \b eCkgSpi    : enumerate the clk_div
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>    \b NONE    :
/// @param <NOTE>    \b : Please use this function carefully , and is restricted to Flash ability
////////////////////////////////////////////////////////////////////////////////
void HAL_SPI_ClkDiv(SPI_DrvClkDiv eClkDivSpi)
{
    switch (eClkDivSpi)
    {
        case E_SPI_DIV2:
            SPI_WRITE(SPI_REG_CLKDIV,SPI_CLKDIV2);
            break;
        case E_SPI_DIV4:
            SPI_WRITE(SPI_REG_CLKDIV,SPI_CLKDIV4);
            break;
        case E_SPI_DIV8:
            SPI_WRITE(SPI_REG_CLKDIV,SPI_CLKDIV8);
            break;
        case E_SPI_DIV16:
            SPI_WRITE(SPI_REG_CLKDIV,SPI_CLKDIV16);
            break;
        case E_SPI_DIV32:
            SPI_WRITE(SPI_REG_CLKDIV,SPI_CLKDIV32);
            break;
        case E_SPI_DIV64:
            SPI_WRITE(SPI_REG_CLKDIV,SPI_CLKDIV64);
            break;
        case E_SPI_DIV128:
            SPI_WRITE(SPI_REG_CLKDIV,SPI_CLKDIV128);
            break;
        case E_SPI_ClkDiv_NOT_SUPPORT:
            DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SPI_SetCKG()
/// @brief \b Function \b Description: This function is used to set ckg_spi dynamically
/// @param <IN>        \b eCkgSpi    : enumerate the ckg_spi
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>    \b NONE    :
/// @param <NOTE>    \b : Please use this function carefully , and is restricted to Flash ability
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_SPI_SetCKG(SPI_DrvCKG eCkgSpi)
{
    MS_BOOL Ret = FALSE;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
    
    if(_hal_ckg_spi_nonpm[eCkgSpi] == NULL | _hal_ckg_spi_pm[eCkgSpi] == NULL)
    {
         DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("CLOCK NOT SUPPORT \n"));
         return Ret;
    }
    // NON-PM Doman
    CLK_WRITE_MASK(REG_CLK0_CKG_SPI,CLK0_CLK_SWITCH_OFF,CLK0_CLK_SWITCH_MASK);
    CLK_WRITE_MASK(REG_CLK0_CKG_SPI,_hal_ckg_spi_nonpm[eCkgSpi],CLK0_CKG_SPI_MASK); // set ckg_spi
    CLK_WRITE_MASK(REG_CLK0_CKG_SPI,CLK0_CLK_SWITCH_ON,CLK0_CLK_SWITCH_MASK);       // run @ ckg_spi

    // PM Doman
    PM_WRITE_MASK(REG_PM_CKG_SPI,PM_SPI_CLK_SWITCH_OFF,PM_SPI_CLK_SWITCH_MASK); // run @ 12M
    PM_WRITE_MASK(REG_PM_CKG_SPI,_hal_ckg_spi_pm[eCkgSpi],PM_SPI_CLK_SEL_MASK); // set ckg_spi 
    PM_WRITE_MASK(REG_PM_CKG_SPI,PM_SPI_CLK_SWITCH_ON,PM_SPI_CLK_SWITCH_MASK);  // run @ ckg_spi
    Ret = TRUE;
    return Ret;
}

void HAL_SPI_SetMaxClk(MS_U32 u32MaxClk)
{
    MS_U16 u16SPIClkBit = 0;
	MS_U32 u32McuClk = 0, u32TempMcuClk = 0;
	MS_BOOL bCLKMatch = FALSE;
    MS_U8  u8ClkDiv = 0;
    u16SPIClkBit = ((PM_READ(REG_CLK0_CKG_SPI) >> 2)&0xF);
	//printk("HAL_SPI_SetMaxClk %d %d \r\n",u32MaxClk,u16SPIClkBit);
	switch(u16SPIClkBit)
	{
	case 0:
        u32McuClk = 12000000;
        break;		
    case 2:
        u32McuClk = 170000000;
        break;
    case 3:
        u32McuClk = 160000000;
        break;
    case 4:
        u32McuClk = 144000000;
        break;
    case 5:
        u32McuClk = 123000000;
        break;
    case 6:
        u32McuClk = 108000000;
        break;
    case 9:
		u32McuClk = 12000000;
        break;
    case 11:
        u32McuClk = 24000000;
        break;
    default:
		printk("clock not support %x !!!!\r\n",u16SPIClkBit);
	}
    // default hw setting

    do
    {
        u32TempMcuClk = u32McuClk >> u8ClkDiv;
        if(u32MaxClk < u32TempMcuClk)
        {
            u8ClkDiv++;
		}
		else
		{
           bCLKMatch = TRUE; 
        }
    }
    while(!bCLKMatch);
    HAL_SPI_ClkDiv((SPI_DrvClkDiv)u8ClkDiv);
}
