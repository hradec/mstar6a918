/*
 * xHCI host controller driver
 *
 * Copyright (C) 2012 MStar Inc.
 *
 * Date: May 2012
 */

#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include "xhci.h"
#include "xhci-mstar.h"

#include <linux/kthread.h>

static const char hcd_name[] = "mstar_xhci_hcd";
#include "../core/bc-mstar.h"		

void  DEQ_init(unsigned int U3PHY_D_base, unsigned int U3PHY_A_base)
{

    writeb(0x00,   (void*)(U3PHY_A_base+0xAE*2));   
    writew(0x080C, (void*)(U3PHY_D_base+0x82*2));   
    writeb(0x10,   (void*)(U3PHY_D_base+0xA4*2)); //0x10  0x30  
    writew(0x4100, (void*)(U3PHY_D_base+0xA0*2));   
	
    writew(0x06,   (void*)(U3PHY_A_base+0x06*2));   

}

void  XHCI_enable_testbus(unsigned int CHIPTOP_base, unsigned int U3TOP_base, unsigned int XHCI_base)
{
	
    writeb(0x00,   (void*)(CHIPTOP_base+0xEC*2));   
    writeb(0x20,   (void*)(CHIPTOP_base+0x24*2));   
    writeb(0x42,   (void*)(CHIPTOP_base+0xEE*2));   // select usb30_test_out
    // writeb(0x00,   (void*)(CHIPTOP_base+0xEA*2));   
    writeb(0x40,   (void*)(CHIPTOP_base+0xEB*2-1));   
    //writew(0xFFFF, (void*)(CHIPTOP_base+0x12*2));   
    //writew(0xFFFF, (void*)(CHIPTOP_base+0x14*2));   
    //writeb(0xFF,   (void*)(CHIPTOP_base+0x16*2));   
    //writeb(0x01,   (void*)(CHIPTOP_base+0x21*2-1));   
    //writeb(0x02,   (void*)(CHIPTOP_base+0x3A*2));   
	
 //   writeb(0x0d,   (void*)(U3PHY_D_base+0xD5*2-1));   
 //   writeb(0x23,   (void*)(U3PHY_D_base+0x2E*2));   
 //   writeb(0x40,   (void*)(U3PHY_D_base+0x2F*2-1));   

	 //writeb(0x09,   (void*)(U3TOP_base+0x0C*2)); 	// [5] = reg_debug_mask to 1'b0
	                                                // [4:0] = 0x06= rrdy & wrdy
	writeb(0x00,   (void*)(U3TOP_base+0x0C*2));

	writeb(0x11,    (void*) (XHCI_base+0x608c));   
    writeb(0x30,    (void*) (XHCI_base+0x608f));
    writeb(0x39,    (void*) (XHCI_base+0x6805));  
    writeb(0x3a,    (void*) (XHCI_base+0x6806));  
    writeb(0x21,    (void*) (XHCI_base+0x6807));  

	


}

void XHCI_enable_PPC(unsigned int U3TOP_base)
{
	u16 addr_w, bit_num;
	u32 addr;
	u8  value, low_active;

	addr_w = readw((void*)(U3TOP_base+0xFC*2));
	addr = (u32)addr_w << 8;
	addr_w = readw((void*)(U3TOP_base+0xFE*2));
	addr |= addr_w & 0xFF;
	bit_num = (addr_w >> 8) & 0x7;
	low_active = (u8)((addr_w >> 8) & 0x8);
	
	if (addr)
	{
		printk("XHCI_enable_PPC: Turn on USB3.0 port power \n");
		printk("Addr: 0x%x bit_num: %d low_active:%d\n", addr, bit_num, low_active);

		value = 1 << bit_num;

		if (low_active)
		{
			if (addr & 0x1)
				writeb(readb((void*)(_MSTAR_PM_BASE+addr*2-1)) & (u8)(~value), (void*)(_MSTAR_PM_BASE+addr*2-1)); 
			else
				writeb(readb((void*)(_MSTAR_PM_BASE+addr*2)) & (u8)(~value), (void*)(_MSTAR_PM_BASE+addr*2)); 
		}
		else
		{
			if (addr & 0x1)
				writeb(readb((void*)(_MSTAR_PM_BASE+addr*2-1)) | value, (void*)(_MSTAR_PM_BASE+addr*2-1)); 
			else
				writeb(readb((void*)(_MSTAR_PM_BASE+addr*2)) | value, (void*)(_MSTAR_PM_BASE+addr*2)); 
		}
	}

}


