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
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include "drvSERFLASH.h"
#include <linux/vmalloc.h>
#include <asm/io.h>


#include <linux/jiffies.h>

// Common Definition
//#include "MsCommon.h"
//#include "MsIRQ.h"
//#include "MsOS.h"
//#include "drvMMIO.h"

// Internal Definition
#include "regSERFLASH.h"
#include "halSERFLASH.h"
#include "MsTypes.h"

// !!! Uranus Serial Flash Notes: !!!
//  - The clock of DMA & Read via XIU operations must be < 3*CPU clock
//  - The clock of DMA & Read via XIU operations are determined by only REG_ISP_CLK_SRC; other operations by REG_ISP_CLK_SRC only
//  - DMA program can't run on DRAM, but in flash ONLY
//  - DMA from SPI to DRAM => size/DRAM start/DRAM end must be 8-B aligned


//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

#define XIUREAD_MODE                0
#define FSP	0
#define READ_BYTE(_reg)             (*(volatile MS_U8*)(_reg))
#define READ_WORD(_reg)             (*(volatile MS_U16*)(_reg))
#define READ_LONG(_reg)             (*(volatile MS_U32*)(_reg))
#define WRITE_BYTE(_reg, _val)      { (*((volatile MS_U8*)(_reg))) = (MS_U8)(_val); }
#define WRITE_WORD(_reg, _val)      { (*((volatile MS_U16*)(_reg))) = (MS_U16)(_val); }
#define WRITE_LONG(_reg, _val)      { (*((volatile MS_U32*)(_reg))) = (MS_U32)(_val); }

#define WRITE_WORD_MASK(_reg, _val, _mask)  { (*((volatile MS_U16*)(_reg))) = ((*((volatile MS_U16*)(_reg))) & ~(_mask)) | ((MS_U16)(_val) & (_mask)); }


// XIU_ADDR
// #define SFSH_XIU_REG32(addr)                (*((volatile MS_U32 *)(_hal_isp.u32XiuBaseAddr + ((addr)<<2))))

#define SFSH_XIU_READ32(addr)               (*((volatile MS_U32 *)(_hal_isp.u32XiuBaseAddr + ((addr)<<2)))) // TODO: check AEON 32 byte access order issue


// ISP_CMD
// #define ISP_REG16(addr)                     (*((volatile MS_U16 *)(_hal_isp.u32IspBaseAddr + ((addr)<<2))))

#define ISP_READ(addr)                      READ_WORD(_hal_isp.u32IspBaseAddr + ((addr)<<2))
#define ISP_WRITE(addr, val)                WRITE_WORD(_hal_isp.u32IspBaseAddr + ((addr)<<2), (val))
#define ISP_WRITE_MASK(addr, val, mask)     WRITE_WORD_MASK(_hal_isp.u32IspBaseAddr + ((addr)<<2), (val), (mask))

#define SPI_FLASH_CMD(u8FLASHCmd)           ISP_WRITE(REG_ISP_SPI_COMMAND, (MS_U8)u8FLASHCmd)
#define SPI_WRITE_DATA(u8Data)              ISP_WRITE(REG_ISP_SPI_WDATA, (MS_U8)u8Data)
#define SPI_READ_DATA()                     READ_BYTE(_hal_isp.u32IspBaseAddr + ((REG_ISP_SPI_RDATA)<<2))

#define MHEG5_READ(addr)                    READ_WORD(_hal_isp.u32Mheg5BaseAddr + ((addr)<<2))
#define MHEG5_WRITE(addr, val)              WRITE_WORD((_hal_isp.u32Mheg5BaseAddr + (addr << 2)), (val))
#define MHEG5_WRITE_MASK(addr, val, mask)   WRITE_WORD_MASK(_hal_isp.u32Mheg5BaseAddr + ((addr)<<2), (val), (mask))

// PIU_DMA
#define PIU_READ(addr)                      READ_WORD(_hal_isp.u32PiuBaseAddr + ((addr)<<2))
#define PIU_WRITE(addr, val)                WRITE_WORD(_hal_isp.u32PiuBaseAddr + ((addr)<<2), (val))
#define PIU_WRITE_MASK(addr, val, mask)     WRITE_WORD_MASK(_hal_isp.u32PiuBaseAddr + ((addr)<<2), (val), (mask))

// PM_SLEEP CMD.
#define PM_READ(addr)                      READ_WORD(_hal_isp.u32PMBaseAddr+ ((addr)<<2))
#define PM_WRITE(addr, val)                WRITE_WORD(_hal_isp.u32PMBaseAddr+ ((addr)<<2), (val))
#define PM_WRITE_MASK(addr, val, mask)     WRITE_WORD_MASK(_hal_isp.u32PMBaseAddr+ ((addr)<<2), (val), (mask))

// CLK_GEN
#define CLK_READ(addr)                     READ_WORD(_hal_isp.u32CLK0BaseAddr + ((addr)<<2))
#define CLK_WRITE(addr, val)               WRITE_WORD(_hal_isp.u32CLK0BaseAddr + ((addr)<<2), (val))
#define CLK_WRITE_MASK(addr, val, mask)    WRITE_WORD_MASK(_hal_isp.u32CLK0BaseAddr + ((addr)<<2), (val), (mask))

//MS_U32<->MS_U16
#define LOU16(u32Val)   ((MS_U16)(u32Val))
#define HIU16(u32Val)   ((MS_U16)((u32Val) >> 16))

//serial flash mutex wait time
#define SERFLASH_MUTEX_WAIT_TIME    3000

// Time-out system
#if !defined (MSOS_TYPE_NOS) && !defined(MSOS_TYPE_LINUX)
    #define SERFLASH_SAFETY_FACTOR      10
    #define SER_FLASH_TIME(_stamp, _msec)    { (_stamp) = MsOS_GetSystemTime() + (_msec); }
    #define SER_FLASH_EXPIRE(_stamp)         ( (MsOS_GetSystemTime() > (_stamp)) ? 1 : 0 )

#else // defined (MSOS_TYPE_NOS)
    #define SERFLASH_SAFETY_FACTOR      3000
    #define SER_FLASH_TIME(_stamp)                 (do_gettimeofday(&_stamp))
    #define SER_FLASH_EXPIRE(_stamp,_msec)         (_Hal_GetMsTime(_stamp, _msec))
#endif

#define CHK_NUM_WAITDONE     2000
#define SINGLE_WRITE_LENGTH  4
#define SINGLE_READ_LENGTH   8

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef struct
{
    MS_U32  u32XiuBaseAddr;     // REG_SFSH_XIU_BASE
    MS_U32  u32Mheg5BaseAddr;
    MS_U32  u32IspBaseAddr;     // REG_ISP_BASE
    MS_U32  u32PiuBaseAddr;     // REG_PIU_BASE
    MS_U32  u32PMBaseAddr;      // REG_PM_BASE
    MS_U32  u32CLK0BaseAddr;    // REG_PM_BASE
    MS_U32  u32BdmaBaseAddr;    // for supernova.lite
    MS_U32  u32RiuBaseAddr;
} hal_isp_t;

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
hal_SERFLASH_t _hal_SERFLASH;
MS_U8 _u8SERFLASHDbgLevel;
MS_BOOL _bXIUMode = 0;      // default XIU mode, set 0 to RIU mode
MS_BOOL bDetect = FALSE;    // initial flasg : true and false
MS_BOOL _bIBPM = FALSE;     // Individual Block Protect mode : true and false
MS_BOOL _bWPPullHigh = 0;   // WP pin pull high or can control info
extern void *high_memory;
//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static MS_S32 _s32SERFLASH_Mutex;
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

static hal_isp_t _hal_isp =
{
    .u32XiuBaseAddr = BASEADDR_XIU,
    .u32Mheg5BaseAddr = BASEADDR_RIU + BK_MHEG5,
    .u32IspBaseAddr = BASEADDR_RIU + BK_ISP,
    .u32PiuBaseAddr = BASEADDR_RIU + BK_PIU,
    .u32PMBaseAddr = BASEADDR_RIU + BK_PMSLP,
    .u32CLK0BaseAddr = BASEADDR_RIU + BK_CLK0,
    .u32BdmaBaseAddr = BASEADDR_NonPMBankRIU + BK_BDMA,
};

// For linux, thread sync is handled by mtd. So, these functions are empty.
#define MSOS_PROCESS_PRIVATE    0x00000000
#define MSOS_PROCESS_SHARED     0x00000001
static spinlock_t _gtSpiLock;
static MS_U32 _gtSpiFlag; 
/// Suspend type
typedef enum
{
    E_MSOS_PRIORITY,            ///< Priority-order suspension
    E_MSOS_FIFO,                ///< FIFO-order suspension
} MsOSAttribute;

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static void _HAL_SPI_Rest(void);
static void _HAL_ISP_Enable(void);
static void _HAL_ISP_Disable(void);
static void _HAL_ISP_2XMode(MS_BOOL bEnable);
static MS_BOOL _HAL_SERFLASH_WaitWriteCmdRdy(void);
#if(FSP == 0)
static MS_BOOL _HAL_SERFLASH_WaitWriteDataRdy(void);
#endif
static MS_BOOL _HAL_SERFLASH_WaitReadDataRdy(void);
static MS_BOOL _HAL_SERFLASH_WaitWriteDone(void);
#if(FSP == 0)
static MS_BOOL _HAL_SERFLASH_CheckWriteDone(void);
#endif
static MS_BOOL _HAL_SERFLASH_XIURead(MS_U32 u32Addr,MS_U32 u32Size,MS_U8 * pu8Data);
#if(FSP == 0)
static MS_BOOL _HAL_SERFLASH_RIURead(MS_U32 u32Addr,MS_U32 u32Size,MS_U8 * pu8Data);
#endif
static void _HAL_SERFLASH_ActiveFlash_Set_HW_WP(MS_BOOL bEnable);

MS_BOOL MsOS_In_Interrupt (void)
{
    return FALSE;
}

MS_S32 MsOS_CreateMutex ( MsOSAttribute eAttribute, char *pMutexName, MS_U32 u32Flag)
{
    spin_lock_init(&_gtSpiLock);
    return 1;
}

MS_BOOL MsOS_DeleteMutex (MS_S32 s32MutexId)
{
    return TRUE;
}

MS_BOOL MsOS_ObtainMutex (MS_S32 s32MutexId, MS_U32 u32WaitMs)
{
    spin_lock_irq(&_gtSpiLock);
    return TRUE;
}

MS_BOOL MsOS_ReleaseMutex (MS_S32 s32MutexId)
{
    spin_unlock_irq(&_gtSpiLock);
    return TRUE;
}
//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------
BDMA_Result MDrv_BDMA_CopyHnd(MS_PHYADDR u32SrcAddr, MS_PHYADDR u32DstAddr, MS_U32 u32Len, BDMA_CpyType eCpyType, MS_U8 u8OpCfg)
{
    return -1;
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
// Software reset spi_burst
// @return TRUE : succeed
// @return FALSE : fail
// @note : If no spi reset, it may cause BDMA fail.
//-------------------------------------------------------------------------------------------------
static void _HAL_SPI_Rest(void)
{
    ISP_WRITE_MASK(REG_ISP_CHIP_RST, SFSH_CHIP_RESET, SFSH_CHIP_RESET_MASK);
    ISP_WRITE_MASK(REG_ISP_CHIP_RST, SFSH_CHIP_NOTRESET, SFSH_CHIP_RESET_MASK);

    // Call the callback function to switch back the chip selection.
    if(McuChipSelectCB != NULL )
    {
        (*McuChipSelectCB)();
    }
}

//-------------------------------------------------------------------------------------------------
// Enable RIU ISP engine
// @return TRUE : succeed
// @return FALSE : fail
// @note : If Enable ISP engine, the XIU mode does not work
//-------------------------------------------------------------------------------------------------
static void _HAL_ISP_Enable(void)
{
    ISP_WRITE(REG_ISP_PASSWORD, 0xAAAA);
}

//-------------------------------------------------------------------------------------------------
// Disable RIU ISP engine
// @return TRUE : succeed
// @return FALSE : fail
// @note : If Disable ISP engine, the XIU mode works
//-------------------------------------------------------------------------------------------------
static void _HAL_ISP_Disable(void)
{
    ISP_WRITE(REG_ISP_PASSWORD, 0x5555);
    _HAL_SPI_Rest();
}

static void _HAL_BDMA_READ(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data)
{
    MS_U32 u32Addr1;
    MS_U16 u16data;
    struct timeval time_st;
    MS_BOOL bRet;

    if((MS_U32)pu8Data < (MS_U32)high_memory)
    {
        //Clean L2 by range 
        _dma_cache_wback_inv((MS_U32)pu8Data, u32Size);
        //_dma_cache_wback(u32Addr,u32Size);
        u32Addr1 = virt_to_phys((MS_U32)pu8Data);
        //u32Addr  = virt_to_phys(u32Addr);
    }
    else
    {
        printk("high memory need alloce low memory to get PA\n");
    }

    //Set source and destination path
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x0<<2)), 0x00);
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x12<<2)), 0X3035);

    // Set start address
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x14<<2)), (u32Addr & 0x0000FFFF));
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x15<<2)), (u32Addr >> 16));

    // Set end address
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x16<<2)), (u32Addr1 & 0x0000FFFF));
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x17<<2)), (u32Addr1 >> 16));
    // Set Size

    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x18<<2)), (u32Size & 0x0000FFFF));
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x19<<2)), (u32Size >> 16));

    // Trigger
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x10<<2)), 1);

    SER_FLASH_TIME(time_st); 
    do
    {
        
        //check done
        u16data = READ_WORD(_hal_isp.u32BdmaBaseAddr + ((0x11)<<2));
        if(u16data & 8)
        {
            //clear done
            WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x11<<2)), 8);
            bRet = TRUE;
            break;
        } 
    } while(!SER_FLASH_EXPIRE(time_st, SERFLASH_SAFETY_FACTOR));

    if(bRet == FALSE)
    {
        printk("Wait for BDMA Done fails!\n");
    }
    //WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x0<<2)), 0x10);
}

//-------------------------------------------------------------------------------------------------
// Enable/Disable address and data dual mode (SPI command is 0xBB)
// @return TRUE : succeed
// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
static void _HAL_ISP_2XMode(MS_BOOL bEnable)
{
    if(bEnable) // on 2Xmode
    {
        ISP_WRITE_MASK(REG_ISP_SPI_MODE,SFSH_CHIP_2XREAD_ENABLE,SFSH_CHIP_2XREAD_MASK);
    }
    else        // off 2Xmode
    {
        ISP_WRITE_MASK(REG_ISP_SPI_MODE,SFSH_CHIP_2XREAD_DISABLE,SFSH_CHIP_2XREAD_MASK);
    }
}

