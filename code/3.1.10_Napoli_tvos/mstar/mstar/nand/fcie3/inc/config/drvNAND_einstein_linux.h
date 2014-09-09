#ifndef __UNFD_EINSTEIN_LINUX_H__
#define __UNFD_EINSTEIN_LINUX_H__

#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <asm/memory.h>
#include <mach/io.h>

#define REG(Reg_Addr)                       (*(volatile U16*)(Reg_Addr))
#define REG_OFFSET_SHIFT_BITS               2
#define GET_REG_ADDR(x, y)                  (x+((y)<<REG_OFFSET_SHIFT_BITS))

#define REG_WRITE_UINT16(reg_addr, val)     REG(reg_addr) = val
#define REG_READ_UINT16(reg_addr, val)      val = REG(reg_addr)
#define HAL_WRITE_UINT16(reg_addr, val)     (REG(reg_addr) = val)
#define HAL_READ_UINT16(reg_addr, val)      val = REG(reg_addr)
#define REG_SET_BITS_UINT16(reg_addr, val)  REG(reg_addr) |= (val)
#define REG_CLR_BITS_UINT16(reg_addr, val)  REG(reg_addr) &= ~(val)
#define REG_W1C_BITS_UINT16(reg_addr, val)  REG_WRITE_UINT16(reg_addr, REG(reg_addr)&(val))

// PM banks
#define RIU_PM_BASE                         (IO_ADDRESS(0x1F000000))

#define REG_BANK_PM_SLEEP                   (0x700)
#define REG_BANK_TIMER0                     (0x1800)

#define PM_SLEEP_REG_BASE_ADDR              GET_REG_ADDR(RIU_PM_BASE, REG_BANK_PM_SLEEP)
#define TIMER0_REG_BASE_ADDR                GET_REG_ADDR(RIU_PM_BASE, REG_BANK_TIMER0)

// Non-PM banks
#define RIU_BASE                            (IO_ADDRESS(0x1F200000))

#define REG_BANK_CLKGEN                     (0x580)
#define REG_BANK_CHIPTOP                    (0xF00)
#define REG_BANK_FCIE0                      (0x8980)
#define REG_BANK_FCIE2                      (0x8A00)
#define REG_BANK_FCIEPOWERSAVEMODE          (0x11DC0)
#define REG_BANK_EMMC_PLL                   (0x11F80)

#define MPLL_CLK_REG_BASE_ADDR              GET_REG_ADDR(RIU_BASE, REG_BANK_CLKGEN)
#define CHIPTOP_REG_BASE_ADDR               GET_REG_ADDR(RIU_BASE, REG_BANK_CHIPTOP)
#define FCIE_REG_BASE_ADDR                  GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE0)
#define FCIE_NC_CIFD_BASE                   GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE2)
#define FCIE_POWEER_SAVE_MODE_BASE          GET_REG_ADDR(RIU_BASE, REG_BANK_FCIEPOWERSAVEMODE)
#define EMMC_PLL_REG_BASE_ADDR              GET_REG_ADDR(RIU_BASE, REG_BANK_EMMC_PLL)

#define REG50_ECC_CTRL_INIT_VALUE           0

#define UNFD_ST_PLAT                        0x80000000
#define IF_IP_VERIFY                        0 // [CAUTION]: to verify IP and HAL code, defaut 0

// debug
#define FCIE3_DUMP_DEBUG_BUS                1

//FCIE4 DDR Nand
#define FCIE4_DDR                           1

#if defined(FCIE4_DDR) && FCIE4_DDR
#define FCIE4_DDR_EMMC_PLL                  1
#define FCIE4_DDR_RETRY_DQS                 0
#define NAND_DELAY_CELL_PS                  300 // pico-second
#endif

#define FCIE_LFSR                           1

#define SPARE640B_CIFD512B_PATCH            1

#define NAND_PAD_BYPASS_MODE                1
#define NAND_PAD_TOGGLE_MODE                2
#define NAND_PAD_ONFI_SYNC_MODE             3

