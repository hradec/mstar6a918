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

#ifndef _REG_GPIO_H_
#define _REG_GPIO_H_

//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------
#define GPIO_UNIT_NUM               214

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define REG_MIPS_BASE               0xFD000000      //Use 8 bit addressing

#define REG_ALL_PAD_IN              (0x101ea1)      //set all pads (except SPI) as input
#define REG_LVDS_BASE               (0x103200)
#define REG_LVDS_BANK               REG_LVDS_BASE

#define PAD_PM_SPI_CZ               0  
#define PAD_PM_SPI_CK               1  
#define PAD_PM_SPI_DI               2  
#define PAD_PM_SPI_DO               3  
#define PAD_IRIN                    4  
#define PAD_CEC                     5  
#define PAD_AV_LNK                  6  
#define PAD_PWM_PM                  7  
#define PAD_GPIO_PM0                8  
#define PAD_GPIO_PM1                9  
#define PAD_GPIO_PM2                10 
#define PAD_GPIO_PM3                11 
#define PAD_GPIO_PM4                12 
#define PAD_GPIO_PM5                13 
#define PAD_GPIO_PM6                14 
#define PAD_GPIO_PM7                15 
#define PAD_GPIO_PM8                16 
#define PAD_GPIO_PM9                17 
#define PAD_GPIO_PM10               18 
#define PAD_GPIO_PM11               19 
#define PAD_GPIO_PM12               20 
#define PAD_GPIO_PM13               21 
#define PAD_GPIO_PM14               22 
#define PAD_GPIO_PM15               23 
#define PAD_GPIO_PM16               24 
#define PAD_GPIO_PM17               25 
#define PAD_GPIO_PM18               26 
#define PAD_HOTPLUGA                27 
#define PAD_HOTPLUGB                28 
#define PAD_HOTPLUGC                29 
#define PAD_HOTPLUGD                30 
#define PAD_DDCA_CK                 31 
#define PAD_DDCA_DA                 32 
#define PAD_DDCDA_CK                33 
#define PAD_DDCDA_DA                34 
#define PAD_DDCDB_CK                35 
#define PAD_DDCDB_DA                36 
#define PAD_DDCDC_CK                37 
#define PAD_DDCDC_DA                38 
#define PAD_DDCDD_CK                39 
#define PAD_DDCDD_DA                40 
#define PAD_SAR_GPIO0               41 
#define PAD_SAR_GPIO1               42 
#define PAD_SAR_GPIO2               43 
#define PAD_SAR_GPIO3               44 
#define PAD_SAR_GPIO4               45 
#define PAD_LED0                    46 
#define PAD_LED1                    47 
#define PAD_VID0                    48 
#define PAD_VID1                    49 
#define PAD_WOL_INT                 50 
#define PAD_DDCR_DA                 51 
#define PAD_DDCR_CK                 52 
#define PAD_GPIO0                   53 
#define PAD_GPIO1                   54 
#define PAD_GPIO2                   55 
#define PAD_GPIO3                   56 
#define PAD_GPIO4                   57 
#define PAD_GPIO5                   58 
#define PAD_GPIO6                   59 
#define PAD_GPIO7                   60 
#define PAD_GPIO8                   61 
#define PAD_GPIO9                   62 
#define PAD_GPIO10                  63 
#define PAD_GPIO11                  64 
#define PAD_GPIO12                  65 
#define PAD_GPIO13                  66 
#define PAD_GPIO14                  67 
#define PAD_GPIO15                  68 
#define PAD_GPIO16                  69 
#define PAD_GPIO17                  70 
#define PAD_GPIO18                  71 
#define PAD_GPIO19                  72 
#define PAD_GPIO20                  73 
#define PAD_GPIO21                  74 
#define PAD_GPIO22                  75 
#define PAD_GPIO23                  76 
#define PAD_GPIO24                  77 
#define PAD_GPIO25                  78 
#define PAD_GPIO26                  79 
#define PAD_GPIO27                  80 
#define PAD_I2S_IN_WS               81 
#define PAD_I2S_IN_BCK              82 
#define PAD_I2S_IN_SD               83 
#define PAD_SPDIF_IN                84 
#define PAD_SPDIF_OUT               85 
#define PAD_I2S_OUT_WS              86 
#define PAD_I2S_OUT_MCK             87 
#define PAD_I2S_OUT_BCK             88 
#define PAD_I2S_OUT_SD              89 
#define PAD_I2S_OUT_SD1             90 
#define PAD_I2S_OUT_SD2             91 
#define PAD_I2S_OUT_SD3             92 
#define PAD_VSYNC_Like              93 
#define PAD_SPI1_CK                 94 
#define PAD_SPI1_DI                 95 
#define PAD_SPI2_CK                 96 
#define PAD_SPI2_DI                 97 
#define PAD_NAND_CEZ                98 
#define PAD_NAND_CEZ1               99 
#define PAD_NAND_CLE                100
#define PAD_NAND_REZ                101
#define PAD_NAND_WEZ                102
#define PAD_NAND_WPZ                103
#define PAD_NAND_ALE                104
#define PAD_NAND_RBZ                105
#define PAD_NAND_DQS                106
#define PAD_PCM2_CE_N               107
#define PAD_PCM2_IRQA_N             108
#define PAD_PCM2_WAIT_N             109
#define PAD_PCM2_RESET              110
#define PAD_PCM2_CD_N               111
#define PAD_PCM_D3                  112
#define PAD_PCM_D4                  113
#define PAD_PCM_D5                  114
#define PAD_PCM_D6                  115
#define PAD_PCM_D7                  116
#define PAD_PCM_CE_N                117
#define PAD_PCM_A10                 118
#define PAD_PCM_OE_N                119
#define PAD_PCM_A11                 120
#define PAD_PCM_IORD_N              121
#define PAD_PCM_A9                  122
#define PAD_PCM_IOWR_N              123
#define PAD_PCM_A8                  124
#define PAD_PCM_A13                 125
#define PAD_PCM_A14                 126
#define PAD_PCM_WE_N                127
#define PAD_PCM_IRQA_N              128
#define PAD_PCM_A12                 129
#define PAD_PCM_A7                  130
#define PAD_PCM_A6                  131
#define PAD_PCM_A5                  132
#define PAD_PCM_WAIT_N              133
#define PAD_PCM_A4                  134
#define PAD_PCM_A3                  135
#define PAD_PCM_A2                  136
#define PAD_PCM_REG_N               137
#define PAD_PCM_A1                  138
#define PAD_PCM_A0                  139
#define PAD_PCM_D0                  140
#define PAD_PCM_D1                  141
#define PAD_PCM_D2                  142
#define PAD_PCM_RESET               143
#define PAD_PCM_CD_N                144
#define PAD_PWM0                    145
#define PAD_PWM1                    146
#define PAD_PWM2                    147
#define PAD_PWM3                    148
#define PAD_PWM4                    149
#define PAD_PWM5                    150
#define PAD_PWM6                    151
#define PAD_PWM7                    152
#define PAD_PWM8                    153
#define PAD_PWM9                    154
#define PAD_PWM10                   155
#define PAD_PWM11                   156
#define PAD_PWM12                   157
#define PAD_EMMC_RSTN               158
#define PAD_EMMC_CLK                159
#define PAD_EMMC_CMD                160
#define PAD_TCON0                   161
#define PAD_TCON1                   162
#define PAD_TCON2                   163
#define PAD_TCON3                   164
#define PAD_TCON4                   165
#define PAD_TCON5                   166
#define PAD_TCON6                   167
#define PAD_TCON7                   168
#define PAD_TCON8                   169
#define PAD_TCON9                   170
#define PAD_TCON10                  171
#define PAD_TCON11                  172
#define PAD_TCON12                  173
#define PAD_TCON13                  174
#define PAD_TCON14                  175
#define PAD_TCON15                  176
#define PAD_TGPIO0                  177
#define PAD_TGPIO1                  178
#define PAD_TGPIO2                  179
#define PAD_TGPIO3                  180
#define PAD_TS0_D0                  181
#define PAD_TS0_D1                  182
#define PAD_TS0_D2                  183
#define PAD_TS0_D3                  184
#define PAD_TS0_D4                  185
#define PAD_TS0_D5                  186
#define PAD_TS0_D6                  187
#define PAD_TS0_D7                  188
#define PAD_TS0_VLD                 189
#define PAD_TS0_SYNC                190
#define PAD_TS0_CLK                 191
#define PAD_TS1_CLK                 192
#define PAD_TS1_SYNC                193
#define PAD_TS1_VLD                 194
#define PAD_TS1_D7                  195
#define PAD_TS1_D6                  196
#define PAD_TS1_D5                  197
#define PAD_TS1_D4                  198
#define PAD_TS1_D3                  199
#define PAD_TS1_D2                  200
#define PAD_TS1_D1                  201
#define PAD_TS1_D0                  202
#define PAD_TS2_D0                  203
#define PAD_TS2_D1                  204
#define PAD_TS2_D2                  205
#define PAD_TS2_D3                  206
#define PAD_TS2_D4                  207
#define PAD_TS2_D5                  208
#define PAD_TS2_D6                  209
#define PAD_TS2_D7                  210
#define PAD_TS2_VLD                 211
#define PAD_TS2_SYNC                212
#define PAD_TS2_CLK                 213

#define GPIO_OEN                    0   //set o to nake output
#define GPIO_ODN                    1

#define IN_HIGH                     1   //input high
#define IN_LOW                      0   //input low

#define OUT_HIGH                    1   //output high
#define OUT_LOW                     0   //output low

#define MHal_GPIO_REG(addr)         (*(volatile U8*)(REG_MIPS_BASE + (((addr) & ~1) << 1) + (addr & 1)))

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

#endif // _REG_GPIO_H_