//-------------------------------------------------------------------------------------------------
// Wait for SPI Write Cmd Ready
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static MS_BOOL _HAL_SERFLASH_WaitWriteCmdRdy(void)
{
    MS_BOOL bRet = FALSE;


#if (defined (MSOS_TYPE_LINUX) || defined (MSOS_TYPE_ECOS))
    struct timeval time_st;
    SER_FLASH_TIME(time_st);
#else
    MS_U32 u32Timer;
    SER_FLASH_TIME(u32Timer, SERFLASH_SAFETY_FACTOR);
#endif
    do
    {
        if ( (ISP_READ(REG_ISP_SPI_WR_CMDRDY) & ISP_SPI_WR_CMDRDY_MASK) == ISP_SPI_WR_CMDRDY )
        {
            bRet = TRUE;
            break;
        }
#if (defined (MSOS_TYPE_LINUX) || defined (MSOS_TYPE_ECOS))
    } while (!SER_FLASH_EXPIRE(time_st, SERFLASH_SAFETY_FACTOR * 30));
#else
     } while (!SER_FLASH_EXPIRE(u32Timer));
#endif
    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("Wait for SPI Write Cmd Ready fails!\n"));
    }

    return bRet;
}

#if(FSP == 0)

//-------------------------------------------------------------------------------------------------
// Wait for SPI Write Data Ready
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static MS_BOOL _HAL_SERFLASH_WaitWriteDataRdy(void)
{
    MS_BOOL bRet = FALSE;


#if (defined (MSOS_TYPE_LINUX) || defined (MSOS_TYPE_ECOS))
    struct timeval time_st;
    SER_FLASH_TIME(time_st);
#else
    MS_U32 u32Timer;
    SER_FLASH_TIME(u32Timer, SERFLASH_SAFETY_FACTOR);
#endif
    do
    {
        if ( (ISP_READ(REG_ISP_SPI_WR_DATARDY) & ISP_SPI_WR_DATARDY_MASK) == ISP_SPI_WR_DATARDY )
        {
            bRet = TRUE;
            break;
        }
#if (defined (MSOS_TYPE_LINUX) || defined (MSOS_TYPE_ECOS))
    } while (!SER_FLASH_EXPIRE(time_st, SERFLASH_SAFETY_FACTOR * 30));
#else
    } while (!SER_FLASH_EXPIRE(u32Timer));
#endif


    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("Wait for SPI Write Data Ready fails!\n"));
    }

    return bRet;
}
#endif

//-------------------------------------------------------------------------------------------------
// Wait for SPI Read Data Ready
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static MS_BOOL _HAL_SERFLASH_WaitReadDataRdy(void)
{
    MS_BOOL bRet = FALSE;

#if (defined (MSOS_TYPE_LINUX) || defined (MSOS_TYPE_ECOS))
    struct timeval time_st;
    SER_FLASH_TIME(time_st);
#else
    MS_U32 u32Timer;
    SER_FLASH_TIME(u32Timer, SERFLASH_SAFETY_FACTOR);
#endif
    do
    {
        if ( (ISP_READ(REG_ISP_SPI_RD_DATARDY) & ISP_SPI_RD_DATARDY_MASK) == ISP_SPI_RD_DATARDY )
        {
            bRet = TRUE;
            break;
        }
#if (defined (MSOS_TYPE_LINUX) || defined (MSOS_TYPE_ECOS))
    } while (!SER_FLASH_EXPIRE(time_st, SERFLASH_SAFETY_FACTOR * 30));
#else
    } while (!SER_FLASH_EXPIRE(u32Timer));
#endif


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
static MS_BOOL _HAL_SERFLASH_WaitWriteDone(void)
{
    MS_BOOL bRet = FALSE;


#if (defined (MSOS_TYPE_LINUX) || defined (MSOS_TYPE_ECOS))
    struct timeval time_st;
    SER_FLASH_TIME(time_st);
#else
    MS_U32 u32Timer;
    SER_FLASH_TIME(u32Timer, SERFLASH_SAFETY_FACTOR*SERFLASH_MAX_CHIP_WR_DONE_TIMEOUT);
#endif

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        return FALSE;
    }

    do
    {

        ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_RDSR); // RDSR

        ISP_WRITE(REG_ISP_SPI_RDREQ, ISP_SPI_RDREQ); // SPI read request

        if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
        {
            break;
        }

        if ( (ISP_READ(REG_ISP_SPI_RDATA) & SF_SR_WIP_MASK) == 0 ) // WIP = 0 write done
        {
            bRet = TRUE;
            break;
        }
#if (defined (MSOS_TYPE_LINUX) || defined (MSOS_TYPE_ECOS))
    } while (!SER_FLASH_EXPIRE(time_st, SERFLASH_SAFETY_FACTOR * 100));
#else
    } while (!SER_FLASH_EXPIRE(u32Timer));
#endif


    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("Wait for Write to be done fails!\n"));
    }

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    return bRet;
}

#if(FSP == 0)

//-------------------------------------------------------------------------------------------------
// Check Write/Erase to be done
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static MS_BOOL _HAL_SERFLASH_CheckWriteDone(void)
{
    MS_BOOL bRet = FALSE;

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto _HAL_SERFLASH_CheckWriteDone_return;
    }

    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_RDSR); // RDSR

    ISP_WRITE(REG_ISP_SPI_RDREQ, ISP_SPI_RDREQ); // SPI read request

    if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
    {
        goto _HAL_SERFLASH_CheckWriteDone_return;
    }

    if ( (ISP_READ(REG_ISP_SPI_RDATA) & SF_SR_WIP_MASK) == 0 ) // WIP = 0 write done
    {
        bRet = TRUE;
    }

_HAL_SERFLASH_CheckWriteDone_return:

    return bRet;
}
#endif

//-------------------------------------------------------------------------------------------------
/// Enable/Disable flash HW WP
/// @param  bEnable \b IN: enable or disable HW protection
//-------------------------------------------------------------------------------------------------
static void _HAL_SERFLASH_ActiveFlash_Set_HW_WP(MS_BOOL bEnable)
{
    extern void msFlash_ActiveFlash_Set_HW_WP(MS_BOOL bEnable) __attribute__ ((weak));

    if (msFlash_ActiveFlash_Set_HW_WP != NULL)
    {
        msFlash_ActiveFlash_Set_HW_WP(bEnable);
    }
    else
    {
        /*if(FlashSetHWWPCB != NULL )
        {
           (*FlashSetHWWPCB)(bEnable);
        }*/   


        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_NOTICE, printk("msFlash_ActiveFlash_Set_HW_WP() is not defined in this system\n"));
        // ASSERT(msFlash_ActiveFlash_Set_HW_WP != NULL);
    }

    return;
}

#if defined (MCU_AEON)
//Aeon SPI Address is 64K bytes windows
static MS_BOOL _HAL_SetAeon_SPIMappingAddr(MS_U32 u32addr)
{
    MS_U16 u16MHEGAddr = (MS_U16)((_hal_isp.u32XiuBaseAddr + u32addr) >> 16);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, 0x%08X)\n", __FUNCTION__, (int)u32addr, (int)u16MHEGAddr));
    MHEG5_WRITE(REG_SPI_BASE, u16MHEGAddr);

    return TRUE;
}
#endif

#if(FSP == 0)

static MS_BOOL _HAL_SERFLASH_RIURead(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data)
{
    MS_BOOL bRet = FALSE;
    //MS_U8 *pu8ReadBuf = pu8Data;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, %d, %p)\n", __FUNCTION__, (int)u32Addr, (int)u32Size, pu8Data));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );

    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }
#if 0

    _HAL_ISP_Enable();

    do{
        if(_HAL_SERFLASH_WaitWriteDone()) break;
        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis
    }while(1);

    ISP_WRITE(REG_ISP_SPI_ADDR_L, LOU16(u32Addr));
    ISP_WRITE(REG_ISP_SPI_ADDR_H, HIU16(u32Addr));

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_Read_return;
    }

    SPI_FLASH_CMD(ISP_SPI_CMD_READ);// READ // 0x0B fast Read : HW doesn't support now

    for ( u32I = 0; u32I < u32Size; u32I++ )
    {
        ISP_WRITE(REG_ISP_SPI_RDREQ, ISP_SPI_RDREQ); // SPI read request

        if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_Read_return;
        }

            *(pu8Data + u32I)  = (MS_U8)SPI_READ_DATA();
    }
#endif
   //--- Flush OCP memory --------
   // MsOS_FlushMemory(); 

    bRet = TRUE;

HAL_SERFLASH_Read_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

#if defined (MCU_AEON)
    //restore default value
    _HAL_SetAeon_SPIMappingAddr(0);
#endif
    _HAL_BDMA_READ(u32Addr, u32Size, pu8Data);

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    return bRet;
}
#endif

static MS_BOOL _HAL_SERFLASH_XIURead(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32I;
    MS_U8 *pu8ReadBuf = pu8Data;
    MS_U32 u32Value, u32AliSize;
    MS_U32 u32AliAddr, u32RemSize = u32Size;
    MS_U32 u32pos;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, %d, %p)\n", __FUNCTION__, (int)u32Addr, (int)u32Size, pu8Data));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );

    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    do{
        if(_HAL_SERFLASH_WaitWriteDone()) break;
        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis
    }while(1);

    _HAL_ISP_Disable();

    // 4-BYTE Aligment for 32 bit CPU Aligment
    u32AliAddr = (u32Addr & 0xFFFFFFFC);
    u32pos = u32AliAddr >> 2;

#if defined (MCU_AEON)
    //write SPI mapping address
    _HAL_SetAeon_SPIMappingAddr(u32AliAddr);
#endif

    //---- Read first data for not aligment address ------
    if(u32AliAddr < u32Addr)
    {
        u32Value = SFSH_XIU_READ32(u32pos);
        u32pos++;
        for(u32I = 0; (u32I < 4) && (u32RemSize > 0); u32I++)
        {
            if(u32AliAddr >= u32Addr)
            {
                *pu8ReadBuf++ = (MS_U8)(u32Value & 0xFF);
                u32RemSize--;
            }
            u32Value >>= 8;
            u32AliAddr++;
        }
    }
    //----Read datum for aligment address------
    u32AliSize = (u32RemSize & 0xFFFFFFFC);
    for( u32I = 0; u32I < u32AliSize; u32I += 4)
    {
#if defined (MCU_AEON)
            if((u32AliAddr & 0xFFFF) == 0)
                _HAL_SetAeon_SPIMappingAddr(u32AliAddr);
#endif

            // only indirect mode
            u32Value = SFSH_XIU_READ32(u32pos);

            *pu8ReadBuf++ = ( u32Value >> 0) & 0xFF;
            *pu8ReadBuf++ = ( u32Value >> 8) & 0xFF;
            *pu8ReadBuf++ = ( u32Value >> 16)& 0xFF;
            *pu8ReadBuf++ = ( u32Value >> 24)& 0xFF;

            u32pos++;
            u32AliAddr += 4;
        }

    //--- Read remain datum --------
    if(u32RemSize > u32AliSize)
    {
#if defined (MCU_AEON)
            if((u32AliAddr & 0xFFFF) == 0)
                _HAL_SetAeon_SPIMappingAddr(u32AliAddr);
#endif
            u32Value = SFSH_XIU_READ32(u32pos);
        }
        while(u32RemSize > u32AliSize)
        {
            *pu8ReadBuf++ = (u32Value & 0xFF);
            u32Value >>= 8;
            u32AliSize++;
        }
    //--- Flush OCP memory --------
    //MsOS_FlushMemory(); 


        bRet = TRUE;

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

#if defined (MCU_AEON)
    //restore default value
    _HAL_SetAeon_SPIMappingAddr(0);
#endif
     MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    return bRet;
}


// FSP related static function
#if(FSP	== 1)

#if 0
static MS_BOOL _HAL_FSP_WaitForDone(void)
{
    MS_U32 start_time = MsOS_GetSystemTime();
    MS_U16 u16Status;

#define DEFAULT_FSP_TIMEOUT	2000

    ISP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_FIRE_MASK);	//Start FSP

    while(1)//Polling Done
    {
        #if 1
        if( (MsOS_GetSystemTime() - start_time) >=  DEFAULT_FSP_TIMEOUT )
        {
            printk("%s() error : Time out!!!\n", __FUNCTION__);

            return FALSE;
            break;
        }
        #endif

        u16Status = ISP_READ(REG_FSP_DONE_FLAG);

        if( (u16Status & FSP_DONE_FLAG) == FSP_DONE_FLAG )
        {
            ISP_WRITE_MASK(REG_FSP_CLR_FLAG,FSP_CLR_FLAG,FSP_CLR_FLAG_MASK);//Clear Done Flag

            break;
        }
    }

    return TRUE;
}
#else
static MS_BOOL _HAL_FSP_WaitForDone(void)
{
    MS_U32 u32Timer;
    MS_U16 u16Status;
    MS_BOOL bRet = FALSE;

#if (defined (MSOS_TYPE_LINUX) || defined (MSOS_TYPE_ECOS))
    SER_FLASH_TIME(u32Timer, SERFLASH_SAFETY_FACTOR*SERFLASH_MAX_CHIP_WR_DONE_TIMEOUT*1000);
#else
    SER_FLASH_TIME(u32Timer, SERFLASH_SAFETY_FACTOR*SERFLASH_MAX_CHIP_WR_DONE_TIMEOUT);
#endif

    ISP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_FIRE_MASK); //Start FSP

    do//Polling Done
    {
        u16Status = ISP_READ(REG_FSP_DONE_FLAG);

        if( (u16Status & FSP_DONE_FLAG) == FSP_DONE_FLAG )
        {
            ISP_WRITE_MASK(REG_FSP_CLR_FLAG,FSP_CLR_FLAG,FSP_CLR_FLAG_MASK);//Clear Done Flag
            bRet = TRUE;

            break;
        }
    }while (!SER_FLASH_EXPIRE(u32Timer));

    return bRet;

}


#endif

