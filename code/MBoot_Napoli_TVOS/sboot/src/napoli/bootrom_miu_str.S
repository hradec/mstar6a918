/*
 *#############################################################################
 *
 * Copyright (c) 2006-2013 MStar Semiconductor, Inc.
 * All rights reserved.
 *
 * Unless otherwise stipulated in writing, any and all information contained
 * herein regardless in any format shall remain the sole proprietary of
 * MStar Semiconductor Inc. and be kept in strict confidence
 * (¡§MStar Confidential Information¡¨) by the recipient.
 * Any unauthorized act including without limitation unauthorized disclosure,
 * copying, use, reproduction, sale, distribution, modification, disassembling,
 * reverse engineering and compiling of the contents of MStar Confidential
 * Information is unlawful and strictly prohibited. MStar hereby reserves the
 * rights to any and all damages, losses, costs and expenses resulting therefrom.
 *
 *#############################################################################
 */

#ifndef _BOOT_INC_
#include "boot.inc"
#endif

#ifndef _BOARD_H_
#include "Board.h"
#endif

#ifndef _DRV_MIU_INIT_STR_H_
#include "drv_miu_init_str.h"
#endif

#ifndef _MIUSEL_NAPOLI_2MIU_H_
#include "MIUSEL_NAPOLI_2MIU.h"
#endif




    .text
    .global     BootRom_InitMiu_STR
BootRom_InitMiu_STR:
    mov         r13, lr
    PUTK        'S'
    PUTK        'T'
    PUTK        'R'
    PUTK        '_'
    PUTK        'D'
    PUTK        'C' 
    @//PUTK        '_'
    @//PUTK        'O'
    @//PUTK        'N'
    PUTK        '\r'
    PUTK        '\n'
    @// ====== set OENZ = 1 ======
    bl          Miu_str_oenz_enable
    
#if (ENABLE_MSTAR_NAPOLI_BD_MST051B_10ANV)  || (ENABLE_MSTAR_NAPOLI_BD_MST051D_20ANV)  ||       \
    (ENABLE_MSTAR_NAPOLI_BD_MST181B_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST181D_D01A_S) ||       \
    (ENABLE_MSTAR_NAPOLI_BD_MST189C_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST190A_D01A_S) ||       \
    (ENABLE_MSTAR_NAPOLI_BD_MST190A_D01A)   || (ENABLE_MSTAR_NAPOLI_BD_MST181B_D01A_S_CASHMERE) 
    WREG        (MS_BUS_RIU_ADDR + (0x110d00 << 1)), 0x6019
    WREG        (MS_BUS_RIU_ADDR + (0x161600 << 1)), 0x6019
    DELAYUS     1000
    WREG        (MS_BUS_RIU_ADDR + (0x110d00 << 1)), 0x0009
    WREG        (MS_BUS_RIU_ADDR + (0x161600 << 1)), 0x0009
#else
    WREG        (MS_BUS_RIU_ADDR + (0x110d00 << 1)), 0x6018
    WREG        (MS_BUS_RIU_ADDR + (0x161600 << 1)), 0x6018
    WREG        (MS_BUS_RIU_ADDR + (0x110d00 << 1)), 0x0008
    WREG        (MS_BUS_RIU_ADDR + (0x161600 << 1)), 0x0008
#endif
    DELAYUS     1000                                          @// delay 200us
    WREG        (MS_BUS_RIU_ADDR + (0x110d7E << 1)), 0x0000   @// LD Power
    WREG        (MS_BUS_RIU_ADDR + (0x16167E << 1)), 0x0000

    @// ====== Re MIU Init ======
    ldr         r0, =MIU_PreInit_STR
    bl          MDrv_WriteRegTbl32
    
    @// ====== AVD reset ======
    bl          Reset_AVD_str
    
    @// ========== MIU0 DDR INIT ==========//@
#if !defined(CONFIG_MIU0_DRAM_NONE)
    ldr         r0, =MIU0_DDR_Init_STR
    bl          MDrv_WriteRegTbl32

    DELAYUS     1000
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00C0
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x33C8

    DELAYUS     5000

    @//Enable MIU0
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x1401
    DELAYUS     1
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x1408

