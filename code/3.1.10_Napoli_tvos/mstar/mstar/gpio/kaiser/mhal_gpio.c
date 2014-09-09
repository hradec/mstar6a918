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

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
//#include "MsCommon.h"
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <asm/io.h>

#include "mhal_gpio.h"
#include "mhal_gpio_reg.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#if 1
#define _CONCAT( a, b )     a##b
#define CONCAT( a, b )      _CONCAT( a, b )
/*
#define BIT0    BIT(0)
#define BIT1    BIT(1)
#define BIT2    BIT(2)
#define BIT3    BIT(3)
#define BIT4    BIT(4)
#define BIT5    BIT(5)
#define BIT6    BIT(6)
#define BIT7    BIT(7)
*/
    // Dummy
#define GPIO999_OEN     0, 0
#define GPIO999_OUT     0, 0
#define GPIO999_IN      0, 0

#define GPIO0_PAD PAD_I2S_OUT_WS
#define GPIO0_OEN 0x10262e, BIT5
#define GPIO0_OUT 0x10262e, BIT4
#define GPIO0_IN  0x10262e, BIT0

#define GPIO1_PAD PAD_I2S_OUT_MCK
#define GPIO1_OEN 0x102630, BIT5
#define GPIO1_OUT 0x102630, BIT4
#define GPIO1_IN  0x102630, BIT0

#define GPIO2_PAD PAD_I2S_OUT_BCK
#define GPIO2_OEN 0x102632, BIT5
#define GPIO2_OUT 0x102632, BIT4
#define GPIO2_IN  0x102632, BIT0

#define GPIO3_PAD PAD_I2S_OUT_SD0
#define GPIO3_OEN 0x102634, BIT5
#define GPIO3_OUT 0x102634, BIT4
#define GPIO3_IN  0x102634, BIT0

#define GPIO4_PAD PAD_I2S_OUT_SD1
#define GPIO4_OEN 0x102636, BIT5
#define GPIO4_OUT 0x102636, BIT4
#define GPIO4_IN  0x102636, BIT0

#define GPIO5_PAD PAD_I2S_OUT_SD2
#define GPIO5_OEN 0x102638, BIT5
#define GPIO5_OUT 0x102638, BIT4
#define GPIO5_IN  0x102638, BIT0

#define GPIO6_PAD PAD_I2S_OUT_SD3
#define GPIO6_OEN 0x10263a, BIT5
#define GPIO6_OUT 0x10263a, BIT4
#define GPIO6_IN  0x10263a, BIT0

#define GPIO7_PAD PAD_SDIO_CLK
#define GPIO7_OEN 0x102676, BIT5
#define GPIO7_OUT 0x102676, BIT4
#define GPIO7_IN  0x102676, BIT0

#define GPIO8_PAD PAD_SDIO_CMD
#define GPIO8_OEN 0x102678, BIT5
#define GPIO8_OUT 0x102678, BIT4
#define GPIO8_IN  0x102678, BIT0

#define GPIO9_PAD PAD_SDIO_D0
#define GPIO9_OEN 0x10267a, BIT5
#define GPIO9_OUT 0x10267a, BIT4
#define GPIO9_IN  0x10267a, BIT0

#define GPIO10_PAD PAD_SDIO_D1
#define GPIO10_OEN 0x10267c, BIT5
#define GPIO10_OUT 0x10267c, BIT4
#define GPIO10_IN  0x10267c, BIT0

#define GPIO11_PAD PAD_SDIO_D2
#define GPIO11_OEN 0x10267e, BIT5
#define GPIO11_OUT 0x10267e, BIT4
#define GPIO11_IN  0x10267e, BIT0

#define GPIO12_PAD PAD_SDIO_D3
#define GPIO12_OEN 0x102680, BIT5
#define GPIO12_OUT 0x102680, BIT4
#define GPIO12_IN  0x102680, BIT0

#define GPIO13_PAD PAD_SM0_CD
#define GPIO13_OEN 0x102500, BIT5
#define GPIO13_OUT 0x102500, BIT4
#define GPIO13_IN  0x102500, BIT0

#define GPIO14_PAD PAD_SM0_RST
#define GPIO14_OEN 0x102502, BIT5
#define GPIO14_OUT 0x102502, BIT4
#define GPIO14_IN  0x102502, BIT0

#define GPIO15_PAD PAD_SM0_VCC
#define GPIO15_OEN 0x102504, BIT5
#define GPIO15_OUT 0x102504, BIT4
#define GPIO15_IN  0x102504, BIT0

#define GPIO16_PAD PAD_SM0_IO
#define GPIO16_OEN 0x102506, BIT5
#define GPIO16_OUT 0x102506, BIT4
#define GPIO16_IN  0x102506, BIT0

#define GPIO17_PAD PAD_SM0_CLK
#define GPIO17_OEN 0x102508, BIT5
#define GPIO17_OUT 0x102508, BIT4
#define GPIO17_IN  0x102508, BIT0

#define GPIO18_PAD PAD_SM0_GPIO0
#define GPIO18_OEN 0x10250a, BIT5
#define GPIO18_OUT 0x10250a, BIT4
#define GPIO18_IN  0x10250a, BIT0

#define GPIO19_PAD PAD_SM0_GPIO1
#define GPIO19_OEN 0x10250c, BIT5
#define GPIO19_OUT 0x10250c, BIT4
#define GPIO19_IN  0x10250c, BIT0

#define GPIO20_PAD PAD_SM1_CD
#define GPIO20_OEN 0x10250e, BIT5
#define GPIO20_OUT 0x10250e, BIT4
#define GPIO20_IN  0x10250e, BIT0

#define GPIO21_PAD PAD_SM1_RST
#define GPIO21_OEN 0x102510, BIT5
#define GPIO21_OUT 0x102510, BIT4
#define GPIO21_IN  0x102510, BIT0

#define GPIO22_PAD PAD_SM1_VCC
#define GPIO22_OEN 0x102512, BIT5
#define GPIO22_OUT 0x102512, BIT4
#define GPIO22_IN  0x102512, BIT0

#define GPIO23_PAD PAD_SM1_IO
#define GPIO23_OEN 0x102514, BIT5
#define GPIO23_OUT 0x102514, BIT4
#define GPIO23_IN  0x102514, BIT0

