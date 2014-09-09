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


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include "drvSPINAND.h"
#include "drvSPICMD.h"
#include "chip_int.h"
#include "chip_setup.h"

extern struct device *spi_nand_dev;

//-------------------------------------------------------------------------------------------------
//  Macro definition
//-------------------------------------------------------------------------------------------------
#define READ_WORD(_reg)                     (*(volatile U16*)(_reg))
#define WRITE_WORD(_reg, _val)              { (*((volatile U16*)(_reg))) = (U16)(_val); }
#define BDMA_READ(addr)                     READ_WORD(_hal_fsp.u32BDMABaseAddr + (addr<<2))
#define BDMA_WRITE(addr, val)               WRITE_WORD(_hal_fsp.u32BDMABaseAddr + (addr<<2),(val))
#define QSPI_READ(addr)                     READ_WORD(_hal_fsp.u32QspiBaseAddr + (addr<<2))
#define QSPI_WRITE(addr, val)               WRITE_WORD(_hal_fsp.u32QspiBaseAddr + (addr<<2),(val))
#define CLK_WRITE(addr, val)                WRITE_WORD(_hal_fsp.u32CLK0BaseAddr + (addr<<2),(val))
//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef struct
{
    U32  u32FspBaseAddr;     // REG_ISP_BASE
    U32  u32QspiBaseAddr;    // REG_QSPI_BASE
    U32  u32PMBaseAddr;      // REG_PM_BASE
    U32  u32CLK0BaseAddr;    // REG_PM_BASE
    U32  u32RiuBaseAddr;     // REG_PM_BASE
    U32  u32BDMABaseAddr;    // REG_BDMA_BASE
} hal_fsp_t;

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static hal_fsp_t _hal_fsp =
{
    .u32FspBaseAddr = RIU_PM_BASE + BK_FSP,
    .u32QspiBaseAddr = RIU_PM_BASE + BK_QSPI,
    .u32PMBaseAddr = RIU_PM_BASE + BK_PMSLP,
    .u32CLK0BaseAddr = RIU_BASE + BK_CHIP,
    .u32RiuBaseAddr= RIU_PM_BASE,
    .u32BDMABaseAddr = RIU_BASE + BK_BDMA,
};
U8 _u8SPINANDDbgLevel;

BOOL _gbRIURead = FALSE;

static BOOL FSP_WRITE_BYTE(U32 u32RegAddr, U8 u8Val)
{
    if (!u32RegAddr)
    {
        printk(KERN_ERR"%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

    ((volatile U8*)(_hal_fsp.u32FspBaseAddr))[(u32RegAddr << 1) - (u32RegAddr & 1)] = u8Val;
    return TRUE;
}

static BOOL FSP_WRITE(U32 u32RegAddr, U16 u16Val)
{
    if (!u32RegAddr)
    {
        printk(KERN_ERR"%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

    ((volatile U16*)(_hal_fsp.u32FspBaseAddr))[u32RegAddr] = u16Val;
    return TRUE;
}

static U8 FSP_READ_BYTE(U32 u32RegAddr)
{
    return ((volatile U8*)(_hal_fsp.u32FspBaseAddr))[(u32RegAddr << 1) - (u32RegAddr & 1)];
}

static U16 FSP_READ(U32 u32RegAddr)
{
    return ((volatile U16*)(_hal_fsp.u32FspBaseAddr))[u32RegAddr];
}

static BOOL _HAL_FSP_ChkWaitDone(void)
{
//consider as it spend very long time to check if FSP done, so it may implment timeout method to improve
    U16 u16Try = 0;
    U8 u8DoneFlag = 0;

    while (u16Try < CHK_NUM_WAITDONE)
    {
        u8DoneFlag = FSP_READ(REG_FSP_DONE);
        if((u8DoneFlag & DONE_FSP) == DONE_FSP)
        {
            return TRUE;
        }

        if(++u16Try%1000 == 0)
            cond_resched();

        udelay(1);
    }
    return FALSE;
}

static void _HAL_FSP_GetRData(U8 *pu8Data, U8 u8DataSize)
{
    U8 u8Index = 0;
    //printk("Read Pointer %lx\r\n",(U32)pu8Data);
    for (u8Index = 0; u8Index < u8DataSize; u8Index++)
    {
        pu8Data[u8Index] = FSP_READ_BYTE(REG_FSP_READ_BUFF + u8Index);
    }
}

static U32 _HAL_FSP_CHECK_SPINAND_DONE(U8 *pu8Status)
{
    U8 u8Data = SPI_NAND_STAT_OIP;
    U16 u16Try = 0;
    while(u16Try < CHK_NUM_WAITDONE)
    {
        //FSP init config
        FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
        FSP_WRITE(REG_FSP_CTRL2, 0);
        FSP_WRITE(REG_FSP_CTRL4, 0);
        FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_GF);
        FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 1, SPI_NAND_REG_STAT);
        FSP_WRITE(REG_FSP_WRITE_SIZE, 0x2);
        FSP_WRITE(REG_FSP_READ_SIZE, 0x1);
        //Trigger FSP
        FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

        //Check FSP done flag
        if (_HAL_FSP_ChkWaitDone() == FALSE)
        {
            DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR"CD Wait FSP Done Time Out !!!!\r\n"));
            return ERR_SPINAND_TIMEOUT;
        }

        //Clear FSP done flag
        FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
		
        _HAL_FSP_GetRData((&u8Data), 1);
        if((u8Data & SPI_NAND_STAT_OIP) == 0 )
            break;

        if(++u16Try%1000 == 0)
            cond_resched();

        udelay(1);
    }

    if(u16Try == CHK_NUM_WAITDONE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR"CD Wait OIP Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    if(pu8Status)
        *pu8Status = u8Data;
	
    return ERR_SPINAND_SUCCESS;
}

static U32 _HAL_SPINAND_WRITE_ENABLE(void)
{
    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    // FIRSET COMMAND PRELOAD
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_WREN);

    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x1);
    // read buffer size
    FSP_WRITE(REG_FSP_READ_SIZE, 0);	
    //Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR"WE Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    return ERR_SPINAND_SUCCESS;

}

