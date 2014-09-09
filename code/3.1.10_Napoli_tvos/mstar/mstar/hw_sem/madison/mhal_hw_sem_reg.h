#include "mdrv_types.h"

#ifndef __REG_HW_SEM_H__
#define __REG_HW_SEM_H__

#define REG_SEM_MAX_NUM                         (8)
#define REG_SEM_BASE                            0x0C00

#define REG_SEM_ID0                             (REG_SEM_BASE+ 0x0000) // (SEM_REG_BASE+0x00) // 0xC00+ 0x00
#define REG_SEM_ID1                             (REG_SEM_BASE+ 0x0001) // (SEM_REG_BASE+0x02) // 0xC00+ 0x01
#define REG_SEM_ID2                             (REG_SEM_BASE+ 0x0002) // (SEM_REG_BASE+0x04) // 0xC00+ 0x02
#define REG_SEM_ID3                             (REG_SEM_BASE+ 0x0003) // (SEM_REG_BASE+0x06) // 0xC00+ 0x03
#define REG_SEM_ID4                             (REG_SEM_BASE+ 0x0004) // (SEM_REG_BASE+0x08) // 0xC00+ 0x04
#define REG_SEM_ID5                             (REG_SEM_BASE+ 0x0005) // (SEM_REG_BASE+0x0A) // 0xC00+ 0x05
#define REG_SEM_ID6                             (REG_SEM_BASE+ 0x0006) // (SEM_REG_BASE+0x0C) // 0xC00+ 0x06
#define REG_SEM_ID7                             (REG_SEM_BASE+ 0x0007) // (SEM_REG_BASE+0x0E) // 0xC00+ 0x07

#endif // #ifndef __REG_SEM_H__