#define GPIO24_PAD PAD_SM1_CLK
#define GPIO24_OEN 0x102516, BIT5
#define GPIO24_OUT 0x102516, BIT4
#define GPIO24_IN  0x102516, BIT0

#define GPIO25_PAD PAD_SM1_GPIO0
#define GPIO25_OEN 0x102518, BIT5
#define GPIO25_OUT 0x102518, BIT4
#define GPIO25_IN  0x102518, BIT0

#define GPIO26_PAD PAD_SM1_GPIO1
#define GPIO26_OEN 0x10251a, BIT5
#define GPIO26_OUT 0x10251a, BIT4
#define GPIO26_IN  0x10251a, BIT0

#define GPIO27_PAD PAD_EJ_DINT
#define GPIO27_OEN 0x10268e, BIT5
#define GPIO27_OUT 0x10268e, BIT4
#define GPIO27_IN  0x10268e, BIT0

#define GPIO28_PAD PAD_EJ_RSTZ
#define GPIO28_OEN 0x102690, BIT5
#define GPIO28_OUT 0x102690, BIT4
#define GPIO28_IN  0x102690, BIT0

#define GPIO29_PAD PAD_CI_A0
#define GPIO29_OEN 0x10255a, BIT5
#define GPIO29_OUT 0x10255a, BIT4
#define GPIO29_IN  0x10255a, BIT0

#define GPIO30_PAD PAD_CI_A1
#define GPIO30_OEN 0x10255c, BIT5
#define GPIO30_OUT 0x10255c, BIT4
#define GPIO30_IN  0x10255c, BIT0

#define GPIO31_PAD PAD_CI_A2
#define GPIO31_OEN 0x10255e, BIT5
#define GPIO31_OUT 0x10255e, BIT4
#define GPIO31_IN  0x10255e, BIT0

#define GPIO32_PAD PAD_CI_A3
#define GPIO32_OEN 0x102560, BIT5
#define GPIO32_OUT 0x102560, BIT4
#define GPIO32_IN  0x102560, BIT0

#define GPIO33_PAD PAD_CI_A4
#define GPIO33_OEN 0x102562, BIT5
#define GPIO33_OUT 0x102562, BIT4
#define GPIO33_IN  0x102562, BIT0

#define GPIO34_PAD PAD_CI_A5
#define GPIO34_OEN 0x102564, BIT5
#define GPIO34_OUT 0x102564, BIT4
#define GPIO34_IN  0x102564, BIT0

#define GPIO35_PAD PAD_CI_A6
#define GPIO35_OEN 0x102566, BIT5
#define GPIO35_OUT 0x102566, BIT4
#define GPIO35_IN  0x102566, BIT0

#define GPIO36_PAD PAD_CI_A7
#define GPIO36_OEN 0x102568, BIT5
#define GPIO36_OUT 0x102568, BIT4
#define GPIO36_IN  0x102568, BIT0

#define GPIO37_PAD PAD_CI_A8
#define GPIO37_OEN 0x10256a, BIT5
#define GPIO37_OUT 0x10256a, BIT4
#define GPIO37_IN  0x10256a, BIT0

#define GPIO38_PAD PAD_CI_A9
#define GPIO38_OEN 0x10256c, BIT5
#define GPIO38_OUT 0x10256c, BIT4
#define GPIO38_IN  0x10256c, BIT0

#define GPIO39_PAD PAD_CI_A10
#define GPIO39_OEN 0x10256e, BIT5
#define GPIO39_OUT 0x10256e, BIT4
#define GPIO39_IN  0x10256e, BIT0

#define GPIO40_PAD PAD_CI_A11
#define GPIO40_OEN 0x102570, BIT5
#define GPIO40_OUT 0x102570, BIT4
#define GPIO40_IN  0x102570, BIT0

#define GPIO41_PAD PAD_CI_A12
#define GPIO41_OEN 0x102572, BIT5
#define GPIO41_OUT 0x102572, BIT4
#define GPIO41_IN  0x102572, BIT0

#define GPIO42_PAD PAD_CI_A13
#define GPIO42_OEN 0x102574, BIT5
#define GPIO42_OUT 0x102574, BIT4
#define GPIO42_IN  0x102574, BIT0

#define GPIO43_PAD PAD_CI_A14
#define GPIO43_OEN 0x102576, BIT5
#define GPIO43_OUT 0x102576, BIT4
#define GPIO43_IN  0x102576, BIT0

#define GPIO44_PAD PAD_CI_D0
#define GPIO44_OEN 0x102578, BIT5
#define GPIO44_OUT 0x102578, BIT4
#define GPIO44_IN  0x102578, BIT0

#define GPIO45_PAD PAD_CI_D1
#define GPIO45_OEN 0x10257a, BIT5
#define GPIO45_OUT 0x10257a, BIT4
#define GPIO45_IN  0x10257a, BIT0

#define GPIO46_PAD PAD_CI_D2
#define GPIO46_OEN 0x10257c, BIT5
#define GPIO46_OUT 0x10257c, BIT4
#define GPIO46_IN  0x10257c, BIT0

#define GPIO47_PAD PAD_CI_D3
#define GPIO47_OEN 0x10257e, BIT5
#define GPIO47_OUT 0x10257e, BIT4
#define GPIO47_IN  0x10257e, BIT0

#define GPIO48_PAD PAD_CI_D4
#define GPIO48_OEN 0x102580, BIT5
#define GPIO48_OUT 0x102580, BIT4
#define GPIO48_IN  0x102580, BIT0

#define GPIO49_PAD PAD_CI_D5
#define GPIO49_OEN 0x102582, BIT5
#define GPIO49_OUT 0x102582, BIT4
#define GPIO49_IN  0x102582, BIT0

#define GPIO50_PAD PAD_CI_D6
#define GPIO50_OEN 0x102584, BIT5
#define GPIO50_OUT 0x102584, BIT4
#define GPIO50_IN  0x102584, BIT0

#define GPIO51_PAD PAD_CI_D7
#define GPIO51_OEN 0x102586, BIT5
#define GPIO51_OUT 0x102586, BIT4
#define GPIO51_IN  0x102586, BIT0

#define GPIO52_PAD PAD_CI_IRQAZ
#define GPIO52_OEN 0x102588, BIT5
#define GPIO52_OUT 0x102588, BIT4
#define GPIO52_IN  0x102588, BIT0

