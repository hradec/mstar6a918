#ifndef __eMMC_NAPOLI_LINUX__
#define __eMMC_NAPOLI_LINUX__

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/dma-mapping.h>
#include <linux/mmc/host.h>
#include <linux/scatterlist.h>
#include <mstar/mstar_chip.h>
#include <mach/io.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif
#include "chip_int.h"

#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/kthread.h>

#include "chip_int.h"
#include "chip_setup.h"
#if defined(CONFIG_ARM)
#include <mstar/mstar_chip.h>
#endif

//=====================================================
// tool-chain attributes
//===================================================== 
//[FIXME] -->
#define eMMC_CACHE_LINE                 0x80 // [FIXME]

#define eMMC_PACK0
#define eMMC_PACK1                      __attribute__((__packed__))
#define eMMC_ALIGN0
#define eMMC_ALIGN1                     __attribute__((aligned(eMMC_CACHE_LINE)))
// <-- [FIXME]

//=====================================================
// HW registers
//=====================================================
#define REG_OFFSET_SHIFT_BITS           2

#define REG_FCIE_U16(Reg_Addr)          (*(volatile U16*)(Reg_Addr))
#define GET_REG_ADDR(x, y)              ((x)+((y) << REG_OFFSET_SHIFT_BITS))

#define REG_FCIE(reg_addr)              REG_FCIE_U16(reg_addr)
#define REG_FCIE_W(reg_addr, val)       REG_FCIE(reg_addr) = (val)
#define REG_FCIE_R(reg_addr, val)       val = REG_FCIE(reg_addr)
#define REG_FCIE_SETBIT(reg_addr, val)  REG_FCIE(reg_addr) |= (val)
#define REG_FCIE_CLRBIT(reg_addr, val)  REG_FCIE(reg_addr) &= ~(val)
#define REG_FCIE_W1C(reg_addr, val)     REG_FCIE_W(reg_addr, REG_FCIE(reg_addr)&(val))

//------------------------------
#define RIU_PM_BASE                     (IO_ADDRESS(0x1F000000))
#define RIU_BASE                        (IO_ADDRESS(0x1F200000))

#define REG_BANK_FCIE0                  0x8980
#define REG_BANK_FCIE1                  0x89E0
#define REG_BANK_FCIE2                  0x8A00
#define REG_BANK_FCIEPOWERSAVEMODE      0x11D80

#define FCIE0_BASE                      GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE0)
#define FCIE1_BASE                      GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE1)
#define FCIE2_BASE                      GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE2)
#define FCIE_POWEER_SAVE_MODE_BASE      GET_REG_ADDR(RIU_BASE, REG_BANK_FCIEPOWERSAVEMODE)

#define FCIE_REG_BASE_ADDR              FCIE0_BASE
#define FCIE_CIFC_BASE_ADDR             FCIE1_BASE
#define FCIE_CIFD_BASE_ADDR             FCIE2_BASE

#include "eMMC_reg.h"

//--------------------------------clock gen------------------------------------
#define REG_BANK_CLKGEN0                0x0580	// (0x100B - 0x1000) x 80h
#define CLKGEN0_BASE                    GET_REG_ADDR(RIU_BASE, REG_BANK_CLKGEN0)

#define reg_ckg_fcie                    GET_REG_ADDR(CLKGEN0_BASE, 0x64)
#define BIT_FCIE_CLK_GATING             BIT0
#define BIT_FCIE_CLK_INVERSE            BIT1
#define BIT_CLKGEN_FCIE_MASK            (BIT5|BIT4|BIT3|BIT2)
#define BIT_FCIE_CLK_SRC_SEL            BIT6

//--------------------------------chiptop--------------------------------------
#define REG_BANK_CHIPTOP                0x0F00	// (0x101E - 0x1000) x 80h
#define PAD_CHIPTOP_BASE                GET_REG_ADDR(RIU_BASE, REG_BANK_CHIPTOP)

