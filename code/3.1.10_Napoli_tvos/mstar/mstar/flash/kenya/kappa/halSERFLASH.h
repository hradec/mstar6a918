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

#ifndef _HAL_SERFLASH_H_
#define _HAL_SERFLASH_H_

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

// Flash IC
#define FLASH_IC_UNKNOWN        0x0000
#define FLASH_IC_SST25VF080B    0x0100      // 1M    SST
#define FLASH_IC_SST25VF016B    0x0101      // 2M
#define FLASH_IC_PM25LV080      0x0200      // 1M    PMC
#define FLASH_IC_PM25LV016      0x0201      // 2M
#define FLASH_IC_PM25LQ032C     0x0202
#define FLASH_IC_ST25P80_A      0x0300      // 1M    ST
#define FLASH_IC_ST25P16_A      0x0301      // 2M
#define FLASH_IC_STM25P32       0x0302      // 4M
#define FLASH_IC_AT26DF081A     0x0400      // 1M    ATMEL
#define FLASH_IC_AT26DF161      0x0401      // 2M
#define FLASH_IC_AT26DF321      0x0402      // 4M
#define FLASH_IC_MX25L8005      0x0500      // 1M    MXIC
#define FLASH_IC_MX25L8006E     0x0510      // 1M    MXIC
#define FLASH_IC_MX25L1605A     0x0501      // 2M
#define FLASH_IC_MX25L3205D     0x0502      // 4M
#define FLASH_IC_MX25L1655D     0x0511		// 2M for secure option
#define FLASH_IC_MX25L3255D     0x0512		// 4M for secure option
#define FLASH_IC_MX25L6405D     0x0503      // 8M
#define FLASH_IC_MX25L12805D    0x0504      // 16M
#define FLASH_IC_MX25L6445E     0x0505      // 8M
#define FLASH_IC_MX25L12845E    0x0506      // 16M
#define FLASH_IC_MX25L1606E		0x0507		// 2M
#define FLASH_IC_MX25L12855E    0x0508      // 16M
#define FLASH_IC_MX25L4006E     0x0509      // 512K    MXIC
#define FLASH_IC_MX25L6455E     0x050A      // 8M
#define FLASH_IC_NX25P80        0x0600      // 1M    NX
#define FLASH_IC_NX25P16        0x0601      // 2M
#define FLASH_IC_W25X80         0x0700      // 1M    WINB
#define FLASH_IC_W25X16         0x0701      // 2M
#define FLASH_IC_W25Q16         0x0711      // 2M
#define FLASH_IC_W25X32         0x0702      // 4M
#define FLASH_IC_W25Q32         0x0703      // 4M
#define FLASH_IC_W25X64         0x0704      // 8M
#define FLASH_IC_W25Q64         0x0705      // 8M
#define FLASH_IC_W25Q128        0x0706      // 16M
#define FLASH_IC_W25Q80         0x0707      // 1M
#define FLASH_IC_W25X40         0x0708      // 512K    WINB
#define FLASH_IC_S25FL008A      0x0800      // 1M    SPANSION
#define FLASH_IC_S25FL016A      0x0801      // 2M
#define FLASH_IC_S25FL128P      0x0802      // 16M
#define FLASH_IC_S25FL064       0x0803      // 8M
#define FLASH_IC_S25FL032       0x0804      // 4M
#define FLASH_IC_EN25B20T       0x0900      // 2M    EON
#define FLASH_IC_EN25B20B       0x0901      // 2M
#define FLASH_IC_EN25B10T       0x0902      // 1M
#define FLASH_IC_EN25B10B       0x0903      // 1M
#define FLASH_IC_EN25B32B       0x0904      // 4M    (Bottom Boot)
#define FLASH_IC_EN25B64B       0x0905      // 4M
#define FLASH_IC_EN25F80        0x0906      // 1M
#define FLASH_IC_EN25P16        0x0907      // 2M
#define FLASH_IC_EN25F10        0x0999      // 128K for secure boot
#define FLASH_IC_EN25Q64        0x0908      // 8M
#define FLASH_IC_EN25Q128       0x0909      // 16M
#define FLASH_IC_EN25F40        0x090A      // 512k
#define FLASH_IC_EN25Q32A       0x090B
#define FLASH_IC_EN25F16        0x090C      // 2M
#define FLASH_IC_EN25F32        0x090D      // 4M
#define FLASH_IC_EN25QH16       0x0914      // 2M
#define FLASH_IC_ESMT_F25L016A  0x0A00      // 2M
#define FLASH_IC_NUMONYX_M25PX16  0x0B00      // 2M
#define FLASH_IC_NUMONYX_N25Q32   0x0B01      // 4M
#define FLASH_IC_NUMONYX_N25Q64   0x0B02      // 8M
#define FLASH_IC_GD25Q16        0x0B01      // 2M
#define FLASH_IC_GD25Q32        0x0B02      // 4M
#define FLASH_IC_GD25Q64          0x0C01      // 2M
#define FLASH_IC_GD25S80          0x0C02
#define FLASH_IC_A25LM032         0x0D00
// Flash Manufacture ID
#define MID_MXIC                0xC2
#define MID_WB                  0xEF
#define MID_EON                 0x1C
#define MID_ST                  0x20
#define MID_SST                 0xBF
#define MID_PMC                 0x7F
#define MID_ATMEL               0x1F
#define MID_SPAN                0x01
#define MID_NUMONYX             0x20
#define MID_GD                  0xC8
#define MID_AM                 0x37
// Flash Storage Size
#define SIZE_1KB                0x400
#define SIZE_2KB                0x800
#define SIZE_4KB                0x1000
#define SIZE_8KB                0x2000
#define SIZE_16KB               0x4000
#define SIZE_32KB               0x8000
#define SIZE_64KB               0x10000
#define SIZE_128KB              0x20000
#define SIZE_256KB              0x40000
#define SIZE_512KB              0x80000
#define SIZE_1MB                0x100000
#define SIZE_2MB                0x200000
#define SIZE_4MB                0x400000
#define SIZE_8MB                0x800000
#define SIZE_16MB               0x1000000

