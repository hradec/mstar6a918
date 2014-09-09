/***************************************************************************************************************
 *
 * FileName hal_card_platform.h
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 * 	   This file is the header file of hal_card_platform_XX.c.
 *	   Every project has the same header file.
 *
 ***************************************************************************************************************/

#ifndef __HAL_CARD_PLATFORM_H
#define __HAL_CARD_PLATFORM_H

#include "hal_card_regs.h"

#if (D_PROJECT == D_PROJECT__EAGLE) || \
    (D_PROJECT == D_PROJECT__EIFFEL) || \
    (D_PROJECT == D_PROJECT__EDISON) || \
    (D_PROJECT == D_PROJECT__NIKE) || \
    (D_PROJECT == D_PROJECT__KAISER)||\
    (D_PROJECT == D_PROJECT__EINSTEIN) || \
    (D_PROJECT == D_PROJECT__KAISERS) || \
    (D_PROJECT == D_PROJECT__NAPOLI)

//----------------------------------------------
// Platform Register Basic Address
//----------------------------------------------
#define A_PMGPIO_BANK       GET_CARD_REG_ADDR(A_RIU_PM_BASE, 0x780)
#define A_CHIPTOP_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0x0F00)
#define A_CLKGEN_BANK       GET_CARD_REG_ADDR(A_RIU_BASE, 0x0580)
#define A_INTR_CTRL_BANK    GET_CARD_REG_ADDR(A_RIU_BASE, 0x0C80)


//----------------------------------------------
// Clock Level Setting (From High Speed to Low Speed)
//----------------------------------------------
#define CLK_F          48000000
#define CLK_E          43200000
#define CLK_D          40000000
#define CLK_C          36000000
#define CLK_B          32000000
#define CLK_A          27000000
#define CLK_9          20000000
#define CLK_8          12000000
#define CLK_7          300000
#define CLK_6          0
#define CLK_5          0
#define CLK_4          0
#define CLK_3          0
#define CLK_2          0
#define CLK_1          0
#define CLK_0          0

#define SDCLK_48M      (0x0F<<2)
#define SDCLK_43_2M    (0x06<<2)
#define SDCLK_40M      (0x05<<2)
#define SDCLK_36M      (0x04<<2)
#define SDCLK_32M      (0x03<<2)
#define SDCLK_27M      (0x02<<2)
#define SDCLK_20M      (0x01<<2)
#define SDCLK_300K     (0x0D<<2)

//----------------------------------------------
// GPIO for SD CDz
//----------------------------------------------

#define REG_GPIO_PM10       GET_CARD_REG_ADDR(A_PMGPIO_BANK, 0x0A)
#define REG_GPIO_PM16       GET_CARD_REG_ADDR(A_PMGPIO_BANK, 0x10)

#define BIT_GPIO10_IN         BIT02
#define BIT_GPIO10_FIQ_MASK   BIT04
#define BIT_GPIO10_FIQ_CLR    BIT06
#define BIT_GPIO10_FIQ_POL    BIT07
#define BIT_GPIO10_FIQ_FINAL  BIT08

#define BIT_GPIO16_IN         BIT02
#define BIT_GPIO16_FIQ_MASK   BIT04
#define BIT_GPIO16_FIQ_CLR    BIT06
#define BIT_GPIO16_FIQ_POL    BIT07
#define BIT_GPIO16_FIQ_FINAL  BIT08

//Use Ext GPIO 8 interrupt (PAD_SPI2_DI)
#define REG_EXT_GPIO_MASK   GET_CARD_REG_ADDR(A_INTR_CTRL_BANK, 0x47)
#define REG_EXT_GPIO_POL    GET_CARD_REG_ADDR(A_INTR_CTRL_BANK, 0x4B)
#define REG_EXT_GPIO_FINAL  GET_CARD_REG_ADDR(A_INTR_CTRL_BANK, 0x4F)
#define REG_EXT_GPIO_CONFIG GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x48)
#define REG_CHIPTOP_DUMMY   GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x1F)
#define REG_EXT_GPIO_FORCE  GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x43)