#define reg_chiptop_0x08                GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x08)
#define BIT_EMMC_DRV_RSTZ               BIT0
#define BIT_EMMC_DRV_CLK                BIT1
#define BIT_EMMC_DRV_CMD                BIT2

#define reg_chiptop_0x0C                GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x0C)

#define reg_fcie2macro_bypass           GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x10)
#define BIT_FCIE2MACRO_BYPASS           BIT8

#define reg_test_mode                   GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x12)
#define reg_test_in_mode_mask           (BIT6|BIT5|BIT4)
#define reg_test_out_mode_mask          (BIT2|BIT1|BIT0)

#define reg_nand_pad_driving            GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x13)

#define reg_sd_use_bypass               GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x40)
#define BIT_SD_USE_BYPASS               BIT0

#define reg_chiptop_0x43                GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x43)
#define BIT_EMMC_RSTZ_VAL               BIT8

#define reg_chiptop_0x4F                GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x4F)
#define BIT_EMMC_RSTZ_EN                BIT2

#define reg_chiptop_0x50                GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x50)
#define BIT_ALL_PAD_IN                  BIT15

#define reg_sd_config                   GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x5A)
#define BIT_SD_CONFIG                   (BIT9|BIT8)

#define reg_emmc_config                 GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x6E)
#define BIT_EMMC_CONFIG_MASK            (BIT7|BIT6)
#define BIT_EMMC_CONFIG_MODE1            BIT6


#define reg_chiptop_0x64                GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x64)
#define BIT_CAADCONFIG                  BIT0
#define BIT_PCMADCONFIG                 BIT4
#define BIT_PCM2CTRLCONFIG              BIT3

#define reg_nand_config                 GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x6F)
#define BIT_NAND_CS1_EN                 BIT5
#define BIT_NAND_MODE                   (BIT7|BIT6)

#define reg_sdio_config                 GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x7B)
#define BIT_SDIO_CONFIG                 (BIT5|BIT4)


//--------------------------------emmc pll--------------------------------------
#define REG_BANK_EMMC_PLL               0x11F80	// (0x123F - 0x1000) x 80h
#define EMMC_PLL_BASE                   GET_REG_ADDR(RIU_BASE, REG_BANK_EMMC_PLL)

#define reg_emmcpll_0x03                GET_REG_ADDR(EMMC_PLL_BASE, 0x03)
#define BIT_CLK_PH_MASK                 (BIT0|BIT1|BIT2|BIT3)
#define BIT_DQ_PH_MASK                  (BIT7|BIT6|BIT5|BIT4)
#define BIT_CMD_PH_MASK                 (BIT11|BIT10|BIT9|BIT8)
#define BIT_SKEW4_MASK                  (BIT15|BIT14|BIT13|BIT12)

#define reg_emmcpll_fbdiv               GET_REG_ADDR(EMMC_PLL_BASE, 0x04)
#define reg_emmcpll_pdiv                GET_REG_ADDR(EMMC_PLL_BASE, 0x05)
#define reg_emmc_pll_reset              GET_REG_ADDR(EMMC_PLL_BASE, 0x06)
#define reg_emmc_pll_test               GET_REG_ADDR(EMMC_PLL_BASE, 0x07)
#define reg_ddfset_15_00                GET_REG_ADDR(EMMC_PLL_BASE, 0x18)
#define reg_ddfset_23_16                GET_REG_ADDR(EMMC_PLL_BASE, 0x19)
#define reg_emmc_test                   GET_REG_ADDR(EMMC_PLL_BASE, 0x1A)
#define reg_atop_patch                  GET_REG_ADDR(EMMC_PLL_BASE, 0x1C)
#define BIT_HS200_PATCH                 BIT0 
#define BIT_HS_RSP_META_PATCH_HW        BIT2
#define BIT_HS_D0_META_PATCH_HW         BIT4
#define BIT_HS_DIN0_PATCH               BIT5
#define BIT_HS_EMMC_DQS_PATCH           BIT6
#define BIT_HS_RSP_MASK_PATCH           BIT7
#define BIT_DDR_RSP_PATCH               BIT8  
#define BIT_ATOP_PATCH_MASK            (BIT0|BIT1|BIT2|BIT4|BIT5|BIT6|BIT7|BIT8)