#ifndef UNUSED
#define UNUSED(x) ((x)=(x))
#endif

#define MSOS_TYPE_LINUX 1

// ISP_DEV_SEL
#define ISP_DEV_PMC             BITS(2:0, 0)
#define ISP_DEV_NEXTFLASH       BITS(2:0, 1)
#define ISP_DEV_ST              BITS(2:0, 2)
#define ISP_DEV_SST             BITS(2:0, 3)
#define ISP_DEV_ATMEL           BITS(2:0, 4)

// ISP_SPI_ENDIAN_SEL
#define ISP_SPI_ENDIAN_BIG      BITS(0:0, 1)
#define ISP_SPI_ENDIAN_LITTLE   BITS(0:0, 0)


#define NUMBER_OF_SERFLASH_SECTORS          (_hal_SERFLASH.u32NumSec)
#define SERFLASH_SECTOR_SIZE                (_hal_SERFLASH.u32SecSize)
#define SERFLASH_PAGE_SIZE                  (_hal_SERFLASH.u16PageSize)
#define SERFLASH_MAX_CHIP_WR_DONE_TIMEOUT   (_hal_SERFLASH.u16MaxChipWrDoneTimeout)
#define SERFLASH_WRSR_BLK_PROTECT           (_hal_SERFLASH.u8WrsrBlkProtect)
#define ISP_DEV_SEL                         (_hal_SERFLASH.u16DevSel)
#define ISP_SPI_ENDIAN_SEL                  (_hal_SERFLASH.u16SpiEndianSel)


