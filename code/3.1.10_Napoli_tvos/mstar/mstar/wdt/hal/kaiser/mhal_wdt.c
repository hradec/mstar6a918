
//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MST_XTAL_CLOCK_HZ   12000000UL
#define WDT_RESET_TIME  0x10    //unit: sec
//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
#include "mhal_wdt_reg.h"

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

#define REG16(_u32RegBase)     *((volatile U16*)((_u32RegBase)))

static void HAL_WDT_Write2Byte(U32 u32RegBase, U16 data)
{
	REG16(u32RegBase) = data;
}

static U16 HAL_WDT_Read2Byte(U32 u32RegBase)
{
	return REG16(u32RegBase);
}

static void HAL_WDT_Write4Byte(U32 _u32RegBase, U32 data)
{
	REG16(_u32RegBase) = data & 0xFFFF;
	REG16((_u32RegBase+0x4)) = (data >>16) & 0xFFFF;
}

static U32 HAL_WDT_Read4Byte(U32 _u32RegBase)
{
	U32 data = 0;
	data = REG16(_u32RegBase+0x4) ;
	data = data <<16 | REG16(_u32RegBase);
	return data;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void HAL_WDT_DisableWDT(void)
{
    HAL_WDT_Write4Byte(REG_WDT_MAX, 0x0000UL);
}

BOOL HAL_WDT_IsWDTEnabled(void)
{
    if( 0 != HAL_WDT_Read4Byte(REG_WDT_MAX) )
        return true;
    else
        return false;
}

BOOL HAL_WDT_IsResetByWDT(void)
{
    return (HAL_WDT_Read2Byte(REG_WDT_RST_FLAG) & BIT0);
}


static int count = 0;
void HAL_WDT_ClearWDT(void)
{
    HAL_WDT_Write2Byte(REG_WDT_CLR, (HAL_WDT_Read2Byte(REG_WDT_CLR) | BIT0));

	HAL_WDT_Write2Byte(REG_TEST, count++);
}

void HAL_WDT_ClearWDTStatus(void)
{
    HAL_WDT_Write2Byte(REG_WDT_RST_FLAG, (HAL_WDT_Read2Byte(REG_WDT_RST_FLAG) | BIT0));
}

void HAL_WDT_SetWDT_MS(S32 msec)
{
    HAL_WDT_Write4Byte(REG_WDT_MAX, ((msec) * (MST_XTAL_CLOCK_HZ/1000)));
}

void HAL_WDT_SetWDTInt(U16 sec)
{
    HAL_WDT_SetWDT_MS(sec*1000);
}