#define reg_emmc_tigger                 GET_REG_ADDR(EMMC_PLL_BASE, 0x20)
#define BIT_EMMC_TRIGGER_MASK           (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7|BIT8)

//----------------------------------ACP------------------------------------------

#define REG_BANK_100E		0x700
#define REG_100E_BASE		GET_REG_ADDR(RIU_BASE, REG_BANK_100E)

#define reg_100E_Rx03		GET_REG_ADDR(REG_100E_BASE, 0x3)
#define reg_100E_Rx04		GET_REG_ADDR(REG_100E_BASE, 0x4)
#define reg_100E_Rx10		GET_REG_ADDR(REG_100E_BASE, 0x10)
#define reg_100E_Rx11		GET_REG_ADDR(REG_100E_BASE, 0x11)
#define reg_100E_Rx12		GET_REG_ADDR(REG_100E_BASE, 0x12)
#define reg_100E_Rx13		GET_REG_ADDR(REG_100E_BASE, 0x13)
#define reg_100E_Rx14		GET_REG_ADDR(REG_100E_BASE, 0x14)
#define reg_100E_Rx15		GET_REG_ADDR(REG_100E_BASE, 0x15)
#define reg_100E_Rx16		GET_REG_ADDR(REG_100E_BASE, 0x16)

#define reg_emmc_acp_miu_sel	reg_100E_Rx16

#define BIT_EMMC_ACP_MIU0_SEL	0x2812

#define BIT_EMMC_ACP_MIU1_SEL	0xA812

#define REG_RQ_MASK			(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7)

#define REG_BANK_1239		0x11C80
#define REG_1239_BASE		GET_REG_ADDR(RIU_BASE, REG_BANK_1239)

#define reg_1239_Rx16		GET_REG_ADDR(REG_1239_BASE, 0x16)
#define reg_1239_Rx26		GET_REG_ADDR(REG_1239_BASE, 0x26)

#define REG_BANK_ACP_BRIGE	0xC00			//(0x1018 - 0x1000) x 0x80

#define REG_ACP_BRIGE_BASE	GET_REG_ADDR(RIU_BASE, REG_BANK_ACP_BRIGE)

#define reg_acp_idle		GET_REG_ADDR(REG_ACP_BRIGE_BASE, 0x4E)

#define BIT_ACP_IDLE_STS	BIT8

#ifdef CONFIG_MP_ACP_L2
#define CONFIG_ENABLE_EMMC_ACP		1
#endif

#if defined(CONFIG_ENABLE_EMMC_ACP) && CONFIG_ENABLE_EMMC_ACP
#undef eMMC_SET_ACP_MIU0
#undef eMMC_SET_ACP_MIU1

#define eMMC_SET_ACP_MIU0()  REG_FCIE_W(reg_emmc_acp_miu_sel, BIT_EMMC_ACP_MIU0_SEL)
#define eMMC_SET_ACP_MIU1()  REG_FCIE_W(reg_emmc_acp_miu_sel, BIT_EMMC_ACP_MIU1_SEL)

#endif

//--------------------------------clock gen------------------------------------
#define BIT_FCIE_CLK_20M                0x1
#define BIT_FCIE_CLK_27M                0x2
#define BIT_FCIE_CLK_32M                0x3
#define BIT_FCIE_CLK_36M                0x4
#define BIT_FCIE_CLK_40M                0x5
#define BIT_FCIE_CLK_43_2M              0x6
#define BIT_FCIE_CLK_54M                0x7
#define BIT_FCIE_CLK_62M                0x8
#define BIT_FCIE_CLK_72M                0x9
#define BIT_FCIE_CLK_EMMC_PLL           0xA
#define BIT_FCIE_CLK_86M                0xB
                                    //	0xC
