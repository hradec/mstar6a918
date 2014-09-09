////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
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
/// @file   Mhal_mtlb.c
/// @brief  MTLB Driver
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include "mhal_mtlb.h"
#include "mdrv_mtlb.h"
#include <linux/pci.h>
#include <linux/version.h>

//-------------------------------------------------------------------------------------------------
//  Define
//-------------------------------------------------------------------------------------------------
#ifndef BIT0
#define BIT0	                    0x00000001
#endif

#ifndef BIT1
#define BIT1	                    0x00000002
#endif

#ifndef BIT2
#define BIT2	                    0x00000004
#endif

#ifndef BIT3
#define BIT3	                    0x00000008
#endif

#ifndef BIT4
#define BIT4	                    0x00000010
#endif

#ifndef BIT5
#define BIT5	                    0x00000020
#endif

#ifndef BIT6
#define BIT6	                    0x00000040
#endif

#ifndef BIT7
#define BIT7	                    0x00000080
#endif

#ifndef BIT8
#define BIT8	                    0x00000100
#endif

#ifndef BIT9
#define BIT9	                    0x00000200
#endif

#ifndef BIT10
#define BIT10	                    0x00000400
#endif

#ifndef BIT11
#define BIT11	                    0x00000800
#endif

#ifndef BIT12
#define BIT12	                    0x00001000
#endif

#ifndef BIT13
#define BIT13	                    0x00002000
#endif

#ifndef BIT14
#define BIT14	                     0x00004000
#endif

#ifndef BIT15
#define BIT15  	                   0x00008000
#endif

//-------------------------------------------------------------------------------------------------
//  Macros
//-------------------------------------------------------------------------------------------------
#define Write_TLB_Entry(entry, data)               *(volatile u32*)(entry) = data;           

//-------------------------------------------------------------------------------------------------
//  Local Variable
//-------------------------------------------------------------------------------------------------
spinlock_t mhal_mtlb_lock;
extern void _chip_flush_miu_pipe(void);
//-------------------------------------------------------------------------------------------------
//  MTLB hardware
//-------------------------------------------------------------------------------------------------

/*8-bit RIU address*/
u8 MHal_MTLB_ReadReg8( u32 u32bank, u32 u32reg )
{
    u8 val;
    u32 address = MTLB_RIU_REG_BASE + u32bank*0x100*2;
    address = address + (u32reg << 1) - (u32reg & 1);

    val = *( ( volatile u8* ) address );
    return val;
}

void MHal_MTLB_WritReg8( u32 u32bank, u32 u32reg, u8 u8val )
{
    u32 address = MTLB_RIU_REG_BASE + u32bank*0x100*2;
    address = address + (u32reg << 1) - (u32reg & 1);

    *( ( volatile u8* ) address ) = u8val;
}

u16 MHal_MTLB_ReadReg16( u32 u32bank, u32 u32reg )
{
    u16 val;
    u32 address = MTLB_RIU_REG_BASE + u32bank*0x100*2;
    address = address + (u32reg << 1);

    val = *( ( volatile u16* ) address );
    return val;
}

void MHal_MTLB_WritReg16( u32 u32bank, u32 u32reg, u16 u16val )
{
    u32 address = MTLB_RIU_REG_BASE + u32bank*0x100*2;
    address = address + (u32reg << 1);

    *( ( volatile u16* ) address ) = u16val;
}

void MHal_MTLB_WritReg8Bit( u32 u32bank, u32 u32reg, bool bEnable, u8 u8Mask )
{
    u8 val = MHal_MTLB_ReadReg8( u32bank, u32reg );
    val = (bEnable) ? (val | u8Mask) : (val & ~u8Mask );
    MHal_MTLB_WritReg8( u32bank, u32reg, val );
}

void MHal_MTLB_WritReg16Bit( u32 u32bank, u32 u32reg, bool bEnable, u16 u16Mask )
{
    u16 val = MHal_MTLB_ReadReg16( u32bank, u32reg );
    val = (bEnable) ? (val | u16Mask) : (val & ~u16Mask );
    MHal_MTLB_WritReg16( u32bank, u32reg, val );
}
void flush_miu_pipe(void)
{
   unsigned int    dwReadData = 0;

    //Flush Pipe Data
    dwReadData = *(volatile unsigned int *)(0xBF000000 + (0x101882 << 1));

	  dwReadData &= ~(BIT4 | BIT3 | BIT2 | BIT1 | BIT0);
    dwReadData |= 0x02;    //Mode 2: Flush Pipe Data

    *(volatile unsigned int *)(0xBF000000 + (0x101882 << 1)) = dwReadData;
    *(volatile unsigned int *)(0xBF000000 + (0x101882 << 1)) |= BIT4;

    do
    {
        dwReadData = *(volatile unsigned int *)(0xBF000000 + (0x1018BE << 1));
        dwReadData &= BIT9;  //Check Status of Flush Pipe Finish

    } while(dwReadData == 0);
}

