#ifndef	__PLATFORM_H__
#define	__PLATFORM_H__

//------------------------------------------------------------------------------
//  Include Files
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  Macros
//------------------------------------------------------------------------------

/*
 * Physical base addresses
 */
#define chip_CA9X4_MPIC		(0x16000000)

#define chip_MPCORE_SCU		(chip_CA9X4_MPIC + 0x0000)
#define chip_MPCORE_GIC_CPU	(chip_CA9X4_MPIC + 0x0100)
#define chip_MPCORE_GIT		(chip_CA9X4_MPIC + 0x0200)
#define chip_MPCORE_TWD		(chip_CA9X4_MPIC + 0x0600)
#define chip_MPCORE_GIC_DIST	(chip_CA9X4_MPIC + 0x1000)

//------------------------------------------------------------------------------
//
//  Define:  agate_BASE_REG_TIMER_PA
//
//  Locates the timer register base.
//
#define chip_BASE_REG_TIMER0_PA              (0x1F006040)
#define chip_BASE_REG_TIMER1_PA              (0x1F006080)
#define chip_BASE_REG_TIMER2_PA              (0xA0007780)


//------------------------------------------------------------------------------
//  Function prototypes
//------------------------------------------------------------------------------
int Mstar_ehc_platform_init(void);

#endif // __PLATFORM_H__

/* 	END */