static U32 _HAL_SPINAND_RIU_READ(U16 u16Addr, U32 u32DataSize, U8 *u8pData)
{
    U32 u32Index = 0;
    U8  u8Addr = 0;
    U32 u32RealLength = 0;
    U8  u8WbufIndex = 0;
    S8  s8Index = 0;

    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_RFC);
    u8WbufIndex++;

    //Set Write & Read Length
    FSP_WRITE(REG_FSP_WRITE_SIZE, 4); //add 4 because it is included of command and Address setting length
    if(u32DataSize > MAX_READ_BUF_CNT)
        FSP_WRITE(REG_FSP_READ_SIZE, MAX_READ_BUF_CNT);
    else
        FSP_WRITE(REG_FSP_READ_SIZE, u32DataSize);

    for(u32Index = 0; u32Index < u32DataSize; u32Index +=MAX_READ_BUF_CNT)
    {

        for(s8Index = (SPI_NAND_PAGE_ADDR_LEN - 1); s8Index >= 0 ; s8Index--)
        {
            u8Addr = (u16Addr >> (8 * s8Index) )& 0xFF;
            FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), u8Addr);
            u8WbufIndex++;
        }
        // set dummy byte
        FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), 0x00);
        //Trigger FSP
        FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

        //Check FSP done flag
        if (_HAL_FSP_ChkWaitDone() == FALSE){
            DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR"RIUR Wait FSP Done Time Out !!!!\r\n"));
            return ERR_SPINAND_TIMEOUT;
        }
        //Get Read Data
        u32RealLength = u32DataSize - u32Index;
        if(u32RealLength >= MAX_READ_BUF_CNT)
            u32RealLength = MAX_READ_BUF_CNT;

        _HAL_FSP_GetRData((u8pData + u32Index), u32RealLength);

        //Clear FSP done flag
        FSP_WRITE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
        // update Read Start Address
        u16Addr += u32RealLength;
        //printk("u16Addr = %x \r\n",u16Addr);
        u8WbufIndex -= SPI_NAND_PAGE_ADDR_LEN;

    }
    return ERR_SPINAND_SUCCESS;
}