#if (defined(FCIE4_DDR) && FCIE4_DDR)
#define NC_SET_DDR_MODE()                   REG_WRITE_UINT16(NC_DDR_CTRL, pNandDrv->u16_Reg58_DDRCtrl);
#define NC_CLR_DDR_MODE()                   REG_CLR_BITS_UINT16(NC_DDR_CTRL, BIT_DDR_MASM);
#else
#define NC_SET_DDR_MODE()
#define NC_CLR_DDR_MODE()
#endif

#if (defined(FCIE4_DDR) && FCIE4_DDR)
/*
    Einstein doesn't use DQS mode any more.
    We use 'DLL phase selection' to instead the 'DQS mode'.
    However, we use old term name
    DQS_MODE_TABLE_CNT and DQS_MODE_SEARCH_TABLE
    for backward compatiblity.
*/
#define DQS_MODE_TABLE_CNT                  16
#define DQS_MODE_SEARCH_TABLE               {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}	//{7,8,6,9,5,10,4,11,3,12,2,13, 1,14 , 0, 15}

#define	NC_ONFI_DEFAULT_TRR                 12
#define	NC_ONFI_DEFAULT_TCS                 7
#define NC_ONFI_DEFAULT_TWW                 8
#define NC_ONFI_DEFAULT_TWHR                5
#define NC_ONFI_DEFAULT_TADL                6
#define NC_ONFI_DEFAULT_TCWAW               4
#define	NC_ONFI_DEFAULT_RX40CMD             4
#define	NC_ONFI_DEFAULT_RX40ADR             7
#define	NC_ONFI_DEFAULT_RX56                10

#define	NC_TOGGLE_DEFAULT_TRR               8
#define	NC_TOGGLE_DEFAULT_TCS               6
#define NC_TOGGLE_DEFAULT_TWW               7
#define	NC_TOGGLE_DEFAULT_TWHR              5
#define	NC_TOGGLE_DEFAULT_TADL              7
#define	NC_TOGGLE_DEFAULT_TCWAW             2
#define	NC_TOGGLE_DEFAULT_RX40CMD           4
#define	NC_TOGGLE_DEFAULT_RX40ADR           5
#define	NC_TOGGLE_DEFAULT_RX56              15
#endif

#define	NC_SDR_DEFAULT_TRR                  7
#define	NC_SDR_DEFAULT_TCS                  6
#define NC_SDR_DEFAULT_TWW                  5
#define	NC_SDR_DEFAULT_TWHR                 4
#define	NC_SDR_DEFAULT_TADL                 8
#define	NC_SDR_DEFAULT_TCWAW                2
#define	NC_SDR_DEFAULT_RX40CMD              4
#define	NC_SDR_DEFAULT_RX40ADR              5
#define	NC_SDR_DEFAULT_RX56                 5

#define	NC_INST_DELAY                       1
#define	NC_HWCMD_DELAY                      1
#define	NC_TRR_TCS                          1
#define	NC_TWHR_TCLHZ                       1
#define	NC_TCWAW_TADL                       1


#define NC_SEL_FCIE3                        1
#if NC_SEL_FCIE3
#include "drvNAND_reg_v3.h"
#else
#error "Error! no FCIE registers selected."
#endif

#define IF_FCIE_SHARE_PINS                  0 // 1: need to nand_pads_switch at HAL's functions.
#define IF_FCIE_SHARE_CLK                   0 // 1: need to nand_clock_setting at HAL's functions.
#define IF_FCIE_SHARE_IP                    1 // A3 might use SD

#define ENABLE_NAND_INTERRUPT_MODE          1

#define ENABLE_NAND_POWER_SAVING_MODE       1
#define ENABLE_NAND_POWER_SAVING_DEGLITCH   1

#define NAND_DRIVER_ROM_VERSION             0 // to save code size
#define AUTO_FORMAT_FTL                     0

#define ENABLE_CUS_READ_ENHANCEMENT         0

