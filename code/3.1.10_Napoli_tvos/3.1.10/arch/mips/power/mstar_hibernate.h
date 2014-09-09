#ifndef __MSTAR_HIBERNATE_H__
#define __MSTAR_HIBERNATE_H__
#include <asm/regdef.h>

#define SAVE_MAGIC 0x4D535452 /*MSTR*/

#define SAVEGEN_OFFSET_AT            0
#define SAVEGEN_OFFSET_S0            4
#define SAVEGEN_OFFSET_S1            8
#define SAVEGEN_OFFSET_S2            12
#define SAVEGEN_OFFSET_S3            16
#define SAVEGEN_OFFSET_S4            20
#define SAVEGEN_OFFSET_S5            24
#define SAVEGEN_OFFSET_S6            28
#define SAVEGEN_OFFSET_S7            32
#define SAVEGEN_OFFSET_K0            36
#define SAVEGEN_OFFSET_K1            40
#define SAVEGEN_OFFSET_GP            44
#define SAVEGEN_OFFSET_S8            48
#define SAVEGEN_OFFSET_SP            52
#define SAVEGEN_OFFSET_RA            56

#define SAVEGEN_LEN                  60

#define SAVECP0_OFFSET_IR            0
#define SAVECP0_OFFSET_EntryLo0     4
#define SAVECP0_OFFSET_EntryLo1     8
#define SAVECP0_OFFSET_CTX          12
#define SAVECP0_OFFSET_PageMask    16
#define SAVECP0_OFFSET_PageGrain   20
#define SAVECP0_OFFSET_Wired       24
#define SAVECP0_OFFSET_HWREna      28
#define SAVECP0_OFFSET_Count       32
#define SAVECP0_OFFSET_EntryHi     36
#define SAVECP0_OFFSET_Compare     40
#define SAVECP0_OFFSET_Status      44
#define SAVECP0_OFFSET_IntCtrl     48
#define SAVECP0_OFFSET_SRSCtl      52
#define SAVECP0_OFFSET_SRSMap      56
#define SAVECP0_OFFSET_Cause       60
#define SAVECP0_OFFSET_EPC         64
#define SAVECP0_OFFSET_EBase       68
#define SAVECP0_OFFSET_CFG0        72
#define SAVECP0_OFFSET_CFG1        76
#define SAVECP0_OFFSET_CFG2        80
#define SAVECP0_OFFSET_CFG3        84
#define SAVECP0_OFFSET_EEPC        88
#define SAVECP0_OFFSET_FrmMsk      92
#define SAVECP0_OFFSET_WchL0       96
#define SAVECP0_OFFSET_WchH0       100
#define SAVECP0_OFFSET_WchL1       104
#define SAVECP0_OFFSET_WchH1       108
#define SAVECP0_OFFSET_WchL2       112
#define SAVECP0_OFFSET_WchH2       116
#define SAVECP0_OFFSET_WchL3       120
#define SAVECP0_OFFSET_WchH3       124
#define SAVECP0_OFFSET_WchCnt      128
#define SAVECP0_OFFSET_CFGCnt      132
#define SAVECP0_OFFSET_DbgCnt      136

#define SAVECP0_LEN 140
#endif
