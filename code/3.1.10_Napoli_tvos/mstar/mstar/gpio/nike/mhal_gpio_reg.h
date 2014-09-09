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
#define PAD_PM_LED                  22
#define PAD_HOTPLUGA                23
#define PAD_HOTPLUGB                24
#define PAD_HOTPLUGC                25
#define PAD_HOTPLUGD                26
#define PAD_DDCDA_CK                27
#define PAD_DDCDA_DA                28
#define PAD_DDCDB_CK                29
#define PAD_DDCDB_DA                30
#define PAD_DDCDC_CK                31
#define PAD_DDCDC_DA                32
#define PAD_DDCDD_CK                33
#define PAD_DDCDD_DA                34
#define PAD_SAR0                    35
#define PAD_SAR1                    36
#define PAD_SAR2                    37
#define PAD_SAR3                    38
#define PAD_SAR4                    39
#define PAD_VID0                    40
#define PAD_VID1                    41
#define PAD_DDCR_DA                 42
#define PAD_DDCR_CK                 43
#define PAD_GPIO0                   44
#define PAD_GPIO1                   45
#define PAD_GPIO2                   46
#define PAD_GPIO3                   47
#define PAD_GPIO4                   48
#define PAD_GPIO5                   49
#define PAD_GPIO6                   50
#define PAD_GPIO7                   51
#define PAD_GPIO8                   52
#define PAD_GPIO9                   53
#define PAD_GPIO10                  54
#define PAD_GPIO11                  55
#define PAD_GPIO12                  56
#define PAD_GPIO13                  57
#define PAD_GPIO14                  58
#define PAD_GPIO15                  59
#define PAD_GPIO16                  60
#define PAD_GPIO17                  61
#define PAD_GPIO18                  62
#define PAD_GPIO19                  63
#define PAD_GPIO20                  64
#define PAD_GPIO21                  65
#define PAD_GPIO22                  66
#define PAD_GPIO23                  67
#define PAD_GPIO24                  68
#define PAD_GPIO25                  69
#define PAD_GPIO26                  70
#define PAD_GPIO27                  71
#define PAD_I2S_IN_WS               72
#define PAD_I2S_IN_BCK              73
#define PAD_I2S_IN_SD               74
#define PAD_SPDIF_IN                75
#define PAD_SPDIF_OUT               76
#define PAD_I2S_OUT_WS              77
#define PAD_I2S_OUT_MCK             78
#define PAD_I2S_OUT_BCK             79
#define PAD_I2S_OUT_SD              80
#define PAD_I2S_OUT_SD1             81
#define PAD_I2S_OUT_SD2             82
#define PAD_I2S_OUT_SD3             83
#define PAD_SPI1_CK                 84
#define PAD_SPI1_DI                 85
#define PAD_SPI2_CK                 86
#define PAD_SPI2_DI                 87
#define PAD_VSYNC_LIKE              88
#define PAD_NAND_CEZ                89
#define PAD_NAND_CEZ1               90
#define PAD_NAND_CLE                91
#define PAD_NAND_REZ                92
#define PAD_NAND_WEZ                93
#define PAD_NAND_WPZ                94
#define PAD_NAND_ALE                95
#define PAD_NAND_RBZ                96
#define PAD_PCM2_CE_N               97
#define PAD_PCM2_IRQA_N             98
#define PAD_PCM2_WAIT_N             99
#define PAD_PCM2_RESET              100
#define PAD_PCM2_CD_N               101
#define PAD_PCM_D3                  102
#define PAD_PCM_D4                  103
#define PAD_PCM_D5                  104
#define PAD_PCM_D6                  105
#define PAD_PCM_D7                  106
#define PAD_PCM_CE_N                107
#define PAD_PCM_A10                 108
#define PAD_PCM_OE_N                109
#define PAD_PCM_A11                 110
#define PAD_PCM_IORD_N              111
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
#define PAD_PWM5                    140
#define PAD_EMMC_RSTN               141
#define PAD_EMMC_CLK                142
#define PAD_EMMC_CMD                143
#define PAD_TCON0                   144
#define PAD_TCON1                   145
#define PAD_TCON2                   146
#define PAD_TCON3                   147
#define PAD_TCON4                   148
#define PAD_TCON5                   149
#define PAD_TCON6                   150
#define PAD_TCON7                   151
#define PAD_TGPIO0                  152
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
#define PAD_UART_TX2                178
#define PAD_UART_RX2                179

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