static MS_U8 _HAL_SERFLASH_ReadStatusByFSP(void)
{
    ISP_WRITE(REG_FSP_WD0,ISP_SPI_CMD_RDSR);

    ISP_WRITE_MASK(REG_FSP_WD1,FSP_WD1(0x0),REG_FSP_WD1_MASK);
    ISP_WRITE_MASK(REG_FSP_WD2,FSP_WD2(0x0),REG_FSP_WD2_MASK);
    ISP_WRITE_MASK(REG_FSP_WD3,FSP_WD3(0x0),REG_FSP_WD3_MASK);

    ISP_WRITE_MASK(REG_FSP_WD4,FSP_WD4(0x0),REG_FSP_WD4_MASK);
    ISP_WRITE_MASK(REG_FSP_WD5,FSP_WD5(0x0),REG_FSP_WD5_MASK);
    ISP_WRITE_MASK(REG_FSP_WD6,FSP_WD6(0x0),REG_FSP_WD6_MASK);
    ISP_WRITE_MASK(REG_FSP_WD7,FSP_WD7(0x0),REG_FSP_WD7_MASK);
    ISP_WRITE_MASK(REG_FSP_WD8,FSP_WD8(0x0),REG_FSP_WD8_MASK);
    ISP_WRITE_MASK(REG_FSP_WD9,FSP_WD9(0x0),REG_FSP_WD9_MASK);

    ISP_WRITE(REG_FSP_WBF_SIZE,0);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE1(0),REG_FSP_WBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE2(0),REG_FSP_WBF_SIZE2_MASK);

    ISP_WRITE(REG_FSP_RBF_SIZE,0);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE0(1),REG_FSP_RBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE2(0),REG_FSP_RBF_SIZE2_MASK);

    ISP_WRITE(REG_FSP_CTRL,0);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_RST(0),REG_FSP_CTRL0_RST_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_ACS(1),REG_FSP_CTRL1_ACS_MASK);


    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_RDSR_1CMD,REG_FSP_CTRL1_RDSR_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_ACS(1),REG_FSP_CTRL1_ACS_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_FSP(1),REG_FSP_CTRL0_FSP_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_RST(1),REG_FSP_CTRL0_RST_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_ACE(1),REG_FSP_CTRL0_ACE_MASK);

    _HAL_FSP_WaitForDone();

    return (MS_U8)ISP_READ(REG_FSP_RD0);
}

static MS_BOOL _HAL_FSP_SERFLASH_WaitWriteDone(void)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32Timer;

#if (defined (MSOS_TYPE_LINUX) || defined (MSOS_TYPE_ECOS))
    SER_FLASH_TIME(u32Timer, SERFLASH_SAFETY_FACTOR*SERFLASH_MAX_CHIP_WR_DONE_TIMEOUT*1000);
#else
    SER_FLASH_TIME(u32Timer, SERFLASH_SAFETY_FACTOR*SERFLASH_MAX_CHIP_WR_DONE_TIMEOUT);
#endif

    do
    {
        if ((_HAL_SERFLASH_ReadStatusByFSP() & SF_SR_WIP_MASK) == 0) // WIP = 0 write done
        {
            bRet = TRUE;
            break;
        }

    } while (!SER_FLASH_EXPIRE(u32Timer));

    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("Wait for Write to be done fails!\n"));
    }

    return bRet;
}



static MS_BOOL _HAL_FSP_WaitForEraseChipDone(void)
{
    MS_U32 start_time = MsOS_GetSystemTime();
    MS_U16 u16Status;

#define DEFAULT_FSP_EASE_CHIP_TIMEOUT 	50000

    ISP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_FIRE_MASK);	//Start FSP

    while(1)//Polling Done
    {
        if( (MsOS_GetSystemTime() - start_time) >=  DEFAULT_FSP_EASE_CHIP_TIMEOUT )
        {
            printk("%s() error : Time out!!!\n", __FUNCTION__);
            return FALSE;
        }

        u16Status = ISP_READ(REG_FSP_DONE_FLAG);
        //printk("u16Status = 0x%x !!!\n", u16Status);
        //if( (ISP_READ(REG_FSP_DONE_FLAG)&REG_FSP_DONE_MASK) == FSP_DONE_FLAG )
        if( (u16Status & FSP_DONE_FLAG) == FSP_DONE_FLAG )
        {
            ISP_WRITE_MASK(REG_FSP_CLR_FLAG,FSP_CLR_FLAG,FSP_CLR_FLAG_MASK);//Clear Done Flag
            break;
        }
    }
    return TRUE;
}

static MS_BOOL _HAL_SERFLASH_EraseByFSP(MS_U32 u32Addr, MS_U8 EraseCmd)
{

    MS_BOOL bRet;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }


    ISP_WRITE(REG_FSP_WD0,ISP_SPI_CMD_WREN);


    ISP_WRITE_MASK(REG_FSP_WD1,FSP_WD1(EraseCmd),REG_FSP_WD1_MASK);
    ISP_WRITE_MASK(REG_FSP_WD2,FSP_WD2((MS_U8)(u32Addr>>16)),REG_FSP_WD2_MASK);
    ISP_WRITE_MASK(REG_FSP_WD3,FSP_WD3((MS_U8)(u32Addr>>8)),REG_FSP_WD3_MASK);
    ISP_WRITE_MASK(REG_FSP_WD4,FSP_WD4((MS_U8)(u32Addr)),REG_FSP_WD4_MASK);

    ISP_WRITE_MASK(REG_FSP_WD5,FSP_WD5(ISP_SPI_CMD_RDSR),REG_FSP_WD5_MASK);

    ISP_WRITE_MASK(REG_FSP_WD6,FSP_WD6(0x0),REG_FSP_WD6_MASK);
    ISP_WRITE_MASK(REG_FSP_WD7,FSP_WD7(0x0),REG_FSP_WD7_MASK);
    ISP_WRITE_MASK(REG_FSP_WD8,FSP_WD8(0x0),REG_FSP_WD8_MASK);
    ISP_WRITE_MASK(REG_FSP_WD9,FSP_WD9(0x0),REG_FSP_WD9_MASK);

    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE1(4),REG_FSP_WBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE2(1),REG_FSP_WBF_SIZE2_MASK);

    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE0(0),REG_FSP_RBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE2(1),REG_FSP_RBF_SIZE2_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_2CMD(1),REG_FSP_CTRL1_2CMD_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_3CMD(1),REG_FSP_CTRL1_3CMD_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_ACS(1),REG_FSP_CTRL1_ACS_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_RDSR_3CMD,REG_FSP_CTRL1_RDSR_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_FSP(1),REG_FSP_CTRL0_FSP_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_RST(1),REG_FSP_CTRL0_RST_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_ACE(1),REG_FSP_CTRL0_ACE_MASK);


    if(_HAL_FSP_WaitForDone() == FALSE)
    {
        printk("_HAL_FSP_WaitForDone Fail \n");
        bRet = FALSE;
        goto HAL_SERFLASH_EraseByFSP_return;
    }


    if(_HAL_FSP_SERFLASH_WaitWriteDone() == FALSE)
    {
        printk("_HAL_FSP_SERFLASH_WaitWriteDone Fail \n");
        bRet = FALSE;
        goto HAL_SERFLASH_EraseByFSP_return;
    }

    bRet = TRUE;

HAL_SERFLASH_EraseByFSP_return:

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("\n"));

    return bRet;


}


static MS_BOOL _HAL_SERFLASH_WriteEnableByFSP(void)
{

    MS_BOOL bRet;

    ISP_WRITE(REG_FSP_WD0,ISP_SPI_CMD_WREN);
    ISP_WRITE_MASK(REG_FSP_WD1,FSP_WD1(0x0),REG_FSP_WD1_MASK);

    ISP_WRITE_MASK(REG_FSP_WD2,FSP_WD2(0x0),REG_FSP_WD2_MASK);
    ISP_WRITE_MASK(REG_FSP_WD3,FSP_WD3(0x0),REG_FSP_WD3_MASK);
    ISP_WRITE_MASK(REG_FSP_WD4,FSP_WD4(0x0),REG_FSP_WD4_MASK);

    ISP_WRITE_MASK(REG_FSP_WD5,FSP_WD5(0x0),REG_FSP_WD5_MASK);
    ISP_WRITE_MASK(REG_FSP_WD6,FSP_WD6(0x0),REG_FSP_WD6_MASK);
    ISP_WRITE_MASK(REG_FSP_WD7,FSP_WD7(0x0),REG_FSP_WD7_MASK);
    ISP_WRITE_MASK(REG_FSP_WD8,FSP_WD8(0x0),REG_FSP_WD8_MASK);
    ISP_WRITE_MASK(REG_FSP_WD9,FSP_WD9(0x0),REG_FSP_WD9_MASK);

    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE0(0x1),REG_FSP_WBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE1(0x0),REG_FSP_WBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE2(0x0),REG_FSP_WBF_SIZE2_MASK);

    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE0(0x0),REG_FSP_RBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE1(0x0),REG_FSP_RBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE2(0x0),REG_FSP_RBF_SIZE2_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_2CMD(0x0),REG_FSP_CTRL1_2CMD_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_3CMD(0x0),REG_FSP_CTRL1_3CMD_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_ACS(0x0),REG_FSP_CTRL1_ACS_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_RDSR_RESER,REG_FSP_CTRL1_RDSR_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_FSP(0x1),REG_FSP_CTRL0_FSP_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_RST(0x1),REG_FSP_CTRL0_RST_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_ACE(0x1),REG_FSP_CTRL0_ACE_MASK);

    bRet = _HAL_FSP_WaitForDone();

    return bRet;

}

static MS_BOOL _HAL_SERFLASH_ProgramFlashCMDByFSP(MS_U32 u32Addr)
{
    MS_BOOL bRet;

    ISP_WRITE(REG_FSP_WD0,ISP_SPI_CMD_PP);

    ISP_WRITE_MASK(REG_FSP_WD1,FSP_WD1((MS_U8)(u32Addr>>16)),REG_FSP_WD1_MASK);
    ISP_WRITE_MASK(REG_FSP_WD2,FSP_WD2((MS_U8)(u32Addr>>8)),REG_FSP_WD2_MASK);
    ISP_WRITE_MASK(REG_FSP_WD3,FSP_WD3((MS_U8)(u32Addr)),REG_FSP_WD3_MASK);

    ISP_WRITE_MASK(REG_FSP_WD4,FSP_WD4(0x00),REG_FSP_WD4_MASK);

    ISP_WRITE_MASK(REG_FSP_WD5,FSP_WD5(0x00),REG_FSP_WD5_MASK);
    ISP_WRITE_MASK(REG_FSP_WD6,FSP_WD6(0x00),REG_FSP_WD6_MASK);
    ISP_WRITE_MASK(REG_FSP_WD7,FSP_WD7(0x00),REG_FSP_WD7_MASK);
    ISP_WRITE_MASK(REG_FSP_WD8,FSP_WD8(0x00),REG_FSP_WD8_MASK);
    ISP_WRITE_MASK(REG_FSP_WD9,FSP_WD9(0x00),REG_FSP_WD9_MASK);

    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE0(0x4),REG_FSP_WBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE1(0x00),REG_FSP_WBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE2(0x00),REG_FSP_WBF_SIZE2_MASK);

    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE0(0x00),REG_FSP_RBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE1(0x00),REG_FSP_RBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE2(0x00),REG_FSP_RBF_SIZE2_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_2CMD(0x00),REG_FSP_CTRL1_2CMD_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_3CMD(0x00),REG_FSP_CTRL1_3CMD_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_ACS(0x00),REG_FSP_CTRL1_ACS_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_RDSR_RESER,REG_FSP_CTRL1_RDSR_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_FSP(0x1),REG_FSP_CTRL0_FSP_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_RST(0x1),REG_FSP_CTRL0_RST_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_ACE(0x1),REG_FSP_CTRL0_ACE_MASK);

    bRet = _HAL_FSP_WaitForDone();

    return bRet;

}

static MS_BOOL _HAL_SERFLASH_FlashWriteDataByFSP(MS_U8* pu8Data,MS_U8 u8Size)
{
    MS_BOOL bRet;

    if(u8Size > 8)
    {
        return FALSE;

    }

    ISP_WRITE_MASK(REG_FSP_WD0,FSP_WD0(pu8Data[0]),REG_FSP_WD0_MASK);

    if(u8Size > 1)
    {
        ISP_WRITE_MASK(REG_FSP_WD1,FSP_WD1(pu8Data[1]),REG_FSP_WD1_MASK);
    }
    else
    {
        ISP_WRITE_MASK(REG_FSP_WD1,FSP_WD1(0x0),REG_FSP_WD1_MASK);
    }

    if(u8Size > 2)
    {
        ISP_WRITE_MASK(REG_FSP_WD2,FSP_WD2(pu8Data[2]),REG_FSP_WD2_MASK);
    }
    else
    {
        ISP_WRITE_MASK(REG_FSP_WD2,FSP_WD2(0x0),REG_FSP_WD2_MASK);
    }

    if(u8Size > 3)
    {
        ISP_WRITE_MASK(REG_FSP_WD3,FSP_WD3(pu8Data[3]),REG_FSP_WD3_MASK);
    }
    else
    {
        ISP_WRITE_MASK(REG_FSP_WD3,FSP_WD3(0x0),REG_FSP_WD3_MASK);
    }

    if(u8Size > 4)
    {
        ISP_WRITE_MASK(REG_FSP_WD4,FSP_WD4(pu8Data[4]),REG_FSP_WD4_MASK);
    }
    else
    {
        ISP_WRITE_MASK(REG_FSP_WD4,FSP_WD4(0x0),REG_FSP_WD4_MASK);
    }

    if(u8Size > 5)
    {
        ISP_WRITE_MASK(REG_FSP_WD5,FSP_WD5(pu8Data[5]),REG_FSP_WD5_MASK);
    }
    else
    {
        ISP_WRITE_MASK(REG_FSP_WD5,FSP_WD5(0x0),REG_FSP_WD5_MASK);
    }

    if(u8Size > 6)
    {
        ISP_WRITE_MASK(REG_FSP_WD6,FSP_WD6(pu8Data[6]),REG_FSP_WD6_MASK);
    }
    else
    {
        ISP_WRITE_MASK(REG_FSP_WD6,FSP_WD6(0x0),REG_FSP_WD6_MASK);
    }

    if(u8Size > 7)
    {
        ISP_WRITE_MASK(REG_FSP_WD7,FSP_WD7(pu8Data[7]),REG_FSP_WD7_MASK);
    }
    else
    {
        ISP_WRITE_MASK(REG_FSP_WD7,FSP_WD7(0x0),REG_FSP_WD7_MASK);
    }

    ISP_WRITE_MASK(REG_FSP_WD8,FSP_WD8(0x0),REG_FSP_WD8_MASK);
    ISP_WRITE_MASK(REG_FSP_WD9,FSP_WD9(0x0),REG_FSP_WD9_MASK);

    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE0(u8Size),REG_FSP_WBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE1(0x0),REG_FSP_WBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE2(0x0),REG_FSP_WBF_SIZE2_MASK);

    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE0(0),REG_FSP_RBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE2(0),REG_FSP_RBF_SIZE2_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_2CMD(0),REG_FSP_CTRL1_2CMD_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_3CMD(0),REG_FSP_CTRL1_3CMD_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_ACS(0),REG_FSP_CTRL1_ACS_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_RDSR_RESER,REG_FSP_CTRL1_RDSR_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_FSP(1),REG_FSP_CTRL0_FSP_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_RST(1),REG_FSP_CTRL0_RST_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_ACE(1),REG_FSP_CTRL0_ACE_MASK);

    bRet = _HAL_FSP_WaitForDone();

    return bRet;
}