#define R_08h_EMMC_DRV            GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x8)
#define R_09h_PCM_PE_1                GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x9)
#define R_0Ah_PCM_PE_2              GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0A)
#define R_0Bh_PCM_PE_3              GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0B)
#define R_0Ch_NAND_DRV              GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0C)
#define R_10h_FCIE2MACRO_SD_BYPASS  GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x10)
#define R_12h_TEST_MODE              GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x12)
#define R_1Dh_DUMMY                   GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x1D)
#define R_40h_SD_USE_BYPASS          GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x40)
#define R_50h_ALL_PAIN                GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x50)
#define R_5Ah_SD_CONFIG             GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x5A)
#define R_64h_CI_PCM_CFG            GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x64)
#define R_6Eh_EMMC_CONFIG          GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x6E)
#define R_6Fh_NAND_CS1_n_MODE     GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x6F)

#if (D_PROJECT==D_PROJECT__EAGLE)

//----------------------------------------------
// CHIPTOP for SD part
//----------------------------------------------
#define FCIE_MACRO_BYPASS       GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x43)
#define SD_USE_BYPASS           BIT00
#define FCIE2MACRO_SD_BYPASS    BIT01

#define TOP_EMMC_CFG            GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x5D)
#define REG_EMMC_CFG_CLR        (BIT14|BIT15)

#define NAND_CONFIG             GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0B)
#define REG_SD0_CONFIG_SET1     BIT02
#define REG_SD0_CONFIG_SET2     BIT03
#define REG_SD0_CONFIG2_SET1    BIT04
#define REG_SD0_CONFIG2_SET2    BIT05
#define REG_SD1_CONFIG_SET1     BIT06
#define REG_SD1_CONFIG_SET2     BIT07
#define REG_SD1_CONFIG2_SET1    BIT08
#define REG_SD1_CONFIG2_SET2    BIT09
#define REG_CHK_NAND_PAD        (BIT12|BIT13|BIT14)
#endif
#if (D_PROJECT==D_PROJECT__EDISON)
#define RIU_BASE_FCIE           GET_CARD_REG_ADDR(A_RIU_BASE, 0x8980)
#define  REG_FCIE_2D			GET_CARD_REG_ADDR(RIU_BASE_FCIE,0x2D)
#define  REG_FCIE_2F				GET_CARD_REG_ADDR(RIU_BASE_FCIE,0x2F)
#define REG_PCM_PE				GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x9)
#define REG_PCM_PE2				GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0xa)
#define REG_PCM_PE3				GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0xb)

#define FCIE_MACRO_BYPASS       GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x10)
#define REG_SD_USE_BYPASS  	GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x40)
#define REG_SD_CONFIG		  	GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x5A)

#define REG_EMMC_CFG            GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x6E)

#endif

#if (D_PROJECT == D_PROJECT__KAISER) || \
    (D_PROJECT == D_PROJECT__KAISERS)
#define PMGPIO_BANK       GET_CARD_REG_ADDR(A_RIU_PM_BASE, 0x700)
#define RIU_BASE_FCIE           GET_CARD_REG_ADDR(A_RIU_BASE, 0x8800)

#define CHIPTOP6		 GET_CARD_REG_ADDR (A_CHIPTOP_BANK,0x06)
#define CHIPTOP25   		  GET_CARD_REG_ADDR (A_CHIPTOP_BANK,0x25)
#define CHIPTOP35   		   GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x35)
#define ALL_PAD_IN              GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x50)
#define PMGPIO_CTL                   GET_CARD_REG_ADDR(PMGPIO_BANK,0x35)
#define PMGPIO_OE			 GET_CARD_REG_ADDR(PMGPIO_BANK,0x0F)
#define PMGPIO_MASK		 GET_CARD_REG_ADDR(PMGPIO_BANK,0)
#define PMGPIO_RAWST		 GET_CARD_REG_ADDR(PMGPIO_BANK,0x0C)
#define PMGPIO_FINALST		GET_CARD_REG_ADDR(PMGPIO_BANK,0x0A)
#define PMGPIO_CLR			GET_CARD_REG_ADDR(PMGPIO_BANK,0x04)
#define PMGPIO_POL			GET_CARD_REG_ADDR(PMGPIO_BANK,0x06)
#define MIE_PATH_CTL            GET_CARD_REG_ADDR(RIU_BASE_FCIE,0x0A)
#define SD_MODE                 	GET_CARD_REG_ADDR(RIU_BASE_FCIE,0x10)
#define NC_REORDER                 GET_CARD_REG_ADDR(RIU_BASE_FCIE,0x2D)