#define GPIO53_PAD PAD_CI_RST
#define GPIO53_OEN 0x10258a, BIT5
#define GPIO53_OUT 0x10258a, BIT4
#define GPIO53_IN  0x10258a, BIT0

#define GPIO54_PAD PAD_CI_IORDZ
#define GPIO54_OEN 0x10258c, BIT5
#define GPIO54_OUT 0x10258c, BIT4
#define GPIO54_IN  0x10258c, BIT0

#define GPIO55_PAD PAD_CI_IOWRZ
#define GPIO55_OEN 0x10258e, BIT5
#define GPIO55_OUT 0x10258e, BIT4
#define GPIO55_IN  0x10258e, BIT0

#define GPIO56_PAD PAD_CI_OEZ
#define GPIO56_OEN 0x102590, BIT5
#define GPIO56_OUT 0x102590, BIT4
#define GPIO56_IN  0x102590, BIT0

#define GPIO57_PAD PAD_CI_WEZ
#define GPIO57_OEN 0x102592, BIT5
#define GPIO57_OUT 0x102592, BIT4
#define GPIO57_IN  0x102592, BIT0

#define GPIO58_PAD PAD_CI_REGZ
#define GPIO58_OEN 0x102594, BIT5
#define GPIO58_OUT 0x102594, BIT4
#define GPIO58_IN  0x102594, BIT0

#define GPIO59_PAD PAD_CI_CEZ
#define GPIO59_OEN 0x102596, BIT5
#define GPIO59_OUT 0x102596, BIT4
#define GPIO59_IN  0x102596, BIT0

#define GPIO60_PAD PAD_CI_WAITZ
#define GPIO60_OEN 0x102598, BIT5
#define GPIO60_OUT 0x102598, BIT4
#define GPIO60_IN  0x102598, BIT0

#define GPIO61_PAD PAD_CI_CDZ
#define GPIO61_OEN 0x10259a, BIT5
#define GPIO61_OUT 0x10259a, BIT4
#define GPIO61_IN  0x10259a, BIT0

#define GPIO62_PAD PAD_I2CM1_SDA
#define GPIO62_OEN 0x102520, BIT5
#define GPIO62_OUT 0x102520, BIT4
#define GPIO62_IN  0x102520, BIT0

#define GPIO63_PAD PAD_I2CM1_SCL
#define GPIO63_OEN 0x102522, BIT5
#define GPIO63_OUT 0x102522, BIT4
#define GPIO63_IN  0x102522, BIT0

#define GPIO64_PAD PAD_TS3_CLK
#define GPIO64_OEN 0x102692, BIT5
#define GPIO64_OUT 0x102692, BIT4
#define GPIO64_IN  0x102692, BIT0

#define GPIO65_PAD PAD_TS3_SYNC
#define GPIO65_OEN 0x102694, BIT5
#define GPIO65_OUT 0x102694, BIT4
#define GPIO65_IN  0x102694, BIT0

#define GPIO66_PAD PAD_TS3_VLD
#define GPIO66_OEN 0x102696, BIT5
#define GPIO66_OUT 0x102696, BIT4
#define GPIO66_IN  0x102696, BIT0

#define GPIO67_PAD PAD_TS3_D0
#define GPIO67_OEN 0x102698, BIT5
#define GPIO67_OUT 0x102698, BIT4
#define GPIO67_IN  0x102698, BIT0

#define GPIO68_PAD PAD_TS3_D1
#define GPIO68_OEN 0x10269a, BIT5
#define GPIO68_OUT 0x10269a, BIT4
#define GPIO68_IN  0x10269a, BIT0

#define GPIO69_PAD PAD_TS3_D2
#define GPIO69_OEN 0x10269c, BIT5
#define GPIO69_OUT 0x10269c, BIT4
#define GPIO69_IN  0x10269c, BIT0

#define GPIO70_PAD PAD_TS3_D3
#define GPIO70_OEN 0x10269e, BIT5
#define GPIO70_OUT 0x10269e, BIT4
#define GPIO70_IN  0x10269e, BIT0

#define GPIO71_PAD PAD_TS3_D4
#define GPIO71_OEN 0x1026a0, BIT5
#define GPIO71_OUT 0x1026a0, BIT4
#define GPIO71_IN  0x1026a0, BIT0

#define GPIO72_PAD PAD_TS3_D5
#define GPIO72_OEN 0x1026a2, BIT5
#define GPIO72_OUT 0x10256a2, BIT4
#define GPIO72_IN  0x1026a2, BIT0

#define GPIO73_PAD PAD_TS3_D6
#define GPIO73_OEN 0x1026a4, BIT5
#define GPIO73_OUT 0x1026a4, BIT4
#define GPIO73_IN  0x1026a4, BIT0

#define GPIO74_PAD PAD_TS3_D7
#define GPIO74_OEN 0x1026a6, BIT5
#define GPIO74_OUT 0x1026a6, BIT4
#define GPIO74_IN  0x1026a6, BIT0

#define GPIO75_PAD PAD_I2CM0_SDA
#define GPIO75_OEN 0x10251c, BIT5
#define GPIO75_OUT 0x10251c, BIT4
#define GPIO75_IN  0x10251c, BIT0

#define GPIO76_PAD PAD_I2CM0_SCL
#define GPIO76_OEN 0x10251e, BIT5
#define GPIO76_OUT 0x10251e, BIT4
#define GPIO76_IN  0x10251e, BIT0

#define GPIO77_PAD PAD_SPDIF_OUT
#define GPIO77_OEN 0x10262c, BIT5
#define GPIO77_OUT 0x10262c, BIT4
#define GPIO77_IN  0x10262c, BIT0

#define GPIO78_PAD PAD_EJ_TDO
#define GPIO78_OEN 0x102684, BIT5
#define GPIO78_OUT 0x102684, BIT4
#define GPIO78_IN  0x102684, BIT0

#define GPIO79_PAD PAD_EJ_TDI
#define GPIO79_OEN 0x102686, BIT5
#define GPIO79_OUT 0x102686, BIT4
#define GPIO79_IN  0x102686, BIT0

#define GPIO80_PAD PAD_EJ_TMS
#define GPIO80_OEN 0x102688, BIT5
#define GPIO80_OUT 0x102688, BIT4
#define GPIO80_IN  0x102688, BIT0

