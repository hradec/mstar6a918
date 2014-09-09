



#ifndef _REG_WDT_H
#define _REG_WDT_H

#include "mdrv_types.h"

//------------------------------------------------------------------------------
// PIU_MISC Reg
//------------------------------------------------------------------------------

#define RIU_MAP 0xFD000000

#define WDT_REG_BASE                             (0x3000)

//                                              bank, regiter
// WDT 
#define REG_WDT_CLR             ((RIU_MAP)+ (WDT_REG_BASE + 0x00*2)*2)
#define REG_WDT_RST_FLAG        ((RIU_MAP)+ (WDT_REG_BASE + 0x02*2)*2)
#define REG_WDT_RST_LEN         ((RIU_MAP)+ (WDT_REG_BASE + 0x02*2)*2)
#define REG_WDT_INT             ((RIU_MAP)+ (WDT_REG_BASE + 0x03*2)*2)
#define REG_WDT_MAX             ((RIU_MAP)+ (WDT_REG_BASE + 0x04*2)*2)


#define TEST_REG_BASE                             (0x3d00)

#define REG_TEST             ((RIU_MAP)+ (TEST_REG_BASE + 0xb6)*2)


#endif  // _REG_WDT_H