#if (ENABLE_MSTAR_NAPOLI_BD_MST181B_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST181D_D01A_S) ||    \
    (ENABLE_MSTAR_NAPOLI_BD_MST051B_10ANV)  || (ENABLE_MSTAR_NAPOLI_BD_MST051D_20ANV)  ||    \
    (ENABLE_MSTAR_NAPOLI_BD_MST190A_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST189C_D01A_S) ||    \
    (ENABLE_MSTAR_NAPOLI_BD_MST190A_D01A)   || (ENABLE_MSTAR_NAPOLI_BD_MST055D_10AWD)  ||    \
    (ENABLE_MSTAR_NAPOLI_BD_MST181B_D01A_S_CASHMERE) 
    WREG        (MS_BUS_RIU_ADDR + (0x110d00 << 1)), 0x0009  @// reg_pkg_sel		1	: [0]
#else
    WREG        (MS_BUS_RIU_ADDR + (0x110d00 << 1)), 0x0008  @// reg_pkg_sel		1	: [0]
#endif
    WREG        (MS_BUS_RIU_ADDR + (0x110d02 << 1)), 0xAAAA  @// reg_cko_state	cc	: [7:0]
    WREG        (MS_BUS_RIU_ADDR + (0x110d18 << 1)), 0x0000

#if (ENABLE_MIU0_4X_MODE)
    WREG        (MS_BUS_RIU_ADDR + (0x101202 << 1)), 0xF2A7
#else
    WREG        (MS_BUS_RIU_ADDR + (0x101202 << 1)), 0xF3A7
#endif

    DELAYUS     200  
#endif // endif CONFIG_MIU0_DRAM_NONE

    @// ========== MIU1 DDR INIT ==========//@
#if !defined(CONFIG_MIU1_DRAM_NONE)   
    ldr         r0, =MIU1_DDR_Init_STR
    bl          MDrv_WriteRegTbl32
    
    DELAYUS     1000
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00C0
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x33C8

    DELAYUS     5000
    @//Enable MIU1
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x1401
    DELAYUS     1
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x1408

#if (ENABLE_MSTAR_NAPOLI_BD_MST181B_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST181D_D01A_S) ||    \
    (ENABLE_MSTAR_NAPOLI_BD_MST051B_10ANV)  || (ENABLE_MSTAR_NAPOLI_BD_MST051D_20ANV)  ||    \
    (ENABLE_MSTAR_NAPOLI_BD_MST190A_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST189C_D01A_S) ||    \
    (ENABLE_MSTAR_NAPOLI_BD_MST190A_D01A)   || (ENABLE_MSTAR_NAPOLI_BD_MST055D_10AWD)  ||    \
    (ENABLE_MSTAR_NAPOLI_BD_MST181B_D01A_S_CASHMERE)
    WREG        (MS_BUS_RIU_ADDR + (0x161600 << 1)), 0x0009  @//reg_pkg_sel		1	: [0] 
#else
    WREG        (MS_BUS_RIU_ADDR + (0x161600 << 1)), 0x0008  @//reg_pkg_sel		1	: [0]
#endif

    WREG        (MS_BUS_RIU_ADDR + (0x161602 << 1)), 0xAAAA  @//reg_cko_state		cc	: [7:0]
    WREG        (MS_BUS_RIU_ADDR + (0x161618 << 1)), 0x0000

#if (ENABLE_MSTAR_NAPOLI_BD_MST189A_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST189C_D01A_S)
#if (ENABLE_MIU1_4X_MODE)
    WREG        (MS_BUS_RIU_ADDR + (0x100602 << 1)), 0xF2A3
#else 
    WREG        (MS_BUS_RIU_ADDR + (0x100602 << 1)), 0xF3A3
#endif

#elif (ENABLE_MSTAR_NAPOLI_BD_MST181C_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST189B_D01A_S) ||    \
      (ENABLE_MSTAR_NAPOLI_BD_MST189D_D01A_S)
    WREG        (MS_BUS_RIU_ADDR + (0x100602 << 1)), 0xF3A7
    
#elif (ENABLE_MSTAR_NAPOLI_BD_MST181B_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST181D_D01A_S) ||    \
      (ENABLE_MSTAR_NAPOLI_BD_MST051B_10ANV)  || (ENABLE_MSTAR_NAPOLI_BD_MST051D_20ANV)  ||    \
      (ENABLE_MSTAR_NAPOLI_BD_MST190A_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST190A_D01A)   ||    \
      (ENABLE_MSTAR_NAPOLI_BD_MST055D_10AWD)  || (ENABLE_MSTAR_NAPOLI_BD_MST181B_D01A_S_CASHMERE) 
#if (ENABLE_MIU1_4X_MODE)
    WREG        (MS_BUS_RIU_ADDR + (0x100602 << 1)), 0xF2A7