#define GPIO81_PAD PAD_EJ_TCK
#define GPIO81_OEN 0x10268a, BIT5
#define GPIO81_OUT 0x10268a, BIT4
#define GPIO81_IN  0x10268a, BIT0

#define GPIO82_PAD PAD_EJ_TRST_N
#define GPIO82_OEN 0x10268c, BIT5
#define GPIO82_OUT 0x10268c, BIT4
#define GPIO82_IN  0x10268c, BIT0

#define GPIO83_PAD PAD_TS2_CLK
#define GPIO83_OEN 0x102550, BIT5
#define GPIO83_OUT 0x102550, BIT4
#define GPIO83_IN  0x102550, BIT0

#define GPIO84_PAD PAD_TS2_SYNC
#define GPIO84_OEN 0x102552, BIT5
#define GPIO84_OUT 0x102552, BIT4
#define GPIO84_IN  0x102552, BIT0

#define GPIO85_PAD PAD_TS2_VLD
#define GPIO85_OEN 0x102554, BIT5
#define GPIO85_OUT 0x102554, BIT4
#define GPIO85_IN  0x102554, BIT0

#define GPIO86_PAD PAD_TS2_D0
#define GPIO86_OEN 0x102556, BIT5
#define GPIO86_OUT 0x102556, BIT4
#define GPIO86_IN  0x102556, BIT0

#define GPIO87_PAD PAD_TS2_D1
#define GPIO87_OEN 0x1025e6, BIT5
#define GPIO87_OUT 0x1025e6, BIT4
#define GPIO87_IN  0x1025e6, BIT0

#define GPIO88_PAD PAD_TS2_D2
#define GPIO88_OEN 0x1025e8, BIT5
#define GPIO88_OUT 0x1025e8, BIT4
#define GPIO88_IN  0x1025e8, BIT0

#define GPIO89_PAD PAD_TS2_D3
#define GPIO89_OEN 0x1025ea, BIT5
#define GPIO89_OUT 0x1025ea, BIT4
#define GPIO89_IN  0x1025ea, BIT0

#define GPIO90_PAD PAD_TS2_D4
#define GPIO90_OEN 0x1025ec, BIT5
#define GPIO90_OUT 0x1025ec, BIT4
#define GPIO90_IN  0x1025ec, BIT0

#define GPIO91_PAD PAD_TS2_D5
#define GPIO91_OEN 0x1025ee, BIT5
#define GPIO91_OUT 0x1025ee, BIT4
#define GPIO91_IN  0x1025ee, BIT0

#define GPIO92_PAD PAD_TS2_D6
#define GPIO92_OEN 0x1025f0, BIT5
#define GPIO92_OUT 0x1025f0, BIT4
#define GPIO92_IN  0x1025f0, BIT0

#define GPIO93_PAD PAD_TS2_D7
#define GPIO93_OEN 0x1025f2, BIT5
#define GPIO93_OUT 0x1025f2, BIT4
#define GPIO93_IN  0x1025f2, BIT0

#define GPIO94_PAD PAD_TS1_CLK
#define GPIO94_OEN 0x10253a, BIT5
#define GPIO94_OUT 0x10253a, BIT4
#define GPIO94_IN  0x10253a, BIT0

#define GPIO95_PAD PAD_TS1_SYNC
#define GPIO95_OEN 0x10253c, BIT5
#define GPIO95_OUT 0x10253c, BIT4
#define GPIO95_IN  0x10253c, BIT0

#define GPIO96_PAD PAD_TS1_VLD
#define GPIO96_OEN 0x10253e, BIT5
#define GPIO96_OUT 0x10253e, BIT4
#define GPIO96_IN  0x10253e, BIT0

#define GPIO97_PAD PAD_TS1_D0
#define GPIO97_OEN 0x10254e, BIT5
#define GPIO97_OUT 0x10254e, BIT4
#define GPIO97_IN  0x10254e, BIT0

#define GPIO98_PAD PAD_TS1_D1
#define GPIO98_OEN 0x10254c, BIT5
#define GPIO98_OUT 0x10254c, BIT4
#define GPIO98_IN  0x10254c, BIT0

#define GPIO99_PAD PAD_TS1_D2
#define GPIO99_OEN 0x10254a, BIT5
#define GPIO99_OUT 0x10254a, BIT4
#define GPIO99_IN  0x10254a, BIT0

#define GPIO100_PAD PAD_TS1_D3
#define GPIO100_OEN 0x102548, BIT5
#define GPIO100_OUT 0x102548, BIT4
#define GPIO100_IN  0x102548, BIT0

#define GPIO101_PAD PAD_TS1_D4
#define GPIO101_OEN 0x102546, BIT5
#define GPIO101_OUT 0x102546, BIT4
#define GPIO101_IN  0x102546, BIT0

#define GPIO102_PAD PAD_TS1_D5
#define GPIO102_OEN 0x102544, BIT5
#define GPIO102_OUT 0x102544, BIT4
#define GPIO102_IN  0x102544, BIT0

#define GPIO103_PAD PAD_TS1_D6
#define GPIO103_OEN 0x102542, BIT5
#define GPIO103_OUT 0x102542, BIT4
#define GPIO103_IN  0x102542, BIT0

#define GPIO104_PAD PAD_TS1_D7
#define GPIO104_OEN 0x102540, BIT5
#define GPIO104_OUT 0x102540, BIT4
#define GPIO104_IN  0x102540, BIT0

#define GPIO105_PAD PAD_TS0_CLK
#define GPIO105_OEN 0x102524, BIT5
#define GPIO105_OUT 0x102524, BIT4
#define GPIO105_IN  0x102524, BIT0

#define GPIO106_PAD PAD_TS0_SYNC
#define GPIO106_OEN 0x102528, BIT5
#define GPIO106_OUT 0x102528, BIT4
#define GPIO106_IN  0x102528, BIT0

#define GPIO107_PAD PAD_TS0_VLD
#define GPIO107_OEN 0x102526, BIT5
#define GPIO107_OUT 0x102526, BIT4
#define GPIO107_IN  0x102526, BIT0

#define GPIO108_PAD PAD_TS0_D0
#define GPIO108_OEN 0x102538, BIT5
#define GPIO108_OUT 0x102538, BIT4
#define GPIO108_IN  0x102538, BIT0

#define GPIO109_PAD PAD_TS0_D1
#define GPIO109_OEN 0x102536, BIT5
#define GPIO109_OUT 0x102536, BIT4
#define GPIO109_IN  0x102536, BIT0