static MS_BOOL _HAL_SERFLASH_SetCSZtoGPIO(MS_U8 u8CSZ,MS_BOOL bEnable)
{

    if(u8CSZ == 0)
    {
        if(bEnable == TRUE)
        {
            PM_WRITE_MASK(REG_PM_SPI_IS_GPIO,PM_SPI_CSZ0_IS_GPIO(0x1),PM_SPI_CSZ0_GPIO_MASK);
        }
        else
        {
            PM_WRITE_MASK(REG_PM_SPI_IS_GPIO,PM_SPI_CSZ0_IS_GPIO(0x0),PM_SPI_CSZ0_GPIO_MASK);
        }
    }
    else
    {
         return FALSE;
    }

    return TRUE;

}

static MS_BOOL _HAL_SERFLASH_SetCSZtoHigh(MS_U8 u8CSZ,MS_BOOL bEnable)
{

    if(u8CSZ == 0)
    {
        if(bEnable == TRUE)
        {
            PM_WRITE_MASK(REG_PM_EXTRA_GPIO_OUT,PM_EXTRA_CSZ0_GPIO_OUT(0x1),PM_EXTRA_CSZ0_GPIO_OUT_MASK);
            PM_WRITE_MASK(REG_PM_EXTRA_GPIO_OEN,PM_EXTRA_CSZ0_GPIO_OEN(0x0),PM_EXTRA_CSZ0_GPIO_MASK);

        }
        else
        {
            PM_WRITE_MASK(REG_PM_EXTRA_GPIO_OUT,PM_EXTRA_CSZ0_GPIO_OUT(0x0),PM_EXTRA_CSZ0_GPIO_OUT_MASK);
            PM_WRITE_MASK(REG_PM_EXTRA_GPIO_OEN,PM_EXTRA_CSZ0_GPIO_OEN(0x0),PM_EXTRA_CSZ0_GPIO_MASK);

        }
    }
    else
    {
         return FALSE;
    }

    return TRUE;

}

/* the max size = 10 bytes*/
static MS_BOOL _HAL_SERFLASH_ReadFlashByFSP(MS_U32 u32Addr, MS_U8 *pu8Buf, MS_U8 size)
{
    MS_U8 index;
    MS_U16 u16Data[5]={0};

    MS_BOOL bRet = FALSE;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }


    ISP_WRITE(REG_FSP_WD0,ISP_SPI_CMD_READ);

    ISP_WRITE_MASK(REG_FSP_WD1,FSP_WD1((MS_U8)(u32Addr>>16)),REG_FSP_WD1_MASK);
    ISP_WRITE_MASK(REG_FSP_WD2,FSP_WD2((MS_U8)(u32Addr>>8)),REG_FSP_WD2_MASK);
    ISP_WRITE_MASK(REG_FSP_WD3,FSP_WD3((MS_U8)(u32Addr)),REG_FSP_WD3_MASK);

    ISP_WRITE(REG_FSP_WBF_SIZE,0);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE0(4),REG_FSP_WBF_SIZE0_MASK);
    ISP_WRITE(REG_FSP_RBF_SIZE,0);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE0(size),REG_FSP_RBF_SIZE0_MASK);


    ISP_WRITE(REG_FSP_CTRL,0);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_RDSR_1CMD,REG_FSP_CTRL1_RDSR_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_FSP(1),REG_FSP_CTRL0_FSP_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_RST(1),REG_FSP_CTRL0_RST_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_ACE(1),REG_FSP_CTRL0_ACE_MASK);


    bRet = _HAL_FSP_WaitForDone();

    u16Data[0] = ISP_READ(REG_FSP_RD0);
    u16Data[1] = ISP_READ(REG_FSP_RD2);
    u16Data[2] = ISP_READ(REG_FSP_RD4);
    u16Data[3] = ISP_READ(REG_FSP_RD6);
    u16Data[4] = ISP_READ(REG_FSP_RD8);

    for(index =0 ;index < size; index++)
    {
        if((index & 0x1) == 0)
        {
            *(pu8Buf + index) = (MS_U8)u16Data[index>>1]&0xFF;
        }
        else
        {
            *(pu8Buf + index) = (MS_U8)(u16Data[index>>1]>>8);
        }
    }

//HAL_SERFLASH_ReadFlashByFSP_return:

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("\n"));

    return bRet;


}

static MS_BOOL _HAL_SERFLASH_FSP_Read(MS_U32 u32FlashAddr, MS_U32 u32FlashSize, MS_U8 *user_buffer)
{
    MS_U32 Index;
    MS_BOOL bRet = FALSE;

#define FSP_READ_SIZE	10

    for(Index = 0; Index < u32FlashSize; )
    {
        if((Index + FSP_READ_SIZE) < u32FlashSize)
        {
            bRet = _HAL_SERFLASH_ReadFlashByFSP(u32FlashAddr+Index, user_buffer+Index,FSP_READ_SIZE);
            if(bRet == FALSE)
            {
                return bRet;
            }
        }
        else
        {
            bRet = _HAL_SERFLASH_ReadFlashByFSP(u32FlashAddr+Index, user_buffer+Index,u32FlashSize-Index);
            if(bRet == FALSE)
            {
                return bRet;
            }
        }
        Index += FSP_READ_SIZE;
    }

    return bRet;
}

#endif

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SERFLASH_SetCKG()
/// @brief \b Function \b Description: This function is used to set ckg_spi dynamically
/// @param <IN>        \b eCkgSpi    : enumerate the ckg_spi
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>    \b NONE    :
/// @param <NOTE>    \b : Please use this function carefully , and is restricted to Flash ability
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_SERFLASH_SetCKG(SPI_DrvCKG eCkgSpi)
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

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SERFLASH_ClkDiv()
/// @brief \b Function \b Description: This function is used to set clock div dynamically
/// @param <IN>        \b eCkgSpi    : enumerate the clk_div
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>    \b NONE    :
/// @param <NOTE>    \b : Please use this function carefully , and is restricted to Flash ability
////////////////////////////////////////////////////////////////////////////////
void HAL_SERFLASH_ClkDiv(SPI_DrvClkDiv eClkDivSpi)
{
    switch (eClkDivSpi)
    {
        case E_SPI_DIV2:
            ISP_WRITE(REG_ISP_SPI_CLKDIV,ISP_SPI_CLKDIV2);
            break;
        case E_SPI_DIV4:
            ISP_WRITE(REG_ISP_SPI_CLKDIV,ISP_SPI_CLKDIV4);
            break;
        case E_SPI_DIV8:
            ISP_WRITE(REG_ISP_SPI_CLKDIV,ISP_SPI_CLKDIV8);
            break;
        case E_SPI_DIV16:
            ISP_WRITE(REG_ISP_SPI_CLKDIV,ISP_SPI_CLKDIV16);
            break;
        case E_SPI_DIV32:
            ISP_WRITE(REG_ISP_SPI_CLKDIV,ISP_SPI_CLKDIV32);
            break;
        case E_SPI_DIV64:
            ISP_WRITE(REG_ISP_SPI_CLKDIV,ISP_SPI_CLKDIV64);
            break;
        case E_SPI_DIV128:
            ISP_WRITE(REG_ISP_SPI_CLKDIV,ISP_SPI_CLKDIV128);
            break;
        case E_SPI_ClkDiv_NOT_SUPPORT:
            DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SERFLASH_SetMode()
/// @brief \b Function \b Description: This function is used to set RIU/XIU dynamically
/// @param <IN>        \b bXiuRiu    : Enable for XIU (Default) Disable for RIU(Optional)
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>    \b NONE    :
/// @param <NOTE>    \b : XIU is faster than RIU, but is sensitive to ckg.
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_SERFLASH_SetMode(MS_BOOL bXiuRiu)
{
    MS_BOOL Ret = FALSE;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
    _bXIUMode = bXiuRiu;
    Ret = TRUE;
    return Ret;
}

void HAL_SERFLASH_SelectReadMode(SPI_READ_MODE eReadMode)
{
    switch(eReadMode)
    {
    case E_SINGLE_MODE:
        ISP_WRITE_MASK(REG_ISP_SPI_MODE, SFSH_CHIP_FAST_DISABLE, SFSH_CHIP_FAST_MASK);
    break;
    case E_FAST_MODE:
        ISP_WRITE_MASK(REG_ISP_SPI_MODE, SFSH_CHIP_FAST_ENABLE, SFSH_CHIP_FAST_MASK);
    break;
    case E_DUAL_D_MODE:
        ISP_WRITE_MASK(REG_ISP_SPI_MODE, SFSH_CHIP_2XREAD_ENABLE, SFSH_CHIP_2XREAD_MASK);
        ISP_WRITE_MASK(REG_ISP_SPI_MODE, SFSH_CHIP_FAST_DISABLE, SFSH_CHIP_FAST_MASK);
    break;
    case E_DUAL_AD_MODE:
        ISP_WRITE_MASK(REG_ISP_SPI_MODE, SFSH_CHIP_2XREAD_ENABLE, SFSH_CHIP_2XREAD_MASK);
    break;
    case E_QUAD_MODE:
    break;
    }

}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SERFLASH_Set2XREAD()
/// @brief \b Function \b Description: This function is used to set 2XREAD dynamically
/// @param <IN>        \b b2XMode    : ENABLE for 2XREAD DISABLE for NORMAL
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>    \b NONE    :
/// @param <NOTE>    \b : Please use this function carefully, and needs Flash support
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_SERFLASH_Set2XREAD(MS_BOOL b2XMode)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
    if(!bDetect)
    {
        HAL_SERFLASH_DetectType();
    }
    MS_ASSERT(_hal_SERFLASH.b2XREAD); // check hw support or not
    if(_hal_SERFLASH.b2XREAD)
    {
        _HAL_ISP_2XMode(b2XMode);
    }
    else
    {
        UNUSED(b2XMode);
        printk("%s This flash does not support 2XREAD!!!\n", __FUNCTION__);
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SERFLASH_ChipSelect()
/// @brief \b Function \b Description: set active flash among multi-spi flashes
/// @param <IN>        \b u8FlashIndex : flash index (0 or 1)
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_SERFLASH_ChipSelect(MS_U8 u8FlashIndex)
{
    MS_BOOL Ret = FALSE;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X)\n", __FUNCTION__, (int)u8FlashIndex));
    switch (u8FlashIndex)
    {
        case FLASH_ID0:
            ISP_WRITE_MASK(REG_ISP_SPI_CHIP_SELE,SFSH_CHIP_SELE_EXT1,SFSH_CHIP_SELE_MASK);
            Ret = TRUE;
            break;
        case FLASH_ID1:
            ISP_WRITE_MASK(REG_ISP_SPI_CHIP_SELE,SFSH_CHIP_SELE_EXT2,SFSH_CHIP_SELE_MASK);
            Ret = TRUE;
            break;
        case FLASH_ID2:
            ISP_WRITE_MASK(REG_ISP_SPI_CHIP_SELE,SFSH_CHIP_SELE_EXT3,SFSH_CHIP_SELE_MASK);
            Ret = TRUE;
            break;
        case FLASH_ID3:
            UNUSED(u8FlashIndex); //Reserved
        default:
            UNUSED(u8FlashIndex); //Invalid flash ID
            Ret = FALSE;
            break;
    }
    WAIT_SFSH_CS_STAT(); // wait for chip select done
    return Ret;
}

void HAL_SERFLASH_Config(MS_U32 u32PMRegBaseAddr, MS_U32 u32NonPMRegBaseAddr, MS_U32 u32XiuBaseAddr)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, 0x%08X, 0x%08X)\n", __FUNCTION__, (int)u32PMRegBaseAddr, (int)u32NonPMRegBaseAddr, (int)u32XiuBaseAddr));
    _hal_isp.u32XiuBaseAddr = u32XiuBaseAddr;
    _hal_isp.u32Mheg5BaseAddr = u32NonPMRegBaseAddr + BK_MHEG5;
    _hal_isp.u32IspBaseAddr = u32PMRegBaseAddr + BK_ISP;
    _hal_isp.u32PiuBaseAddr = u32PMRegBaseAddr + BK_PIU;
    _hal_isp.u32PMBaseAddr = u32PMRegBaseAddr + BK_PMSLP;
    _hal_isp.u32CLK0BaseAddr = u32NonPMRegBaseAddr + BK_CLK0;
    _hal_isp.u32BdmaBaseAddr = u32NonPMRegBaseAddr + BK_BDMA;
    _hal_isp.u32RiuBaseAddr =  u32PMRegBaseAddr;
}


void HAL_SERFLASH_Init(void)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));

    _s32SERFLASH_Mutex = MsOS_CreateMutex(E_MSOS_FIFO, "Mutex SERFLASH", MSOS_PROCESS_SHARED);
    MS_ASSERT(_s32SERFLASH_Mutex >= 0);

    // U4 51 MCU runs at 108M Hz, SPI runs at 54M Hz for SPANSION FLASH **2010/08/27**
    ISP_WRITE(REG_ISP_SPI_CLKDIV, ISP_SPI_CLKDIV4);

    ISP_WRITE(REG_ISP_SPI_ENDIAN, ISP_SPI_ENDIAN_SEL);
}

void HAL_SERFLASH_SetGPIO(MS_BOOL bSwitch)
{
    if(bSwitch)// The PAD of the SPI set as GPIO IN.
    {
        PM_WRITE_MASK(REG_PM_SPI_IS_GPIO,   PM_SPI_IS_GPIO,  PM_SPI_GPIO_MASK);
        PM_WRITE_MASK(REG_PM_EXTRA_GPIO_OEN,PM_EXTRA_GPIO_IN,PM_EXTRA_GPIO_MASK);
    }
    else
    {
        PM_WRITE_MASK(REG_PM_SPI_IS_GPIO,   PM_SPI_NOT_GPIO, PM_SPI_GPIO_MASK);
        PM_WRITE_MASK(REG_PM_EXTRA_GPIO_OEN,PM_EXTRA_GPIO_IN,PM_EXTRA_GPIO_MASK);
    }
}

