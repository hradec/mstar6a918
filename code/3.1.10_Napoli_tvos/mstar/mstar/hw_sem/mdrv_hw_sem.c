#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>

#include "mhal_hw_sem.h"
#include "mdrv_hw_sem.h"

BOOL MDrv_SEM_Init(void)
{
    return TRUE; 
}
EXPORT_SYMBOL(MDrv_SEM_Init);

BOOL MDrv_SEM_Get_Resource(U8 u8SemID, U16 u16ResId)
{
    return HAL_SEM_Get_Resource(u8SemID, u16ResId);
}
EXPORT_SYMBOL(MDrv_SEM_Get_Resource);

BOOL MDrv_SEM_Free_Resource(U8 u8SemID, U16 u16ResId)
{
    return HAL_SEM_Free_Resource(u8SemID, u16ResId);
}
EXPORT_SYMBOL(MDrv_SEM_Free_Resource);

BOOL MDrv_SEM_Reset_Resource(U8 u8SemID)
{
    return HAL_SEM_Reset_Resource(u8SemID);
}
EXPORT_SYMBOL(MDrv_SEM_Reset_Resource);

BOOL MDrv_SEM_Get_ResourceID(U8 u8SemID, U16* pu16ResId)
{
    return HAL_SEM_Get_ResourceID(u8SemID, pu16ResId);
}
EXPORT_SYMBOL(MDrv_SEM_Get_ResourceID);

U32 MDrv_SEM_Get_Num(void)
{
    return HAL_SEM_Get_Num();
}
EXPORT_SYMBOL(MDrv_SEM_Get_Num);

//EXPORT_SYMBOL(MDrv_SEM_Get_Resource);