void Mstar_U3phy_init(unsigned int U3PHY_D_base,unsigned int U3PHY_A_base)
{

	//U3phy initial sequence 
	writew(0x0,    (void*) (U3PHY_A_base)); 		 // power on rx atop 
	writew(0x0,    (void*) (U3PHY_A_base+0x2*2));	 // power on tx atop
	writew(0x0910, (void*) (U3PHY_D_base+0x4*2));  
	writew(0x0,    (void*) (U3PHY_A_base+0x3A*2)); 
	writew(0x0160, (void*) (U3PHY_D_base+0x18*2)); 
	writew(0x0,    (void*) (U3PHY_D_base+0x20*2));	 // power on u3_phy clockgen 
	writew(0x0,    (void*) (U3PHY_D_base+0x22*2));	 // power on u3_phy clockgen 

	writew(0x013F, (void*) (U3PHY_D_base+0x4A*2)); 
	writew(0x1010, (void*) (U3PHY_D_base+0x4C*2)); 

	writew(0x0,    (void*) (U3PHY_A_base+0x3A*2));	 // override PD control
	writeb(0x1C,   (void*) (U3PHY_A_base+0xCD*2-1)); // reg_test_usb3aeq_acc;  long EQ converge 
	writeb(0x40,   (void*) (U3PHY_A_base+0xC9*2-1)); // reg_gcr_usb3aeq_threshold_abs
	writeb(0x10,   (void*) (U3PHY_A_base+0xE5*2-1)); // [4]: AEQ select PD no-delay and 2elay path, 0: delay, 1: no-
	writeb(0x11,   (void*) (U3PHY_A_base+0xC6*2));	 // analog symbol lock and EQ converage step 
	writeb(0x02,   (void*) (U3PHY_D_base+0xA0*2));	 // [1] aeq mode

	writeb(0x07,   (void*) (U3PHY_A_base+0xB0*2));	 // reg_gcr_usb3rx_eq_str_ov_value

	#if (ENABLE_XHCI_SSC)  
		writew(0x04D8,	(void*) (U3PHY_D_base+0xC6*2));  //reg_tx_synth_span
		writew(0x0003,	(void*) (U3PHY_D_base+0xC4*2));  //reg_tx_synth_step
		writew(0x9375,	(void*) (U3PHY_D_base+0xC0*2));  //reg_tx_synth_set
		writeb(0x18,	(void*) (U3PHY_D_base+0xC2*2));  //reg_tx_synth_set
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

	#if (XHCI_TX_SWING_PATCH)
		writeb(0x3F, (void*)(U3PHY_A_base+0x60*2)); 
		writeb(0x39, (void*)(U3PHY_A_base+0x62*2)); 
	#endif 

}

#if defined(XHCI_PHY_EFUSE) && defined(XHCI_PHY_MT28) 
void Mstar_U3phy_MT28_eFuse(unsigned int U3PHY_D_M1_base)
{
	u16 i, value=0;	
	unsigned long	flags;
	spinlock_t	efuse_lock=__SPIN_LOCK_UNLOCKED(efuse_lock);

	#if (XHC_EFUSE_FSM == 1)
		spin_lock_irqsave (&efuse_lock, flags);
		writew(XHC_EFUSE_OFFSET,  (void*)(_MSTAR_EFUSE_BASE+0x4E*2));  //bit[8]=0
		writeb(readb((void*)(_MSTAR_EFUSE_BASE+0x4C*2)) | 0x1,  (void*)(_MSTAR_EFUSE_BASE+0x4C*2));  //bit[0]=1
		for (i=0;  i<100; i++) {
			if ((readb((void*)(_MSTAR_EFUSE_BASE+0x4C*2)) & 0x1) == 0)
				break;
			mdelay(1);
		}
		if (100==i) {
			printk(" !!! WARNING: read eFuse timeout !!!\n");
			return;
		}
		value = readw((void*)(_MSTAR_EFUSE_BASE+0x80*2));
		spin_unlock_irqrestore (&efuse_lock, flags);
	#else
		printk(" !!! ERROR: XHC_EFUSE_FSM0 not implement !!!\n");
		while(1);
	#endif

	if (value & 0x8000) {
		printk("\n [ XHC: get efuse value: 0x%x ] \n", value);
		writew(0x3FF,  (void*)(U3PHY_D_M1_base+0x80*2));
		writew(value&0x3FF,  (void*)(U3PHY_D_M1_base+0x82*2));
	}

}
#endif