MS_BOOL HAL_SERFLASH_DetectType(void)
{
    #define READ_ID_SIZE    3
    #define READ_REMS4_SIZE 2
    MS_U8   u8Status0 = 0;
    MS_U8   u8Status1 = 0;
    MS_U8   u8FlashId[READ_ID_SIZE];
    MS_U8   u8FlashREMS4[READ_REMS4_SIZE];
    MS_U32  u32Index;

    memset(&_hal_SERFLASH, 0, sizeof(_hal_SERFLASH));

    // If use MXIC MX25L6445E
    HAL_SERFLASH_ReadREMS4(u8FlashREMS4,READ_REMS4_SIZE);

    if (HAL_SERFLASH_ReadID(u8FlashId, sizeof(u8FlashId))== TRUE)
    {
        /* find current serial flash */
        for (u32Index = 0; _hal_SERFLASH_table[u32Index].u8MID != 0; u32Index++)
        {

            if (   (_hal_SERFLASH_table[u32Index].u8MID  == u8FlashId[0])
                && (_hal_SERFLASH_table[u32Index].u8DID0 == u8FlashId[1])
                && (_hal_SERFLASH_table[u32Index].u8DID1 == u8FlashId[2])
                )
            {
                memcpy(&_hal_SERFLASH, &(_hal_SERFLASH_table[u32Index]), sizeof(_hal_SERFLASH));

                // patch : MXIC 6405D vs 6445E(MXIC 12805D vs 12845E)
                if( u8FlashREMS4[0] == 0xC2)
                {
                    if( u8FlashREMS4[1] == 0x16)
                    {
                        _hal_SERFLASH.u16FlashType = FLASH_IC_MX25L6445E;
                        _hal_SERFLASH.pWriteProtectTable = _pstWriteProtectTable_MX25L6445E;
                    }
                    if( u8FlashREMS4[1] == 0x17)
                    {
                        _hal_SERFLASH.u16FlashType = FLASH_IC_MX25L12845E;
                        _hal_SERFLASH.pWriteProtectTable = _pstWriteProtectTable_MX25L12845E;
                    }
                }

                DEBUG_SER_FLASH(E_SERFLASH_DBGLV_INFO,
                                printk("Flash is detected (0x%04X, 0x%02X, 0x%02X, 0x%02X)\n",
                                       _hal_SERFLASH.u16FlashType,
                                       _hal_SERFLASH.u8MID,
                                       _hal_SERFLASH.u8DID0,
                                       _hal_SERFLASH.u8DID1
                                       )
                                );
                bDetect = TRUE;
                break;
            }
            else
            {
                continue;
            }
        }
        // If the Board uses a unknown flash type, force setting a secure flash type for booting. //FLASH_IC_MX25L6405D
        if( bDetect != TRUE )
        {
            memcpy(&_hal_SERFLASH, &(_hal_SERFLASH_table[2]), sizeof(_hal_SERFLASH));

            DEBUG_SER_FLASH(E_SERFLASH_DBGLV_INFO,
                            printk("detect flash type (0x%02X, 0x%02X, 0x%02X)\n",
                                   u8FlashId[0],
                                   u8FlashId[1],
                                   u8FlashId[2]
                                   )
                            );

            DEBUG_SER_FLASH(E_SERFLASH_DBGLV_INFO,
                            printk("Unknown flash type (0x%02X, 0x%02X, 0x%02X) and use default flash type 0x%04X\n",
                                   _hal_SERFLASH.u8MID,
                                   _hal_SERFLASH.u8DID0,
                                   _hal_SERFLASH.u8DID1,
                                   _hal_SERFLASH.u16FlashType
                                   )
                            );
            bDetect = TRUE;
        }

        if( _hal_SERFLASH.u8MID == MID_GD )
        {
            HAL_SERFLASH_ReadStatusReg(&u8Status0);
            HAL_SERFLASH_ReadStatusReg2(&u8Status1);
            HAL_SERFLASH_WriteStatusReg(((u8Status1 << 8)|u8Status0|0x4000));//CMP = 1
            _hal_SERFLASH.u8WrsrBlkProtect = BITS(6:2, 0x00);
            _hal_SERFLASH.pWriteProtectTable = _pstWriteProtectTable_GD25Q32_CMP1;
        }

        if( _hal_SERFLASH.u16FlashType == FLASH_IC_S25FL032K )
        {
            HAL_SERFLASH_ReadStatusReg(&u8Status0);
            HAL_SERFLASH_ReadStatusReg2(&u8Status1);
            HAL_SERFLASH_WriteStatusReg(((u8Status1 << 8)|u8Status0|0x4000));//CMP = 1
            _hal_SERFLASH.u8WrsrBlkProtect = BITS(6:2, 0x00);
            _hal_SERFLASH.pWriteProtectTable = _pstWriteProtectTable_S25FL032K_CMP1;
        }

    }
    ISP_WRITE(REG_ISP_DEV_SEL, ISP_DEV_SEL);
    HAL_SERFLASH_SelectReadMode(_hal_SERFLASH.u16SPIMaxClk[1]);
    HAL_SERFLASH_SetCKG(_hal_SERFLASH.u16SPIMaxClk[0]);
    return bDetect;

}

MS_BOOL HAL_SERFLASH_DetectSize(MS_U32  *u32FlashSize)
{
    MS_BOOL Ret = FALSE;

    do{

        *u32FlashSize = _hal_SERFLASH.u32FlashSize;
        Ret = TRUE;

    }while(0);

    return Ret;
}

MS_BOOL HAL_SERFLASH_AddressToBlock(MS_U32 u32FlashAddr, MS_U32 *pu32BlockIndex)
{
    MS_U32  u32NextAddr;
    MS_BOOL bRet = FALSE;

    if (_hal_SERFLASH.pSpecialBlocks == NULL)
    {
        *pu32BlockIndex = u32FlashAddr / SERFLASH_SECTOR_SIZE;

        bRet = TRUE;
    }
    else
    {
        // TODO: review, optimize this flow
        for (u32NextAddr = 0, *pu32BlockIndex = 0; *pu32BlockIndex < NUMBER_OF_SERFLASH_SECTORS; (*pu32BlockIndex)++)
        {
            // outside the special block
            if (   *pu32BlockIndex < _hal_SERFLASH.pSpecialBlocks->u16Start
                || *pu32BlockIndex > _hal_SERFLASH.pSpecialBlocks->u16End
                )
            {
                u32NextAddr += SERFLASH_SECTOR_SIZE; // i.e. normal block size
            }
            // inside the special block
            else
            {
                u32NextAddr += _hal_SERFLASH.pSpecialBlocks->au32SizeList[*pu32BlockIndex - _hal_SERFLASH.pSpecialBlocks->u16Start];
            }

            if (u32NextAddr > u32FlashAddr)
            {
                bRet = TRUE;
                break;
            }
        }
    }

    return bRet;
}


MS_BOOL HAL_SERFLASH_BlockToAddress(MS_U32 u32BlockIndex, MS_U32 *pu32FlashAddr)
{
    if (   _hal_SERFLASH.pSpecialBlocks == NULL
        || u32BlockIndex <= _hal_SERFLASH.pSpecialBlocks->u16Start
        )
    {
        *pu32FlashAddr = u32BlockIndex * SERFLASH_SECTOR_SIZE;
    }
    else
    {
        MS_U32 u32Index;

        *pu32FlashAddr = _hal_SERFLASH.pSpecialBlocks->u16Start * SERFLASH_SECTOR_SIZE;

        for (u32Index = _hal_SERFLASH.pSpecialBlocks->u16Start;
             u32Index < u32BlockIndex && u32Index <= _hal_SERFLASH.pSpecialBlocks->u16End;
             u32Index++
             )
        {
            *pu32FlashAddr += _hal_SERFLASH.pSpecialBlocks->au32SizeList[u32Index - _hal_SERFLASH.pSpecialBlocks->u16Start];
        }

        if (u32BlockIndex > _hal_SERFLASH.pSpecialBlocks->u16End + 1)
        {
            *pu32FlashAddr += (u32BlockIndex - _hal_SERFLASH.pSpecialBlocks->u16End - 1) * SERFLASH_SECTOR_SIZE;
        }
    }

    return TRUE;
}

MS_BOOL HAL_SERFLASH_DMA(MS_U32 u32FlashStart, MS_U32 u32DRAMStart, MS_U32 u32Size)
{
    MS_BOOL bRet = FALSE;


#if (defined (MSOS_TYPE_LINUX) || defined (MSOS_TYPE_ECOS))
    struct timeval time_st;
    MS_U32 u32Timeout = SERFLASH_SAFETY_FACTOR*u32Size/(108*1000/4/8);
#else
    MS_U32 u32Timer;
    MS_U32 u32Timeout = SERFLASH_SAFETY_FACTOR;
#endif

    u32Timeout=u32Timeout; //to make compiler happy
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, 0x%08X, %d)\n", __FUNCTION__, (int)u32FlashStart, (int)u32DRAMStart, (int)u32Size));

    // [URANUS_REV_A][OBSOLETE] // TODO: <-@@@ CHIP SPECIFIC
    #if 0   // TODO: review
    if (MDrv_SYS_GetChipRev() == 0x00)
    {
        // DMA program can't run on DRAM, but in flash ONLY
        return FALSE;
    }
    #endif  // TODO: review
    // [URANUS_REV_A][OBSOLETE] // TODO: <-@@@ CHIP SPECIFIC

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    ISP_WRITE_MASK(REG_ISP_CHIP_SEL, SFSH_CHIP_SEL_RIU, SFSH_CHIP_SEL_MODE_SEL_MASK);   // For DMA only

    _HAL_ISP_Disable();

    // SFSH_RIU_REG16(REG_SFSH_SPI_CLK_DIV) = 0x02; // 108MHz div3 (max. 50MHz for this ST flash) for FAST_READ

    PIU_WRITE(REG_PIU_DMA_SIZE_L, LOU16(u32Size));
    PIU_WRITE(REG_PIU_DMA_SIZE_H, HIU16(u32Size));
    PIU_WRITE(REG_PIU_DMA_DRAMSTART_L, LOU16(u32DRAMStart));
    PIU_WRITE(REG_PIU_DMA_DRAMSTART_H, HIU16(u32DRAMStart));
    PIU_WRITE(REG_PIU_DMA_SPISTART_L, LOU16(u32FlashStart));
    PIU_WRITE(REG_PIU_DMA_SPISTART_H, HIU16(u32FlashStart));
    // SFSH_PIU_REG16(REG_SFSH_DMA_CMD) = 0 << 5; // 0: little-endian 1: big-endian
    // SFSH_PIU_REG16(REG_SFSH_DMA_CMD) |= 1; // trigger
    PIU_WRITE(REG_PIU_DMA_CMD, PIU_DMA_CMD_LE | PIU_DMA_CMD_FIRE); // trigger

    // Wait for DMA to be done
#if (defined (MSOS_TYPE_LINUX) || defined (MSOS_TYPE_ECOS))
    SER_FLASH_TIME(time_st);
#else
    SER_FLASH_TIME(u32Timer, u32Timeout);
#endif
    do
    {
        if ( (PIU_READ(REG_PIU_DMA_STATUS) & PIU_DMA_DONE_MASK) == PIU_DMA_DONE ) // finished
        {
            bRet = TRUE;
            break;
        }
#if (defined (MSOS_TYPE_LINUX) || defined (MSOS_TYPE_ECOS))
    } while (!SER_FLASH_EXPIRE(time_st, u32Timeout));
#else
    } while (!SER_FLASH_EXPIRE(u32Timer));
#endif
    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("DMA timeout!\n"));
    }

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    return bRet;
}




#if(FSP == 0) // RIU ISP mode

MS_BOOL HAL_SERFLASH_EraseChip(void)
{
    MS_BOOL bRet = FALSE;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if(!_HAL_SERFLASH_WaitWriteDone())
    {
        goto HAL_SERFLASH_EraseChip_return;
    }

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_EraseChip_return;
    }
    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN


    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_EraseChip_return;
    }
    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_CE); // CHIP_ERASE

    bRet = _HAL_SERFLASH_WaitWriteDone();

HAL_SERFLASH_EraseChip_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    return bRet;
}


MS_BOOL HAL_SERFLASH_BlockErase(MS_U32 u32StartBlock, MS_U32 u32EndBlock, MS_BOOL bWait)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32I;
    MS_U32 u32FlashAddr = 0;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, 0x%08X, %d)\n", __FUNCTION__, (int)u32StartBlock, (int)u32EndBlock, bWait));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return bRet;
    }

    if( u32StartBlock > u32EndBlock || u32EndBlock >= _hal_SERFLASH.u32NumSec )
    {
        printk("%s (0x%08X, 0x%08X, %d)\n", __FUNCTION__, (int)u32StartBlock, (int)u32EndBlock, bWait);
        bRet = FALSE;
        goto HAL_SERFLASH_BlockErase_return;
    }

    _HAL_ISP_Enable();

    if(!_HAL_SERFLASH_WaitWriteDone())
    {
        bRet = FALSE;
        goto HAL_SERFLASH_BlockErase_return;
    }

    for( u32I = u32StartBlock; u32I <= u32EndBlock; u32I++)
    {
        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

        if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
        {
            goto HAL_SERFLASH_BlockErase_return;
        }
        SPI_FLASH_CMD(ISP_SPI_CMD_WREN);    // WREN

        if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
        {
            goto HAL_SERFLASH_BlockErase_return;
        }

        ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333); // enable trigger mode

        ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_64BE); // BLOCK_ERASE

        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_BlockErase_return;
        }

        if (HAL_SERFLASH_BlockToAddress(u32I, &u32FlashAddr) == FALSE )
        {
            goto HAL_SERFLASH_BlockErase_return;
        }

        ISP_WRITE(REG_ISP_SPI_WDATA, HIU16(u32FlashAddr) & 0xFF);

        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_BlockErase_return;
        }

        ISP_WRITE(REG_ISP_SPI_WDATA, LOU16(u32FlashAddr) >> 8);

        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_BlockErase_return;
        }

        ISP_WRITE(REG_ISP_SPI_WDATA, LOU16(u32FlashAddr) & 0xFF);

        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_BlockErase_return;
        }

        ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x5555); // disable trigger mode

        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

        if(bWait == TRUE )
        {
            if(!_HAL_SERFLASH_WaitWriteDone())
            {
                printk("%s : Wait Write Done Fail!!!\n", __FUNCTION__ );
                bRet = FALSE;
                goto HAL_SERFLASH_BlockErase_return;
            }
            else
            {
                bRet = TRUE;
            }
        }
        else
        {
            bRet = TRUE;
        }
    }

HAL_SERFLASH_BlockErase_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    return bRet;
}


