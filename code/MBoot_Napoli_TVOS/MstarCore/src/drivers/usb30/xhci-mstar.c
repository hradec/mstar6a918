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

#include "xhci.h"
#include "xhci-mstar.h"

//#define MS_DBG
#ifdef MS_DBG
#define ms_dbg(fmt, args...)   do { printf(fmt , ## args); } while (0)
#else
#define ms_dbg(fmt, args...)   do { } while (0)
#endif

#define ms_err(fmt, args...)   do { printf(fmt , ## args); } while (0)

struct xhci_hcd     ms_xhci;
//struct usb_device	*u3dev; //MBOOT_XHCI
unsigned long  gUsbStatusXHC;
int gXHCI_rh_port_sel;

extern int usb_hub_probe(struct usb_device *dev, int ifnum);
extern int ms_usb_bulk_msg_u3(struct usb_device *pDev, unsigned int pipe,
		 void *pBuf, int len, int *pAct_len, int timeout);
extern void usb_bc_enable(u32 utmi_base, u32 bc_base, u8 enable);

#if (XHCI_ENABLE_DEQ)
static void  DEQ_init(unsigned int U3PHY_D_base, unsigned int U3PHY_A_base)
{

    writeb(0x00,   (void*)(U3PHY_A_base+0xAE*2));
    writew(0x080C, (void*)(U3PHY_D_base+0x82*2));
    writeb(0x10,   (void*)(U3PHY_D_base+0xA4*2)); //0x10  0x30
    writew(0x4100, (void*)(U3PHY_D_base+0xA0*2));

    writew(0x06,   (void*)(U3PHY_A_base+0x06*2));

}
#endif

void XHCI_enable_PPC(unsigned int U3TOP_base, u8 enable)
{
	u16 addr_w, bit_num;
	u32 addr;
	u8  value;

	addr_w = readw((void*)(U3TOP_base+0xFC*2));
	addr = (u32)addr_w << 8;
	addr_w = readw((void*)(U3TOP_base+0xFE*2));
	addr |= addr_w & 0xFF;
	bit_num = (addr_w >> 8) & 0x7;

	if (addr)
	{
		printf("XHCI_enable_PPC: Turn %s USB3.0 port power \n", enable? "on":"off");
		printf("Addr: 0x%x bit_num: %d \n", addr, bit_num);

		value = 1 << bit_num;
		if (addr & 0x01)
			addr = addr*2-1;
		else
			addr = addr*2;

		if (enable)
			writeb(readb((void*)(_MSTAR_PM_BASE+addr)) | value, (void*)(_MSTAR_PM_BASE+addr));
		else
			writeb(readb((void*)(_MSTAR_PM_BASE+addr)) & ~value, (void*)(_MSTAR_PM_BASE+addr));
	}

}


