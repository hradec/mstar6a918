//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>
#ifndef __XHCI_REG_H
#define __XHCI_REG_H

/* --- XHCI Capability Registers --- */
struct xhci_cap_regs {
	__le32	caplen_hciver;
	__le32	u32HcsParams1;
	__le32	u32HcsParams2;
	__le32	u32HcsParams3;
	__le32	u32HccParams;
	__le32	u32DbOff;
	__le32	u32RtsOff;
};

/* Capability register length bitmasks*/
#define CAP_REGS_LENGTH(p)	(((p)>>00)&0x00ff)

/* HCSPARAMS1 */
/* Bit 0:7 - Number of Device Slots */
#define HCS_MAX_NUM_SLOTS(p)	(((p) >> 0) & 0xff)
#define HCS_MAX_SLOTS_MASK		0xff
/* Bit 8:18 - Number of Interrupters */
/* Bit 24:31 - Number of Ports - maximum to 127 ports */
#define HCS_MAX_NUM_PORTS(p)	(((p) >> 24) & 0x7f)

/* HCSPARAMS2 */
/* Bit 0:3 - Isochronous Scheduling Threshold */
/* Bit 4:7 - Event Ring Segment Table Max */
/* Bit 26 - Scratchpad Restore */
/* Bit 27:31 - Max Scratchpad Buffers */
#define HCS_MAX_SPR(p)   (((p) >> 27) & 0x1f)

/* HCCPARAMS */
/* Bit2 - If "1", xHC uses 64 byte Context data structures */
#define HCC_64BYTE_CONTEXT_SIZE(p)	((p) & (1 << 2))

/* Doorbell offset register bitmask - Bit 0~1 reserved */
#define	DBOFF_RSVD_MASK	(~0x3)

/* Runtime register space offset bitmask - Bit 0~4 reserved */
#define	RTSOFF_RSVD_MASK	(~0x1f)

/* Number of regs per xHC port */
#define	PORT_REGS_NUM	4

/* --- XHCI Operational Registers --- */
struct xhci_op_regs {
	__le32	u32UsbCmd;
	__le32	u32UsbSts;
	__le32	u32PageSize;
	__le32	u32Rsvd1;
	__le32	u32Rsvd2;
	__le32	u32DnCtrl;
	__le64	u64CrCr;
	__le32	u32Rsvd3[4];
	__le64	u64DcbaaPtr;
	__le32	u32Config;
	__le32	u32Rsvd4[241];
	__le32	u32PortSc;
	__le32	u32PortPmsc;
	__le32	u32PortLi;
	__le32	u32Rsvd5;
	__le32	u32Rsvd6[PORT_REGS_NUM*254];
};

/* USB Command Register */
/* Bit0 - Run/Stop the xHC controller */
#define USBCMD_RUN		(1 << 0)
/* Bit1 - Host Controller Reset */
#define USBCMD_RST	    (1 << 1)
/* Bit2 - Interrupter Enable */
#define USBCMD_INTE		(1 << 2)
/* Bit3 - Host System Error Enable */
#define USBCMD_HSEE	    (1 << 3)
/* Bit7 - lLight Host Controller Reset */
#define USBCMD_LHCRST	(1 << 7)
/* Bit8 - Controller Save State */
#define USBCMD_CSS		(1 << 8)
/* Bit9 - Controller Restore State */
#define USBCMD_CRS		(1 << 9)
/* Bit10 - Enable Wrap Event */
#define USBCMD_EWE	    (1 << 10)
/* Bit11 - Enable U3 MFINDEX Stop */
#define USBCMD_EU3S	    (1 << 11)

/* USB Status Register */
/* Bit0 - HCHalted */
#define USBSTS_HALT	(1<<0)

/* serious error, e.g. PCI parity error.  The HC will clear the run/stop bit. */
#define USBSTS_HSE	(1 << 2)
/* event interrupt - clear this prior to clearing any IP flags in IR set*/
#define USBSTS_EINT	(1 << 3)
/* port change detect */
#define USBSTS_PCD	(1 << 4)
/* Bit 5:7 reserved and zeroed */
/* save state status - '1' means xHC is saving state */
#define USBSTS_SSS	(1 << 8)
/* restore state status - '1' means xHC is restoring state */
#define USBSTS_RSS	(1 << 9)
/* true: save or restore error */
#define USBSTS_SRE		(1 << 10)
/* true: Controller Not Ready to accept doorbell or op reg writes after reset */
#if 0 //MBOOT_XHCI
#define USBSTS_CNR		XHCI_STS_CNR
#else
#define USBSTS_CNR		(1 << 11)
#endif
/* true: internal Host Controller Error - SW needs to reset and reinitialize */
#define USBSTS_HCE		(1 << 12)
/* Bit 13:31 reserved and should be preserved */