void Mstar_U3phy_MT28_init(unsigned int U3PHY_D_M0_base, unsigned int U3PHY_D_M1_base, 
	                       unsigned int U3PHY_A_M0_base, unsigned int U3PHY_A_M1_base)
{
	u16 value;
	//u8  u8value;

	//DA_SSUSB_TX_BIASI_B
	value = (readw((void*)(U3PHY_D_M1_base+0xA*2)) & (u16)(~0x0E00)) | 0x2000;
	writew(value,  (void*)(U3PHY_D_M1_base+0xA*2));
	//DA_SSUSB_idem_6db_b_olt
	value = (readw((void*)(U3PHY_D_M1_base+0x8*2)) & (u16)(~0x003F));
	value |= (0x100|0x24);	   //(0x100|0x18);
	writew(value,  (void*)(U3PHY_D_M1_base+0x8*2));
	//DA_SSUSB_IDRV_6DB_B_olt
	value = (readw((void*)(U3PHY_D_M1_base+0x6*2)) & (u16)(~0x3F00));
	value |= (0x4000|0x3600);  //(0x4000|0x2400);
	writew(value,  (void*)(U3PHY_D_M1_base+0x6*2));	
	//DA_SSUSB_IDEM_3P5db_B_olt	
	value = (readw((void*)(U3PHY_D_M1_base+0x6*2)) & (u16)(~0x003F));
	value |= (0x40|0x18);      //(0x40|0x10);
	writew(value,  (void*)(U3PHY_D_M1_base+0x6*2));
	//DA_SSUSB_IDRV_3P5db_b_olt
	value = (readw((void*)(U3PHY_D_M1_base+0x4*2)) & (u16)(~0x3F00));
	value |= (0x4000|0x3C00);  //(0x4000|0x2800);
	writew(value,  (void*)(U3PHY_D_M1_base+0x4*2));
	//DA_SSUSB_IDRV_0DB_b_olt
	value = (readw((void*)(U3PHY_D_M1_base+0x4*2)) & (u16)(~0x003F));
	value |= (0x40|0x3F);      //(0x40|0x30);
	writew(value,  (void*)(U3PHY_D_M1_base+0x4*2));

	//reg_ssusb_sigdet
	value = (readw((void*)(U3PHY_D_M0_base+0x8E*2)) & (u16)(~0x7F00));
	value |= (0x500);
	writew(value,  (void*)(U3PHY_D_M0_base+0x8E*2));

	#if defined(XHCI_PHY_EFUSE)
		Mstar_U3phy_MT28_eFuse(U3PHY_D_M1_base);	
	#endif
	
}