void Mstar_xhc_init(unsigned int UTMI_base, unsigned int U3PHY_D_base, unsigned int U3PHY_A_base,
	                   unsigned int XHCI_base, unsigned int U3TOP_base, unsigned int U3BC_base, unsigned int flag)
{
	ms_dbg("Mstar_xhc_init start\n");

	if (0 == readw((void*)(UTMI_base+0x20*2)))
	{
		printf("utmi clk enable\n");
	    writew(0x8051, (void*) (UTMI_base+0x20*2));
	    writew(0x2088, (void*) (UTMI_base+0x22*2));
	    writew(0x0004, (void*) (UTMI_base+0x2*2));
	    writew(0x6BC3, (void*) (UTMI_base));
	    mdelay(1);
	    writew(0x69C3, (void*) (UTMI_base));
	    mdelay(1);
	    writew(0x0001, (void*) (UTMI_base));
	    mdelay(1);
	}

#if (ENABLE_AGATE)
    writeb(0x03, (void*) (UTMI_base+0x8*2));
#else
    writeb(0x07, (void*) (UTMI_base+0x8*2));   //default value 0x7; don't change it.
#endif

    if (flag & EHCFLAG_TESTPKG)
    {
	    writew(0x2084, (void*) (UTMI_base+0x2*2));
	    writew(0x8051, (void*) (UTMI_base+0x20*2));
    }

#if _USB_HS_CUR_DRIVE_DM_ALLWAYS_HIGH_PATCH
	/*
	 * patch for DM always keep high issue
	 * init overwrite register
	 */
	writeb(readb((void*)(UTMI_base+0x0*2)) & ~BIT3, (void*) (UTMI_base+0x0*2)); //DP_PUEN = 0
	writeb(readb((void*)(UTMI_base+0x0*2)) & ~BIT4, (void*) (UTMI_base+0x0*2)); //DM_PUEN = 0

	writeb(readb((void*)(UTMI_base+0x0*2)) & ~BIT5, (void*) (UTMI_base+0x0*2)); //R_PUMODE = 0

	writeb(readb((void*)(UTMI_base+0x0*2)) | BIT6, (void*) (UTMI_base+0x0*2)); //R_DP_PDEN = 1
	writeb(readb((void*)(UTMI_base+0x0*2)) | BIT7, (void*) (UTMI_base+0x0*2)); //R_DM_PDEN = 1

	writeb(readb((void*)(UTMI_base+0x10*2)) | BIT6, (void*) (UTMI_base+0x10*2)); //hs_txser_en_cb = 1
	writeb(readb((void*)(UTMI_base+0x10*2)) & ~BIT7, (void*) (UTMI_base+0x10*2)); //hs_se0_cb = 0

	/* turn on overwrite mode */
	writeb(readb((void*)(UTMI_base+0x0*2)) | BIT1, (void*) (UTMI_base+0x0*2)); //tern_ov = 1
#endif

	writeb(readb((void*)(UTMI_base+0x09*2-1)) & ~0x08, (void*) (UTMI_base+0x09*2-1)); // Disable force_pll_on
	writeb(readb((void*)(UTMI_base+0x08*2)) & ~0x80, (void*) (UTMI_base+0x08*2)); // Enable band-gap current
	writeb(0xC3, (void*)(UTMI_base)); // reg_pdn: bit<15>, bit <2> ref_pdn
	mdelay(1);	// delay 1ms

	writeb(0x69, (void*) (UTMI_base+0x01*2-1)); // Turn on UPLL, reg_pdn: bit<9>
	mdelay(2);	// delay 2ms

	writeb(0x01, (void*) (UTMI_base)); // Turn all (including hs_current) use override mode
	writeb(0, (void*) (UTMI_base+0x01*2-1)); // Turn on UPLL, reg_pdn: bit<9>

	writeb(readb((void*)(UTMI_base+0x3C*2)) | 0x01, (void*) (UTMI_base+0x3C*2)); // set CA_START as 1
	mdelay(10);

	writeb(readb((void*)(UTMI_base+0x3C*2)) & ~0x01, (void*) (UTMI_base+0x3C*2)); // release CA_START

	while ((readb((void*)(UTMI_base+0x3C*2)) & 0x02) == 0);	// polling bit <1> (CA_END)

    if (flag & EHCFLAG_DPDM_SWAP)
    	writeb(readb((void*)(UTMI_base+0x0b*2-1)) |0x20, (void*) (UTMI_base+0x0b*2-1)); // dp dm swap

	writeb((readb((void*)(UTMI_base+0x06*2)) & 0x9F) | 0x40, (void*) (UTMI_base+0x06*2)); //reg_tx_force_hs_current_enable

	writeb(readb((void*)(UTMI_base+0x03*2-1)) | 0x28, (void*) (UTMI_base+0x03*2-1)); //Disconnect window select
	writeb(readb((void*)(UTMI_base+0x03*2-1)) & 0xef, (void*) (UTMI_base+0x03*2-1)); //Disconnect window select

	writeb(readb((void*)(UTMI_base+0x07*2-1)) & 0xfd, (void*) (UTMI_base+0x07*2-1)); //Disable improved CDR
	writeb(readb((void*)(UTMI_base+0x09*2-1)) |0x81, (void*) (UTMI_base+0x09*2-1)); // UTMI RX anti-dead-loc, ISI effect improvement
	writeb(readb((void*)(UTMI_base+0x0b*2-1)) |0x80, (void*) (UTMI_base+0x0b*2-1)); // TX timing select latch path
	writeb(readb((void*)(UTMI_base+0x15*2-1)) |0x20, (void*) (UTMI_base+0x15*2-1)); // Chirp signal source select

	//Enable XHCI preamble function
	writeb(readb((void*)(UTMI_base+0x3F*2-1)) |0x80, (void*) (UTMI_base+0x3F*2-1)); // Enable XHCI preamble function

#if (!ENABLE_AGATE)  //exclude Agate
	writeb(readb((void*)(UTMI_base+0x08*2)) | 0x18, (void*) (UTMI_base+0x08*2)); // for 240's phase as 120's clock
	#if (ENABLE_EIFFEL)
		//Fix it for Eiffel Only.
		writeb(readb((void*)(UTMI_base+0x08*2)) & 0xF7, (void*) (UTMI_base+0x08*2)); // Clear setting of "240's phase as 120's clock"
	#endif
#endif

	// change to 55 timing; for all chips.
	writeb(readb((void*)(UTMI_base+0x15*2-1)) |0x40, (void*) (UTMI_base+0x15*2-1)); // change to 55 timing

    // for CLK 120 override enable; for xHCI on all chips
	writeb(readb((void*)(UTMI_base+0x09*2-1)) |0x04, (void*) (UTMI_base+0x09*2-1)); // for CLK 120 override enable

	writeb(XHC_UTMI_EYE_2C, (void*) (UTMI_base+0x2c*2));
	writeb(XHC_UTMI_EYE_2D, (void*) (UTMI_base+0x2d*2-1));
	writeb(XHC_UTMI_EYE_2E, (void*) (UTMI_base+0x2e*2));
	writeb(XHC_UTMI_EYE_2F, (void*) (UTMI_base+0x2f*2-1));

#if (ENABLE_KAISER)
	writeb(readb((void*)(UTMI_base+0x10*2)) |0x40, (void*) (UTMI_base+0x10*2));  //TXSER_EN override value
#endif

#if defined(ENABLE_LS_CROSS_POINT_ECO)
	writeb(readb((void*)(UTMI_base+0x04*2)) | 0x40, (void*) (UTMI_base+0x04*2));  //enable deglitch SE0；(low-speed cross point)
#endif

#if defined(ENABLE_TX_RX_RESET_CLK_GATING_ECO)
	writeb(readb((void*)(UTMI_base+0x04*2)) | 0x20, (void*) (UTMI_base+0x04*2)); //enable hw auto deassert sw reset(tx/rx reset)
#endif

#if defined(ENABLE_KEEPALIVE_ECO)
	writeb(readb((void*)(UTMI_base+0x04*2)) | 0x80, (void*) (UTMI_base+0x04*2));    //enable LS keep alive & preamble
#endif

#if defined(ENABLE_HS_DM_KEEP_HIGH_ECO)
	/* Change override to hs_txser_en.  Dm always keep high issue */
	writeb(readb((void*)(UTMI_base+0x10*2)) | BIT6, (void*) (UTMI_base+0x10*2));
#endif

#if _USB_HS_CUR_DRIVE_DM_ALLWAYS_HIGH_PATCH
	/*
	 * patch for DM always keep high issue
	 * init overwrite register
	 */
	writeb(readb((void*)(UTMI_base+0x0*2)) | BIT6, (void*) (UTMI_base+0x0*2)); //R_DP_PDEN = 1
	writeb(readb((void*)(UTMI_base+0x0*2)) | BIT7, (void*) (UTMI_base+0x0*2)); //R_DM_PDEN = 1

	/* turn on overwrite mode */
	writeb(readb((void*)(UTMI_base+0x0*2)) | BIT1, (void*) (UTMI_base+0x0*2)); //tern_ov = 1
#endif

    if (flag & EHCFLAG_TESTPKG)
    {
	    writew(0x0600, (void*) (UTMI_base+0x14*2));
	    writew(0x0038, (void*) (UTMI_base+0x10*2));
	    writew(0x0BFE, (void*) (UTMI_base+0x32*2));
    }

    //U3phy initial sequence
    writew(0x0,    (void*) (U3PHY_A_base));          // power on rx atop
    writew(0x0,    (void*) (U3PHY_A_base+0x2*2));    // power on tx atop
    writew(0x0910, (void*) (U3PHY_D_base+0x4*2));
    writew(0x0,    (void*) (U3PHY_A_base+0x3A*2));
    writew(0x0160, (void*) (U3PHY_D_base+0x18*2));
    writew(0x0,    (void*) (U3PHY_D_base+0x20*2));   // power on u3_phy clockgen
    writew(0x0,    (void*) (U3PHY_D_base+0x22*2));   // power on u3_phy clockgen

    writew(0x013F, (void*) (U3PHY_D_base+0x4A*2));
    writew(0x1010, (void*) (U3PHY_D_base+0x4C*2));

    writew(0x0,    (void*) (U3PHY_A_base+0x3A*2));   // override PD control
    writeb(0x1C,   (void*) (U3PHY_A_base+0xCD*2-1)); // reg_test_usb3aeq_acc;  long EQ converge
    writeb(0x40,   (void*) (U3PHY_A_base+0xC9*2-1)); // reg_gcr_usb3aeq_threshold_abs
    writeb(0x10,   (void*) (U3PHY_A_base+0xE5*2-1)); // [4]: AEQ select PD no-delay and 2elay path, 0: delay, 1: no-
    writeb(0x11,   (void*) (U3PHY_A_base+0xC6*2));   // analog symbol lock and EQ converage step
    writeb(0x02,   (void*) (U3PHY_D_base+0xA0*2));   // [1] aeq mode

    writeb(0x07,   (void*) (U3PHY_A_base+0xB0*2));   // reg_gcr_usb3rx_eq_str_ov_value

#if (ENABLE_XHCI_SSC)
	writew(0x04D8,  (void*) (U3PHY_D_base+0xC6*2));  //reg_tx_synth_span
	writew(0x0003,  (void*) (U3PHY_D_base+0xC4*2));  //reg_tx_synth_step
	writew(0x9375,  (void*) (U3PHY_D_base+0xC0*2));  //reg_tx_synth_set
	writeb(0x18,    (void*) (U3PHY_D_base+0xC2*2));  //reg_tx_synth_set
#endif

    ////Set Tolerance  //only for Agate_U01
    /// writew(0x0103, (void*) (U3PHY_D_base+0x44*2));

    // Comma
    // writeb(0x84,   (void*) (U3PHY_A_base+0xCD*2-1)); // reg_test_aeq_acc, 8bit

	// RX phase control
    writew(0x0100, (void*)(U3PHY_A_base+0x90*2));
    writew(0x0302, (void*)(U3PHY_A_base+0x92*2));
    writew(0x0504, (void*)(U3PHY_A_base+0x94*2));
    writew(0x0706, (void*)(U3PHY_A_base+0x96*2));
    writew(0x1708, (void*)(U3PHY_A_base+0x98*2));
    writew(0x1516, (void*)(U3PHY_A_base+0x9A*2));
    writew(0x1314, (void*)(U3PHY_A_base+0x9C*2));
    writew(0x1112, (void*)(U3PHY_A_base+0x9E*2));
    writew(0x3000, (void*)(U3PHY_D_base+0xA8*2));
	writew(0x7380, (void*)(U3PHY_A_base+0x40*2));

	#if (XHCI_ENABLE_DEQ)
    DEQ_init(U3PHY_D_base, U3PHY_A_base);
	#endif

	//First token idle
	writeb(readb((void*)(XHCI_base+0x4308)) | 0x0C, (void*)(XHCI_base+0x4308));  //First token idle (bit2~3 = "11")
	//Inter packet delay
	writeb(readb((void*)(XHCI_base+0x430F)) | 0xC0, (void*)(XHCI_base+0x430F));  //Inter packet delay (bit6~7 = "11")
	//LFPS patch
	writeb(readb((void*)(XHCI_base+0x681A)) | 0x10, (void*)(XHCI_base+0x681A));  //LFPS patch  (bit4 = 1)

	//Bus Reset setting => default 50ms
#if (ENABLE_AGATE)
	writeb((readb((void*)(U3TOP_base+0xFC*2)) & 0xF0) | 0x8, (void*)(U3TOP_base+0xFC*2));    // T1=30; T2=20
#else
	writeb((readb((void*)(U3TOP_base+0x24*2)) & 0xF0) | 0x8, (void*)(U3TOP_base+0x24*2));    // [5] = reg_debug_mask to 1'b0
#endif


#if !( (ENABLE_AGATE)   || \
        (ENABLE_EDISON)  || \
	    (ENABLE_KAISER)  )
	writeb(readb((void*)(U3TOP_base+0x12*2)) | 0x8, (void*)(U3TOP_base+0x12*2));  //check both last_down_z & data count enable