#define BIT_FCIE_CLK_300K               0xD
#define BIT_FCIE_CLK_XTAL               0xE
#define BIT_FCIE_CLK_48M                0xF

#define eMMC_PLL_FLAG         0x80
#define eMMC_PLL_CLK__20M	  (0x01|eMMC_PLL_FLAG)
#define eMMC_PLL_CLK__27M	  (0x02|eMMC_PLL_FLAG)
#define eMMC_PLL_CLK__32M	  (0x03|eMMC_PLL_FLAG)
#define eMMC_PLL_CLK__36M	  (0x04|eMMC_PLL_FLAG)
#define eMMC_PLL_CLK__40M	  (0x05|eMMC_PLL_FLAG)
#define eMMC_PLL_CLK__48M	  (0x06|eMMC_PLL_FLAG)
#define eMMC_PLL_CLK__52M	  (0x07|eMMC_PLL_FLAG)
#define eMMC_PLL_CLK__62M	  (0x08|eMMC_PLL_FLAG)
#define eMMC_PLL_CLK__72M	  (0x09|eMMC_PLL_FLAG)
#define eMMC_PLL_CLK__80M	  (0x0A|eMMC_PLL_FLAG)
#define eMMC_PLL_CLK__86M	  (0x0B|eMMC_PLL_FLAG)
#define eMMC_PLL_CLK_100M	  (0x0C|eMMC_PLL_FLAG)
#define eMMC_PLL_CLK_120M	  (0x0D|eMMC_PLL_FLAG)
#define eMMC_PLL_CLK_140M	  (0x0E|eMMC_PLL_FLAG)
#define eMMC_PLL_CLK_160M	  (0x0F|eMMC_PLL_FLAG)
#define eMMC_PLL_CLK_200M	  (0x10|eMMC_PLL_FLAG)

#define PLL_SKEW4_CNT               9
#define eMMC_FCIE_VALID_CLK_CNT     3 // FIXME
#define MIN_OK_SKEW_CNT             5
extern  U8 gau8_FCIEClkSel[];
extern  U8 gau8_eMMCPLLSel_52[];
extern  U8 gau8_eMMCPLLSel_200[]; // for DDR52 or HS200

typedef eMMC_PACK0 struct _eMMC_FCIE_ATOP_SET {

    U32 u32_ScanResult;
	U8  u8_Clk;
	U8  u8_Reg2Ch, u8_Skew4;

} eMMC_PACK1 eMMC_FCIE_ATOP_SET_t;

// ----------------------------------------------
#define eMMC_RST_L()                    { REG_FCIE_SETBIT(reg_chiptop_0x4F, BIT_EMMC_RSTZ_EN);  \
                                          REG_FCIE_CLRBIT(reg_chiptop_0x43, BIT_EMMC_RSTZ_VAL); }
#define eMMC_RST_H()                    { REG_FCIE_SETBIT(reg_chiptop_0x4F, BIT_EMMC_RSTZ_EN);\
                                          REG_FCIE_SETBIT(reg_chiptop_0x43, BIT_EMMC_RSTZ_VAL);}

//--------------------------------power saving mode----------------------------
#define REG_BANK_PM_SLEEP               (0x700)
#define PM_SLEEP_REG_BASE_ADDR          GET_REG_ADDR(RIU_PM_BASE, REG_BANK_PM_SLEEP)
#define reg_pwrgd_int_glirm             GET_REG_ADDR(PM_SLEEP_REG_BASE_ADDR, 0x61)
#define BIT_PWRGD_INT_GLIRM_EN          BIT9
#define BIT_PWEGD_INT_GLIRM_MASK        (BIT15|BIT14|BIT13|BIT12|BIT11|BIT10)

//=====================================================
// API declarations
//=====================================================
extern  U32 eMMC_hw_timer_delay(U32 u32us);
extern  U32 eMMC_hw_timer_sleep(U32 u32ms);

