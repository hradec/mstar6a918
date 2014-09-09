#ifndef __CHIP_INT_H__
#define __CHIP_INT_H__

#include <mach/platform.h>

/******************************************************
       CA9 Private Timer
*******************************************************/
#define INT_PPI_FIQ                   28 //GIC PPI FIQ number
#define INT_ID_PTIMER	              29
#define INT_PPI_IRQ                   31 //GIC PPI IRQ number 


#define CONFIG_SPI  0

#if CONFIG_SPI
   #define CONFIG_SPI_IRQ                 1    
   #define INT_SPI_IRQ_HOST0             63
   #define INT_SPI_IRQ_HOST1             64
   #define INT_SPI_IRQ_HOST2             65
   #define INT_SPI_IRQ_HOST3             66
   #define INT_SPI_FIQ_HOST0             67
   #define INT_SPI_FIQ_HOST1             68
   #define INT_SPI_FIQ_HOST2             69
   #define INT_SPI_FIQ_HOST3             70
   #define INT_SPI_IRQ    INT_SPI_IRQ_HOST0
   #define SPI_MIN_NUM                   32
   #define SPI_MAX_NUM                 1020

#endif // end of CONFIG_SPI

//Cortex-A9 GIC PPI 
#define IRQ_LOCALTIMER		29
#define IRQ_LOCALWDOG		30

//Cortex-A9 PMU(Performance Monitor Unit)
#define CHIP_IRQ_PMU0 59
#define CHIP_IRQ_PMU1 60


#define MSTAR_CHIP_INT_END                      256


#define REG_INT_BASE_PA		    0x1F203200
#define REG_INT_BASE                0xFD203200	

#define NR_IRQS		                         256
//IRQ registers
#define REG_IRQ_MASK_L              (REG_INT_BASE+ (0x0034<< 2))
#define REG_IRQ_MASK_H              (REG_INT_BASE+ (0x0035<< 2))
#define REG_IRQ_PENDING_L           (REG_INT_BASE+ (0x003c<< 2))
#define REG_IRQ_PENDING_H           (REG_INT_BASE+ (0x003d<< 2))

//IRQ EXP registers
#define REG_IRQ_EXP_MASK_L          (REG_INT_BASE+ (0x0036<< 2))
#define REG_IRQ_EXP_MASK_H          (REG_INT_BASE+ (0x0037<< 2))
#define REG_IRQ_EXP_PENDING_L       (REG_INT_BASE+ (0x003e<< 2))
#define REG_IRQ_EXP_PENDING_H       (REG_INT_BASE+ (0x003f<< 2))

#if defined(CONFIG_MP_PLATFORM_MSTAR_LEGANCY_INTR)
//FIQ registers
#define REG_FIQ_MASK_L              (REG_INT_BASE+ (0x024<< 2))
#define REG_FIQ_MASK_H              (REG_INT_BASE+ (0x025<< 2))
#define REG_FIQ_CLEAR_L             (REG_INT_BASE+ (0x02c<< 2))
#define REG_FIQ_CLEAR_H             (REG_INT_BASE+ (0x02d<< 2))
#define REG_FIQ_PENDING_L           (REG_INT_BASE+ (0x02c<< 2))
#define REG_FIQ_PENDING_H           (REG_INT_BASE+ (0x02d<< 2))

//FIQ EXP registers
#define REG_FIQ_EXP_MASK_L          (REG_INT_BASE+ (0x026<< 2))
#define REG_FIQ_EXP_MASK_H          (REG_INT_BASE+ (0x027<< 2))
#define REG_FIQ_EXP_CLEAR_L         (REG_INT_BASE+ (0x02e<< 2))
#define REG_FIQ_EXP_CLEAR_H         (REG_INT_BASE+ (0x02f<< 2))
#define REG_FIQ_EXP_PENDING_L       (REG_INT_BASE+ (0x02e<< 2))
#define REG_FIQ_EXP_PENDING_H       (REG_INT_BASE+ (0x02f<< 2))

#else

//FIQ registers
#define REG_FIQ_MASK_L              (REG_INT_BASE+ (0x004<< 2))
#define REG_FIQ_MASK_H              (REG_INT_BASE+ (0x005<< 2))
#define REG_FIQ_CLEAR_L             (REG_INT_BASE+ (0x00c<< 2))
#define REG_FIQ_CLEAR_H             (REG_INT_BASE+ (0x00d<< 2))
#define REG_FIQ_PENDING_L           (REG_INT_BASE+ (0x00c<< 2))
#define REG_FIQ_PENDING_H           (REG_INT_BASE+ (0x00d<< 2))