MS_BOOL HAL_SERFLASH_SectorErase(MS_U32 u32SectorAddress)
{
    MS_BOOL bRet = FALSE;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X)\n", __FUNCTION__, (int)u32SectorAddress));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s ENTRY fails!\n", __FUNCTION__));
        return bRet;
    }

    if( u32SectorAddress > _hal_SERFLASH.u32FlashSize )
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s (0x%08X)\n", __FUNCTION__, (int)u32SectorAddress));
        goto HAL_SERFLASH_BlockErase_return;
    }

    _HAL_ISP_Enable();

    if(!_HAL_SERFLASH_WaitWriteDone())
    {
        goto HAL_SERFLASH_BlockErase_return;
    }

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
            goto HAL_SERFLASH_BlockErase_return;
    }

    SPI_FLASH_CMD(ISP_SPI_CMD_WREN);    // WREN

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
            goto HAL_SERFLASH_BlockErase_return;
    }

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333); // enable trigger mode

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_SE);

    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
            goto HAL_SERFLASH_BlockErase_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, HIU16(u32SectorAddress) & 0xFF);

    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
            goto HAL_SERFLASH_BlockErase_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, LOU16(u32SectorAddress) >> 8);

    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
            goto HAL_SERFLASH_BlockErase_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, LOU16(u32SectorAddress) & 0xFF);

    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
            goto HAL_SERFLASH_BlockErase_return;
    }

    bRet = TRUE;

HAL_SERFLASH_BlockErase_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x5555);     // disable trigger mode

    _HAL_ISP_Disable();

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    return bRet;
}


MS_BOOL HAL_SERFLASH_CheckWriteDone(void)
{
    MS_BOOL bRet = FALSE;

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR);    // SPI CEB dis

    bRet = _HAL_SERFLASH_CheckWriteDone();

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR);    // SPI CEB dis

    _HAL_ISP_Disable();

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s() = %d\n", __FUNCTION__, bRet));

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    return bRet;
}


MS_BOOL HAL_SERFLASH_Write(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data)
{
    MS_BOOL bRet = FALSE;
    MS_U16 u16I, u16Rem, u16WriteBytes;
    MS_U8 *u8Buf = pu8Data;
    MS_BOOL b2XREAD = FALSE;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, %d, %p)\n", __FUNCTION__, (int)u32Addr, (int)u32Size, pu8Data));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    b2XREAD = (BIT(2) & ISP_READ(REG_ISP_SPI_MODE))? 1 : 0;

    HAL_SERFLASH_SelectReadMode(E_FAST_MODE);

    _HAL_ISP_Enable();

    if(!_HAL_SERFLASH_WaitWriteDone())
    {
        bRet = FALSE;
        goto HAL_SERFLASH_Write_return;
    }

    u16Rem = u32Addr % SERFLASH_PAGE_SIZE;

    if (u16Rem)
    {
        u16WriteBytes = SERFLASH_PAGE_SIZE - u16Rem;
        if (u32Size < u16WriteBytes)
        {
            u16WriteBytes = u32Size;
        }

        if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
        {
            goto HAL_SERFLASH_Write_return;
        }

        SPI_FLASH_CMD(ISP_SPI_CMD_WREN);

        ISP_WRITE(REG_ISP_SPI_ADDR_L, LOU16(u32Addr));
        ISP_WRITE(REG_ISP_SPI_ADDR_H, (MS_U8)HIU16(u32Addr));

        if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
        {
            goto HAL_SERFLASH_Write_return;
        }

        SPI_FLASH_CMD(ISP_SPI_CMD_PP);  // PAGE_PROG

        for ( u16I = 0; u16I < u16WriteBytes; u16I++ )
        {
            SPI_WRITE_DATA( *(u8Buf + u16I) );

            if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
            {
                goto HAL_SERFLASH_Write_return;
            }
        }

        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

        bRet = _HAL_SERFLASH_WaitWriteDone();

        if ( bRet == TRUE )
        {
            u32Addr += u16WriteBytes;
            u8Buf   += u16WriteBytes;
            u32Size -= u16WriteBytes;
        }
        else
        {
            goto HAL_SERFLASH_Write_return;
        }
    }

    while(u32Size)
    {
        if( u32Size > SERFLASH_PAGE_SIZE)
        {
            u16WriteBytes = SERFLASH_PAGE_SIZE;  //write SERFLASH_PAGE_SIZE bytes one time
        }
        else
        {
            u16WriteBytes = u32Size;
        }

        if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
        {
            goto HAL_SERFLASH_Write_return;
        }

        SPI_FLASH_CMD(ISP_SPI_CMD_WREN);    // WREN

        ISP_WRITE(REG_ISP_SPI_ADDR_L, LOU16(u32Addr));
        ISP_WRITE(REG_ISP_SPI_ADDR_H, (MS_U8)HIU16(u32Addr));

        if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
        {
            goto HAL_SERFLASH_Write_return;
        }
        SPI_FLASH_CMD(ISP_SPI_CMD_PP);  // PAGE_PROG

        // Improve flash write speed
        if(u16WriteBytes == 256)
        {
            // Write 256 bytes to flash
            MS_U8 u8Index = 0;

            do{

                SPI_WRITE_DATA( *(u8Buf + u8Index) );

                u8Index++;

                if( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
                {
                    goto HAL_SERFLASH_Write_return;
                }

            }while(u8Index != 0);
        }
        else
        {

            for ( u16I = 0; u16I < u16WriteBytes; u16I++ )
            {
                SPI_WRITE_DATA( *(u8Buf + u16I) );

                if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
                {
                    goto HAL_SERFLASH_Write_return;
                }
            }
        }

        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

        bRet = _HAL_SERFLASH_WaitWriteDone();

        if ( bRet == TRUE )
        {
            u32Addr += u16WriteBytes;
            u8Buf   += u16WriteBytes;
            u32Size -= u16WriteBytes;
        }
        else
        {
            goto HAL_SERFLASH_Write_return;
        }
    }


HAL_SERFLASH_Write_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

    //  restore the 2x READ setting.
    
    HAL_SERFLASH_SelectReadMode(_hal_SERFLASH.u16SPIMaxClk[1]);
    

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    return bRet;
}


MS_BOOL HAL_SERFLASH_Read(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data)
{
    MS_BOOL Ret = FALSE;

    if( _bXIUMode )
    {
        Ret = _HAL_SERFLASH_XIURead( u32Addr, u32Size, pu8Data);
    }
    else// RIU mode
    {
        Ret = _HAL_SERFLASH_RIURead( u32Addr, u32Size, pu8Data);
    }

    return Ret;
}


#endif

EN_WP_AREA_EXISTED_RTN HAL_SERFLASH_WP_Area_Existed(MS_U32 u32UpperBound, MS_U32 u32LowerBound, MS_U8 *pu8BlockProtectBits)
{
    ST_WRITE_PROTECT   *pWriteProtectTable;
    MS_U8               u8Index;
    MS_BOOL             bPartialBoundFitted;
    MS_BOOL             bEndOfTable;
    MS_U32              u32PartialFittedLowerBound = u32UpperBound;
    MS_U32              u32PartialFittedUpperBound = u32LowerBound;


    if (NULL == _hal_SERFLASH.pWriteProtectTable)
    {
        return WP_TABLE_NOT_SUPPORT;
    }


    for (u8Index = 0, bEndOfTable = FALSE, bPartialBoundFitted = FALSE; FALSE == bEndOfTable; u8Index++)
    {
        pWriteProtectTable = &(_hal_SERFLASH.pWriteProtectTable[u8Index]);

        if (   0xFFFFFFFF == pWriteProtectTable->u32LowerBound
            && 0xFFFFFFFF == pWriteProtectTable->u32UpperBound
            )
        {
            bEndOfTable = TRUE;
        }

        if (   pWriteProtectTable->u32LowerBound == u32LowerBound
            && pWriteProtectTable->u32UpperBound == u32UpperBound
            )
        {
            *pu8BlockProtectBits = pWriteProtectTable->u8BlockProtectBits;

            return WP_AREA_EXACTLY_AVAILABLE;
        }
        else if (u32LowerBound <= pWriteProtectTable->u32LowerBound && pWriteProtectTable->u32UpperBound <= u32UpperBound)
        {
            //
            // u32PartialFittedUpperBound & u32PartialFittedLowerBound would be initialized first time when bPartialBoundFitted == FALSE (init value)
            // 1. first match:  FALSE == bPartialBoundFitted
            // 2. better match: (pWriteProtectTable->u32UpperBound - pWriteProtectTable->u32LowerBound) > (u32PartialFittedUpperBound - u32PartialFittedLowerBound)
            //

            if (   FALSE == bPartialBoundFitted
                || (pWriteProtectTable->u32UpperBound - pWriteProtectTable->u32LowerBound) > (u32PartialFittedUpperBound - u32PartialFittedLowerBound)
                )
            {
                u32PartialFittedUpperBound = pWriteProtectTable->u32UpperBound;
                u32PartialFittedLowerBound = pWriteProtectTable->u32LowerBound;
                *pu8BlockProtectBits = pWriteProtectTable->u8BlockProtectBits;
            }

            bPartialBoundFitted = TRUE;
        }
    }

    if (TRUE == bPartialBoundFitted)
    {
        return WP_AREA_PARTIALLY_AVAILABLE;
    }
    else
    {
        return WP_AREA_NOT_AVAILABLE;
    }
}


#if(FSP == 0)  // RIU ISP mode

MS_BOOL HAL_SERFLASH_WriteProtect_Area(MS_BOOL bEnableAllArea, MS_U8 u8BlockProtectBits)
{
    MS_BOOL bRet = FALSE;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(%d, 0x%02X)\n", __FUNCTION__, bEnableAllArea, u8BlockProtectBits));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_SERFLASH_ActiveFlash_Set_HW_WP(DISABLE);
    //MsOS_DelayTask(bEnableAllArea ? 5 : 20); // when disable WP, delay more time

    _HAL_ISP_Enable();

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_Flash_WriteProtect_Area_return;
    }

    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_Flash_WriteProtect_Area_return;
    }

    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WRSR); // WRSR

    if (TRUE == bEnableAllArea)
    {
        if (_hal_SERFLASH.u16FlashType == FLASH_IC_AT26DF321)
        {
            ISP_WRITE(REG_ISP_SPI_WDATA, SERFLASH_WRSR_BLK_PROTECT); // SPRL 1 -> 0

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_Flash_WriteProtect_Area_return;
            }

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_Flash_WriteProtect_Area_return;
            }

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WRSR); // WRSR
        }

        ISP_WRITE(REG_ISP_SPI_WDATA, SF_SR_SRWD | SERFLASH_WRSR_BLK_PROTECT); // SF_SR_SRWD: SRWD Status Register Write Protect
    }
    else
    {
        if (_hal_SERFLASH.u16FlashType == FLASH_IC_AT26DF321)
        {
            ISP_WRITE(REG_ISP_SPI_WDATA, u8BlockProtectBits); // [4:2] or [5:2] protect blocks // SPRL 1 -> 0

            // programming sector protection
            {
                int i;
                MS_U32 u32FlashAddr;

                // search write protect table
                for (i = 0;
                     0xFFFFFFFF != _hal_SERFLASH.pWriteProtectTable[i].u32LowerBound && 0xFFFFFFFF != _hal_SERFLASH.pWriteProtectTable[i].u32UpperBound; // the end of write protect table
                     i++
                     )
                {
                    // if found, write
                    if (u8BlockProtectBits == _hal_SERFLASH.pWriteProtectTable[i].u8BlockProtectBits)
                    {
                        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("u8BlockProtectBits = 0x%X, u32LowerBound = 0x%X, u32UpperBound = 0x%X\n",
                                                                       (unsigned int)u8BlockProtectBits,
                                                                       (unsigned int)_hal_SERFLASH.pWriteProtectTable[i].u32LowerBound,
                                                                       (unsigned int)_hal_SERFLASH.pWriteProtectTable[i].u32UpperBound
                                                                       )
                                        );
                        for (u32FlashAddr = 0; u32FlashAddr < _hal_SERFLASH.u32FlashSize; u32FlashAddr += _hal_SERFLASH.u32SecSize)
                        {
                            if (_hal_SERFLASH.pWriteProtectTable[i].u32LowerBound <= (u32FlashAddr + _hal_SERFLASH.u32SecSize - 1) &&
                                u32FlashAddr <= _hal_SERFLASH.pWriteProtectTable[i].u32UpperBound)
                            {
                                continue;
                            }

                            ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

                            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
                            {
                                goto HAL_Flash_WriteProtect_Area_return;
                            }

                            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

                            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
                            {
                                goto HAL_Flash_WriteProtect_Area_return;
                            }

                            ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333); // enable trigger mode

                            ISP_WRITE(REG_ISP_SPI_WDATA, 0x39); // unprotect sector

                            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
                            {
                                goto HAL_Flash_WriteProtect_Area_return;
                            }

                            ISP_WRITE(REG_ISP_SPI_WDATA, (u32FlashAddr >> 16) & 0xFF);

                            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
                            {
                                goto HAL_Flash_WriteProtect_Area_return;
                            }

                            ISP_WRITE(REG_ISP_SPI_WDATA, ((MS_U16)u32FlashAddr) >> 8);

                            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
                            {
                                goto HAL_Flash_WriteProtect_Area_return;
                            }

                            ISP_WRITE(REG_ISP_SPI_WDATA, u32FlashAddr & 0xFF);

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_Flash_WriteProtect_Area_return;
            }

                            ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222); // disable trigger mode

            ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

                            bRet = _HAL_SERFLASH_WaitWriteDone();
                        }
                        break;
                    }
                }
            }

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_Flash_WriteProtect_Area_return;
            }

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_Flash_WriteProtect_Area_return;
            }

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WRSR); // WRSR
        }

        ISP_WRITE(REG_ISP_SPI_WDATA, SF_SR_SRWD | u8BlockProtectBits); // [4:2] or [5:2] protect blocks
    }

    bRet = _HAL_SERFLASH_WaitWriteDone();

HAL_Flash_WriteProtect_Area_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

    if (bEnableAllArea)// _REVIEW_
    {
        _HAL_SERFLASH_ActiveFlash_Set_HW_WP(bEnableAllArea);
    }

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    return bRet;
}