#define GPIO110_PAD PAD_TS0_D2
#define GPIO110_OEN 0x102534, BIT5
#define GPIO110_OUT 0x102534, BIT4
#define GPIO110_IN  0x102534, BIT0

#define GPIO111_PAD PAD_TS0_D3
#define GPIO111_OEN 0x102532, BIT5
#define GPIO111_OUT 0x102532, BIT4
#define GPIO111_IN  0x102532, BIT0

#define GPIO112_PAD PAD_TS0_D4
#define GPIO112_OEN 0x102530, BIT5
#define GPIO112_OUT 0x102530, BIT4
#define GPIO112_IN  0x102530, BIT0

#define GPIO113_PAD PAD_TS0_D5
#define GPIO113_OEN 0x10252e, BIT5
#define GPIO113_OUT 0x10252e, BIT4
#define GPIO113_IN  0x10252e, BIT0

#define GPIO114_PAD PAD_TS0_D6
#define GPIO114_OEN 0x10252c, BIT5
#define GPIO114_OUT 0x10252c, BIT4
#define GPIO114_IN  0x10252c, BIT0

#define GPIO115_PAD PAD_TS0_D7
#define GPIO115_OEN 0x10252a, BIT5
#define GPIO115_OUT 0x10252a, BIT4
#define GPIO115_IN  0x10252a, BIT0

#define GPIO116_PAD PAD_NF_CLE
#define GPIO116_OEN 0x10264a, BIT5
#define GPIO116_OUT 0x10264a, BIT4
#define GPIO116_IN  0x10264a, BIT0

#define GPIO117_PAD PAD_NF_ALE
#define GPIO117_OEN 0x10264c, BIT5
#define GPIO117_OUT 0x10264c, BIT4
#define GPIO117_IN  0x10264c, BIT0

#define GPIO118_PAD PAD_NF_WEZ
#define GPIO118_OEN 0x10264e, BIT5
#define GPIO118_OUT 0x10264e, BIT4
#define GPIO118_IN  0x10264e, BIT0

#define GPIO119_PAD PAD_NF_WPZ
#define GPIO119_OEN 0x102650, BIT5
#define GPIO119_OUT 0x102650, BIT4
#define GPIO119_IN  0x102650, BIT0

#define GPIO120_PAD PAD_NF_D0
#define GPIO120_OEN 0x102652, BIT5
#define GPIO120_OUT 0x102652, BIT4
#define GPIO120_IN  0x102652, BIT0

#define GPIO121_PAD PAD_NF_D1
#define GPIO121_OEN 0x102654, BIT5
#define GPIO121_OUT 0x102654, BIT4
#define GPIO121_IN  0x102654, BIT0

#define GPIO122_PAD PAD_NF_D2
#define GPIO122_OEN 0x102656, BIT5
#define GPIO122_OUT 0x102656, BIT4
#define GPIO122_IN  0x102656, BIT0

#define GPIO123_PAD PAD_NF_D3
#define GPIO123_OEN 0x102658, BIT5
#define GPIO123_OUT 0x102658, BIT4
#define GPIO123_IN  0x102658, BIT0

#define GPIO124_PAD PAD_NF_D4
#define GPIO124_OEN 0x10265a, BIT5
#define GPIO124_OUT 0x10265a, BIT4
#define GPIO124_IN  0x10265a, BIT0

#define GPIO125_PAD PAD_NF_D5
#define GPIO125_OEN 0x10265c, BIT5
#define GPIO125_OUT 0x10265c, BIT4
#define GPIO125_IN  0x10265c, BIT0

#define GPIO126_PAD PAD_NF_D6
#define GPIO126_OEN 0x10265e, BIT5
#define GPIO126_OUT 0x10265e, BIT4
#define GPIO126_IN  0x10265e, BIT0

#define GPIO127_PAD PAD_NF_D7
#define GPIO127_OEN 0x102660, BIT5
#define GPIO127_OUT 0x102660, BIT4
#define GPIO127_IN  0x102660, BIT0

#define GPIO128_PAD PAD_NF_RBZ
#define GPIO128_OEN 0x102646, BIT5
#define GPIO128_OUT 0x102646, BIT4
#define GPIO128_IN  0x102646, BIT0

#define GPIO129_PAD PAD_NF_REZ
#define GPIO129_OEN 0x102648, BIT5
#define GPIO129_OUT 0x102648, BIT4
#define GPIO129_IN  0x102648, BIT0

#define GPIO130_PAD PAD_NF_CEZ_BGA
#define GPIO130_OEN 0x102644, BIT5
#define GPIO130_OUT 0x102644, BIT4
#define GPIO130_IN  0x102644, BIT0

#define GPIO131_PAD PAD_NF_CE1Z_BGA
#define GPIO131_OEN 0x102662, BIT5
#define GPIO131_OUT 0x102662, BIT4
#define GPIO131_IN  0x102662, BIT0

#define GPIO132_PAD PAD_DM_GPIO1
#define GPIO132_OEN 0x102642, BIT5
#define GPIO132_OUT 0x102642, BIT4
#define GPIO132_IN  0x102642, BIT0

#define GPIO133_PAD PAD_DM_GPIO0
#define GPIO133_OEN 0x102640, BIT5
#define GPIO133_OUT 0x102640, BIT4
#define GPIO133_IN  0x102640, BIT0

#define GPIO134_PAD PAD_S_GPIO0
#define GPIO134_OEN 0x101e41, BIT0
#define GPIO134_OUT 0x101e40, BIT0
#define GPIO134_IN  0x101e44, BIT0

#define GPIO135_PAD PAD_S_GPIO1
#define GPIO135_OEN 0x101e41, BIT1
#define GPIO135_OUT 0x101e40, BIT1
#define GPIO135_IN  0x101e44, BIT1

#define GPIO136_PAD PAD_S_GPIO2
#define GPIO136_OEN 0x101e41, BIT2
#define GPIO136_OUT 0x101e40, BIT2
#define GPIO136_IN  0x101e44, BIT2

#define GPIO137_PAD PAD_VSYNC_OUT
#define GPIO137_OEN 0x10263e, BIT5
#define GPIO137_OUT 0x10263e, BIT4
#define GPIO137_IN  0x10263e, BIT0