#endif
#if defined(ENABLE_NIKE) || defined(ENABLE_MADISON) || defined(ENABLE_CLIPPERS) || defined(ENABLE_NADAL) || (ENABLE_MIAMI)
           writeb(readb((void*)(U3TOP_base+0x11*2-1)) | 0x2, (void*)(U3TOP_base+0x11*2-1));  //set reg_miu1_sel to check address bit 30
#endif
	#if (XHCI_CHIRP_PATCH)
		#if (!ENABLE_AGATE)  //exlcude Agate.
		writeb(0x0, (void*) (UTMI_base+0x3E*2)); //override value
		writeb((readb((void*)(U3TOP_base+0x24*2)) & 0xF0) | 0x0A, (void*)(U3TOP_base+0x24*2)); // set T1=50, T2=20
		writeb(readb((void*)(UTMI_base+0x3F*2-1)) | 0x1, (void*) (UTMI_base+0x3F*2-1)); //enable the patch
		#endif
	#endif

	#if (XHCI_TX_SWING_PATCH)
    writeb(0x3F, (void*)(U3PHY_A_base+0x60*2));
    writeb(0x39, (void*)(U3PHY_A_base+0x62*2));
	#endif

	#if defined (XHCI_ENABLE_LOOPBACK_ECO)
		#if (ENABLE_EDISON)  || \
            (ENABLE_KAISER)
		//Only for Edison and Kaiser
		writeb(readb((void*)(U3TOP_base+0xFF*2-1))|0x60 , (void*)(U3TOP_base+0xFF*2-1));
		#elif (ENABLE_EIFFEL) || \
			  (ENABLE_EINSTEIN)
		//Only for Eiffel and Einstein
		writeb(readb((void*)(U3TOP_base+0x20*2))|0x30 , (void*)(U3TOP_base+0x20*2));
		#endif
	#endif

	#if (XHCI_CURRENT_SHARE_PATCH)
	// change LDO to internal R-array divider, not see VBG voltage.  GCR_USB3TX_REG_VSEL[2] = 1
    writeb(readb((void*)(U3PHY_A_base+0x11*2-1)) | 0x4, (void*)(U3PHY_A_base+0x11*2-1));
	// Change EQ/LA current to 2X. (even 2X, the current is still 0.6X of original.)
    // TEST_USB3RX_IBIAS[5:0] = 6・h2A
