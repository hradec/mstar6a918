
#ifndef _HAL_HW_SEM_H_
#define _HAL_HW_SEM_H_

#include "mhal_hw_sem_reg.h"
//#include <asm/types.h>
//#include "mdrv_types.h"

extern  BOOL HAL_SEM_SetBank(U32 u32BaseAddr);
extern  BOOL HAL_SEM_Get_Resource(U8 u8SemID, U16 u16ResId);
extern  BOOL HAL_SEM_Free_Resource(U8 u8SemID, U16 u16ResId);
extern  BOOL HAL_SEM_Reset_Resource(U8 u8SemID);
extern  BOOL HAL_SEM_Get_ResourceID(U8 u8SemID, U16* pu16ResId);
extern  U32 HAL_SEM_Get_Num(void);

#endif
