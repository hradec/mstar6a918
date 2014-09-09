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

#ifndef IR_FORMAT_H
#define IR_FORMAT_H

//-------------------------------------------------------------------------------------------
// Customer IR Specification parameter define (Please modify them by IR SPEC)
//-------------------------------------------------------------------------------------------
#define IR_INT_NP_EDGE_TRIG
#define IR_MODE_SEL             IR_TYPE_HWRC_MODE
// IR Header code define
#define IR_HEADER_CODE0         0x80    // Custom 0
#define IR_HEADER_CODE1         0x7F    // Custom 1

#define IR_RC_LONGPULSE_THR     1778    //us
#define IR_RC_LONGPULSE_MAR     192     //us only for RC6????
#define IR_RC_INTG_THR_TIME     887     //us
#define IR_RC_WDOG_TIME         24576   // us ???
#define IR_RC_TIMEOUT_TIME      24576   // us ???
#define IR_RC_POWER_WAKEUP_KEY  0xff
#define IR_RC5_BITS             14


// IR Timing define //remove??
#define IR_HEADER_CODE_TIME     9000    // us
#define IR_OFF_CODE_TIME        4500    // us
#define IR_OFF_CODE_RP_TIME     2500    // us
#define IR_LOGI_01H_TIME        560     // us
#define IR_LOGI_0_TIME          1120    // us
#define IR_LOGI_1_TIME          2240    // us
#define IR_TIMEOUT_CYC          140000  // us

#define IR_RC5_DATA_BITS                 26
#define BIT_DELTA                   250
#define HALF_BIT_COUNT             806     // 844/1.048
#define ONE_BIT_COUNT               1611    // 1688/1.048

#define HALF_BIT_LB                 (HALF_BIT_COUNT-BIT_DELTA)
#define HALF_BIT_UB                 (HALF_BIT_COUNT+BIT_DELTA)
#define ONE_BIT_LB                  (ONE_BIT_COUNT-BIT_DELTA)
#define ONE_BIT_UB                  (ONE_BIT_COUNT+BIT_DELTA)

// IR Format define
#define IRKEY_DUMY              0xFF
#define IRDA_KEY_MAPPING_POWER  IRKEY_POWER

typedef enum _IrCommandType
{
    IRKEY_TV_RADIO          = IRKEY_DUMY,
    IRKEY_CHANNEL_LIST      = IRKEY_DUMY,
    IRKEY_CHANNEL_FAV_LIST  = IRKEY_DUMY,
    IRKEY_CHANNEL_RETURN    = IRKEY_DUMY,
    IRKEY_CHANNEL_PLUS      = 0x3A,
    IRKEY_CHANNEL_MINUS     = 0x3B,

    IRKEY_AUDIO             = 0x13,
    IRKEY_VOLUME_PLUS       = 0x3C,
    IRKEY_VOLUME_MINUS      = 0x3D,

    IRKEY_UP                = 0x3A,
    IRKEY_POWER             = 0x2F,
    IRKEY_EXIT              = IRKEY_DUMY,
    IRKEY_MENU              = 0x15,
    IRKEY_DOWN              = 0x3B,
    IRKEY_LEFT              = 0x3D,
    IRKEY_SELECT            = IRKEY_DUMY,
    IRKEY_RIGHT             = 0x3C,

    IRKEY_NUM_0             = 0x30,
    IRKEY_NUM_1             = 0x31,
    IRKEY_NUM_2             = 0x32,
    IRKEY_NUM_3             = 0x33,
    IRKEY_NUM_4             = 0x34,
    IRKEY_NUM_5             = 0x35,
    IRKEY_NUM_6             = 0x36,
    IRKEY_NUM_7             = 0x37,
    IRKEY_NUM_8             = 0x38,
    IRKEY_NUM_9             = 0x39,

    IRKEY_MUTE              = 0x2d,
    IRKEY_PAGE_UP           = IRKEY_DUMY,
    IRKEY_PAGE_DOWN         = IRKEY_DUMY,
    IRKEY_CLOCK             = IRKEY_DUMY,

    IRKEY_INFO              = IRKEY_DUMY,
    IRKEY_RED               = IRKEY_DUMY,
    IRKEY_GREEN             = IRKEY_DUMY,
    IRKEY_YELLOW            = IRKEY_DUMY,
    IRKEY_BLUE              = IRKEY_DUMY,
    IRKEY_MTS               = IRKEY_DUMY,
    IRKEY_NINE_LATTICE      = IRKEY_DUMY,
    IRKEY_TTX               = IRKEY_DUMY,
    IRKEY_CC                = IRKEY_DUMY,
    IRKEY_INPUT_SOURCE      = 0x2E,
    IRKEY_CRADRD            = IRKEY_DUMY,
//    IRKEY_PICTURE           = 0x40,
    IRKEY_ZOOM              = 0x1A,
    IRKEY_DASH              = IRKEY_DUMY,
    IRKEY_SLEEP             = IRKEY_DUMY,
    IRKEY_EPG               = IRKEY_DUMY,
    IRKEY_PIP               = IRKEY_DUMY,

  	IRKEY_MIX               = IRKEY_DUMY,
    IRKEY_INDEX             = IRKEY_DUMY,
    IRKEY_HOLD              = IRKEY_DUMY,
    IRKEY_PREVIOUS          = 0x03,
    IRKEY_NEXT              = 0x04,
    IRKEY_BACKWARD          = 0x05,
    IRKEY_FORWARD           = 0x06,
    IRKEY_PLAY              = 0x01,
    IRKEY_RECORD            = IRKEY_DUMY,
    IRKEY_STOP              = 0x02,
    IRKEY_PAUSE             = 0x01,

    IRKEY_SIZE              = IRKEY_DUMY,
    IRKEY_REVEAL            = IRKEY_DUMY,
    IRKEY_SUBCODE           = IRKEY_DUMY,
}IrCommandType;
//-------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
// IR system parameter define for H/W setting (Please don't modify them)
//-------------------------------------------------------------------------------------------
#define IR_CKDIV_NUM                        ((XTAL_CLOCK_FREQ+500000)/1000000) //XTAL_CLOCK_FREQ = 12000000hz
#define IR_CKDIV_NUM_BOOT                   XTAL_CLOCK_FREQ
#define IR_CLK_BOOT                         (XTAL_CLOCK_FREQ/1000000)
#define IR_CLK                              IR_CLK_BOOT