/*
 * DNCTRL - Device Notification Control Register - dev_notification bitmasks
 * Generate a device notification event when the HC sees a transaction with a
 * notification type that matches a bit set in this bit field.
 */
//#define	DEV_NOTE_MASK		(0xffff)
//#define ENABLE_DEV_NOTE(x)	(1 << (x))
/* Most of the device notification types should only be used for debug.
 * SW does need to pay attention to function wake notifications.
 */
//#define	DEV_NOTE_FWAKE		ENABLE_DEV_NOTE(1)

/* CRCR - Command Ring Control Register - cmd_ring bitmasks */
/* bit 0 is the command ring cycle state */
/* stop ring operation after completion of the currently executing command */
//#define CMD_RING_PAUSE		(1 << 1)
/* stop ring immediately - abort the currently executing command */
//#define CMD_RING_ABORT		(1 << 2)
/* true: command ring is running */
//#define CMD_RING_RUNNING	(1 << 3)
/* Bit 4:5 reserved and should be preserved */
/* Command Ring pointer - bit mask for the lower 32 Bit. */
#define CMD_RING_MASK_BITS	(0x3f)

/* CONFIG - Configure Register - config_reg bitmasks */
/* Bit 0:7 - maximum number of device slots enabled (NumSlotsEn) */
//#define MAX_DEVS(p)	((p) & 0xff)
/* Bit 8:31 - reserved and should be preserved */

/* PORTSC - Port Status and Control Register - port_status_base bitmasks */
/* true: device connected */
#define PORTSC_CCS	(1 << 0)
/* true: port enabled */
#define PORTSC_PED		(1 << 1)
/* bit 2 reserved and zeroed */
/* true: port has an over-current condition */
#define PORTSC_OCA		(1 << 3)
/* true: port reset signaling asserted */
#define PORTSC_RST	(1 << 4)
/* Port Link State - Bit 5:8
 * A read gives the current link PM state of the port,
 * a write with Link State Write Strobe set sets the link state.
 */
#define PORTSC_PLS_MASK	(0xf << 5)
#define LINK_U0		(0x0 << 5)
#define LINK_U3		(0x3 << 5)
#define LINK_RESUME	(0xf << 5)
/* true: port has power (see HCC_PPC) */
#define PORTSC_POWER	(1 << 9)
/* Bit 10:13 indicate device speed:
 * 0 - undefined speed - port hasn't be initialized by a reset yet
 * 1 - full speed
 * 2 - low speed
 * 3 - high speed
 * 4 - super speed
 * 5-15 reserved
 */
#define PORTSC_SPEED_MASK		(0xf << 10)
#define	U3DEV_FS			(0x1 << 10)
#define	U3DEV_LS			(0x2 << 10)
#define	U3DEV_HS			(0x3 << 10)
#define	U3DEV_SS			(0x4 << 10)
#define DEV_IS_UNDEFSPEED(p)	(((p) & PORTSC_SPEED_MASK) == (0x0<<10))
#define DEV_IS_FULLSPEED(p)	(((p) & PORTSC_SPEED_MASK) == U3DEV_FS)
#define DEV_IS_LOWSPEED(p)		(((p) & PORTSC_SPEED_MASK) == U3DEV_LS)
#define DEV_IS_HIGHSPEED(p)	(((p) & PORTSC_SPEED_MASK) == U3DEV_HS)
#define DEV_IS_SUPERSPEED(p)	(((p) & PORTSC_SPEED_MASK) == U3DEV_SS)
/* Bits 20:23 in the Slot Context are the speed for the device */
#define	SLOT_CTX_SPEED_FS		(U3DEV_FS << 10)
#define	SLOT_CTX_SPEED_LS		(U3DEV_LS << 10)
#define	SLOT_CTX_SPEED_HS		(U3DEV_HS << 10)
#define	SLOT_CTX_SPEED_SS		(U3DEV_SS << 10)
/* Port Indicator Control */
//#define PORT_LED_OFF	(0 << 14)
//#define PORT_LED_AMBER	(1 << 14)
//#define PORT_LED_GREEN	(2 << 14)
//#define PORT_LED_MASK	(3 << 14)
/* Port Link State Write Strobe - set this when changing link state */
#define PORTSC_LWS	(1 << 16)
/* true: connect status change */
#define PORTSC_CSC	(1 << 17)
/* true: port enable change */
#define PORTSC_PEC	(1 << 18)
/* true: warm reset for a USB 3.0 device is done.  A "hot" reset puts the port
 * into an enabled state, and the device into the default state.  A "warm" reset
 * also resets the link, forcing the device through the link training sequence.
 * SW can also look at the Port Reset register to see when warm reset is done.
 */