#endif
#if (D_PROJECT == D_PROJECT__NIKE)
#define PMGPIO_BANK       GET_CARD_REG_ADDR(A_RIU_PM_BASE, 0x700)
#define RIU_BASE_FCIE           GET_CARD_REG_ADDR(A_RIU_BASE, 0x8980)
#define CHIPTOP_0C   		  GET_CARD_REG_ADDR (A_CHIPTOP_BANK, 0x0c )
#define CHIPTOP_12   		GET_CARD_REG_ADDR   (A_CHIPTOP_BANK, 0x12 )
#define CHIPTOP_1F   		GET_CARD_REG_ADDR   (A_CHIPTOP_BANK, 0x1F )
#define CHIPTOP_42   		GET_CARD_REG_ADDR   (A_CHIPTOP_BANK, 0x42 )
#define CHIPTOP_43   		  GET_CARD_REG_ADDR (A_CHIPTOP_BANK, 0x43 )
#define CHIPTOP_4F   		 GET_CARD_REG_ADDR  (A_CHIPTOP_BANK, 0x4F )
#define CHIPTOP_50   		 GET_CARD_REG_ADDR  (A_CHIPTOP_BANK, 0x50 )
#define CHIPTOP_5B   		   GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x5B )
#define CHIPTOP_5D   		   GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x5D )

#define CHIPTOP_64   		   GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x64 )

#define SD_MODE                 	GET_CARD_REG_ADDR(RIU_BASE_FCIE,0x10)
#define FCIE_2F                 	GET_CARD_REG_ADDR(RIU_BASE_FCIE,0x2F)

#define PMGPIO_OE			 GET_CARD_REG_ADDR(PMGPIO_BANK,0x0F)
#define PMGPIO_MASK		 GET_CARD_REG_ADDR(PMGPIO_BANK,0)
#define PMGPIO_RAWST		 GET_CARD_REG_ADDR(PMGPIO_BANK,0x0C)
#define PMGPIO_FINALST		GET_CARD_REG_ADDR(PMGPIO_BANK,0x0A)
#define PMGPIO_CLR			GET_CARD_REG_ADDR(PMGPIO_BANK,0x04)
#define PMGPIO_POL			GET_CARD_REG_ADDR(PMGPIO_BANK,0x06)

#endif
#if (D_PROJECT == D_PROJECT__EINSTEIN) || \
    (D_PROJECT == D_PROJECT__NAPOLI)
#define PMGPIO_BANK       GET_CARD_REG_ADDR(A_RIU_PM_BASE, 0x780)
#define MIU2_BANK	       	 GET_CARD_REG_ADDR(A_RIU_BASE, 0x300)
#define RIU_BASE_FCIE           GET_CARD_REG_ADDR(A_RIU_BASE, 0x10780)
#define CHIPTOP_0A  		  	GET_CARD_REG_ADDR (A_CHIPTOP_BANK, 0x0A )
#define CHIPTOP_12   		GET_CARD_REG_ADDR   (A_CHIPTOP_BANK, 0x12 )
#define CHIPTOP_1F   		GET_CARD_REG_ADDR   (A_CHIPTOP_BANK, 0x1F )
#define CHIPTOP_42   		GET_CARD_REG_ADDR   (A_CHIPTOP_BANK, 0x42 )
#define CHIPTOP_43   		  GET_CARD_REG_ADDR (A_CHIPTOP_BANK, 0x43 )
#define CHIPTOP_4F   		 GET_CARD_REG_ADDR  (A_CHIPTOP_BANK, 0x4F )
#define CHIPTOP_50   		 GET_CARD_REG_ADDR  (A_CHIPTOP_BANK, 0x50 )
#define CHIPTOP_52   		 GET_CARD_REG_ADDR  (A_CHIPTOP_BANK, 0x52 )
#define CHIPTOP_5B   		   GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x5B )
#define CHIPTOP_5D   		   GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x5D )
#define CHIPTOP_7B   		   GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x7B )
#define CHIPTOP_64   		   GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x64 )
#define MIU2_79				  GET_CARD_REG_ADDR(MIU2_BANK, 0x79)

#define SD_MODE                 	GET_CARD_REG_ADDR(RIU_BASE_FCIE,0x10)
#define FCIE_2F                 	GET_CARD_REG_ADDR(RIU_BASE_FCIE,0x2F)

#define PMGPIO_7	       	 GET_CARD_REG_ADDR(PMGPIO_BANK,0x07)
#define PMGPIO_11	       	 GET_CARD_REG_ADDR(PMGPIO_BANK,0x0b)

#define BIT_GPIO_IN         BIT02
#define BIT_GPIO_FIQ_MASK   BIT04
#define BIT_GPIO_FIQ_CLR    BIT06
#define BIT_GPIO_FIQ_POL    BIT07
#define BIT_GPIO_FIQ_FINAL  BIT08
#endif
#endif