#define eMMC_HW_TIMER_HZ               1000000
#define FCIE_eMMC_DISABLE               0
#define FCIE_eMMC_DDR                   1
#define FCIE_eMMC_SDR                   2
#define FCIE_eMMC_BYPASS                3 // never use this
#define FCIE_eMMC_TMUX                  4
#define FCIE_eMMC_HS200                 5
#define FCIE_eMMC_HS400		            6
#define FCIE_DEFAULT_PAD                FCIE_eMMC_SDR

extern  U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type);
extern  U32 eMMC_clock_setting(U16 u16_ClkParam);
extern  U32 eMMC_clock_gating(void);
extern void eMMC_set_WatchDog(U8 u8_IfEnable);
extern void eMMC_reset_WatchDog(void);
extern  U32 eMMC_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt);
extern  U32 eMMC_DMA_MAP_address(U32 u32_Buffer, U32 u32_ByteCnt, int mode);
extern  void eMMC_DMA_UNMAP_address(U32 u32_DMAAddr, U32 u32_ByteCnt, int mode);
extern void eMMC_flush_data_cache_buffer(U32 u32_DMAAddr, U32 u32_ByteCnt);
extern void eMMC_Invalidate_data_cache_buffer(U32 u32_DMAAddr, U32 u32_ByteCnt);
extern void eMMC_flush_miu_pipe(void);
extern  U32 eMMC_PlatformResetPre(void);
extern  U32 eMMC_PlatformResetPost(void);
extern  U32 eMMC_PlatformInit(void);
extern  U32 eMMC_PlatformDeinit(void);
extern  U32 eMMC_CheckIfMemCorrupt(void);
extern void eMMC_DumpPadClk(void);

#define eMMC_BOOT_PART_W                BIT0
#define eMMC_BOOT_PART_R                BIT1

extern U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP);
extern U32 eMMC_BootPartitionHandler_E(U16 u16_PartType);
extern U32 eMMC_hw_timer_start(void);
extern U32 eMMC_hw_timer_tick(void);
extern irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy); // [FIXME]
extern U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec);
extern struct mutex FCIE3_mutex;
extern void eMMC_LockFCIE(U8 *pu8_str);
extern void eMMC_UnlockFCIE(U8 *pu8_str);
extern int  mstar_mci_Housekeep(void *pData);
extern U32  mstar_SD_CardChange(void);

//=====================================================
// partitions config
//=====================================================
// every blk is 512 bytes (reserve 2MB-64KB for internal use)
#define eMMC_DRV_RESERVED_BLK_CNT       ((0x200000-0x10000)/0x200)

#define eMMC_CIS_NNI_BLK_CNT            2
#define eMMC_CIS_PNI_BLK_CNT            2
#define eMMC_TEST_BLK_CNT               (0x100000/0x200) // 1MB

#define eMMC_CIS_BLK_0                  (64*1024/512) // from 64KB
#define eMMC_NNI_BLK_0                  (eMMC_CIS_BLK_0+0)
#define eMMC_NNI_BLK_1                  (eMMC_CIS_BLK_0+1)
#define eMMC_PNI_BLK_0                  (eMMC_CIS_BLK_0+2)
#define eMMC_PNI_BLK_1                  (eMMC_CIS_BLK_0+3)
#define eMMC_DDRTABLE_BLK_0             (eMMC_CIS_BLK_0+4)
#define eMMC_DDRTABLE_BLK_1             (eMMC_CIS_BLK_0+5)
#define eMMC_HS200TABLE_BLK_0           (eMMC_CIS_BLK_0+6)
#define eMMC_HS200TABLE_BLK_1           (eMMC_CIS_BLK_0+7)
#define eMMC_HS400TABLE_BLK_0           (eMMC_CIS_BLK_0+8)
#define eMMC_HS400TABLE_BLK_1           (eMMC_CIS_BLK_0+9)
#define eMMC_DrvContext_BLK_0           (eMMC_CIS_BLK_0+10)
#define eMMC_DrvContext_BLK_1           (eMMC_CIS_BLK_0+11)
#define eMMC_ALLRSP_BLK_0               (eMMC_CIS_BLK_0+12)
#define eMMC_ALLRSP_BLK_1               (eMMC_CIS_BLK_0+13)
#define eMMC_BURST_LEN_BLK_0            (eMMC_CIS_BLK_0+14)

