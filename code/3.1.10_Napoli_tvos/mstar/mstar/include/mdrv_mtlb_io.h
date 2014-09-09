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
/// @file   mdrv_mtlb_io.h
/// @brief  MIU TLB  Driver IO Interface
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_MTLB_IO_H_
#define _MDRV_MTLB_IO_H_

/* Use 'T' as magic number */
#define MTLB_IOC_MAGIC                'T'

#define MTLB_IOC_MAP           _IOR(MTLB_IOC_MAGIC, 0x00, DevMtlb_Map_Info_t)
#define MTLB_IOC_UNMAP         _IOR(MTLB_IOC_MAGIC, 0x01, DevMtlb_Map_Info_t)
#define MTLB_IOC_TRAVERSE      _IO(MTLB_IOC_MAGIC, 0x02)

#endif