#define __VER_UNFD_FTL__                    0
//=====================================================
// Nand Driver configs
//=====================================================
#define NAND_BUF_USE_STACK                  0
#define NAND_ENV_FPGA                       1
#define NAND_ENV_ASIC                       2
#ifdef __FPGA_MODE__
#define NAND_DRIVER_ENV                     NAND_ENV_FPGA
#else
#define NAND_DRIVER_ENV                     NAND_ENV_ASIC
#endif

#define UNFD_CACHE_LINE                     0x80
//=====================================================
// tool-chain attributes
//=====================================================
#define UNFD_PACK0
#define UNFD_PACK1                          __attribute__((__packed__))
#define UNFD_ALIGN0
#define UNFD_ALIGN1                         __attribute__((aligned(UNFD_CACHE_LINE)))

//=====================================================
// debug option
//=====================================================
#define NAND_TEST_IN_DESIGN                 0      /* [CAUTION] */

#ifndef NAND_DEBUG_MSG
#define NAND_DEBUG_MSG                      1
#endif

/* Define trace levels. */
#define UNFD_DEBUG_LEVEL_ERROR              (1)    /* Error condition debug messages. */
#define UNFD_DEBUG_LEVEL_WARNING            (2)    /* Warning condition debug messages. */
#define UNFD_DEBUG_LEVEL_HIGH               (3)    /* Debug messages (high debugging). */
#define UNFD_DEBUG_LEVEL_MEDIUM             (4)    /* Debug messages. */
#define UNFD_DEBUG_LEVEL_LOW                (5)    /* Debug messages (low debugging). */

/* Higer debug level means more verbose */
#ifndef UNFD_DEBUG_LEVEL
#define UNFD_DEBUG_LEVEL                    UNFD_DEBUG_LEVEL_WARNING
#endif

