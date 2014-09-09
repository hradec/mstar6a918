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
/// @file   mdrv_MTLB.h
/// @brief  MALLOC Driver Interface
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_MTLB_H_
#define _DRV_MTLB_H_

#include "mdrv_mtlb_io.h"
#include "mdrv_mtlb_st.h"
//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_MTLB_MIU_0 = 0,
    E_MTLB_MIU_1,
    E_MTLB_MIU_NUM,
}MIU_ID;

#endif // _DRV_MTLB_H_