#define GPIO138_PAD PAD_HSYNC_OUT
#define GPIO138_OEN 0x10263c, BIT5
#define GPIO138_OUT 0x10263c, BIT4
#define GPIO138_IN  0x10263c, BIT0

#define GPIO139_PAD PAD_GPIO_PM0
#define GPIO139_OEN 0x0e1e, BIT0
#define GPIO139_OUT 0x0e20, BIT0
#define GPIO139_IN  0x0e22, BIT0

#define GPIO140_PAD PAD_GPIO_PM1
#define GPIO140_OEN 0x0e1e, BIT1
#define GPIO140_OUT 0x0e20, BIT1
#define GPIO140_IN  0x0e22, BIT1

#define GPIO141_PAD PAD_GPIO_PM2
#define GPIO141_OEN 0x0e1e, BIT2
#define GPIO141_OUT 0x0e20, BIT2
#define GPIO141_IN  0x0e22, BIT2

#define GPIO142_PAD PAD_GPIO_PM3
#define GPIO142_OEN 0x0e1e, BIT3
#define GPIO142_OUT 0x0e20, BIT3
#define GPIO142_IN  0x0e22, BIT3

#define GPIO143_PAD PAD_GPIO_PM4
#define GPIO143_OEN 0x0e1e, BIT4
#define GPIO143_OUT 0x0e20, BIT4
#define GPIO143_IN  0x0e22, BIT4

#define GPIO144_PAD PAD_GPIO_PM5
#define GPIO144_OEN 0x0e1e, BIT5
#define GPIO144_OUT 0x0e20, BIT5
#define GPIO144_IN  0x0e22, BIT5

#define GPIO145_PAD PAD_GPIO_PM6
#define GPIO145_OEN 0x0e1e, BIT6
#define GPIO145_OUT 0x0e20, BIT6
#define GPIO145_IN  0x0e22, BIT6

#define GPIO146_PAD PAD_GPIO_PM7
#define GPIO146_OEN 0x0e1e, BIT7
#define GPIO146_OUT 0x0e20, BIT7
#define GPIO146_IN  0x0e22, BIT7

#define GPIO147_PAD PAD_GPIO_PM8
#define GPIO147_OEN 0x0e1f, BIT0
#define GPIO147_OUT 0x0e21, BIT0
#define GPIO147_IN  0x0e23, BIT0

#define GPIO148_PAD PAD_GPIO_PM9
#define GPIO148_OEN 0x0e1f, BIT1
#define GPIO148_OUT 0x0e21, BIT1
#define GPIO148_IN  0x0e23, BIT1

#define GPIO149_PAD PAD_GPIO_PM10
#define GPIO149_OEN 0x0e1f, BIT2
#define GPIO149_OUT 0x0e21, BIT2
#define GPIO149_IN  0x0e23, BIT2

#define GPIO150_PAD PAD_GPIO_PM11
#define GPIO150_OEN 0x0e1f, BIT3
#define GPIO150_OUT 0x0e21, BIT3
#define GPIO150_IN  0x0e23, BIT3

#define GPIO151_PAD PAD_GPIO_PM12
#define GPIO151_OEN 0x0e1f, BIT4
#define GPIO151_OUT 0x0e21, BIT4
#define GPIO151_IN  0x0e23, BIT4

#define GPIO152_PAD PAD_IRIN
#define GPIO152_OEN 0x0e3c, BIT0
#define GPIO152_OUT 0x0e3a, BIT0
#define GPIO152_IN  0x0e3b, BIT0

#define GPIO153_PAD PAD_CEC_RX
#define GPIO153_OEN 0x0e3c, BIT2
#define GPIO153_OUT 0x0e3a, BIT2
#define GPIO153_IN  0x0e3b, BIT2

#define GPIO154_PAD PAD_IRIN2
#define GPIO154_OEN 0x0e3c, BIT3
#define GPIO154_OUT 0x0e3a, BIT3
#define GPIO154_IN  0x0e3b, BIT3

#define GPIO155_PAD PAD_PM_SPI_CZ
#define GPIO155_OEN 0x0e3c, BIT4
#define GPIO155_OUT 0x0e3a, BIT4
#define GPIO155_IN  0x0e3b, BIT4

#define GPIO156_PAD PAD_PM_SPI_CK
#define GPIO156_OEN 0x0e3c, BIT5
#define GPIO156_OUT 0x0e3a, BIT5
#define GPIO156_IN  0x0e3b, BIT5

#define GPIO157_PAD PAD_PM_SPI_DI
#define GPIO157_OEN 0x0e3c, BIT6
#define GPIO157_OUT 0x0e3a, BIT6
#define GPIO157_IN  0x0e3b, BIT6

#define GPIO158_PAD PAD_PM_SPI_DO
#define GPIO158_OEN 0x0e3c, BIT7
#define GPIO158_OUT 0x0e3a, BIT7
#define GPIO158_IN  0x0e3b, BIT7

#define GPIO159_PAD PAD_SAR_GPIO0
#define GPIO159_OEN 0x1423, BIT0
#define GPIO159_OUT 0x1424, BIT0
#define GPIO159_IN  0x1425, BIT0

#define GPIO160_PAD PAD_SAR_GPIO1
#define GPIO160_OEN 0x1423, BIT1
#define GPIO160_OUT 0x1424, BIT1
#define GPIO160_IN  0x1425, BIT1

#define GPIO161_PAD PAD_SAR_GPIO2
#define GPIO161_OEN 0x1423, BIT2
#define GPIO161_OUT 0x1424, BIT2
#define GPIO161_IN  0x1425, BIT2

#define GPIO162_PAD PAD_SAR_GPIO3
#define GPIO162_OEN 0x1423, BIT3
#define GPIO162_OUT 0x1424, BIT3
#define GPIO162_IN  0x1425, BIT3

#define GPIO163_PAD PAD_ET_COL
#define GPIO163_OEN 0x0ed2, BIT0
#define GPIO163_OUT 0x0ece, BIT0
#define GPIO163_IN  0x0ed0, BIT0

#define GPIO164_PAD PAD_ET_RXD0
#define GPIO164_OEN 0x0ed2, BIT1
#define GPIO164_OUT 0x0ece, BIT1
#define GPIO164_IN  0x0ed0, BIT1