#if defined(NAND_DEBUG_MSG) && NAND_DEBUG_MSG
#define nand_print_tmt
#define nand_printf                         printk
#define nand_debug(dbg_lv, tag, str, ...)           \
    do {                                            \
        if (dbg_lv > UNFD_DEBUG_LEVEL)              \
            break;                                  \
        else {                                      \
            if (tag)                                \
            {                                       \
                nand_printf(KERN_ERR"[%s]\t",__func__);     \
            }                                       \
                                                    \
            nand_printf(KERN_ERR str, ##__VA_ARGS__);        \
        }                                           \
    } while(0)
#else /* NAND_DEBUG_MSG */
#define nand_printf(...)
#define nand_debug(enable, tag, str, ...)   {}
#endif /* NAND_DEBUG_MSG */

static __inline void nand_assert(int condition)
{
    if (!condition) {
        panic("%s\n UNFD Assert(%d)\n", __func__, __LINE__);
    }
}

#define nand_die()      \
    do {                \
        nand_assert(0); \
    } while(0);

#define nand_stop() \
    while(1)  nand_reset_WatchDog();

//=====================================================
// HW Timer for Delay
//=====================================================
#define TIMER0_ENABLE                       GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x20)
#define TIMER0_HIT                          GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x21)
#define TIMER0_MAX_LOW                      GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x22)
#define TIMER0_MAX_HIGH                     GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x23)
#define TIMER0_CAP_LOW                      GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x24)
#define TIMER0_CAP_HIGH                     GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x25)

#define HW_TIMER_DELAY_1us                  1
#define HW_TIMER_DELAY_10us                 10
#define HW_TIMER_DELAY_100us                100
#define HW_TIMER_DELAY_1ms                  (1000 * HW_TIMER_DELAY_1us)
#define HW_TIMER_DELAY_5ms                  (5    * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_10ms                 (10   * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_100ms                (100  * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_500ms                (500  * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_1s                   (1000 * HW_TIMER_DELAY_1ms)

extern void delay_us(unsigned us);
extern U32  nand_hw_timer_delay(U32 u32usTick);
extern U32 	nand_check_DDR_pad(void);

//=====================================================
// Pads Switch
//=====================================================
#define reg_pcm_a_pe                        GET_REG_ADDR(CHIPTOP_REG_BASE_ADDR, 0x0A)
#define reg_ts2_d0                          GET_REG_ADDR(CHIPTOP_REG_BASE_ADDR, 0x0C)
#define reg_emmc_ps                         GET_REG_ADDR(CHIPTOP_REG_BASE_ADDR, 0x0F)

#define reg_fcie2macro_sd_bypass            GET_REG_ADDR(CHIPTOP_REG_BASE_ADDR, 0x10)
#define BIT_FCIE2MACRO_SD_BYPASS            BIT8

#define reg_sd_use_bypass                   GET_REG_ADDR(CHIPTOP_REG_BASE_ADDR, 0x40)
#define BIT_SD_USE_BYPASS                   BIT0

#define reg_allpad_in                       GET_REG_ADDR(CHIPTOP_REG_BASE_ADDR, 0x50)

#define reg_pad_drv                         GET_REG_ADDR(CHIPTOP_REG_BASE_ADDR, 0x13)
#define BIT_PAD_DRV_MASK                    (BIT12 - 1)

#define REG_EMMC_CONFIG                     GET_REG_ADDR(CHIPTOP_REG_BASE_ADDR, 0x6E)

#define reg_nf_en                           GET_REG_ADDR(CHIPTOP_REG_BASE_ADDR, 0x6F)
#define REG_NAND_MODE_MASK                  (BIT7|BIT6)
#define NAND_MODE1                          (BIT6)  // PCM_A
#define NAND_MODE2                          (BIT7)  // EMMC+TS2+DQS

#define REG_NAND_CS1_EN                     GET_REG_ADDR(CHIPTOP_REG_BASE_ADDR, 0x6F)
#define BIT_NAND_CS1_EN                     BIT5

#define reg_sd_config                       GET_REG_ADDR(CHIPTOP_REG_BASE_ADDR, 0x5A)
#define reg_sdio_config                     GET_REG_ADDR(CHIPTOP_REG_BASE_ADDR, 0x7B)

#define REG_NAND_DQS_UL                     GET_REG_ADDR(CHIPTOP_REG_BASE_ADDR, 0x0F)
#define BIT_DQS_PULL_H                      BIT11
#define NC_DQS_PULL_H()                     REG_SET_BITS_UINT16(REG_NAND_DQS_UL, BIT_DQS_PULL_H)
#define NC_DQS_PULL_L()                     REG_CLR_BITS_UINT16(REG_NAND_DQS_UL, BIT_DQS_PULL_H)

extern U32 nand_pads_switch(U32 u32EnableFCIE);

//=====================================================
// set FCIE clock
//=====================================================
#define DECIDE_CLOCK_BY_NAND                1

#define NFIE_CLK_20M                        (BIT6|(1<<2))
#define NFIE_CLK_27M                        (BIT6|(2<<2))
#define NFIE_CLK_32M                        (BIT6|(3<<2))
#define NFIE_CLK_36M                        (BIT6|(4<<2))
#define NFIE_CLK_40M                        (BIT6|(5<<2))
#define NFIE_CLK_43M                        (BIT6|(6<<2))
#define NFIE_CLK_54M                        (BIT6|(7<<2))
#define NFIE_CLK_62M                        (BIT6|(8<<2))
#define NFIE_CLK_72M                        (BIT6|(9<<2))
#define NFIE_CLK_EMMC_PLL                   (BIT6|(10<<2))
#define NFIE_CLK_86M                        (BIT6|(11<<2))
#define NFIE_CLK_48M                        (BIT6|(15<<2))

#define NFIE_CLK_TABLE_CNT                  11
#define NFIE_CLK_TABLE {                    NFIE_CLK_20M,   \
                                            NFIE_CLK_27M,   \
                                            NFIE_CLK_32M,   \
                                            NFIE_CLK_36M,   \
                                            NFIE_CLK_40M,   \
                                            NFIE_CLK_43M,   \
                                            NFIE_CLK_48M,   \
                                            NFIE_CLK_54M,   \
                                            NFIE_CLK_62M,   \
                                            NFIE_CLK_72M,   \
                                            NFIE_CLK_86M }