MS_BOOL HAL_SERFLASH_WriteProtect(MS_BOOL bEnable)
{
// Note: Temporarily don't call this function until MSTV_Tool ready
#if 1
    MS_BOOL bRet = FALSE;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(%d)\n", __FUNCTION__, bEnable));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_SERFLASH_ActiveFlash_Set_HW_WP(DISABLE);
    //MsOS_DelayTask(bEnable ? 5 : 20); // when disable WP, delay more time

    _HAL_ISP_Enable();

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_WriteProtect_return;
    }
    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_WriteProtect_return;
    }
    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WRSR); // WRSR

    if (bEnable)
    {
        if (_hal_SERFLASH.u16FlashType == FLASH_IC_AT26DF321)
        {
            ISP_WRITE(REG_ISP_SPI_WDATA, SERFLASH_WRSR_BLK_PROTECT); // SPRL 1 -> 0

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_SERFLASH_WriteProtect_return;
            }

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_SERFLASH_WriteProtect_return;
            }

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WRSR); // WRSR
        }

        ISP_WRITE(REG_ISP_SPI_WDATA, SF_SR_SRWD | SERFLASH_WRSR_BLK_PROTECT); // SF_SR_SRWD: SRWD Status Register Write Protect
    }
    else
    {
        MS_U16 u16temp;
        u16temp = PM_READ(REG_PM_SPI_GPIO);
        u16temp |= PM_SPI_WP_DISABLE;
        PM_WRITE(REG_PM_SPI_GPIO,u16temp);
        if (_hal_SERFLASH.u16FlashType == FLASH_IC_AT26DF321)
        {
            ISP_WRITE(REG_ISP_SPI_WDATA, 0 << 2); // [4:2] or [5:2] protect blocks // SPRL 1 -> 0

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_SERFLASH_WriteProtect_return;
            }

            ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_SERFLASH_WriteProtect_return;
            }

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WRSR); // WRSR
        }

        ISP_WRITE(REG_ISP_SPI_WDATA, SF_SR_SRWD | 0 << 2); // [4:2] or [5:2] protect blocks
    }

    bRet = _HAL_SERFLASH_WaitWriteDone();

HAL_SERFLASH_WriteProtect_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

    if (bEnable) // _REVIEW_
    {
        _HAL_SERFLASH_ActiveFlash_Set_HW_WP(bEnable);
    }

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    return bRet;
#else
    return TRUE;
#endif
}

MS_BOOL HAL_SERFLASH_ReadID(MS_U8 *pu8Data, MS_U32 u32Size)
{
    // HW doesn't support ReadID on MX/ST flash; use trigger mode instead.
    MS_BOOL bRet = FALSE;
    MS_U32 u32I;
    MS_U8 *u8ptr = pu8Data;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if(!_HAL_SERFLASH_WaitWriteDone())
    {
        goto HAL_SERFLASH_ReadID_return;
    }

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadID_return;
    }
    // SFSH_RIU_REG16(REG_SFSH_SPI_COMMAND) = ISP_SPI_CMD_RDID; // RDID
    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333); // enable trigger mode

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_RDID); // RDID
    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadID_return;
    }

    for ( u32I = 0; u32I < u32Size; u32I++ )
    {
        ISP_WRITE(REG_ISP_SPI_RDREQ, ISP_SPI_RDREQ); // SPI read request

        if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_ReadID_return;
        }

        u8ptr[u32I] = ISP_READ(REG_ISP_SPI_RDATA);

        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk(" 0x%02X", u8ptr[u32I]));
    }
    bRet = TRUE;

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222); // disable trigger mode


HAL_SERFLASH_ReadID_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("\n"));

    return bRet;
}

MS_U64 HAL_SERFLASH_ReadUID(void)
{
    #define READ_UID_SIZE 8
    MS_U8  u8I;
    MS_U8  u8ptr[READ_UID_SIZE];
    MS_U8  u8Size = READ_UID_SIZE;

    MS_U64   u64FlashUId = 0;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if(!_HAL_SERFLASH_WaitWriteDone())
    {
        goto HAL_SERFLASH_ReadUID_return;
    }

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadUID_return;
    }
    // SFSH_RIU_REG16(REG_SFSH_SPI_COMMAND) = ISP_SPI_CMD_RDID; // RDID
    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333); // enable trigger mode

    if(_hal_SERFLASH.u16FlashType == FLASH_IC_EN25QH16)
    {
        ISP_WRITE(REG_ISP_SPI_WDATA, 0x5A); // RDUID
        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_ReadUID_return;
        }
        for(u8I = 0;u8I < 2;u8I++)
        {
            ISP_WRITE(REG_ISP_SPI_WDATA, 0x00);
            if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
            {
                goto HAL_SERFLASH_ReadUID_return;
            }
        }
        ISP_WRITE(REG_ISP_SPI_WDATA, 0x80); //start address
        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_ReadUID_return;
        }
        ISP_WRITE(REG_ISP_SPI_WDATA, 0xFF);
        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_ReadUID_return;
        }
    }
    else if( _hal_SERFLASH.u16FlashType == FLASH_IC_W25Q16)
    {
        ISP_WRITE(REG_ISP_SPI_WDATA, 0x4B); // RDUID
        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_ReadUID_return;
        }
        for(u8I = 0;u8I < 4;u8I++)
        {
            ISP_WRITE(REG_ISP_SPI_WDATA, 0xFF); // RDUID
            if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
            {
                goto HAL_SERFLASH_ReadUID_return;
            }
        }
    }
    SPI_FLASH_CMD(ISP_SPI_CMD_READ);

    for ( u8I = 0; u8I < u8Size; u8I++ )
    {
        ISP_WRITE(REG_ISP_SPI_RDREQ, ISP_SPI_RDREQ); // SPI read request

        if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_ReadUID_return;
        }

        u8ptr[u8I] = ISP_READ(REG_ISP_SPI_RDATA);
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s, %d 0x%02X\n",__FUNCTION__, u8I, u8ptr[u8I]));
    }

    for(u8I = 0;u8I < 8;u8I++)
    {
        u64FlashUId <<= 8;
        u64FlashUId += u8ptr[u8I];
    }

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222); // disable trigger mode


HAL_SERFLASH_ReadUID_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("\n"));

    return u64FlashUId;
}


MS_BOOL HAL_SERFLASH_ReadREMS4(MS_U8 * pu8Data, MS_U32 u32Size)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32Index;
    MS_U8 *u8ptr = pu8Data;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if ( !_HAL_SERFLASH_WaitWriteDone() )
    {
        goto HAL_SERFLASH_ReadREMS4_return;
    }

    if ( !_HAL_SERFLASH_WaitWriteCmdRdy() )
    {
        goto HAL_SERFLASH_ReadREMS4_return;
    }

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333);           // Enable trigger mode

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_REMS4);   // READ_REMS4 for new MXIC Flash

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SERFLASH_ReadREMS4_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_WDATA_DUMMY);
    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadREMS4_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_WDATA_DUMMY);
    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadREMS4_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, 0x00); // if ADD is 0x00, MID first. if ADD is 0x01, DID first
    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadREMS4_return;
    }

    for ( u32Index = 0; u32Index < u32Size; u32Index++ )
    {
        ISP_WRITE(REG_ISP_SPI_RDREQ, ISP_SPI_RDREQ);   // SPI read request

        if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_ReadREMS4_return;
        }

        u8ptr[u32Index] = ISP_READ(REG_ISP_SPI_RDATA);

        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk(" 0x%02X",  u8ptr[u32Index]));
    }

    bRet = TRUE;

        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

        ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222);     // disable trigger mode

HAL_SERFLASH_ReadREMS4_return:

        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

        _HAL_ISP_Disable();

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

return bRet;

}


MS_BOOL HAL_SERFLASH_ReadStatusReg(MS_U8 *pu8StatusReg)
{
    MS_BOOL bRet = FALSE;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    *pu8StatusReg = 0xFF;

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadStatusReg_return;
    }
    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_RDSR); // RDSR

    ISP_WRITE(REG_ISP_SPI_RDREQ, 0x01); // SPI read request

    if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadStatusReg_return;
    }

    *pu8StatusReg = ISP_READ(REG_ISP_SPI_RDATA);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk(" 0x%02X", *pu8StatusReg));

    bRet = TRUE;

HAL_SERFLASH_ReadStatusReg_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("\n"));

    return bRet;
}

MS_BOOL HAL_SERFLASH_ReadStatusReg2(MS_U8 *pu8StatusReg)
{
    MS_BOOL bRet = FALSE;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    *pu8StatusReg = 0x00;

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if ( !_HAL_SERFLASH_WaitWriteDone() )
    {
        goto HAL_SERFLASH_ReadStatusReg_return;
    }

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadStatusReg_return;
    }

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333);           // Enable trigger mode

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_RDSR2);   // RDSR2

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SERFLASH_ReadStatusReg_return;
    }

    ISP_WRITE(REG_ISP_SPI_RDREQ, 0x01); // SPI read request

    if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadStatusReg_return;
    }

    *pu8StatusReg = ISP_READ(REG_ISP_SPI_RDATA);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk(" 0x%02X", *pu8StatusReg));

    bRet = TRUE;

HAL_SERFLASH_ReadStatusReg_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222);     // disable trigger mode

    _HAL_ISP_Disable();

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("\n"));

    return bRet;
}


MS_BOOL HAL_SERFLASH_WriteStatusReg(MS_U16 u16StatusReg)
{
    MS_BOOL bRet = FALSE;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_WriteStatusReg_return;
    }

    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_WriteStatusReg_return;
    }

    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WRSR);   // WRSR

    SPI_WRITE_DATA(u16StatusReg);

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SERFLASH_WriteStatusReg_return;
    }

    SPI_WRITE_DATA((u16StatusReg>>8));

    bRet = TRUE;

HAL_SERFLASH_WriteStatusReg_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("\n"));

    return bRet;
}

MS_BOOL HAL_SPI_EnterIBPM(void)
{
	MS_BOOL bRet = FALSE;

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if ( !_HAL_SERFLASH_WaitWriteDone() )
    {
        goto HAL_SPI_EnableIBPM_return;
    }

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SPI_EnableIBPM_return;
    }

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333);           // Enable trigger mode

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_WPSEL);   // WPSEL

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SPI_EnableIBPM_return;
    }

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_RDSCUR);	// Read Security Register

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SPI_EnableIBPM_return;
    }

    ISP_WRITE(REG_ISP_SPI_RDREQ, 0x01); // SPI read request

    if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
    {
        goto HAL_SPI_EnableIBPM_return;
    }

    if((ISP_READ(REG_ISP_SPI_RDATA) & BIT(7)) == BIT(7))
    {
        bRet = TRUE;
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG,
                        printk("MXIC Security Register 0x%02X\n", ISP_READ(REG_ISP_SPI_RDATA)));
    }

HAL_SPI_EnableIBPM_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222);     // disable trigger mode

    _HAL_ISP_Disable();

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    return bRet;
}



MS_BOOL HAL_SPI_SingleBlockLock(MS_PHYADDR u32FlashAddr, MS_BOOL bLock)
{
    MS_BOOL bRet = FALSE;

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return bRet;
    }

    if ( _bIBPM != TRUE )
    {
        printk("%s not in Individual Block Protect Mode\n", __FUNCTION__);
        MsOS_ReleaseMutex(_s32SERFLASH_Mutex);
        return bRet;
    }

    _HAL_ISP_Enable();

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333);	// Enable trigger mode

    if( bLock )
    {
        ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_SBLK);   // Single Block Lock Protection
    }
    else
    {
        ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_SBULK);   // Single Block unLock Protection
    }

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x10)&0xFF)));
    if(!_HAL_SERFLASH_WaitWriteDataRdy())
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x08)&0xFF)));
    if(!_HAL_SERFLASH_WaitWriteDataRdy())
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x00)&0xFF)));
    if(!_HAL_SERFLASH_WaitWriteDataRdy())
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

#if defined (MS_DEBUG)
    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_RDBLOCK);	// Read Block Lock Status

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
       	goto HAL_SPI_SingleBlockLock_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x10)&0xFF)));
    if(!_HAL_SERFLASH_WaitWriteDataRdy())
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x08)&0xFF)));
    if(!_HAL_SERFLASH_WaitWriteDataRdy())
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x00)&0xFF)));
    if(!_HAL_SERFLASH_WaitWriteDataRdy())
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

    ISP_WRITE(REG_ISP_SPI_RDREQ, 0x01); // SPI read request

    if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

    if( bLock )
    {
        if( ISP_READ(REG_ISP_SPI_RDATA) == 0xFF )
            bRet = TRUE;
    }
    else
    {
        if( ISP_READ(REG_ISP_SPI_RDATA) == 0x00 )
            bRet = TRUE;
    }
#else//No Ceck
    bRet = TRUE;
#endif

HAL_SPI_SingleBlockLock_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222);     // disable trigger mode

    _HAL_ISP_Disable();

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    return bRet;
}


MS_BOOL HAL_SPI_GangBlockLock(MS_BOOL bLock)
{
    MS_BOOL bRet = FALSE;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(%d)\n", __FUNCTION__, bLock));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return bRet;
    }

    if ( _bIBPM != TRUE )
    {
        printk("%s not in Individual Block Protect Mode\n", __FUNCTION__);
        MsOS_ReleaseMutex(_s32SERFLASH_Mutex);
        return bRet;
    }

    _HAL_SERFLASH_ActiveFlash_Set_HW_WP(bLock);
    //MsOS_DelayTask(bLock ? 5 : 20); // when disable WP, delay more time

    _HAL_ISP_Enable();

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_WriteProtect_return;
    }

    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_WriteProtect_return;
    }

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333);	// Enable trigger mode

    if( bLock )
    {
        ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_GBLK);   // Gang Block Lock Protection
    }
    else
    {
        ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_GBULK);   // Gang Block unLock Protection
    }

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SERFLASH_WriteProtect_return;
    }

    bRet = TRUE;

HAL_SERFLASH_WriteProtect_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222);     // disable trigger mode

    _HAL_ISP_Disable();

    if (bLock) // _REVIEW_
    {
        _HAL_SERFLASH_ActiveFlash_Set_HW_WP(bLock);
    }

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    return bRet;
}

MS_U8 HAL_SPI_ReadBlockStatus(MS_PHYADDR u32FlashAddr)
{
    MS_U8 u8Val = 0xA5;

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    if (FALSE == MsOS_ObtainMutex(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return u8Val;
    }

    if ( _bIBPM != TRUE )
    {
        printk("%s not in Individual Block Protect Mode\n", __FUNCTION__);
        goto HAL_SPI_ReadBlockStatus_return;
    }

    _HAL_ISP_Enable();

    if ( !_HAL_SERFLASH_WaitWriteDone() )
    {
        goto HAL_SPI_ReadBlockStatus_return;
    }

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SPI_ReadBlockStatus_return;
    }

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333);          // Enable trigger mode

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_RDBLOCK);	// Read Block Lock Status

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
       	goto HAL_SPI_ReadBlockStatus_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x10)&0xFF)));
    if(!_HAL_SERFLASH_WaitWriteDataRdy())
    {
        goto HAL_SPI_ReadBlockStatus_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x08)&0xFF)));
    if(!_HAL_SERFLASH_WaitWriteDataRdy())
    {
        goto HAL_SPI_ReadBlockStatus_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x00)&0xFF)));
    if(!_HAL_SERFLASH_WaitWriteDataRdy())
    {
        goto HAL_SPI_ReadBlockStatus_return;
    }

    ISP_WRITE(REG_ISP_SPI_RDREQ, 0x01); // SPI read request

    if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
    {
        goto HAL_SPI_ReadBlockStatus_return;
    }

    u8Val = ISP_READ(REG_ISP_SPI_RDATA);