#define GPIO165_PAD PAD_ET_RXD1(TMII_CRS)
#define GPIO165_OEN 0x0ed2, BIT2
#define GPIO165_OUT 0x0ece, BIT2
#define GPIO165_IN  0x0ed0, BIT2

#define GPIO166_PAD PAD_ET_TXD0(TMII_RXER)
#define GPIO166_OEN 0x0ed2, BIT3
#define GPIO166_OUT 0x0ece, BIT3
#define GPIO166_IN  0x0ed0, BIT3

#define GPIO167_PAD PAD_ET_TXD1(TMII_COL)
#define GPIO167_OEN 0x0ed2, BIT4
#define GPIO167_OUT 0x0ece, BIT4
#define GPIO167_IN  0x0ed0, BIT4

#define GPIO168_PAD PAD_ET_TX_EN(TMII_MDIO)
#define GPIO168_OEN 0x0ed2, BIT5
#define GPIO168_OUT 0x0ece, BIT5
#define GPIO168_IN  0x0ed0, BIT5

#define GPIO169_PAD PAD_ET_MDC
#define GPIO169_OEN 0x0ed2, BIT6
#define GPIO169_OUT 0x0ece, BIT6
#define GPIO169_IN  0x0ed0, BIT6

#define GPIO170_PAD PAD_ET_MDIO
#define GPIO170_OEN 0x0ed2, BIT7
#define GPIO170_OUT 0x0ece, BIT7
#define GPIO170_IN  0x0ed0, BIT7

#define GPIO171_PAD PAD_ET_TX_CLK
#define GPIO171_OEN 0x0ed3, BIT0
#define GPIO171_OUT 0x0ecf, BIT0
#define GPIO171_IN  0x0ed1, BIT0

#define GPIO172_PAD PAD_LED0_PM
#define GPIO172_OEN 0x0ed3, BIT1
#define GPIO172_OUT 0x0ecf, BIT1
#define GPIO172_IN  0x0ed1, BIT1

#define GPIO173_PAD PAD_LED1_PM
#define GPIO173_OEN 0x0ed3, BIT2
#define GPIO173_OUT 0x0ecf, BIT2
#define GPIO173_IN  0x0ed1, BIT2

#define GPIO174_PAD PAD_GT_RX_CTL
#define GPIO174_OEN 0x0ef2, BIT0
#define GPIO174_OUT 0x0eee, BIT0
#define GPIO174_IN  0x0ef0, BIT0

#define GPIO175_PAD PAD_GT_RX_D0
#define GPIO175_OEN 0x0ef2, BIT1
#define GPIO175_OUT 0x0eee, BIT1
#define GPIO175_IN  0x0ef0, BIT1

#define GPIO176_PAD PAD_GT_RX_D1
#define GPIO176_OEN 0x0ef2, BIT2
#define GPIO176_OUT 0x0eee, BIT2
#define GPIO176_IN  0x0ef0, BIT2

#define GPIO177_PAD PAD_GT_RX_D2
#define GPIO177_OEN 0x0ef2, BIT3
#define GPIO177_OUT 0x0eee, BIT3
#define GPIO177_IN  0x0ef0, BIT3

#define GPIO178_PAD PAD_GT_RX_D3
#define GPIO178_OEN 0x0ef2, BIT4
#define GPIO178_OUT 0x0eee, BIT4
#define GPIO178_IN  0x0ef0, BIT4

#define GPIO179_PAD PAD_GT_RX_CLK
#define GPIO179_OEN 0x0ef2, BIT5
#define GPIO179_OUT 0x0eee, BIT5
#define GPIO179_IN  0x0ef0, BIT5

#define GPIO180_PAD PAD_GT_TX_CTL
#define GPIO180_OEN 0x0ef2, BIT6
#define GPIO180_OUT 0x0eee, BIT6
#define GPIO180_IN  0x0ef0, BIT6

#define GPIO181_PAD PAD_GT_TX_D0
#define GPIO181_OEN 0x0ef2, BIT7
#define GPIO181_OUT 0x0eee, BIT7
#define GPIO181_IN  0x0ef0, BIT7

#define GPIO182_PAD PAD_GT_TX_D1
#define GPIO182_OEN 0x0ef3, BIT0
#define GPIO182_OUT 0x0eef, BIT0
#define GPIO182_IN  0x0ef1, BIT0

#define GPIO183_PAD PAD_GT_TX_D2
#define GPIO183_OEN 0x0ef3, BIT1
#define GPIO183_OUT 0x0eef, BIT1
#define GPIO183_IN  0x0ef1, BIT1

#define GPIO184_PAD PAD_GT_TX_D3
#define GPIO184_OEN 0x0ef3, BIT2
#define GPIO184_OUT 0x0eef, BIT2
#define GPIO184_IN  0x0ef1, BIT2

#define GPIO185_PAD PAD_GT_TX_CLK
#define GPIO185_OEN 0x0ef3, BIT3
#define GPIO185_OUT 0x0eef, BIT3
#define GPIO185_IN  0x0ef1, BIT3

#define GPIO186_PAD PAD_GT_MDC
#define GPIO186_OEN 0x0ef3, BIT4
#define GPIO186_OUT 0x0eef, BIT4
#define GPIO186_IN  0x0ef1, BIT4

#define GPIO187_PAD PAD_GT_MDIO
#define GPIO187_OEN 0x0ef3, BIT5
#define GPIO187_OUT 0x0eef, BIT5
#define GPIO187_IN  0x0ef1, BIT5

#define GPIO188_PAD PAD_GT_FR_CLK
#define GPIO188_OEN 0x0ef3, BIT6
#define GPIO188_OUT 0x0eef, BIT6
#define GPIO188_IN  0x0ef1, BIT6



//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static const struct gpio_setting
{
    U32 r_oen;
    U8  m_oen;
    U32 r_out;
    U8  m_out;
    U32 r_in;
    U8  m_in;
} gpio_table[] =
{
#define __GPIO__(_x_)   { CONCAT(CONCAT(GPIO, _x_), _OEN),   \
                          CONCAT(CONCAT(GPIO, _x_), _OUT),   \
                          CONCAT(CONCAT(GPIO, _x_), _IN) }
#define __GPIO(_x_)     __GPIO__(_x_)