//    writeb(0x2A, (void*)(U3PHY_A_base+0xE*2));
    writeb(0x3F, (void*)(U3PHY_A_base+0xE*2));  //Erik
	// Disable rxpll vringcmp
	// TEST_USB3RXPLL[47:40] = 8・h0
    writeb(0x0, (void*)(U3PHY_A_base+0x25*2-1));

	//Change TX current to 0.75X
    writeb(0x30, (void*)(U3PHY_A_base+0x60*2));

	//patch for Edison U01 current sharing issue
	writeb((readb((void*)(UTMI_base+0x2D*2-1))&0x87) | 0x28, (void*)(UTMI_base+0x2D*2-1)); // HS_TX_TEST[14:11]=4b'0101   usb3_utmi+0x2d[6:3] =4・b0101
	writeb(readb((void*)(U3BC_base+0x2*2)) | 0x8, (void*)(U3BC_base+0x2*2)); // EN_BC_VDP_DATREF=1b'1  ->  usb3_usbbc+0x02[3] = 1・b1
    writeb((readb((void*)(UTMI_base+0x2B*2-1))&0xFC) | 0x2, (void*)(UTMI_base+0x2B*2-1));  // REF_TEST[9:8] =10  ( TX:25.4u   RX:20.4u)   usb3_utmi+0x2b[1:0] =2・b10
	#endif

	#if (XHCI_ENABLE_TESTBUS)
	XHCI_enable_testbus((_MSTAR_USB_BASEADR+(0x1E00*2)), U3PHY_D_base, U3TOP_base, XHCI_base);
	#endif


	#ifdef XHCI_ENABLE_PPC
	XHCI_enable_PPC(U3TOP_base, true);
	#endif

}