void Mstar_xhc_init(unsigned int UTMI_base, unsigned int XHCI_base, 
	unsigned int U3TOP_base, unsigned int U3BC_base, unsigned int flag)
{
	printk("Mstar_xhc_init version:%s\n", XHCI_MSTAR_VERSION);

	if (0 == readw((void*)(UTMI_base+0x20*2)))
	{
		printk("utmi clk enable\n");
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
	
	writeb(0x07, (void*) (UTMI_base+0x8*2));   //default value 0x7; don't change it. 

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

	writeb(readb((void*)(UTMI_base+0x3C*2)) | 0x01, (void*) (UTMI_base+0x3C*2)); // set CA_START as 1
	mdelay(1); // 10 -> 1

	writeb(readb((void*)(UTMI_base+0x3C*2)) & ~0x01, (void*) (UTMI_base+0x3C*2)); // release CA_START

	while ((readb((void*)(UTMI_base+0x3C*2)) & 0x02) == 0);	// polling bit <1> (CA_END)
	if ((0xFFF0 == (readw((void*)(UTMI_base+0x3C*2)) & 0xFFF0 )) ||
		(0x0000 == (readw((void*)(UTMI_base+0x3C*2)) & 0xFFF0 ))  )
		printk("\n\n\n\n\nWARNING: CA Fail !! \n\n");

    if (flag & EHCFLAG_DPDM_SWAP)
    	writeb(readb((void*)(UTMI_base+0x0b*2-1)) |0x20, (void*) (UTMI_base+0x0b*2-1)); // dp dm swap

	writeb((readb((void*)(UTMI_base+0x06*2)) & 0x9F) | 0x40, (void*) (UTMI_base+0x06*2)); //CDR stage select

	writeb(readb((void*)(UTMI_base+0x03*2-1)) | 0x28, (void*) (UTMI_base+0x03*2-1)); //Disconnect window select
	writeb(readb((void*)(UTMI_base+0x03*2-1)) & 0xef, (void*) (UTMI_base+0x03*2-1)); //Disconnect window select

	writeb(readb((void*)(UTMI_base+0x07*2-1)) & 0xfd, (void*) (UTMI_base+0x07*2-1)); //Disable improved CDR
	writeb(readb((void*)(UTMI_base+0x09*2-1)) |0x81, (void*) (UTMI_base+0x09*2-1)); // UTMI RX anti-dead-loc, ISI effect improvement
	writeb(readb((void*)(UTMI_base+0x0b*2-1)) |0x80, (void*) (UTMI_base+0x0b*2-1)); // TX timing select latch path
	writeb(readb((void*)(UTMI_base+0x15*2-1)) |0x20, (void*) (UTMI_base+0x15*2-1)); // Chirp signal source select

	//Enable XHCI preamble function with keep-alive
	writeb(readb((void*)(UTMI_base+0x3F*2-1)) |0x80, (void*) (UTMI_base+0x3F*2-1)); // Enable XHCI preamble function

	#if defined(XHCI_ENABLE_240MHZ)  //exclude Agate
		writeb(readb((void*)(UTMI_base+0x08*2)) | 0x18, (void*) (UTMI_base+0x08*2)); // for 240's phase as 120's clock
		#if defined(XHCI_USE_120_PHASE)
			//Fix it for Eiffel Only. 
			writeb(readb((void*)(UTMI_base+0x08*2)) & 0xF7, (void*) (UTMI_base+0x08*2)); // Clear setting of "240's phase as 120's clock"
		#endif	
	#endif

	// change to 55 timing; for all chips.
	writeb(readb((void*)(UTMI_base+0x15*2-1)) |0x40, (void*) (UTMI_base+0x15*2-1)); // change to 55 timing

    // for CLK 120 override enable; for xHCI on all chips
	writeb(readb((void*)(UTMI_base+0x09*2-1)) |0x04, (void*) (UTMI_base+0x09*2-1)); // for CLK 120 override enable

	writeb(UTMI_DISCON_LEVEL_2A , (void*)(UTMI_base+0x2a*2));

	writeb(XHC_UTMI_EYE_2C, (void*) (UTMI_base+0x2c*2));
	writeb(XHC_UTMI_EYE_2D, (void*) (UTMI_base+0x2d*2-1));
	writeb(XHC_UTMI_EYE_2E, (void*) (UTMI_base+0x2e*2));
	writeb(XHC_UTMI_EYE_2F, (void*) (UTMI_base+0x2f*2-1));

	#if defined(ENABLE_LS_CROSS_POINT_ECO)
		writeb(readb((void*)(UTMI_base+0x04*2)) | 0x40, (void*) (UTMI_base+0x04*2));  //enable deglitch SE0би(low-speed cross point)
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

	#if _USB_ANALOG_RX_SQUELCH_PATCH
		/* squelch level adjust by calibration value */
	{
		unsigned int ca_da_ov, ca_db_ov, ca_tmp;
		ca_tmp = readw((void*)(UTMI_base+0x3c*2));
		ca_da_ov = (((ca_tmp >> 4) & 0x3f) - 5) + 0x40;
		ca_db_ov = (((ca_tmp >> 10) & 0x3f) - 5) + 0x40;
		printk("[%x]-5 ->(ca_da_ov, ca_db_ov) = (%x,%x)\n", ca_tmp, ca_da_ov, ca_db_ov);
		writeb(ca_da_ov ,(void*)(UTMI_base+0x3B*2-1));
		writeb(ca_db_ov ,(void*)(UTMI_base+0x24*2));
	}
	#endif

    if (flag & EHCFLAG_TESTPKG)
    {
	    writew(0x0600, (void*) (UTMI_base+0x14*2)); 
	    writew(0x0038, (void*) (UTMI_base+0x10*2)); 
	    writew(0x0BFE, (void*) (UTMI_base+0x32*2)); 
    }

	// Init USB3 PHY 
	#if defined(XHCI_PHY_MT28)
		Mstar_U3phy_MT28_init(_MSTAR_U3PHY_DTOP_M0_BASE, _MSTAR_U3PHY_DTOP_M1_BASE,
		                      _MSTAR_U3PHY_ATOP_M0_BASE, _MSTAR_U3PHY_ATOP_M1_BASE);
	#else
		Mstar_U3phy_init(_MSTAR_U3PHY_DTOP_BASE, _MSTAR_U3PHY_ATOP_BASE);
	#endif

	//First token idle
	writeb(readb((void*)(XHCI_base+0x4308)) | 0x0C, (void*)(XHCI_base+0x4308));  //First token idle (bit2~3 = "11")
	//Inter packet delay
	writeb(readb((void*)(XHCI_base+0x430F)) | 0xC0, (void*)(XHCI_base+0x430F));  //Inter packet delay (bit6~7 = "11")
	//LFPS patch
	writeb(readb((void*)(XHCI_base+0x681A)) | 0x10, (void*)(XHCI_base+0x681A));  //LFPS patch  (bit4 = 1)

	#if defined(XHCI_DISABLE_COMPLIANCE)
		writeb(readb((void*)(XHCI_base+0x6817)) | 0x80, (void*)(XHCI_base+0x6817));
	#endif
	
	//Bus Reset setting => default 50ms; T1=30; T2=20
	#if defined(XHCI_BUSRESET_REG_OFFSET_CHG)
		writeb((readb((void*)(U3TOP_base+XHCI_NEW_BUSRESET_REG_OFFSET)) & 0xF0) | 0x8, (void*)(U3TOP_base+XHCI_NEW_BUSRESET_REG_OFFSET)); 	
	#else
		writeb((readb((void*)(U3TOP_base+0x24*2)) & 0xF0) | 0x8, (void*)(U3TOP_base+0x24*2));    // [5] = reg_debug_mask to 1'b0
	#endif


	#if defined(XHCI_ENABLE_LASTDOWNZ)
		writeb(readb((void*)(U3TOP_base+0x12*2)) | 0x8, (void*)(U3TOP_base+0x12*2));  //check both last_down_z & data count enable
	#endif

	#if defined(XHCI_MIU1_SEL_BIT30)
    	writeb(readb((void*)(U3TOP_base+0x11*2-1)) | 0x2, (void*)(U3TOP_base+0x11*2-1));  //set reg_miu1_sel to check address bit 30
	#endif
	
	#if (XHCI_CHIRP_PATCH)
		//exclude Agate.
		writeb(UTMI_DISCON_LEVEL_2A, (void*) (UTMI_base+0x3E*2)); //override value 
		writeb((readb((void*)(U3TOP_base+0x24*2)) & 0xF0) | 0x0A, (void*)(U3TOP_base+0x24*2)); // set T1=50, T2=20
		writeb(readb((void*)(UTMI_base+0x3F*2-1)) | 0x1, (void*) (UTMI_base+0x3F*2-1)); //enable the patch 
	#endif
	
	#if defined(XHCI_ENABLE_LOOPBACK_ECO)
		writeb(readb((void*)(U3TOP_base+LOOPBACK_ECO_OFFSET))|LOOPBACK_ECO_BIT , (void*)(U3TOP_base+LOOPBACK_ECO_OFFSET));
	#endif

	#if defined(XHCI_ENABLE_HOTRESET_ECO)
		//Re-enable again to prevent from overwitting by sboot PPC function. Only for Edison.  
		writeb(readb((void*)(U3TOP_base+0xFF*2-1))|0x80, (void*)(U3TOP_base+0xFF*2-1));
	#endif
	
	#if defined(XHCI_ALLOW_LOOPBACK_MODE) //only for Nike, allow HW enter loopback mode
		writeb(readb((void*)(U3TOP_base+0xf8*2)) | 0x1, (void*)(U3TOP_base+0xf8*2));
	#endif

	#if (XHCI_ENABLE_TESTBUS)
	XHCI_enable_testbus((_MSTAR_USB_BASEADR+(0x1E00*2)), U3TOP_base, XHCI_base);
	#endif
	
	#ifdef XHCI_ENABLE_PPC
	XHCI_enable_PPC(U3TOP_base);
	#endif
	
}

/* called during probe() after chip reset completes */
static int xhci_pci_setup(struct usb_hcd *hcd)
{
	struct xhci_hcd		*xhci = hcd_to_xhci(hcd);
	int			retval;
	u32			temp;

	hcd->self.sg_tablesize = TRBS_PER_SEGMENT - 2;

	if (usb_hcd_is_primary_hcd(hcd)) {
		xhci = kzalloc(sizeof(struct xhci_hcd), GFP_KERNEL);
		if (!xhci)
			return -ENOMEM;
		*((struct xhci_hcd **) hcd->hcd_priv) = xhci;
		xhci->main_hcd = hcd;
		/* Mark the first roothub as being USB 2.0.
		 * The xHCI driver will register the USB 3.0 roothub.
		 */
		hcd->speed = HCD_USB2;
		hcd->self.root_hub->speed = USB_SPEED_HIGH;
		/*
		 * USB 2.0 roothub under xHCI has an integrated TT,
		 * (rate matching hub) as opposed to having an OHCI/UHCI
		 * companion controller.
		 */
		hcd->has_tt = 1;
	} else {
		/* xHCI private pointer was set in xhci_pci_probe for the second
		 * registered roothub.
		 */
		xhci = hcd_to_xhci(hcd);
		temp = xhci_readl(xhci, &xhci->cap_regs->hcc_params);
		if (HCC_64BIT_ADDR(temp)) {
			xhci_dbg(xhci, "Enabling 64-bit DMA addresses.\n");
			dma_set_mask(hcd->self.controller, DMA_BIT_MASK(64));
		} else {
			dma_set_mask(hcd->self.controller, DMA_BIT_MASK(32));
		}
		return 0;
	}

	xhci->cap_regs = hcd->regs;
	xhci->op_regs = hcd->regs +
		HC_LENGTH(xhci_readl(xhci, &xhci->cap_regs->hc_capbase));
	xhci->run_regs = hcd->regs +
		(xhci_readl(xhci, &xhci->cap_regs->run_regs_off) & RTSOFF_MASK);
	/* Cache read-only capability registers */
	xhci->hcs_params1 = xhci_readl(xhci, &xhci->cap_regs->hcs_params1);
	xhci->hcs_params2 = xhci_readl(xhci, &xhci->cap_regs->hcs_params2);
	xhci->hcs_params3 = xhci_readl(xhci, &xhci->cap_regs->hcs_params3);
	xhci->hcc_params = xhci_readl(xhci, &xhci->cap_regs->hc_capbase);
	//xhci->hci_version = HC_VERSION(xhci->hcc_params);
	xhci->hci_version = 0x96;  //Jonas modified for real HW version. 
	xhci->hcc_params = xhci_readl(xhci, &xhci->cap_regs->hcc_params);
	xhci_print_registers(xhci);

	/* Look for vendor-specific quirks */
	xhci->quirks |= XHCI_SPURIOUS_SUCCESS;
	xhci->quirks |= XHCI_RESET_ON_RESUME;
	//xhci_dbg(xhci, "QUIRK: Resetting on resume\n");

	/* Make sure the HC is halted. */
	retval = xhci_halt(xhci);
	if (retval)
		goto error;

	xhci_dbg(xhci, "Resetting HCD\n");
	/* Reset the internal HC memory state and registers. */
	retval = xhci_reset(xhci);
	if (retval)
		goto error;
	xhci_dbg(xhci, "Reset complete\n");

	temp = xhci_readl(xhci, &xhci->cap_regs->hcc_params);
	if (HCC_64BIT_ADDR(temp)) {
		xhci_dbg(xhci, "Enabling 64-bit DMA addresses.\n");
		dma_set_mask(hcd->self.controller, DMA_BIT_MASK(64));
	} else {
		dma_set_mask(hcd->self.controller, DMA_BIT_MASK(32));
	}

	xhci_dbg(xhci, "Calling HCD init\n");
	/* Initialize HCD and host controller data structures. */
	retval = xhci_init(hcd);
	if (retval)
		goto error;
	xhci_dbg(xhci, "Called HCD init\n");

	return retval;

error:
	kfree(xhci);
	return retval;
}

void usb_hcd_mstar_remove(struct usb_hcd *hcd, struct platform_device *pdev)
{

	/* Fake an interrupt request in order to give the driver a chance
	 * to test whether the controller hardware has been removed (e.g.,
	 * cardbus physical eject).
	 */
	//local_irq_disable();
	//usb_hcd_irq(0, hcd);
	//local_irq_enable();

	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
}


int xhci_hcd_mstar_probe(const struct hc_driver *driver, struct platform_device *dev)
{
	int retval=0;
	struct usb_hcd *hcd;
	//struct xhci_hcd *xhci;    
	unsigned int flag=0;

	if (usb_disabled())
		return -ENODEV;

	if (!driver)
		return -EINVAL;	

	printk("xHCI_%x%04x \n", readb((void*)(_MSTAR_PM_BASE+0x1ECC*2)), readw((void*)(_MSTAR_PM_BASE+0x1ECE*2)));

	printk("Mstar-xhci-1 H.W init\n");
	#if defined(XHCI_ENABLE_DPDM_SWAP)
	flag |= EHCFLAG_DPDM_SWAP;
	#endif
	Mstar_xhc_init(_MSTAR_U3UTMI_BASE, _MSTAR_XHCI_BASE,
	           _MSTAR_U3TOP_BASE, _MSTAR_U3BC_BASE, flag);

	if (dev->resource[2].flags != IORESOURCE_IRQ) {
		pr_debug("resource[1] is not IORESOURCE_IRQ");
		retval = -ENOMEM;
	}
	hcd = usb_create_hcd(driver, &dev->dev, "mstar");
	if (!hcd)
		return -ENOMEM;
	hcd->rsrc_start = dev->resource[1].start;
	hcd->rsrc_len = dev->resource[1].end - dev->resource[1].start + 0;

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, hcd_name)) {
		printk("request_mem_region failed");
		retval = -EBUSY;
		goto clear_companion;
	}

	hcd->regs = (void *)(u32)(hcd->rsrc_start);	        	
	if (!hcd->regs) {
		printk("ioremap failed");
		retval = -ENOMEM;
		goto release_mem_region;
	}

	hcd->xhci_base = _MSTAR_XHCI_BASE;