//
// !! WARNING !! DO NOT MODIFIY !!!!
//
// These defines order must match following
// 1. the PAD name in GPIO excel
// 2. the perl script to generate the package header file
//
    //__GPIO(999), // 0 is not used

    __GPIO(0), __GPIO(1), __GPIO(2), __GPIO(3), __GPIO(4),
    __GPIO(5), __GPIO(6), __GPIO(7), __GPIO(8), __GPIO(9),
    __GPIO(10), __GPIO(11), __GPIO(12), __GPIO(13), __GPIO(14),
    __GPIO(15), __GPIO(16), __GPIO(17), __GPIO(18), __GPIO(19),
    __GPIO(20), __GPIO(21), __GPIO(22), __GPIO(23), __GPIO(24),
    __GPIO(25), __GPIO(26), __GPIO(27), __GPIO(28), __GPIO(29),
    __GPIO(30), __GPIO(31), __GPIO(32), __GPIO(33), __GPIO(34),
    __GPIO(35), __GPIO(36), __GPIO(37), __GPIO(38), __GPIO(39),
    __GPIO(40), __GPIO(41), __GPIO(42), __GPIO(43), __GPIO(44),
    __GPIO(45), __GPIO(46), __GPIO(47), __GPIO(48), __GPIO(49),
    __GPIO(50), __GPIO(51), __GPIO(52), __GPIO(53), __GPIO(54),
    __GPIO(55), __GPIO(56), __GPIO(57), __GPIO(58), __GPIO(59),
    __GPIO(60), __GPIO(61), __GPIO(62), __GPIO(63), __GPIO(64),
    __GPIO(65), __GPIO(66), __GPIO(67), __GPIO(68), __GPIO(69),
    __GPIO(70), __GPIO(71), __GPIO(72), __GPIO(73), __GPIO(74),
    __GPIO(75), __GPIO(76), __GPIO(77), __GPIO(78), __GPIO(79),
    __GPIO(80), __GPIO(81), __GPIO(82), __GPIO(83), __GPIO(84),
    __GPIO(85), __GPIO(86), __GPIO(87), __GPIO(88), __GPIO(89),
    __GPIO(90), __GPIO(91), __GPIO(92), __GPIO(93), __GPIO(94),
    __GPIO(95), __GPIO(96), __GPIO(97), __GPIO(98), __GPIO(99),
    __GPIO(100), __GPIO(101), __GPIO(102), __GPIO(103), __GPIO(104),
    __GPIO(105), __GPIO(106), __GPIO(107), __GPIO(108), __GPIO(109),
    __GPIO(110), __GPIO(111), __GPIO(112), __GPIO(113), __GPIO(114),
    __GPIO(115), __GPIO(116), __GPIO(117), __GPIO(118), __GPIO(119),
    __GPIO(120), __GPIO(121), __GPIO(122), __GPIO(123), __GPIO(124),
    __GPIO(125), __GPIO(126), __GPIO(127), __GPIO(128), __GPIO(129),
    __GPIO(130), __GPIO(131), __GPIO(132), __GPIO(133), __GPIO(134),
    __GPIO(135), __GPIO(136), __GPIO(137), __GPIO(138), __GPIO(139),
    __GPIO(140), __GPIO(141), __GPIO(142), __GPIO(143), __GPIO(144),
    __GPIO(145), __GPIO(146), __GPIO(147), __GPIO(148), __GPIO(149),
    __GPIO(150), __GPIO(151), __GPIO(152), __GPIO(153), __GPIO(154),
    __GPIO(155), __GPIO(156), __GPIO(157), __GPIO(158), __GPIO(159),
    __GPIO(160), __GPIO(161), __GPIO(162), __GPIO(163), __GPIO(164),
    __GPIO(165), __GPIO(166), __GPIO(167), __GPIO(168), __GPIO(169),
    __GPIO(170), __GPIO(171), __GPIO(172), __GPIO(173), __GPIO(174),
    __GPIO(175), __GPIO(176), __GPIO(177), __GPIO(178), __GPIO(179),
    __GPIO(180), __GPIO(181), __GPIO(182), __GPIO(183), __GPIO(184),
    __GPIO(185), __GPIO(186), __GPIO(187), __GPIO(188),

};
#endif

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

//the functions of this section set to initialize
void MHal_GPIO_Init(void)
{
    MHal_GPIO_REG(REG_ALL_PAD_IN) &= ~BIT7;
}

void MHal_GPIO_WriteRegBit(U32 u32Reg, U8 u8Enable, U8 u8BitMsk)
{
    if(u8Enable)
        MHal_GPIO_REG(u32Reg) |= u8BitMsk;
    else
        MHal_GPIO_REG(u32Reg) &= (~u8BitMsk);
}

U8 MHal_GPIO_ReadRegBit(U32 u32Reg, U8 u8BitMsk)
{
    return ((MHal_GPIO_REG(u32Reg)&u8BitMsk)? 1 : 0);
}

void MHal_GPIO_Pad_Set(U8 u8IndexGPIO)
{

}
void MHal_GPIO_Pad_Oen(U8 u8IndexGPIO)
{
    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_oen) &= (~gpio_table[u8IndexGPIO].m_oen);
}

void MHal_GPIO_Pad_Odn(U8 u8IndexGPIO)
{
    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_oen) |= gpio_table[u8IndexGPIO].m_oen;
}

U8 MHal_GPIO_Pad_Level(U8 u8IndexGPIO)
{
    return ((MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_in)&gpio_table[u8IndexGPIO].m_in)? 1 : 0);
}

U8 MHal_GPIO_Pad_InOut(U8 u8IndexGPIO)
{
    return ((MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_oen)&gpio_table[u8IndexGPIO].m_oen)? 1 : 0);
}

void MHal_GPIO_Pull_High(U8 u8IndexGPIO)
{
    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_out) |= gpio_table[u8IndexGPIO].m_out;
}

void MHal_GPIO_Pull_Low(U8 u8IndexGPIO)
{
    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_out) &= (~gpio_table[u8IndexGPIO].m_out);
}

void MHal_GPIO_Set_High(U8 u8IndexGPIO)
{
    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_oen) &= (~gpio_table[u8IndexGPIO].m_oen);
    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_out) |= gpio_table[u8IndexGPIO].m_out;
}

void MHal_GPIO_Set_Low(U8 u8IndexGPIO)
{
    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_oen) &= (~gpio_table[u8IndexGPIO].m_oen);
    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_out) &= (~gpio_table[u8IndexGPIO].m_out);
}

