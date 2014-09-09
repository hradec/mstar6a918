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
///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   mdrv_trustzone.h
/// @brief  MALLOC Driver Interface
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_TRUSTZONE_H_
#define _DRV_TRUSTZONE_H_

#include "mdrv_trustzone_io.h"
#include "mdrv_trustzone_st.h"
//#include "mdrv_smc.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
typedef enum {
      TZ_LLV_RES_OK                      //= 0
     ,TZ_LLV_RES_ERR                     //= 1
     ,TZ_LLV_RES_UND                     //= 2
     ,TZ_LLV_RES_MAX
}TZ_LLV_Result;

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
struct tz_struct{

    volatile unsigned int cmd1;
    volatile unsigned int cmd2;

    volatile unsigned int* private_data;

};

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
//void smc_call(struct tz_struct *tz);
int tz_call(struct tz_struct *tz);
#endif // _DRV_TRUSTZONE_H_