HAL_SPI_ReadBlockStatus_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222);     // disable trigger mode

    _HAL_ISP_Disable();

    MsOS_ReleaseMutex(_s32SERFLASH_Mutex);

    return u8Val;
}



#endif


MS_BOOL HAL_SERFLASH_WriteProtect_Area_Lookup(MS_U32 u32ProtectSpace, MS_U32 u32NonProtectSpace, MS_U8 *pu8BlockProtectBits)
{
    MS_U32 i;
    MS_U32 j;

    j=-1;
    for (i = 0;
        (0xFFFFFFFF != _hal_SERFLASH.pWriteProtectTable[i].u32LowerBound) && (0xFFFFFFFF != _hal_SERFLASH.pWriteProtectTable[i].u32UpperBound); // the end of write protect table
        i++
        )
       {
        if(_hal_SERFLASH.pWriteProtectTable[i].u32LowerBound == 0x0)
        {
            if((((_hal_SERFLASH.pWriteProtectTable[i].u32UpperBound-_hal_SERFLASH.pWriteProtectTable[i].u32LowerBound)>= u32ProtectSpace) && (_hal_SERFLASH.u32FlashSize-(_hal_SERFLASH.pWriteProtectTable[i].u32UpperBound-_hal_SERFLASH.pWriteProtectTable[i].u32LowerBound)))>= u32NonProtectSpace)
            {
                  if(((_hal_SERFLASH.pWriteProtectTable[j].u32UpperBound-_hal_SERFLASH.pWriteProtectTable[j].u32LowerBound) >= (_hal_SERFLASH.pWriteProtectTable[i].u32UpperBound-_hal_SERFLASH.pWriteProtectTable[i].u32LowerBound)) || (j==-1))
                   {
                      j = i;
                      *pu8BlockProtectBits = _hal_SERFLASH.pWriteProtectTable[j].u8BlockProtectBits;
                      DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG,printk("pu8BlockProtectBits = %x\n",*pu8BlockProtectBits));
                   }
              }
        }
    }

    if(j==-1)  //check Protect Bits
        return FALSE;
    else
        return TRUE;

}

MS_U32 HAL_SERFLASH_WriteProtect_Area_Boundary(void)
{

    MS_U32  i;
    MS_U32  k;

    MS_U8 u8stats = 0x00;
    k = -1;

    HAL_SERFLASH_ReadStatusReg(&u8stats);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG,printk("u8stats = %x\n",u8stats&_hal_SERFLASH.u8WrsrBlkProtect));

    for (i = 0;
        (0xFFFFFFFF != _hal_SERFLASH.pWriteProtectTable[i].u32LowerBound) && (0xFFFFFFFF != _hal_SERFLASH.pWriteProtectTable[i].u32UpperBound); // the end of write protect table
        i++
        )
    {
        if(_hal_SERFLASH.pWriteProtectTable[i].u8BlockProtectBits == (u8stats&_hal_SERFLASH.u8WrsrBlkProtect))
        {
            k = i;
            DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG,printk("u32UpperBound = %lx\n",_hal_SERFLASH.pWriteProtectTable[k].u32UpperBound));
            return _hal_SERFLASH.pWriteProtectTable[k].u32UpperBound;
        }
    }
    return FALSE;
}


MS_BOOL HAL_WriteByte(MS_U32 u32RegAddr, MS_U8 u8Val)
{
    if (!u32RegAddr)
    {
        printk("%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

    ((volatile MS_U8*)(_hal_isp.u32RiuBaseAddr))[(u32RegAddr << 1) - (u32RegAddr & 1)] = u8Val;
    return TRUE;
}

MS_BOOL HAL_Write2Byte(MS_U32 u32RegAddr, MS_U16 u16Val)
{
    if (!u32RegAddr)
    {
        printk("%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

    ((volatile MS_U16*)(_hal_isp.u32RiuBaseAddr))[u32RegAddr] = u16Val;
    return TRUE;
}

MS_U8 HAL_ReadByte(MS_U32 u32RegAddr)
{
    return ((volatile MS_U8*)(_hal_isp.u32RiuBaseAddr))[(u32RegAddr << 1) - (u32RegAddr & 1)];
}

MS_U16 HAL_Read2Byte(MS_U32 u32RegAddr)
{
    return ((volatile MS_U16*)(_hal_isp.u32RiuBaseAddr))[u32RegAddr];
}

#if 0
static void _HAL_FSP_SetWData(MS_U8 *pu8Data, MS_U8 u8DataSize)
{
    MS_U8 u8Index = 0;
    MS_U32 u32Addr = 0;
    for (u8Index = 0; u8Index < u8DataSize; u8Index++)
    {
        u32Addr = REG_FSP_WRITE_BUFF + REG8_FSP_WDATA + u8Index;
        HAL_WriteByte(u32Addr, pu8Data[u8Index]);
    }
}
#endif

static MS_U32 _Get_Time()
{
    struct timespec         ts;

    getnstimeofday(&ts);
    return ts.tv_sec* 1000+ ts.tv_nsec/1000000;
}


static void _HAL_FSP_WaitForDone(void)
{

    MS_U32 start_time = _Get_Time(); 
    //MS_U32 u32Timer;
    //SER_FLASH_TIME(u32Timer, SERFLASH_SAFETY_FACTOR * 30);


#define DEFAULT_FSP_TIMEOUT	2000

    ISP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_FIRE_MASK);	//Start FSP

    while(1)//Polling Done
    { 
        //if( (MsOS_GetSystemTime() - start_time) >=  DEFAULT_FSP_TIMEOUT )
        if( (_Get_Time() - start_time) >=  DEFAULT_FSP_TIMEOUT )
        {
            printk("%s() error : Time out!!!\n", __FUNCTION__);
            break;
        }

        if( (ISP_READ(REG_FSP_DONE_FLAG)&REG_FSP_DONE_MASK) == FSP_DONE_FLAG )
        {
            ISP_WRITE_MASK(REG_FSP_CLR_FLAG,FSP_CLR_FLAG,FSP_CLR_FLAG_MASK);//Clear Done Flag
            break;
        }
    }//while(!SER_FLASH_EXPIRE(u32Timer)); //add by eddie

}

void HAL_SERFLASH_ReadWordFlashByFSP(MS_U32 u32Addr, MS_U8 *pu8Buf)
{
    MS_U16 u16Data[2]={0};

    ISP_WRITE(REG_FSP_WD0,ISP_SPI_CMD_READ);

    ISP_WRITE_MASK(REG_FSP_WD1,FSP_WD1((MS_U8)(u32Addr>>16)),REG_FSP_WD1_MASK);
    ISP_WRITE_MASK(REG_FSP_WD2,FSP_WD2((MS_U8)(u32Addr>>8)),REG_FSP_WD2_MASK);
    ISP_WRITE_MASK(REG_FSP_WD3,FSP_WD3((MS_U8)(u32Addr)),REG_FSP_WD3_MASK);

    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE0(4),REG_FSP_WBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE0(4),REG_FSP_RBF_SIZE0_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_RDSR_1CMD,REG_FSP_CTRL1_RDSR_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_FSP(1),REG_FSP_CTRL0_FSP_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_RST(1),REG_FSP_CTRL0_RST_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_ACE(1),REG_FSP_CTRL0_ACE_MASK);

    _HAL_FSP_WaitForDone();

    u16Data[0] = ISP_READ(REG_FSP_RD0);
    u16Data[1] = ISP_READ(REG_FSP_RD2);

    *(pu8Buf + 0) = (MS_U8)u16Data[0]&0xFF;
    *(pu8Buf + 1) = (MS_U8)(u16Data[0]>>8);
    *(pu8Buf + 2) = (MS_U8)u16Data[1]&0xFF;
    *(pu8Buf + 3) = (MS_U8)(u16Data[1]>>8);
}

void HAL_SERFLASH_ProgramFlashByFSP(MS_U32 u32Addr, MS_U32 u32Data)
{
    ISP_WRITE(REG_FSP_WD0,ISP_SPI_CMD_WREN);
    ISP_WRITE_MASK(REG_FSP_WD0,FSP_WD1(0x02),REG_FSP_WD1_MASK);

    ISP_WRITE_MASK(REG_FSP_WD2,FSP_WD2((MS_U8)(u32Addr>>16)),REG_FSP_WD2_MASK);
    ISP_WRITE_MASK(REG_FSP_WD3,FSP_WD3((MS_U8)(u32Addr>>8)),REG_FSP_WD3_MASK);
    ISP_WRITE_MASK(REG_FSP_WD4,FSP_WD4((MS_U8)(u32Addr)),REG_FSP_WD4_MASK);

    ISP_WRITE_MASK(REG_FSP_WD5,FSP_WD5((MS_U8)(u32Data >> 0)),REG_FSP_WD5_MASK);
    ISP_WRITE_MASK(REG_FSP_WD6,FSP_WD6((MS_U8)(u32Data >> 8)),REG_FSP_WD6_MASK);
    ISP_WRITE_MASK(REG_FSP_WD7,FSP_WD7((MS_U8)(u32Data >> 16)),REG_FSP_WD7_MASK);
    ISP_WRITE_MASK(REG_FSP_WD8,FSP_WD8((MS_U8)(u32Data >> 24)),REG_FSP_WD8_MASK);
    ISP_WRITE_MASK(REG_FSP_WD9,FSP_WD9((MS_U8)(0x05)),REG_FSP_WD9_MASK);

    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE1(8),REG_FSP_WBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE2(1),REG_FSP_WBF_SIZE2_MASK);

    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE0(0),REG_FSP_RBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE2(1),REG_FSP_RBF_SIZE2_MASK)

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_2CMD(1),REG_FSP_CTRL1_2CMD_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_3CMD(1),REG_FSP_CTRL1_3CMD_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_ACS(1),REG_FSP_CTRL1_ACS_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_RDSR_3CMD,REG_FSP_CTRL1_RDSR_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_2CMD(1),REG_FSP_CTRL1_2CMD_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_3CMD(1),REG_FSP_CTRL1_3CMD_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_ACS(1),REG_FSP_CTRL1_ACS_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_RDSR_3CMD,REG_FSP_CTRL1_RDSR_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_FSP(1),REG_FSP_CTRL0_FSP_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_RST(1),REG_FSP_CTRL0_RST_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_ACE(1),REG_FSP_CTRL0_ACE_MASK);	

    _HAL_FSP_WaitForDone();
}

MS_U8 HAL_SERFLASH_ReadStatusByFSP(void)
{
    ISP_WRITE(REG_FSP_WD0,ISP_SPI_CMD_RDSR);

    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE1(0),REG_FSP_WBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE2(0),REG_FSP_WBF_SIZE2_MASK);

    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE0(1),REG_FSP_RBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE2(0),REG_FSP_RBF_SIZE2_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_RST(0),REG_FSP_CTRL0_RST_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_ACS(1),REG_FSP_CTRL1_ACS_MASK);


    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_RDSR_1CMD,REG_FSP_CTRL1_RDSR_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_ACS(1),REG_FSP_CTRL1_ACS_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_FSP(1),REG_FSP_CTRL0_FSP_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_RST(1),REG_FSP_CTRL0_RST_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_ACE(1),REG_FSP_CTRL0_ACE_MASK);

    _HAL_FSP_WaitForDone();

    return (MS_U8)ISP_READ(REG_FSP_RD0);
}

static void HAL_SERFLASH_EraseByFSP(MS_U32 u32Addr, MS_U8 EraseCmd)
{
    ISP_WRITE(REG_FSP_WD0,ISP_SPI_CMD_WREN);

    ISP_WRITE_MASK(REG_FSP_WD1,FSP_WD1(EraseCmd),REG_FSP_WD1_MASK);
    ISP_WRITE_MASK(REG_FSP_WD2,FSP_WD2((MS_U8)(u32Addr>>16)),REG_FSP_WD2_MASK);
    ISP_WRITE_MASK(REG_FSP_WD3,FSP_WD3((MS_U8)(u32Addr>>8)),REG_FSP_WD3_MASK);
    ISP_WRITE_MASK(REG_FSP_WD4,FSP_WD4((MS_U8)(u32Addr)),REG_FSP_WD4_MASK);

    ISP_WRITE(REG_FSP_WD5,ISP_SPI_CMD_RDSR);

    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE1(4),REG_FSP_WBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_WBF_SIZE,FSP_WBF_SIZE2(1),REG_FSP_WBF_SIZE2_MASK);

    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE0(0),REG_FSP_RBF_SIZE0_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    ISP_WRITE_MASK(REG_FSP_RBF_SIZE,FSP_RBF_SIZE2(1),REG_FSP_RBF_SIZE2_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_2CMD(1),REG_FSP_CTRL1_2CMD_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_3CMD(1),REG_FSP_CTRL1_3CMD_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_ACS(1),REG_FSP_CTRL1_ACS_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL1_RDSR_3CMD,REG_FSP_CTRL1_RDSR_MASK);

    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_FSP(1),REG_FSP_CTRL0_FSP_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_RST(1),REG_FSP_CTRL0_RST_MASK);
    ISP_WRITE_MASK(REG_FSP_CTRL,FSP_CTRL0_ACE(1),REG_FSP_CTRL0_ACE_MASK);	
    
    _HAL_FSP_WaitForDone();
}

void HAL_SERFLASH_EraseSectorByFSP(MS_U32 u32Addr) //4 // erase 4K bytes, need to aligned in 4K boundary
{
    HAL_SERFLASH_EraseByFSP(u32Addr,0x20);
}

void HAL_SERFLASH_EraseBlock32KByFSP(MS_U32 u32Addr) //4 // erase 32K bytes, need to aligned in 4K boundary
{
    HAL_SERFLASH_EraseByFSP(u32Addr,0x52);
}

void HAL_SERFLASH_EraseBlock64KByFSP(MS_U32 u32Addr) //4 // erase 64K bytes, need to aligned in 4K boundary
{
    HAL_SERFLASH_EraseByFSP(u32Addr,0xD8);
}