static U32 _HAL_SPINAND_BDMA_READ(U16 u16Addr, U32 u32DataSize, U8 *u8pData)
{
    U32 u32Addr1;
    U16 u16data;
    U32 u32Timer = 0;
    U32 u32Ret = ERR_SPINAND_TIMEOUT;

    //Set source and destination path
    BDMA_WRITE(0x00, 0x0000);

    u32Addr1 = (U32)u8pData;
    #if defined(CONFIG_MIPS)
    _dma_cache_wback_inv((U32)u8pData, u32DataSize);
    if( u32Addr1 > MIPS_MIU1_BASE)
    {
        BDMA_WRITE(0x02, 0x4145);
        u32Addr1 -= MIPS_MIU1_BASE;
    }
    else
    {
        BDMA_WRITE(0x02, 0x4045);
        u32Addr1 -= MIPS_MIU0_BASE;
    }
    #elif defined(CONFIG_ARM)
    u32Addr1 = dma_map_single(spi_nand_dev, (void*)u8pData, u32DataSize, DMA_FROM_DEVICE);
    if( u32Addr1 >= MSTAR_MIU1_BUS_BASE)
    {
        BDMA_WRITE(0x02, 0x4145);
        u32Addr1 -= MSTAR_MIU1_BUS_BASE;
    }
    else
    {
        BDMA_WRITE(0x02, 0x4045);
        u32Addr1 -= MSTAR_MIU0_BUS_BASE;
    }
    #else
    BDMA_WRITE(0x02, 0x4045);
    #endif

    // Set start address
    BDMA_WRITE(0x04, (u16Addr & 0x0000FFFF));
    BDMA_WRITE(0x05, (u16Addr>>16));

    // Set end address
    BDMA_WRITE(0x06, (u32Addr1 & 0x0000FFFF));
    BDMA_WRITE(0x07, (u32Addr1 >> 16));
    // Set Size

    BDMA_WRITE(0x08, (u32DataSize & 0x0000FFFF));
    BDMA_WRITE(0x09, (u32DataSize >> 16));

    // Trigger
    BDMA_WRITE(0x00, 1);

    do
    {
        //check done
        u16data = BDMA_READ(0x01);
        if(u16data & 8)
        {
            //clear done
            BDMA_WRITE(0x01, 8);
            u32Ret = ERR_SPINAND_SUCCESS;
            break;
        }
        if(++u32Timer%1000 == 0)
            cond_resched();

        udelay(1);
    } while (u32Timer < CHK_NUM_WAITDONE);

    #if defined(CONFIG_MIPS)
    #elif defined(CONFIG_ARM)
    dma_unmap_single(spi_nand_dev, u32Addr1, u32DataSize, DMA_FROM_DEVICE);
    #endif

    return u32Ret;

}

U32 HAL_SPINAND_Init(void)
{
    //set pad mux for spinand
    CLK_WRITE(0x04, 0x13);
    QSPI_WRITE(0x7A, 0x01);
    // reset spinand
    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_RESET);

    FSP_WRITE(REG_FSP_WRITE_SIZE, 1);

    //Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR"INI Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    // For first RESET condition after power up, tRST will be 1ms maximum 
    msleep(2);

    return ERR_SPINAND_SUCCESS;
}

void HAL_SPINAND_Config(U32 u32PMRegBaseAddr, U32 u32NonPMRegBaseAddr)
{
    DEBUG_SPINAND(E_SPINAND_DBGLV_DEBUG, printk(KERN_INFO"%s(0x%08X, 0x%08X)\n", __FUNCTION__, (int)u32PMRegBaseAddr, (int)u32NonPMRegBaseAddr));
    _hal_fsp.u32FspBaseAddr = u32PMRegBaseAddr + BK_FSP;
    _hal_fsp.u32PMBaseAddr = u32PMRegBaseAddr + BK_PMSLP;
    _hal_fsp.u32QspiBaseAddr = u32PMRegBaseAddr + BK_QSPI;
    _hal_fsp.u32CLK0BaseAddr = u32NonPMRegBaseAddr + BK_CHIP;//BK_CLK0;
    _hal_fsp.u32BDMABaseAddr = u32NonPMRegBaseAddr + BK_BDMA;
    _hal_fsp.u32RiuBaseAddr =  u32PMRegBaseAddr;

    
}

BOOL HAL_SPINAND_PLANE_HANDLER(U32 u32Addr)
{
    if(PLANE && (((u32Addr/BLOCK_SIZE)&0x1) == 1))
    {
        u32Addr =  QSPI_READ(REG_SPI_WRAP_VAL);
        u32Addr |= (1 << REG_SPI_WRAP_BIT_OFFSET);
        QSPI_WRITE(REG_SPI_WRAP_VAL, u32Addr);
    }
    else
    {
        u32Addr =  QSPI_READ(REG_SPI_WRAP_VAL);
        u32Addr &= ~(1 << REG_SPI_WRAP_BIT_OFFSET);
        QSPI_WRITE(REG_SPI_WRAP_VAL, u32Addr);
    }
    return TRUE;
}