#define eMMC_CIS_BLK_END                eMMC_BURST_LEN_BLK_0
// last 1MB in reserved area, use for eMMC test
#define eMMC_TEST_BLK_0                 (eMMC_CIS_BLK_END+1)


//=====================================================
// Driver configs
//=====================================================
#define DRIVER_NAME                     "mstar_mci"
#define eMMC_UPDATE_FIRMWARE            0

#define eMMC_ST_PLAT                    0x80000000
// [CAUTION]: to verify IP and HAL code, defaut 0
#define IF_IP_VERIFY                    0 // [FIXME] -->

// need to eMMC_pads_switch
// need to eMMC_clock_setting
#define IF_FCIE_SHARE_IP                1

//------------------------------
#define FICE_BYTE_MODE_ENABLE           1 // always 1
#define ENABLE_eMMC_INTERRUPT_MODE      1
#define ENABLE_eMMC_RIU_MODE            0 // for debug cache issue

#define ENABLE_EMMC_POWER_SAVING_MODE   1

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,20)
#define ENABLE_EMMC_ASYNC_IO            1
#define ENABLE_EMMC_PRE_DEFINED_BLK     1
#endif

//------------------------------
// DDR48, DDR52, HS200, HS400
#define IF_DETECT_eMMC_DDR_TIMING           0 // DDR48 (digital macro)
#define ENABLE_eMMC_ATOP		            1 // DDR52 (ATOP)
#define ENABLE_eMMC_HS200		            1 // HS200
#define ENABLE_eMMC_HS400		            0 // HS400

#if ENABLE_eMMC_RIU_MODE || ENABLE_eMMC_ATOP
#undef IF_DETECT_eMMC_DDR_TIMING
#define IF_DETECT_eMMC_DDR_TIMING			0 
#endif

#define eMMC_IF_TUNING_TTABLE()          (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_TUNING_TTABLE)

//------------------------------
#define eMMC_FEATURE_RELIABLE_WRITE     1
#if eMMC_UPDATE_FIRMWARE
#undef  eMMC_FEATURE_RELIABLE_WRITE
#define eMMC_FEATURE_RELIABLE_WRITE     0
#endif

//------------------------------
#define eMMC_RSP_FROM_RAM               1
#define eMMC_BURST_LEN_AUTOCFG          1
#define eMMC_PROFILE_WR                 0

//------------------------------
#define eMMC_SECTOR_BUF_BYTECTN         eMMC_SECTOR_BUF_16KB
extern U8 gau8_eMMC_SectorBuf[];
extern U8 gau8_eMMC_PartInfoBuf[];


//=====================================================
// debug option
//=====================================================
#define eMMC_TEST_IN_DESIGN             0 // [FIXME]: set 1 to verify HW timer

#ifndef eMMC_DEBUG_MSG
#define eMMC_DEBUG_MSG                  1
#endif

/* Define trace levels. */
#define eMMC_DEBUG_LEVEL_ERROR          (1)    /* Error condition debug messages. */
#define eMMC_DEBUG_LEVEL_WARNING        (2)    /* Warning condition debug messages. */
#define eMMC_DEBUG_LEVEL_HIGH           (3)    /* Debug messages (high debugging). */
#define eMMC_DEBUG_LEVEL_MEDIUM         (4)    /* Debug messages. */
#define eMMC_DEBUG_LEVEL_LOW            (5)    /* Debug messages (low debugging). */

/* Higer debug level means more verbose */
#ifndef eMMC_DEBUG_LEVEL
#define eMMC_DEBUG_LEVEL                eMMC_DEBUG_LEVEL_WARNING
#endif

