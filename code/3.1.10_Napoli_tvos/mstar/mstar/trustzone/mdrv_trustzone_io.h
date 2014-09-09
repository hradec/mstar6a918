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
/// @file   mdrv_trustzone_io.h
/// @brief  Trustzone Driver IO Interface
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_TRUSTZONE_IO_H_
#define _MDRV_TRUSTZONE_IO_H_



/* Use 'T' as magic number */
#define TZ_IOC_MAGIC                'T'

#define TZ_IOC_INFO                             _IOWR(TZ_IOC_MAGIC, 0x00, DrvTZ_Info_t)
#define TZ_IOC_Get_Reg  			                  _IOWR(TZ_IOC_MAGIC, 0x01, DrvTZ_Cpu_Regs_t)
#define TZ_IOC_Register_Class	                  _IOWR(TZ_IOC_MAGIC, 0x02, unsigned int)
#define TZ_IOC_Call			                      	_IOWR(TZ_IOC_MAGIC, 0x03, DrvTZ_Args_t)

#endif

