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

#ifndef _XHCI_MSTAR_H
#define _XHCI_MSTAR_H

// ----- Don't modify it !----------
#define _USB_T3_WBTIMEOUT_PATCH 1
#define XHCI_PA_PATCH         1
#define XHCI_FLUSHPIPE_PATCH  1
//------------------------------

#define XHCI_CHIRP_PATCH  1
#define ENABLE_XHCI_SSC   1
#define XHCI_TX_SWING_PATCH  1

//------ for test -----------------
#define XHCI_CURRENT_SHARE_PATCH 0   //Only for USB3; will cause USB2 chirp handshake fail.
#define XHCI_ENABLE_DEQ   0
//--------------------------------
//Inter packet delay setting for all chips
#define XHCI_IPACKET_DELAY_PATCH

#if (!ENABLE_AGATE)  //exclude Agate.
#define XHCI_ENABLE_PPC
#endif

#define TEMP_DISABLE_FOR_DEVELOP


#if (ENABLE_AGATE)
#define ENABLE_KEEPALIVE_ECO
#endif


#if defined(__ARM__)
#define HW_BASE         0x1f200000  //0x1f200000   0xfd200000
#else
#define HW_BASE         0xbf200000
#endif

#define _MSTAR_USB_BASEADR    HW_BASE

#if defined(CONFIG_ARM)
#define _MSTAR_PM_BASE         0x1F000000
#else
#define _MSTAR_PM_BASE         0xBF000000
#endif

#if (ENABLE_EDISON)  || (ENABLE_AGATE) || (ENABLE_NIKE) || \
	(ENABLE_MADISON) || (ENABLE_NADAL) || (ENABLE_MIAMI)
#define _MSTAR_U3PHY_DTOP_BASE (_MSTAR_USB_BASEADR+(0x22100*2))
#define _MSTAR_U3PHY_ATOP_BASE (_MSTAR_USB_BASEADR+(0x22200*2))
#define _MSTAR_U3UTMI_BASE     (_MSTAR_USB_BASEADR+(0x22300*2))
//#define _MSTAR_U3INDCTL_BASE   (_MSTAR_USB_BASEADR+(0x22400*2))
#define _MSTAR_U3TOP_BASE      (_MSTAR_USB_BASEADR+(0x22500*2))
#define _MSTAR_XHCI_BASE       (_MSTAR_USB_BASEADR+(0x90000*2))
#define _MSTAR_U3BC_BASE       (_MSTAR_USB_BASEADR+(0x23660*2))
#endif

#if (ENABLE_EIFFEL)
#define _MSTAR_U3PHY_DTOP_BASE (_MSTAR_USB_BASEADR+(0x22100*2))
#define _MSTAR_U3PHY_ATOP_BASE (_MSTAR_USB_BASEADR+(0x22200*2))
#define _MSTAR_U3UTMI_BASE     (_MSTAR_USB_BASEADR+(0x22300*2))
#define _MSTAR_U3TOP_BASE      (_MSTAR_USB_BASEADR+(0x22500*2))
#define _MSTAR_XHCI_BASE       (_MSTAR_USB_BASEADR+(0x90000*2))
#define _MSTAR_U3BC_BASE       (_MSTAR_USB_BASEADR+(0x236C0*2))
#endif

#if (ENABLE_KAISER)
#define _MSTAR_U3PHY_DTOP_BASE (_MSTAR_USB_BASEADR+(0x22C00*2))
#define _MSTAR_U3PHY_ATOP_BASE (_MSTAR_USB_BASEADR+(0x22D00*2))
#define _MSTAR_U3UTMI_BASE     (_MSTAR_USB_BASEADR+(0x22B00*2))
#define _MSTAR_U3TOP_BASE      (_MSTAR_USB_BASEADR+(0x22000*2))
#define _MSTAR_XHCI_BASE       (_MSTAR_USB_BASEADR+(0x90000*2))
#define _MSTAR_U3BC_BASE       (_MSTAR_USB_BASEADR+(0x22FC0*2))
#endif

//------ UTMI eye diagram parameters --------
#if (ENABLE_AGATE)
	// for 40nm
	#define XHC_UTMI_EYE_2C	(0x98)
	#define XHC_UTMI_EYE_2D	(0x02)
	#define XHC_UTMI_EYE_2E	(0x10)
	#define XHC_UTMI_EYE_2F	(0x01)
#elif (ENABLE_EDISON) || (ENABLE_KAISER) || (ENABLE_EIFFEL)
	// for 40nm after Agate, use 55nm setting2
	#define XHC_UTMI_EYE_2C	(0x90)
	#define XHC_UTMI_EYE_2D	(0x02)
	#define XHC_UTMI_EYE_2E	(0x00)
	#define XHC_UTMI_EYE_2F	(0x81)
#else
	// for 40nm after Agate, use 55nm setting1, the default
	#define XHC_UTMI_EYE_2C	(0x10)
	#define XHC_UTMI_EYE_2D	(0x02)
	#define XHC_UTMI_EYE_2E	(0x00)
	#define XHC_UTMI_EYE_2F	(0x81)
#endif

//-----------------------------------------
// Mstar_xhc_init flag:
#define EHCFLAG_NONE         0
#define EHCFLAG_DPDM_SWAP    1
#define EHCFLAG_TESTPKG      2
//-----------------------------------------


#define port_is_superspeed(x)  (x>1)  //for Agate



#define in_interrupt()  (0)

#define XHCI_MAX_DEV	4
#define XHCI_MAX_BULK_NUM  3

#endif	/* _XHCI_MSTAR_H */