#if defined(eMMC_DEBUG_MSG) && eMMC_DEBUG_MSG
#define eMMC_printf(fmt, arg...)  printk(KERN_ERR fmt, ##arg)
#define eMMC_debug(dbg_lv, tag, str, ...)						\
	do {										\
		if (dbg_lv > eMMC_DEBUG_LEVEL)						\
			break;								\
		else if(eMMC_IF_TUNING_TTABLE())\
	        break;						\
	    else if(eMMC_IF_DISABLE_LOG())  \
			break;                      \
		else {									\
			if (tag)							\
				eMMC_printf("[ %s() Ln.%u ] ", __FUNCTION__, __LINE__);	\
											\
			eMMC_printf(str, ##__VA_ARGS__);				\
		}									\
	} while(0)
#else
#define eMMC_printf(...)
#define eMMC_debug(enable, tag, str, ...)	do{}while(0)
#endif

#define eMMC_die(str) {eMMC_printf("eMMC Die: %s() Ln.%u, %s \n", __FUNCTION__, __LINE__, str); \
	                   panic("\n");}

#define eMMC_stop() \
	while(1)  eMMC_reset_WatchDog();

#define REG_BANK_TIMER1                 0x1800
#define TIMER1_BASE                     GET_REG_ADDR(RIU_PM_BASE, REG_BANK_TIMER1)

#define TIMER1_ENABLE                   GET_REG_ADDR(TIMER1_BASE, 0x20)
#define TIMER1_HIT                      GET_REG_ADDR(TIMER1_BASE, 0x21)
#define TIMER1_MAX_LOW                  GET_REG_ADDR(TIMER1_BASE, 0x22)
#define TIMER1_MAX_HIGH                 GET_REG_ADDR(TIMER1_BASE, 0x23)
#define TIMER1_CAP_LOW                  GET_REG_ADDR(TIMER1_BASE, 0x24)
#define TIMER1_CAP_HIGH                 GET_REG_ADDR(TIMER1_BASE, 0x25)


//=====================================================
// unit for HW Timer delay (unit of us)
//=====================================================
#define HW_TIMER_DELAY_1us              1
#define HW_TIMER_DELAY_5us              5
#define HW_TIMER_DELAY_10us             10
#define HW_TIMER_DELAY_100us            100
#define HW_TIMER_DELAY_500us            500
#define HW_TIMER_DELAY_1ms              (1000 * HW_TIMER_DELAY_1us)
#define HW_TIMER_DELAY_5ms              (5    * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_10ms             (10   * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_100ms            (100  * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_500ms            (500  * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_1s               (1000 * HW_TIMER_DELAY_1ms)

//=====================================================
// set FCIE clock
//=====================================================
// [FIXME] -->
#define FCIE_SLOWEST_CLK	BIT_FCIE_CLK_300K
#define FCIE_SLOW_CLK		BIT_FCIE_CLK_20M
#define FCIE_DEFAULT_CLK	BIT_FCIE_CLK_48M // BIT_FCIE_CLK_36M

// used for test 
#define eMMC_PLL_CLK_SLOW	eMMC_PLL_CLK__20M
#define eMMC_PLL_CLK_FAST	eMMC_PLL_CLK_200M

// <-- [FIXME]
//=====================================================
// transfer DMA Address
//=====================================================
#define MIU_BUS_WIDTH_BITS              4 // Need to confirm
/*
 * Important:
 * The following buffers should be large enough for a whole eMMC block
 */
// FIXME, this is only for verifing IP
#define DMA_W_ADDR                      0x40C00000
#define DMA_R_ADDR                      0x40D00000
#define DMA_W_SPARE_ADDR                0x40E00000
#define DMA_R_SPARE_ADDR                0x40E80000
#define DMA_BAD_BLK_BUF                 0x40F00000


//=====================================================
// misc
//=====================================================
//#define BIG_ENDIAN
#define LITTLE_ENDIAN

#endif /* __eMMC_NAPOLI_LINUX__ */