#define NFIE_CLK_TABLE_STR {                "20M",  \
                                            "27M",  \
                                            "32M",  \
                                            "36M",  \
                                            "40M",  \
                                            "43M",  \
                                            "48M",  \
                                            "54M",  \
                                            "62M",  \
                                            "72M",  \
                                            "86M" }

#define NFIE_20M_VALUE                      20000000
#define NFIE_27M_VALUE                      27000000
#define NFIE_32M_VALUE                      32000000
#define NFIE_36M_VALUE                      36000000
#define NFIE_40M_VALUE                      40000000
#define NFIE_43M_VALUE                      43200000
#define NFIE_48M_VALUE                      48000000
#define NFIE_54M_VALUE                      54000000
#define NFIE_62M_VALUE                      62000000
#define NFIE_72M_VALUE                      72000000
#define NFIE_86M_VALUE                      86000000

#define NFIE_CLK_VALUE_TABLE {              NFIE_20M_VALUE,     \
                                            NFIE_27M_VALUE,     \
                                            NFIE_32M_VALUE,     \
                                            NFIE_36M_VALUE,     \
                                            NFIE_40M_VALUE,     \
                                            NFIE_43M_VALUE,     \
                                            NFIE_48M_VALUE,     \
                                            NFIE_54M_VALUE,     \
                                            NFIE_62M_VALUE,     \
                                            NFIE_72M_VALUE,     \
                                            NFIE_86M_VALUE }

/*Define 1 cycle Time for each clock note: define value must be the (real value -1)*/
#define NFIE_1T_20M                         50
#define NFIE_1T_27M                         37
#define NFIE_1T_32M                         31
#define NFIE_1T_36M                         27
#define NFIE_1T_40M                         25
#define NFIE_1T_43M                         23
#define NFIE_1T_48M                         20
#define NFIE_1T_54M                         18
#define NFIE_1T_62M                         16
#define NFIE_1T_72M                         13
#define NFIE_1T_86M                         11

#define NFIE_1T_TABLE {                     NFIE_1T_20M,    \
                                            NFIE_1T_27M,    \
                                            NFIE_1T_32M,    \
                                            NFIE_1T_36M,    \
                                            NFIE_1T_40M,    \
                                            NFIE_1T_43M,    \
                                            NFIE_1T_48M,    \
                                            NFIE_1T_54M,    \
                                            NFIE_1T_62M,    \
                                            NFIE_1T_72M,    \
                                            NFIE_1T_86M }

#define NFIE_4CLK_TABLE_CNT                 11
#define NFIE_4CLK_TABLE {                   NFIE_CLK_20M,   \
                                            NFIE_CLK_27M,   \
                                            NFIE_CLK_32M,   \
                                            NFIE_CLK_36M,   \
                                            NFIE_CLK_40M,   \
                                            NFIE_CLK_43M,   \
                                            NFIE_CLK_48M,   \
                                            NFIE_CLK_54M,   \
                                            NFIE_CLK_62M,   \
                                            NFIE_CLK_72M,   \
                                            NFIE_CLK_86M }

#define NFIE_4CLK_TABLE_STR {               "20M",  \
                                            "27M",  \
                                            "32M",  \
                                            "36M",  \
                                            "40M",  \
                                            "43M",  \
                                            "48M",  \
                                            "54M",  \
                                            "62M",  \
                                            "72M",  \
                                            "86M" }

#define NFIE_4CLK_VALUE_TABLE {             NFIE_20M_VALUE,     \
                                            NFIE_27M_VALUE,     \
                                            NFIE_32M_VALUE,     \
                                            NFIE_36M_VALUE,     \
                                            NFIE_40M_VALUE,     \
                                            NFIE_43M_VALUE,     \
                                            NFIE_48M_VALUE,     \
                                            NFIE_54M_VALUE,     \
                                            NFIE_62M_VALUE,     \
                                            NFIE_72M_VALUE,     \
                                            NFIE_86M_VALUE }