#define PORTSC_WRC	(1 << 19)
/* true: over-current change */
#define PORTSC_OCC	(1 << 20)
/* true: reset change - 1 to 0 transition of PORTSC_RST */
#define PORTSC_PRC		(1 << 21)
/* port link status change - set on some port link state transitions:
 *  Transition				Reason
 *  ------------------------------------------------------------------------------
 *  - U3 to Resume			Wakeup signaling from a device
 *  - Resume to Recovery to U0		USB 3.0 device resume
 *  - Resume to U0			USB 2.0 device resume
 *  - U3 to Recovery to U0		Software resume of USB 3.0 device complete
 *  - U3 to U0				Software resume of USB 2.0 device complete
 *  - U2 to U0				L1 resume of USB 2.1 device complete
 *  - U0 to U0 (???)			L1 entry rejection by USB 2.1 device
 *  - U0 to disabled			L1 entry error with USB 2.1 device
 *  - Any state to inactive		Error on USB 3.0 port
 */
#define PORTSC_PLC	(1 << 22)
/* port configure error change - port failed to configure its link partner */
//#define PORT_CEC	(1 << 23)
/* bit 24 reserved */
/* wake on connect (enable) */
#define PORTSC_WCE	(1 << 25)
/* wake on disconnect (enable) */
#define PORTSC_WDE	(1 << 26)
/* wake on over-current (enable) */
#define PORTSC_WOE	(1 << 27)
/* Bit 28:29 reserved */
/* true: device is removable - for USB 3.0 roothub emulation */
#define PORTSC_DR	(1 << 30)
/* Initiate a warm port reset - complete when PORTSC_WRC is '1' */
#define PORTSC_WPR		(1 << 31)

/* We mark duplicate entries with -1 */
//#define DUPLICATE_ENTRY ((u8)(-1))

/* Port Power Management Status and Control - port_power_base bitmasks */
/* Inactivity timer value for transitions into U1, in microseconds.
 * Timeout can be up to 127us.  0xFF means an infinite timeout.
 */
//#define PORT_U1_TIMEOUT(p)	((p) & 0xff)
/* Inactivity timer value for transitions into U2 */
//#define PORT_U2_TIMEOUT(p)	(((p) & 0xff) << 8)
/* Bits 24:31 for port testing */

/* USB2 Protocol PORTSPMSC */
//#define PORT_RWE	(1 << 0x3)


/* --- XHCI Interrupt Registers --- */
struct xhci_intr_reg {
	__le32	u32IMAN;
	__le32	u32IMOD;
	__le32	u32ERSTSZ;
	__le32	u32Reserved;
	__le64	u32ERSTBA;
	__le64	u32ERDP;
};

/* irq_pending bitmasks */
//#define	ER_IRQ_PENDING(p)	((p) & 0x1)
/* Bit 2:31 need to be preserved */
/* THIS IS BUGGY - FIXME - IP IS WRITE 1 TO CLEAR */
#define	IMAN_INT_CLEAR(p)		((p) & 0xfffffffe)
#define	IMAN_INT_ENABLE(p)	((IMAN_INT_CLEAR(p)) | 0x2)
#define	IMAN_INT_DISABLE(p)	((IMAN_INT_CLEAR(p)) & ~(0x2))

/* irq_control bitmasks */
/* Minimum interval between interrupts (in 250ns intervals).  The interval
 * between interrupts will be longer if there are no events on the event ring.
 * Default is 4000 (1 ms).
 */
#define IMOD_INTERVAL_MASK	(0xffff)
/* Counter used to count down the time to the next interrupt - HW use only */
#define IMOD_COUNTER_MASK	(0xffff << 16)

/* erst_size bitmasks */
/* Preserve Bit 16:31 of erst_size */
#define	ERST_SZ_MASK		(0xffff << 16)

/* erst_dequeue bitmasks */
/* Dequeue ERST Segment Index (DESI) - Segment number (or alias)
 * where the current dequeue pointer lies.  This is an optional HW hint.
 */
//#define ERST_DESI_MASK		(0x7)
/* Event Handler Busy (EHB) - is the event ring scheduled to be serviced by
 * a work queue (or delayed service routine)?
 */
#define ERDP_EHB		(1 << 3)
#define ERDP_MASK		(0xf)

/* --- XHCI Runtime Registers --- */
struct xhci_run_regs {
	__le32			u32MfIndex;
	__le32			reserved[7];
	struct xhci_intr_reg	stIrSet[128];
};

/* --- XHCI Doorbell Registers --- */
struct xhci_doorbell_array {
	__le32	u32DoorBellReg[256];
};

#define XHCI_DB_VALUE(ep, stream)	((((ep) + 1) & 0xff) | ((stream) << 16))
#define XHCI_DB_VALUE_HOST		0x00000000

#endif //__XHCI_REG_H