#else
    WREG        (MS_BUS_RIU_ADDR + (0x100602 << 1)), 0xF3A7
#endif
#endif 
    DELAYUS     2000                @//delay 200us
#endif // endif CONFIG_MIU1_DRAM_NONE
                                             
     
   @// ========== MIU0 INIT Cycle ==========//@
#if !defined(CONFIG_MIU0_DRAM_NONE)
    WREG        (MS_BUS_RIU_ADDR + (0x110d08 << 1)), 0x003f @//rx enable
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x0000 @//hold miu_0
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x0008 @//DRAM reset
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x000c @//DRAM reset+CS
    DELAYUS     1000                                        @//delay 1000us
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x000e @//DRAM reset+CS+CKE
    DELAYUS     2000                                        @//delay 2000us
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x001f @//DRAM reset+CS+CKE+auto cycle+ODE
    DELAYUS     200                                         @//delay 200us
    @// open mask
    @//WREG        (MS_BUS_RIU_ADDR + (0x101246 << 1)), 0x0000
    @//WREG        (MS_BUS_RIU_ADDR + (0x101266 << 1)), 0x0000
    @//WREG        (MS_BUS_RIU_ADDR + (0x101286 << 1)), 0x0000
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012A6 << 1)), 0x0000
    
#if !(ENABLE_MSTAR_NAPOLI_BD_MST051D_20ANV || ENABLE_MSTAR_NAPOLI_BD_MST181B_D01A_S)
#if (ENABLE_MSTAR_NAPOLI_BD_MST189A_D01A_S)
    WREG        (MS_BUS_RIU_ADDR + (0x110d0e << 1)), 0x00ED
#elif (ENABLE_MSTAR_NAPOLI_BD_MST181B_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST181D_D01A_S) ||    \
      (ENABLE_MSTAR_NAPOLI_BD_MST051B_10ANV)  || (ENABLE_MSTAR_NAPOLI_BD_MST190A_D01A_S) ||    \
      (ENABLE_MSTAR_NAPOLI_BD_MST189C_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST190A_D01A)   ||    \
      (ENABLE_MSTAR_NAPOLI_BD_MST055D_10AWD)  || (ENABLE_MSTAR_NAPOLI_BD_MST181B_D01A_S_CASHMERE)
    WREG        (MS_BUS_RIU_ADDR + (0x110d0e << 1)), 0x00EF
#endif
    WREG        (MS_BUS_RIU_ADDR + (0x110d1e << 1)), 0x0007

    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E4 << 1)), 0x0001
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E0 << 1)), 0x0000
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E0 << 1)), 0x0200
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E0 << 1)), 0x0201
    
#if (ENABLE_MSTAR_NAPOLI_BD_MST181C_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST189B_D01A_S) ||    \
    (ENABLE_MSTAR_NAPOLI_BD_MST189A_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST189D_D01A_S)
#if (ENABLE_MIU0_4X_MODE)
    WREG        (MS_BUS_RIU_ADDR + (0x110d0e << 1)), 0x00BD
#else
    WREG        (MS_BUS_RIU_ADDR + (0x110d0e << 1)), 0x00AF
#endif

#else
    WREG        (MS_BUS_RIU_ADDR + (0x110d0e << 1)), 0x00AD
    
#endif

    WREG        (MS_BUS_RIU_ADDR + (0x110d1e << 1)), 0x0003

    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E0 << 1)), 0x0000
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E0 << 1)), 0x0200
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E0 << 1)), 0x0201
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E0 << 1)), 0x0000
#endif // endif ENABLE_MSTAR_NAPOLI_BD_MST051D_20ANV 
#endif // endif CONFIG_MIU0_DRAM_NONE

    @// ========== MIU1 INIT Cycle ==========//@
#ifndef CONFIG_MIU1_DRAM_NONE   
    WREG        (MS_BUS_RIU_ADDR + (0x161608 << 1)), 0x003f @//rx enable
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x0000 @//hold miu_0
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x0008 @//DRAM reset
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x000c @//DRAM reset+CS
    DELAYUS     1000                                        @//delay 1000us
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x000e @//DRAM reset+CS+CKE
    DELAYUS     2000                                        @//delay 2000us
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x001f @//DRAM reset+CS+CKE+auto cycle+ODE
    DELAYUS     200                                         @//delay 200us
    @// open mask
    @//WREG        (MS_BUS_RIU_ADDR + (0x100646 << 1)), 0x0000
    @//WREG        (MS_BUS_RIU_ADDR + (0x100666 << 1)), 0x0000
    @//WREG        (MS_BUS_RIU_ADDR + (0x100686 << 1)), 0x0000
    @//WREG        (MS_BUS_RIU_ADDR + (0x1006A6 << 1)), 0x0000
    