#if defined(XHCI_PHY_MT28)
	hcd->u3phy_d_m0_base = _MSTAR_U3PHY_DTOP_M0_BASE;
	hcd->u3phy_d_m1_base = _MSTAR_U3PHY_DTOP_M1_BASE;
	hcd->u3phy_a_m0_base = _MSTAR_U3PHY_ATOP_M0_BASE;
	hcd->u3phy_a_m1_base = _MSTAR_U3PHY_ATOP_M1_BASE;
#else	
	hcd->u3phy_d_base = _MSTAR_U3PHY_DTOP_BASE;
	hcd->u3phy_a_base = _MSTAR_U3PHY_ATOP_BASE;
#endif	
	hcd->u3top_base = _MSTAR_U3TOP_BASE;
	//hcd->u3indctl_base = _MSTAR_U3INDCTL_BASE;
	hcd->utmi_base = _MSTAR_U3UTMI_BASE;
	hcd->bc_base = _MSTAR_U3BC_BASE;

#ifdef ENABLE_BATTERY_CHARGE
	usb_bc_enable(hcd, true);
#else
	#if defined(USB_NO_BC_FUNCTION)  //list for ICs before Edison 
	//do nothing.
	#else
	//Disable default setting.
	usb_bc_enable(hcd, false);
	#endif
