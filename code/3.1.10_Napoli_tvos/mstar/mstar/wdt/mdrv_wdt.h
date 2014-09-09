
#ifndef _DRV_WDT_H
#define _DRV_WDT_H


#include "mdrv_types.h"

void MDrv_WDT_DisableWDT(void);
void MDrv_WDT_SetWDT_MS(S32 s32msec);
void MDrv_WDT_SetWDTInt(U16 u16Sec);
void MDrv_WDT_ClearWDT(void);
#endif