#if !(ENABLE_MSTAR_NAPOLI_BD_MST051D_20ANV || ENABLE_MSTAR_NAPOLI_BD_MST181B_D01A_S)    
#if (ENABLE_MSTAR_NAPOLI_BD_MST189A_D01A_S)
    WREG        (MS_BUS_RIU_ADDR + (0x16160e << 1)), 0x00EF
#elif (ENABLE_MSTAR_NAPOLI_BD_MST051D_20ANV)
#if (ENABLE_MIU0_4X_MODE)
    WREG        (MS_BUS_RIU_ADDR + (0x16160e << 1)), 0x00FD
#else
    WREG        (MS_BUS_RIU_ADDR + (0x16160e << 1)), 0x00ED
#endif
#else
    WREG        (MS_BUS_RIU_ADDR + (0x16160e << 1)), 0x00ED
    
#endif
    WREG        (MS_BUS_RIU_ADDR + (0x16161e << 1)), 0x0007
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012F0 << 1)), 0x8000
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E4 << 1)), 0x0001
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E0 << 1)), 0x0000
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E0 << 1)), 0x0200
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E0 << 1)), 0x0201
#if (ENABLE_MSTAR_NAPOLI_BD_MST181C_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST189B_D01A_S) ||    \
    (ENABLE_MSTAR_NAPOLI_BD_MST189A_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST051D_20ANV)  ||    \
    (ENABLE_MSTAR_NAPOLI_BD_MST189D_D01A_S)
#if (ENABLE_MIU1_4X_MODE)
    WREG        (MS_BUS_RIU_ADDR + (0x16160e << 1)), 0x00BD
#else
    WREG        (MS_BUS_RIU_ADDR + (0x16160e << 1)), 0x00AF
#endif

#else
    WREG        (MS_BUS_RIU_ADDR + (0x16160e << 1)), 0x00AD
    
#endif
    WREG        (MS_BUS_RIU_ADDR + (0x16161e << 1)), 0x0003
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E0 << 1)), 0x0000
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E0 << 1)), 0x0200
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E0 << 1)), 0x0201
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012E0 << 1)), 0x0000
    @//WREG        (MS_BUS_RIU_ADDR + (0x1012F0 << 1)), 0x0000
#endif // endif ENABLE_MSTAR_NAPOLI_BD_MST051D_20ANV
#endif // endif CONFIG_MIU1_DRAM_NONE

    bl  Miu_auto_phase_str
    
    @// ========== Enter Self Refresh mode ==========//@
    bl  Enter_DDR_RefreshMode
    
    @// ========== OENZ = 0 ==========//@
    bl  Miu_str_oenz_disable
    
    @// ========== ATOP_PD_OFF ==========//@
#ifndef CONFIG_MIU0_DRAM_NONE
    WREG        (MS_BUS_RIU_ADDR + (0x110D54 <<1 )), 0xC000 @//ATOP_PD off
#endif  
#ifndef CONFIG_MIU1_DRAM_NONE
    WREG        (MS_BUS_RIU_ADDR + (0x161654 <<1 )), 0xC000 @//ATOP_PD off
#endif

    @// ========== Rx enable ==========//@
#ifndef CONFIG_MIU0_DRAM_NONE
    WREG        (MS_BUS_RIU_ADDR + (0x110d08 << 1)), 0x003f 
#endif
#ifndef CONFIG_MIU1_DRAM_NONE
    WREG        (MS_BUS_RIU_ADDR + (0x161608 << 1)), 0x003f
#endif    

    @// ========== exit self refresh mode ==========//@     
    bl  Exit_DDR_RefreshMode
    DELAYUS     300 

@//    bl  Push_Dram_STR

@//-----------------------------------STR--------------------------------------
   @// DBGREG      0x0027
#if !defined(CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH) && \
    !defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
    bl          MIU_Set_Group_Priority