#endif

#if _UTMI_PWR_SAV_MODE_ENABLE
	usb_power_saving_enable(hcd, true);
#endif

	hcd->lock_usbreset=__SPIN_LOCK_UNLOCKED(hcd->lock_usbreset);

	retval = usb_add_hcd(hcd, dev->resource[2].start, IRQF_DISABLED /* | IRQF_SHARED */);
	if (retval != 0)
		goto release_mem_region;
	//set_hs_companion(dev, hcd);


	return retval;
    
release_mem_region:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
clear_companion:
	usb_put_hcd(hcd);    
	//clear_hs_companion(dev, hcd);
	return retval;
}


/*
 * We need to register our own probe function (instead of the USB core's
 * function) in order to create a second roothub under xHCI.
 */
static int xhci_mstar_probe(struct hc_driver *driver, struct platform_device *dev)
{
	int retval;
	struct xhci_hcd *xhci;
	struct usb_hcd *hcd;

	/* Register the USB 2.0 roothub.
	 * FIXME: USB core must know to register the USB 2.0 roothub first.
	 * This is sort of silly, because we could just set the HCD driver flags
	 * to say USB 2.0, but I'm not sure what the implications would be in
	 * the other parts of the HCD code.
	 */
	retval = xhci_hcd_mstar_probe(driver, dev);

	if (retval)
		return retval;

	/* USB 2.0 roothub is stored in the PCI device now. */
	hcd = dev_get_drvdata(&dev->dev);
	xhci = hcd_to_xhci(hcd);
	xhci->shared_hcd = usb_create_shared_hcd(driver, &dev->dev,
				"mstar", hcd);
	if (!xhci->shared_hcd) {
		retval = -ENOMEM;
		goto dealloc_usb2_hcd;
	}

	/* Set the xHCI pointer before xhci_pci_setup() (aka hcd_driver.reset)
	 * is called by usb_add_hcd().
	 */
	*((struct xhci_hcd **) xhci->shared_hcd->hcd_priv) = xhci;
	
	xhci->shared_hcd->xhci_base = _MSTAR_XHCI_BASE;
#if defined(XHCI_PHY_MT28)
	xhci->shared_hcd->u3phy_d_m0_base = _MSTAR_U3PHY_DTOP_M0_BASE;
	xhci->shared_hcd->u3phy_d_m1_base = _MSTAR_U3PHY_DTOP_M1_BASE;
	xhci->shared_hcd->u3phy_a_m0_base = _MSTAR_U3PHY_ATOP_M0_BASE;
	xhci->shared_hcd->u3phy_a_m1_base = _MSTAR_U3PHY_ATOP_M1_BASE;
#else
	xhci->shared_hcd->u3phy_d_base = _MSTAR_U3PHY_DTOP_BASE;
	xhci->shared_hcd->u3phy_a_base = _MSTAR_U3PHY_ATOP_BASE;
#endif
	xhci->shared_hcd->u3top_base = _MSTAR_U3TOP_BASE;
	//xhci->shared_hcd->u3indctl_base = _MSTAR_U3INDCTL_BASE;
	xhci->shared_hcd->utmi_base = _MSTAR_U3UTMI_BASE;
	xhci->shared_hcd->bc_base = _MSTAR_U3BC_BASE;	
	xhci->shared_hcd->lock_usbreset=__SPIN_LOCK_UNLOCKED(xhci->shared_hcd->lock_usbreset);

	retval = usb_add_hcd(xhci->shared_hcd, dev->resource[2].start,
			IRQF_DISABLED | IRQF_SHARED);
	if (retval)
		goto put_usb3_hcd;
	/* Roothub already marked as USB 3.0 speed */
    
	return 0;

put_usb3_hcd:
	usb_put_hcd(xhci->shared_hcd);
dealloc_usb2_hcd:
	usb_hcd_mstar_remove(hcd, dev);
	return retval;
}