#define DEBUG_SER_FLASH(debug_level, x)     do { if (_u8SERFLASHDbgLevel >= (debug_level)) (x); } while(0)
#define WAIT_SFSH_CS_STAT()             {while(ISP_READ(REG_ISP_SPI_CHIP_SELE_BUSY) == SFSH_CHIP_SELE_SWITCH){}}

#define SERFLASH_BLOCK_SIZE   0x10000

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

typedef enum
{
    FLASH_ID0       = 0x00,
    FLASH_ID1       = 0x01,
    FLASH_ID2       = 0x02,
    FLASH_ID3       = 0x03
} EN_FLASH_ID;

typedef enum
{
    WP_AREA_EXACTLY_AVAILABLE,
    WP_AREA_PARTIALLY_AVAILABLE,
    WP_AREA_NOT_AVAILABLE,
    WP_TABLE_NOT_SUPPORT,
} EN_WP_AREA_EXISTED_RTN;

typedef struct
{
    MS_U8   u8BlockProtectBits;  // Block Protection Bits
    MS_U32  u32LowerBound;
    MS_U32  u32UpperBound;
} ST_WRITE_PROTECT;

typedef struct
{
    MS_U16  u16Start;           // Start block # of special block size
    MS_U16  u16End;             // End   block # of special block size
    MS_U32  au32SizeList[8];    // List of special block sizes. Total size must be equal to FLASH_BLOCK_SIZE
} ST_SPECIAL_BLOCKS;