#endif

    ldr         r0, =MIU_PostInit_STR
    bl          MDrv_WriteRegTbl32

    @//DBGREG      0x0030
	
    @//Enable MIU to keep servicing timeouted client when there is no other request to MIU 
    RREG        r3, (MS_BUS_RIU_ADDR + (0x10121E << 1))
    ldr         r4, =0x8000
    orr         r3, r3, r4
    REG_C2M     r3, (MS_BUS_RIU_ADDR + (0x10121E << 1))
    
    RREG        r3, (MS_BUS_RIU_ADDR + (0x10061E << 1))
    ldr         r4, =0x8000
    orr         r3, r3, r4
    REG_C2M     r3, (MS_BUS_RIU_ADDR + (0x10061E << 1))

    #if 0
    @// for U02 ECO delay line K/2 + offset
    @//[3:0] reg_dline_sum0_0 = DLINE_SUM0_A0 (For DQ 7-0)
    @//[3:0] reg_dline_sum1_0 = DLINE_SUM1_A0 (For DQ 15-8)
    @//[3:0] reg_dline_sum0_1 = DLINE_SUM0_A1 (For DQ 23-16)
    @//[3:0] reg_dline_sum1_1 = DLINE_SUM1_A1 (For DQ 31-24)
    WREG        (MS_BUS_RIU_ADDR + (0x110d70 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110D92 << 1)), 0x4506
    WREG        (MS_BUS_RIU_ADDR + (0x110D90 << 1)), 0x0003
    WREG        (MS_BUS_RIU_ADDR + (0x110D70 << 1)), 0x0800

    WREG        (MS_BUS_RIU_ADDR + (0x161670 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161692 << 1)), 0x1647
    WREG        (MS_BUS_RIU_ADDR + (0x161690 << 1)), 0x0003
    WREG        (MS_BUS_RIU_ADDR + (0x161670 << 1)), 0x0800
    #endif

    @//DLL calibration
    WREG        (MS_BUS_RIU_ADDR + (0x110d62 << 1)), 0x007f // pulse width
    WREG        (MS_BUS_RIU_ADDR + (0x110d64 << 1)), 0xf000 // phase, code
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00c7 
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00c3 
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00cb 
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00c3 
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00cb 
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00c2 

    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00c0 
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x33c8 

    WREG        (MS_BUS_RIU_ADDR + (0x161662 << 1)), 0x007f // pulse width
    WREG        (MS_BUS_RIU_ADDR + (0x161664 << 1)), 0xf000 // phase, code
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00c7 
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00c3 
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00cb 
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00c3 
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00cb 
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00c2 

    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00c0 
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x33c8 


    #if defined(CONFIG_EFUSE_MIU_DESKEW)
#if !defined(CONFIG_MIU0_DRAM_NONE)
    mov         r0, #0
    bl          Miu_DeSkew
#endif
#if !defined(CONFIG_MIU1_DRAM_NONE)
    mov         r0, #1
    bl          Miu_DeSkew
#endif
#endif



    mov         lr, r13
    bx          lr
    mov         lr, r13
    bx          lr
    .size       BootRom_InitMiu_STR,.-BootRom_InitMiu_STR

@//-------------------------------------------------------------------------------------------------
@// Miu_str_oenz_enable
@// @param  None
@// @return None
@// @note   Miu_str_oenz_enable
@//-------------------------------------------------------------------------------------------------
Miu_str_oenz_enable:
    RREG        r8, (MS_BUS_RIU_ADDR + (0x101202 << 1))
    orr         r8, r8, #0xf000
    REG_C2M     r8, (MS_BUS_RIU_ADDR + (0x101202 << 1))
#ifndef CONFIG_MIU1_DRAM_NONE
    RREG        r8, (MS_BUS_RIU_ADDR + (0x100602 << 1))
    orr         r8, r8, #0xf000
    REG_C2M     r8, (MS_BUS_RIU_ADDR + (0x100602 << 1))
#endif
    bx          lr
@//-------------------------------------------------------------------------------------------------
@// Miu_str_oenz_disable
@// @param  None
@// @return None
@// @note   Miu_str_oenz_disable
@//-------------------------------------------------------------------------------------------------
Miu_str_oenz_disable:

    RREG        r8, (MS_BUS_RIU_ADDR + (0x101202 << 1))
    bic         r8, r8, #0xf000
    REG_C2M     r8, (MS_BUS_RIU_ADDR + (0x101202 << 1))
#ifndef CONFIG_MIU1_DRAM_NONE
    RREG        r8, (MS_BUS_RIU_ADDR + (0x100602 << 1))
    bic         r8, r8, #0xf000
    REG_C2M     r8, (MS_BUS_RIU_ADDR + (0x100602 << 1))
#endif
    bx          lr