#define irGetMinCnt_BOOT(time, tolerance)   ((u32)(((double)time*((double)IR_CLK_BOOT)/(IR_CKDIV_NUM_BOOT+1))*((double)1-tolerance)))
#define irGetMaxCnt_BOOT(time, tolerance)   ((u32)(((double)time*((double)IR_CLK_BOOT)/(IR_CKDIV_NUM_BOOT+1))*((double)1+tolerance)))
#define irGetMinCnt(time, tolerance)        ((u32)(((double)time*((double)IR_CLK)/(IR_CKDIV_NUM+1))*((double)1-tolerance)))
#define irGetMaxCnt(time, tolerance)        ((u32)(((double)time*((double)IR_CLK)/(IR_CKDIV_NUM+1))*((double)1+tolerance)))

#define irGetCnt_BOOT(time)                 ((u32)((double)time*((double)IR_CLK_BOOT)/(IR_CKDIV_NUM_BOOT+1)))
#define irGetCnt(time)                      ((u32)((double)time*((double)IR_CLK)/(IR_CKDIV_NUM+1)))

// 12Mhz
#define IR_RP_TIMEOUT_BOOT      irGetCnt_BOOT(IR_TIMEOUT_CYC)
#define IR_HDC_UPB_BOOT         irGetMaxCnt_BOOT(IR_HEADER_CODE_TIME, 0.2)
#define IR_HDC_LOB_BOOT         irGetMinCnt_BOOT(IR_HEADER_CODE_TIME, 0.2)
#define IR_OFC_UPB_BOOT         irGetMaxCnt_BOOT(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_LOB_BOOT         irGetMinCnt_BOOT(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_RP_UPB_BOOT      irGetMaxCnt_BOOT(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_OFC_RP_LOB_BOOT      irGetMinCnt_BOOT(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_LG01H_UPB_BOOT       irGetMaxCnt_BOOT(IR_LOGI_01H_TIME, 0.35)
#define IR_LG01H_LOB_BOOT       irGetMinCnt_BOOT(IR_LOGI_01H_TIME, 0.3)
#define IR_LG0_UPB_BOOT         irGetMaxCnt_BOOT(IR_LOGI_0_TIME, 0.2)
#define IR_LG0_LOB_BOOT         irGetMinCnt_BOOT(IR_LOGI_0_TIME, 0.2)
#define IR_LG1_UPB_BOOT         irGetMaxCnt_BOOT(IR_LOGI_1_TIME, 0.2)
#define IR_LG1_LOB_BOOT         irGetMinCnt_BOOT(IR_LOGI_1_TIME, 0.2)

// 90Mhz
#define IR_RP_TIMEOUT           irGetCnt(IR_TIMEOUT_CYC)
#define IR_HDC_UPB              irGetMaxCnt(IR_HEADER_CODE_TIME, 0.2)
#define IR_HDC_LOB              irGetMinCnt(IR_HEADER_CODE_TIME, 0.2)
#define IR_OFC_UPB              irGetMaxCnt(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_LOB              irGetMinCnt(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_RP_UPB           irGetMaxCnt(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_OFC_RP_LOB           irGetMinCnt(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_LG01H_UPB            irGetMaxCnt(IR_LOGI_01H_TIME, 0.35)
#define IR_LG01H_LOB            irGetMinCnt(IR_LOGI_01H_TIME, 0.3)
#define IR_LG0_UPB              irGetMaxCnt(IR_LOGI_0_TIME, 0.2)
#define IR_LG0_LOB              irGetMinCnt(IR_LOGI_0_TIME, 0.2)
#define IR_LG1_UPB              irGetMaxCnt(IR_LOGI_1_TIME, 0.2)
#define IR_LG1_LOB              irGetMinCnt(IR_LOGI_1_TIME, 0.2)

//-------------------------------------------------------------------------------------------

//#######################################
//#     (1) ==> Operation on 4/3 MHz
//#######################################
#define IR_RC_CLK_DIV(ClkMhz)       (ClkMhz*3/4-1)
#define IR_RC_LGPS_THR(UnitTime)    ((UnitTime))            //(UnitTime)*(4/3)*(3/4)
#define IR_RC_INTG_THR(UnitTime)    (((UnitTime)*2/3-7)/8) //((UnitTime)*(4/3)*(1/2)-7)/8
#define IR_RC_WDOG_CNT(UnitTime)    ((UnitTime)/768)        //(UnitTime)*(4/3)/1024 
#define IR_RC_TMOUT_CNT(UnitTime)   ((UnitTime)/1536)       //(UnitTime)*(4/3)/2048
#define IR_RC6_LDPS_THR(UnitTime)   ((UnitTime)*8/9-31)     //(UnitTime)*(4/3)*(2/3)-31
#define IR_RC6_LGPS_MAR(UnitTime)   ((UnitTime)*2/3)        //(UnitTime)*(4/3)*(1/2)


#endif