//FIQ EXP registers
#define REG_FIQ_EXP_MASK_L          (REG_INT_BASE+ (0x006<< 2))
#define REG_FIQ_EXP_MASK_H          (REG_INT_BASE+ (0x007<< 2))
#define REG_FIQ_EXP_CLEAR_L         (REG_INT_BASE+ (0x00e<< 2))
#define REG_FIQ_EXP_CLEAR_H         (REG_INT_BASE+ (0x00f<< 2))
#define REG_FIQ_EXP_PENDING_L       (REG_INT_BASE+ (0x00e<< 2))
#define REG_FIQ_EXP_PENDING_H       (REG_INT_BASE+ (0x00f<< 2))
#endif /* CONFIG_MP_PLATFORM_MSTAR_LEGANCY_INTR */

#define MSTAR_INT_BASE  			128

/*******************************************************/
/*   THE IRQ AND FIQ ARE NOT COMPLETED.                */
/*   FOR EACH IP OWNER, PLEASE REVIEW IT BY YOURSELF   */
/*******************************************************/
typedef enum
{
    // IRQ
    E_IRQL_START                = 64 + MSTAR_INT_BASE,
    E_IRQ_UART0                 = E_IRQL_START + 0,
    E_IRQ_MIIC1                 = E_IRQL_START + 1,
    E_IRQ_DIPW1                 = E_IRQL_START + 2,
    E_IRQ_MVD                   = E_IRQL_START + 3,
    E_IRQ_PS                    = E_IRQL_START + 4,
    E_IRQ_NFIE                  = E_IRQL_START + 5,
    E_IRQ_USB                   = E_IRQL_START + 6,
    E_IRQ_UHC                   = E_IRQL_START + 7,
    E_IRQ_DISP1                 = E_IRQL_START + 8,
    E_IRQ_EMAC                  = E_IRQL_START + 9,
    E_IRQ_DISP                  = E_IRQL_START + 10, /*be used in "kernel/irq/proc.c"*/
    E_IRQ_U3_DPHY               = E_IRQL_START + 11,
    E_IRQ_OTG                   = E_IRQL_START + 12,
    E_IRQ_USB_2                 = E_IRQL_START + 13,
    E_IRQ_BT_DMA                = E_IRQL_START + 14,
    E_IRQ_UHC_2                 = E_IRQL_START + 15,
    E_IRQL_END                  = 15 + E_IRQL_START ,

    E_IRQH_START                = 16 + E_IRQL_START ,
    E_IRQ_TSP2HK                = E_IRQH_START + 0,
    E_IRQ_VE                    = E_IRQH_START + 1,
    E_IRQ_DCSUB                 = E_IRQH_START + 2,
    E_IRQ_DC                    = E_IRQH_START + 3,
    E_IRQ_GOP                   = E_IRQH_START + 4,
    E_IRQ_PCM                   = E_IRQH_START + 5,
    E_IRQ_MIIC0                 = E_IRQH_START + 6,
    E_IRQ_RTC0                  = E_IRQH_START + 7,
    E_IRQ_KEYPAD                = E_IRQH_START + 8, 
    E_IRQ_PM                    = E_IRQH_START + 9,  
    E_IRQ_DDC2BI                = E_IRQH_START + 10,
    E_IRQ_UP_IRQ_EMM_ECM        = E_IRQH_START + 11,
    E_IRQ_UP_IRQ_UART_CA        = E_IRQH_START + 12,
    E_IRQ_RTC1                  = E_IRQH_START + 13,
    E_IRQ_UHC30                 = E_IRQH_START + 14,
    E_IRQ_BT_TABLE              = E_IRQH_START + 15,
    E_IRQH_END                  = 15 + E_IRQH_START,

    // FIQ
    E_FIQL_START                = MSTAR_INT_BASE,
    E_FIQ_EXTIMER0              = E_FIQL_START + 0,
    E_FIQ_EXTIMER1              = E_FIQL_START + 1,
    E_FIQ_WDT                   = E_FIQL_START + 2,
//  E_FIQ_RESERVED              = E_FIQH_START + 3,
//  E_FIQ_RESERVED              = E_FIQL_START + 4,
//  E_FIQ_RESERVED              = E_FIQL_START + 5,
    E_FIQ_MB_DSP2TOMCU0         = E_FIQL_START + 6,
    E_FIQ_MB_DSP2TOMCU1         = E_FIQL_START + 7,
//  E_FIQ_RESERVED              = E_FIQL_START + 8,
    E_FIQ_UP_IRQ_UART           = E_FIQL_START + 9,
    E_FIQ_SATA_PHY              = E_FIQL_START + 10,
    E_FIQ_HDMI_NON_PCM          = E_FIQL_START + 11,
    E_FIQ_SPDIF_IN_NON_PCM      = E_FIQL_START + 12,
    E_FIQ_EMAC                  = E_FIQL_START + 13,
    E_FIQ_SE_DSP2UP             = E_FIQL_START + 14,
    E_FIQ_TSP2AEON              = E_FIQL_START + 15,
    E_FIQL_END                  = 15 + E_FIQL_START,

    E_FIQH_START                = 16 + E_FIQL_START,  //16
    E_FIQ_VIVALDI_STR           = E_FIQH_START + 0,
    E_FIQ_VIVALDI_PTS           = E_FIQH_START + 1,
    E_FIQ_DSP_MIU_PROT          = E_FIQH_START + 2,
    E_FIQ_XIU_TIMEOUT           = E_FIQH_START + 3,
    E_FIQ_DMDMCU2HK0            = E_FIQH_START + 4,
    E_FIQ_VSYNC_VE4VBI          = E_FIQH_START + 5,
    E_FIQ_FIELD_VE4VBI          = E_FIQH_START + 6,
    E_FIQ_VDMCU2HK              = E_FIQH_START + 7,
    E_FIQ_VE_DONE_TT            = E_FIQH_START + 8,
    E_FIQ_CCFL                  = E_FIQH_START + 9,
    E_FIQ_INT_IN                = E_FIQH_START + 10,
    E_FIQ_IR                    = E_FIQH_START + 11,
//  E_FIQ_RESERVED              = E_FIQH_START + 12,
    E_FIQ_DEC_DSP2UP            = E_FIQH_START + 13,
    E_FIQ_DMDMCU2HK1            = E_FIQH_START + 14,
    E_FIQ_DSP2ARM               = E_FIQH_START + 15,
    E_FIQH_END                  = 15 + E_FIQH_START,

    //IRQEXP
    E_IRQEXPL_START             = 16 + E_IRQH_START , //32
    E_IRQEXPL_HVD               = E_IRQEXPL_START + 0,
    E_IRQEXPL_USB1              = E_IRQEXPL_START + 1,
    E_IRQEXPL_UHC1              = E_IRQEXPL_START + 2,
    E_IRQEXPL_MIU               = E_IRQEXPL_START + 3,
    E_IRQEXPL_MFE               = E_IRQEXPL_START + 4,
    E_IRQEXPL_SATA              = E_IRQEXPL_START + 5,
    E_IRQEXPL_AEON2HI           = E_IRQEXPL_START + 6,
    E_IRQEXPL_UART1             = E_IRQEXPL_START + 7,
    E_IRQEXPL_UART2             = E_IRQEXPL_START + 8,
    E_IRQEXPL_SDIO              = E_IRQEXPL_START + 9,
    E_IRQEXPL_MPIF              = E_IRQEXPL_START + 10,
    E_IRQEXPL_GE                = E_IRQEXPL_START + 11,
    E_IRQEXPL_GPD               = E_IRQEXPL_START + 12,
    E_IRQEXPL_JPD               = E_IRQEXPL_START + 13,
    E_IRQEXPL_CA_MVD_CHECKSUMFAIL = E_IRQEXPL_START + 14,
    E_IRQEXPL_CA_TSP_CHECKSUMFAIL = E_IRQEXPL_START + 15,
    E_IRQEXPL_END               = 15 + E_IRQEXPL_START,

    E_IRQEXPH_START             = 16 + E_IRQEXPL_START , //48
    E_IRQEXPH_BDMA0             = E_IRQEXPH_START + 0,
    E_IRQEXPH_BDMA1             = E_IRQEXPH_START + 1,
    E_IRQEXPH_UART2MCU          = E_IRQEXPH_START + 2,
    E_IRQEXPH_URDMA2MCU         = E_IRQEXPH_START + 3,
    E_IRQEXPH_DVI_HDMI_HDCP     = E_IRQEXPH_START + 4,
    E_IRQEXPH_CEC               = E_IRQEXPH_START + 5,
    E_IRQEXPH_HDMITX            = E_IRQEXPH_START + 6,
    E_IRQEXPH_GPU2MCU           = E_IRQEXPH_START + 7,
    E_IRQEXPH_HDCP_X74          = E_IRQEXPH_START + 8,
    E_IRQEXPH_WADR_ERR_INT      = E_IRQEXPH_START + 9,
    E_IRQEXPH_CA_I3             = E_IRQEXPH_START + 10,
    E_IRQEXPH_CA_SVP_CMD        = E_IRQEXPH_START + 11,
    E_IRQEXPH_RASP1             = E_IRQEXPH_START + 12,
    E_IRQEXPH_RASP0             = E_IRQEXPH_START + 13,
    E_IRQEXPH_SECEMAC           = E_IRQEXPH_START + 14,
    E_IRQEXPH_FRM_PM            = E_IRQEXPH_START + 15,
    E_IRQEXPH_END               = 15 + E_IRQEXPH_START ,

    // FIQEXP
    E_FIQEXPL_START             = 16 + E_FIQH_START,   //32
    E_FIQEXPL_IR_INT_RC         = E_FIQEXPL_START + 0,
    E_FIQEXPL_AU_DMA_BUF_INT    = E_FIQEXPL_START + 1,
    E_FIQEXPL_VE_SW_WR2BUF      = E_FIQEXPL_START + 2,
    E_FIQEXPL_EMM_ECM           = E_FIQEXPL_START + 3,
    E_FIQEXPL_8051_TO_SECURER2  = E_FIQEXPL_START + 4,
    E_FIQEXPL_8051_TO_ARMC1     = E_FIQEXPL_START + 5,
    E_FIQEXPL_8051_TO_ARM       = E_FIQEXPL_START + 6,
//  E_FIQEXPL_RESERVED          = E_FIQEXPL_START + 7,
    E_FIQEXPL_ARM_TO_SECURER2   = E_FIQEXPL_START + 8,
    E_FIQ_INT_AEON_TO_ARM      = E_FIQEXPL_START + 9,
    E_FIQEXPL_ARM_TO_8051       = E_FIQEXPL_START + 10,
//  E_FIQEXPL_RESERVED          = E_FIQEXPL_START + 11,
    E_FIQEXPL_ARMC1_TO_SECURER2 = E_FIQEXPL_START + 12,
    E_FIQEXPL_ARMC1_TO_ARM      = E_FIQEXPL_START + 13,
    E_FIQEXPL_ARMC1_TO_8051     = E_FIQEXPL_START + 14,
//  E_FIQEXPL_RESERVED          = E_FIQEXPL_START + 15,
    E_FIQEXPL_END               = 15 + E_FIQEXPL_START ,

    E_FIQEXPH_START             = 16 + E_FIQEXPL_START,  //48
    E_FIQEXPL_SECURER2_TO_ARMC1 = E_FIQEXPH_START + 0,
    E_FIQEXPL_SECURER2_TO_ARM   = E_FIQEXPH_START + 1,
    E_FIQEXPL_SECURER2_TO_8051  = E_FIQEXPH_START + 2,
    E_FIQEXPH_SECEMAC           = E_FIQEXPH_START + 3,
//  E_FIQEXPH_RESERVED          = E_FIQEXPH_START + 4,
//  E_FIQEXPH_RESERVED          = E_FIQEXPH_START + 5,
    E_FIQEXPH_HDMITX_EDG        = E_FIQEXPH_START + 6,
//  E_FIQEXPH_RESERVED          = E_FIQEXPH_START + 7,
    E_FIQEXPH_PVR2MI0           = E_FIQEXPH_START + 8,
    E_FIQEXPH_PVR2MI1           = E_FIQEXPH_START + 9,
    E_FIQEXPH_IR2               = E_FIQEXPH_START + 10,
    E_FIQEXPH_IR2_RC            = E_FIQEXPH_START + 11,
//  E_FIQEXPH_RESERVED          = E_FIQEXPH_START + 12,
//  E_FIQEXPH_RESERVED          = E_FIQEXPH_START + 13,
//  E_FIQEXPH_RESERVED          = E_FIQEXPH_START + 14,
    E_FIQEXPH_FRM_PM            = E_FIQEXPH_START + 15,
    E_FIQEXPH_END               = 15 + E_FIQEXPL_START ,

  #if 0
    // IRQ Virtual Low
    E_IRQVIRL_START                     = 128 + MSTAR_INT_BASE,
    E_IRQVIRL_END                       = 15 + E_IRQVIRL_START ,

    // FIQ Virtual Low
    E_FIQVIRL_START                     = 144 + MSTAR_INT_BASE,
    E_FIQVIRL_8051_TO_AEON              = E_FIQVIRL_START+0,
    E_FIQVIRL_AEON_TO_8051              = E_FIQVIRL_START+1,
    E_FIQVIRL_ARM_TO_FRCR2             = E_FIQVIRL_START+2,
    E_FIQVIRL_END                       = 15 +  E_FIQVIRL_START  ,
  #endif
    E_IRQ_FIQ_ALL               = 0xFF //all IRQs & FIQs
} InterruptNum;

    // REG_FIQ_MASK_L
    //FIQ Low 16 bits
    #define FIQL_MASK                           0xFFFF
    #define FIQ_EXTIMER0                        (0x1 << (E_FIQ_EXTIMER0         - E_FIQL_START) )
    #define FIQ_EXTIMER1                        (0x1 << (E_FIQ_EXTIMER1         - E_FIQL_START) )
    #define FIQ_WDT                             (0x1 << (E_FIQ_WDT              - E_FIQL_START) )
    #define FIQ_MB_DSP2TOMCU0                   (0x1 << (E_FIQ_MB_DSP2TOMCU0    - E_FIQL_START) )
    #define FIQ_MB_DSP2TOMCU1                   (0x1 << (E_FIQ_MB_DSP2TOMCU1    - E_FIQL_START) )
    #define FIQ_UP_IRQ_UAR                      (0x1 << (E_FIQ_UP_IRQ_UAR       - E_FIQL_START) )
    #define E_FIQ_SATA                          (0x1 << (E_FIQ_SATA             - E_FIQL_START) )
    #define FIQ_HDMI_NON_PCM                    (0x1 << (E_FIQ_HDMI_NON_PCM     - E_FIQL_START) )
    #define FIQ_SPDIF_IN_NON_PCM                (0x1 << (E_FIQ_SPDIF_IN_NON_PCM - E_FIQL_START) )
    #define FIQ_EMAC                            (0x1 << (E_FIQ_EMAC             - E_FIQL_START) )
    #define FIQ_SE_DSP2UP                       (0x1 << (E_FIQ_SE_DSP2UP        - E_FIQL_START) )
    #define FIQ_TSP2AEON                        (0x1 << (E_FIQ_TSP2AEON         - E_FIQL_START) )


    // REG_FIQ_MASK_H
    //FIQ High 16 bits
    #define FIQH_MASK                           0xFFFF
    #define FIQ_VIVALDI_STR                     (0x1 << (E_FIQ_VIVALDI_STR      - E_FIQH_START) )
    #define FIQ_VIVALDI_PTS                     (0x1 << (E_FIQ_VIVALDI_PTS      - E_FIQH_START) )
    #define FIQ_DSP_MIU_PROT                    (0x1 << (E_FIQ_DSP_MIU_PROT     - E_FIQH_START) )
    #define FIQ_XIU_TIMEOUT                     (0x1 << (E_FIQ_XIU_TIMEOUT      - E_FIQH_START) )
    #define FIQ_DMDMCU2HK0                      (0x1 << (E_FIQ_DMDMCU2HK0       - E_FIQH_START) )
    #define FIQ_VSYNC_VE4VBI                    (0x1 << (E_FIQ_VSYNC_VE4VBI     - E_FIQH_START) )
    #define FIQ_FIELD_VE4VBI                    (0x1 << (E_FIQ_FIELD_VE4VBI     - E_FIQH_START) )
    #define FIQ_VDMCU2HK                        (0x1 << (E_FIQ_VDMCU2HK         - E_FIQH_START) )
    #define FIQ_VE_DONE_TT                      (0x1 << (E_FIQ_VE_DONE_TT       - E_FIQH_START) )
    #define FIQ_CCFL                            (0x1 << (E_FIQ_CCFL             - E_FIQH_START) )
    #define FIQ_INT_IN                          (0x1 << (E_FIQ_INT_IN           - E_FIQH_START) )
    #define FIQ_IR                              (0x1 << (E_FIQ_IR               - E_FIQH_START) )
    #define FIQ_DEC_DSP2UP                      (0x1 << (E_FIQ_DEC_DSP2UP       - E_FIQH_START) )
    #define FIQ_DMDMCU2HK1                      (0x1 << (E_FIQ_DMDMCU2HK1        - E_FIQH_START) )
    #define FIQ_DSP2ARM                         (0x1 << (E_FIQ_DSP2ARM          - E_FIQH_START) )



    // #define REG_IRQ_PENDING_L
    #define IRQL_MASK                           0xFFFF
    #define IRQ_UART0                           (0x1 << (E_IRQ_UART0            - E_IRQL_START) )
    #define IRQ_MIIC1                           (0x1 << (E_IRQ_MIIC1            - E_IRQL_START) )
    #define IRQ_DIPW1                           (0x1 << (E_IRQ_DIPW1            - E_IRQL_START) )
    #define IRQ_MVD                             (0x1 << (E_IRQ_MVD              - E_IRQL_START) )
    #define IRQ_PS                              (0x1 << (E_IRQ_PS               - E_IRQL_START) )
    #define IRQ_NFIE                            (0x1 << (E_IRQ_NFIE             - E_IRQL_START) )
    #define IRQ_USB                             (0x1 << (E_IRQ_USB              - E_IRQL_START) )
    #define IRQ_UHC                             (0x1 << (E_IRQ_UHC              - E_IRQL_START) )
    #define IRQ_DISP1                           (0x1 << (E_IRQ_DISP1            - E_IRQL_START) )
    #define IRQ_EMAC                            (0x1 << (E_IRQ_EMAC             - E_IRQL_START) )
    #define IRQ_DISP                            (0x1 << (E_IRQ_DISP             - E_IRQL_START) )
    #define IRQ_U3_DPHY                         (0x1 << (E_IRQ_U3_DPHY          - E_IRQL_START) )
    #define IRQ_OTG                             (0x1 << (E_IRQ_OTG              - E_IRQL_START) )
    #define IRQ_USB2                            (0x1 << (E_USB2                 - E_IRQL_START) )
    #define IRQ_BT_DMA                          (0x1 << (E_IRQ_BT_DMA           - E_IRQL_START) )
    #define IRQ_UHC_2                           (0x1 << (E_IRQ_UHC_2            - E_IRQL_START) )


    // #define REG_IRQ_PENDING_H
    #define IRQH_MASK                           0xFFFF
    #define IRQ_TSP2HK                          (0x1 << (E_IRQ_TSP2HK           - E_IRQH_START) )
    #define IRQ_VE                              (0x1 << (E_IRQ_VE               - E_IRQH_START) )
    #define IRQ_DCSUB                           (0x1 << (E_IRQ_DCSUB            - E_IRQH_START) )
    #define IRQ_DC                              (0x1 << (E_IRQ_DC               - E_IRQH_START) )
    #define IRQ_GOP                             (0x1 << (E_IRQ_GOP              - E_IRQH_START) )
    #define IRQ_PCM                             (0x1 << (E_IRQ_PCM              - E_IRQH_START) )
    #define IRQ_MIIC0                           (0x1 << (E_IRQ_MIIC0            - E_IRQH_START) )
    #define IRQ_RTC0                            (0x1 << (E_IRQ_RTC0             - E_IRQH_START) )
    #define IRQ_KEYPAD                          (0x1 << (E_IRQ_KEYPAD           - E_IRQH_START) )
    #define IRQ_PM                              (0x1 << (E_IRQ_PM               - E_IRQH_START) )
    #define IRQ_DDC2BI                          (0x1 << (E_IRQ_DDC2BI           - E_IRQH_START) )
    #define IRQ_UP_IRQ_EMM_ECM                  (0x1 << (E_IRQ_UP_IRQ_EMM_EC    - E_IRQH_START) )
    #define IRQ_UP_IRQ_UART_CA                  (0x1 << (E_IRQ_UP_IRQ_UART_CA   - E_IRQH_START) )
    #define IRQ_RTC1                            (0x1 << (E_IRQ_RTC1             - E_IRQH_START) )
    #define IRQ_UHC30                           (0x1 << (E_IRQ_UHC30            - E_IRQH_START) )
    #define IRQ_BT_TABLE                        (0x1 << (E_IRQ_BT_TABLE         - E_IRQH_START) )

    // #define REG_IRQEXP_PENDING_L
    #define IRQEXPL_MASK                        0xFFFF
    #define IRQEXPL_HVD                         (0x1 << (E_IRQEXPL_HVD          - E_IRQEXPL_START) )
    #define IRQEXPL_USB1                        (0x1 << (E_IRQEXPL_USB1         - E_IRQEXPL_START) )
    #define IRQEXPL_UHC1                        (0x1 << (E_IRQEXPL_UHC1         - E_IRQEXPL_START) )
    #define IRQEXPL_MIU                         (0x1 << (E_IRQEXPL_MIU          - E_IRQEXPL_START) )
    #define IRQEXPL_MFE                         (0x1 << (E_IRQEXPL_MFE          - E_IRQEXPL_START) )
    #define IRQEXPL_SATA                        (0x1 << (E_IRQEXPL_SATA         - E_IRQEXPL_START) )
    #define IRQEXPL_AEON2HI                     (0x1 << (E_IRQEXPL_AEON2HI      - E_IRQEXPL_START) )
    #define IRQEXPL_UART1                       (0x1 << (E_IRQEXPL_UART1        - E_IRQEXPL_START) )
    #define IRQEXPL_UART2                       (0x1 << (E_IRQEXPL_UART2        - E_IRQEXPL_START) )
    #define IRQEXPL_SDIO                        (0x1 << (E_IRQEXPL_SDIO         - E_IRQEXPL_START) )
    #define IRQEXPL_MPIF                        (0x1 << (E_IRQEXPL_MPIF         - E_IRQEXPL_START) )
    #define IRQEXPL_GE                          (0x1 << (E_IRQEXPL_GE           - E_IRQEXPL_START) )
    #define IRQEXPL_GPD                         (0x1 << (E_IRQEXPL_GPD          - E_IRQEXPL_START) )
    #define IRQEXPL_JPD                         (0x1 << (E_IRQEXPL_JPD          - E_IRQEXPL_START) )
    #define IRQEXPL_CA_MVD_CHECKSUMFAIL         (0x1 << (E_IRQEXPL_CA_MVD_CHECKSUMFAIL  - E_IRQEXPL_START) )
    #define IRQEXPL_CA_TSP_CHECKSUMFAIL         (0x1 << (E_IRQEXPL_CA_MVD_CHECKSUMFAIL  - E_IRQEXPL_START) )

    // #define REG_IRQEXP_PENDING_H
    #define IRQEXPH_MASK                        0xFFFF
    #define IRQEXPH_BDMA0                       (0x1 << (E_IRQEXPH_BDMA0        - E_IRQEXPH_START) )
    #define IRQEXPH_BDMA1                       (0x1 << (E_IRQEXPH_BDMA1        - E_IRQEXPH_START) )
    #define IRQEXPH_UART2MCU                    (0x1 << (E_IRQEXPH_UART2MCU     - E_IRQEXPH_START) )
    #define IRQEXPH_URDMA2MCU                   (0x1 << (E_IRQEXPH_URDMA2MCU    - E_IRQEXPH_START) )
    #define IRQEXPH_DVI_HDMI_HDCP               (0x1 << (E_IRQEXPH_DVI_HDMI_HDCP- E_IRQEXPH_START) )
    #define IRQEXPH_CEC                         (0x1 << (E_IRQEXPH_CEC          - E_IRQEXPH_START) )
    #define IRQEXPH_HDMITX                      (0x1 << (E_IRQEXPH_HDMITX       - E_IRQEXPH_START) )
    #define IRQEXPH_GPU2MCU                     (0x1 << (E_IRQEXPH_GPU2MCU      - E_IRQEXPH_START) )
    #define IRQEXPH_HDCP_X74                    (0x1 << (E_IRQEXPH_HDCP_X74     - E_IRQEXPH_START) )
    #define IRQEXPH_WADR_ERR_INT                (0x1 << (E_IRQEXPH_WADR_ERR_INT - E_IRQEXPH_START) )
    #define IRQEXPH_CA_I3                       (0x1 << (E_IRQEXPH_CA_I3        - E_IRQEXPH_START) )
    #define IRQEXPH_CA_SVP_CMD                  (0x1 << (E_IRQEXPH_CA_SVP_CMD   - E_IRQEXPH_START) )
    #define IRQEXPH_RASP1                       (0x1 << (E_IRQEXPH_RASP1        - E_IRQEXPH_START) )
    #define IRQEXPH_RASP0                       (0x1 << (E_IRQEXPH_RASP0        - E_IRQEXPH_START) )
    #define IRQEXPH_SECEMAC                     (0x1 << (E_IRQEXPH_SECEMAC      - E_IRQEXPH_START) )
    #define IRQEXPH_FRM_PM                      (0x1 << (E_IRQEXPH_FRM_PM       - E_IRQEXPH_START) )

    // #define REG_FIQEXP_PENDING_L
    #define FIQEXPL_MASK                        0xFFFF
    #define FIQEXPL_IR_INT_RC                   (0x1 << (E_FIQEXPL_IR_INT_RC       - E_FIQEXPL_START) )
    #define FIQEXPL_AU_DMA_BUF_INT              (0x1 << (E_FIQEXPL_AU_DMA_BUF_INT  - E_FIQEXPL_START) )
    #define FIQEXPL_VE_SW_WR2BUF                (0x1 << (E_FIQEXPL_VE_SW_WR2BUF    - E_FIQEXPL_START) )
    #define FIQEXPL_EMM_ECM                     (0x1 << (E_FIQEXPL_EMM_ECM         - E_FIQEXPL_START) )
    #define FIQEXPL_8051_TO_SECURER2            (0x1 << (E_FIQEXPL_8051_TO_SECURER2 - E_FIQEXPL_START) ) 
    #define FIQEXPL_8051_TO_ARMC1               (0x1 << (E_FIQEXPL_8051_TO_ARMC1   - E_FIQEXPL_START) )
    #define FIQEXPL_8051_TO_ARM                 (0x1 << (E_FIQEXPL_8051_TO_ARM     - E_FIQEXPL_START) ) 
    #define FIQEXPL_ARM_TO_SECURER2             (0x1 << (E_FIQEXPL_ARM_TO_SECURER2 - E_FIQEXPL_START) )
    #define FIQEXPL_ARM_TO_ARMC1                (0x1 << (E_FIQEXPL_ARM_TO_ARMC1    - E_FIQEXPL_START) )
    #define FIQEXPL_ARM_TO_8051                 (0x1 << (E_FIQEXPL_ARM_TO_8051     - E_FIQEXPL_START) ) 
    #define FIQEXPL_ARMC1_TO_SECURER2           (0x1 << (E_FIQEXPL_ARMC1_TO_SECURER2 - E_FIQEXPL_START) )
    #define FIQEXPL_ARMC1_TO_ARM                (0x1 << (E_FIQEXPL_ARMC1_TO_ARM    - E_FIQEXPL_START) ) 
    #define FIQEXPL_ARMC1_TO_8051               (0x1 << (E_FIQEXPL_ARMC1_TO_8051   - E_FIQEXPL_START) )

    // #define REG_FIQEXP_PENDING_H
    #define FIQEXPH_SECURER2_TO_ARMC1           (0x1 << (E_FIQEXPH_SECURER2_TO_ARMC1- E_FIQEXPH_START) )
    #define FIQEXPH_SECURER2_TO_ARM             (0x1 << (E_FIQEXPH_SECURER2_TO_ARM  - E_FIQEXPH_START) )
    #define FIQEXPH_SECURER2_TO_8051            (0x1 << (E_FIQEXPH_SECURER2_TO_8051 - E_FIQEXPH_START) )
    #define FIQEXPH_SECEMAC                     (0x1 << (E_FIQEXPH_SECEMAC       - E_FIQEXPH_START) )
    #define FIQEXPH_HDMITX_EDG                  (0x1 << (E_FIQEXPH_HDMITX_EDG       - E_FIQEXPH_START) )
    #define FIQEXPH_PVR2MI0                     (0x1 << (E_FIQEXPH_PVR2MI0          - E_FIQEXPH_START) )
    #define FIQEXPH_PVR2MI1                     (0x1 << (E_FIQEXPH_PVR2MI1          - E_FIQEXPH_START) )
    #define FIQEXPH_IR2                         (0x1 << (E_FIQEXPH_IR2              - E_FIQEXPH_START) )
    #define FIQEXPH_IR2_RC                      (0x1 << (E_FIQEXPH_IR2_RC           - E_FIQEXPH_START) )
    #define FIQEXPH_FRM_PM                      (0x1 << (E_FIQEXPH_FRM_PM           - E_FIQEXPH_START) )


#define IRQL_EXP_ALL                    0xFFFF
#define IRQH_EXP_ALL                    0xFFFF
#define FIQL_EXP_ALL                    0xFFFF
#define FIQH_EXP_ALL                    0xFFFF

#endif // #ifndef __CHIP_INT_H__