#define NFIE_4CLK_1T_TABLE {                NFIE_1T_20M,    \
                                            NFIE_1T_27M,    \
                                            NFIE_1T_32M,    \
                                            NFIE_1T_36M,    \
                                            NFIE_1T_40M,    \
                                            NFIE_1T_43M,    \
                                            NFIE_1T_48M,    \
                                            NFIE_1T_54M,    \
                                            NFIE_1T_62M,    \
                                            NFIE_1T_72M,    \
                                            NFIE_1T_86M }

//=====================================================
// set EMMC PLL clocks
//=====================================================
#define REG_EMMC_PLL_RX01                   GET_REG_ADDR(EMMC_PLL_REG_BASE_ADDR, 0x01)
#define REG_EMMC_PLL_RX02                   GET_REG_ADDR(EMMC_PLL_REG_BASE_ADDR, 0x02)
#define REG_EMMC_PLL_RX03                   GET_REG_ADDR(EMMC_PLL_REG_BASE_ADDR, 0x03)
#define REG_EMMC_PLL_RX04                   GET_REG_ADDR(EMMC_PLL_REG_BASE_ADDR, 0x04)
#define REG_EMMC_PLL_RX05                   GET_REG_ADDR(EMMC_PLL_REG_BASE_ADDR, 0x05)
#define REG_EMMC_PLL_RX06                   GET_REG_ADDR(EMMC_PLL_REG_BASE_ADDR, 0x06)
#define REG_EMMC_PLL_RX07                   GET_REG_ADDR(EMMC_PLL_REG_BASE_ADDR, 0x07)

#define REG_EMMC_PLL_RX09                   GET_REG_ADDR(EMMC_PLL_REG_BASE_ADDR, 0x09)
#define BIT_EMMC_RXDLL_PHASE_SEL_MASK       (BIT7|BIT6|BIT5|BIT4)
#define BIT_EMMC_RXDLL_PHASE_SEL_SHIFT      4

#define ENABLE_DELAY_CELL					0

#if defined(ENABLE_DELAY_CELL) && ENABLE_DELAY_CELL
#undef DQS_MODE_SEARCH_TABLE
#define DQS_MODE_SEARCH_TABLE               {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}
#endif

#define BIT_EMMC_RXDELAY_CELL_MASK          (BIT7|BIT6|BIT5|BIT4)
#define BIT_EMMC_RXDELAY_CELL_SHIFT         4

#define REG_EMMC_PLL_RX18                   GET_REG_ADDR(EMMC_PLL_REG_BASE_ADDR, 0x18)
#define REG_EMMC_PLL_RX19                   GET_REG_ADDR(EMMC_PLL_REG_BASE_ADDR, 0x19)
#define REG_EMMC_PLL_RX1B                   GET_REG_ADDR(EMMC_PLL_REG_BASE_ADDR, 0x1B)
#define REG_EMMC_PLL_RX30                   GET_REG_ADDR(EMMC_PLL_REG_BASE_ADDR, 0x30)
#define REG_EMMC_PLL_RX32                   GET_REG_ADDR(EMMC_PLL_REG_BASE_ADDR, 0x32)
#define REG_EMMC_PLL_RX33                   GET_REG_ADDR(EMMC_PLL_REG_BASE_ADDR, 0x33)
#define REG_EMMC_PLL_RX34                   GET_REG_ADDR(EMMC_PLL_REG_BASE_ADDR, 0x34)

#define EMMC_PLL_1XCLK_TABLE_CNT            11

#define EMMC_PLL_1XCLK_RX05_MASK            (BIT2|BIT1|BIT0)

#define EMMC_PLL_1XCLK_20M_IDX              0
#define EMMC_PLL_1XCLK_20M_RX05             7
#define EMMC_PLL_1XCLK_20M_RX18             (0x3333)
#define EMMC_PLL_1XCLK_20M_RX19             (0x002B)

