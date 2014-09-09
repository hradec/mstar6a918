#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/bug.h>


#include "mhal_hw_sem_reg.h"
#include "mhal_hw_sem.h"

static U32    _u32RegBase = 0xBF200000;

#define REG16(addr)     *((volatile U16*)((_u32RegBase+ ((addr)<< 2))))

U32 HAL_SEM_Get_Num(void)
{
    return REG_SEM_MAX_NUM;
}

BOOL HAL_SEM_Get_Resource(U8 u8SemID, U16 u16ResId)
{
    if(u8SemID> REG_SEM_MAX_NUM)
        return FALSE;
    REG16(REG_SEM_ID0+ u8SemID)= u16ResId;
    return (u16ResId == REG16(REG_SEM_ID0+ u8SemID))? TRUE: FALSE;
}

BOOL HAL_SEM_Free_Resource(U8 u8SemID, U16 u16ResId)
{
	U16 u16CurResId = 0;
    if(u8SemID> REG_SEM_MAX_NUM)
        return FALSE;
	u16CurResId = REG16(REG_SEM_ID0+ u8SemID);
    if (u16ResId != u16CurResId)
    {
    	printk(KERN_EMERG "==Error== %s , current resource id=%d, request resource id=%d\n",
				__FUNCTION__, u16CurResId, u16ResId);
        return FALSE;
    }
    REG16(REG_SEM_ID0+ u8SemID)= 0x0000;
	return TRUE;
    //return (0x0000 == REG16(REG_SEM_ID0+ u8SemID))? TRUE: FALSE;
}

BOOL HAL_SEM_Reset_Resource(U8 u8SemID)
{
    if(u8SemID> REG_SEM_MAX_NUM)
        return FALSE;
    REG16(REG_SEM_ID0+ u8SemID)= 0x00;
    return (0x00 == REG16(REG_SEM_ID0+ u8SemID))? TRUE: FALSE;
}

BOOL HAL_SEM_Get_ResourceID(U8 u8SemID, U16* pu16ResId)
{
    if(u8SemID> REG_SEM_MAX_NUM)
        return FALSE;
    *pu16ResId = REG16(REG_SEM_ID0+ u8SemID);
    return TRUE;
}