static void xhci_mstar_remove(struct usb_hcd *hcd, struct platform_device *pdev)
{
	struct xhci_hcd *xhci;

	xhci = hcd_to_xhci(hcd);
	if (xhci->shared_hcd) {
		usb_remove_hcd(xhci->shared_hcd);
		usb_put_hcd(xhci->shared_hcd);
	}
	usb_hcd_mstar_remove(hcd, pdev);
	kfree(xhci);
}

#ifdef CONFIG_PM
static int xhci_pci_suspend(struct usb_hcd *hcd, bool do_wakeup)
{
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);
	int	retval = 0;

	printk("xhci_pci_suspend \n");
	
	if (hcd->state != HC_STATE_SUSPENDED ||
			xhci->shared_hcd->state != HC_STATE_SUSPENDED)
		return -EINVAL;

	retval = xhci_suspend(xhci);

	return retval;
}

static int xhci_pci_resume(struct usb_hcd *hcd, bool hibernated)
{
	struct xhci_hcd		*xhci = hcd_to_xhci(hcd);
	int			retval = 0;

	printk("xhci_pci_resume \n");

	retval = xhci_resume(xhci, hibernated);
	return retval;
}
#endif /* CONFIG_PM */

static struct hc_driver xhci_mstar_hc_driver = {
	.description =		hcd_name,
	.product_desc =		"Mstar xHCI Host Controller",
	.hcd_priv_size =	sizeof(struct xhci_hcd *),