#define EMMC_PLL_1XCLK_27M_IDX              1
#define EMMC_PLL_1XCLK_27M_RX05             4
#define EMMC_PLL_1XCLK_27M_RX18             (0x0000)
#define EMMC_PLL_1XCLK_27M_RX19             (0x0040)

#define EMMC_PLL_1XCLK_32M_IDX              2
#define EMMC_PLL_1XCLK_32M_RX05             4
#define EMMC_PLL_1XCLK_32M_RX18             (0x0000)
#define EMMC_PLL_1XCLK_32M_RX19             (0x0036)

#define EMMC_PLL_1XCLK_36M_IDX              3
#define EMMC_PLL_1XCLK_36M_RX05             4
#define EMMC_PLL_1XCLK_36M_RX18             (0x0000)
#define EMMC_PLL_1XCLK_36M_RX19             (0x0030)

#define EMMC_PLL_1XCLK_40M_IDX              4
#define EMMC_PLL_1XCLK_40M_RX05             4
#define EMMC_PLL_1XCLK_40M_RX18             (0x3333)
#define EMMC_PLL_1XCLK_40M_RX19             (0x002B)

#define EMMC_PLL_1XCLK_43M_IDX              5
#define EMMC_PLL_1XCLK_43M_RX05             4
#define EMMC_PLL_1XCLK_43M_RX18             (0x2FA0)
#define EMMC_PLL_1XCLK_43M_RX19             (0x0028)

#define EMMC_PLL_1XCLK_48M_IDX              6
#define EMMC_PLL_1XCLK_48M_RX05             2
#define EMMC_PLL_1XCLK_48M_RX18             (0x0000)
#define EMMC_PLL_1XCLK_48M_RX19             (0x0048)

#define EMMC_PLL_1XCLK_54M_IDX              7
#define EMMC_PLL_1XCLK_54M_RX05             2
#define EMMC_PLL_1XCLK_54M_RX18             (0x0000)
#define EMMC_PLL_1XCLK_54M_RX19             (0x0040)

#define EMMC_PLL_1XCLK_62M_IDX              8
#define EMMC_PLL_1XCLK_62M_RX05             2
#define EMMC_PLL_1XCLK_62M_RX18             (0xBDEF)
#define EMMC_PLL_1XCLK_62M_RX19             (0x0037)

#define EMMC_PLL_1XCLK_72M_IDX              9
#define EMMC_PLL_1XCLK_72M_RX05             2
#define EMMC_PLL_1XCLK_72M_RX18             (0x0000)
#define EMMC_PLL_1XCLK_72M_RX19             (0x0030)

#define EMMC_PLL_1XCLK_86M_IDX              10
#define EMMC_PLL_1XCLK_86M_RX05             2
#define EMMC_PLL_1XCLK_86M_RX18             (0x2FA0)
#define EMMC_PLL_1XCLK_86M_RX19             (0x0028)

typedef struct _EMMC_PLL_SETTINGS
{
    U16 emmc_pll_1xclk_rx05;
    U16 emmc_pll_1xclk_rx18;
    U16 emmc_pll_1xclk_rx19;
} EMMC_PLL_SETTINGS;