typedef struct
{
    MS_U16  u16FlashType;   // flash type
    MS_U8   u8MID;          // Manufacture ID
    MS_U8   u8DID0;         // Device ID (memory type)
    MS_U8   u8DID1;         // Device ID (memory capacity)

    ST_WRITE_PROTECT   *pWriteProtectTable;
    ST_SPECIAL_BLOCKS  *pSpecialBlocks;

    MS_U32  u32FlashSize;               // Flash Size
    MS_U32  u32NumSec;                  // NUMBER_OF_SERFLASH_SECTORS                  // number of sectors
    MS_U32  u32SecSize;                 // SERFLASH_SECTOR_SIZE                                // sector size
    MS_U16  u16PageSize;                // SERFLASH_PAGE_SIZE                                    // page size
    MS_U16  u16MaxChipWrDoneTimeout;    // SERFLASH_MAX_CHIP_WR_DONE_TIMEOUT    // max timeout for chip write done
    MS_U8   u8WrsrBlkProtect;           // SERFLASH_WRSR_BLK_PROTECT                     // BP bits @ Serial Flash Status Register
    MS_U16  u16DevSel;                  // ISP_DEV_SEL                                                  // reg_device_select
    MS_U16  u16SpiEndianSel;            // ISP_SPI_ENDIAN_SEL                                     // reg_endian_sel_spi
    MS_BOOL b2XREAD;
} hal_SERFLASH_t;


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
extern MS_BOOL HAL_SERFLASH_SetCKG(SPI_DrvCKG eCkgSpi);
extern void HAL_SERFLASH_ClkDiv(SPI_DrvClkDiv eClkDivSpi);
extern MS_BOOL HAL_SERFLASH_SetMode(MS_BOOL bXiuRiu);
extern MS_BOOL HAL_SERFLASH_Set2XREAD(MS_BOOL b2XMode);
extern MS_BOOL HAL_SERFLASH_ChipSelect(MS_U8 u8FlashIndex);
extern void HAL_SERFLASH_Config(MS_U32 u32PMRegBaseAddr, MS_U32 u32NonPMRegBaseAddr, MS_U32 u32XiuBaseAddr);
extern void HAL_SERFLASH_Init(void);
extern void HAL_SERFLASH_SetGPIO(MS_BOOL bSwitch);
extern MS_BOOL HAL_SERFLASH_DetectType(void);
extern MS_BOOL HAL_SERFLASH_DetectSize(MS_U32  *u32FlashSize);
extern MS_BOOL HAL_SERFLASH_EraseChip(void);
extern MS_BOOL HAL_SERFLASH_AddressToBlock(MS_U32 u32FlashAddr, MS_U32 *pu32BlockIndex);
extern MS_BOOL HAL_SERFLASH_BlockToAddress(MS_U32 u32BlockIndex, MS_U32 *pu32FlashAddr);
extern MS_BOOL HAL_SERFLASH_BlockErase(MS_U32 u32StartBlock,MS_U32 u32EndBlock,MS_BOOL bWait);
extern MS_BOOL HAL_SERFLASH_SectorErase(MS_U32 u32SectorAddress);
extern MS_BOOL HAL_SERFLASH_CheckWriteDone(void);
extern MS_BOOL HAL_SERFLASH_Write(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data);
extern MS_BOOL HAL_SERFLASH_Read(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data);
extern EN_WP_AREA_EXISTED_RTN HAL_SERFLASH_WP_Area_Existed(MS_U32 u32UpperBound, MS_U32 u32LowerBound, MS_U8 *pu8BlockProtectBits);
extern MS_BOOL HAL_SERFLASH_WriteProtect_Area(MS_BOOL bEnableAllArea, MS_U8 u8BlockProtectBits);
extern MS_BOOL HAL_SERFLASH_WriteProtect(MS_BOOL bEnable);
extern MS_BOOL HAL_SERFLASH_ReadID(MS_U8 *pu8Data, MS_U32 u32Size);
extern MS_BOOL HAL_SERFLASH_ReadREMS4(MS_U8 * pu8Data, MS_U32 u32Size);
extern MS_BOOL HAL_SERFLASH_DMA(MS_U32 u32FlashStart, MS_U32 u32DRAMStart, MS_U32 u32Size);
extern MS_BOOL HAL_SERFLASH_ReadStatusReg(MS_U8 *pu8StatusReg);
extern MS_BOOL HAL_SERFLASH_ReadStatusReg2(MS_U8 *pu8StatusReg);
extern MS_BOOL HAL_SERFLASH_WriteStatusReg(MS_U16 u16StatusReg);
//#if MXIC_ONLY
extern MS_BOOL HAL_SPI_EnterIBPM(void);
extern MS_BOOL HAL_SPI_SingleBlockLock(MS_PHYADDR u32FlashAddr, MS_BOOL bLock);
extern MS_BOOL HAL_SPI_GangBlockLock(MS_BOOL bLock);
extern MS_U8 HAL_SPI_ReadBlockStatus(MS_PHYADDR u32FlashAddr);
extern MS_U64 HAL_SERFLASH_ReadUID(void);
//#endif//MXIC_ONLY
// DON'T USE THESE DIRECTLY
extern hal_SERFLASH_t _hal_SERFLASH;
extern MS_U8 _u8SERFLASHDbgLevel;
extern MS_BOOL _bIBPM;
extern MS_BOOL _bWPPullHigh;
extern MS_BOOL HAL_SERFLASH_WriteProtect_Area_Lookup(MS_U32 u32ProtectSpace, MS_U32 u32NonProtectSpace, MS_U8 *pu8BlockProtectBits);
extern MS_U32 HAL_SERFLASH_WriteProtect_Area_Boundary(void);

extern void HAL_SERFLASH_ReadWordFlashByFSP(MS_U32 u32Addr, MS_U8 *pu8Buf);
extern void HAL_SERFLASH_ProgramFlashByFSP(MS_U32 u32Addr, MS_U32 u32Data);
extern MS_U8 HAL_SERFLASH_ReadStatusByFSP(void);
extern void HAL_SERFLASH_EraseSectorByFSP(MS_U32 u32Addr);
extern void HAL_SERFLASH_EraseBlock32KByFSP(MS_U32 u32Addr);
extern void HAL_SERFLASH_EraseBlock64KByFSP(MS_U32 u32Addr);
#endif // _HAL_SERFLASH_H_