@//-------------------------------------------------------------------------------------------------
@// Miu_auto_phase_str
@// @param  None
@// @return None
@// @note   Miu_auto_phase_str
@//-------------------------------------------------------------------------------------------------
Miu_auto_phase_str:

    @ // Re write MIU auto phase
    RREG        r8, (MS_BUS_RIU_ADDR + (0x3DB2 << 1))
    REG_C2M     r8, (MS_BUS_RIU_ADDR + (0x110D6C << 1))
    RREG        r8, (MS_BUS_RIU_ADDR + (0x3DB6 << 1))
    REG_C2M     r8, (MS_BUS_RIU_ADDR + (0x110D6E << 1))
#ifndef CONFIG_MIU1_DRAM_NONE
    RREG        r8, (MS_BUS_RIU_ADDR + (0x3DB8 << 1))
    REG_C2M     r8, (MS_BUS_RIU_ADDR + (0x16166C << 1))
    RREG        r8, (MS_BUS_RIU_ADDR + (0x3DBA << 1))
    REG_C2M     r8, (MS_BUS_RIU_ADDR + (0x16166E << 1))
#endif
    bx          lr

@//-------------------------------------------------------------------------------------------------
@// Enter_DDR_RefreshMode
@// @param  None
@// @return None
@// @note   Enter_DDR_RefreshMode
@//-------------------------------------------------------------------------------------------------

Enter_DDR_RefreshMode:
    @// [DDR3][enter] <MIU0>
    WREG        (MS_BUS_RIU_ADDR + (0x1012e0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x101246 << 1)), 0x7FFE
    WREG        (MS_BUS_RIU_ADDR + (0x101266 << 1)), 0xFFFF
    WREG        (MS_BUS_RIU_ADDR + (0x101286 << 1)), 0xFFFF
    WREG        (MS_BUS_RIU_ADDR + (0x1012a6 << 1)), 0xFFFF

    WREG        (MS_BUS_RIU_ADDR + (0x101218 << 1)), 0x0400 @//reg_mrx
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x002F @//Bit[05]reg_auto_ref_off
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x042F @//trig precharge all
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x052F
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x002F
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x022F @//trig refresh
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x032F
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x002F
#if (ENABLE_MSTAR_NAPOLI_BD_MST051D_20ANV)
#ifdef CONFIG_MIU0_DDR3_2133
    WREG        (MS_BUS_RIU_ADDR + (0x101206 << 1)), 0x1E2B @//Bit[12]reg_cke_always_on
#elifdef CONFIG_MIU0_DDR3_1866
    WREG        (MS_BUS_RIU_ADDR + (0x101206 << 1)), 0x1D2B @//trig refresh
#endif

#elif (ENABLE_MSTAR_NAPOLI_BD_MST181C_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST189A_D01A_S)
    WREG        (MS_BUS_RIU_ADDR + (0x101206 << 1)), 0x1E2B @//Bit[12]reg_cke_always_on
#else
    WREG        (MS_BUS_RIU_ADDR + (0x101206 << 1)), 0x1D2B @//trig refresh
#endif

    WREG        (MS_BUS_RIU_ADDR + (0x101246 << 1)), 0xFFFF
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x202F

#ifndef CONFIG_MIU1_DRAM_NONE
   @// [DDR3][enter] <MIU1>
    WREG        (MS_BUS_RIU_ADDR + (0x1006e0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x100646 << 1)), 0x7FFE
    WREG        (MS_BUS_RIU_ADDR + (0x100666 << 1)), 0xFFFF
    WREG        (MS_BUS_RIU_ADDR + (0x100686 << 1)), 0xFFFF
    WREG        (MS_BUS_RIU_ADDR + (0x1006a6 << 1)), 0xFFFF
    
    WREG        (MS_BUS_RIU_ADDR + (0x100618 << 1)), 0x0400 @//reg_mrx
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x002F @//Bit[05]reg_auto_ref_off
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x042F @//trig precharge all
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x052F
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x002F
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x022F @//trig refresh
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x032F
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x002F
#if ENABLE_MSTAR_NAPOLI_BD_MST051D_20ANV
#ifdef CONFIG_MIU0_DDR3_2133
    WREG        (MS_BUS_RIU_ADDR + (0x100606 << 1)), 0x1E2B @//Bit[12]reg_cke_always_on
#elifdef CONFIG_MIU0_DDR3_1866
    WREG        (MS_BUS_RIU_ADDR + (0x100606 << 1)), 0x1D2B @//trig refresh
#endif

#elif (ENABLE_MSTAR_NAPOLI_BD_MST181C_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST189A_D01A_S)
    WREG        (MS_BUS_RIU_ADDR + (0x100606 << 1)), 0x1E2B @//Bit[12]reg_cke_always_on