/* called during probe() after chip reset completes */
int ms_xhci_setup(struct xhci_hcd *xhci)
{
	int			retval;

	//hcd->self.sg_tablesize = MAX_TRBS_IN_SEG - 2;

	xhci->cap_regs = xhci->regs;
	//XHCI_Dbg("xhci cap_regs: 0x%x\n", xhci->cap_regs);
	xhci->op_regs = xhci->regs +
		CAP_REGS_LENGTH(xhci_readl(xhci, &xhci->cap_regs->caplen_hciver));
	//XHCI_Dbg("xhci op_regs: 0x%x\n", xhci->op_regs);
	xhci->run_regs = xhci->regs +
		(xhci_readl(xhci, &xhci->cap_regs->u32RtsOff) & RTSOFF_RSVD_MASK);
	//XHCI_Dbg("xhci run_regs: 0x%x\n", xhci->run_regs);
	/* Cache read-only capability registers */
#if 0 //MBOOT_XHCI
	xhci->hcs_params1 = xhci_readl(xhci, &xhci->cap_regs->u32HcsParams1);
	xhci->hcs_params2 = xhci_readl(xhci, &xhci->cap_regs->u32HcsParams2);
	xhci->hcs_params3 = xhci_readl(xhci, &xhci->cap_regs->u32HcsParams3);
	xhci->hcc_params = xhci_readl(xhci, &xhci->cap_regs->caplen_hciver);
	xhci->hci_version = HC_VERSION(xhci->hcc_params);
#endif
	xhci->hci_version = 0x96;  //Jonas modified for real HW version.
	xhci->hcc_params = xhci_readl(xhci, &xhci->cap_regs->u32HccParams);
	ms_xhci_dump_regs(xhci);

	/* Make sure the HC is halted. */
	retval = ms_xhci_halt(xhci);
	if (retval)
		goto error;

	ms_dbg("Resetting HCD\n");
	/* Reset the internal HC memory state and registers. */
	retval = ms_xhci_reset(xhci);
	if (retval)
		goto error;
	ms_dbg("Reset complete\n");

	ms_dbg("Calling HCD init\n");
	/* Initialize HCD and host controller data structures. */
	retval = ms_xhci_init(xhci);
	if (retval) {
		ms_err("%s ms_xhci_init fail (err=%d)\n", __func__, retval);
	}

error:
	return retval;
}

