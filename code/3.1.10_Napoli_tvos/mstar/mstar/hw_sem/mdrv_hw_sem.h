#ifndef _DRV_HW_SEM_H_
#define _DRV_HW_SEM_H_



//#include <asm/types.h>
#include "mdrv_types.h"

#define SEMID_MBX_HKCPU2PM_RECV  6
#define SEMID_MBX_HKCPU2PM_SEND  7
    #define RESID_HKCPU 0x1
    #define RESID_PM 0x2

BOOL MDrv_SEM_Init(void);
BOOL MDrv_SEM_Get_Resource(U8 u8SemID, U16 u16ResId);
BOOL MDrv_SEM_Free_Resource(U8 u8SemID, U16 u16ResId);
BOOL MDrv_SEM_Reset_Resource(U8 u8SemID);
BOOL MDrv_SEM_Get_ResourceID(U8 u8SemID, U16* pu16ResId);
U32 MDrv_SEM_Get_Num(void);

#endif