	/*
	 * generic hardware linkage
	 */
	.irq =			xhci_irq,
	.flags =		HCD_MEMORY | HCD_USB3 | HCD_SHARED,

	/*
	 * basic lifecycle operations
	 */
	.reset =		xhci_pci_setup,
	.start =		xhci_run,
#ifdef CONFIG_PM
	.pci_suspend =          xhci_pci_suspend,
	.pci_resume =           xhci_pci_resume,
#endif
	.stop =			xhci_stop,
	.shutdown =		xhci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		xhci_urb_enqueue,
	.urb_dequeue =		xhci_urb_dequeue,
	.alloc_dev =		xhci_alloc_dev,
	.free_dev =		xhci_free_dev,
	.alloc_streams =	xhci_alloc_streams,
	.free_streams =		xhci_free_streams,
	.add_endpoint =		xhci_add_endpoint,
	.drop_endpoint =	xhci_drop_endpoint,
	.endpoint_reset =	xhci_endpoint_reset,
	.check_bandwidth =	xhci_check_bandwidth,
	.reset_bandwidth =	xhci_reset_bandwidth,
	.address_device =	xhci_address_device,
	.update_hub_device =	xhci_update_hub_device,
	.reset_device =		xhci_discover_or_reset_device,

	/*
	 * scheduling support
	 */
	.get_frame_number =	xhci_get_frame,

	/* Root hub support */
	.hub_control =		xhci_hub_control,
	.hub_status_data =	xhci_hub_status_data,
	.bus_suspend =		xhci_bus_suspend,
	.bus_resume =		xhci_bus_resume,
};

static int xhci_hcd_mstar_drv_probe(struct platform_device *pdev)
{
	int ret;

	pr_debug("In xhci_hcd_mstar_drv_probe\n");

	if (usb_disabled())
		return -ENODEV;

	ret = xhci_mstar_probe(&xhci_mstar_hc_driver, pdev);
	return ret;
}

static int xhci_hcd_mstar_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	xhci_mstar_remove(hcd, pdev);
	return 0;
}

#ifdef CONFIG_PM
static int xhci_hcd_mstar_drv_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct usb_hcd *hcd;
	struct xhci_hcd *xhci;

	printk("xhci_hcd_mstar_drv_suspend \n");

	hcd = platform_get_drvdata(pdev);
	xhci = hcd_to_xhci(hcd);
	
	return xhci_suspend(xhci);
}

static int xhci_hcd_mstar_drv_resume(struct platform_device *pdev)
{
	unsigned int flag=0;
	struct xhci_hcd *xhci;
	int	   retval = 0;
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	printk("xhci_hcd_mstar_drv_resume \n");
	
	#if defined(XHCI_ENABLE_DPDM_SWAP)
	flag |= EHCFLAG_DPDM_SWAP;
	#endif
	Mstar_xhc_init(_MSTAR_U3UTMI_BASE, _MSTAR_XHCI_BASE,
	           _MSTAR_U3TOP_BASE, _MSTAR_U3BC_BASE, flag);    

#ifdef ENABLE_BATTERY_CHARGE
	usb_bc_enable(hcd, true);
#else
	#if defined(USB_NO_BC_FUNCTION)  //list for ICs before Edison 
	//do nothing.
	#else
	//Disable default setting.
	usb_bc_enable(hcd, false);
	#endif
#endif
	xhci = hcd_to_xhci(hcd);
	retval = xhci_resume(xhci, false);
    if (retval) {
        printk(" xhci_resume FAIL : -0x%x !!", -retval); 
        return retval; 
    }        
	//enable_irq(hcd->irq);

	return 0;
}
#endif

/*-------------------------------------------------------------------------*/

static struct platform_driver xhci_mstar_driver = {

	.probe =	xhci_hcd_mstar_drv_probe,
	.remove =	xhci_hcd_mstar_drv_remove,
	// .shutdown	= usb_hcd_platform_shutdown,

	
	#ifdef CONFIG_PM	
	.suspend	= xhci_hcd_mstar_drv_suspend,
	.resume		= xhci_hcd_mstar_drv_resume,    
	#endif

    .driver = {
        .name   = "Mstar-xhci-1",
        // .bus    = &platform_bus_type,
	},
};

int xhci_register_mstar(void)
{
	return platform_driver_register(&xhci_mstar_driver);
}

void xhci_unregister_mstar(void)
{
	platform_driver_unregister(&xhci_mstar_driver);
}