int xhci_hcd_mstar_probe(struct xhci_hcd *xhci)
{
	int retval=0;
	//struct xhci_hcd *xhci;
	unsigned int flag=0;

	xhci->xhci_base = _MSTAR_XHCI_BASE;
	xhci->u3phy_d_base = _MSTAR_U3PHY_DTOP_BASE;
	xhci->u3phy_a_base = _MSTAR_U3PHY_ATOP_BASE;
	xhci->u3top_base = _MSTAR_U3TOP_BASE;
	//xhci->u3indctl_base = _MSTAR_U3INDCTL_BASE;
	xhci->utmi_base = _MSTAR_U3UTMI_BASE;

	ms_dbg("Mstar-xhci-1 H.W init\n");
	Mstar_xhc_init(xhci->utmi_base, xhci->u3phy_d_base, xhci->u3phy_a_base,
		           xhci->xhci_base, xhci->u3top_base, _MSTAR_U3BC_BASE, flag);

	xhci->regs = (void *)(u32)xhci->xhci_base;

	usb_bc_enable(xhci->utmi_base, _MSTAR_U3BC_BASE, false);

	retval = ms_xhci_setup(xhci);
	if (retval)
	{
		printf("ms_xhci_setup fail: %d\n", -retval);
		return retval;
	}

	retval = ms_xhci_run(xhci);
	if (retval)
	{
		printf("ms_xhci_run fail: %d\n", -retval);
		return retval;
	}

	return retval;
}