#define EMMC_PLL_CLK_TABLE {                                                                                                \
                                            {EMMC_PLL_1XCLK_20M_RX05, EMMC_PLL_1XCLK_20M_RX18, EMMC_PLL_1XCLK_20M_RX19},    \
                                            {EMMC_PLL_1XCLK_27M_RX05, EMMC_PLL_1XCLK_27M_RX18, EMMC_PLL_1XCLK_27M_RX19},    \
                                            {EMMC_PLL_1XCLK_32M_RX05, EMMC_PLL_1XCLK_32M_RX18, EMMC_PLL_1XCLK_32M_RX19},    \
                                            {EMMC_PLL_1XCLK_36M_RX05, EMMC_PLL_1XCLK_36M_RX18, EMMC_PLL_1XCLK_36M_RX19},    \
                                            {EMMC_PLL_1XCLK_40M_RX05, EMMC_PLL_1XCLK_40M_RX18, EMMC_PLL_1XCLK_40M_RX19},    \
                                            {EMMC_PLL_1XCLK_43M_RX05, EMMC_PLL_1XCLK_43M_RX18, EMMC_PLL_1XCLK_43M_RX19},    \
                                            {EMMC_PLL_1XCLK_48M_RX05, EMMC_PLL_1XCLK_48M_RX18, EMMC_PLL_1XCLK_48M_RX19},    \
                                            {EMMC_PLL_1XCLK_54M_RX05, EMMC_PLL_1XCLK_54M_RX18, EMMC_PLL_1XCLK_54M_RX19},    \
                                            {EMMC_PLL_1XCLK_62M_RX05, EMMC_PLL_1XCLK_62M_RX18, EMMC_PLL_1XCLK_62M_RX19},    \
                                            {EMMC_PLL_1XCLK_72M_RX05, EMMC_PLL_1XCLK_72M_RX18, EMMC_PLL_1XCLK_72M_RX19},    \
                                            {EMMC_PLL_1XCLK_86M_RX05, EMMC_PLL_1XCLK_86M_RX18, EMMC_PLL_1XCLK_86M_RX19} }

#define DUTY_CYCLE_PATCH                    0 // 1: to enlarge low width for tREA's worst case of 25ns
#if DUTY_CYCLE_PATCH
#define FCIE3_SW_DEFAULT_CLK                NFIE_CLK_86M
#define FCIE_REG41_VAL                      ((2<<9)|(2<<3)) // RE,WR pulse, Low:High=3:1
#define REG57_ECO_FIX_INIT_VALUE            0
#else
#define FCIE3_SW_DEFAULT_CLK                NFIE_CLK_54M
#define FCIE_REG41_VAL                      0               // RE,WR pulse, Low:High=1:1
#define REG57_ECO_FIX_INIT_VALUE            BIT_NC_LATCH_DATA_1_5_T	// delay 1.0T
#endif
#define FCIE3_SW_SLOWEST_CLK                NFIE_CLK_20M

#define NAND_SEQ_ACC_TIME_TOL               16 //in unit of ns

#define reg_ckg_fcie                        GET_REG_ADDR(MPLL_CLK_REG_BASE_ADDR, 0x64)

extern U32  nand_clock_setting(U32 u32ClkParam);

extern void nand_DumpPadClk(void);

//=====================================================
// set EMMC PLL clocks
//=====================================================
#define reg_pwrgd_int_glirm                 GET_REG_ADDR(PM_SLEEP_REG_BASE_ADDR, 0x61)
#define BIT_PWRGD_INT_GLIRM_EN              BIT9
#define BIT_PWEGD_INT_GLIRM_MASK            (BIT15|BIT14|BIT13|BIT12|BIT11|BIT10)

//=====================================================
// transfer DMA Address
//=====================================================
#define MIU_BUS_WIDTH_BITS                  3 // Need to confirm

#define MIU_CHECK_LAST_DONE                 1

//=====================================================
// misc
//=====================================================
//#define BIG_ENDIAN
#define LITTLE_ENDIAN

typedef struct NAND_DRIVER_PLATFORM_DEPENDENT
{
    U8 *pu8_PageSpareBuf;
    U8 *pu8_PageDataBuf;
    U32	u32_DMAAddrOffset;
    U32	u32_RAMBufferOffset;
    U32	u32_RAMBufferLen;

}NAND_DRIVER_PLAT_CTX, *P_NAND_DRIVER_PLAT;

#define NC_REG_MIU_LAST_DONE                NC_MIE_EVENT

#define NANDINFO_ECC_TYPE                   ECC_TYPE_40BIT1KB
#define malloc(x)                           kmalloc(x, GFP_KERNEL)
#define free                                kfree

#endif