#else
    WREG        (MS_BUS_RIU_ADDR + (0x100606 << 1)), 0x1D2B @//trig refresh
#endif

    WREG        (MS_BUS_RIU_ADDR + (0x100646 << 1)), 0xFFFF
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x202F
#endif
    bx          lr

@//-------------------------------------------------------------------------------------------------
@// Exit_DDR_RefreshMode
@// @param  None
@// @return None
@// @note   Exit_DDR_RefreshMode
@//-------------------------------------------------------------------------------------------------

Exit_DDR_RefreshMode:

    @// [DDR3][exit] <MIU0>
#ifndef CONFIG_MIU0_DRAM_NONE
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x002F
#if (ENABLE_MSTAR_NAPOLI_BD_MST051D_20ANV)
#ifdef CONFIG_MIU0_DDR3_2133
    WREG        (MS_BUS_RIU_ADDR + (0x101206 << 1)), 0x162B @//trig refresh   // for 181C
#elifdef CONFIG_MIU0_DDR3_1866
    WREG        (MS_BUS_RIU_ADDR + (0x101206 << 1)), 0x152B @//trig refresh
#endif

#elif (ENABLE_MSTAR_NAPOLI_BD_MST181C_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST189A_D01A_S)
    WREG        (MS_BUS_RIU_ADDR + (0x101206 << 1)), 0x162B @//trig refresh   // for 181C
    
#else
    WREG        (MS_BUS_RIU_ADDR + (0x101206 << 1)), 0x152B @//trig refresh
#endif
    
    DELAYUS     10
    
    WREG        (MS_BUS_RIU_ADDR + (0x101246 << 1)), 0xFFFE
    
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x022F
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x032F
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x002F
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x000F
    
    DELAYUS     5
    
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x001F

    WREG        (MS_BUS_RIU_ADDR + (0x101246 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x101266 << 1)), 0x0000 @//Bit[12]reg_cke_always_on
    WREG        (MS_BUS_RIU_ADDR + (0x101286 << 1)), 0x0000 @//reg_mrx
    WREG        (MS_BUS_RIU_ADDR + (0x1012A6 << 1)), 0x0000 @//Bit[05]reg_auto_ref_off
#endif // endif CONFIG_MIU0_DRAM_NONE

    @// [DDR3][enter] <MIU1>
#ifndef CONFIG_MIU1_DRAM_NONE
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x002F
    
#if (ENABLE_MSTAR_NAPOLI_BD_MST051D_20ANV)
#ifdef CONFIG_MIU0_DDR3_2133
    WREG        (MS_BUS_RIU_ADDR + (0x100606 << 1)), 0x162B @//trig refresh   // for 181C
#elifdef CONFIG_MIU0_DDR3_1866
    WREG        (MS_BUS_RIU_ADDR + (0x100606 << 1)), 0x152B @//trig refresh
#endif    

#elif (ENABLE_MSTAR_NAPOLI_BD_MST181C_D01A_S) || (ENABLE_MSTAR_NAPOLI_BD_MST189A_D01A_S)
    WREG        (MS_BUS_RIU_ADDR + (0x100606 << 1)), 0x162B @//trig refresh  // for 181C
    
#else
    WREG        (MS_BUS_RIU_ADDR + (0x100606 << 1)), 0x152B @//trig refresh
#endif

    WREG        (MS_BUS_RIU_ADDR + (0x100646 << 1)), 0xFFFE
    
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x022F
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x032F
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x002F
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x000F
    
    DELAYUS     5
    
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x001F
    
    WREG        (MS_BUS_RIU_ADDR + (0x100646 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x100666 << 1)), 0x0000 @//Bit[12]reg_cke_always_on
    WREG        (MS_BUS_RIU_ADDR + (0x100686 << 1)), 0x0000 @//reg_mrx
    WREG        (MS_BUS_RIU_ADDR + (0x1006A6 << 1)), 0x0000 @//Bit[05]reg_auto_ref_off
#endif // endif CONFIG_MIU1_DRAM_NONE
    bx          lr


@//-------------------------------------------------------------------------------------------------
@// Push_Dram_STR
@// @param  None
@// @return None
@// @note   Push_Dram_STR
@//-------------------------------------------------------------------------------------------------
#if 0
Push_Dram_STR:

#if defined(CONFIG_MSTAR_BD_MST038B_10AHT_EAGLE)
    WREG        (MS_BUS_RIU_ADDR + (0x110d0e <<1 )), 0x00e7 @// reg_en_mask @//Table: MIU0_DDR_Init_STR
    WREG        (MS_BUS_RIU_ADDR + (0x110d0e <<1 )), 0x00a7 @// reg_en_mask
    WREG        (MS_BUS_RIU_ADDR + (0x110d0e <<1 )), 0x00e7 @// reg_en_mask
    WREG        (MS_BUS_RIU_ADDR + (0x110d0e <<1 )), 0x00a7
#elif defined(CONFIG_MSTAR_BD_MST028B_10AFX_EAGLE)
    WREG        (MS_BUS_RIU_ADDR + (0x110d0e <<1 )), 0x00e5 @// reg_en_mask
    WREG        (MS_BUS_RIU_ADDR + (0x110d0e <<1 )), 0x00a5 @// reg_en_mask
    WREG        (MS_BUS_RIU_ADDR + (0x110d0e <<1 )), 0x00e5 @// reg_en_mask
    WREG        (MS_BUS_RIU_ADDR + (0x110d0e <<1 )), 0x00a5
#endif

#ifndef CONFIG_MIU1_DRAM_NONE
    WREG        (MS_BUS_RIU_ADDR + (0x110d8e <<1 )), 0x00e5 @// reg_en_mask @//Table: MIU1_DDR_Init_STR
    WREG        (MS_BUS_RIU_ADDR + (0x110d8e <<1 )), 0x00a5 @// reg_en_mask
    WREG        (MS_BUS_RIU_ADDR + (0x110d8e <<1 )), 0x00e5 @// reg_en_mask
    WREG        (MS_BUS_RIU_ADDR + (0x110d8e <<1 )), 0x00a5
#endif


    WREG        (MS_BUS_RIU_ADDR + (0x1012e4 <<1 )), 0x0004 @// reg_en_mask
    WREG        (MS_BUS_RIU_ADDR + (0x1012e0 <<1 )), 0x0100 @// reg_en_mask
    WREG        (MS_BUS_RIU_ADDR + (0x1012e0 <<1 )), 0x0101

    DELAYUS     1                                         @//delay 200us
    WREG        (MS_BUS_RIU_ADDR + (0x1012e0 <<1 )), 0x0100 @// reg_en_mask
    WREG        (MS_BUS_RIU_ADDR + (0x1012e0 <<1 )), 0x0101
    DELAYUS     1                                         @//delay 200us
    WREG        (MS_BUS_RIU_ADDR + (0x1012e0 <<1 )), 0x0000

#ifndef CONFIG_MIU1_DRAM_NONE
    WREG        (MS_BUS_RIU_ADDR + (0x1012f0 <<1 )), 0x8000
    WREG        (MS_BUS_RIU_ADDR + (0x1012e4 <<1 )), 0x0004 @// reg_en_mask
    WREG        (MS_BUS_RIU_ADDR + (0x1012e0 <<1 )), 0x0100 @// reg_en_mask
    WREG        (MS_BUS_RIU_ADDR + (0x1012e0 <<1 )), 0x0101

    DELAYUS     1                                         @//delay 200us
    WREG        (MS_BUS_RIU_ADDR + (0x1012e0 <<1 )), 0x0100 @// reg_en_mask
    WREG        (MS_BUS_RIU_ADDR + (0x1012e0 <<1 )), 0x0101
    DELAYUS     1                                         @//delay 200us
    WREG        (MS_BUS_RIU_ADDR + (0x1012e0 <<1 )), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x1012f0 <<1 )), 0x0000
#endif
    bx          lr
#endif
@//-------------------------------------------------------------------------------------------------
@// Reset_AVD_str
@// @param  None
@// @return None
@// @note  Reset_AVD_str
@//-------------------------------------------------------------------------------------------------

Reset_AVD_str:

    RREG        r8, (MS_BUS_RIU_ADDR + (0x103612 << 1))
    bic         r8, r8, #0x4000
    REG_C2M     r8, (MS_BUS_RIU_ADDR + (0x103612 << 1))

    RREG        r8, (MS_BUS_RIU_ADDR + (0x103612 << 1))
    orr         r8, r8, #0x4000
    REG_C2M     r8, (MS_BUS_RIU_ADDR + (0x103612 << 1))

    RREG        r8, (MS_BUS_RIU_ADDR + (0x103612 << 1))
    bic         r8, r8, #0x4000
    REG_C2M     r8, (MS_BUS_RIU_ADDR + (0x103612 << 1))

    bx          lr
