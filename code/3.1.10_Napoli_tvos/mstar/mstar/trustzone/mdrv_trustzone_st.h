
#ifndef _MDRV_TRUSTZONE_ST_H_
#define _MDRV_TRUSTZONE_ST_H_

//#include "mdrv_smc.h"

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
//

#define DrvTZ_CPU_NUM 4

typedef struct
{
    unsigned int                u32NW_Ver_Main;
    unsigned int                u32NW_Ver_Sub;
        
} DrvTZ_Info_t;

/*
typedef struct
{
    unsigned int  u32Cpu_Regs[TZ_LLV_REGS_MAX];
} DrvTZ_Cpu_Regs_t;
*/
typedef struct
{
    unsigned int 		u32Args[DrvTZ_CPU_NUM];  // Arm GP register r0-r3
} DrvTZ_Args_t;

#endif // _MDRV_TRUSTZONE_ST_H_