typedef enum
{
	EV_PAD1,
	EV_PAD2,
	EV_PAD3,
	EV_PAD4,
	EV_PAD5,
	EV_PAD6,
	EV_PAD7,
	EV_PAD8,
	EV_NPAD,

} PADEmType;

typedef enum
{
	EV_PULLDOWN,
	EV_PULLUP,

} PinPullEmType;

typedef enum
{
	EV_SD_CMD_DAT0_3,
	EV_SD_CDZ,
	EV_SD_WP,

} PinEmType;

typedef enum
{
	EV_GPIO1        = 0,
	EV_GPIO2        = 1,
	EV_GPIO3        = 2,

} GPIOEmType;

typedef enum
{
	EV_GPIO_OPT1 = 0,
	EV_GPIO_OPT2 = 1,
	EV_GPIO_OPT3 = 2,
	EV_GPIO_OPT4 = 3,

} GPIOOptEmType;

typedef enum
{
	EV_NORVOL        = 0,
	EV_MINVOL        = 1,
	EV_LOWVOL        = 2,
	EV_HIGHVOL       = 3,
    EV_MAXVOL        = 4,

} PADVddEmType;

typedef enum
{
	EV_VDD_DUMMY     = 0,
	EV_VDD_165_195   = BIT07,
	EV_VDD_20_21     = BIT08,
	EV_VDD_21_22     = BIT09,
	EV_VDD_22_23     = BIT10,
	EV_VDD_23_24     = BIT11,
	EV_VDD_24_25     = BIT12,
	EV_VDD_25_26     = BIT13,
	EV_VDD_26_27     = BIT14,
	EV_VDD_27_28     = BIT15,
	EV_VDD_28_29     = BIT16,
	EV_VDD_29_30     = BIT17,
	EV_VDD_30_31     = BIT18,
	EV_VDD_31_32     = BIT19,
	EV_VDD_32_33     = BIT20,
	EV_VDD_33_34     = BIT21,
	EV_VDD_34_35     = BIT22,
	EV_VDD_35_36     = BIT23,
	EV_VDD_50        = BIT24,

} VddEmType;

typedef enum
{
	E_RISING        = 0,
	E_FALLING       = 1,

} E_INTR_POLARITY;


void Hal_CARD_IPOnceSetting(IPEmType eIP);
BOOL_T Hal_CARD_Wait_Emmc_D0(void);
void Hal_CARD_IPBeginSetting(IPEmType eIP, PADEmType ePAD);
void Hal_CARD_IPEndSetting(IPEmType eIP, PADEmType ePAD);
BOOL_T Hal_CARD_Wait_D0_ForEmmc(IPEmType eIP, PADEmType ePAD);

void Hal_CARD_InitPADPin(PADEmType ePAD, BOOL_T bTwoCard);
void Hal_CARD_SetPADToPortPath(IPEmType eIP, PortEmType ePort, PADEmType ePAD, BOOL_T bTwoCard);
void Hal_CARD_PullPADPin(PADEmType ePAD, PinEmType ePin, PinPullEmType ePinPull, BOOL_T bTwoCard);

void Hal_CARD_SetClock(IPEmType eIP, U32_T u32ClkFromIPSet);
U32_T Hal_CARD_FindClockSetting(IPEmType eIP, U32_T u32ReffClk, U8_T u8PassLevel, U8_T u8DownLevel);

void Hal_CARD_SetPADPower(PADEmType ePAD, PADVddEmType ePADVdd);
void Hal_CARD_PowerOn(PADEmType ePAD, U16_T u16DelayMs);
void Hal_CARD_PowerOff(PADEmType ePAD, U16_T u16DelayMs);

void Hal_CARD_SetHighSpeed(IPEmType eIP, BOOL_T bEnable);


U32_T Hal_CARD_GetGPIONum(GPIOEmType eGPIO);
BOOL_T Hal_CARD_GetGPIOState(GPIOEmType eGPIO);
void Hal_CARD_InitGPIO(GPIOEmType eGPIO, BOOL_T bEnable);

void Hal_CARD_SetGPIOIntAttr(GPIOEmType eGPIO, GPIOOptEmType eGPIOOPT);
BOOL_T Hal_CARD_GPIOIntFilter(GPIOEmType eGPIO);

U32_T Hal_CARD_TransMIUAddr(IPEmType eIP, U32_T u32Addr);

#endif //End of __HAL_CARD_PLATFORM_H