U32 HAL_SPINAND_RFC(U32 u32Addr, U8 *pu8Data)
{
    U8  u8Addr = 0;
    U8  u8WbufIndex = 0;
    S8  s8Index;

    //DEBUG_SPINAND(E_SPINAND_DBGLV_DEBUG, printk("HAL_SPINAND_RFC : u32Addr = %lx \r\n",u32Addr));
    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP|ENABLE_THR_CMD|ENABLE_SEC_CMD));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    // FIRSET COMMAND PRELOAD
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_WREN);
    u8WbufIndex++;

    //SECOND COMMAND READ COMMAND + 3BYTE ADDRESS
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_PGRD);
    u8WbufIndex++;
    //seet Read Start Address
    for(s8Index = (SPI_NAND_ADDR_LEN - 1); s8Index >= 0 ; s8Index--)
    {
        u8Addr = (u32Addr >> (8 * s8Index) )& 0xFF;
        FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex),u8Addr);
        u8WbufIndex++;
    }
    
    //THIRD COMMAND GET FATURE CHECK CAHCHE READY
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_GF);
    u8WbufIndex++;
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_REG_STAT);
    u8WbufIndex++;

    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x241);

    FSP_WRITE(REG_FSP_READ_SIZE, 0x100);

    //Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR"RFC Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    _HAL_FSP_GetRData((pu8Data), 1);
    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    return _HAL_FSP_CHECK_SPINAND_DONE(pu8Data);
}

U32 HAL_SPINAND_PROGRAM_LOAD_DATA(U16 u16ColumnAddr, U16 u16DataSize, U8 *pu8Data, U8 *pu8_SpareBuf)
{
    U32 u32Ret = ERR_SPINAND_TIMEOUT;
    U8  u8Addr = 0, u8DataIndex =0;
    U16 u16RealLength = 0;
    U8  u8WbufIndex = 0;
    S8  s8Index;
    U32 u32WrteBuf = REG_FSP_WRITE_BUFF;
    U16 u16DataIndex;
    U8 *pu8Wdata;

    pu8Wdata = pu8Data;
    //DEBUG_SPINAND(E_SPINAND_DBGLV_DEBUG, printk("u16ColumnAddr %x u16DataSize %x Data %x \r\n", u16ColumnAddr, u16DataSize, *pu8Data));
    // Write Enable
    u32Ret = _HAL_SPINAND_WRITE_ENABLE();
    if(u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    u32Ret = HAL_SPINAND_WriteProtect(FALSE);
    if(u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP write Command
    // FIRSET COMMAND PRELOAD
    FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), SPI_NAND_CMD_PP);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x00);
    u8WbufIndex++;

    //PAGE Address        
    for(s8Index = (SPI_NAND_PAGE_ADDR_LEN - 1); s8Index >= 0 ; s8Index--)
    {
        u8Addr = (u16ColumnAddr >> (8 * s8Index) )& 0xFF;
        FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex),u8Addr);
        u8WbufIndex++;
    }

    

    for(u16DataIndex = 0; u16DataIndex < u16DataSize; u16DataIndex+= u16RealLength)
    {
        u16RealLength = u16DataSize  - u16DataIndex;
        if(u16RealLength >= SINGLE_WRITE_SIZE)
            u16RealLength = SINGLE_WRITE_SIZE - u8WbufIndex;
         
        //Write Data
        for(u8DataIndex = 0; u8DataIndex < u16RealLength; u8DataIndex++)
        {
            if((u16DataIndex + u8DataIndex)== PAGE_SIZE)
            {
                pu8Wdata = pu8_SpareBuf;
            }

            //printk("u8DataIndex %x u16RealLength %x \r\n",u8DataIndex, u16RealLength);
            FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), *pu8Wdata);
            u8WbufIndex++;
            pu8Wdata++;
            if(u8WbufIndex >= FSP_WRITE_BUF_JUMP_OFFSET)
            {
                u32WrteBuf = REG_FSP_WRITE_BUFF2;
                u8WbufIndex = 0;
            }
        }

        FSP_WRITE(REG_FSP_WRITE_SIZE, SINGLE_WRITE_SIZE);
        //Trigger FSP
        
        FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);
        QSPI_WRITE(REG_SPI_BURST_WRITE,REG_SPI_ENABLE_BURST);

        //Check FSP done flag
        if (_HAL_FSP_ChkWaitDone() == FALSE)
        {
            DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR"PL Wait FSP Done Time Out !!!!\r\n"));    
            return ERR_SPINAND_TIMEOUT;
        }

        u8WbufIndex = 0;
        u32WrteBuf = REG_FSP_WRITE_BUFF;
        //Clear FSP done flag
        FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    }
    QSPI_WRITE(REG_SPI_BURST_WRITE,REG_SPI_DISABLE_BURST);
    return _HAL_FSP_CHECK_SPINAND_DONE(NULL);

}

