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
/// file    halMSPI.c
/// @brief  Master SPI Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/string.h>
#include <linux/kernel.h>
#include "MsTypes.h"
#include "halMSPI.h"
#include "regMSPI.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define READ_BYTE(_reg)             (*(volatile MS_U8*)(_reg))
#define READ_WORD(_reg)             (*(volatile MS_U16*)(_reg))
#define READ_LONG(_reg)             (*(volatile MS_U32*)(_reg))
#define WRITE_BYTE(_reg, _val)      { (*((volatile MS_U8*)(_reg))) = (MS_U8)(_val); }
#define WRITE_WORD(_reg, _val)      { (*((volatile MS_U16*)(_reg))) = (MS_U16)(_val); }
#define WRITE_LONG(_reg, _val)      { (*((volatile MS_U32*)(_reg))) = (MS_U32)(_val); }

#define MSPI_READ(addr)                      READ_WORD(_hal_msp.u32MspBaseAddr + ((addr)<<2))
#define MSPI_WRITE(addr, val)                WRITE_WORD(_hal_msp.u32MspBaseAddr + ((addr)<<2), (val))
#define CLK_READ(addr)                      READ_WORD(_hal_msp.u32ClkBaseAddr + ((addr)<<2))
#define CLK_WRITE(addr, val)                WRITE_WORD(_hal_msp.u32ClkBaseAddr + ((addr)<<2), (val))
#define _HAL_MSPI_Trigger()                 MSPI_WRITE(MSPI_TRIGGER_OFFSET,MSPI_TRIGGER)
#define _HAL_MSPI_ClearDone()               MSPI_WRITE(MSPI_DONE_CLEAR_OFFSET,MSPI_CLEAR_DONE)
#define MAX_CHECK_CNT                       2000
#define DEBUG_HAL_MSPI(debug_level, x)     do { if (_u8HalMSPIDbgLevel >= (debug_level)) (x); } while(0)

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef struct
{
    MS_U32 u32MspBaseAddr;
    MS_U32 u32ClkBaseAddr;

} hal_msp_t;

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static hal_msp_t _hal_msp =
{
    .u32MspBaseAddr = BASEADDR_RIU + BK_MSP,
    .u32ClkBaseAddr = BASEADDR_RIU + BK_CLK0
};

static MS_U8 guChipSelect = 0;
MS_U8 _u8HalMSPIDbgLevel;
static MS_BOOL _gbMspiIntEnable;

//-------------------------------------------------------------------------------------------------
// Local Functions
//-------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Description : Set MSPI read /write buf size in current operation
/// @param Direct \b OUT: read /write operation direction
/// @param Size   \b OUT: size of read /write buffer
//------------------------------------------------------------------------------

static void _HAL_MSPI_RWBUFSize(MS_BOOL Direct, MS_U8 Size)
{
    MS_U16 u16Data = 0;
    u16Data = MSPI_READ(MSPI_RBF_SIZE_OFFSET);

    if(Direct == MSPI_READ_INDEX)
    {
        u16Data &= MSPI_RWSIZE_MASK;
        u16Data |= Size << MSPI_RSIZE_BIT_OFFSET;
    }
    else
    {
        u16Data &= ~MSPI_RWSIZE_MASK;
        u16Data |= Size;
    }
    MSPI_WRITE(MSPI_RBF_SIZE_OFFSET, u16Data);
}

//------------------------------------------------------------------------------
/// Description : SPI chip select enable and disable
/// @param Enable \b OUT: enable or disable chip select
//------------------------------------------------------------------------------
static void _HAL_MSPI_ChipSelect(MS_BOOL Enable)
{
    MS_U16 regdata = 0;
    MS_U8 bitmask = 0;
    regdata = MSPI_READ(MSPI_CHIP_SELECT_OFFSET);
    if(Enable)
    {
        bitmask = ~(1 << guChipSelect);
        regdata &= bitmask;
    }
    else
    {
        bitmask = (1 << guChipSelect);
        regdata |= bitmask;
    }
    MSPI_WRITE(MSPI_CHIP_SELECT_OFFSET, regdata);
}