void MHal_MTLB_Init(void)
{	
    unsigned long flag=0;

    spin_lock_irqsave(&mhal_mtlb_lock,flag);

    /*set lower bound*/
    MHal_MTLB_WritReg16(MTLB_RIU_BANK, REG_TLB_L_BOUND, 0x0000);

	/*set upper bound*/
    MHal_MTLB_WritReg16(MTLB_RIU_BANK, REG_TLB_H_BOUND, 0xffff);

    /*set TLB write header*/
    MHal_MTLB_WritReg16(MTLB_RIU_BANK, REG_TLB_W_HEADER, TLB_W_HEADER);

	/*set GE to TLB cleint ID0*/
    MHal_MTLB_WritReg8(MTLB_RIU_BANK, REG_TLB_W_ID0, TLB_GE_ID);

    /*enable TLB cleint ID0*/
	MHal_MTLB_WritReg8Bit(MTLB_RIU_BANK, REG_TLB_W_ID0, 1, TLB_ID0_ENABLE);

    /*set MIPS to TLB updater*/
    MHal_MTLB_WritReg8(MTLB_RIU_BANK, REG_TLB_W_CTL, TLB_MIPS_ID);

    spin_unlock_irqrestore(&mhal_mtlb_lock,flag);


}

bool MHal_MTLB_Mapping(u8 u8miu, u32 u32va, u32 u32pa, u32 u32size)
{
    u32 u32header0;
    u16 u16data0;
	unsigned long flag=0;

	
    /* MTLB only support MIU0 */
	if(u8miu > E_MTLB_MIU_0)
    {
		MTLB_DBG("MTLB only support MIU0\n");
        return FALSE;
    }
	
    /* Virtual address maximum is 32MB */
    if(u32va > 0x2000000)
    {
		MTLB_DBG("u32va = 0x%x, which should be in 32MB\n", u32va);
        return FALSE;
    }
	
    /* Virtual address and physical address should be align to 0x1000 */
    if((u32va & (0x1000-1)) || (u32pa & (0x1000-1)))
    {
		MTLB_DBG("u32va = 0x%x, u32pa = 0x%x, which should align to 0x1000\n", u32va, u32pa);
        return FALSE;
    }

	/* Size should be be align to 0x1000 */
	if(u32size & (0x1000-1))
    {
		MTLB_DBG("u32size = 0x%x, which should align to 0x1000\n", u32size);
        return FALSE;
    }

	spin_lock_irqsave(&mhal_mtlb_lock,flag);
	
    /*Enable TLB write*/
	MHal_MTLB_WritReg8Bit(MTLB_RIU_BANK, REG_TLB_W_CTL, 1, TLB_W_ENABLE);

	mb();
    
    while(u32size != 0)
    {
        u32header0 = 0xa0000000 + (TLB_W_HEADER << 17);
        u32header0 = u32header0 + (u32va >> 12 << 4);
        u16data0 = (u32pa >> 12);
	   
		mb();
	
        Write_TLB_Entry(u32header0, u16data0);

		u32va += 0x1000;
        u32pa += 0x1000;
		u32size -= 0x1000;
    }
	
	mb();

    /*Flush miu pipe to push data to TLB sram*/
    flush_miu_pipe();
    
    /* Disable TLB write*/
    MHal_MTLB_WritReg8Bit(MTLB_RIU_BANK, REG_TLB_W_CTL, 0, TLB_W_ENABLE);

    spin_unlock_irqrestore(&mhal_mtlb_lock,flag);

    return TRUE;

}

bool MHal_MTLB_Dump(u8 u8miu, u32 *u32va, u32 *u32pa)
{
    u16 u16data;
	u16 index;

    /* MTLB only support MIU0 */
	if(u8miu > E_MTLB_MIU_0)
    {
		MTLB_DBG("MTLB only support MIU0\n");
        return FALSE;
    }

    /* Virtual address maximum is 32MB */
    if(*u32va > 0x2000000)
    {
		MTLB_DBG("*u32va = 0x%x, which should be in 32MB\n", *u32va);
        return FALSE;
    }

    /* Virtual address and physical address should be align to 0x1000 */
    if((*u32va & (0x1000-1)))
    {
		MTLB_DBG("*u32va = 0x%x, which should align to 0x2000\n", *u32va);
        return FALSE;
    }
	
    /*Enable TLB sram dump*/
	u16data = MHal_MTLB_ReadReg16(MTLB_RIU_BANK, REG_TLB_CTL);
	u16data |= TLB_DEBUG_ENABLE;
    MHal_MTLB_WritReg16(MTLB_RIU_BANK, REG_TLB_CTL, u16data);

    /*Dump mapping from TLB sram*/
	index = (u16)(*u32va >> 12);
	MHal_MTLB_WritReg16(MTLB_RIU_BANK, REG_TLB_DEB_ADDR, index);
    u16data = MHal_MTLB_ReadReg16(MTLB_RIU_BANK, REG_TLB_DEB_OUT);
    *u32pa = (u32)u16data << 12;

    /*Disable TLB sram dump*/
	u16data = MHal_MTLB_ReadReg16(MTLB_RIU_BANK, REG_TLB_CTL);
	u16data &= TLB_DEBUG_DISABLE;
    MHal_MTLB_WritReg16(MTLB_RIU_BANK, REG_TLB_CTL, u16data);

    return TRUE;

}