U32 HAL_SPINAND_READ_STATUS(U8 *pu8Status, U8 u8Addr)
{
    U8  u8WbufIndex = 0;

    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    // FIRSET COMMAND PRELOAD
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_GF);

    //SECOND COMMAND SET READ PARAMETER
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), u8Addr);
    // write buffer size
    FSP_WRITE(REG_FSP_WRITE_SIZE, 2);
    // read buffer size
    FSP_WRITE(REG_FSP_READ_SIZE, 1);

    //Trigger FSP
    FSP_WRITE_BYTE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR"RS Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_SUCCESS;
    }
    //Get Read Data
    _HAL_FSP_GetRData(pu8Status, 1);

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    return ERR_SPINAND_SUCCESS;
}

U32 HAL_SPINAND_BLOCKERASE(U32 u32_PageIdx)
{
    U8  u8WbufIndex = 0;
    S8  s8Index;
    U8  u8Addr;
    U32 u32Ret;
    U8  u8Status;

    u32Ret = _HAL_SPINAND_WRITE_ENABLE();
    if(u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    u32Ret = HAL_SPINAND_WriteProtect(FALSE);
    if(u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP|ENABLE_SEC_CMD|ENABLE_THR_CMD));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    // FIRSET COMMAND PRELOAD
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_WREN);
    u8WbufIndex++;

    //SECOND COMMAND SET Erase Command
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_BE);
    u8WbufIndex++;

    //seet erase Start Address
    for(s8Index = (SPI_NAND_ADDR_LEN - 1); s8Index >= 0 ; s8Index--)
    {
        u8Addr = (u32_PageIdx >> (8 * s8Index) )& 0xFF;
        FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex),u8Addr);
        u8WbufIndex++;
    }

    //THIRD COMMAND GET FATURE CHECK CAHCHE READY
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_GF);
    u8WbufIndex++;
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_REG_STAT);
    u8WbufIndex++;

    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x241);

    FSP_WRITE(REG_FSP_READ_SIZE,  0x100);

    //Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR"BE Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    u32Ret = _HAL_FSP_CHECK_SPINAND_DONE(&u8Status);
    if(u32Ret == ERR_SPINAND_SUCCESS)
        if(u8Status & E_FAIL)
            u32Ret = ERR_SPINAND_E_FAIL;

    return u32Ret;
}

U32 HAL_SPINAND_Write(U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf)
{
    U32 u32Ret;
    U8  u8Addr = 0;
    U8  u8WbufIndex = 0;
    S8  s8Index;
    U16 u16DataSize;
    U8  u8Status;
    U16 u16ColumnAddr = 0;

    //calculate write data size
    u16DataSize = SPARE_SIZE + PAGE_SIZE;

    //DEBUG_SPINAND(E_SPINAND_DBGLV_DEBUG, printk("addr %lx u8Data %x u32PageIndex %lx\r\n", (U32)u8Data, *u8Data, u32_PageIdx));

    if(PLANE && (((u32_PageIdx/BLOCK_SIZE)&0x1) == 1))
        u16ColumnAddr = (1<<12); // plane select for MICRON
    u32Ret = HAL_SPINAND_PROGRAM_LOAD_DATA(u16ColumnAddr, u16DataSize, u8Data, pu8_SpareBuf);
    if(u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP|ENABLE_SEC_CMD));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // FIRST COMMAND PAGE PROGRAM EXECUTE
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_PE);
    u8WbufIndex++;

    //seet Write Start Address
    for(s8Index = (SPI_NAND_ADDR_LEN - 1); s8Index >= 0 ; s8Index--)
    {
        u8Addr = (u32_PageIdx >> (8 * s8Index) )& 0xFF;
        FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex),u8Addr);
        u8WbufIndex++;
    }
    //SECOND COMMAND GET FATURE CHECK CAHCHE READY
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_GF);
    u8WbufIndex++;
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_REG_STAT);
    u8WbufIndex++;

    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x024);

    FSP_WRITE(REG_FSP_READ_SIZE, 0x010);
    //Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR"W Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    u32Ret = _HAL_FSP_CHECK_SPINAND_DONE(&u8Status);
    if(u32Ret == ERR_SPINAND_SUCCESS)
        if(u8Status & P_FAIL)
            u32Ret = ERR_SPINAND_W_FAIL;

    return u32Ret;

}