extern int UsbPortSelect;
extern u32 g_u3_chk_conn_time;
int USB3_init(void)
{
	int ret, i, port;

	UsbPortSelect=4;
	memset(&ms_xhci, 0, sizeof(struct xhci_hcd));

	ret=xhci_hcd_mstar_probe(&ms_xhci);
	if (ret < 0 )
		return ret;
	//USB3_enumerate();

	for (i=0 ; i <= g_u3_chk_conn_time; i++)
	{
		port = ms_xhci_port_connect(&ms_xhci);
		if (port)
			break;

		if (i < g_u3_chk_conn_time)
			mdelay(1000);
	}

	if (port == 0)
	{
		printf("No USB device found. \n");
		return -1;
	}
	else
	{
		printf("usb connect at %d loop\n",i);
	}

	return 0;
}
extern struct usb_device *usb_alloc_new_device(void);

extern char usb_started;
int USB3_enumerate(struct usb_device *pdev)
{
	u32 __iomem *addr;
	u32 temp;
	int retval,i;

	usb_started = 0;

	for (gXHCI_rh_port_sel=0 ; gXHCI_rh_port_sel < 4 ; gXHCI_rh_port_sel++)
	{
		addr = &ms_xhci.op_regs->u32PortSc + PORT_REGS_NUM*gXHCI_rh_port_sel;

		temp = xhci_readl(&ms_xhci, addr);

		if ((temp & PORTSC_CCS))
		{
			printf("\n\nXHCI port %d is connecting...\n", gXHCI_rh_port_sel);
			//memset(pdev, 0, sizeof(struct usb_device));
			retval = ms_xhci_dev_enum(&ms_xhci, pdev, gXHCI_rh_port_sel);
			usb_started = 1;

			if (retval == 0)
			{
				if (pdev->config.if_desc[0].desc.bInterfaceClass != USB_CLASS_HUB)
				{
					ms_dbg("Found a device, stop the enumeration\n");
					break; //Found a device, stop the enumeration
				}
				else
				{
					usb_hub_probe(pdev, 0);
					for (i=0; i<USB_MAXCHILDREN; i++)
					{
						if (pdev->children[i])
						{
							ms_dbg("Found a device under the hub, stop the enumeration\n");
							break;  //Found a device under the hub, stop the enumeration
						}
					}

					if (i<USB_MAXCHILDREN)
						break;
				}
			}
		}
	}

	return 0;
}

unsigned int Send_Receive_Bulk_Data_XHC(struct usb_device *dev, void *buffer, int len, int dir_out)
{
	int actual;
	unsigned int pipe;
	int eptin_addr, eptout_addr;

	if (dev->bulk1.desc.bEndpointAddress & USB_DIR_IN)
	{
		eptin_addr = dev->bulk1.desc.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
		eptout_addr = dev->bulk2.desc.bEndpointAddress;
	}
	else
	{
		eptin_addr = dev->bulk2.desc.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
		eptout_addr = dev->bulk1.desc.bEndpointAddress;
	}

	if (dir_out)
		pipe = usb_sndbulkpipe(dev, eptout_addr);
	else
		pipe = usb_rcvbulkpipe(dev, eptin_addr);

	/*printf("Send_Receive_Bulk_Data_XHC, eptin_addr: %d, eptout_addr: %d\n",
			eptin_addr, eptout_addr);*/
	if (ms_usb_bulk_msg_u3(dev, pipe, buffer, len, &actual, MS_BULK_TIMEOUT))
		return 1;
	else
		return 0;
}

extern void ms_usb_free_config(struct usb_device *dev);
void MDrv_UsbCloseXHCI(void)
{
	int i;
	struct usb_device *pUDev;

	if (ms_xhci.xhci_base != _MSTAR_XHCI_BASE) //XHCI never be initialed
		return;

	ms_xhci_halt(&ms_xhci);

	for (i=0; i<=XHCI_MAX_DEV; i++)
	{
		pUDev = usb_get_dev_index(i);
		if (pUDev)
			ms_usb_free_config(pUDev);
	}

	for (i=1; i<=XHCI_MAX_DEV; i++)
		ms_xhci_free_virt_dev(&ms_xhci, i);

	ms_xhci_ring_free(ms_xhci.cmd_ring);
	ms_xhci_ring_free(ms_xhci.event_ring);
}

void MDrv_UsbEnableXhciPower(u8 enable)
{
#ifdef XHCI_ENABLE_PPC
	XHCI_enable_PPC(_MSTAR_U3TOP_BASE, enable);
#endif
}

