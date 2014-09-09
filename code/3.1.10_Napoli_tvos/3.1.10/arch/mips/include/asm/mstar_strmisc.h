#ifndef _MSTAR_STRMISC_H_
#define _MSTAR_STRMISC_H_

#define RIU_MAP 0xBF200000

#define RIU     ((unsigned short volatile *) RIU_MAP)
#define RIU8    ((unsigned char  volatile *) RIU_MAP)

#define REG_XC_BASE                               (0x2F00)
#define XC_REG(addr)                               RIU[(addr<<1)+REG_XC_BASE]

#endif