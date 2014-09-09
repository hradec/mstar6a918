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
#define GPIO_UNIT_NUM               180

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
#define PAD_DDCA_CK                 8
#define PAD_DDCA_DA                 9
#define PAD_GPIO_PM0                10
#define PAD_GPIO_PM1                11
#define PAD_GPIO_PM2                12
#define PAD_GPIO_PM3                13
#define PAD_GPIO_PM4                14
#define PAD_GPIO_PM5                15
#define PAD_GPIO_PM6                16
#define PAD_GPIO_PM7                17
#define PAD_GPIO_PM8                18
#define PAD_GPIO_PM9                19
#define PAD_GPIO_PM10               20
#define PAD_GPIO_PM11               21
#define PAD_GPIO_PM12               22
#define PAD_GPIO_PM13               23
#define PAD_GPIO_PM14               24
#define PAD_GPIO_PM15               25
#define PAD_GPIO_PM16               26
#define PAD_GPIO_PM17               27
#define PAD_GPIO_PM18               28
#define PAD_LED0                    29
#define PAD_LED1                    30
#define PAD_HOTPLUGA                31
#define PAD_HOTPLUGB                32
#define PAD_HOTPLUGC                33
#define PAD_HOTPLUGD                34
#define PAD_DDCDA_CK                35
#define PAD_DDCDA_DA                36
#define PAD_DDCDB_CK                37
#define PAD_DDCDB_DA                38
#define PAD_DDCDC_CK                39
#define PAD_DDCDC_DA                40
#define PAD_DDCDD_CK                41
#define PAD_DDCDD_DA                42
#define PAD_SAR0                    43
#define PAD_SAR1                    44
#define PAD_SAR2                    45
#define PAD_SAR3                    46
#define PAD_SAR4                    47
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
#define PAD_GPIO28                  81
#define PAD_GPIO29                  82
#define PAD_GPIO30                  83
#define PAD_GPIO31                  84
#define PAD_GPIO32                  85
#define PAD_GPIO33                  86
#define PAD_GPIO34                  87
#define PAD_GPIO35                  88
#define PAD_GPIO36                  89
#define PAD_GPIO37                  90
#define PAD_I2S_IN_WS               91
#define PAD_I2S_IN_BCK              92
#define PAD_I2S_IN_SD               93
#define PAD_SPDIF_IN                94
#define PAD_SPDIF_OUT               95
#define PAD_I2S_OUT_WS              96
#define PAD_I2S_OUT_MCK             97
#define PAD_I2S_OUT_BCK             98
#define PAD_I2S_OUT_SD              99
#define PAD_I2S_OUT_SD1             100
#define PAD_I2S_OUT_SD2             101
#define PAD_I2S_OUT_SD3             102
#define PAD_VSYNC_Like              103
#define PAD_SPI1_CK                 104
#define PAD_SPI1_DI                 105
#define PAD_SPI2_CK                 106
#define PAD_SPI2_DI                 107
#define PAD_DIM0                    108
#define PAD_DIM1                    109
#define PAD_DIM2                    110
#define PAD_DIM3                    111
#define PAD_PCM2_CE_N               112
#define PAD_PCM2_IRQA_N             113
#define PAD_PCM2_WAIT_N             114
#define PAD_PCM2_RESET              115
#define PAD_PCM2_CD_N               116
#define PAD_PCM_D3                  117
#define PAD_PCM_D4                  118
#define PAD_PCM_D5                  119
#define PAD_PCM_D6                  120
#define PAD_PCM_D7                  121
#define PAD_PCM_CE_N                122
#define PAD_PCM_A10                 123
#define PAD_PCM_OE_N                124
#define PAD_PCM_A11                 125
#define PAD_PCM_IORD_N              126
#define PAD_PCM_A9                  112
#define PAD_PCM_IOWR_N              113
#define PAD_PCM_A8                  114
#define PAD_PCM_A13                 115
#define PAD_PCM_A14                 116
#define PAD_PCM_WE_N                117
#define PAD_PCM_IRQA_N              118
#define PAD_PCM_A12                 119
#define PAD_PCM_A7                  120
#define PAD_PCM_A6                  121
#define PAD_PCM_A5                  122
#define PAD_PCM_WAIT_N              123
#define PAD_PCM_A4                  124
#define PAD_PCM_A3                  125
#define PAD_PCM_A2                  126
#define PAD_PCM_REG_N               127
#define PAD_PCM_A1                  128
#define PAD_PCM_A0                  129
#define PAD_PCM_D0                  130
#define PAD_PCM_D1                  131
#define PAD_PCM_D2                  132
#define PAD_PCM_RESET               133
#define PAD_PCM_CD_N                134
#define PAD_PWM0                    135
#define PAD_PWM1                    136
#define PAD_PWM2                    137
#define PAD_PWM3                    138
#define PAD_PWM4                    139
#define PAD_TCON0                    140
#define PAD_TCON1               141
#define PAD_TCON2                142
#define PAD_TCON3                143
#define PAD_TCON4                   144
#define PAD_TCON5                   145
#define PAD_TCON6                   146
#define PAD_TCON7                   147
#define PAD_TCON8                   148
#define PAD_TCON9                   149
#define PAD_TCON10                   150
#define PAD_TCON11                   151
#define PAD_TCON12                  152
#define PAD_TCON13                  153
#define PAD_TGPIO0                  153
#define PAD_TGPIO1                  153
#define PAD_TGPIO2                  154
#define PAD_TGPIO3                  155
#define PAD_TS0_D0                  156
#define PAD_TS0_D1                  157
#define PAD_TS0_D2                  158
#define PAD_TS0_D3                  159
#define PAD_TS0_D4                  160
#define PAD_TS0_D5                  161
#define PAD_TS0_D6                  162
#define PAD_TS0_D7                  163
#define PAD_TS0_VLD                 164
#define PAD_TS0_SYNC                165
#define PAD_TS0_CLK                 166
#define PAD_TS1_CLK                 167
#define PAD_TS1_SYNC                168
#define PAD_TS1_VLD                 169
#define PAD_TS1_D7                  170
#define PAD_TS1_D6                  171
#define PAD_TS1_D5                  172
#define PAD_TS1_D4                  173
#define PAD_TS1_D3                  174
#define PAD_TS1_D2                  175
#define PAD_TS1_D1                  176
#define PAD_TS1_D0                  177
#define PAD_NAND_CEZ                178
#define PAD_NAND_CEZ1                178
#define PAD_NAND_CLE                179
#define PAD_NAND_REZ                179
#define PAD_NAND_WEZ                179
#define PAD_NAND_WPZ                179
#define PAD_NAND_ALE                179
#define PAD_NAND_RBZ                179
#define PAD_NAND_DQS                179
#define PAD_EMMC_RSTN                179
#define PAD_EMMC_CLK                179

#define PAD_EMMC_CMD                179

#define PAD_TS2_D0                179
#define PAD_TS2_VLD                179
#define PAD_TS2_SYNC                179
#define PAD_TS2_CLK                179

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

