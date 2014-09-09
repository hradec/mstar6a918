

#ifndef _HAL_WDT_H_
#define _HAL_WDT_H_

#include "mdrv_types.h"



void HAL_WDT_DisableWDT(void);

BOOL HAL_WDT_IsWDTEnabled(void);

BOOL HAL_WDT_IsResetByWDT(void);

void HAL_WDT_ClearWDT(void);

void HAL_WDT_ClearWDTStatus(void);

void HAL_WDT_SetWDT_MS(S32 msec);

void HAL_WDT_SetWDTInt(S32 msec);



#endif