//------------------------------------------------------------------------------
/// Description : check MSPI operation complete
/// @return TRUE :  operation complete
/// @return FAIL : failed timeout 
//------------------------------------------------------------------------------ 
static MS_BOOL _HAL_MSPI_CheckDone(void)
{
    MS_U16 uCheckDoneCnt = 0;
    MS_U16 uDoneFlag = 0;
    while(uCheckDoneCnt < MAX_CHECK_CNT)
    {
        uDoneFlag = MSPI_READ(MSPI_DONE_OFFSET);
        if(uDoneFlag & MSPI_DONE_FLAG)
            return TRUE;
        uCheckDoneCnt++;
    }
    DEBUG_MSPI(E_MSPI_DBGLV_ERR,printk("ERROR:MSPI Operation Timeout!!!!!\n"));
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Description : MSPI initial
/// @return void :
//------------------------------------------------------------------------------ 
void HAL_MSPI_Init(void)
{

	printk(KERN_EMERG"HAL_MSPI_Init \r\n");

    MS_U16 TempData;
    //init  MSP
    DEBUG_MSPI(E_MSPI_DBGLV_INFO,printk("HAL_MSPI_Init\n"));

    // CLK SETTING
    TempData = CLK_READ(MSPI_SRC_CLK_OFFSET);
    TempData &= ~MSPI_SRC_CLK_MASK;
    TempData |= MSPI_SRC_CLK_DEFAULT;
    CLK_WRITE(MSPI_SRC_CLK_OFFSET, TempData);

    if(_gbMspiIntEnable)
    {
        printk(KERN_EMERG"HAL_MSPI_Init INT ENABLE\r\n");
        MSPI_WRITE(MSPI_CTRL_OFFSET,(MSPI_INT_ENABLE|MSPI_RESET|MSPI_ENABLE));
    }
    else
    {
        printk(KERN_EMERG"HAL_MSPI_Init INT DISABLE\r\n");
        MSPI_WRITE(MSPI_CTRL_OFFSET,(MSPI_RESET|MSPI_ENABLE));
    }
}

//------------------------------------------------------------------------------
/// Description : MSPI	interrupt enable
/// @param bEnable \b OUT: enable or disable mspi interrupt
/// @return void:
//------------------------------------------------------------------------------
void HAL_MSPI_IntEnable(MS_BOOL bEnable)
{
    if(bEnable)
        _gbMspiIntEnable = TRUE;
    else
        _gbMspiIntEnable = FALSE;
}

//------------------------------------------------------------------------------
/// Description : Get Max of chip select
/// @param void:
/// @return Max of Chip select
//------------------------------------------------------------------------------ 
MS_U8 HAL_MSPI_ChipSelectMax(void)
{
    return MSPI_CHIP_SELECT_MAX;
}

//------------------------------------------------------------------------------
/// Description : Set MSPI chip select
/// @param u8CS \u8 OUT: MSPI chip select
/// @return void: 
//------------------------------------------------------------------------------ 
void HAL_MSPI_SetChipSelect(MS_U8 u8CS)
{
    guChipSelect = u8CS;
}
#if 0
//------------------------------------------------------------------------------
/// Description : Config MSP MMIO base address
/// @param u32PMBankBaseAddr \b IN :base address of MMIO (PM domain)
/// @param u32NONPMRegBaseAddr \b IN :base address of MMIO
/// @param u8DeviceIndex \b IN: index of HW IP
//------------------------------------------------------------------------------ 
void HAL_MSPI_MMIOConfig(MS_U32 u32PMBankBaseAddr, MS_U32 u32NONPMRegBaseAddr, MS_U8 u8DeviceIndex)
{
    if(u8DeviceIndex)
    {
        DEBUG_MSPI(E_MSPI_DBGLV_DEBUG,printk("%s(0x%08X) only support MSPI0\n", __FUNCTION__, (int)u32NONPMRegBaseAddr));
    }
    else
    {
        DEBUG_MSPI(E_MSPI_DBGLV_DEBUG,printk("%s(0x%08X) \n", __FUNCTION__, (int)u32PMBankBaseAddr));
    }
    _hal_msp.u32MspBaseAddr = u32PMBankBaseAddr + BK_MSP;
    _hal_msp.u32ClkBaseAddr = u32PMBankBaseAddr + BK_CLK0;
}
#endif
//-------------------------------------------------------------------------------------------------
/// Description : read data from MSPI
/// @param pData \b IN :pointer to receive data from MSPI read buffer 
/// @param u16Size \ b OTU : read data size
/// @return TRUE  : read data success
/// @return FALSE : read data fail
//-------------------------------------------------------------------------------------------------
MS_BOOL HAL_MSPI_Read(MS_U8 *pData, MS_U16 u16Size)
{
    MS_U8  u8Index = 0;
	MS_U16  u16TempBuf = 0;
    MS_U16 i =0, j = 0;

    for(i = 0; i < u16Size; i+= MAX_READ_BUF_SIZE)
    {
        u16TempBuf = u16Size - i;
        if(u16TempBuf > MAX_READ_BUF_SIZE)
        {
            j = MAX_READ_BUF_SIZE;
        }
        else
        {
            j = u16TempBuf;
        }
        _HAL_MSPI_RWBUFSize(MSPI_READ_INDEX, j);

        HAL_MSPI_Trigger();
        DEBUG_MSPI(E_MSPI_DBGLV_DEBUG,printk("Read Size %x\n",j));
        for(u8Index = 0; u8Index < j; u8Index++)
        {

            if(u8Index & 1)
            {
                u16TempBuf = MSPI_READ((MSPI_READ_BUF_OFFSET + (u8Index >> 1)));
                DEBUG_MSPI(E_MSPI_DBGLV_DEBUG,printk("read Buf data %x index %d\n",u16TempBuf, u8Index));
                pData[u8Index] = u16TempBuf >> 8;
                pData[u8Index-1] = u16TempBuf & 0xFF;
            }
            else if(u8Index == (j -1))
            {
                u16TempBuf = MSPI_READ((MSPI_READ_BUF_OFFSET + (u8Index >> 1)));
                DEBUG_MSPI(E_MSPI_DBGLV_DEBUG,printk("read Buf data %x index %d\n",u16TempBuf, u8Index));
                pData[u8Index] = u16TempBuf & 0xFF;
            }
        }
        pData+= j;
    }
    return TRUE;
}

//------------------------------------------------------------------------------
/// Description : read data from MSPI
/// @param pData \b OUT :pointer to write  data to MSPI write buffer 
/// @param u16Size \ b OTU : write data size
/// @return TRUE  : write data success
/// @return FALSE : wirte data fail
//------------------------------------------------------------------------------
MS_BOOL HAL_MSPI_Wirte(MS_U8 *pData, MS_U16 u16Size)
{
    MS_U8  u8Index = 0;
    MS_U16 u16TempBuf = 0;
    MS_U16 i =0, j = 0;

    for(i = 0; i < u16Size; i+= MAX_WRITE_BUF_SIZE)
    {
        u16TempBuf = u16Size - i;
        if(u16TempBuf > MAX_WRITE_BUF_SIZE)
        {
            j = MAX_WRITE_BUF_SIZE;
        }
        else
        {
            j = u16TempBuf;
        }
        DEBUG_MSPI(E_MSPI_DBGLV_DEBUG,printk("Write Size %x\n",j));
        for(u8Index = 0; u8Index < j; u8Index++)
        {
            if(u8Index & 1)
            {
                u16TempBuf = (pData[u8Index] << 8) | pData[u8Index-1];
                DEBUG_MSPI(E_MSPI_DBGLV_DEBUG,printk("write Buf data %x index %d\n",u16TempBuf, u8Index));
                MSPI_WRITE((MSPI_WRITE_BUF_OFFSET + (u8Index >> 1)),u16TempBuf);
            }
            else if(u8Index == (j -1))
            {
                DEBUG_MSPI(E_MSPI_DBGLV_DEBUG,printk("write Buf data %x index %d\n",pData[u8Index], u8Index));
                MSPI_WRITE((MSPI_WRITE_BUF_OFFSET + (u8Index >> 1)),pData[u8Index]);	
            }
        }

        pData += j;
        _HAL_MSPI_RWBUFSize(MSPI_WRITE_INDEX, j);
        HAL_MSPI_Trigger();        
    }
        // set write data size

    return TRUE;
}

//------------------------------------------------------------------------------
/// Description : Reset  DC register setting of MSPI
/// @param NONE
/// @return TRUE  : reset complete
//------------------------------------------------------------------------------
MS_BOOL HAL_MSPI_Reset_DCConfig(void)
{
	//DC reset
    MSPI_WRITE(MSPI_DC_TR_START_OFFSET, 0x00);
    MSPI_WRITE(MSPI_DC_TB_OFFSET, 0x00);
	return TRUE;
}

//------------------------------------------------------------------------------
/// Description : Reset  Frame register setting of MSPI
/// @param NONE
/// @return TRUE  : reset complete
//------------------------------------------------------------------------------
MS_BOOL HAL_MSPI_Reset_FrameConfig(void)
{
    // Frame reset
    MSPI_WRITE(MSPI_FRAME_WBIT_OFFSET, 0xFFF);
	MSPI_WRITE(MSPI_FRAME_WBIT_OFFSET+2, 0xFFF);
	MSPI_WRITE(MSPI_FRAME_RBIT_OFFSET, 0xFFF);
	MSPI_WRITE(MSPI_FRAME_RBIT_OFFSET+2, 0xFFF);
	return TRUE;
}

//------------------------------------------------------------------------------
/// Description : Reset  CLK register setting of MSPI
/// @param NONE
/// @return TRUE  : reset complete
//------------------------------------------------------------------------------
MS_BOOL HAL_MSPI_Reset_CLKConfig(void)
{
    MS_U16 Tempbuf;
	//reset clock
    Tempbuf = MSPI_READ(MSPI_CTRL_OFFSET);
    Tempbuf &= 0x3F;
    MSPI_WRITE(MSPI_CTRL_OFFSET, Tempbuf);
	return TRUE;
}

//------------------------------------------------------------------------------
/// Description : get read/write buffer size
/// @param bDirection    \b OUT   specify to get read/write buffer size
/// @return buffer sizel
//------------------------------------------------------------------------------
MS_U8 HAL_MSPI_GetBufSize(MS_BOOL bDirection)
{
    if(bDirection == MSPI_READ_INDEX)
        return MAX_READ_BUF_SIZE;
    else
        return MAX_WRITE_BUF_SIZE;
}

//------------------------------------------------------------------------------
/// Description : Trigger MSPI operation
/// @return TRUE  : operation success
/// @return FALSE : operation timeout
//------------------------------------------------------------------------------
MS_BOOL HAL_MSPI_Trigger(void)
{
    // chip select enable
  //  _HAL_MSPI_ChipSelect(TRUE);

    // trigger operation
    _HAL_MSPI_Trigger();
    // check operation complete
    if(!_HAL_MSPI_CheckDone())
        return FALSE;
    // clear done flag
    _HAL_MSPI_ClearDone();
    // chip select disable
   // _HAL_MSPI_ChipSelect(FALSE);
    // reset read/write buffer size
    MSPI_WRITE(MSPI_RBF_SIZE_OFFSET,0x0);
	return TRUE;
}

void HAL_MSPI_SlaveEnable(MS_BOOL Enable)
{
     _HAL_MSPI_ChipSelect(Enable);
}

//------------------------------------------------------------------------------
/// Description : get valid range of DC timing
/// @param eDCField    \b OUT enum of DC timing type
/// @return NONE
//------------------------------------------------------------------------------
MS_U8 HAL_MSPI_DCConfigMax(eDC_config eDCField)
{
    switch(eDCField)
    {
    case E_MSPI_TRSTART:
        return MSPI_DC_TRSTART_MAX;
    case E_MSPI_TREND:
        return MSPI_DC_TREND_MAX;
    case E_MSPI_TB:
        return MSPI_DC_TB_MAX;
    case E_MSPI_TRW:
        return MSPI_DC_TRW_MAX;
    default:
        return 0;
    }
	
}

//------------------------------------------------------------------------------
/// Description : config spi transfer timing
/// @param ptDCConfig    \b OUT  struct pointer of bits of buffer tranferred to slave config
/// @return NONE
//------------------------------------------------------------------------------
void HAL_MSPI_SetDcTiming (eDC_config eDCField, MS_U8 u8DCtiming)
{
   MS_U16 u16TempBuf = 0;
   switch(eDCField)
   {
   case E_MSPI_TRSTART:
       u16TempBuf = MSPI_READ(MSPI_DC_TR_START_OFFSET);
       u16TempBuf &= (~MSPI_DC_MASK);
       u16TempBuf |= u8DCtiming;
       MSPI_WRITE(MSPI_DC_TR_START_OFFSET, u16TempBuf);
       break;
   case E_MSPI_TREND:
       u16TempBuf = MSPI_READ(MSPI_DC_TR_END_OFFSET);
	   u16TempBuf &= MSPI_DC_MASK;
	   u16TempBuf |= u8DCtiming << MSPI_DC_BIT_OFFSET; 
       MSPI_WRITE(MSPI_DC_TR_END_OFFSET, u16TempBuf);
       break;
   case E_MSPI_TB:
       u16TempBuf = MSPI_READ(MSPI_DC_TB_OFFSET);
       u16TempBuf &= (~MSPI_DC_MASK);
       u16TempBuf |= u8DCtiming;
       MSPI_WRITE(MSPI_DC_TB_OFFSET, u16TempBuf);
       break;
   case E_MSPI_TRW:
       u16TempBuf = MSPI_READ(MSPI_DC_TRW_OFFSET);
       u16TempBuf &= MSPI_DC_MASK;
       u16TempBuf |= u8DCtiming << MSPI_DC_BIT_OFFSET; 
       MSPI_WRITE(MSPI_DC_TRW_OFFSET, u16TempBuf);
       break;
   }
}

MS_U8 HAL_MSPI_CLKConfigMax(eCLK_config eCLKField)
{
    switch(eCLKField)
    {
    case E_MSPI_POL:
        return MSPI_CLK_POLARITY_MAX;
    case E_MSPI_PHA:
        return MSPI_CLK_PHASE_MAX;
    case E_MSPI_CLK:
        return MSPI_CLK_CLOCK_MAX;
    default:
        return 0;
    }
}

void HAL_MSPI_SetCLKTiming(eCLK_config eCLKField, MS_U8 u8CLKVal)
{
   MS_U16 u16TempBuf = 0;
   switch(eCLKField)
   {
   case E_MSPI_POL:
       u16TempBuf = MSPI_READ(MSPI_CLK_CLOCK_OFFSET);
	   u16TempBuf &= ~(MSPI_CLK_POLARITY_MASK);
       u16TempBuf |= u8CLKVal << MSPI_CLK_POLARITY_BIT_OFFSET;     
       break;
   case E_MSPI_PHA:
       u16TempBuf = MSPI_READ(MSPI_CLK_CLOCK_OFFSET);
	   u16TempBuf &= ~(MSPI_CLK_PHASE_MASK);
       u16TempBuf |= u8CLKVal << MSPI_CLK_PHASE_BIT_OFFSET;
       break;
   case E_MSPI_CLK:
       u16TempBuf = MSPI_READ(MSPI_CLK_CLOCK_OFFSET);
       u16TempBuf &= MSPI_CLK_CLOCK_MASK;
       u16TempBuf |= u8CLKVal << MSPI_CLK_CLOCK_BIT_OFFSET;
       break;
   }
   MSPI_WRITE(MSPI_CLK_CLOCK_OFFSET, u16TempBuf);
}

MS_U8 HAL_MSPI_FrameConfigMax(void)
{
    return MSPI_FRAME_BIT_MAX;
}

void HAL_MSPI_SetPerFrameSize(MS_BOOL bDirect, MS_U8 u8BufOffset, MS_U8 u8PerFrameSize)
{
    MS_U8 u8Index = 0;
    MS_U16 u16TempBuf = 0;
	MS_U8 u8BitOffset = 0;
	MS_U16 u16regIndex = 0;
    if(bDirect == MSPI_READ_INDEX)
    {
        u16regIndex = MSPI_FRAME_RBIT_OFFSET;
	}
	else
    {
        u16regIndex = MSPI_FRAME_WBIT_OFFSET;
    }
    if(u8BufOffset >=4)
    {
        u8Index++;
        u8BufOffset -= 4;
    }
    u8BitOffset = u8BufOffset * MSPI_FRAME_BIT_FIELD;
    u16TempBuf = MSPI_READ(u16regIndex+ u8Index);
    u16TempBuf &= ~(MSPI_FRAME_BIT_MASK << u8BitOffset);
    u16TempBuf |= u8PerFrameSize << u8BitOffset;
    MSPI_WRITE((u16regIndex + u8Index), u16TempBuf);
}

void HAL_MSPI_Disable(void)
{
    MSPI_WRITE(MSPI_CTRL_OFFSET,0);
}

MS_U32 HAL_MSPI_Get_Clk(void)
{
    MS_U16 u16RegClk;
    MS_U32 u32SrcClk = 0;
	MS_U16 u16ClkDiv = 0;
	u16RegClk = CLK_READ(MSPI_SRC_CLK_OFFSET);
	u16RegClk &= MSPI_SRC_CLK_MASK;
	u16RegClk >> MSPI_SRC_CLK_SHIFT;
    switch (u16RegClk)
    {
    case 0:
		u32SrcClk = 24000000;
        break;
	case 1:
		u32SrcClk = 12000000;
		break;
	case 2:
		u32SrcClk = 48000000;
		break;
    case 3:
		u32SrcClk = 108000000;
		break;
    default:
		break;
    }
	u16ClkDiv = MSPI_READ(MSPI_CLK_CLOCK_OFFSET) >> MSPI_CLK_CLOCK_BIT_OFFSET;
	u32SrcClk = u32SrcClk >> (u16ClkDiv + 1);
    return u32SrcClk;
}

void HAL_MSPI_Set_Clk(MS_U32 u32MaxClk)
{
    MS_U16 u16RegClk;
    MS_U32 u32SrcClk = 0;
	u16RegClk = CLK_READ(MSPI_SRC_CLK_OFFSET);
	u16RegClk &= MSPI_SRC_CLK_MASK;
	u16RegClk >> MSPI_SRC_CLK_SHIFT;
    switch (u16RegClk)
    {
    case 0:
		u32SrcClk = 24000000;
        break;
	case 1:
		u32SrcClk = 12000000;
		break;
	case 2:
		u32SrcClk = 48000000;
		break;
    case 3:
		u32SrcClk = 108000000;
		break;
    default:
		break;
    }
}