U32 HAL_SPINAND_Read (U32 u32Addr, U32 u32DataSize, U8 *pu8Data)
{
    U16 u16Addr = u32Addr & 0xFFFF;
    U32 ret;
    if(_gbRIURead)
    {
        ret = _HAL_SPINAND_RIU_READ(u16Addr, u32DataSize, pu8Data);
    }
    else
    {
        // config SPI waveform for BDMA
        if(PLANE)
        {
            QSPI_WRITE(REG_SPI_FUNC_SET, (REG_SPI_ADDR2_EN|REG_SPI_DUMMY_EN|REG_SPI_WRAP_EN));
        }
        else
        {
            QSPI_WRITE(REG_SPI_FUNC_SET, (REG_SPI_ADDR2_EN|REG_SPI_DUMMY_EN));
        }
        //Set dummy cycle
        HAL_SPINAND_SetMode(E_SPINAND_SINGLE_MODE);
        //QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_SINGAL);
        ret = _HAL_SPINAND_BDMA_READ(u16Addr, u32DataSize, pu8Data);
        if(ret != ERR_SPINAND_SUCCESS)
            printk(KERN_ERR"R Wait BDMA Done Time Out !!!!\r\n");    
    }
    return ret;
}

U32 HAL_SPINAND_ReadID(U32 u32DataSize, U8 *pu8Data)
{
    U16 u16Index =0;
    U32 u32RealLength = 0;

    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_RDID);
    //Set Start Address
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 1, 0x00);

    FSP_WRITE(REG_FSP_WRITE_SIZE, 2);

    //Set Write & Read Length
    for(u16Index = 0; u16Index < u32DataSize; u16Index += u32RealLength)
    {
        if(u32DataSize > (MAX_READ_BUF_CNT))
        {
            FSP_WRITE(REG_FSP_READ_SIZE, MAX_READ_BUF_CNT);
            u32RealLength = MAX_READ_BUF_CNT;
        }
        else
        {
            FSP_WRITE(REG_FSP_READ_SIZE, u32DataSize);
            u32RealLength = u32DataSize;
        }

        //Trigger FSP
        FSP_WRITE_BYTE(REG_FSP_TRIGGER, TRIGGER_FSP);

        //Check FSP done flag
        if (_HAL_FSP_ChkWaitDone() == FALSE)
        {
            DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR"RID Wait FSP Done Time Out !!!!\r\n"));
            return ERR_SPINAND_TIMEOUT;
        }
        //Get Read Data
        _HAL_FSP_GetRData((pu8Data + u16Index), u32RealLength);

        //Clear FSP done flag
        FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    }
    return ERR_SPINAND_SUCCESS;

}

U32 HAL_SPINAND_WriteProtect(BOOL bEnable)
{
    U8 u8WbufIndex = 0;
    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_SF);
    u8WbufIndex++;
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + u8WbufIndex, SPI_NAND_REG_PROT);
    u8WbufIndex++;
    if(bEnable)
        FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + u8WbufIndex,0x38);
    else
        FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + u8WbufIndex,0x00);
      
    FSP_WRITE(REG_FSP_WRITE_SIZE, 3);
    // read buffer size
    FSP_WRITE(REG_FSP_READ_SIZE, 0);
    //Trigger FSP
    FSP_WRITE_BYTE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR"WP Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    return ERR_SPINAND_SUCCESS;
}

U32 HAL_SPINAND_SetMode(SPINAND_MODE eMode)
{
    switch(eMode)
    {
    case E_SPINAND_SINGLE_MODE:
        QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_SINGAL);
        break;
    case E_SPINAND_FAST_MODE:
        QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_SINGAL);
        QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_FAST_READ);
        break;
    case E_SPINAND_DUAL_MODE:
        QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_SINGAL);
        QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_CMD_3B);
        break;
    case E_SPINAND_DUAL_MODE_IO:
        QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_DUAL);
        QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_CMD_BB);
        break;
    case E_SPINAND_QUAD_MODE:
        QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_SINGAL);
        QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_CMD_6B);
        break;
    case E_SPINAND_QUAD_MODE_IO:
        QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_QUAD);
        QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_CMD_EB);
        break;
    }
    return ERR_SPINAND_SUCCESS;
}

