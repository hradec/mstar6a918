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

#include "eMMC.h"
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>

#if defined(UNIFIED_eMMC_DRIVER) && UNIFIED_eMMC_DRIVER

U32 gu32_eMMCDrvExtFlag = 0;

// common for TV linux platform
extern bool ms_sdmmc_card_chg(unsigned int slotNo);
extern struct platform_device sg_mstar_emmc_device_st;

U32 eMMC_hw_timer_delay(U32 u32us)
{
	#if 0 // udelay not work
    volatile u32 i = 0;

	for (i = 0; i < (u32us>>0); i++)
	{
		#if 1
		volatile int j = 0, tmp;
		for (j = 0; j < 0x38; j++)
		{
			tmp = j;
		}
		#endif
	}
	#else
    U32 u32_i = u32us;

	while(u32_i > 1000)
	{
		mdelay(1);
		u32_i -= 1000;
	}
    udelay(u32_i);
	#endif
	return u32us;
}


U32 eMMC_hw_timer_sleep(U32 u32ms)
{
	U32 u32_i = u32ms;

	while(u32_i > 1000)
	{
	    msleep(1000);
		u32_i -= 1000;
	}

	msleep(u32_i);
    return u32ms;
}


//--------------------------------
// use to performance test
static U32 u32_ms_start;
U32 eMMC_hw_timer_start(void)
{
	u32_ms_start = jiffies_to_usecs(jiffies);
	return eMMC_ST_SUCCESS;
}

U32 eMMC_hw_timer_tick(void)
{
	return jiffies_to_usecs(jiffies) - u32_ms_start;
}


//--------------------------------
void eMMC_set_WatchDog(U8 u8_IfEnable)
{
	// do nothing
}

void eMMC_reset_WatchDog(void)
{
	// do nothing
}


//---------------------------------------
U32 eMMC_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt)
{
	if(u32_DMAAddr >= MSTAR_MIU1_BUS_BASE)//MIU1
	{
		REG_FCIE_SETBIT(FCIE_MIU_DMA_26_16, BIT_MIU1_SELECT);
		u32_DMAAddr -= MSTAR_MIU1_BUS_BASE;
	}
    else
    {
    	REG_FCIE_CLRBIT(FCIE_MIU_DMA_26_16, BIT_MIU1_SELECT);
		u32_DMAAddr -= MSTAR_MIU0_BUS_BASE;
    }
	return u32_DMAAddr;
}

U32 eMMC_DMA_MAP_address(U32 u32_Buffer, U32 u32_ByteCnt, int mode)
{
	dma_addr_t dma_addr;
	#if defined(CONFIG_ENABLE_EMMC_ACP) && CONFIG_ENABLE_EMMC_ACP
	dma_addr = dma_map_single(&sg_mstar_emmc_device_st.dev, (void*)u32_Buffer, u32_ByteCnt, DMA_ACP);
	#else
	if(mode == 0)	//write
	{
		dma_addr = dma_map_single(&sg_mstar_emmc_device_st.dev, (void*)u32_Buffer, u32_ByteCnt, DMA_TO_DEVICE);
	}
	else
	{
		dma_addr = dma_map_single(&sg_mstar_emmc_device_st.dev, (void*)u32_Buffer, u32_ByteCnt, DMA_FROM_DEVICE);
	}
	#endif

	return dma_addr;
}

void eMMC_DMA_UNMAP_address(U32 u32_DMAAddr, U32 u32_ByteCnt, int mode)
{
	#if defined(CONFIG_ENABLE_EMMC_ACP) && CONFIG_ENABLE_EMMC_ACP
	dma_unmap_single(&sg_mstar_emmc_device_st.dev, u32_DMAAddr, u32_ByteCnt, DMA_ACP);

	#else
	if(mode == 0)	//write
	{
		dma_unmap_single(&sg_mstar_emmc_device_st.dev, u32_DMAAddr, u32_ByteCnt, DMA_TO_DEVICE);
	}
	else
	{
		dma_unmap_single(&sg_mstar_emmc_device_st.dev, u32_DMAAddr, u32_ByteCnt, DMA_FROM_DEVICE);
	}
	#endif
}


void eMMC_flush_data_cache_buffer(U32 u32_DMAAddr, U32 u32_ByteCnt)
{
    Chip_Clean_Cache_Range_VA_PA(u32_DMAAddr,__pa(u32_DMAAddr), u32_ByteCnt);
}

void eMMC_Invalidate_data_cache_buffer(U32 u32_DMAAddr, U32 u32_ByteCnt)
{
    Chip_Inv_Cache_Range_VA_PA(u32_DMAAddr,__pa(u32_DMAAddr), u32_ByteCnt);
}

void eMMC_flush_miu_pipe(void)
{

}


void eMMC_DumpChiptop(void)
{
	U16 u16_i, u16_reg;

	eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n\n chiptop:");

	for(u16_i=0 ; u16_i<0x80; u16_i++)
	{
		if(0 == u16_i%8)
			eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n%02Xh:| ", u16_i);

		REG_FCIE_R(GET_REG_ADDR(PAD_CHIPTOP_BASE, u16_i), u16_reg);
		eMMC_debug(eMMC_DEBUG_LEVEL, 0, "%04Xh ", u16_reg);

		if((u16_i & 0x7) == 0x7)
			eMMC_debug(eMMC_DEBUG_LEVEL, 0, "| %02Xh ", u16_i);
	}
}

extern U32 gu32_pwrsvr_gpio_enable;
extern U32 gu32_pwrsvr_gpio_addr;
extern U32 gu32_pwrsvr_gpio_bit;
extern U32 gu32_pwrsvr_gpio_trigger;

void eMMC_CheckPowerCut(void)
{

#if ((defined(eMMC_DRV_EIFFEL_LINUX) && eMMC_DRV_EIFFEL_LINUX) || (defined(eMMC_DRV_NIKE_LINUX) && eMMC_DRV_NIKE_LINUX))
	/* SAR5=ON in set_config will enable this feature */
	extern u8 u8_enable_sar5;
	if(u8_enable_sar5 == 1)
	{
    	while(((REG_FCIE(reg_vplug_in_pwrgd) >> 14)&0x0001) == 0)
    	{
    		eMMC_debug(0,1,"detect power cut\n");
    		//eMMC_hw_timer_delay(HW_TIMER_DELAY_500ms);
    		while(1);
    	}
    }
    else if(gu32_pwrsvr_gpio_enable)
    {
        if((REG_FCIE(IO_ADDRESS(gu32_pwrsvr_gpio_addr))&(1<<gu32_pwrsvr_gpio_bit))==gu32_pwrsvr_gpio_trigger)
        {
            eMMC_debug(0,1,"detect power cut\n");
            //eMMC_hw_timer_delay(HW_TIMER_DELAY_500ms);
            eMMC_CMD0(0);
            while(1);
        }
    }
#endif
}

void eMMC_Prepare_Power_Saving_Mode_Queue(void)
{
    #if ((defined(eMMC_DRV_EIFFEL_LINUX) && eMMC_DRV_EIFFEL_LINUX) || \
		(defined(eMMC_DRV_NIKE_LINUX) && eMMC_DRV_NIKE_LINUX) || \
		(defined(CONFIG_MSTAR_EINSTEIN) && CONFIG_MSTAR_EINSTEIN)||\
		(defined(eMMC_DRV_NAPOLI_LINUX) && eMMC_DRV_NAPOLI_LINUX))

    REG_FCIE_SETBIT(reg_pwrgd_int_glirm, BIT_PWRGD_INT_GLIRM_EN);

    #if ((defined(CONFIG_MSTAR_EINSTEIN) && CONFIG_MSTAR_EINSTEIN) || \
         (defined(CONFIG_MSTAR_NAPOLI) && CONFIG_MSTAR_NAPOLI))
    REG_FCIE_CLRBIT(reg_pwrgd_int_glirm, BIT_PWEGD_INT_GLIRM_MASK);
    REG_FCIE_SETBIT(reg_pwrgd_int_glirm, (1<<10));
    #endif

    /* (1) Clear HW Enable */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x40), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x41),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0A);

	/* (2) Clear All Interrupt */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x42), 0xffff);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x43),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x00);

	/* (3) Clear SDE MODE Enable */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x44), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x45),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x10);

	/* (4) Clear SDE CTL Enable */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x46), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x47),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x11);

	/* (5) Reset Start */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x48), 0x4800);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x49),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x30);

	/* (6) Reset End */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x4A), 0x5800);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x4B),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x30);

	/* (7) Set "SD_MOD" */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x4C), 0x0051);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x4D),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x10);

	/* (8) Enable "csreg_sd_en" */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x4E), 0x0002);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x4F),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0A);

	/* (9) Command Content, IDLE */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x50), 0x0040);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x51),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x60);

	/* (10) Command Content, STOP */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x52), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x53),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x61);

	/* (11) Command Content, STOP */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x54), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x55),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x62);

	/* (12) Command Size */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x56), 0x0005);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x57),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0E);

	/* (13) Response Size */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x58), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x59),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0D);

	/* (14) Enable Interrupt, SD_CMD_END */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x5A), 0x0002);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x5B),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x01);

	/* (15) Command Enable */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x5C), 0x0004);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x5D),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x11);

	/* (16) Wait Interrupt */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x5E), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x5F),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WINT);

	/* (17) Clear Interrupt */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x60), 0x0002);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x61),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x00);

    /* (18) Clear HW Enable */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x62), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x63),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0A);

	/* (19) STOP */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x64), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x65),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_STOP);

    REG_FCIE_W(FCIE_PWR_SAVE_MODE, BIT_POWER_SAVE_MODE_EN | BIT_SD_POWER_SAVE_RST);
    #endif
}


//--------------------------------
#if defined(ENABLE_eMMC_ATOP) && ENABLE_eMMC_ATOP

U8 gau8_eMMCPLLSel_52[eMMC_FCIE_VALID_CLK_CNT] = {
	eMMC_PLL_CLK__52M,
	eMMC_PLL_CLK__32M,
    eMMC_PLL_CLK__27M
};

U8 gau8_eMMCPLLSel_200[eMMC_FCIE_VALID_CLK_CNT] = {
	eMMC_PLL_CLK_200M,
	eMMC_PLL_CLK_160M,
	eMMC_PLL_CLK_140M
};

static  eMMC_FCIE_ATOP_SET_t sgTSetTmp_t;

static U32 eMMC_FCIE_DetectTiming_Ex(U32 u32_Skew4Result)
{
	U16 u16_i, u16_StartBit=0xFFFF, u16_EndBit=0xFFFF, u16_cnt;
	U32 u32_tmp;
		
	sgTSetTmp_t.u32_ScanResult = u32_Skew4Result;
		
	// ==================================================
	// case.1: all bits set
	if((u32_Skew4Result & ((1<<PLL_SKEW4_CNT*2)-1)) == ((1<<PLL_SKEW4_CNT*2)-1) )
	{
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"  case1: all\n");
		sgTSetTmp_t.u8_Reg2Ch=1;
		sgTSetTmp_t.u8_Skew4=0;
	}
	// ==================================================
	// case.2: not wrapped
	else if( 0==(u32_Skew4Result&BIT0) || 0==(u32_Skew4Result&(1<<(PLL_SKEW4_CNT*2-1))) )
	{
		for(u16_i=0; u16_i<PLL_SKEW4_CNT*2; u16_i++)
		{
			if((u32_Skew4Result&1<<u16_i) && 0xFFFF==u16_StartBit)
				u16_StartBit = u16_i;
			else if(0xFFFF!=u16_StartBit && 0==(u32_Skew4Result&1<<u16_i))
				u16_EndBit = u16_i-1;
	
			if(0xFFFF != u16_EndBit)
			{
				if(u16_EndBit-u16_StartBit+1<MIN_OK_SKEW_CNT)
				{	// to ignore "discontinous case"
					u16_StartBit = 0xFFFF;
					u16_EndBit = 0xFFFF;
				}
				else
					break;
			}
		}
		if(0xFFFF==u16_EndBit)
		{
			if( u32_Skew4Result&1<<(u16_i-1) )
				u16_EndBit = u16_i-1;
			else
				u16_EndBit = u16_StartBit;
		}		
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"  case2: not wrapped: %u %u\n", u16_StartBit, u16_EndBit);
	
		if(u16_EndBit-u16_StartBit+1<MIN_OK_SKEW_CNT)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Clk:%uMHz Case2 not enough skew4: %Xh %Xh\n", 
				g_eMMCDrv.u32_ClkKHz/1000, u32_Skew4Result, u16_EndBit-u16_StartBit+1);
			return eMMC_ST_ERR_SKEW4;
		}
			
		if( (u16_StartBit+u16_EndBit)/2 < PLL_SKEW4_CNT)
		{
			sgTSetTmp_t.u8_Reg2Ch=0;
			sgTSetTmp_t.u8_Skew4=(u16_StartBit+u16_EndBit)/2;
		}
		else
		{	sgTSetTmp_t.u8_Reg2Ch=1;
			sgTSetTmp_t.u8_Skew4=(u16_StartBit+u16_EndBit)/2 -PLL_SKEW4_CNT;
		}
	}
	// ==================================================
	// case.3: wrapped
	else
	{	// --------------------------------
		// to ignore "discontinous case"
		u16_cnt = 0;
		for(u16_i=0; u16_i<PLL_SKEW4_CNT*2; u16_i++)
		{
			if(u32_Skew4Result&1<<u16_i)
				u16_cnt++;
			else
			{	u16_StartBit = u16_i-1;
				break;
			}
		}
		for(u16_i=PLL_SKEW4_CNT*2-1; u16_i>0; u16_i--)
		{
			if(u32_Skew4Result&1<<u16_i)
				u16_cnt++;
			else
			{	u16_EndBit = u16_i+1;
				break;
			}
		}
		if(u16_cnt < MIN_OK_SKEW_CNT)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL,0,"  case3: wrapped but goto case2: %u, %u %u\n", 
				u16_cnt, u16_StartBit, u16_EndBit);
			u32_Skew4Result &= ~((1<<(u16_StartBit+1))-1);
			return eMMC_FCIE_DetectTiming_Ex(u32_Skew4Result);
		}
		else
		{	u32_tmp = u32_Skew4Result;
			for(u16_i=u16_StartBit+1; u16_i<u16_EndBit; u16_i++)
				u32_Skew4Result &= ~(1<<u16_i);

			u16_StartBit = 0xFFFF;
			u16_EndBit = 0xFFFF;
		}		
	
		// --------------------------------
		// normal judgement
		for(u16_i=0; u16_i<PLL_SKEW4_CNT*2; u16_i++)
		{
			if(0==(u32_Skew4Result&1<<u16_i) && 0xFFFF==u16_StartBit)
				u16_StartBit = u16_i-1;
			else if(0xFFFF!=u16_StartBit && (u32_Skew4Result&1<<u16_i))
				u16_EndBit = u16_i;
	
			if(0xFFFF != u16_EndBit)
				break;
		}
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"  case3: wrapped: %u %u, %Xh %Xh\n", 
			u16_StartBit, u16_EndBit, u32_tmp, u32_Skew4Result);
			
		if(u16_StartBit+1 > PLL_SKEW4_CNT*2-u16_EndBit)
		{
			sgTSetTmp_t.u8_Reg2Ch=0;
			sgTSetTmp_t.u8_Skew4 = (u16_StartBit-(PLL_SKEW4_CNT*2-u16_EndBit))/2;
		}
		else
		{	sgTSetTmp_t.u8_Reg2Ch=1;
			sgTSetTmp_t.u8_Skew4 = u16_EndBit + 
				((PLL_SKEW4_CNT*2-u16_EndBit)+u16_StartBit)/2 -
				PLL_SKEW4_CNT;
		}
	}
	
	eMMC_debug(eMMC_DEBUG_LEVEL, 0, "  %Xh %Xh \n", sgTSetTmp_t.u8_Reg2Ch, sgTSetTmp_t.u8_Skew4);
	if(sgTSetTmp_t.u8_Skew4 >= PLL_SKEW4_CNT)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: Skew4:%u > %u\n", sgTSetTmp_t.u8_Skew4, PLL_SKEW4_CNT);
			
	return eMMC_ST_SUCCESS;
}


U32 eMMC_FCIE_DetectDDRTiming(void)
{
	U32 u32_i, u32_err, u32_Skew4Result=0;
	U16 u16_SkewCnt=0;
	
	g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_TUNING_TTABLE; // turn on tunning flag

	REG_FCIE_CLRBIT(FCIE_SM_STS, BIT11);	
	for(u32_i=0; u32_i<PLL_SKEW4_CNT; u32_i++) 
	{
		// make sure a complete outside clock cycle
		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); 
		
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i<<12);
		u32_err = eMMCTest_BlkWRC_ProbeTiming(eMMC_TEST_BLK_0);
		if(eMMC_ST_SUCCESS==u32_err) 
		{
			u16_SkewCnt++;
			u32_Skew4Result |= (1<<u32_i);
		}
	}

	REG_FCIE_SETBIT(FCIE_SM_STS, BIT11);
	for(u32_i=PLL_SKEW4_CNT; u32_i<PLL_SKEW4_CNT*2; u32_i++) 
	{
		// make sure a complete outside clock cycle
		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); 
		
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, (u32_i-PLL_SKEW4_CNT)<<12);		
		u32_err = eMMCTest_BlkWRC_ProbeTiming(eMMC_TEST_BLK_0);
		if(eMMC_ST_SUCCESS==u32_err)  
		{
			u16_SkewCnt++;
			u32_Skew4Result |= (1<<u32_i);
		}
	}

	g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_TUNING_TTABLE; // turn off tunning flag

	if(0==u32_Skew4Result || u16_SkewCnt<MIN_OK_SKEW_CNT)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: Clk: %uMHz SkewResult: %Xh SkewCnt: %u\n", 
			g_eMMCDrv.u32_ClkKHz, u32_Skew4Result, u16_SkewCnt);
		return eMMC_ST_ERR_SKEW4;
	}
	
    return eMMC_FCIE_DetectTiming_Ex(u32_Skew4Result);	
}


U32 eMMC_FCIE_BuildDDRTimingTable(void)
{
	U32 u32_err, u32_ret;
	U8  u8_SetIdx, u8_ClkIdx;

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: building DDR table, please wait... \n");
	memset((void*)&g_eMMCDrv.TimingTable_t, 0, sizeof(g_eMMCDrv.TimingTable_t));
	u8_SetIdx = 0;

	if(!eMMC_IF_NORMAL_SDR())
	{
		u32_err = eMMC_FCIE_EnableSDRMode();
		if(eMMC_ST_SUCCESS != u32_err)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC Err: set SDR IF fail: %X\n", u32_err);
			return u32_err;
		}
	}	

	u32_err = eMMC_FCIE_EnableFastMode_Ex(FCIE_eMMC_DDR);
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC Err: set DDR IF fail: %X\n", u32_err);
		return u32_err;
	}
	
	for(u8_ClkIdx=0; u8_ClkIdx<eMMC_FCIE_VALID_CLK_CNT; u8_ClkIdx++)
	{	
		eMMC_clock_setting(gau8_eMMCPLLSel_52[u8_ClkIdx]);
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\nclk: %uMHz\n", g_eMMCDrv.u32_ClkKHz/1000);
		
		// ---------------------------
		// search and set the Windows
		u32_err = eMMC_FCIE_DetectDDRTiming();

		// ---------------------------
		// set the Table
		if(eMMC_ST_SUCCESS == u32_err)
		{
			g_eMMCDrv.TimingTable_t.u8_SetCnt++;
			g_eMMCDrv.TimingTable_t.Set[u8_ClkIdx].u8_Clk = gau8_eMMCPLLSel_52[u8_ClkIdx];
			g_eMMCDrv.TimingTable_t.Set[u8_ClkIdx].u8_Reg2Ch = sgTSetTmp_t.u8_Reg2Ch;
			g_eMMCDrv.TimingTable_t.Set[u8_ClkIdx].u8_Skew4 = sgTSetTmp_t.u8_Skew4;
			g_eMMCDrv.TimingTable_t.Set[u8_ClkIdx].u32_ScanResult = sgTSetTmp_t.u32_ScanResult;
			g_eMMCDrv.TimingTable_t.u32_VerNo = eMMC_TIMING_TABLE_VERSION;
			eMMC_DumpTimingTable();
			eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\nOK\n");

			g_eMMCDrv.TimingTable_t.u32_ChkSum =
		        eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, 
		        sizeof(g_eMMCDrv.TimingTable_t)-eMMC_TIMING_TABLE_CHKSUM_OFFSET);
		    memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t));

		    eMMC_FCIE_ErrHandler_ReInit();

			u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
	        u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
	        if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	        {
		        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: Save TTable fail: %Xh %Xh\n",
			        u32_err, u32_ret);
		        return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	        }
			
	        return eMMC_ST_SUCCESS;
		}
	}	
	
	eMMC_DumpTimingTable();
	eMMC_die("");
	return eMMC_ST_ERR_SKEW4;
	
}


U32 eMMC_FCIE_ResetToHS200(U8 u8_ClkParam)
{
	U32 u32_err;
	
	// --------------------------------
	// reset FCIE & eMMC to normal SDR mode
	if(!eMMC_IF_NORMAL_SDR())
	{
		//eMMC_debug(0,1,"eMMC: re-init to SDR\n");
		g_eMMCDrv.u32_DrvFlag = 0;
	    eMMC_PlatformInit();
		u32_err = eMMC_FCIE_Init();
	    if(u32_err)
	    {
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: eMMC_FCIE_Init fail, %Xh\n", u32_err);
		    return u32_err;
	    }
		u32_err = eMMC_Init_Device_Ex();
		if(eMMC_ST_SUCCESS != u32_err)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC Err: eMMC_Init_Device_Ex fail: %X\n", u32_err);
			return u32_err;
		}
	}
	
	// --------------------------------
	// set eMMC to HS200 mode
	u32_err = eMMC_FCIE_EnableFastMode_Ex(FCIE_eMMC_HS200);
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC Err: set HS200 IF fail: %X\n", u32_err);

		return u32_err;
	}

	eMMC_clock_setting(u8_ClkParam);

	return u32_err;
}

U32 eMMC_FCIE_DetectHS200Timing(void)
{
	U32 u32_i, u32_err, u32_Skew4Result=0;
	U16 u16_SkewCnt=0;
	
	for(u32_i=0; u32_i<PLL_SKEW4_CNT; u32_i++) 
	{
		// make sure a complete outside clock cycle
		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); 

		g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_TUNING_TTABLE; // turn on tunning flag
		REG_FCIE_CLRBIT(FCIE_SM_STS, BIT11);	
		
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i<<12);
		u32_err = eMMC_CMD21();
		if(eMMC_ST_SUCCESS==u32_err && eMMC_ST_SUCCESS==eMMCTest_BlkWRC_ProbeTiming(eMMC_TEST_BLK_0)) 
		{
			u16_SkewCnt++;
			u32_Skew4Result |= (1<<u32_i);
		}
		else
		{	//u32_err = eMMC_FCIE_Init();
		    eMMC_debug(0,1,"failed skew4: %u\n", u32_i);
			u32_err = eMMC_FCIE_ResetToHS200((U8)g_eMMCDrv.u16_ClkRegVal);
			if(eMMC_ST_SUCCESS != u32_err)
			    break;
		}
	}

	for(u32_i=PLL_SKEW4_CNT; u32_i<PLL_SKEW4_CNT*2; u32_i++) 
	{
		// make sure a complete outside clock cycle
		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); 

		g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_TUNING_TTABLE; // turn on tunning flag
		REG_FCIE_SETBIT(FCIE_SM_STS, BIT11);

		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, (u32_i-PLL_SKEW4_CNT)<<12);
		u32_err = eMMC_CMD21();
		if(eMMC_ST_SUCCESS==u32_err && eMMC_ST_SUCCESS==eMMCTest_BlkWRC_ProbeTiming(eMMC_TEST_BLK_0)) 
		{
			u16_SkewCnt++;
			u32_Skew4Result |= (1<<u32_i);
		}
		else
		{	//u32_err = eMMC_FCIE_Init();
		    eMMC_debug(0,1,"failed skew4: %u\n", u32_i);
			u32_err = eMMC_FCIE_ResetToHS200((U8)g_eMMCDrv.u16_ClkRegVal);
			if(eMMC_ST_SUCCESS != u32_err)
			    break;
		}
	}

	g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_TUNING_TTABLE; // turn off tunning flag
	
	if(0==u32_Skew4Result || u16_SkewCnt<MIN_OK_SKEW_CNT)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: Clk: %uMHz SkewResult: %Xh SkewCnt: %u\n", 
			g_eMMCDrv.u32_ClkKHz, u32_Skew4Result, u16_SkewCnt);
		return eMMC_ST_ERR_SKEW4;
	}

	return eMMC_FCIE_DetectTiming_Ex(u32_Skew4Result);
}


U32 eMMC_FCIE_BuildHS200TimingTable(void)
{
	U32 u32_err, u32_ret;
	U8  u8_SetIdx, u8_ClkIdx;

	eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC Info: building HS200 table, please wait... \n");
	memset((void*)&g_eMMCDrv.TimingTable_t, 0, sizeof(g_eMMCDrv.TimingTable_t));
	u8_SetIdx = 0;

	// --------------------------------
	// tuning FCIE & macro for HS200
	for(u8_ClkIdx=0; u8_ClkIdx<eMMC_FCIE_VALID_CLK_CNT; u8_ClkIdx++)
	{
		u32_err = eMMC_FCIE_ResetToHS200(gau8_eMMCPLLSel_200[u8_ClkIdx]);
		if(eMMC_ST_SUCCESS != u32_err)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: eMMC_FCIE_ResetToHS200 fail: %Xh\n", u32_err);
			return u32_err;
		}
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"\nclk: %uMHz\n", g_eMMCDrv.u32_ClkKHz/1000);
				
		// ---------------------------
		// search and set the Windows
		u32_err = eMMC_FCIE_DetectHS200Timing();
		// ---------------------------
		// set the Table
		if(eMMC_ST_SUCCESS == u32_err)
		{	
			//eMMC_FCIE_Init();
			g_eMMCDrv.TimingTable_t.u8_SetCnt++;
			g_eMMCDrv.TimingTable_t.Set[0].u8_Clk = gau8_eMMCPLLSel_200[u8_ClkIdx];
			g_eMMCDrv.TimingTable_t.Set[0].u8_Reg2Ch = sgTSetTmp_t.u8_Reg2Ch;
			g_eMMCDrv.TimingTable_t.Set[0].u8_Skew4 = sgTSetTmp_t.u8_Skew4;
			g_eMMCDrv.TimingTable_t.Set[0].u32_ScanResult = sgTSetTmp_t.u32_ScanResult;
            g_eMMCDrv.TimingTable_t.u32_VerNo = eMMC_TIMING_TABLE_VERSION;
			eMMC_DumpTimingTable();
	        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\nOK\n");

			g_eMMCDrv.TimingTable_t.u32_ChkSum =
		        eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, 
		        sizeof(g_eMMCDrv.TimingTable_t)-eMMC_TIMING_TABLE_CHKSUM_OFFSET);
		    memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t));

			eMMC_FCIE_ErrHandler_ReInit();
			
			u32_err = eMMC_CMD24(eMMC_HS200TABLE_BLK_0, gau8_eMMC_SectorBuf);
	        u32_ret = eMMC_CMD24(eMMC_HS200TABLE_BLK_1, gau8_eMMC_SectorBuf);
	        if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	        {
		        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: Save TTable fail: %Xh %Xh\n",
			        u32_err, u32_ret);
		        return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	        }
			return eMMC_ST_SUCCESS;
		}		
	}	

	eMMC_DumpTimingTable();
	eMMC_die("");
	return eMMC_ST_ERR_SKEW4;
}

#endif


//=============================================================
#if (defined(eMMC_DRV_AGATE_LINUX)&&eMMC_DRV_AGATE_LINUX) // [FIXME] clone for your flatform
// [FIXME] -->
void eMMC_DumpPadClk(void)
{
        U16 u16dbg;
	U16 reg_table[0x80]={0};
	int i,j;


	//print all
	for (i = 0; i < 0x80; i++)
	{
		REG_FCIE_R( GET_REG_ADDR(FCIE_REG_BASE_ADDR, i ), u16dbg);
		reg_table[i]=u16dbg;
	}

	printk("\n------------------------------------------------------------------------------------\n");
	printk("1113  00/8 01/9 02/A 03/B 04/C 05/D 06/E 07/F\n");
	for (j = 0; j < 8*2; j++)
	{
		printk("%04X: ", j*0x8 );
		for (i = 0; i < 0x8; i++)
		{ printk("%04X ", reg_table[j*0x8+i] );
		}
		printk("\n");
	}
	printk("\n------------------------------------------------------------------------------------\n");


	//print all
	for (i = 0; i < 0x80; i++)
	{
		REG_FCIE_R( GET_REG_ADDR(PAD_CHIPTOP_BASE, i), u16dbg);
		reg_table[i]=u16dbg;
	}

	printk("\n------------------------------------------------------------------------------------\n");
	printk("101E  00/8 01/9 02/A 03/B 04/C 05/D 06/E 07/F\n");
	for (j = 0; j < 8*2; j++)
	{
		printk("%04X: ", j*0x8 );
		for (i = 0; i < 0x8; i++)
		{ printk("%04X ", reg_table[j*0x8+i] );
		}
		printk("\n");
	}
	printk("\n------------------------------------------------------------------------------------\n");


	//print all
	for (i = 0; i < 0x80; i++)
	{
		REG_FCIE_R( GET_REG_ADDR(CLKGEN_BASE, i), u16dbg);
		reg_table[i]=u16dbg;
	}

	printk("\n------------------------------------------------------------------------------------\n");
	printk("100B  00/8 01/9 02/A 03/B 04/C 05/D 06/E 07/F\n");
	for (j = 0; j < 8*2; j++)
	{
		printk("%04X: ", j*0x8 );
		for (i = 0; i < 0x8; i++)
		{ printk("%04X ", reg_table[j*0x8+i] );
		}
		printk("\n");
	}
	printk("\n------------------------------------------------------------------------------------\n");



	//----------------------------------------------
	eMMC_debug(0, 1, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
	eMMC_debug(0, 1, "FCIE 1X (0x%X):0x%X\n", reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X));
	eMMC_debug(0, 1, "FCIE 4X (0x%X):0x%X\n", reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));
	eMMC_debug(0, 1, "CMU (0x%X):0x%X\n", reg_ckg_CMU, REG_FCIE_U16(reg_ckg_CMU));

	//----------------------------------------------
	eMMC_debug(0, 1, "\n[pad setting]: ");
	switch(g_eMMCDrv.u8_PadType)
	{
	case FCIE_eMMC_DDR:
		eMMC_debug(0,0,"DDR\n");  break;
	case FCIE_eMMC_SDR:
		eMMC_debug(0,0,"SDR\n");  break;
	case FCIE_eMMC_BYPASS:
		eMMC_debug(0,0,"BYPASS\n");  break;
	default:
		eMMC_debug(0,0,"eMMC Err: Pad unknown, %Xh\n", g_eMMCDrv.u8_PadType);  eMMC_die("\n");
	}

	eMMC_debug(0, 1, "chiptop_0x64 (0x%X):0x%X\n", reg_chiptop_0x64, REG_FCIE_U16(reg_chiptop_0x64));
	eMMC_debug(0, 1, "chiptop_0x6F (0x%X):0x%X\n\n",reg_chiptop_0x6F, REG_FCIE_U16(reg_chiptop_0x6F));
	eMMC_debug(0, 1, "chiptop_0x40 (0x%X):0x%X\n", reg_chiptop_0x40, REG_FCIE_U16(reg_chiptop_0x40));
	eMMC_debug(0, 1, "chiptop_0x10 (0x%X):0x%X\n", reg_chiptop_0x10, REG_FCIE_U16(reg_chiptop_0x10));
	eMMC_debug(0, 1, "chiptop_0x6E (0x%X):0x%X\n", reg_chiptop_0x6E, REG_FCIE_U16(reg_chiptop_0x6E));
	eMMC_debug(0, 1, "chiptop_0x5A (0x%X):0x%X\n", reg_chiptop_0x5A, REG_FCIE_U16(reg_chiptop_0x5A));
	eMMC_debug(0, 1, "chiptop_0x50 (0x%X):0x%X\n", reg_chiptop_0x50, REG_FCIE_U16(reg_chiptop_0x50));

	eMMC_debug(0, 1, "\n");
}

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT_MACRO_EN|BIT_SD_DDR_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS|BIT_SD_FROM_TMUX);

	switch(u32_FCIE_IF_Type){
		case FCIE_eMMC_DDR:

			eMMC_debug(0,1,"eMMC pads: DDR \n");

			REG_FCIE_SETBIT(reg_chiptop_0x40,	BIT0);
			REG_FCIE_CLRBIT(reg_chiptop_0x10, BIT8 );

			// set DDR mode
			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_MACRO_EN|BIT_SD_DDR_EN);
			// set chiptop
			g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;

			break;

		case FCIE_eMMC_SDR:
          eMMC_debug(0,1,"eMMC pads: SDR  \n");

			REG_FCIE_SETBIT(reg_chiptop_0x40, BIT0);
			REG_FCIE_CLRBIT(reg_chiptop_0x10, BIT8);	//clear reg_fcie2macro_sd_bypass

			// set SDR mode

			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_MACRO_EN);	//b8
			g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
			break;


		case FCIE_eMMC_BYPASS:
			eMMC_debug(0,1,"eMMC pads: BYPASS \n");

			REG_FCIE_SETBIT(reg_chiptop_0x40,	BIT0);
			REG_FCIE_SETBIT(reg_chiptop_0x10, BIT8 );

			// set bypass mode
			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_MACRO_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS);	//b8,b10,b11
			g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
			break;

		default:
			//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
			return eMMC_ST_ERR_INVALID_PARAM;
	}

	// set chiptop



	REG_FCIE_CLRBIT(reg_chiptop_0x40, BIT5 );	//
	REG_FCIE_CLRBIT(reg_chiptop_0x50, BIT_ALL_PAD_IN);	//
	REG_FCIE_CLRBIT(reg_chiptop_0x5A, BIT8|BIT9|BIT10|BIT11);	//
	REG_FCIE_CLRBIT(reg_chiptop_0x64, BIT0|BIT3|BIT4);	//
	REG_FCIE_W(reg_chiptop_0x6E, BIT_eMMC_CFG_MODE2 );	//	[7:6]=3
	REG_FCIE_W(reg_chiptop_0x6F, 0x00);	//
	REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT_NEW_DESIGN_ENn);	//


	return eMMC_ST_SUCCESS;
}



//agate has only clock 4x_div
//static U8 sgau8_FCIEClk_1X_To_4X_[0x10]= // index is 1X reg value
//{0, BIT_FCIE_CLK4X_20M, BIT_FCIE_CLK4X_27M, 0,
// BIT_FCIE_CLK4X_36M, BIT_FCIE_CLK4X_40M, 0, 0,
// 0, 0, 0, 0,
// 0, 0, 0, BIT_FCIE_CLK4X_48M};


U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	switch(u16_ClkParam)	{

	case BIT_FCIE_CLK_300K:  g_eMMCDrv.u32_ClkKHz = 300;    break;
	case BIT_FCIE_CLK_XTAL:  g_eMMCDrv.u32_ClkKHz = 12000;  break;
	case BIT_FCIE_CLK_20M:   g_eMMCDrv.u32_ClkKHz = 20000;  break;
  case BIT_FCIE_CLK_27M:   g_eMMCDrv.u32_ClkKHz = 27000;  break;
	case BIT_FCIE_CLK_32M:   g_eMMCDrv.u32_ClkKHz = 32000;  break;
  case BIT_FCIE_CLK_36M:   g_eMMCDrv.u32_ClkKHz = 36000;  break;
	case BIT_FCIE_CLK_40M:   g_eMMCDrv.u32_ClkKHz = 40000;  break;
  case BIT_FCIE_CLK_43_2M: g_eMMCDrv.u32_ClkKHz = 43200;  break;
  case BIT_FCIE_CLK_48M:   g_eMMCDrv.u32_ClkKHz = 48000;  break;
	default:
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: invalid clk: %Xh\n", u16_ClkParam);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_SEL|(u16_ClkParam<<BIT_FCIE_CLK_SHIFT));
	REG_FCIE_SETBIT(reg_chiptop_0x40,	BIT1);


  REG_FCIE_CLRBIT(reg_ckg_fcie_4X,	BIT_FCIE_CLK4X_Gate);

	if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
	{
			eMMC_debug(0,0,"SET DDR CLOCK");

	    REG_FCIE_SETBIT(reg_chiptop_0x40,	reg_clk4x_div_en);

			//REG_FCIE_CLRBIT(FCIE_PATH_CTRL, BIT_SD_EN);
			//REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
			//REG_FCIE_SETBIT(reg_chip_dummy1,BIT_DDR_TIMING_PATCH);
	    //REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z_EN);
			//REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z);
			//eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
	    //REG_FCIE_CLRBIT(reg_chip_dummy1,BIT_SW_RST_Z);
			//REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_SD_EN);

			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
	}

	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;


	eMMC_debug(0, 1, "clk:%uKHz, Param:%Xh, fcie_1X(%Xh):%Xh, fcie_4X(%Xh):%Xh, chiptop_40h(%Xh):%Xh\n",
		g_eMMCDrv.u32_ClkKHz, u16_ClkParam,
		reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X),
		reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X),
		reg_chiptop_0x40, REG_FCIE_U16(reg_chiptop_0x40) );

    eMMC_PlatformResetPost();

	return eMMC_ST_SUCCESS;
}

//v
U32 eMMC_clock_gating(void)
{
	eMMC_PlatformResetPre();
	g_eMMCDrv.u32_ClkKHz = 0;
	REG_FCIE_W(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate);
	REG_FCIE_W(reg_ckg_fcie_4X, BIT_FCIE_CLK4X_Gate);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_40M,
	BIT_FCIE_CLK_36M,
	BIT_FCIE_CLK_27M,
	BIT_FCIE_CLK_20M,
};


//---------------------------------------
#if defined(ENABLE_eMMC_INTERRUPT_MODE)&&ENABLE_eMMC_INTERRUPT_MODE

static DECLARE_WAIT_QUEUE_HEAD(fcie_wait);
static volatile U32 fcie_int = 0;

#define eMMC_IRQ_DEBUG    1

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
	volatile u16 u16_Events;

	if((REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE) != BIT_EMMC_ACTIVE)
	{
		return IRQ_NONE;
	}

	// one time enable one bit
	u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

	if(u16_Events & BIT_MIU_LAST_DONE)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CARD_DMA_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
	else if(u16_Events & BIT_SD_CMD_END)
	{
		REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_CMD_END);

		fcie_int = 1;
		wake_up(&fcie_wait);
		return IRQ_HANDLED;
	}

	#if eMMC_IRQ_DEBUG
	if(0==fcie_int)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: Int St:%Xh, En:%Xh, Evt:%Xh \n",
		    REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN), u16_Events);
	#endif

	return IRQ_NONE;
}


U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
{
	U32 u32_i=0;

	#if eMMC_IRQ_DEBUG
	U32 u32_isr_tmp[2];
	unsigned long long u64_jiffies_tmp, u64_jiffies_now;
	struct timeval time_st;
	time_t sec_tmp;
	suseconds_t us_tmp;

	u32_isr_tmp[0] = fcie_int;
	do_gettimeofday(&time_st);
	sec_tmp = time_st.tv_sec;
	us_tmp = time_st.tv_usec;
	u64_jiffies_tmp = jiffies_64;
	#endif

	//----------------------------------------
	if(wait_event_timeout(fcie_wait, (fcie_int == 1), usecs_to_jiffies(u32_MicroSec)) == 0)
    {
		#if eMMC_IRQ_DEBUG
		u32_isr_tmp[1] = fcie_int;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh, ISR:%u->%u->%u \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN),
			u32_isr_tmp[0], u32_isr_tmp[1], fcie_int);

		do_gettimeofday(&time_st);
		u64_jiffies_now = jiffies_64;
	    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
		    " PassTime: %lu s, %lu us, %llu jiffies.  WaitTime: %u us, %lu jiffies, HZ:%u.\n",
		    time_st.tv_sec-sec_tmp, time_st.tv_usec-us_tmp, u64_jiffies_now-u64_jiffies_tmp,
		    u32_MicroSec, usecs_to_jiffies(u32_MicroSec), HZ);
		#else
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN));
		#endif

		// switch to polling
        for(u32_i=0; u32_i<u32_MicroSec; u32_i++)
        {
            if((REG_FCIE(u32_RegAddr) & u16_WaitEvent) == u16_WaitEvent )
                break;

            eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        }

		if(u32_i == u32_MicroSec)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: events lose, WaitEvent: %Xh \n", u16_WaitEvent);
			eMMC_DumpDriverStatus();  eMMC_DumpPadClk();
		    eMMC_FCIE_DumpRegisters();eMMC_FCIE_DumpDebugBus();
			return eMMC_ST_ERR_INT_TO;
		}
		else
		{	REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
		}

    }

	//----------------------------------------
	if(u16_WaitEvent & BIT_MIU_LAST_DONE)
	{
		for(u32_i=0; u32_i<TIME_WAIT_1_BLK_END; u32_i++)
		{
			if(REG_FCIE(u32_RegAddr) & BIT_CARD_DMA_END)
				break; // should be very fase
			eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
		}

		if(TIME_WAIT_1_BLK_END == u32_i)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no CARD_DMA_END\n");
			eMMC_FCIE_ErrHandler_Stop();
		}
	}

	//----------------------------------------
	fcie_int = 0;
	return eMMC_ST_SUCCESS;
}


#endif


int mstar_mci_Housekeep(void *pData)
{
	#if !(defined(eMMC_HOUSEKEEP_THREAD) && eMMC_HOUSEKEEP_THREAD)
    return 0;
	#endif

	#if eMMC_PROFILE_WR
	g_eMMCDrv.u32_CNT_MinRBlk = 0xFFFFFFFF;
	g_eMMCDrv.u32_CNT_MinWBlk = 0xFFFFFFFF;
	#endif

	while(1)
	{
		#if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
		msleep(1000);
		eMMC_dump_WR_Count();
		#endif

		if(kthread_should_stop())
			break;
	}

	return 0;
}


int mstar_mci_exit_checkdone_Ex(void)
{
    U16 u16Reg = 0;

    u16Reg = REG_FCIE(FCIE_SD_STATUS);

    if(u16Reg & BIT_SD_D0)
        return 1;
    else
        return 0;
}

bool mstar_mci_exit_checkdone_ForCI(void)
{
	U32 u32_Cnt=0;

    // eagle eMMC_mode_1 share pins
	if(BIT_eMMC_CFG_MODE1 != (REG_FCIE(reg_chiptop_0x6E)&BIT_eMMC_CFG_MASK))
		return true;

	REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*3)
		{
			REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
			eMMC_debug(0,1,"eMMC Info: PCM check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			schedule_timeout(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	return true;
}

EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForCI);


bool mstar_mci_exit_checkdone_ForSD(void)
{
	U32 u32_Cnt=0;

    // eagle SD_mode_x does not share pins
	//if(BIT_eMMC_CFG_MODE2 == (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
	//	return TRUE;
    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*3)
		{
			REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
			eMMC_debug(0,1,"eMMC Info: SD check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			schedule_timeout(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	return true;
}
EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForSD);


U32 mstar_SD_CardChange(void)
{
	return 0;
}

//---------------------------------------

extern struct semaphore	PfModeSem;
void eMMC_LockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	down(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);

    // Temp patch for someone driver overwrite the register
    // The patch for SD card SD_CDZ issue (Eagle).
	//REG_FCIE_SETBIT(reg_chiptop_0x1F, BIT0);

	REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	REG_FCIE_SETBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
	eMMC_pads_switch(g_eMMCDrv.u8_PadType);
	#endif
}

void eMMC_UnlockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	REG_FCIE_CLRBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);
	up(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);
}

//---------------------------------------

U32 eMMC_PlatformResetPre(void)
{
	/**((volatile unsigned short *)(0x25007DCC))|=0x02;	// emi mask
	*((volatile unsigned short *)(0x25007C18))|=0x02;	// imi0 mask
	*((volatile unsigned short *)(0x25007C58))|=0x02;	// imi1 mask
	*/
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	/**((volatile unsigned short *)(0x25007DCC))&=(~0x02);	// emi unmask
	*((volatile unsigned short *)(0x25007C18))&=(~0x02);	// imi0 unmask
	*((volatile unsigned short *)(0x25007C58))&=(~0x02);	// imi1 unmask
	*/
	return eMMC_ST_SUCCESS;
}


U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_DEFAULT_PAD);

	#if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
    //eMMC_clock_setting(FCIE_SLOW_CLK);
    eMMC_clock_setting(FCIE_SLOWEST_CLK);
	#else
	// as usual
	eMMC_clock_setting(FCIE_SLOWEST_CLK);
	#endif

	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformDeinit(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_StartSector += BL_BLK_OFFSET;
		break;

	case eMMC_PART_OTP:
		u32_StartSector += OTP_BLK_OFFSET;
		break;

	case eMMC_PART_SECINFO:
		u32_StartSector += SecInfo_BLK_OFFSET;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"SecAddr: %Xh, SecCnt: %Xh\n", u32_StartSector, u32_SectorCnt);

	if(eMMC_BOOT_PART_W == u8_OP)
		return eMMC_WriteBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);
	else
		return eMMC_ReadBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);

}


U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
	U32 u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end;

	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_eMMCBlkAddr_start = 0;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+BL_BLK_CNT-1;
		break;

	case eMMC_PART_OTP:
		u32_eMMCBlkAddr_start = OTP_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+OTP_BLK_CNT-1;
		break;

	case eMMC_PART_SECINFO:
		u32_eMMCBlkAddr_start = SecInfo_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+SecInfo_BLK_CNT-1;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"BlkAddr_start: %Xh, BlkAddr_end: %Xh\n",
		u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);

    return eMMC_EraseBootPart(u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end, 1);

}


char *gpas8_eMMCPartName[]={
	"e2pbak", "nvrambak", "hwcfgs", "recovery", "os",
	"fdd", "tdd", "blogo", "apanic","misc", "cus",
	"e2p0","e2p1","nvram0","nvram1", "system","cache", "data", "internal sd"};

// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

U32 eMMC_CheckIfMemCorrupt(void)
{
	if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
		return eMMC_ST_ERR_MEM_CORRUPT;

	return eMMC_ST_SUCCESS;
}
// <-- [FIXME]

// ==================================================
#elif (defined(eMMC_DRV_AMBER3_LINUX)&&eMMC_DRV_AMBER3_LINUX)
void eMMC_DumpPadClk(void)
{
	U16 u16_i, u16_reg;

	//----------------------------------------------
	eMMC_debug(0, 1, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
	eMMC_debug(0, 1, "FCIE 1X (0x%X):0x%X\n", reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X));
	eMMC_debug(0, 1, "MIU (0x%X):0x%X\n", reg_ckg_MIU, REG_FCIE_U16(reg_ckg_MIU));
	eMMC_debug(0, 1, "MCU (0x%X):0x%X\n", reg_ckg_MCU, REG_FCIE_U16(reg_ckg_MCU));

	eMMC_DumpChiptop();

	//----------------------------------------------
	eMMC_debug(0, 1, "\n[pad setting]: ");
	switch(g_eMMCDrv.u8_PadType)
	{
	case FCIE_eMMC_DDR:
		eMMC_debug(0,0,"DDR\n");  break;
	case FCIE_eMMC_SDR:
		eMMC_debug(0,0,"SDR\n");  break;
	case FCIE_eMMC_BYPASS:
		eMMC_debug(0,0,"BYPASS\n");  break;
	default:
		eMMC_debug(0,0,"eMMC Err: Pad unknown, %Xh\n", g_eMMCDrv.u8_PadType);  //eMMC_die("\n");
	}

	eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n CHIPTOP:");
	for(u16_i=0 ; u16_i<0x80; u16_i++)
	{
		if(0 == u16_i%8)
			eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n%02Xh:| ", u16_i);

		REG_FCIE_R(GET_REG_ADDR(PAD_CHIPTOP_BASE, u16_i), u16_reg);
		eMMC_debug(eMMC_DEBUG_LEVEL, 0, "%04Xh ", u16_reg);
	}

	eMMC_debug(0, 1, "\n");
}


static U16 sgu16_eMMCCfg_reg10h=0xFFFF;
U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	// avoid possible glitch in clock
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	// keep mode 1 or 2
	if(0xFFFF == sgu16_eMMCCfg_reg10h)
		REG_FCIE_R(reg_chiptop_0x10, sgu16_eMMCCfg_reg10h);

	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,
		BIT_MACRO_EN|BIT_SD_DDR_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS|BIT_SD_FROM_TMUX);

	REG_FCIE_CLRBIT(reg_chiptop_0x6E, BIT_emmc_config_MASK);

	switch(u32_FCIE_IF_Type){
		case FCIE_eMMC_DDR:
			panic("%s: eMMC A3 can't support DDR mode \n", __func__);
			break;

		case FCIE_eMMC_SDR:
            panic("%s: eMMC A3 can't support DDR mode \n", __func__);
			break;

		case FCIE_eMMC_BYPASS:
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"eMMC pads: BYPASS\n");
			if(sgu16_eMMCCfg_reg10h & BIT_force_boot_from_emmc2)
				REG_FCIE_CLRBIT(reg_chiptop_0x1C, BIT_disable_emmc2_pad);

			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_SD_BYPASS_MODE_EN);

			//REG_FCIE_CLRBIT(reg_chiptop_0x6E, BIT_emmc_config_MASK);
			//REG_FCIE_SETBIT(reg_chiptop_0x6E, BIT_emmc_config_bypass);

			g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
			break;

		case FCIE_eMMC_DISABLE:
			//REG_FCIE_CLRBIT(reg_chiptop_0x6E, BIT_emmc_config_MASK);

			if(sgu16_eMMCCfg_reg10h & BIT_force_boot_from_emmc2)
				REG_FCIE_SETBIT(reg_chiptop_0x1C, BIT_disable_emmc2_pad);

			g_eMMCDrv.u8_PadType = FCIE_eMMC_DISABLE;
			break;

		default:
			//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
			return eMMC_ST_ERR_INVALID_PARAM;
	}

	return eMMC_ST_SUCCESS;
}

#if defined(CONFIG_MSTAR_SDMMC)
extern void enable_top_sdpad(u8 enable);
#endif
void enable_top_emmcpad(u8 enable)
{
    if (enable)
    {
        eMMC_pads_switch(FCIE_eMMC_BYPASS);
    }
    else
    {
        eMMC_pads_switch(FCIE_eMMC_DISABLE);
    }
}

extern u32 mstar_mci_WaitD0High(u32 u32_us);

void emmc_bus_obtain(u8 select)
{
    if (select)   // select emmc
    {
		REG_FCIE_W(FCIE_MIE_INT_EN, 0);

        #if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
        enable_top_sdpad(0);
		eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
		enable_top_emmcpad(1);
        #endif

		REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
		REG_FCIE_SETBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);
    }
    else // release
    {
		REG_FCIE_CLRBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);

		#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
		// mode 1, SD/eMMC share pin
		if(0 == (sgu16_eMMCCfg_reg10h & BIT_force_boot_from_emmc2))
		if(eMMC_ST_SUCCESS != mstar_mci_WaitD0High(TIME_WAIT_DAT0_HIGH))
		{
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: wait D0 H TO\n");
		    eMMC_FCIE_ErrHandler_Stop();
		}

		enable_top_emmcpad(0);
        enable_top_sdpad(1);
        #endif

		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
		REG_FCIE_W(FCIE_MIU_DMA_26_16, 0);
    }
}


U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam)	{
	case BIT_FCIE_CLK_300K:  g_eMMCDrv.u32_ClkKHz = 300;    break;
	case BIT_FCIE_CLK_12M:   g_eMMCDrv.u32_ClkKHz = 12000;  break;
	case BIT_FCIE_CLK_20M:   g_eMMCDrv.u32_ClkKHz = 20000;  break;
    case BIT_FCIE_CLK_27M:   g_eMMCDrv.u32_ClkKHz = 27000;  break;
	case BIT_FCIE_CLK_32M:   g_eMMCDrv.u32_ClkKHz = 32000; break;
    case BIT_FCIE_CLK_36M:   g_eMMCDrv.u32_ClkKHz = 36000;  break;
	case BIT_FCIE_CLK_40M:   g_eMMCDrv.u32_ClkKHz = 40000; break;
    case BIT_FCIE_CLK_43_2M: g_eMMCDrv.u32_ClkKHz = 43200; break;
    case BIT_FCIE_CLK_48M:   g_eMMCDrv.u32_ClkKHz = 48000; break;
	default:
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: invalid clk: %Xh\n", u16_ClkParam);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_SEL|(u16_ClkParam<<BIT_FCIE_CLK_SHIFT));

	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "clk:%uKHz, Param:%Xh, fcie_1X(%Xh):%Xh\n",
		g_eMMCDrv.u32_ClkKHz, u16_ClkParam,
		reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X));

    eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_gating(void)
{
	eMMC_PlatformResetPre();
	g_eMMCDrv.u32_ClkKHz = 0;
	REG_FCIE_W(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate);
	REG_FCIE_W(reg_ckg_fcie_4X, BIT_FCIE_CLK4X_Gate);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
	BIT_FCIE_CLK_27M,
	BIT_FCIE_CLK_24M,
	BIT_FCIE_CLK_20M,
	BIT_FCIE_CLK_12M,
	BIT_FCIE_CLK_300K
};

//---------------------------------------
#if defined(ENABLE_eMMC_INTERRUPT_MODE)&&ENABLE_eMMC_INTERRUPT_MODE

static DECLARE_WAIT_QUEUE_HEAD(fcie_wait);
static volatile U32 fcie_int = 0;

#define eMMC_IRQ_DEBUG    0

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
	volatile u16 u16_Events;

	if((REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE) != BIT_EMMC_ACTIVE)
	{
		return IRQ_NONE;
	}

	// one time enable one bit
	u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

	if(u16_Events & BIT_MIU_LAST_DONE)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CARD_DMA_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
	else if(u16_Events & BIT_SD_CMD_END)
	{
		REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_CMD_END);

		fcie_int = 1;
		wake_up(&fcie_wait);
		return IRQ_HANDLED;
	}

	#if eMMC_IRQ_DEBUG
	if(0==fcie_int)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: Int St:%Xh, En:%Xh, Evt:%Xh \n",
		    REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN), u16_Events);
	#endif

	return IRQ_NONE;
}


U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
{
	U32 u32_i=0;

	#if eMMC_IRQ_DEBUG
	U32 u32_isr_tmp[2];
	unsigned long long u64_jiffies_tmp, u64_jiffies_now;
	struct timeval time_st;
	time_t sec_tmp;
	suseconds_t us_tmp;

	u32_isr_tmp[0] = fcie_int;
	do_gettimeofday(&time_st);
	sec_tmp = time_st.tv_sec;
	us_tmp = time_st.tv_usec;
	u64_jiffies_tmp = jiffies_64;
	#endif

	//----------------------------------------
	if(wait_event_timeout(fcie_wait, (fcie_int == 1), usecs_to_jiffies(u32_MicroSec)) == 0)
    {
		if(sgu16_eMMCCfg_reg10h & BIT_force_boot_from_emmc2)
		if(REG_FCIE(reg_chiptop_0x1C) & BIT_disable_emmc2_pad)
		{
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			    "eMMC Warn: SOMEONE DISABLE MY PAD !!\n");

			eMMC_DumpDriverStatus();  eMMC_DumpPadClk();
		    eMMC_FCIE_DumpRegisters();eMMC_FCIE_DumpDebugBus();
			return eMMC_ST_ERR_INT_TO;
		}

		#if eMMC_IRQ_DEBUG
		u32_isr_tmp[1] = fcie_int;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh, ISR:%u->%u->%u \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN),
			u32_isr_tmp[0], u32_isr_tmp[1], fcie_int);

		do_gettimeofday(&time_st);
		u64_jiffies_now = jiffies_64;
	    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
		    " PassTime: %lu s, %lu us, %llu jiffies.  WaitTime: %u us, %lu jiffies, HZ:%u.\n",
		    time_st.tv_sec-sec_tmp, time_st.tv_usec-us_tmp, u64_jiffies_now-u64_jiffies_tmp,
		    u32_MicroSec, usecs_to_jiffies(u32_MicroSec), HZ);
		#else
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN));
		#endif

		// switch to polling
        for(u32_i=0; u32_i<u32_MicroSec; u32_i++)
        {
            if((REG_FCIE(u32_RegAddr) & u16_WaitEvent) == u16_WaitEvent )
                break;

            eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        }

		if(u32_i == u32_MicroSec)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: events lose, WaitEvent: %Xh \n", u16_WaitEvent);
			eMMC_DumpDriverStatus();  eMMC_DumpPadClk();
		    eMMC_FCIE_DumpRegisters();eMMC_FCIE_DumpDebugBus();
			return eMMC_ST_ERR_INT_TO;
		}
		else
		{	REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
		}

    }

	//----------------------------------------
	if(u16_WaitEvent & BIT_MIU_LAST_DONE)
	{
		for(u32_i=0; u32_i<TIME_WAIT_1_BLK_END; u32_i++)
		{
			if(REG_FCIE(u32_RegAddr) & BIT_CARD_DMA_END)
				break; // should be very fase
			eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
		}

		if(TIME_WAIT_1_BLK_END == u32_i)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no CARD_DMA_END\n");
			eMMC_FCIE_ErrHandler_Stop();
		}
	}

	//----------------------------------------
	fcie_int = 0;
	return eMMC_ST_SUCCESS;
}
#endif

// now watching chiptop reg.0x1C bit.12
// and do error handling
int mstar_mci_Housekeep(void *pData)
{
	#if !(defined(eMMC_HOUSEKEEP_THREAD) && eMMC_HOUSEKEEP_THREAD)
    return 0;
	#endif

	#if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
	g_eMMCDrv.u32_CNT_MinRBlk = 0xFFFFFFFF;
	g_eMMCDrv.u32_CNT_MinWBlk = 0xFFFFFFFF;
	#endif

	while(1)
	{
		if(REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE)
		{
			if(sgu16_eMMCCfg_reg10h & BIT_force_boot_from_emmc2)
		    if(REG_FCIE(reg_chiptop_0x1C) & BIT_disable_emmc2_pad)
			if(REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE)
		    {
				eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: SOMEONE DISABLE MY PAD !!\n");
				emmc_bus_obtain(1);
				g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_ERROR_RETRY;
		    }
			msleep(500);
		}
		else
		{
			//eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC: idel\n");
		    msleep(1000);
		}

		#if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
		eMMC_dump_WR_Count();
		#endif

		if(kthread_should_stop())
			break;
	}

	return 0;
}

int mstar_mci_check_D0_status(void)
{
    u16 u16Reg = 0;

    emmc_bus_obtain(1);

    REG_FCIE_R(FCIE_SD_STATUS, u16Reg);

    emmc_bus_obtain(0); // for SD card_detect

    if( u16Reg & BIT8 )
        return 1;
    else
        return 0;

    return -1;
}
EXPORT_SYMBOL(mstar_mci_check_D0_status);

bool mstar_mci_exit_checkdone_ForCI(void)
{
	u32 u32_Cnt=0;

	if (REG_FCIE(reg_chiptop_0x10) & BIT_force_boot_from_emmc2)
        return true;

	while (mstar_mci_check_D0_status() == 0) /* card busy */
	{
	    if ((u32_Cnt++) == 10000) // 10 sec
		{
		    printk("eMMC Info: PCM check -> mstar_mci: card busy\n");
			return false;
		}
		msleep(1);	 //Use schedule_timeout for improving the CPU usage
    }
	return true;
}
EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForCI);


bool mstar_mci_exit_checkdone_ForSD(void)
{
	return mstar_mci_exit_checkdone_ForCI();
}
EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForSD);


U32 mstar_SD_CardChange(void)
{
	#if defined(CONFIG_MSTAR_SDMMC)
	if (REG_FCIE(reg_chiptop_0x10) & BIT_force_boot_from_emmc2)
        return 0;
	else
		return (U32)ms_sdmmc_card_chg(0);
	#else
    return 0;
	#endif
}

//---------------------------------------
extern struct semaphore	PfModeSem;

void eMMC_LockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,0,"%s lock..\n", pu8_str);
	down(&PfModeSem);
	emmc_bus_obtain(1);
	//eMMC_debug(0,0,"%s lock \n", pu8_str);
}

void eMMC_UnlockFCIE(U8 *pu8_str)
{

	//eMMC_debug(0,0,"%s unlock\n", pu8_str);
	emmc_bus_obtain(0);
	up(&PfModeSem);
}

//---------------------------------------

U32 eMMC_PlatformResetPre(void)
{
	/**((volatile unsigned short *)(0x25007DCC))|=0x02;	// emi mask
	*((volatile unsigned short *)(0x25007C18))|=0x02;	// imi0 mask
	*((volatile unsigned short *)(0x25007C58))|=0x02;	// imi1 mask
	*/
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	/**((volatile unsigned short *)(0x25007DCC))&=(~0x02);	// emi unmask
	*((volatile unsigned short *)(0x25007C18))&=(~0x02);	// imi0 unmask
	*((volatile unsigned short *)(0x25007C58))&=(~0x02);	// imi1 unmask
	*/
	return eMMC_ST_SUCCESS;
}

static U16 sgu16_eMMC_Deinit=0; // currently used for DisableBootMode when resume.

U32 eMMC_PlatformInit(void)
{
	if(sgu16_eMMC_Deinit)
		eMMC_DisableBootMode();

	eMMC_pads_switch(FCIE_DEFAULT_PAD);

	#if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
    //eMMC_clock_setting(FCIE_SLOW_CLK);
    eMMC_clock_setting(FCIE_SLOWEST_CLK);
	#else
	// as usual
	eMMC_clock_setting(FCIE_SLOWEST_CLK);
	#endif

	sgu16_eMMC_Deinit = 0;
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformDeinit(void)
{
	sgu16_eMMC_Deinit=1;
	return eMMC_ST_SUCCESS;
}


#define fcie_readw(addr)                reg_readl(addr)
#define fcie_writew(val, addr)          reg_writel(val, addr)
#define fcie_reg_read(host, reg)        reg_readl(((host)->baseaddr)+(reg))
#define fcie_reg_write(host, reg, val)  reg_writel(val, ((host)->baseaddr)+(reg))
#define fcie_setbits(host, reg, val)    fcie_reg_write(host, reg, (fcie_reg_read(host, reg)|val))
#define fcie_clrbits(host, reg, val)    fcie_reg_write(host, reg, (fcie_reg_read(host, reg)&~val))

U32 eMMC_DisableBootMode(void)
{
	U16 u16_reg, u16_TimeoutCount = 0;

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"\n");

	fcie_writew((fcie_readw(0x1f203d40) & (~0x8000)), 0x1f203d40);
    fcie_writew((fcie_readw(0x1f203dbc) & (~0xC0)), 0x1f203dbc);
    fcie_writew((fcie_readw(0x1f203d90) & (~0x10)), 0x1f203d90);

	REG_FCIE_CLRBIT(reg_chiptop_0x6E, 0xC0);
	REG_FCIE_SETBIT(reg_chiptop_0x6E, 0x40);
	fcie_writew((fcie_readw(0x1f203c40) | 0x2), 0x1f203c40);
	REG_FCIE_SETBIT(FCIE_TEST_MODE, 0x100);
	msleep(100);

	REG_FCIE_R(FCIE_DEBUG_BUS, u16_reg);
	//--------------------------
	if ((u16_reg != 0x1F) && (u16_reg != 0x0F))
    {
        if (u16_reg == 0x0E)
        {
			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, 0x2);

			REG_FCIE_R(FCIE_MIE_EVENT, u16_reg);

            while((u16_reg&0x8000) == 0)
            {
                if( ++u16_TimeoutCount < 1000000 )
                    udelay(1);
                else
                    panic("eMMC Err: %s: Wait MIE event timeout case 1\n",
						__FUNCTION__);
				REG_FCIE_R(FCIE_MIE_EVENT, u16_reg);
            }

			REG_FCIE_W(FCIE_JOB_BL_CNT, 0);
			REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
			REG_FCIE_W(FCIE_SD_STATUS, u16_reg);
			REG_FCIE_R(FCIE_MIE_EVENT, u16_reg);
			REG_FCIE_W(FCIE_MIE_EVENT, u16_reg);
        }

        //fcie_reg_write(pHost_st, FCIE_CARD_INT_EN, u16_reg);
        eMMC_debug(eMMC_DEBUG_LEVEL,1,"case 1 ok\n");
        return eMMC_ST_SUCCESS;
    }

	//--------------------------
	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	REG_FCIE_SETBIT(FCIE_SD_MODE, 0x200);
	udelay(100);
	REG_FCIE_W(FCIE_SD_CTRL, 0x8);

	u16_TimeoutCount = 0;
    while((REG_FCIE(FCIE_MIE_EVENT)&0x4) == 0 )
    {
        if( ++u16_TimeoutCount < 100000 )
            udelay(10);
        else
            panic("eMMC Err: %s: Wait MIE event timeout case 2\n",
				__FUNCTION__);
    }

	udelay(1000);
    REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, 0x2);

    u16_TimeoutCount = 0;
	while((REG_FCIE(FCIE_MIE_EVENT)&BIT_CARD_BOOT_DONE) == 0 )
    {
        if( ++u16_TimeoutCount < 100000 )
            udelay(10);
        else
            panic("eMMC Err: %s: Wait MIE event timeout case 3\n",
                __FUNCTION__);
    }

	REG_FCIE_W(FCIE_JOB_BL_CNT, 0);
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	REG_FCIE_W(FCIE_SD_STATUS, u16_reg);
	REG_FCIE_R(FCIE_MIE_EVENT, u16_reg);
	REG_FCIE_W(FCIE_MIE_EVENT, u16_reg);

	REG_FCIE_CLRBIT(FCIE_SD_MODE, 0x200);
    REG_FCIE_W(FCIE_SD_CTRL, 0);
    eMMC_debug(eMMC_DEBUG_LEVEL,1,"case 3 ok\n");
	return eMMC_ST_SUCCESS;
}


U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_StartSector += BL_BLK_OFFSET;
		break;

	case eMMC_PART_OTP:
		u32_StartSector += OTP_BLK_OFFSET;
		break;

	case eMMC_PART_SECINFO:
		u32_StartSector += SecInfo_BLK_OFFSET;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"SecAddr: %Xh, SecCnt: %Xh\n", u32_StartSector, u32_SectorCnt);

	if(eMMC_BOOT_PART_W == u8_OP)
		return eMMC_WriteBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);
	else
		return eMMC_ReadBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);

}


U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
	U32 u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end;

	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_eMMCBlkAddr_start = 0;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+BL_BLK_CNT-1;
		break;

	case eMMC_PART_OTP:
		u32_eMMCBlkAddr_start = OTP_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+OTP_BLK_CNT-1;
		break;

	case eMMC_PART_SECINFO:
		u32_eMMCBlkAddr_start = SecInfo_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+SecInfo_BLK_CNT-1;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"BlkAddr_start: %Xh, BlkAddr_end: %Xh\n",
		u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);

    return eMMC_EraseBootPart(u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end, 1);

}


char *gpas8_eMMCPartName[]={
	"e2pbak", "nvrambak", "hwcfgs", "recovery", "os",
	"fdd", "tdd", "blogo", "apanic","misc", "cus",
	"e2p0","e2p1","nvram0","nvram1", "system","cache", "data", "internal sd"};

// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

U32 eMMC_CheckIfMemCorrupt(void)
{
	if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
		return eMMC_ST_ERR_MEM_CORRUPT;

	return eMMC_ST_SUCCESS;
}
// <-- [FIXME]

//=============================================================
#elif (defined(eMMC_DRV_EAGLE_LINUX)&&eMMC_DRV_EAGLE_LINUX) // [FIXME] clone for your flatform
// [FIXME] -->
void eMMC_DumpPadClk(void)
{
	U16 u16_i, u16_reg;

	//----------------------------------------------
	eMMC_debug(0, 1, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
	eMMC_debug(0, 1, "FCIE 1X (0x%X):0x%X\n", reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X));
	eMMC_debug(0, 1, "FCIE 4X (0x%X):0x%X\n", reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));
	eMMC_debug(0, 1, "CMU (0x%X):0x%X\n", reg_ckg_CMU, REG_FCIE_U16(reg_ckg_CMU));

	//----------------------------------------------
	eMMC_DumpChiptop();

	eMMC_debug(0, 1, "\n[pad setting]: ");
	switch(g_eMMCDrv.u8_PadType)
	{
	case FCIE_eMMC_DDR:
		eMMC_debug(0,0,"DDR\n");  break;
	case FCIE_eMMC_SDR:
		eMMC_debug(0,0,"SDR\n");  break;
	case FCIE_eMMC_BYPASS:
		eMMC_debug(0,0,"BYPASS\n");  break;
	default:
		eMMC_debug(0,0,"eMMC Err: Pad unknown, %Xh\n", g_eMMCDrv.u8_PadType);  //eMMC_die("\n");
	}
	eMMC_debug(0, 1, "chiptop_0x5A (0x%X):0x%X\n", reg_chiptop_0x5A, REG_FCIE_U16(reg_chiptop_0x5A));
	eMMC_debug(0, 1, "chiptop_0x1F (0x%X):0x%X\n", reg_chiptop_0x1F, REG_FCIE_U16(reg_chiptop_0x1F));
	eMMC_debug(0, 1, "chiptop_0x70 (0x%X):0x%X\n", reg_chiptop_0x70, REG_FCIE_U16(reg_chiptop_0x70));
	eMMC_debug(0, 1, "chiptop_0x64 (0x%X):0x%X\n", reg_chiptop_0x64, REG_FCIE_U16(reg_chiptop_0x64));
	eMMC_debug(0, 1, "chiptop_0x4F (0x%X):0x%X\n", reg_chiptop_0x4F, REG_FCIE_U16(reg_chiptop_0x4F));
	eMMC_debug(0, 1, "chiptop_0x03 (0x%X):0x%X\n", reg_chiptop_0x03, REG_FCIE_U16(reg_chiptop_0x03));
	eMMC_debug(0, 1, "chiptop_0x51 (0x%X):0x%X\n", reg_chiptop_0x51, REG_FCIE_U16(reg_chiptop_0x51));
	eMMC_debug(0, 1, "chiptop_0x6F (0x%X):0x%X\n\n", reg_chiptop_0x6F, REG_FCIE_U16(reg_chiptop_0x6F));

	eMMC_debug(0, 1, "chiptop_0x43 (0x%X):0x%X\n", reg_chiptop_0x43, REG_FCIE_U16(reg_chiptop_0x43));
	eMMC_debug(0, 1, "chiptop_0x50 (0x%X):0x%X\n", reg_chiptop_0x50, REG_FCIE_U16(reg_chiptop_0x50));
	eMMC_debug(0, 1, "chiptop_0x0B (0x%X):0x%X\n", reg_chiptop_0x0B, REG_FCIE_U16(reg_chiptop_0x0B));
	eMMC_debug(0, 1, "chiptop_0x5D (0x%X):0x%X\n", reg_chiptop_0x5D, REG_FCIE_U16(reg_chiptop_0x5D));

	eMMC_debug(0, 1, "reg_pcm_drv (0x%X):0x%X\n",  reg_pcm_drv,  REG_FCIE_U16(reg_pcm_drv));
	eMMC_debug(0, 1, "reg_pcm_d_pe (0x%X):0x%X\n", reg_pcm_d_pe, REG_FCIE_U16(reg_pcm_d_pe));
	eMMC_debug(0, 1, "reg_nand_drv (0x%X):0x%X\n", reg_nand_drv, REG_FCIE_U16(reg_nand_drv));
	eMMC_debug(0, 1, "reg_emmc_drv (0x%X):0x%X\n", reg_emmc_drv, REG_FCIE_U16(reg_emmc_drv));

	eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n CHIPTOP:");
	for(u16_i=0 ; u16_i<0x80; u16_i++)
	{
		if(0 == u16_i%8)
			eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n%02Xh:| ", u16_i);

		REG_FCIE_R(GET_REG_ADDR(PAD_CHIPTOP_BASE, u16_i), u16_reg);
		eMMC_debug(eMMC_DEBUG_LEVEL, 0, "%04Xh ", u16_reg);
	}

	eMMC_debug(0, 1, "\n");
}

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	U16 u16_FCIE_Reg10h;

	// avoid possible glitch in clock
	REG_FCIE_R(FCIE_SD_MODE, u16_FCIE_Reg10h);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,
		BIT_MACRO_EN|BIT_SD_DDR_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS|BIT_SD_FROM_TMUX);

	switch(u32_FCIE_IF_Type){
		case FCIE_eMMC_DDR:
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"eMMC pads: DDR\n");

			REG_FCIE_SETBIT(reg_chiptop_0x43,
				BIT_PAD_EMMC_CLK_SRC|BIT_eMMC_RSTPIN_VAL);
			REG_FCIE_CLRBIT(reg_chiptop_0x43,
				BIT_EMPTY|BIT_eMMC_RSTPIN_EN|BIT_PAD_BYPASS_MACRO);

			// set DDR mode
			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_MACRO_EN|BIT_SD_DDR_EN);
			REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT_ddr_timing_patch);

			g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
			break;

		case FCIE_eMMC_SDR:
            //eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"eMMC pads: SDR\n");

			REG_FCIE_SETBIT(reg_chiptop_0x43,
				BIT_PAD_EMMC_CLK_SRC|BIT_eMMC_RSTPIN_VAL);
			REG_FCIE_CLRBIT(reg_chiptop_0x43,
				BIT_EMPTY|BIT_eMMC_RSTPIN_EN|BIT_PAD_BYPASS_MACRO);

			// set SDR mode
			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_MACRO_EN);
			g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
			break;

		case FCIE_eMMC_BYPASS:
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"eMMC pads: BYPASS\n");

			REG_FCIE_SETBIT(reg_chiptop_0x43,
				BIT_PAD_EMMC_CLK_SRC|BIT_eMMC_RSTPIN_VAL|BIT_PAD_BYPASS_MACRO);
			REG_FCIE_CLRBIT(reg_chiptop_0x43,
				BIT_EMPTY|BIT_eMMC_RSTPIN_EN);

			// set bypadd mode
			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG,
			    BIT_MACRO_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS);
			g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
			break;

		default:
			//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
			REG_FCIE_W(FCIE_SD_MODE, u16_FCIE_Reg10h);
			return eMMC_ST_ERR_INVALID_PARAM;
	}

	// set chiptop
	#if 0
	REG_FCIE_CLRBIT(reg_chiptop_0x5A, BIT0|BIT1|BIT2);
	//REG_FCIE_CLRBIT(reg_chiptop_0x1F, BIT0|BIT1); // don't touch for card-detect
	//REG_FCIE_CLRBIT(reg_chiptop_0x70, BIT2); // don't touch for card-detect
	REG_FCIE_CLRBIT(reg_chiptop_0x64, BIT0);
	REG_FCIE_CLRBIT(reg_chiptop_0x4F, BIT13|BIT14);
	REG_FCIE_CLRBIT(reg_chiptop_0x03, BIT6|BIT7);
	//REG_FCIE_CLRBIT(reg_chiptop_0x51, BIT10|BIT11);  //Cause EAGLE CI card lag issue.
	REG_FCIE_CLRBIT(reg_chiptop_0x6F, BIT14|BIT15);
    #endif

	REG_FCIE_CLRBIT(reg_chiptop_0x50, BIT_NAND_CS1_EN|BIT_ALL_PAD_IN);
	REG_FCIE_CLRBIT(reg_chiptop_0x0B, BIT_NAND_CS2_EN|BIT_NAND_CS3_EN|
		BIT_SD0_CFG|BIT_SD0_CFG2|BIT_SD1_CFG|BIT_SD1_CFG2|BIT_NAND_CFG);

	REG_FCIE_W(FCIE_SD_MODE, u16_FCIE_Reg10h);
	return eMMC_ST_SUCCESS;
}


static U8 sgau8_FCIEClk_1X_To_4X_[0x10]= // index is 1X reg value
{0, BIT_FCIE_CLK4X_20M, BIT_FCIE_CLK4X_27M, 0,
 BIT_FCIE_CLK4X_36M, BIT_FCIE_CLK4X_40M, 0, 0,
 0, 0, 0, 0,
 0, 0, 0, BIT_FCIE_CLK4X_48M};

U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam)	{
	case BIT_FCIE_CLK_300K:  g_eMMCDrv.u32_ClkKHz = 300;    break;
	case BIT_FCIE_CLK_20M:   g_eMMCDrv.u32_ClkKHz = 20000;  break;
    case BIT_FCIE_CLK_27M: g_eMMCDrv.u32_ClkKHz = 27000;  break;
	//case BIT_FCIE_CLK_32M:   g_eMMCDrv.u32_ClkKHz = 32000; break;
    case BIT_FCIE_CLK_36M: g_eMMCDrv.u32_ClkKHz = 36000;  break;
	case BIT_FCIE_CLK_40M:   g_eMMCDrv.u32_ClkKHz = 40000; break;
    //case BIT_FCIE_CLK_43_2M: g_eMMCDrv.u32_ClkKHz = 43200; break;
    case BIT_FCIE_CLK_48M:   g_eMMCDrv.u32_ClkKHz = 48000; break;
	default:
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: invalid clk: %Xh\n", u16_ClkParam);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_SEL|(u16_ClkParam<<BIT_FCIE_CLK_SHIFT));
	if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
	{
	    REG_FCIE_CLRBIT(reg_ckg_fcie_4X,
			BIT_FCIE_CLK4X_Gate|BIT_FCIE_CLK4X_MASK);
	    REG_FCIE_SETBIT(reg_ckg_fcie_4X,
			(sgau8_FCIEClk_1X_To_4X_[u16_ClkParam]<<BIT_FCIE_CLK4X_SHIFT));

		REG_FCIE_CLRBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		//REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_DDR_TIMING_PATCH);
        REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z_EN);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z);
		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        REG_FCIE_CLRBIT(reg_chip_dummy1,BIT_SW_RST_Z);
		REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
	}

	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "clk:%uKHz, Param:%Xh, fcie_1X(%Xh):%Xh, fcie_4X(%Xh):%Xh\n",
		g_eMMCDrv.u32_ClkKHz, u16_ClkParam,
		reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X),
		reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));

    eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_gating(void)
{
	eMMC_PlatformResetPre();
	g_eMMCDrv.u32_ClkKHz = 0;
	REG_FCIE_W(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate);
	REG_FCIE_W(reg_ckg_fcie_4X, BIT_FCIE_CLK4X_Gate);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_40M,
	BIT_FCIE_CLK_36M,
	BIT_FCIE_CLK_27M,
	BIT_FCIE_CLK_20M,
};

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
static char *sgachar_string[]={"  2T", "1.5T", "2.5T","  1T"};

static U32 eMMC_FCIE_DetectDDRTiming_Ex(U8 u8_DQS, U8 u8_DelaySel)
{
	U32 u32_SectorAddr;

	u32_SectorAddr = eMMC_TEST_BLK_0;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\ndqs:%s[%Xh]  cell:%02Xh \n",
		sgachar_string[u8_DQS], u8_DQS, u8_DelaySel);

	eMMC_FCIE_SetDDR48TimingReg(u8_DQS, u8_DelaySel);
    return eMMCTest_BlkWRC_ProbeTiming(u32_SectorAddr);
}

#define FCIE_DELAY_CELL_ts         700 // 0.7ns
static eMMC_FCIE_DDRT_WINDOW_t sg_DDRTWindow[2];

U32 eMMC_FCIE_DetectDDRTiming(void)
{
	U8  u8_dqs, u8_delay_sel, u8_i;
	U8  u8_dqs_prev=0xFF, u8_delay_sel_prev=0;
	U8  au8_DQSRegVal[4]={3,1,0,2}; // 1T, 1.5T, 2T, 2.5T
	U8  au8_DQS_10T[4]={10,15,20,25}; // 1T, 1.5T, 2T, 2.5T
	U8  u8_delay_Sel_max;
	U32 u32_ts;
	U32 u32_err;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow = &sg_DDRTWindow[0];

	g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_TUNING_TTABLE; // just to turn off some log

	//--------------------------------------------------
	sg_DDRTWindow[0].u8_Cnt = 0;
	sg_DDRTWindow[0].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[0].aParam[1].u8_DQS = 0;
    sg_DDRTWindow[1].u8_Cnt = 0;
	sg_DDRTWindow[1].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[1].aParam[1].u8_DQS = 0;
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
	{
		sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]=0;
		sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]=0;
	}

	//--------------------------------------------------
	// calculate delay_Sel_max
	u32_ts = 1000*1000*1000 / g_eMMCDrv.u32_ClkKHz;
	u32_ts >>= 2; // for 4X's 1T

    //--------------------------------------------------
    // no need to try DQS of no delay
	for(u8_dqs=1; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n---------------------------\n");

		if(u8_dqs < (BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT))
		    u8_delay_Sel_max =
		        (au8_DQS_10T[u8_dqs+1]-au8_DQS_10T[u8_dqs])
		        *u32_ts/(FCIE_DELAY_CELL_ts*10);
		else
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);

		if(u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT))
		{
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n", u8_delay_Sel_max);
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"invalid, not try\n\n");
			//continue;
		}
		else
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n\n", u8_delay_Sel_max);

		sg_DDRTWindow[0].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;
		sg_DDRTWindow[1].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;

		//for(u8_delay_sel=0; u8_delay_sel<=(BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT); u8_delay_sel++)
		for(u8_delay_sel=0; u8_delay_sel<=u8_delay_Sel_max; u8_delay_sel++)
		{
			u32_err = eMMC_FCIE_DetectDDRTiming_Ex(au8_DQSRegVal[u8_dqs], u8_delay_sel);
			if(eMMC_ST_SUCCESS == u32_err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"  PASS\n\n");
				pWindow->u8_Cnt++;
				if(0 == pWindow->aParam[0].u8_DQS) // save the window head
				{
					pWindow->aParam[0].u8_DQS = u8_dqs; // dqs uses index
					pWindow->aParam[0].u8_Cell = u8_delay_sel;
				}
				pWindow->au8_DQSValidCellCnt[u8_dqs]++;

				u8_dqs_prev = u8_dqs;
				u8_delay_sel_prev = u8_delay_sel;
			}
			else
			{   // save the window tail
				if(0xFF != u8_dqs_prev)
				{
					pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
					pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
				}
				u8_dqs_prev = 0xFF;

				// discard & re-use the window having less PASS cnt
				pWindow =
				    (sg_DDRTWindow[0].u8_Cnt < sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];
				pWindow->u8_Cnt = 0;
				pWindow->aParam[0].u8_DQS = 0;
				pWindow->aParam[1].u8_DQS = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
					pWindow->au8_DQSValidCellCnt[u8_i]=0;
			}
		}
	}

	// for the case of last try is ok
	if(0xFF != u8_dqs_prev)
	{
		pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
		pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
	}

	g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_TUNING_TTABLE;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W0, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[0].u8_Cnt,
		sg_DDRTWindow[0].aParam[0].u8_DQS, sg_DDRTWindow[0].aParam[0].u8_Cell,
		sg_DDRTWindow[0].aParam[1].u8_DQS, sg_DDRTWindow[0].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]);

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W1, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[1].u8_Cnt,
		sg_DDRTWindow[1].aParam[0].u8_DQS, sg_DDRTWindow[1].aParam[0].u8_Cell,
		sg_DDRTWindow[1].aParam[1].u8_DQS, sg_DDRTWindow[1].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]);

	if(sg_DDRTWindow[0].u8_Cnt || sg_DDRTWindow[1].u8_Cnt)
		return eMMC_ST_SUCCESS;
	else
		return eMMC_ST_ERR_NO_OK_DDR_PARAM;

}


U32 eMMC_FCIE_BuildDDRTimingTable(void)
{
	U8 au8_DQSRegVal[4]={3,1,0,2}; // 0T, 1.5T, 2T, 2.5T
	U8  u8_i, u8_ClkIdx, u8_SetIdx, u8_tmp, u8_DqsIdx=0, u8_CellBase;
	U32 u32_err, u32_ret=eMMC_ST_ERR_NO_OK_DDR_PARAM;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow;

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: building DDR table, please wait... \n");

	memset((void*)&g_eMMCDrv.TimingTable_t, '\0', sizeof(g_eMMCDrv.TimingTable_t));
	u8_SetIdx = 0;

	if(eMMC_IF_NORMAL_SDR()){
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: SDR mode can't detect DDR timing\n");
		u32_err = eMMC_FCIE_EnableFastMode_Ex(FCIE_eMMC_DDR);
		if(eMMC_ST_SUCCESS != u32_err)
		{
		    printf("eMMC Err: set DDR IF fail: %X\n", u32_err);
		    return u32_err;
		}
	}
	for(u8_ClkIdx=0; u8_ClkIdx<eMMC_FCIE_VALID_CLK_CNT; u8_ClkIdx++)
	{

		eMMC_debug(eMMC_DEBUG_LEVEL_LOW,0,"=================================\n");
		eMMC_clock_setting(gau8_FCIEClkSel[u8_ClkIdx]);
		eMMC_debug(eMMC_DEBUG_LEVEL_LOW,0,"=================================\n");

		// ---------------------------
		// search and set the Windows
		u32_err = eMMC_FCIE_DetectDDRTiming();

		// ---------------------------
		// set the Table
		if(eMMC_ST_SUCCESS == u32_err)
		{
			#if 0
			// before 12MHz, should at least 2 clk is ok for DDR
			if(eMMC_FCIE_DDRT_SET_CNT-1 == u8_SetIdx &&
				BIT_FCIE_CLK_12M == gau8_FCIEClkSel[u8_ClkIdx])
				u32_ret = eMMC_ST_SUCCESS;
			#endif

			g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk = gau8_FCIEClkSel[u8_ClkIdx];

			// ---------------------------
		 	// select Window
			pWindow = NULL;

			// pick up the Window of Cell=0 case
			if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS == sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[0];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS == sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[1];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

			// ---------------------------
			if(NULL != pWindow)
			{
				// pick up the DQS having max valid cell
				u8_tmp = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
				{
					if(u8_tmp <= pWindow->au8_DQSValidCellCnt[u8_i]){
						u8_tmp = pWindow->au8_DQSValidCellCnt[u8_i];
						u8_DqsIdx = u8_i;
					}
				}
				if(0 != u8_DqsIdx) // do not use 0T, this should be always TRUE
				{
					if(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] >= 3 &&
						pWindow->au8_DQSValidCellCnt[u8_DqsIdx] >= 3)
					{
						g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[u8_DqsIdx];
						g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell = 0; // nice
					}
					else
					{
						u8_tmp = (pWindow->au8_DQSValidCellCnt[u8_DqsIdx] +
							pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1]) / 2;


						if(u8_tmp < pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1])
						{
							g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx-1];

							u8_CellBase = pWindow->au8_DQSTryCellCnt[u8_DqsIdx-1] - pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1];
							g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell =
								u8_CellBase + (pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx] - u8_tmp);
						}
						else
						{   g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx];
						    g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell =
								(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx]) / 2;
						}
					}
				}
			}

			// ---------------------------
            // or, pick up the Window of large PASS Cnt
			else //if(NULL == pWindow)
			{
			    pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

				g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[pWindow->aParam[0].u8_DQS];
				g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell =
					(pWindow->aParam[0].u8_Cell + pWindow->aParam[1].u8_Cell)/2;
			}

			#if 0
			// ---------------------------
			// use 12M for Set.Min
			if(eMMC_FCIE_DDRT_SET_CNT-2 == u8_SetIdx)
				u8_ClkIdx = eMMC_FCIE_VALID_CLK_CNT-2;
			#else
			if(FCIE_SLOW_CLK == g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.TimingTable_t.u8_SetCnt = u8_SetIdx + 1;
			#endif

			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT > eMMC_FCIE_DDRT_SET_CNT ?
	    eMMC_FCIE_DDRT_SET_CNT : eMMC_FCIE_VALID_CLK_CNT;
	u8_tmp = u8_tmp > g_eMMCDrv.TimingTable_t.u8_SetCnt ?
		g_eMMCDrv.TimingTable_t.u8_SetCnt : u8_tmp;

	eMMC_DumpTimingTable();

	// ======================================================
	// CAUTION: expect 48MHz can have valid DDRT parameter
	if(g_eMMCDrv.TimingTable_t.Set[eMMC_TIMING_SET_MAX].u8_Clk == gau8_FCIEClkSel[0]
		&& 0 != g_eMMCDrv.TimingTable_t.u8_SetCnt)
		u32_ret = eMMC_ST_SUCCESS;
	else if(g_eMMCDrv.TimingTable_t.Set[eMMC_TIMING_SET_MAX].u8_Clk != gau8_FCIEClkSel[0])
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: max clk can NOT run DDR\n");
	else if(0 == g_eMMCDrv.TimingTable_t.u8_SetCnt)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: min clk can NOT run DDR\n");
	// ======================================================

	// ---------------------------
	// save DDRT Table
	if(eMMC_ST_SUCCESS == u32_ret)
	{
		g_eMMCDrv.TimingTable_t.u32_ChkSum =
		    eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-4);
		memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t));

		eMMC_FCIE_ApplyTimingSet(eMMC_TIMING_SET_MAX);

		u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
	    u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
	    if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	    {
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh %Xh\n",
			    u32_err, u32_ret);
		    return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	    }
	}
	else
	{
		gau8_eMMC_SectorBuf[0] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>>24);
		gau8_eMMC_SectorBuf[1] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>>16);
		gau8_eMMC_SectorBuf[2] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>> 8);
		gau8_eMMC_SectorBuf[3] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>> 0);

		u32_ret = eMMC_ST_ERR_BUILD_DDRT;
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: fail! please call CAE or RD for helps, %Xh \n",
			u32_ret);
		eMMC_ResetReadyFlag();
		eMMC_Init();
		eMMC_FCIE_EnableSDRMode();
	}

	return u32_ret;
}

#endif // IF_DETECT_eMMC_DDR_TIMING


//---------------------------------------
#if defined(ENABLE_eMMC_INTERRUPT_MODE)&&ENABLE_eMMC_INTERRUPT_MODE

static DECLARE_WAIT_QUEUE_HEAD(fcie_wait);
static volatile U32 fcie_int = 0;

#define eMMC_IRQ_DEBUG    1

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
	volatile u16 u16_Events;

	if((REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE) != BIT_EMMC_ACTIVE)
	{
		return IRQ_NONE;
	}

	// one time enable one bit
	u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

	if(u16_Events & BIT_MIU_LAST_DONE)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CARD_DMA_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
	else if(u16_Events & BIT_SD_CMD_END)
	{
		REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_CMD_END);

		fcie_int = 1;
		wake_up(&fcie_wait);
		return IRQ_HANDLED;
	}

	#if eMMC_IRQ_DEBUG
	if(0==fcie_int)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: Int St:%Xh, En:%Xh, Evt:%Xh \n",
		    REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN), u16_Events);
	#endif

	return IRQ_NONE;
}


U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
{
	U32 u32_i=0;

	#if eMMC_IRQ_DEBUG
	U32 u32_isr_tmp[2];
	unsigned long long u64_jiffies_tmp, u64_jiffies_now;
	struct timeval time_st;
	time_t sec_tmp;
	suseconds_t us_tmp;

	u32_isr_tmp[0] = fcie_int;
	do_gettimeofday(&time_st);
	sec_tmp = time_st.tv_sec;
	us_tmp = time_st.tv_usec;
	u64_jiffies_tmp = jiffies_64;
	#endif

	//----------------------------------------
	if(wait_event_timeout(fcie_wait, (fcie_int == 1), usecs_to_jiffies(u32_MicroSec)) == 0)
    {
		#if eMMC_IRQ_DEBUG
		u32_isr_tmp[1] = fcie_int;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh, ISR:%u->%u->%u \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN),
			u32_isr_tmp[0], u32_isr_tmp[1], fcie_int);

		do_gettimeofday(&time_st);
		u64_jiffies_now = jiffies_64;
	    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
		    " PassTime: %lu s, %lu us, %llu jiffies.  WaitTime: %u us, %lu jiffies, HZ:%u.\n",
		    time_st.tv_sec-sec_tmp, time_st.tv_usec-us_tmp, u64_jiffies_now-u64_jiffies_tmp,
		    u32_MicroSec, usecs_to_jiffies(u32_MicroSec), HZ);
		#else
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN));
		#endif

		// switch to polling
        for(u32_i=0; u32_i<u32_MicroSec; u32_i++)
        {
            if((REG_FCIE(u32_RegAddr) & u16_WaitEvent) == u16_WaitEvent )
                break;

            eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        }

		if(u32_i == u32_MicroSec)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: events lose, WaitEvent: %Xh \n", u16_WaitEvent);
			eMMC_DumpDriverStatus();  eMMC_DumpPadClk();
		    eMMC_FCIE_DumpRegisters();eMMC_FCIE_DumpDebugBus();
			return eMMC_ST_ERR_INT_TO;
		}
		else
		{	REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
		}

    }

	//----------------------------------------
	if(u16_WaitEvent & BIT_MIU_LAST_DONE)
	{
		for(u32_i=0; u32_i<TIME_WAIT_1_BLK_END; u32_i++)
		{
			if(REG_FCIE(u32_RegAddr) & BIT_CARD_DMA_END)
				break; // should be very fase
			eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
		}

		if(TIME_WAIT_1_BLK_END == u32_i)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no CARD_DMA_END\n");
			eMMC_FCIE_ErrHandler_Stop();
		}
	}

	//----------------------------------------
	fcie_int = 0;
	return eMMC_ST_SUCCESS;
}


#endif

int mstar_mci_Housekeep(void *pData)
{
	#if !(defined(eMMC_HOUSEKEEP_THREAD) && eMMC_HOUSEKEEP_THREAD)
    return 0;
	#endif

	#if eMMC_PROFILE_WR
	g_eMMCDrv.u32_CNT_MinRBlk = 0xFFFFFFFF;
	g_eMMCDrv.u32_CNT_MinWBlk = 0xFFFFFFFF;
	g_eMMCDrv.u32_temp_count = 0;
	#endif

	while(1)
	{
		#if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
		//msleep(1000);
		//eMMC_dump_WR_Count();
		    #if 0//defined(CONFIG_MMC_MSTAR_MMC_EMMC_LIFETEST)
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC R: %llu GB, %llu MB\n",
		    g_eMMCDrv.u64_CNT_TotalRBlk/0x200000, (g_eMMCDrv.u64_CNT_TotalRBlk>>11)%1024);
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC W: %llu GB, %llu MB\n",
		    g_eMMCDrv.u64_CNT_TotalWBlk/0x200000, (g_eMMCDrv.u64_CNT_TotalWBlk>>11)%1024);
		    #endif
		#endif

		eMMC_hw_timer_start();
		msleep(1000);
		eMMC_debug(0,0,"%u \n", eMMC_hw_timer_tick());


		if(kthread_should_stop())
			break;
	}

	return 0;
}


int mstar_mci_exit_checkdone_Ex(void)
{
    U16 u16Reg = 0;

    u16Reg = REG_FCIE(FCIE_SD_STATUS);

    if(u16Reg & BIT_SD_D0)
        return 1;
    else
        return 0;
}

bool mstar_mci_exit_checkdone_ForCI(void)
{
	U32 u32_Cnt=0;

    // eagle eMMC_mode_1 share pins
	if(BIT_eMMC_CFG_MODE1 != (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
		return true;

	REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
			eMMC_debug(0,1,"eMMC Info: PCM check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	return true;
}

EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForCI);


bool mstar_mci_exit_checkdone_ForSD(void)
{
	U32 u32_Cnt=0;

    // eagle SD_mode_x does not share pins
	if(BIT_eMMC_CFG_MODE1 != (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
		return true;

    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
			eMMC_debug(0,1,"eMMC Info: SD check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	return true;
}
EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForSD);


U32 mstar_SD_CardChange(void)
{
	#if defined(CONFIG_MSTAR_SDMMC)
	if(BIT_eMMC_CFG_MODE1 != (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
		return 0;
	else
		return (U32)ms_sdmmc_card_chg(0);
	#else
	return 0;
	#endif
}

//---------------------------------------

extern struct semaphore	PfModeSem;
extern bool ms_sdmmc_wait_d0_for_emmc(void);

void eMMC_LockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	down(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	if(false == ms_sdmmc_wait_d0_for_emmc())
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SD keep D0 low \n");
		eMMC_FCIE_ErrHandler_Stop();
	}

	eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
	eMMC_pads_switch(g_eMMCDrv.u8_PadType);
	#endif

    #if defined(CONFIG_MSTAR_SDMMC1_GPIO_PADSPI2DI)
    // Temp patch for someone driver overwrite the register
    // The patch for SD card SD_CDZ issue (Eagle).
	REG_FCIE_SETBIT(reg_chiptop_0x1F, BIT0);
	#endif

	REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	REG_FCIE_SETBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);
}

void eMMC_UnlockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	REG_FCIE_CLRBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);
	up(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);
}

//---------------------------------------

U32 eMMC_PlatformResetPre(void)
{
	/**((volatile unsigned short *)(0x25007DCC))|=0x02;	// emi mask
	*((volatile unsigned short *)(0x25007C18))|=0x02;	// imi0 mask
	*((volatile unsigned short *)(0x25007C58))|=0x02;	// imi1 mask
	*/
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	/**((volatile unsigned short *)(0x25007DCC))&=(~0x02);	// emi unmask
	*((volatile unsigned short *)(0x25007C18))&=(~0x02);	// imi0 unmask
	*((volatile unsigned short *)(0x25007C58))&=(~0x02);	// imi1 unmask
	*/
	REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT_csreg_miu_wp_last2_sel);
	return eMMC_ST_SUCCESS;
}


U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_DEFAULT_PAD);

	#if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
    //eMMC_clock_setting(FCIE_SLOW_CLK);
    eMMC_clock_setting(FCIE_SLOWEST_CLK);
	#else
	// as usual
	eMMC_clock_setting(FCIE_SLOWEST_CLK);
	#endif

	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformDeinit(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_StartSector += BL_BLK_OFFSET;
		break;

	case eMMC_PART_OTP:
		u32_StartSector += OTP_BLK_OFFSET;
		break;

	case eMMC_PART_SECINFO:
		u32_StartSector += SecInfo_BLK_OFFSET;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"SecAddr: %Xh, SecCnt: %Xh\n", u32_StartSector, u32_SectorCnt);

	if(eMMC_BOOT_PART_W == u8_OP)
		return eMMC_WriteBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);
	else
		return eMMC_ReadBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);

}


U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
	U32 u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end;

	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_eMMCBlkAddr_start = 0;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+BL_BLK_CNT-1;
		break;

	case eMMC_PART_OTP:
		u32_eMMCBlkAddr_start = OTP_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+OTP_BLK_CNT-1;
		break;

	case eMMC_PART_SECINFO:
		u32_eMMCBlkAddr_start = SecInfo_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+SecInfo_BLK_CNT-1;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"BlkAddr_start: %Xh, BlkAddr_end: %Xh\n",
		u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);

    return eMMC_EraseBootPart(u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end, 1);

}


char *gpas8_eMMCPartName[]={
	"e2pbak", "nvrambak", "hwcfgs", "recovery", "os",
	"fdd", "tdd", "blogo", "apanic","misc", "cus",
	"e2p0","e2p1","nvram0","nvram1", "system","cache", "data", "internal sd"};

// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

U32 eMMC_CheckIfMemCorrupt(void)
{
	if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
		return eMMC_ST_ERR_MEM_CORRUPT;

	return eMMC_ST_SUCCESS;
}
// <-- [FIXME]

//===================================================
#elif (defined(eMMC_DRV_EDISON_LINUX)&&eMMC_DRV_EDISON_LINUX) // [FIXME] clone for your flatform
// [FIXME] -->
U16 u16_reg_chiptop_0x10_val,u16_reg_chiptop_0x6E_val;
void eMMC_DumpPadClk(void)
{
	//----------------------------------------------
    eMMC_debug(0, 0, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
    eMMC_debug(0, 0, "FCIE 1X (0x%X):0x%04X\n", reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X));
    eMMC_debug(0, 0, "FCIE 4X (0x%X):0x%04X\n", reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));
    eMMC_debug(0, 0, "MIU (0x%X):0x%04X\n", reg_ckg_MIU, REG_FCIE_U16(reg_ckg_MIU));
    eMMC_debug(0, 0, "MCU (0x%X):0x%04X\n", reg_ckg_MCU, REG_FCIE_U16(reg_ckg_MCU));

    //----------------------------------------------
    eMMC_DumpChiptop();

    eMMC_debug(0, 0, "\n[pad setting]: ");
    switch(g_eMMCDrv.u8_PadType)
	{
	case FCIE_eMMC_DDR:
		eMMC_debug(0,0,"DDR\n");  break;
	case FCIE_eMMC_SDR:
		eMMC_debug(0,0,"SDR\n");  break;
	case FCIE_eMMC_BYPASS:
		eMMC_debug(0,0,"BYPASS\n");  break;
	default:
		eMMC_debug(0,0,"eMMC Err: Pad unknown, %Xh\n", g_eMMCDrv.u8_PadType);  eMMC_die("\n");
	}

    eMMC_debug(0, 0, "reg_all_pad_in (0x%08X):0x%04X\n", reg_all_pad_in, REG_FCIE_U16(reg_all_pad_in));

    eMMC_debug(0, 0, "reg_emmc_pad (0x%08X):0x%04X\n", reg_emmc_pad, REG_FCIE_U16(reg_emmc_pad));
    eMMC_debug(0, 0, "reg_pcm_d_pe (0x%08X):0x%04X\n", reg_pcm_d_pe, REG_FCIE_U16(reg_pcm_d_pe));
    eMMC_debug(0, 0, "reg_pcm_a_pe (0x%08X):0x%04X\n", reg_pcm_a_pe, REG_FCIE_U16(reg_pcm_a_pe));
    eMMC_debug(0, 0, "reg_pcm2_cd_n_pe (0x%08X):0x%04X\n", reg_pcm2_cd_n_pe, REG_FCIE_U16(reg_pcm2_cd_n_pe));

    eMMC_debug(0, 0, "reg_sd_use_bypass (0x%08X):0x%04X\n", reg_sd_use_bypass, REG_FCIE_U16(reg_sd_use_bypass));
    eMMC_debug(0, 0, "reg_fcie2macro_sd_bypass (0x%08X):0x%04X\n", reg_fcie2macro_sd_bypass, REG_FCIE_U16(reg_fcie2macro_sd_bypass));

    eMMC_debug(0, 0, "reg_pcmcia_pad (0x%08X):0x%04X\n", reg_pcmcia_pad, REG_FCIE_U16(reg_pcmcia_pad));
    eMMC_debug(0, 0, "reg_nand_pad (0x%08X):0x%04X\n", reg_nand_pad, REG_FCIE_U16(reg_nand_pad));
    eMMC_debug(0, 0, "reg_sd_pad (0x%08X):0x%04X\n", reg_sd_pad, REG_FCIE_U16(reg_sd_pad));

    eMMC_debug(0, 0, "\n");
}

static U16 sgu16_eMMC_Pad=0xFFFF; // initialized by eMMC_platformInit
U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	U16 u16_FCIE_Reg10h;

	// avoid possible glitch in clock
	REG_FCIE_R(FCIE_SD_MODE, u16_FCIE_Reg10h);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT10|BIT11|BIT12);

    REG_FCIE_CLRBIT(reg_fcie2macro_sd_bypass, BIT8);    // reg_fcie2marco_sd_bypass
    REG_FCIE_SETBIT(reg_sd_use_bypass, BIT0);           // reg_sd_use_bypass

    switch(u32_FCIE_IF_Type){
        case FCIE_eMMC_DDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: DDR\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT9);

            REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT_ddr_timing_patch);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
            break;

        case FCIE_eMMC_SDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: SDR\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
			break;

        case FCIE_eMMC_BYPASS:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: BYPASS\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(reg_fcie2macro_sd_bypass, BIT8);    // reg_fcie2marco_sd_bypass
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT10|BIT11);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
            break;

        case FCIE_eMMC_TMUX:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: TMUX\n");

            REG_FCIE_SETBIT(reg_fcie2macro_sd_bypass, BIT8);    // reg_fcie2marco_sd_bypass
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT10|BIT12);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_TMUX;
            break;

        default:
            //eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
            REG_FCIE_W(FCIE_SD_MODE, u16_FCIE_Reg10h);
            return eMMC_ST_ERR_INVALID_PARAM;
    }

    // set chiptop
    if(0xFFFF == sgu16_eMMC_Pad)
    {
		REG_FCIE_R(reg_emmc_pad, sgu16_eMMC_Pad);
		sgu16_eMMC_Pad &= CFG_EMMC_PAD_MASK;
    }
	REG_FCIE_SETBIT(reg_emmc_pad, sgu16_eMMC_Pad);
	if(sgu16_eMMC_Pad == BIT_EMMC_CFG_MODE1)
	{
		REG_FCIE_SETBIT(reg_pcm_d_pe, 0x00FF);
		REG_FCIE_SETBIT(reg_pcm_d_drv, 0xFF00);
	}
	else if(sgu16_eMMC_Pad == BIT_EMMC_CFG_MODE3)
	{
		REG_FCIE_SETBIT(reg_nand_ps, 0xFFFF);
		REG_FCIE_SETBIT(reg_nand_drv, 0xFF00);
	}
	else
	{
		eMMC_die("\n");
	}

	REG_FCIE_SETBIT(reg_emmc_pad, BIT8|BIT9|BIT10|BIT11|BIT13);
	REG_FCIE_CLRBIT(reg_emmc_pad, BIT12);
    REG_FCIE_SETBIT(reg_emmc_drv, BIT0|BIT1|BIT2);

    // asked by yiikai.wang (<- root cause unknown)
    // comment for "neotion CI+ CAM" failure. .
    //REG_FCIE_CLRBIT(reg_pcmcia_pad, BIT0|BIT4);

    REG_FCIE_CLRBIT(reg_nand_pad, BIT5|BIT6|BIT7);

    REG_FCIE_CLRBIT(reg_all_pad_in, BIT15);

	REG_FCIE_W(FCIE_SD_MODE, u16_FCIE_Reg10h);
	return eMMC_ST_SUCCESS;
}

static U8 sgau8_FCIEClk_1X_To_4X_[0x10]= // index is 1X reg value
{
    0,
    BIT_FCIE_CLK4X_20M,
    BIT_FCIE_CLK4X_27M,
    0,
    BIT_FCIE_CLK4X_36M,
    BIT_FCIE_CLK4X_40M,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    BIT_FCIE_CLK4X_48M
};

U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam)	{
	case BIT_FCIE_CLK_300K:     g_eMMCDrv.u32_ClkKHz = 300;     break;
	case BIT_FCIE_CLK_20M:      g_eMMCDrv.u32_ClkKHz = 20000;   break;
    case BIT_FCIE_CLK_27M:      g_eMMCDrv.u32_ClkKHz = 27000;   break;
	//case BIT_FCIE_CLK_32M:      g_eMMCDrv.u32_ClkKHz = 32000;   break;
    case BIT_FCIE_CLK_36M:      g_eMMCDrv.u32_ClkKHz = 36000;   break;
	case BIT_FCIE_CLK_40M:      g_eMMCDrv.u32_ClkKHz = 40000;   break;
    //case BIT_FCIE_CLK_43_2M:    g_eMMCDrv.u32_ClkKHz = 43200;   break;
    case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
	default:
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_SEL|(u16_ClkParam<<BIT_FCIE_CLK_SHIFT));

    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
	{
	    REG_FCIE_CLRBIT(reg_ckg_fcie_4X,
			BIT_FCIE_CLK4X_Gate|BIT_FCIE_CLK4X_MASK);
	    REG_FCIE_SETBIT(reg_ckg_fcie_4X,
			(sgau8_FCIEClk_1X_To_4X_[u16_ClkParam]<<BIT_FCIE_CLK4X_SHIFT));

        REG_FCIE_CLRBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		//REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_DDR_TIMING_PATCH);
        REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z_EN);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z);
		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        REG_FCIE_CLRBIT(reg_chip_dummy1,BIT_SW_RST_Z);
		REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
	}

	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "clk:%uKHz, Param:%Xh, fcie_1X(%Xh):%Xh, fcie_4X(%Xh):%Xh\n",
		g_eMMCDrv.u32_ClkKHz, u16_ClkParam,
		reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X),
		reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));

	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
    eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_gating(void)
{
	eMMC_PlatformResetPre();
	g_eMMCDrv.u32_ClkKHz = 0;
	REG_FCIE_W(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate);
	REG_FCIE_W(reg_ckg_fcie_4X, BIT_FCIE_CLK4X_Gate);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_40M,
	BIT_FCIE_CLK_36M,
	BIT_FCIE_CLK_27M,
	BIT_FCIE_CLK_20M,
};

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
static char *sgachar_string[]={"  0T","0.5T","  1T","1.5T", "  2T", "2.5T", "  3T", "3.5T"};

static U32 eMMC_FCIE_DetectDDRTiming_Ex(U8 u8_DQS, U8 u8_DelaySel)
{
	U32 u32_SectorAddr;

	u32_SectorAddr = eMMC_TEST_BLK_0;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\ndqs:%s[%Xh]  cell:%02Xh \n",
		sgachar_string[u8_DQS], u8_DQS, u8_DelaySel);

	eMMC_FCIE_SetDDR48TimingReg(u8_DQS, u8_DelaySel);
    return eMMCTest_BlkWRC_ProbeTiming(u32_SectorAddr);
}

#define FCIE_DELAY_CELL_ts         300 // 0.7ns
static eMMC_FCIE_DDRT_WINDOW_t sg_DDRTWindow[2];

U32 eMMC_FCIE_DetectDDRTiming(void)
{
	U8  u8_dqs, u8_delay_sel, u8_i;
	U8  u8_dqs_prev=0xFF, u8_delay_sel_prev=0;
	U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  au8_DQS_10T[8]={0,5,10,15,20,25,30,35}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  u8_delay_Sel_max;
	U32 u32_ts;
	U32 u32_err;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow = &sg_DDRTWindow[0];

	g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_TUNING_TTABLE; // just to turn off some log

	//--------------------------------------------------
	sg_DDRTWindow[0].u8_Cnt = 0;
	sg_DDRTWindow[0].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[0].aParam[1].u8_DQS = 0;
    sg_DDRTWindow[1].u8_Cnt = 0;
	sg_DDRTWindow[1].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[1].aParam[1].u8_DQS = 0;
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
	{
		sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]=0;
		sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]=0;
	}

	//--------------------------------------------------
	// calculate delay_Sel_max
	u32_ts = 1000*1000*1000 / g_eMMCDrv.u32_ClkKHz;
	u32_ts >>= 2; // for 4X's 1T

    //--------------------------------------------------
    // no need to try DQS of no delay
	//for(u8_dqs=0; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	for(u8_dqs=1; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n---------------------------\n");

		if(u8_dqs < (BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT))
		    u8_delay_Sel_max =
		        (au8_DQS_10T[u8_dqs+1]-au8_DQS_10T[u8_dqs])
		        *u32_ts/(FCIE_DELAY_CELL_ts*10);
		else
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);

		if(u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT))
		{
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n", u8_delay_Sel_max);
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"invalid, not try\n\n");
			//continue;
		}
		else
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n\n", u8_delay_Sel_max);

		sg_DDRTWindow[0].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;
		sg_DDRTWindow[1].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;

		//for(u8_delay_sel=0; u8_delay_sel<=(BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT); u8_delay_sel++)
		for(u8_delay_sel=0; u8_delay_sel<=u8_delay_Sel_max; u8_delay_sel++)
		{
			u32_err = eMMC_FCIE_DetectDDRTiming_Ex(au8_DQSRegVal[u8_dqs], u8_delay_sel);
			if(eMMC_ST_SUCCESS == u32_err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"  PASS\n\n");
				pWindow->u8_Cnt++;
				if(0 == pWindow->aParam[0].u8_DQS) // save the window head
				{
					pWindow->aParam[0].u8_DQS = u8_dqs; // dqs uses index
					pWindow->aParam[0].u8_Cell = u8_delay_sel;
				}
				pWindow->au8_DQSValidCellCnt[u8_dqs]++;

				u8_dqs_prev = u8_dqs;
				u8_delay_sel_prev = u8_delay_sel;
			}
			else
			{   // save the window tail
				if(0xFF != u8_dqs_prev)
				{
					pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
					pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
				}
				u8_dqs_prev = 0xFF;

				// discard & re-use the window having less PASS cnt
				pWindow =
				    (sg_DDRTWindow[0].u8_Cnt < sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];
				pWindow->u8_Cnt = 0;
				pWindow->aParam[0].u8_DQS = 0;
				pWindow->aParam[1].u8_DQS = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
					pWindow->au8_DQSValidCellCnt[u8_i]=0;
			}
		}
	}

	// for the case of last try is ok
	if(0xFF != u8_dqs_prev)
	{
		pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
		pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
	}

	g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_TUNING_TTABLE;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W0, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[0].u8_Cnt,
		sg_DDRTWindow[0].aParam[0].u8_DQS, sg_DDRTWindow[0].aParam[0].u8_Cell,
		sg_DDRTWindow[0].aParam[1].u8_DQS, sg_DDRTWindow[0].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]);

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W1, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[1].u8_Cnt,
		sg_DDRTWindow[1].aParam[0].u8_DQS, sg_DDRTWindow[1].aParam[0].u8_Cell,
		sg_DDRTWindow[1].aParam[1].u8_DQS, sg_DDRTWindow[1].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]);

	if(sg_DDRTWindow[0].u8_Cnt || sg_DDRTWindow[1].u8_Cnt)
		return eMMC_ST_SUCCESS;
	else
		return eMMC_ST_ERR_NO_OK_DDR_PARAM;

}


U32 eMMC_FCIE_BuildDDRTimingTable(void)
{
	U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  u8_i, u8_ClkIdx, u8_SetIdx, u8_tmp, u8_DqsIdx = 0, u8_CellBase;
	U32 u32_err, u32_ret=eMMC_ST_ERR_NO_OK_DDR_PARAM;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow;

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: building DDR table, please wait... \n");
	memset((void*)&g_eMMCDrv.TimingTable_t, '\0', sizeof(g_eMMCDrv.TimingTable_t));
	u8_SetIdx = 0;

	if(eMMC_IF_NORMAL_SDR()){
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: SDR mode can't detect DDR timing\n");
		u32_err = eMMC_FCIE_EnableFastMode_Ex(FCIE_eMMC_DDR);
		if(eMMC_ST_SUCCESS != u32_err)
		{
		    printf("eMMC Err: set DDR IF fail: %X\n", u32_err);
		    return u32_err;
		}
	}
	for(u8_ClkIdx=0; u8_ClkIdx<eMMC_FCIE_VALID_CLK_CNT; u8_ClkIdx++)
	{

		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");
		eMMC_clock_setting(gau8_FCIEClkSel[u8_ClkIdx]);
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");

		// ---------------------------
		// search and set the Windows
		u32_err = eMMC_FCIE_DetectDDRTiming();

		// ---------------------------
		// set the Table
		if(eMMC_ST_SUCCESS == u32_err)
		{
			#if 0
			// before 12MHz, should at least 2 clk is ok for DDR
			if(eMMC_FCIE_DDRT_SET_CNT-1 == u8_SetIdx &&
				BIT_FCIE_CLK_12M == gau8_FCIEClkSel[u8_ClkIdx])
				u32_ret = eMMC_ST_SUCCESS;
			#endif

			g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk = gau8_FCIEClkSel[u8_ClkIdx];

			// ---------------------------
		 	// select Window
			pWindow = NULL;

			// pick up the Window of Cell=0 case
			if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS == sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[0];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS == sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[1];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

			// ---------------------------
			if(NULL != pWindow)
			{

				// pick up the DQS having max valid cell
				u8_tmp = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
				{
					if(u8_tmp <= pWindow->au8_DQSValidCellCnt[u8_i]){
						u8_tmp = pWindow->au8_DQSValidCellCnt[u8_i];
						u8_DqsIdx = u8_i;
					}
				}
				if(0 != u8_DqsIdx) // do not use 0T, this should be always TRUE
				{
					if(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] >= 7 &&
						pWindow->au8_DQSValidCellCnt[u8_DqsIdx] >= 7)
					{
						g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[u8_DqsIdx];
						g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell = 0; // nice
					}
					else
					{
						u8_tmp = (pWindow->au8_DQSValidCellCnt[u8_DqsIdx] +
							pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1]) / 2;


						if(u8_tmp < pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1])
						{
							g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx-1];

							u8_CellBase = pWindow->au8_DQSTryCellCnt[u8_DqsIdx-1] - pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1];
							g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell =
								u8_CellBase + pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx] - u8_tmp;
						}
						else
						{   g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx];
						    g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell =
								(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx]) / 2;
						}
					}
				}
			}

			// ---------------------------
            // or, pick up the Window of large PASS Cnt
			else //if(NULL == pWindow)
			{
			    pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

				g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[pWindow->aParam[0].u8_DQS];
				g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell =
					(pWindow->aParam[0].u8_Cell + pWindow->aParam[1].u8_Cell)/2;
			}

			#if 0
			// ---------------------------
			// use 12M for Set.Min
			if(eMMC_FCIE_DDRT_SET_CNT-2 == u8_SetIdx)
				u8_ClkIdx = eMMC_FCIE_VALID_CLK_CNT-2;
			#else
			if(FCIE_SLOW_CLK == g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.TimingTable_t.u8_SetCnt = u8_SetIdx + 1;
			#endif

			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT > eMMC_FCIE_DDRT_SET_CNT ?
	    eMMC_FCIE_DDRT_SET_CNT : eMMC_FCIE_VALID_CLK_CNT;
	u8_tmp = u8_tmp > g_eMMCDrv.TimingTable_t.u8_SetCnt ?
		g_eMMCDrv.TimingTable_t.u8_SetCnt : u8_tmp;

	eMMC_DumpTimingTable();

	// ======================================================
	// CAUTION: expect 48MHz can have valid DDRT parameter
	if(g_eMMCDrv.TimingTable_t.Set[eMMC_TIMING_SET_MAX].u8_Clk == gau8_FCIEClkSel[0]
		&& 0 != g_eMMCDrv.TimingTable_t.u8_SetCnt)
		u32_ret = eMMC_ST_SUCCESS;
	else if(g_eMMCDrv.TimingTable_t.Set[eMMC_TIMING_SET_MAX].u8_Clk != gau8_FCIEClkSel[0])
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: max clk can NOT run DDR\n");
	else if(0 == g_eMMCDrv.TimingTable_t.u8_SetCnt)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: min clk can NOT run DDR\n");
	// ======================================================

	// ---------------------------
	// save DDRT Table
	if(eMMC_ST_SUCCESS == u32_ret)
	{

		g_eMMCDrv.TimingTable_t.u32_ChkSum =
		    eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-4);
		memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t));

		eMMC_FCIE_ApplyTimingSet(eMMC_TIMING_SET_MAX);

		u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
	    u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
	    if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	    {
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: %Xh %Xh\n",
			    u32_err, u32_ret);
		    return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	    }
	}
	else
	{
		gau8_eMMC_SectorBuf[0] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>>24);
		gau8_eMMC_SectorBuf[1] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>>16);
		gau8_eMMC_SectorBuf[2] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>> 8);
		gau8_eMMC_SectorBuf[3] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>> 0);

		u32_ret = eMMC_ST_ERR_BUILD_DDRT;
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: fail! please call CAE or RD for helps, %Xh \n",
			u32_ret);
		eMMC_ResetReadyFlag();
		eMMC_Init();
		eMMC_FCIE_EnableSDRMode();

	}

	return u32_ret;
}

#endif // IF_DETECT_eMMC_DDR_TIMING


//---------------------------------------
#if defined(ENABLE_eMMC_INTERRUPT_MODE)&&ENABLE_eMMC_INTERRUPT_MODE

static DECLARE_WAIT_QUEUE_HEAD(fcie_wait);
static volatile U32 fcie_int = 0;

#define eMMC_IRQ_DEBUG    1

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
	volatile u16 u16_Events;

	if((REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE) != BIT_EMMC_ACTIVE)
	{
		return IRQ_NONE;
	}

	// one time enable one bit
	u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

	if(u16_Events & BIT_MIU_LAST_DONE)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CARD_DMA_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
	else if(u16_Events & BIT_SD_CMD_END)
	{
		REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_CMD_END);

		fcie_int = 1;
		wake_up(&fcie_wait);
		return IRQ_HANDLED;
	}

#if eMMC_IRQ_DEBUG
	if(0==fcie_int)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: Int St:%Xh, En:%Xh, Evt:%Xh \n",
		    REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN), u16_Events);
#endif

	return IRQ_NONE;

}


U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
{
	U32 u32_i=0;

    #if eMMC_IRQ_DEBUG
	U32 u32_isr_tmp[2];
	unsigned long long u64_jiffies_tmp, u64_jiffies_now;
	struct timeval time_st;
	time_t sec_tmp;
	suseconds_t us_tmp;

	u32_isr_tmp[0] = fcie_int;
	do_gettimeofday(&time_st);
	sec_tmp = time_st.tv_sec;
	us_tmp = time_st.tv_usec;
	u64_jiffies_tmp = jiffies_64;
    #endif

	//----------------------------------------
	if(wait_event_timeout(fcie_wait, (fcie_int == 1), usecs_to_jiffies(u32_MicroSec)) == 0)
    {
	    #if eMMC_IRQ_DEBUG
		u32_isr_tmp[1] = fcie_int;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh, ISR:%u->%u->%u \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN),
			u32_isr_tmp[0], u32_isr_tmp[1], fcie_int);

		do_gettimeofday(&time_st);
		u64_jiffies_now = jiffies_64;
	    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
		    " PassTime: %lu s, %lu us, %llu jiffies.  WaitTime: %u us, %lu jiffies, HZ:%u.\n",
		    time_st.tv_sec-sec_tmp, time_st.tv_usec-us_tmp, u64_jiffies_now-u64_jiffies_tmp,
		    u32_MicroSec, usecs_to_jiffies(u32_MicroSec), HZ);
	    #else
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN));
	    #endif

		// switch to polling
        for(u32_i=0; u32_i<u32_MicroSec; u32_i++)
        {
            if((REG_FCIE(u32_RegAddr) & u16_WaitEvent) == u16_WaitEvent )
                break;

            eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        }

		if(u32_i == u32_MicroSec)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: events lose, WaitEvent: %Xh \n", u16_WaitEvent);
			eMMC_DumpDriverStatus();  eMMC_DumpPadClk();
		    eMMC_FCIE_DumpRegisters();eMMC_FCIE_DumpDebugBus();
			return eMMC_ST_ERR_INT_TO;
		}
		else
		{	REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
		}

    }

	//----------------------------------------
	if(u16_WaitEvent & BIT_MIU_LAST_DONE)
	{
		for(u32_i=0; u32_i<TIME_WAIT_1_BLK_END; u32_i++)
		{
			if(REG_FCIE(u32_RegAddr) & BIT_CARD_DMA_END)
				break; // should be very fase
			eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
		}

		if(TIME_WAIT_1_BLK_END == u32_i)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no CARD_DMA_END\n");
			eMMC_FCIE_ErrHandler_Stop();
		}
	}

	//----------------------------------------
	fcie_int = 0;
	return eMMC_ST_SUCCESS;

}


#endif


int mstar_mci_Housekeep(void *pData)
{
	#if !(defined(eMMC_HOUSEKEEP_THREAD) && eMMC_HOUSEKEEP_THREAD)
    return 0;
	#endif

	#if eMMC_PROFILE_WR
	g_eMMCDrv.u32_CNT_MinRBlk = 0xFFFFFFFF;
	g_eMMCDrv.u32_CNT_MinWBlk = 0xFFFFFFFF;
	#endif

	while(1)
	{
		#if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
		msleep(1000);
		eMMC_dump_WR_Count();
		#endif

		if(kthread_should_stop())
			break;
	}

	return 0;
}


int mstar_mci_exit_checkdone_Ex(void)
{
    U16 u16Reg = 0;

    u16Reg = REG_FCIE(FCIE_SD_STATUS);

    if(u16Reg & BIT_SD_D0)
        return 1;
    else
        return 0;
}

bool mstar_mci_exit_checkdone_ForCI(void)
{
	#if 0
	U32 u32_Cnt=0;

    // eagle eMMC_mode_1 share pins
	if(BIT_eMMC_CFG_MODE1 != (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
		return true;

	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			eMMC_debug(0,1,"eMMC Info: PCM check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}
	#endif
	return true;
}

EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForCI);


bool mstar_mci_exit_checkdone_ForSD(void)
{
	U32 u32_Cnt=0;

    // eagle SD_mode_x does not share pins
	//if(BIT_eMMC_CFG_MODE2 == (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
	//	return TRUE;
    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
			eMMC_debug(0,1,"eMMC Info: SD check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	return true;
}
EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForSD);


U32 mstar_SD_CardChange(void)
{
	return 0;
}

//---------------------------------------

extern struct semaphore	PfModeSem;
extern bool ms_sdmmc_wait_d0_for_emmc(void);
#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
static U16 sgu16_SD_Pad;
#endif

void eMMC_LockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	down(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	if(false == ms_sdmmc_wait_d0_for_emmc())
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SD keep D0 low \n");
		eMMC_FCIE_ErrHandler_Stop();
	}

	eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
	eMMC_pads_switch(g_eMMCDrv.u8_PadType);

	// backup sd_config/sd_config2, and clear them
	REG_FCIE_R(reg_sd_pad, sgu16_SD_Pad);
	sgu16_SD_Pad &= CFG_SD_PAD_MASK;
	REG_FCIE_CLRBIT(reg_sd_pad, CFG_SD_PAD_MASK);
	#else
	REG_FCIE_CLRBIT(reg_sd_pad, CFG_SD_PAD_MASK);
	#endif

    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	REG_FCIE_SETBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);
}


void eMMC_UnlockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	REG_FCIE_CLRBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	// restore sd_config/sd_config2
    REG_FCIE_SETBIT(reg_sd_pad, sgu16_SD_Pad);
	#endif

	up(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);
}

//---------------------------------------

U32 eMMC_PlatformResetPre(void)
{
	/**((volatile unsigned short *)(0x25007DCC))|=0x02;	// emi mask
	*((volatile unsigned short *)(0x25007C18))|=0x02;	// imi0 mask
	*((volatile unsigned short *)(0x25007C58))|=0x02;	// imi1 mask
	*/
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	/**((volatile unsigned short *)(0x25007DCC))&=(~0x02);	// emi unmask
	*((volatile unsigned short *)(0x25007C18))&=(~0x02);	// imi0 unmask
	*((volatile unsigned short *)(0x25007C58))&=(~0x02);	// imi1 unmask
	*/
	REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT_csreg_miu_wp_last2_sel);
	return eMMC_ST_SUCCESS;
}


U32 eMMC_PlatformInit(void)
{
	static U8 initialized = 0;

	if(initialized == 0)
	{
		REG_FCIE_R(reg_emmc_pad, sgu16_eMMC_Pad);
		sgu16_eMMC_Pad &= CFG_EMMC_PAD_MASK;
		if(sgu16_eMMC_Pad!=BIT_EMMC_CFG_MODE1 && sgu16_eMMC_Pad!=BIT_EMMC_CFG_MODE3)
		{
			eMMC_die("\n");
		}
		initialized = 1;
	}

	eMMC_pads_switch(FCIE_DEFAULT_PAD);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	return eMMC_ST_SUCCESS;
}


U32 eMMC_PlatformDeinit(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_StartSector += BL_BLK_OFFSET;
		break;

	case eMMC_PART_OTP:
		u32_StartSector += OTP_BLK_OFFSET;
		break;

	case eMMC_PART_SECINFO:
		u32_StartSector += SecInfo_BLK_OFFSET;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"SecAddr: %Xh, SecCnt: %Xh\n", u32_StartSector, u32_SectorCnt);

	if(eMMC_BOOT_PART_W == u8_OP)
		return eMMC_WriteBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);
	else
		return eMMC_ReadBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);

}


U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
	U32 u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end;

	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_eMMCBlkAddr_start = 0;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+BL_BLK_CNT-1;
		break;

	case eMMC_PART_OTP:
		u32_eMMCBlkAddr_start = OTP_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+OTP_BLK_CNT-1;
		break;

	case eMMC_PART_SECINFO:
		u32_eMMCBlkAddr_start = SecInfo_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+SecInfo_BLK_CNT-1;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"BlkAddr_start: %Xh, BlkAddr_end: %Xh\n",
		u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);

    return eMMC_EraseBootPart(u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end, 1);

}


char *gpas8_eMMCPartName[]={
	"e2pbak", "nvrambak", "hwcfgs", "recovery", "os",
	"fdd", "tdd", "blogo", "apanic","misc", "cus",
	"e2p0","e2p1","nvram0","nvram1", "system","cache", "data", "internal sd"};

// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

U32 eMMC_CheckIfMemCorrupt(void)
{
	if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
		return eMMC_ST_ERR_MEM_CORRUPT;

	return eMMC_ST_SUCCESS;
}
// <-- [FIXME]

//===================================================
#elif (defined(eMMC_DRV_EIFFEL_LINUX)&&eMMC_DRV_EIFFEL_LINUX) // [FIXME] clone for your flatform
// [FIXME] -->
void eMMC_DumpPadClk(void)
{
	//----------------------------------------------
    eMMC_debug(0, 0, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
    eMMC_debug(0, 0, "FCIE 1X (0x%X):0x%04X\n", reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X));
    eMMC_debug(0, 0, "FCIE 4X (0x%X):0x%04X\n", reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));
    eMMC_debug(0, 0, "MIU (0x%X):0x%04X\n", reg_ckg_MIU, REG_FCIE_U16(reg_ckg_MIU));
    eMMC_debug(0, 0, "MCU (0x%X):0x%04X\n", reg_ckg_MCU, REG_FCIE_U16(reg_ckg_MCU));

    //----------------------------------------------
    eMMC_DumpChiptop();

    eMMC_debug(0, 0, "\n[pad setting]: ");
    switch(g_eMMCDrv.u8_PadType)
	{
	case FCIE_eMMC_DDR:
		eMMC_debug(0,0,"DDR\n");  break;
	case FCIE_eMMC_SDR:
		eMMC_debug(0,0,"SDR\n");  break;
	case FCIE_eMMC_BYPASS:
		eMMC_debug(0,0,"BYPASS\n");  break;
	default:
		eMMC_debug(0,0,"eMMC Err: Pad unknown, %Xh\n", g_eMMCDrv.u8_PadType);  eMMC_die("\n");
	}

    eMMC_debug(0, 0, "reg_all_pad_in (0x%08X):0x%04X\n", reg_all_pad_in, REG_FCIE_U16(reg_all_pad_in));

    eMMC_debug(0, 0, "reg_emmc_pad (0x%08X):0x%04X\n", reg_emmc_pad, REG_FCIE_U16(reg_emmc_pad));
    eMMC_debug(0, 0, "reg_pcm_a_pe (0x%08X):0x%04X\n", reg_pcm_a_pe, REG_FCIE_U16(reg_pcm_a_pe));

    eMMC_debug(0, 0, "reg_sd_use_bypass (0x%08X):0x%04X\n", reg_sd_use_bypass, REG_FCIE_U16(reg_sd_use_bypass));
    eMMC_debug(0, 0, "reg_fcie2macro_sd_bypass (0x%08X):0x%04X\n", reg_fcie2macro_sd_bypass, REG_FCIE_U16(reg_fcie2macro_sd_bypass));

    eMMC_debug(0, 0, "reg_pcmcia_pad (0x%08X):0x%04X\n", reg_pcmcia_pad, REG_FCIE_U16(reg_pcmcia_pad));
    eMMC_debug(0, 0, "reg_nand_pad (0x%08X):0x%04X\n", reg_nand_pad, REG_FCIE_U16(reg_nand_pad));
    eMMC_debug(0, 0, "reg_sd_pad (0x%08X):0x%04X\n", reg_sd_pad, REG_FCIE_U16(reg_sd_pad));

    eMMC_debug(0, 0, "\n");
}

static U16 sgu16_eMMC_Pad=0xFFFF; // initialized by eMMC_platformInit
U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	U16 u16_FCIE_Reg10h;

	// avoid possible glitch in clock
	REG_FCIE_R(FCIE_SD_MODE, u16_FCIE_Reg10h);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT10|BIT11|BIT12);

    REG_FCIE_CLRBIT(reg_fcie2macro_sd_bypass, FCIE2MACRO_SD_BYPASS);    // reg_fcie2marco_sd_bypass
    REG_FCIE_SETBIT(reg_sd_use_bypass, SD_USE_BYPASS);           // reg_sd_use_bypass

    switch(u32_FCIE_IF_Type){
        case FCIE_eMMC_DDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: DDR\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT9);

            REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT_ddr_timing_patch);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
            break;

        case FCIE_eMMC_SDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: SDR\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
			break;

        case FCIE_eMMC_BYPASS:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: BYPASS\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(reg_fcie2macro_sd_bypass, FCIE2MACRO_SD_BYPASS);    // reg_fcie2marco_sd_bypass
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT10|BIT11);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
            break;


        default:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
            REG_FCIE_W(FCIE_SD_MODE, u16_FCIE_Reg10h);
            return eMMC_ST_ERR_INVALID_PARAM;
    }

    // set chiptop
    if(0xFFFF == sgu16_eMMC_Pad)
    {
		REG_FCIE_R(reg_emmc_pad, sgu16_eMMC_Pad);
		sgu16_eMMC_Pad &= CFG_EMMC_PAD_MASK;
    }
	REG_FCIE_SETBIT(reg_emmc_pad, sgu16_eMMC_Pad);
	if(sgu16_eMMC_Pad == BIT_EMMC_CFG_MODE1)
	{
		REG_FCIE_SETBIT(reg_emmc_drv, 0x0007);
		REG_FCIE_SETBIT(reg_pcm_a_drv, 0xFF00);
		REG_FCIE_CLRBIT(reg_pcm_a_pe, 0x00FF);
	}
	else if(sgu16_eMMC_Pad == BIT_EMMC_CFG_MODE3)
	{
	    REG_FCIE_SETBIT(reg_emmc_drv, 0x0007);
		REG_FCIE_SETBIT(reg_nand_drv, 0x00FF);
	}
	else
	{
		eMMC_die("\n");
	}

    REG_FCIE_CLRBIT(reg_test_in_out_mode, reg_test_in_out_mode_mask);
	REG_FCIE_CLRBIT(reg_pcmcia_pad, CFG_PCMCIA_PAD_MASK);
    REG_FCIE_CLRBIT(reg_sd_pad, CFG_SD_PAD_MASK);


    REG_FCIE_CLRBIT(reg_all_pad_in, BIT_ALL_PAD_IN);

	REG_FCIE_W(FCIE_SD_MODE, u16_FCIE_Reg10h);
	return eMMC_ST_SUCCESS;
}

static U8 sgau8_FCIEClk_1X_To_4X_[0x10]= // index is 1X reg value
{
    0,
    0,
    BIT_FCIE_CLK4X_27M,
    0,
    BIT_FCIE_CLK4X_36M,
    BIT_FCIE_CLK4X_40M,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    BIT_FCIE_CLK4X_48M
};

U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam)	{
	case BIT_FCIE_CLK_300K:     g_eMMCDrv.u32_ClkKHz = 300;     break;
	case BIT_FCIE_CLK_20M:      g_eMMCDrv.u32_ClkKHz = 20000;   break;
    case BIT_FCIE_CLK_27M:      g_eMMCDrv.u32_ClkKHz = 27000;   break;
	//case BIT_FCIE_CLK_32M:      g_eMMCDrv.u32_ClkKHz = 32000;   break;
    case BIT_FCIE_CLK_36M:      g_eMMCDrv.u32_ClkKHz = 36000;   break;
	case BIT_FCIE_CLK_40M:      g_eMMCDrv.u32_ClkKHz = 40000;   break;
    //case BIT_FCIE_CLK_43_2M:    g_eMMCDrv.u32_ClkKHz = 43200;   break;
    case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
	default:
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_SEL|(u16_ClkParam<<BIT_FCIE_CLK_SHIFT));

    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
	{
	    REG_FCIE_CLRBIT(reg_ckg_fcie_4X,
			BIT_FCIE_CLK4X_Gate|BIT_FCIE_CLK4X_MASK);
	    REG_FCIE_SETBIT(reg_ckg_fcie_4X,
			(sgau8_FCIEClk_1X_To_4X_[u16_ClkParam]<<BIT_FCIE_CLK4X_SHIFT));

        REG_FCIE_CLRBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		//REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_DDR_TIMING_PATCH);
        REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z_EN);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z);
		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        REG_FCIE_CLRBIT(reg_chip_dummy1,BIT_SW_RST_Z);
		REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
	}

	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "clk:%uKHz, Param:%Xh, fcie_1X(%Xh):%Xh, fcie_4X(%Xh):%Xh\n",
		g_eMMCDrv.u32_ClkKHz, u16_ClkParam,
		reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X),
		reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));

	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
    eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_gating(void)
{
	eMMC_PlatformResetPre();
	g_eMMCDrv.u32_ClkKHz = 0;
	REG_FCIE_W(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate);
	REG_FCIE_W(reg_ckg_fcie_4X, BIT_FCIE_CLK4X_Gate);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_40M,
	BIT_FCIE_CLK_36M,
	BIT_FCIE_CLK_27M,
};

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
static char *sgachar_string[]={"  0T","0.5T","  1T","1.5T", "  2T", "2.5T", "  3T", "3.5T"};

static U32 eMMC_FCIE_DetectDDRTiming_Ex(U8 u8_DQS, U8 u8_DelaySel)
{
	U32 u32_SectorAddr;

	u32_SectorAddr = eMMC_TEST_BLK_0;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\ndqs:%s[%Xh]  cell:%02Xh \n",
		sgachar_string[u8_DQS], u8_DQS, u8_DelaySel);

	eMMC_FCIE_SetDDR48TimingReg(u8_DQS, u8_DelaySel);
    return eMMCTest_BlkWRC_ProbeTiming(u32_SectorAddr);
}

#define FCIE_DELAY_CELL_ts         300 // 0.7ns
static eMMC_FCIE_DDRT_WINDOW_t sg_DDRTWindow[2];

U32 eMMC_FCIE_DetectDDRTiming(void)
{
	U8  u8_dqs, u8_delay_sel, u8_i;
	U8  u8_dqs_prev=0xFF, u8_delay_sel_prev=0;
	U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  au8_DQS_10T[8]={0,5,10,15,20,25,30,35}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  u8_delay_Sel_max;
	U32 u32_ts;
	U32 u32_err;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow = &sg_DDRTWindow[0];

	g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_TUNING_TTABLE; // just to turn off some log

	//--------------------------------------------------
	sg_DDRTWindow[0].u8_Cnt = 0;
	sg_DDRTWindow[0].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[0].aParam[1].u8_DQS = 0;
    sg_DDRTWindow[1].u8_Cnt = 0;
	sg_DDRTWindow[1].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[1].aParam[1].u8_DQS = 0;
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
	{
		sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]=0;
		sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]=0;
	}

	//--------------------------------------------------
	// calculate delay_Sel_max
	u32_ts = 1000*1000*1000 / g_eMMCDrv.u32_ClkKHz;
	u32_ts >>= 2; // for 4X's 1T

    //--------------------------------------------------
    // no need to try DQS of no delay
	//for(u8_dqs=0; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	for(u8_dqs=1; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n---------------------------\n");

		if(u8_dqs < (BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT))
		    u8_delay_Sel_max =
		        (au8_DQS_10T[u8_dqs+1]-au8_DQS_10T[u8_dqs])
		        *u32_ts/(FCIE_DELAY_CELL_ts*10);
		else
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);

		if(u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT))
		{
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n", u8_delay_Sel_max);
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"invalid, not try\n\n");
			//continue;
		}
		else
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n\n", u8_delay_Sel_max);

		sg_DDRTWindow[0].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;
		sg_DDRTWindow[1].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;

		//for(u8_delay_sel=0; u8_delay_sel<=(BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT); u8_delay_sel++)
		for(u8_delay_sel=0; u8_delay_sel<=u8_delay_Sel_max; u8_delay_sel++)
		{
			u32_err = eMMC_FCIE_DetectDDRTiming_Ex(au8_DQSRegVal[u8_dqs], u8_delay_sel);
			if(eMMC_ST_SUCCESS == u32_err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"  PASS\n\n");
				pWindow->u8_Cnt++;
				if(0 == pWindow->aParam[0].u8_DQS) // save the window head
				{
					pWindow->aParam[0].u8_DQS = u8_dqs; // dqs uses index
					pWindow->aParam[0].u8_Cell = u8_delay_sel;
				}
				pWindow->au8_DQSValidCellCnt[u8_dqs]++;

				u8_dqs_prev = u8_dqs;
				u8_delay_sel_prev = u8_delay_sel;
			}
			else
			{   // save the window tail
				if(0xFF != u8_dqs_prev)
				{
					pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
					pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
				}
				u8_dqs_prev = 0xFF;

				// discard & re-use the window having less PASS cnt
				pWindow =
				    (sg_DDRTWindow[0].u8_Cnt < sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];
				pWindow->u8_Cnt = 0;
				pWindow->aParam[0].u8_DQS = 0;
				pWindow->aParam[1].u8_DQS = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
					pWindow->au8_DQSValidCellCnt[u8_i]=0;
			}
		}
	}

	// for the case of last try is ok
	if(0xFF != u8_dqs_prev)
	{
		pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
		pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
	}

	g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_TUNING_TTABLE;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W0, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[0].u8_Cnt,
		sg_DDRTWindow[0].aParam[0].u8_DQS, sg_DDRTWindow[0].aParam[0].u8_Cell,
		sg_DDRTWindow[0].aParam[1].u8_DQS, sg_DDRTWindow[0].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]);

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W1, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[1].u8_Cnt,
		sg_DDRTWindow[1].aParam[0].u8_DQS, sg_DDRTWindow[1].aParam[0].u8_Cell,
		sg_DDRTWindow[1].aParam[1].u8_DQS, sg_DDRTWindow[1].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]);

	if(sg_DDRTWindow[0].u8_Cnt || sg_DDRTWindow[1].u8_Cnt)
		return eMMC_ST_SUCCESS;
	else
		return eMMC_ST_ERR_NO_OK_DDR_PARAM;

}


U32 eMMC_FCIE_BuildDDRTimingTable(void)
{
	U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  u8_i, u8_ClkIdx, u8_SetIdx, u8_tmp, u8_DqsIdx = 0, u8_CellBase;
	U32 u32_err, u32_ret=eMMC_ST_ERR_NO_OK_DDR_PARAM;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow;

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: building DDR table, please wait... \n");
	memset((void*)&g_eMMCDrv.TimingTable_t, '\0', sizeof(g_eMMCDrv.TimingTable_t));
	u8_SetIdx = 0;

	if(eMMC_IF_NORMAL_SDR()){
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: SDR mode can't detect DDR timing\n");
		u32_err = eMMC_FCIE_EnableFastMode_Ex(FCIE_eMMC_DDR);
		if(eMMC_ST_SUCCESS != u32_err)
		{
		    printk("eMMC Err: set DDR IF fail: %X\n", u32_err);
		    return u32_err;
		}
	}
	for(u8_ClkIdx=0; u8_ClkIdx<eMMC_FCIE_VALID_CLK_CNT; u8_ClkIdx++)
	{

		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");
		eMMC_clock_setting(gau8_FCIEClkSel[u8_ClkIdx]);
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");

		// ---------------------------
		// search and set the Windows
		u32_err = eMMC_FCIE_DetectDDRTiming();

		// ---------------------------
		// set the Table
		if(eMMC_ST_SUCCESS == u32_err)
		{
			#if 0
			// before 12MHz, should at least 2 clk is ok for DDR
			if(eMMC_FCIE_DDRT_SET_CNT-1 == u8_SetIdx &&
				BIT_FCIE_CLK_12M == gau8_FCIEClkSel[u8_ClkIdx])
				u32_ret = eMMC_ST_SUCCESS;
			#endif

			g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk = gau8_FCIEClkSel[u8_ClkIdx];

			// ---------------------------
		 	// select Window
			pWindow = NULL;

			// pick up the Window of Cell=0 case
			if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS == sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[0];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS == sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[1];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

			// ---------------------------
			if(NULL != pWindow)
			{

				// pick up the DQS having max valid cell
				u8_tmp = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
				{
					if(u8_tmp <= pWindow->au8_DQSValidCellCnt[u8_i]){
						u8_tmp = pWindow->au8_DQSValidCellCnt[u8_i];
						u8_DqsIdx = u8_i;
					}
				}
				if(0 != u8_DqsIdx) // do not use 0T, this should be always TRUE
				{
					if(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] >= 7 &&
						pWindow->au8_DQSValidCellCnt[u8_DqsIdx] >= 7)
					{
						g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[u8_DqsIdx];
						g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell = 0; // nice
					}
					else
					{
						u8_tmp = (pWindow->au8_DQSValidCellCnt[u8_DqsIdx] +
							pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1]) / 2;


						if(u8_tmp < pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1])
						{
							g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx-1];

							u8_CellBase = pWindow->au8_DQSTryCellCnt[u8_DqsIdx-1] - pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1];
							g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell =
								u8_CellBase + pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx] - u8_tmp;
						}
						else
						{   g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx];
						    g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell =
								(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx]) / 2;
						}
					}
				}
			}

			// ---------------------------
            // or, pick up the Window of large PASS Cnt
			else //if(NULL == pWindow)
			{
			    pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

				g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[pWindow->aParam[0].u8_DQS];
				g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell =
					(pWindow->aParam[0].u8_Cell + pWindow->aParam[1].u8_Cell)/2;
			}

			#if 0
			// ---------------------------
			// use 12M for Set.Min
			if(eMMC_FCIE_DDRT_SET_CNT-2 == u8_SetIdx)
				u8_ClkIdx = eMMC_FCIE_VALID_CLK_CNT-2;
			#else
			if(FCIE_SLOW_CLK == g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.TimingTable_t.u8_SetCnt = u8_SetIdx + 1;
			#endif

			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT > eMMC_FCIE_DDRT_SET_CNT ?
	    eMMC_FCIE_DDRT_SET_CNT : eMMC_FCIE_VALID_CLK_CNT;
	u8_tmp = u8_tmp > g_eMMCDrv.TimingTable_t.u8_SetCnt ?
		g_eMMCDrv.TimingTable_t.u8_SetCnt : u8_tmp;

	eMMC_DumpTimingTable();

	// ======================================================
	// CAUTION: expect 48MHz can have valid DDRT parameter
	if(g_eMMCDrv.TimingTable_t.Set[eMMC_TIMING_SET_MAX].u8_Clk == gau8_FCIEClkSel[0]
		&& 0 != g_eMMCDrv.TimingTable_t.u8_SetCnt)
		u32_ret = eMMC_ST_SUCCESS;
	else if(g_eMMCDrv.TimingTable_t.Set[eMMC_TIMING_SET_MAX].u8_Clk != gau8_FCIEClkSel[0])
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: max clk can NOT run DDR\n");
	else if(0 == g_eMMCDrv.TimingTable_t.u8_SetCnt)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: min clk can NOT run DDR\n");
	// ======================================================

	// ---------------------------
	// save DDRT Table
	if(eMMC_ST_SUCCESS == u32_ret)
	{

		g_eMMCDrv.TimingTable_t.u32_ChkSum =
		    eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-4);
		memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t));

		eMMC_FCIE_ApplyTimingSet(eMMC_TIMING_SET_MAX);

		u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
	    u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
	    if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	    {
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: %Xh %Xh\n",
			    u32_err, u32_ret);
		    return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	    }
	}
	else
	{
		gau8_eMMC_SectorBuf[0] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>>24);
		gau8_eMMC_SectorBuf[1] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>>16);
		gau8_eMMC_SectorBuf[2] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>> 8);
		gau8_eMMC_SectorBuf[3] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>> 0);

		u32_ret = eMMC_ST_ERR_BUILD_DDRT;
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: fail! please call CAE or RD for helps, %Xh \n",
			u32_ret);
		eMMC_ResetReadyFlag();
		eMMC_Init();
		eMMC_FCIE_EnableSDRMode();

	}

	return u32_ret;
}

#endif // IF_DETECT_eMMC_DDR_TIMING


//---------------------------------------
#if defined(ENABLE_eMMC_INTERRUPT_MODE)&&ENABLE_eMMC_INTERRUPT_MODE

static DECLARE_WAIT_QUEUE_HEAD(fcie_wait);
static volatile U32 fcie_int = 0;

#define eMMC_IRQ_DEBUG    1

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
	volatile u16 u16_Events;

	if((REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE) != BIT_EMMC_ACTIVE)
	{
		return IRQ_NONE;
	}

	// one time enable one bit
	u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

	if(u16_Events & BIT_MIU_LAST_DONE)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CARD_DMA_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
	else if(u16_Events & BIT_SD_CMD_END)
	{
		REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_CMD_END);

		fcie_int = 1;
		wake_up(&fcie_wait);
		return IRQ_HANDLED;
	}

#if eMMC_IRQ_DEBUG
	if(0==fcie_int)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: Int St:%Xh, En:%Xh, Evt:%Xh \n",
		    REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN), u16_Events);
#endif

	return IRQ_NONE;

}


U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
{
	U32 u32_i=0;

    #if eMMC_IRQ_DEBUG
	U32 u32_isr_tmp[2];
	unsigned long long u64_jiffies_tmp, u64_jiffies_now;
	struct timeval time_st;
	time_t sec_tmp;
	suseconds_t us_tmp;

	u32_isr_tmp[0] = fcie_int;
	do_gettimeofday(&time_st);
	sec_tmp = time_st.tv_sec;
	us_tmp = time_st.tv_usec;
	u64_jiffies_tmp = jiffies_64;
    #endif

	//----------------------------------------
	if(wait_event_timeout(fcie_wait, (fcie_int == 1), usecs_to_jiffies(u32_MicroSec)) == 0)
    {
	    #if eMMC_IRQ_DEBUG
		u32_isr_tmp[1] = fcie_int;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh, ISR:%u->%u->%u \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN),
			u32_isr_tmp[0], u32_isr_tmp[1], fcie_int);

		do_gettimeofday(&time_st);
		u64_jiffies_now = jiffies_64;
	    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
		    " PassTime: %lu s, %lu us, %llu jiffies.  WaitTime: %u us, %lu jiffies, HZ:%u.\n",
		    time_st.tv_sec-sec_tmp, time_st.tv_usec-us_tmp, u64_jiffies_now-u64_jiffies_tmp,
		    u32_MicroSec, usecs_to_jiffies(u32_MicroSec), HZ);
	    #else
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN));
	    #endif

		// switch to polling
        for(u32_i=0; u32_i<u32_MicroSec; u32_i++)
        {
            if((REG_FCIE(u32_RegAddr) & u16_WaitEvent) == u16_WaitEvent )
                break;

            eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        }

		if(u32_i == u32_MicroSec)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: events lose, WaitEvent: %Xh \n", u16_WaitEvent);
			eMMC_DumpDriverStatus();  eMMC_DumpPadClk();
		    eMMC_FCIE_DumpRegisters();eMMC_FCIE_DumpDebugBus();
			return eMMC_ST_ERR_INT_TO;
		}
		else
		{	REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
		}

    }

	//----------------------------------------
	if(u16_WaitEvent & BIT_MIU_LAST_DONE)
	{
		for(u32_i=0; u32_i<TIME_WAIT_1_BLK_END; u32_i++)
		{
			if(REG_FCIE(u32_RegAddr) & BIT_CARD_DMA_END)
				break; // should be very fase
			eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
		}

		if(TIME_WAIT_1_BLK_END == u32_i)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no CARD_DMA_END\n");
			eMMC_FCIE_ErrHandler_Stop();
		}
	}

	//----------------------------------------
	fcie_int = 0;
	return eMMC_ST_SUCCESS;

}


#endif


int mstar_mci_Housekeep(void *pData)
{
	#if !(defined(eMMC_HOUSEKEEP_THREAD) && eMMC_HOUSEKEEP_THREAD)
    return 0;
	#endif

	#if eMMC_PROFILE_WR
	g_eMMCDrv.u32_CNT_MinRBlk = 0xFFFFFFFF;
	g_eMMCDrv.u32_CNT_MinWBlk = 0xFFFFFFFF;
	#endif

	while(1)
	{
		#if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
		msleep(1000);
		eMMC_dump_WR_Count();
		#endif

		if(kthread_should_stop())
			break;
	}

	return 0;
}


int mstar_mci_exit_checkdone_Ex(void)
{
    U16 u16Reg = 0;

    u16Reg = REG_FCIE(FCIE_SD_STATUS);

    if(u16Reg & BIT_SD_D0)
        return 1;
    else
        return 0;
}

bool mstar_mci_exit_checkdone_ForCI(void)
{
	#if 0
	U32 u32_Cnt=0;

    // eiffel eMMC_mode_1 share pins
	if(BIT_eMMC_CFG_MODE1 != (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
		return true;

	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			eMMC_debug(0,1,"eMMC Info: PCM check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}
	#endif
	return true;
}

EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForCI);


bool mstar_mci_exit_checkdone_ForSD(void)
{
	U32 u32_Cnt=0;

    // eiffel SD_mode_x does not share pins
	//if(BIT_eMMC_CFG_MODE2 == (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
	//	return TRUE;
    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
			eMMC_debug(0,1,"eMMC Info: SD check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	return true;
}
EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForSD);


U32 mstar_SD_CardChange(void)
{
	return 0;
}

//---------------------------------------

extern struct semaphore	PfModeSem;
extern bool ms_sdmmc_wait_d0_for_emmc(void);
#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
static U16 sgu16_SD_Pad;
#endif

void eMMC_LockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	down(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	if(false == ms_sdmmc_wait_d0_for_emmc())
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SD keep D0 low \n");
		eMMC_FCIE_ErrHandler_Stop();
	}

	eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
	eMMC_pads_switch(g_eMMCDrv.u8_PadType);

	// backup sd_config/sd_config2, and clear them
	REG_FCIE_R(reg_sd_pad, sgu16_SD_Pad);
	sgu16_SD_Pad &= CFG_SD_PAD_MASK;
	REG_FCIE_CLRBIT(reg_sd_pad, CFG_SD_PAD_MASK);
	#else
	REG_FCIE_CLRBIT(reg_sd_pad, CFG_SD_PAD_MASK);
	#endif

    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	REG_FCIE_SETBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);
}


void eMMC_UnlockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
    REG_FCIE_CLRBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);
	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	// restore sd_config/sd_config2
    REG_FCIE_SETBIT(reg_sd_pad, sgu16_SD_Pad);
	#endif

	up(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);
}

//---------------------------------------

U32 eMMC_PlatformResetPre(void)
{
	/**((volatile unsigned short *)(0x25007DCC))|=0x02;	// emi mask
	*((volatile unsigned short *)(0x25007C18))|=0x02;	// imi0 mask
	*((volatile unsigned short *)(0x25007C58))|=0x02;	// imi1 mask
	*/
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	/**((volatile unsigned short *)(0x25007DCC))&=(~0x02);	// emi unmask
	*((volatile unsigned short *)(0x25007C18))&=(~0x02);	// imi0 unmask
	*((volatile unsigned short *)(0x25007C58))&=(~0x02);	// imi1 unmask
	*/
    #if defined(ENABLE_EMMC_POWER_SAVING_MODE) && ENABLE_EMMC_POWER_SAVING_MODE
	eMMC_Prepare_Power_Saving_Mode_Queue();
    #endif

	REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT_csreg_miu_wp_last2_sel);
	return eMMC_ST_SUCCESS;
}


U32 eMMC_PlatformInit(void)
{
	static U8 initialized = 0;

	if(initialized == 0)
	{
		REG_FCIE_R(reg_emmc_pad, sgu16_eMMC_Pad);
		sgu16_eMMC_Pad &= CFG_EMMC_PAD_MASK;
		if(sgu16_eMMC_Pad!=BIT_EMMC_CFG_MODE1 && sgu16_eMMC_Pad!=BIT_EMMC_CFG_MODE3)
		{
			eMMC_die("\n");
		}
		initialized = 1;
	}

	eMMC_pads_switch(FCIE_DEFAULT_PAD);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	return eMMC_ST_SUCCESS;
}


U32 eMMC_PlatformDeinit(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_StartSector += BL_BLK_OFFSET;
		break;

	case eMMC_PART_OTP:
		u32_StartSector += OTP_BLK_OFFSET;
		break;

	case eMMC_PART_SECINFO:
		u32_StartSector += SecInfo_BLK_OFFSET;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"SecAddr: %Xh, SecCnt: %Xh\n", u32_StartSector, u32_SectorCnt);

	if(eMMC_BOOT_PART_W == u8_OP)
		return eMMC_WriteBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);
	else
		return eMMC_ReadBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);

}


U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
	U32 u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end;

	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_eMMCBlkAddr_start = 0;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+BL_BLK_CNT-1;
		break;

	case eMMC_PART_OTP:
		u32_eMMCBlkAddr_start = OTP_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+OTP_BLK_CNT-1;
		break;

	case eMMC_PART_SECINFO:
		u32_eMMCBlkAddr_start = SecInfo_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+SecInfo_BLK_CNT-1;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"BlkAddr_start: %Xh, BlkAddr_end: %Xh\n",
		u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);

    return eMMC_EraseBootPart(u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end, 1);

}


char *gpas8_eMMCPartName[]={
	"e2pbak", "nvrambak", "hwcfgs", "recovery", "os",
	"fdd", "tdd", "blogo", "apanic","misc", "cus",
	"e2p0","e2p1","nvram0","nvram1", "system","cache", "data", "internal sd"};

// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

U32 eMMC_CheckIfMemCorrupt(void)
{
	if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
		return eMMC_ST_ERR_MEM_CORRUPT;

	return eMMC_ST_SUCCESS;
}
// <-- [FIXME]

//===================================================
#elif (defined(eMMC_DRV_NIKE_LINUX)&&eMMC_DRV_NIKE_LINUX) // [FIXME] clone for your flatform
// [FIXME] -->
void eMMC_DumpPadClk(void)
{
	//----------------------------------------------
    eMMC_debug(0, 0, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
    eMMC_debug(0, 0, "FCIE CLK (0x%X):0x%04X\n", reg_ckg_fcie, REG_FCIE_U16(reg_ckg_fcie));
    eMMC_debug(0, 0, "reg_ckg_nfie_div4_en (0x%X):0x%04X\n", reg_ckg_nfie_div4_en, REG_FCIE_U16(reg_ckg_nfie_div4_en));

    //----------------------------------------------
    eMMC_DumpChiptop();

    eMMC_debug(0, 0, "\n[pad setting]: ");
    switch(g_eMMCDrv.u8_PadType)
	{
	case FCIE_eMMC_DDR:
		eMMC_debug(0,0,"DDR\n");  break;
	case FCIE_eMMC_SDR:
		eMMC_debug(0,0,"SDR\n");  break;
	case FCIE_eMMC_BYPASS:
		eMMC_debug(0,0,"BYPASS\n");  break;
	default:
		eMMC_debug(0,0,"eMMC Err: Pad unknown, %Xh\n", g_eMMCDrv.u8_PadType);  eMMC_die("\n");
	}

    eMMC_debug(0, 0, "reg_all_pad_in (0x%08X):0x%04X\n", reg_all_pad_in, REG_FCIE_U16(reg_all_pad_in));
    eMMC_debug(0, 0, "reg_emmc_pad (0x%08X):0x%04X\n", reg_emmc_pad, REG_FCIE_U16(reg_emmc_pad));
    eMMC_debug(0, 0, "reg_fcie2macro_sd_bypass (0x%08X):0x%04X\n", reg_fcie2macro_sd_bypass, REG_FCIE_U16(reg_fcie2macro_sd_bypass));
    eMMC_debug(0, 0, "reg_emmc_pad (0x%08X):0x%04X\n", reg_emmc_pad, REG_FCIE_U16(reg_emmc_pad));
	eMMC_debug(0, 0, "reg_nand_pad (0x%08X):0x%04X\n", reg_nand_pad, REG_FCIE_U16(reg_nand_pad));

    eMMC_debug(0, 0, "\n");
}

static U16 sgu16_eMMC_Pad=0xFFFF; // initialized by eMMC_platformInit

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	U16 u16_FCIE_Reg10h;

	// avoid possible glitch in clock
	REG_FCIE_R(FCIE_SD_MODE, u16_FCIE_Reg10h);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT10|BIT11|BIT12);

    switch(u32_FCIE_IF_Type){
        case FCIE_eMMC_DDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: DDR\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value
            REG_FCIE_CLRBIT(reg_fcie2macro_sd_bypass, FCIE2MACRO_SD_BYPASS);
			REG_FCIE_SETBIT(reg_chip_dummy1,BIT_DDR_TIMING_PATCH);
            REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT_ddr_timing_patch);

			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT9);
            g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
            break;

        case FCIE_eMMC_SDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: SDR\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value
			REG_FCIE_CLRBIT(reg_fcie2macro_sd_bypass, FCIE2MACRO_SD_BYPASS);
			REG_FCIE_CLRBIT(reg_chip_dummy1,BIT_DDR_TIMING_PATCH);
            REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT_ddr_timing_patch);

			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8);
            g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
			break;

        case FCIE_eMMC_BYPASS:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: BYPASS\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value
            REG_FCIE_SETBIT(reg_fcie2macro_sd_bypass, FCIE2MACRO_SD_BYPASS);
			REG_FCIE_CLRBIT(reg_chip_dummy1,BIT_DDR_TIMING_PATCH);
            REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT_ddr_timing_patch);

            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT10|BIT11);
            g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
            break;

        default:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
            REG_FCIE_W(FCIE_SD_MODE, u16_FCIE_Reg10h);
            return eMMC_ST_ERR_INVALID_PARAM;
    }

    // set chiptop
    if(0xFFFF == sgu16_eMMC_Pad)
    {
		REG_FCIE_R(reg_emmc_pad, sgu16_eMMC_Pad);
		sgu16_eMMC_Pad &= CFG_EMMC_PAD_MASK;
    }
	REG_FCIE_SETBIT(reg_emmc_pad, sgu16_eMMC_Pad);
    REG_FCIE_CLRBIT(reg_nand_pad, CFG_NAND_PAD_MASK);
    REG_FCIE_CLRBIT(reg_all_pad_in, BIT_ALL_PAD_IN);
    // fcie driving strength
    REG_FCIE_SETBIT(reg_emmc_drv, CFG_EMMC_CMD_STRENGTH);
    REG_FCIE_SETBIT(reg_emmc_drv, CFG_EMMC_CLK_STRENGTH);
    REG_FCIE_SETBIT(reg_nand_drv, CFG_NAND_DRV_STRENGTH);

	REG_FCIE_W(FCIE_SD_MODE, u16_FCIE_Reg10h);
	return eMMC_ST_SUCCESS;
}

U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam)	{
	case BIT_FCIE_CLK_300K:     g_eMMCDrv.u32_ClkKHz = 300;     break;
	case BIT_FCIE_CLK_12M:      g_eMMCDrv.u32_ClkKHz = 12000;   break;
	case BIT_FCIE_CLK_18M:      g_eMMCDrv.u32_ClkKHz = 18000;   break;
    case BIT_FCIE_CLK_27M:      g_eMMCDrv.u32_ClkKHz = 27000;   break;
	//case BIT_FCIE_CLK_32M:      g_eMMCDrv.u32_ClkKHz = 32000;   break;
    case BIT_FCIE_CLK_36M:      g_eMMCDrv.u32_ClkKHz = 36000;   break;
	case BIT_FCIE_CLK_40M:      g_eMMCDrv.u32_ClkKHz = 40000;   break;
    //case BIT_FCIE_CLK_43_2M:    g_eMMCDrv.u32_ClkKHz = 43200;   break;
    case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
	default:
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie, u16_ClkParam<<BIT_FCIE_CLK_SHIFT);
	REG_FCIE_SETBIT(reg_ckg_nfie_div4_en, BIT_FCIE_CLK_DIV4_EN);

	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "clk:%uKHz, Param:%Xh, fcie(%Xh):%Xh, fcie_4X_en_div4_en(%Xh):%Xh\n",
		g_eMMCDrv.u32_ClkKHz, u16_ClkParam,
		reg_ckg_fcie, REG_FCIE_U16(reg_ckg_fcie),
		reg_ckg_nfie_div4_en, REG_FCIE_U16(reg_ckg_nfie_div4_en));

	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
    eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_gating(void)
{
	eMMC_PlatformResetPre();
	g_eMMCDrv.u32_ClkKHz = 0;
	REG_FCIE_W(reg_ckg_fcie, BIT_FCIE_CLK_Gate);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_40M,
	BIT_FCIE_CLK_36M,
	BIT_FCIE_CLK_27M,
	BIT_FCIE_CLK_18M,

};

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
static char *sgachar_string[]={"  0T","0.5T","  1T","1.5T", "  2T", "2.5T", "  3T", "3.5T"};

static U32 eMMC_FCIE_DetectDDRTiming_Ex(U8 u8_DQS, U8 u8_DelaySel)
{
	U32 u32_SectorAddr;

	u32_SectorAddr = eMMC_TEST_BLK_0;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\ndqs:%s[%Xh]  cell:%02Xh \n",
		sgachar_string[u8_DQS], u8_DQS, u8_DelaySel);

	eMMC_FCIE_SetDDR48TimingReg(u8_DQS, u8_DelaySel);
    return eMMCTest_BlkWRC_ProbeTiming(u32_SectorAddr);
}

#define FCIE_DELAY_CELL_ts         300 // 0.3ns
static eMMC_FCIE_DDRT_WINDOW_t sg_DDRTWindow[2];

U32 eMMC_FCIE_DetectDDRTiming(void)
{
	U8  u8_dqs, u8_delay_sel, u8_i;
	U8  u8_dqs_prev=0xFF, u8_delay_sel_prev=0;
	U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  au8_DQS_10T[8]={0,5,10,15,20,25,30,35}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  u8_delay_Sel_max;
	U32 u32_ts;
	U32 u32_err;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow = &sg_DDRTWindow[0];

	g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_TUNING_TTABLE; // just to turn off some log

	//--------------------------------------------------
	sg_DDRTWindow[0].u8_Cnt = 0;
	sg_DDRTWindow[0].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[0].aParam[1].u8_DQS = 0;
    sg_DDRTWindow[1].u8_Cnt = 0;
	sg_DDRTWindow[1].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[1].aParam[1].u8_DQS = 0;
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
	{
		sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]=0;
		sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]=0;
	}

	//--------------------------------------------------
	// calculate delay_Sel_max
	u32_ts = 1000*1000*1000 / g_eMMCDrv.u32_ClkKHz;
	u32_ts >>= 2; // for 4X's 1T

    //--------------------------------------------------
    // no need to try DQS of no delay
	//for(u8_dqs=0; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	for(u8_dqs=1; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n---------------------------\n");

		if(u8_dqs < (BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT))
		    u8_delay_Sel_max =
		        (au8_DQS_10T[u8_dqs+1]-au8_DQS_10T[u8_dqs])
		        *u32_ts/(FCIE_DELAY_CELL_ts*10);
		else
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);

		if(u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT))
		{
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n", u8_delay_Sel_max);
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"invalid, not try\n\n");
			//continue;
		}
		else
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n\n", u8_delay_Sel_max);

		sg_DDRTWindow[0].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;
		sg_DDRTWindow[1].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;

		//for(u8_delay_sel=0; u8_delay_sel<=(BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT); u8_delay_sel++)
		for(u8_delay_sel=0; u8_delay_sel<=u8_delay_Sel_max; u8_delay_sel++)
		{
			u32_err = eMMC_FCIE_DetectDDRTiming_Ex(au8_DQSRegVal[u8_dqs], u8_delay_sel);
			if(eMMC_ST_SUCCESS == u32_err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"  PASS\n\n");
				pWindow->u8_Cnt++;
				if(0 == pWindow->aParam[0].u8_DQS) // save the window head
				{
					pWindow->aParam[0].u8_DQS = u8_dqs; // dqs uses index
					pWindow->aParam[0].u8_Cell = u8_delay_sel;
				}
				pWindow->au8_DQSValidCellCnt[u8_dqs]++;

				u8_dqs_prev = u8_dqs;
				u8_delay_sel_prev = u8_delay_sel;
			}
			else
			{   // save the window tail
				if(0xFF != u8_dqs_prev)
				{
					pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
					pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
				}
				u8_dqs_prev = 0xFF;

				// discard & re-use the window having less PASS cnt
				pWindow =
				    (sg_DDRTWindow[0].u8_Cnt < sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];
				pWindow->u8_Cnt = 0;
				pWindow->aParam[0].u8_DQS = 0;
				pWindow->aParam[1].u8_DQS = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
					pWindow->au8_DQSValidCellCnt[u8_i]=0;
			}
		}
	}

	// for the case of last try is ok
	if(0xFF != u8_dqs_prev)
	{
		pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
		pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
	}

	g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_TUNING_TTABLE;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W0, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[0].u8_Cnt,
		sg_DDRTWindow[0].aParam[0].u8_DQS, sg_DDRTWindow[0].aParam[0].u8_Cell,
		sg_DDRTWindow[0].aParam[1].u8_DQS, sg_DDRTWindow[0].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]);

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W1, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[1].u8_Cnt,
		sg_DDRTWindow[1].aParam[0].u8_DQS, sg_DDRTWindow[1].aParam[0].u8_Cell,
		sg_DDRTWindow[1].aParam[1].u8_DQS, sg_DDRTWindow[1].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]);

	if(sg_DDRTWindow[0].u8_Cnt || sg_DDRTWindow[1].u8_Cnt)
		return eMMC_ST_SUCCESS;
	else
		return eMMC_ST_ERR_NO_OK_DDR_PARAM;

}


U32 eMMC_FCIE_BuildDDRTimingTable(void)
{
	U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  u8_i, u8_ClkIdx, u8_SetIdx, u8_tmp, u8_DqsIdx = 0, u8_CellBase;
	U32 u32_err, u32_ret=eMMC_ST_ERR_NO_OK_DDR_PARAM;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow;

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: building DDR table, please wait... \n");
	memset((void*)&g_eMMCDrv.TimingTable_t, '\0', sizeof(g_eMMCDrv.TimingTable_t));
	u8_SetIdx = 0;

	if(eMMC_IF_NORMAL_SDR()){
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: SDR mode can't detect DDR timing\n");
		u32_err = eMMC_FCIE_EnableFastMode_Ex(FCIE_eMMC_DDR);
		if(eMMC_ST_SUCCESS != u32_err)
		{
		    printk("eMMC Err: set DDR IF fail: %X\n", u32_err);
		    return u32_err;
		}
	}
	for(u8_ClkIdx=0; u8_ClkIdx<eMMC_FCIE_VALID_CLK_CNT; u8_ClkIdx++)
	{

		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");
		eMMC_clock_setting(gau8_FCIEClkSel[u8_ClkIdx]);
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");

		// ---------------------------
		// search and set the Windows
		u32_err = eMMC_FCIE_DetectDDRTiming();

		// ---------------------------
		// set the Table
		if(eMMC_ST_SUCCESS == u32_err)
		{
			#if 0
			// before 12MHz, should at least 2 clk is ok for DDR
			if(eMMC_FCIE_DDRT_SET_CNT-1 == u8_SetIdx &&
				BIT_FCIE_CLK_12M == gau8_FCIEClkSel[u8_ClkIdx])
				u32_ret = eMMC_ST_SUCCESS;
			#endif

			g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk = gau8_FCIEClkSel[u8_ClkIdx];

			// ---------------------------
		 	// select Window
			pWindow = NULL;

			// pick up the Window of Cell=0 case
			if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS == sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[0];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS == sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[1];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

			// ---------------------------
			if(NULL != pWindow)
			{

				// pick up the DQS having max valid cell
				u8_tmp = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
				{
					if(u8_tmp <= pWindow->au8_DQSValidCellCnt[u8_i]){
						u8_tmp = pWindow->au8_DQSValidCellCnt[u8_i];
						u8_DqsIdx = u8_i;
					}
				}
				if(0 != u8_DqsIdx) // do not use 0T, this should be always TRUE
				{
					if(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] >= 7 &&
						pWindow->au8_DQSValidCellCnt[u8_DqsIdx] >= 7)
					{
						g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[u8_DqsIdx];
						g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell = 0; // nice
					}
					else
					{
						u8_tmp = (pWindow->au8_DQSValidCellCnt[u8_DqsIdx] +
							pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1]) / 2;


						if(u8_tmp < pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1])
						{
							g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx-1];

							u8_CellBase = pWindow->au8_DQSTryCellCnt[u8_DqsIdx-1] - pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1];
							g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell =
								u8_CellBase + pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx] - u8_tmp;
						}
						else
						{   g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx];
						    g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell =
								(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx]) / 2;
						}
					}
				}
			}

			// ---------------------------
            // or, pick up the Window of large PASS Cnt
			else //if(NULL == pWindow)
			{
			    pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

				g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[pWindow->aParam[0].u8_DQS];
				g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell =
					(pWindow->aParam[0].u8_Cell + pWindow->aParam[1].u8_Cell)/2;
			}

			#if 0
			// ---------------------------
			// use 12M for Set.Min
			if(eMMC_FCIE_DDRT_SET_CNT-2 == u8_SetIdx)
				u8_ClkIdx = eMMC_FCIE_VALID_CLK_CNT-2;
			#else
			if(FCIE_SLOW_CLK == g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.TimingTable_t.u8_SetCnt = u8_SetIdx + 1;
			#endif

			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT > eMMC_FCIE_DDRT_SET_CNT ?
	    eMMC_FCIE_DDRT_SET_CNT : eMMC_FCIE_VALID_CLK_CNT;
	u8_tmp = u8_tmp > g_eMMCDrv.TimingTable_t.u8_SetCnt ?
		g_eMMCDrv.TimingTable_t.u8_SetCnt : u8_tmp;

	eMMC_DumpTimingTable();

	// ======================================================
	// CAUTION: expect 48MHz can have valid DDRT parameter
	if(g_eMMCDrv.TimingTable_t.Set[eMMC_TIMING_SET_MAX].u8_Clk == gau8_FCIEClkSel[0]
		&& 0 != g_eMMCDrv.TimingTable_t.u8_SetCnt)
		u32_ret = eMMC_ST_SUCCESS;
	else if(g_eMMCDrv.TimingTable_t.Set[eMMC_TIMING_SET_MAX].u8_Clk != gau8_FCIEClkSel[0])
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: max clk can NOT run DDR\n");
	else if(0 == g_eMMCDrv.TimingTable_t.u8_SetCnt)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: min clk can NOT run DDR\n");
	// ======================================================

	// ---------------------------
	// save DDRT Table
	if(eMMC_ST_SUCCESS == u32_ret)
	{

		g_eMMCDrv.TimingTable_t.u32_ChkSum =
		    eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-4);
		memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t));

		eMMC_FCIE_ApplyTimingSet(eMMC_TIMING_SET_MAX);

		u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
	    u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
	    if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	    {
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: %Xh %Xh\n",
			    u32_err, u32_ret);
		    return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	    }
	}
	else
	{
		gau8_eMMC_SectorBuf[0] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>>24);
		gau8_eMMC_SectorBuf[1] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>>16);
		gau8_eMMC_SectorBuf[2] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>> 8);
		gau8_eMMC_SectorBuf[3] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>> 0);

		u32_ret = eMMC_ST_ERR_BUILD_DDRT;
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: fail! please call CAE or RD for helps, %Xh \n",
			u32_ret);
		eMMC_ResetReadyFlag();
		eMMC_Init();
		eMMC_FCIE_EnableSDRMode();

	}

	return u32_ret;
}

#endif // IF_DETECT_eMMC_DDR_TIMING


//---------------------------------------
#if defined(ENABLE_eMMC_INTERRUPT_MODE)&&ENABLE_eMMC_INTERRUPT_MODE

static DECLARE_WAIT_QUEUE_HEAD(fcie_wait);
static volatile U32 fcie_int = 0;

#define eMMC_IRQ_DEBUG    1

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
	volatile u16 u16_Events;

	if((REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE) != BIT_EMMC_ACTIVE)
	{
		return IRQ_NONE;
	}

	// one time enable one bit
	u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

	if(u16_Events & BIT_MIU_LAST_DONE)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CARD_DMA_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
	else if(u16_Events & BIT_SD_CMD_END)
	{
		REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_CMD_END);

		fcie_int = 1;
		wake_up(&fcie_wait);
		return IRQ_HANDLED;
	}

#if eMMC_IRQ_DEBUG
	if(0==fcie_int)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: Int St:%Xh, En:%Xh, Evt:%Xh \n",
		    REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN), u16_Events);
#endif

	return IRQ_NONE;

}


U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
{
	U32 u32_i=0;

    #if eMMC_IRQ_DEBUG
	U32 u32_isr_tmp[2];
	unsigned long long u64_jiffies_tmp, u64_jiffies_now;
	struct timeval time_st;
	time_t sec_tmp;
	suseconds_t us_tmp;

	u32_isr_tmp[0] = fcie_int;
	do_gettimeofday(&time_st);
	sec_tmp = time_st.tv_sec;
	us_tmp = time_st.tv_usec;
	u64_jiffies_tmp = jiffies_64;
    #endif

	//----------------------------------------
	if(wait_event_timeout(fcie_wait, (fcie_int == 1), usecs_to_jiffies(u32_MicroSec)) == 0)
    {
	    #if eMMC_IRQ_DEBUG
		u32_isr_tmp[1] = fcie_int;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh, ISR:%u->%u->%u \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN),
			u32_isr_tmp[0], u32_isr_tmp[1], fcie_int);

		do_gettimeofday(&time_st);
		u64_jiffies_now = jiffies_64;
	    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
		    " PassTime: %lu s, %lu us, %llu jiffies.  WaitTime: %u us, %lu jiffies, HZ:%u.\n",
		    time_st.tv_sec-sec_tmp, time_st.tv_usec-us_tmp, u64_jiffies_now-u64_jiffies_tmp,
		    u32_MicroSec, usecs_to_jiffies(u32_MicroSec), HZ);
	    #else
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN));
	    #endif

		// switch to polling
        for(u32_i=0; u32_i<u32_MicroSec; u32_i++)
        {
            if((REG_FCIE(u32_RegAddr) & u16_WaitEvent) == u16_WaitEvent )
                break;

            eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        }

		if(u32_i == u32_MicroSec)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: events lose, WaitEvent: %Xh \n", u16_WaitEvent);
			eMMC_DumpDriverStatus();  eMMC_DumpPadClk();
		    eMMC_FCIE_DumpRegisters();eMMC_FCIE_DumpDebugBus();
			return eMMC_ST_ERR_INT_TO;
		}
		else
		{	REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
		}

    }

	//----------------------------------------
	if(u16_WaitEvent & BIT_MIU_LAST_DONE)
	{
		for(u32_i=0; u32_i<TIME_WAIT_1_BLK_END; u32_i++)
		{
			if(REG_FCIE(u32_RegAddr) & BIT_CARD_DMA_END)
				break; // should be very fase
			eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
		}

		if(TIME_WAIT_1_BLK_END == u32_i)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no CARD_DMA_END\n");
			eMMC_FCIE_ErrHandler_Stop();
		}
	}

	//----------------------------------------
	fcie_int = 0;
	return eMMC_ST_SUCCESS;

}


#endif


int mstar_mci_Housekeep(void *pData)
{
	#if !(defined(eMMC_HOUSEKEEP_THREAD) && eMMC_HOUSEKEEP_THREAD)
    return 0;
	#endif

	#if eMMC_PROFILE_WR
	g_eMMCDrv.u32_CNT_MinRBlk = 0xFFFFFFFF;
	g_eMMCDrv.u32_CNT_MinWBlk = 0xFFFFFFFF;
	#endif

	while(1)
	{
		#if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
		msleep(1000);
		eMMC_dump_WR_Count();
		#endif

		if(kthread_should_stop())
			break;
	}

	return 0;
}


int mstar_mci_exit_checkdone_Ex(void)
{
    U16 u16Reg = 0;

    u16Reg = REG_FCIE(FCIE_SD_STATUS);

    if(u16Reg & BIT_SD_D0)
        return 1;
    else
        return 0;
}

bool mstar_mci_exit_checkdone_ForCI(void)
{
	#if 0
	U32 u32_Cnt=0;

    // nike eMMC_mode_1 share pins
	if(BIT_eMMC_CFG_MODE1 != (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
		return true;

	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			eMMC_debug(0,1,"eMMC Info: PCM check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}
	#endif
	return true;
}

EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForCI);


bool mstar_mci_exit_checkdone_ForSD(void)
{
	U32 u32_Cnt=0;

    // nike SD_mode_x does not share pins
	//if(BIT_eMMC_CFG_MODE2 == (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
	//	return TRUE;
    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
			eMMC_debug(0,1,"eMMC Info: SD check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	return true;
}
EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForSD);


U32 mstar_SD_CardChange(void)
{
	return 0;
}

//---------------------------------------

extern struct semaphore	PfModeSem;
extern bool ms_sdmmc_wait_d0_for_emmc(void);
#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
static U16 sgu16_SD_Pad;
#endif

void eMMC_LockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	down(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	if(false == ms_sdmmc_wait_d0_for_emmc())
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SD keep D0 low \n");
		eMMC_FCIE_ErrHandler_Stop();
	}

	eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
	eMMC_pads_switch(g_eMMCDrv.u8_PadType);

	// backup sd_config/sd_config2, and clear them
	REG_FCIE_R(reg_sd_pad, sgu16_SD_Pad);
	sgu16_SD_Pad &= CFG_SD_PAD_MASK;
	REG_FCIE_CLRBIT(reg_sd_pad, CFG_SD_PAD_MASK);
	#else
	REG_FCIE_CLRBIT(reg_sd_pad, CFG_SD_PAD_MASK);
	#endif

    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	REG_FCIE_SETBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);
}


void eMMC_UnlockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	REG_FCIE_CLRBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	// restore sd_config/sd_config2
    REG_FCIE_SETBIT(reg_sd_pad, sgu16_SD_Pad);
	#endif

	up(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);
}

//---------------------------------------

U32 eMMC_PlatformResetPre(void)
{
	/**((volatile unsigned short *)(0x25007DCC))|=0x02;	// emi mask
	*((volatile unsigned short *)(0x25007C18))|=0x02;	// imi0 mask
	*((volatile unsigned short *)(0x25007C58))|=0x02;	// imi1 mask
	*/
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	/**((volatile unsigned short *)(0x25007DCC))&=(~0x02);	// emi unmask
	*((volatile unsigned short *)(0x25007C18))&=(~0x02);	// imi0 unmask
	*((volatile unsigned short *)(0x25007C58))&=(~0x02);	// imi1 unmask
	*/
    #if defined(ENABLE_EMMC_POWER_SAVING_MODE) && ENABLE_EMMC_POWER_SAVING_MODE
	eMMC_Prepare_Power_Saving_Mode_Queue();
    #endif


	REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT_csreg_miu_wp_last2_sel);
	return eMMC_ST_SUCCESS;
}


U32 eMMC_PlatformInit(void)
{
	static U8 initialized = 0;

	if(initialized == 0)
	{
		REG_FCIE_R(reg_emmc_pad, sgu16_eMMC_Pad);
		sgu16_eMMC_Pad &= CFG_EMMC_PAD_MASK;
		if(sgu16_eMMC_Pad!=BIT_EMMC_CFG_MODE1)
		{
			eMMC_die("\n");
		}
		initialized = 1;
	}

	eMMC_pads_switch(FCIE_DEFAULT_PAD);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	return eMMC_ST_SUCCESS;
}


U32 eMMC_PlatformDeinit(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_StartSector += BL_BLK_OFFSET;
		break;

	case eMMC_PART_OTP:
		u32_StartSector += OTP_BLK_OFFSET;
		break;

	case eMMC_PART_SECINFO:
		u32_StartSector += SecInfo_BLK_OFFSET;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"SecAddr: %Xh, SecCnt: %Xh\n", u32_StartSector, u32_SectorCnt);

	if(eMMC_BOOT_PART_W == u8_OP)
		return eMMC_WriteBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);
	else
		return eMMC_ReadBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);

}


U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
	U32 u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end;

	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_eMMCBlkAddr_start = 0;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+BL_BLK_CNT-1;
		break;

	case eMMC_PART_OTP:
		u32_eMMCBlkAddr_start = OTP_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+OTP_BLK_CNT-1;
		break;

	case eMMC_PART_SECINFO:
		u32_eMMCBlkAddr_start = SecInfo_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+SecInfo_BLK_CNT-1;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"BlkAddr_start: %Xh, BlkAddr_end: %Xh\n",
		u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);

    return eMMC_EraseBootPart(u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end, 1);

}


char *gpas8_eMMCPartName[]={
	"e2pbak", "nvrambak", "hwcfgs", "recovery", "os",
	"fdd", "tdd", "blogo", "apanic","misc", "cus",
	"e2p0","e2p1","nvram0","nvram1", "system","cache", "data", "internal sd"};

// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

U32 eMMC_CheckIfMemCorrupt(void)
{
	if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
		return eMMC_ST_ERR_MEM_CORRUPT;

	return eMMC_ST_SUCCESS;
}
// <-- [FIXME]

//===================================================
#elif (defined(eMMC_DRV_EINSTEIN_LINUX)&&eMMC_DRV_EINSTEIN_LINUX) // [FIXME] clone for your flatform

// check some fix value, print only when setting wrong
void eMMC_DumpPadClk(void)
{
	U16 u16_i, u16_reg;

	//---------------------------------------------------------------------
	eMMC_debug(eMMC_DEBUG_LEVEL, 1, "reg_ckg_fcie(0x%X):0x%x", reg_ckg_fcie ,REG_FCIE_U16(reg_ckg_fcie));
    eMMC_debug(eMMC_DEBUG_LEVEL, 0,"  FCIE Clk: %uKHz\n", g_eMMCDrv.u32_ClkKHz);
	eMMC_debug(eMMC_DEBUG_LEVEL, 0,"  Reg Val: %Xh\n", g_eMMCDrv.u16_ClkRegVal);

	eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\n\nemmcpll:");

	for(u16_i=0 ; u16_i<0x60; u16_i++)
	{
		if(0 == u16_i%8)
			eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n%02Xh:| ", u16_i);

		REG_FCIE_R(GET_REG_ADDR(EMMC_PLL_BASE, u16_i), u16_reg);
		eMMC_debug(eMMC_DEBUG_LEVEL, 0, "%04Xh ", u16_reg);
	}

	//---------------------------------------------------------------------
	eMMC_debug(0, 0, "\n\n[pad setting]: ");
	switch(g_eMMCDrv.u8_PadType)
	{
		case FCIE_eMMC_BYPASS:	eMMC_debug(0, 0, "BYPASS\n");	break;
		case FCIE_eMMC_SDR:	eMMC_debug(0, 0, "SDR\n");	break;
		case FCIE_eMMC_DDR:	eMMC_debug(0, 0, "DDR\n");	break;
		case FCIE_eMMC_HS200:	eMMC_debug(0, 0, "HS200\n");	break;
		default:
			eMMC_debug(0, 0, "eMMC Err: Pad unknown, %d\n", g_eMMCDrv.u8_PadType); eMMC_die("\n");
			break;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\nchiptop:");

	for(u16_i=0 ; u16_i<0x80; u16_i++)
	{
		if(0 == u16_i%8)
			eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n%02Xh:| ", u16_i);

		REG_FCIE_R(GET_REG_ADDR(PAD_CHIPTOP_BASE, u16_i), u16_reg);
		eMMC_debug(eMMC_DEBUG_LEVEL, 0, "%04Xh ", u16_reg);
	}
	eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\n");

}


// set pad first, then config clock
U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	REG_FCIE_CLRBIT(reg_test_mode, reg_test_out_mode_mask|reg_test_in_mode_mask);
    REG_FCIE_CLRBIT(reg_chiptop_0x64, BIT_CAADCONFIG|BIT_PCMADCONFIG|BIT_PCM2CTRLCONFIG);
	REG_FCIE_SETBIT(reg_sd_use_bypass, BIT_SD_USE_BYPASS);
	REG_FCIE_CLRBIT(reg_nand_config,BIT_NAND_CS1_EN|BIT_NAND_MODE);
    REG_FCIE_CLRBIT(reg_sd_config,BIT_SD_CONFIG);
    REG_FCIE_CLRBIT(reg_sdio_config,BIT_SDIO_CONFIG);
    REG_FCIE_CLRBIT(reg_emmc_config,BIT_EMMC_CONFIG_MODE1);
	REG_FCIE_SETBIT(reg_emmc_config,BIT6);
	REG_FCIE_CLRBIT(reg_chiptop_0x50,BIT_ALL_PAD_IN);

	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT10|BIT11|BIT12|BIT14|BIT15);
	REG_FCIE_CLRBIT(FCIE_SM_STS, BIT_DQS_DELAY_CELL_MASK);
    REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_CRC_STATUS_4_HS200|BIT_LATE_DATA0_W_IP_CLK);

	switch (u32_FCIE_IF_Type) {

	case FCIE_eMMC_BYPASS:
		REG_FCIE_SETBIT(reg_fcie2macro_bypass, BIT_FCIE2MACRO_BYPASS);
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT10|BIT11);
		g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
		break;

	case FCIE_eMMC_SDR:
		REG_FCIE_SETBIT(reg_fcie2macro_bypass, BIT_FCIE2MACRO_BYPASS);
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8);
		g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
		break;

	case FCIE_eMMC_DDR:
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT14);
		REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_LATE_DATA0_W_IP_CLK);
		REG_FCIE_CLRBIT(reg_emmcpll_0x03,BIT_CLK_PH_MASK|BIT_DQ_PH_MASK|BIT_CMD_PH_MASK);
        REG_FCIE_SETBIT(reg_emmcpll_0x03,BIT2);
		g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
		REG_FCIE_CLRBIT(reg_fcie2macro_bypass, BIT_FCIE2MACRO_BYPASS); // move to last to prevent glitch
		break;

	case FCIE_eMMC_HS200:
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT14|BIT15);
		REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_LATE_DATA0_W_IP_CLK);
		REG_FCIE_CLRBIT(reg_emmcpll_0x03,BIT_CLK_PH_MASK|BIT_DQ_PH_MASK|BIT_CMD_PH_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03,BIT3);
		g_eMMCDrv.u8_PadType = FCIE_eMMC_HS200;
		REG_FCIE_CLRBIT(reg_fcie2macro_bypass, BIT_FCIE2MACRO_BYPASS); // move to last to prevent glitch
		break;

	default:
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: unknown interface: %X\n", u32_FCIE_IF_Type);
		eMMC_die(" ");
		return eMMC_ST_ERR_INVALID_PARAM;
		break;
	}

	//eMMC_debug(1, 0, "eMMC pads: %Xh\n", g_eMMCDrv.u8_PadType);
	return eMMC_ST_SUCCESS;

}

U32 eMMC_pll_setting(U16 u16_ClkParam)
{
	static U16 u16_OldClkParam=0xFFFF;
    U32 u32_value_reg_emmc_pll_pdiv;
	// HS200 --> 200M, 160M, 140M, 120M
	// DDR52 -->  52M,  48M, 40M

	if(u16_ClkParam == u16_OldClkParam)
		return eMMC_ST_SUCCESS;
	else
		u16_OldClkParam = u16_ClkParam;

	// 1. reset emmc pll
	REG_FCIE_SETBIT(reg_emmc_pll_reset,BIT0);
	REG_FCIE_CLRBIT(reg_emmc_pll_reset,BIT0);

	// 2. synth clock
	switch(u16_ClkParam)	{
		case eMMC_PLL_CLK_200M: // 200M
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x22);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x8F5C);
			//printf("pRegEmmcPll->reg_ddfset_15_00 = %Xh\n", pRegEmmcPll->reg_ddfset_15_00);
			//printf("R: %Xh\n", (U32)&(pRegEmmcPll->reg_ddfset_15_00));
			//printf("R: %Xh\n", *(volatile U32 *)(0x1F247E48)); // wrong addr
			//printf("R: %Xh\n", *(volatile U32 *)(0x1F247E60));
			u32_value_reg_emmc_pll_pdiv = 1; // PostDIV: 2
			break;
		case eMMC_PLL_CLK_160M:
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x2B);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x3333);
			u32_value_reg_emmc_pll_pdiv = 1; // PostDIV: 2
			break;
		case eMMC_PLL_CLK_140M:
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x31);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x5F15);
			u32_value_reg_emmc_pll_pdiv = 1; // PostDIV: 2
			break;
		case eMMC_PLL_CLK_120M:
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x39);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x9999);
			u32_value_reg_emmc_pll_pdiv = 1; // PostDIV: 2
			break;
		case eMMC_PLL_CLK_100M:
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x45);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x1EB8);
			u32_value_reg_emmc_pll_pdiv = 1; // PostDIV: 2
			break;
		case eMMC_PLL_CLK__86M:
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x28);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x2FA0);
			u32_value_reg_emmc_pll_pdiv = 2; // PostDIV: 4
			break;
		case eMMC_PLL_CLK__80M:
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x2B);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x3333);
			u32_value_reg_emmc_pll_pdiv = 2; // PostDIV: 4
			break;
		case eMMC_PLL_CLK__72M:
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x30);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x0000);
			u32_value_reg_emmc_pll_pdiv = 2; // PostDIV: 4
			break;
		case eMMC_PLL_CLK__62M:
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x37);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0xBDEF);
			u32_value_reg_emmc_pll_pdiv = 4; // PostDIV: 4
			break;
		case eMMC_PLL_CLK__52M:
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x42);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x7627);
			u32_value_reg_emmc_pll_pdiv = 2; // PostDIV: 4
			break;
		case eMMC_PLL_CLK__48M:
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x48);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x0000);
			u32_value_reg_emmc_pll_pdiv = 2; // PostDIV: 4
			break;
		case eMMC_PLL_CLK__40M:
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x2B);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x3333);
			u32_value_reg_emmc_pll_pdiv = 4; // PostDIV: 8
			break;
		case eMMC_PLL_CLK__36M:
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x30);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x0000);
			u32_value_reg_emmc_pll_pdiv = 4; // PostDIV: 8
			break;
		case eMMC_PLL_CLK__32M:
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x36);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x0000);
			u32_value_reg_emmc_pll_pdiv = 4; // PostDIV: 8
			break;
		case eMMC_PLL_CLK__27M:
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x40);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x0000);
			u32_value_reg_emmc_pll_pdiv = 4; // PostDIV: 8
			break;
		case eMMC_PLL_CLK__20M:
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x2B);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x3333);
			u32_value_reg_emmc_pll_pdiv = 7; // PostDIV: 16
			break;
		default:
			eMMC_debug(1, 0, "eMMC Err: emmc PLL not configed\n");
			eMMC_die("\n");
			return eMMC_ST_ERR_UNKNOWN_CLK;
			break;
	}

	// 3. VCO clock ( loop N = 4 )
	REG_FCIE_CLRBIT(reg_emmcpll_fbdiv,0xffff);
	REG_FCIE_SETBIT(reg_emmcpll_fbdiv,0x6);// PostDIV: 8


	// 4. 1X clock
    REG_FCIE_CLRBIT(reg_emmcpll_pdiv,BIT2|BIT1|BIT0);
	REG_FCIE_SETBIT(reg_emmcpll_pdiv,u32_value_reg_emmc_pll_pdiv);// PostDIV: 8

	if(u16_ClkParam==eMMC_PLL_CLK__20M) {
		REG_FCIE_SETBIT(reg_emmc_pll_test, BIT10);
	}
	else {
		REG_FCIE_CLRBIT(reg_emmc_pll_test, BIT10);
	}

	return eMMC_ST_SUCCESS;
}


// Notice!!! you need to set pad before config clock
U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam) {
		case eMMC_PLL_CLK__20M	: g_eMMCDrv.u32_ClkKHz =  20000; break;
		case eMMC_PLL_CLK__27M	: g_eMMCDrv.u32_ClkKHz =  27000; break;
		case eMMC_PLL_CLK__32M	: g_eMMCDrv.u32_ClkKHz =  32000; break;
		case eMMC_PLL_CLK__36M	: g_eMMCDrv.u32_ClkKHz =  36000; break;
		case eMMC_PLL_CLK__40M	: g_eMMCDrv.u32_ClkKHz =  40000; break;
		case eMMC_PLL_CLK__48M	: g_eMMCDrv.u32_ClkKHz =  48000; break;
		case eMMC_PLL_CLK__52M	: g_eMMCDrv.u32_ClkKHz =  52000; break;
		case eMMC_PLL_CLK__62M	: g_eMMCDrv.u32_ClkKHz =  62000; break;
		case eMMC_PLL_CLK__72M	: g_eMMCDrv.u32_ClkKHz =  72000; break;
		case eMMC_PLL_CLK__80M	: g_eMMCDrv.u32_ClkKHz =  80000; break;
		case eMMC_PLL_CLK__86M	: g_eMMCDrv.u32_ClkKHz =  86000; break;
		case eMMC_PLL_CLK_100M	: g_eMMCDrv.u32_ClkKHz = 100000; break;
		case eMMC_PLL_CLK_120M	: g_eMMCDrv.u32_ClkKHz = 120000; break;
		case eMMC_PLL_CLK_140M	: g_eMMCDrv.u32_ClkKHz = 140000; break;
		case eMMC_PLL_CLK_160M	: g_eMMCDrv.u32_ClkKHz = 160000; break;
		case eMMC_PLL_CLK_200M	: g_eMMCDrv.u32_ClkKHz = 200000; break;

		case BIT_FCIE_CLK_20M	: g_eMMCDrv.u32_ClkKHz =  20000; break;
		case BIT_FCIE_CLK_27M	: g_eMMCDrv.u32_ClkKHz =  27000; break;
		case BIT_FCIE_CLK_32M	: g_eMMCDrv.u32_ClkKHz =  32000; break;
		case BIT_FCIE_CLK_36M	: g_eMMCDrv.u32_ClkKHz =  36000; break;
		case BIT_FCIE_CLK_40M	: g_eMMCDrv.u32_ClkKHz =  40000; break;
		case BIT_FCIE_CLK_43_2M	: g_eMMCDrv.u32_ClkKHz =  43200; break;
		case BIT_FCIE_CLK_300K	: g_eMMCDrv.u32_ClkKHz =    300; break;
		case BIT_FCIE_CLK_48M	: g_eMMCDrv.u32_ClkKHz =  48000; break;

		default:
			eMMC_debug(1, 1, "eMMC Err: clkgen %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
			eMMC_die(" ");
			return eMMC_ST_ERR_INVALID_PARAM; break;
	}

	if(u16_ClkParam & eMMC_PLL_FLAG) {
		REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_FCIE_CLK_GATING|BIT_FCIE_CLK_INVERSE|BIT_CLKGEN_FCIE_MASK);
		REG_FCIE_SETBIT(reg_ckg_fcie, BIT_FCIE_CLK_SRC_SEL|BIT_FCIE_CLK_EMMC_PLL<<2);
		eMMC_pll_setting(u16_ClkParam);
	}
	else {
	    REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_FCIE_CLK_GATING|BIT_FCIE_CLK_INVERSE|BIT_CLKGEN_FCIE_MASK);
        REG_FCIE_SETBIT(reg_ckg_fcie, BIT_FCIE_CLK_SRC_SEL|u16_ClkParam<<2);
	}

	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
	eMMC_PlatformResetPost();

	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_gating(void)
{
	eMMC_PlatformResetPre();
	g_eMMCDrv.u32_ClkKHz = 0;
	REG_FCIE_SETBIT(reg_ckg_fcie, BIT_FCIE_CLK_GATING);// gate clock
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}

U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT] = {
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_36M,
	BIT_FCIE_CLK_20M
};

//---------------------------------------
#if defined(ENABLE_eMMC_INTERRUPT_MODE)&&ENABLE_eMMC_INTERRUPT_MODE

static DECLARE_WAIT_QUEUE_HEAD(fcie_wait);
static volatile U32 fcie_int = 0;

#define eMMC_IRQ_DEBUG    1

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
	volatile u16 u16_Events;

	if((REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE) != BIT_EMMC_ACTIVE)
	{
		return IRQ_NONE;
	}

	// one time enable one bit
	u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

	if(u16_Events & BIT_MIU_LAST_DONE)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CARD_DMA_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
	else if(u16_Events & BIT_SD_CMD_END)
	{
		REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_CMD_END);

		fcie_int = 1;
		wake_up(&fcie_wait);
		return IRQ_HANDLED;
	}

#if eMMC_IRQ_DEBUG
	if(0==fcie_int)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: Int St:%Xh, En:%Xh, Evt:%Xh \n",
		    REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN), u16_Events);
#endif

	return IRQ_NONE;

}


U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
{
	U32 u32_i=0;

    #if eMMC_IRQ_DEBUG
	U32 u32_isr_tmp[2];
	unsigned long long u64_jiffies_tmp, u64_jiffies_now;
	struct timeval time_st;
	time_t sec_tmp;
	suseconds_t us_tmp;

	u32_isr_tmp[0] = fcie_int;
	do_gettimeofday(&time_st);
	sec_tmp = time_st.tv_sec;
	us_tmp = time_st.tv_usec;
	u64_jiffies_tmp = jiffies_64;
    #endif

	//----------------------------------------
	if(wait_event_timeout(fcie_wait, (fcie_int == 1), usecs_to_jiffies(u32_MicroSec)) == 0)
    {
	    #if eMMC_IRQ_DEBUG
		u32_isr_tmp[1] = fcie_int;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh, ISR:%u->%u->%u \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN),
			u32_isr_tmp[0], u32_isr_tmp[1], fcie_int);

		do_gettimeofday(&time_st);
		u64_jiffies_now = jiffies_64;
	    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
		    " PassTime: %lu s, %lu us, %llu jiffies.  WaitTime: %u us, %lu jiffies, HZ:%u.\n",
		    time_st.tv_sec-sec_tmp, time_st.tv_usec-us_tmp, u64_jiffies_now-u64_jiffies_tmp,
		    u32_MicroSec, usecs_to_jiffies(u32_MicroSec), HZ);
	    #else
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN));
	    #endif

		// switch to polling
        for(u32_i=0; u32_i<u32_MicroSec; u32_i++)
        {
            if((REG_FCIE(u32_RegAddr) & u16_WaitEvent) == u16_WaitEvent )
                break;

            eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        }

		if(u32_i == u32_MicroSec)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: events lose, WaitEvent: %Xh \n", u16_WaitEvent);
			eMMC_DumpDriverStatus();  eMMC_DumpPadClk();
		    eMMC_FCIE_DumpRegisters();eMMC_FCIE_DumpDebugBus();
			return eMMC_ST_ERR_INT_TO;
		}
		else
		{	REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
		}

    }

	//----------------------------------------
	if(u16_WaitEvent & BIT_MIU_LAST_DONE)
	{
		for(u32_i=0; u32_i<TIME_WAIT_1_BLK_END; u32_i++)
		{
			if(REG_FCIE(u32_RegAddr) & BIT_CARD_DMA_END)
				break; // should be very fase
			eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
		}

		if(TIME_WAIT_1_BLK_END == u32_i)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no CARD_DMA_END\n");
			eMMC_FCIE_ErrHandler_Stop();
		}
	}

	//----------------------------------------
	fcie_int = 0;
	return eMMC_ST_SUCCESS;

}


#endif

int mstar_mci_Housekeep(void *pData)
{
	#if !(defined(eMMC_HOUSEKEEP_THREAD) && eMMC_HOUSEKEEP_THREAD)
    return 0;
	#endif

	#if eMMC_PROFILE_WR
	g_eMMCDrv.u32_CNT_MinRBlk = 0xFFFFFFFF;
	g_eMMCDrv.u32_CNT_MinWBlk = 0xFFFFFFFF;
	#endif

	while(1)
	{
		#if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
		msleep(1000);
		eMMC_dump_WR_Count();
		#endif

		if(kthread_should_stop())
			break;
	}

	return 0;
}


int mstar_mci_exit_checkdone_Ex(void)
{
    U16 u16Reg = 0;

    u16Reg = REG_FCIE(FCIE_SD_STATUS);

    if(u16Reg & BIT_SD_D0)
        return 1;
    else
        return 0;
}

bool mstar_mci_exit_checkdone_ForCI(void)
{
	#if 0
	U32 u32_Cnt=0;

    // eiffel eMMC_mode_1 share pins
	if(BIT_eMMC_CFG_MODE1 != (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
		return true;

	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			eMMC_debug(0,1,"eMMC Info: PCM check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}
	#endif
	return true;
}

EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForCI);


bool mstar_mci_exit_checkdone_ForSD(void)
{
	#if 0
	U32 u32_Cnt=0;

    // eiffel SD_mode_x does not share pins
	//if(BIT_eMMC_CFG_MODE2 == (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
	//	return TRUE;
    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
			eMMC_debug(0,1,"eMMC Info: SD check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	#endif
	return true;
}
EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForSD);


U32 mstar_SD_CardChange(void)
{
	return 0;
}

//---------------------------------------
extern struct semaphore	PfModeSem;
extern bool ms_sdmmc_wait_d0_for_emmc(void);

void eMMC_LockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	down(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	if(false == ms_sdmmc_wait_d0_for_emmc())
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SD keep D0 low \n");
		eMMC_FCIE_ErrHandler_Stop();
	}

	eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
	eMMC_pads_switch(g_eMMCDrv.u8_PadType);
	#endif

    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	REG_FCIE_SETBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);
}

void eMMC_UnlockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	REG_FCIE_CLRBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);
	up(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);
}

//---------------------------------------

U32 eMMC_PlatformResetPre(void)
{
	/**((volatile unsigned short *)(0x25007DCC))|=0x02;	// emi mask
	*((volatile unsigned short *)(0x25007C18))|=0x02;	// imi0 mask
	*((volatile unsigned short *)(0x25007C58))|=0x02;	// imi1 mask
	*/
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	/**((volatile unsigned short *)(0x25007DCC))&=(~0x02);	// emi unmask
	*((volatile unsigned short *)(0x25007C18))&=(~0x02);	// imi0 unmask
	*((volatile unsigned short *)(0x25007C58))&=(~0x02);	// imi1 unmask
	*/
    #if defined(ENABLE_EMMC_POWER_SAVING_MODE) && ENABLE_EMMC_POWER_SAVING_MODE
	eMMC_Prepare_Power_Saving_Mode_Queue();
    #endif
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
    #if 1	// for eMMC 4.5 HS200 need 1.8V, unify all eMMC IO power to 1.8V
	// works both for eMMC 4.4 & 4.5
	// printf("1.8V IO power for eMMC\n");
	// Irwin Tyan: set this bit to boost IO performance at low power supply.

	if((REG_FCIE(reg_emmc_test)&BIT0) != 1) {
		//eMMC_debug(0, 0, "eMMC Err: not 1.8V IO setting\n");
		REG_FCIE_SETBIT(reg_emmc_test, BIT0); // 1.8V must set this bit
		REG_FCIE_CLRBIT(reg_nand_pad_driving, 0xffff);// 1.8V must set this bit
		REG_FCIE_SETBIT(reg_nand_pad_driving, 0xf);// 1.8V must set this bit
	}
    #else
	//printf("3.3V IO power for eMMC\n");
	pRegEmmcPll->reg_emmc_test = 0; // 3.3V must clear this bit
    #endif

	#if defined(CONFIG_ENABLE_EMMC_ACP) && CONFIG_ENABLE_EMMC_ACP
	//set once only , ACP can't be disabled
	eMMC_debug(0, 1, "eMMC: enable ACP\n");

	REG_FCIE_CLRBIT(reg_1239_Rx16, BIT8|BIT9|BIT10);

	REG_FCIE_CLRBIT(reg_1239_Rx26, BIT8|BIT9|BIT10);
	REG_FCIE_SETBIT(reg_100E_Rx04, BIT6);
	#endif

	eMMC_pads_switch(FCIE_DEFAULT_PAD);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	//pRegChiptop->reg_emmc_pad_driving = 0xF; // emmc pad driving

	//eMMC_SkewSelectTester();
	//printf("gau8_eMMC_SectorBuf --> %08Xh\n", (U32)gau8_eMMC_SectorBuf);
	//printf("gau8_eMMC_PartInfoBuf --> %08Xh\n", (U32)gau8_eMMC_PartInfoBuf);

	return eMMC_ST_SUCCESS;
}


U32 eMMC_PlatformDeinit(void)
{
	return eMMC_ST_SUCCESS;
}

// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

U32 eMMC_CheckIfMemCorrupt(void)
{
	if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
		return eMMC_ST_ERR_MEM_CORRUPT;

	return eMMC_ST_SUCCESS;
}
// <-- [FIXME]

//=============================================================
#elif (defined(eMMC_DRV_KAISER_LINUX)&&eMMC_DRV_KAISER_LINUX) || \
      (defined(eMMC_DRV_KAISERS_LINUX)&&eMMC_DRV_KAISERS_LINUX) // [FIXME] clone for your flatform
// [FIXME] -->
void eMMC_DumpPadClk(void)
{
	//----------------------------------------------
    eMMC_debug(0, 0, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
    eMMC_debug(0, 0, "FCIE 1X (0x%X):0x%04X\n", reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X));
    eMMC_debug(0, 0, "FCIE 4X (0x%X):0x%04X\n", reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));
    eMMC_debug(0, 0, "MIU (0x%X):0x%04X\n", reg_ckg_MIU, REG_FCIE_U16(reg_ckg_MIU));
    eMMC_debug(0, 0, "MCU (0x%X):0x%04X\n", reg_ckg_MCU, REG_FCIE_U16(reg_ckg_MCU));

    //----------------------------------------------
    eMMC_DumpChiptop();

    eMMC_debug(0, 0, "\n[pad setting]: ");
    switch(g_eMMCDrv.u8_PadType)
	{
	case FCIE_eMMC_DDR:
		eMMC_debug(0,0,"DDR\n");  break;
	case FCIE_eMMC_SDR:
		eMMC_debug(0,0,"SDR\n");  break;
	case FCIE_eMMC_BYPASS:
		eMMC_debug(0,0,"BYPASS\n");  break;
	default:
		eMMC_debug(0,0,"eMMC Err: Pad unknown, %Xh\n", g_eMMCDrv.u8_PadType);  eMMC_die("\n");
	}

    eMMC_debug(0, 0, "reg_all_pad_in (0x%08X):0x%04X\n", reg_all_pad_in, REG_FCIE_U16(reg_all_pad_in));

    eMMC_debug(0, 0, "reg_emmc_pad (0x%08X):0x%04X\n", reg_emmc_pad, REG_FCIE_U16(reg_emmc_pad));

    eMMC_debug(0, 0, "reg_fcie2macro_sd_bypass (0x%08X):0x%04X\n", reg_fcie2macro_sd_bypass, REG_FCIE_U16(reg_fcie2macro_sd_bypass));

    eMMC_debug(0, 0, "\n");
}

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT10|BIT11|BIT12);
	// reg_fcie2marco_sd_bypass
    REG_FCIE_CLRBIT(reg_fcie2macro_sd_bypass, BIT_FCIE2MACRO_SD_BYPASS);

    switch(u32_FCIE_IF_Type){
        case FCIE_eMMC_DDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: DDR\n");

			REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT0);
            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT9);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
            break;

        case FCIE_eMMC_SDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: SDR\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
            break;

        case FCIE_eMMC_BYPASS:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: BYPASS\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value
            REG_FCIE_SETBIT(reg_fcie2macro_sd_bypass, BIT_FCIE2MACRO_SD_BYPASS);    // reg_fcie2marco_sd_bypass
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT10|BIT11);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
            break;

        default:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
            return eMMC_ST_ERR_INVALID_PARAM;
    }

    // set chiptop
	REG_FCIE_CLRBIT(reg_emmc_pad, CFG_SD_PAD_MASK|CFG_NAND_PAD_MASK|CFG_EMMC_PAD_MASK);
	REG_FCIE_SETBIT(reg_emmc_pad, BIT_EMMC_PAD_MODE1);
    // reg_sd_use_bypass
    REG_FCIE_SETBIT(reg_fcie2macro_sd_bypass, BIT_SD_USE_BYPASS);

	REG_FCIE_CLRBIT(reg_fcie2macro_sd_bypass, BIT_PAD_IN_SEL_SD|BIT_PAD_IN_SEL_SDIO);

	REG_FCIE_CLRBIT(FCIE_REG_2Dh,BIT14|BIT7);

    REG_FCIE_CLRBIT(reg_all_pad_in, BIT_ALL_PAD_IN);

	return eMMC_ST_SUCCESS;

}

#if 1//defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
static U8 sgau8_FCIEClk_1X_To_4X_[0x10]= // index is 1X reg value
{
    0,
    BIT_FCIE_CLK4X_18M,
    0,
    BIT_FCIE_CLK4X_27M,
    0,
    BIT_FCIE_CLK4X_36M,
    BIT_FCIE_CLK4X_40M,
    0,
    BIT_FCIE_CLK4X_43M,
    BIT_FCIE_CLK4X_48M,
    0,
    0,
    0,
    0,
    0,
    0
};
#endif

U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam)	{
	case BIT_FCIE_CLK_300K:     g_eMMCDrv.u32_ClkKHz = 300;     break;
	case BIT_FCIE_CLK_18M:      g_eMMCDrv.u32_ClkKHz = 18000;   break;
    case BIT_FCIE_CLK_27M:      g_eMMCDrv.u32_ClkKHz = 27000;   break;
	case BIT_FCIE_CLK_36M:      g_eMMCDrv.u32_ClkKHz = 36000;   break;
	case BIT_FCIE_CLK_40M:      g_eMMCDrv.u32_ClkKHz = 40000;   break;
    case BIT_FCIE_CLK_43M:      g_eMMCDrv.u32_ClkKHz = 43000;   break;
	case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
	default:
		eMMC_debug(eMMC_DEBUG_LEVEL_LOW,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);

	if(u16_ClkParam == BIT_FCIE_CLK_300K)
	    REG_FCIE_SETBIT(reg_ckg_fcie_1X, (u16_ClkParam<<BIT_FCIE_CLK_SHIFT));
    else
	{
	    REG_FCIE_SETBIT(reg_ckg_fcie_1X, (BIT_FCIE_CLK4X_DIV4 << BIT_FCIE_CLK_SHIFT));	//clk_fcie_4x / 4
	    REG_FCIE_CLRBIT(reg_ckg_fcie_4X, BIT_FCIE_CLK4X_Gate|BIT_FCIE_CLK4X_MASK);
	    REG_FCIE_SETBIT(reg_ckg_fcie_4X, (sgau8_FCIEClk_1X_To_4X_[u16_ClkParam]<<BIT_FCIE_CLK4X_SHIFT));
	}

	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "clk:%uKHz, Param:%Xh, fcie_1X(%Xh):%Xh, fcie_4X(%Xh):%Xh\n",
		g_eMMCDrv.u32_ClkKHz, u16_ClkParam,
		reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X),
		reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));

	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
    eMMC_PlatformResetPost();

	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_gating(void)
{
	eMMC_PlatformResetPre();
	g_eMMCDrv.u32_ClkKHz = 0;
	REG_FCIE_W(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate);
	REG_FCIE_W(reg_ckg_fcie_4X, BIT_FCIE_CLK4X_Gate);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_43M,
    BIT_FCIE_CLK_40M,
    BIT_FCIE_CLK_36M,
    BIT_FCIE_CLK_27M,
    BIT_FCIE_CLK_18M
};

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
static char *sgachar_string[]={"  0T","0.5T","  1T","1.5T", "  2T", "2.5T", "  3T", "3.5T"};

static U32 eMMC_FCIE_DetectDDRTiming_Ex(U8 u8_DQS, U8 u8_DelaySel)
{
	U32 u32_SectorAddr;

	u32_SectorAddr = eMMC_TEST_BLK_0;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\ndqs:%s[%Xh]  cell:%02Xh \n",
		sgachar_string[u8_DQS], u8_DQS, u8_DelaySel);

	eMMC_FCIE_SetDDR48TimingReg(u8_DQS, u8_DelaySel);
    return eMMCTest_BlkWRC_ProbeTiming(u32_SectorAddr);
}

#define FCIE_DELAY_CELL_ts         300 // 0.3ns
static eMMC_FCIE_DDRT_WINDOW_t sg_DDRTWindow[2];

U32 eMMC_FCIE_DetectDDRTiming(void)
{
	U8  u8_dqs, u8_delay_sel, u8_i;
	U8  u8_dqs_prev=0xFF, u8_delay_sel_prev=0;
	U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  au8_DQS_10T[8]={0,5,10,15,20,25,30,35}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  u8_delay_Sel_max;
	U32 u32_ts;
	U32 u32_err;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow = &sg_DDRTWindow[0];

	g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_TUNING_TTABLE; // just to turn off some log

	//--------------------------------------------------
	sg_DDRTWindow[0].u8_Cnt = 0;
	sg_DDRTWindow[0].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[0].aParam[1].u8_DQS = 0;
    sg_DDRTWindow[1].u8_Cnt = 0;
	sg_DDRTWindow[1].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[1].aParam[1].u8_DQS = 0;
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
	{
		sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]=0;
		sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]=0;
	}

	//--------------------------------------------------
	// calculate delay_Sel_max
	u32_ts = 1000*1000*1000 / g_eMMCDrv.u32_ClkKHz;
	u32_ts >>= 2; // for 4X's 1T

    //--------------------------------------------------
    // no need to try DQS of no delay
	//for(u8_dqs=0; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	for(u8_dqs=1; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n---------------------------\n");

		if(u8_dqs < (BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT))
		    u8_delay_Sel_max =
		        (au8_DQS_10T[u8_dqs+1]-au8_DQS_10T[u8_dqs])
		        *u32_ts/(FCIE_DELAY_CELL_ts*10);
		else
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);

		if(u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT))
		{
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n", u8_delay_Sel_max);
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"invalid, not try\n\n");
			//continue;
		}
		else
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n\n", u8_delay_Sel_max);

		sg_DDRTWindow[0].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;
		sg_DDRTWindow[1].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;

		//for(u8_delay_sel=0; u8_delay_sel<=(BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT); u8_delay_sel++)
		for(u8_delay_sel=0; u8_delay_sel<=u8_delay_Sel_max; u8_delay_sel++)
		{
			u32_err = eMMC_FCIE_DetectDDRTiming_Ex(au8_DQSRegVal[u8_dqs], u8_delay_sel);
			if(eMMC_ST_SUCCESS == u32_err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"  PASS\n\n");
				pWindow->u8_Cnt++;
				if(0 == pWindow->aParam[0].u8_DQS) // save the window head
				{
					pWindow->aParam[0].u8_DQS = u8_dqs; // dqs uses index
					pWindow->aParam[0].u8_Cell = u8_delay_sel;
				}
				pWindow->au8_DQSValidCellCnt[u8_dqs]++;

				u8_dqs_prev = u8_dqs;
				u8_delay_sel_prev = u8_delay_sel;
			}
			else
			{   // save the window tail
				if(0xFF != u8_dqs_prev)
				{
					pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
					pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
				}
				u8_dqs_prev = 0xFF;

				// discard & re-use the window having less PASS cnt
				pWindow =
				    (sg_DDRTWindow[0].u8_Cnt < sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];
				pWindow->u8_Cnt = 0;
				pWindow->aParam[0].u8_DQS = 0;
				pWindow->aParam[1].u8_DQS = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
					pWindow->au8_DQSValidCellCnt[u8_i]=0;
			}
		}
	}

	// for the case of last try is ok
	if(0xFF != u8_dqs_prev)
	{
		pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
		pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
	}

	g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_TUNING_TTABLE;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W0, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[0].u8_Cnt,
		sg_DDRTWindow[0].aParam[0].u8_DQS, sg_DDRTWindow[0].aParam[0].u8_Cell,
		sg_DDRTWindow[0].aParam[1].u8_DQS, sg_DDRTWindow[0].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]);

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W1, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[1].u8_Cnt,
		sg_DDRTWindow[1].aParam[0].u8_DQS, sg_DDRTWindow[1].aParam[0].u8_Cell,
		sg_DDRTWindow[1].aParam[1].u8_DQS, sg_DDRTWindow[1].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]);

	if(sg_DDRTWindow[0].u8_Cnt || sg_DDRTWindow[1].u8_Cnt)
		return eMMC_ST_SUCCESS;
	else
		return eMMC_ST_ERR_NO_OK_DDR_PARAM;

}


U32 eMMC_FCIE_BuildDDRTimingTable(void)
{
	U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  u8_i, u8_ClkIdx, u8_SetIdx, u8_tmp, u8_DqsIdx = 0, u8_CellBase;
	U32 u32_err, u32_ret=eMMC_ST_ERR_NO_OK_DDR_PARAM;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow;

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: building DDR table, please wait... \n");
	memset((void*)&g_eMMCDrv.TimingTable_t, '\0', sizeof(g_eMMCDrv.TimingTable_t));
	u8_SetIdx = 0;

	if(eMMC_IF_NORMAL_SDR()){
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: SDR mode can't detect DDR timing\n");
		u32_err = eMMC_FCIE_EnableFastMode_Ex(FCIE_eMMC_DDR);
		if(eMMC_ST_SUCCESS != u32_err)
		{
		    printf("eMMC Err: set DDR IF fail: %X\n", u32_err);
		    return u32_err;
		}
	}
	for(u8_ClkIdx=0; u8_ClkIdx<eMMC_FCIE_VALID_CLK_CNT; u8_ClkIdx++)
	{

		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");
		eMMC_clock_setting(gau8_FCIEClkSel[u8_ClkIdx]);
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");

		// ---------------------------
		// search and set the Windows
		u32_err = eMMC_FCIE_DetectDDRTiming();

		// ---------------------------
		// set the Table
		if(eMMC_ST_SUCCESS == u32_err)
		{
			#if 0
			// before 12MHz, should at least 2 clk is ok for DDR
			if(eMMC_FCIE_DDRT_SET_CNT-1 == u8_SetIdx &&
				BIT_FCIE_CLK_12M == gau8_FCIEClkSel[u8_ClkIdx])
				u32_ret = eMMC_ST_SUCCESS;
			#endif

			g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk = gau8_FCIEClkSel[u8_ClkIdx];

			// ---------------------------
		 	// select Window
			pWindow = NULL;

			// pick up the Window of Cell=0 case
			if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS == sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[0];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS == sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[1];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

			// ---------------------------
			if(NULL != pWindow)
			{

				// pick up the DQS having max valid cell
				u8_tmp = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
				{
					if(u8_tmp <= pWindow->au8_DQSValidCellCnt[u8_i]){
						u8_tmp = pWindow->au8_DQSValidCellCnt[u8_i];
						u8_DqsIdx = u8_i;
					}
				}
				if(0 != u8_DqsIdx) // do not use 0T, this should be always TRUE
				{
					if(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] >= 7 &&
						pWindow->au8_DQSValidCellCnt[u8_DqsIdx] >= 7)
					{
						g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[u8_DqsIdx];
						g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell = 0; // nice
					}
					else
					{
						u8_tmp = (pWindow->au8_DQSValidCellCnt[u8_DqsIdx] +
							pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1]) / 2;


						if(u8_tmp < pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1])
						{
							g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx-1];

							u8_CellBase = pWindow->au8_DQSTryCellCnt[u8_DqsIdx-1] - pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1];
							g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell =
								u8_CellBase + pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx] - u8_tmp;
						}
						else
						{   g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx];
						    g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell =
								(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx]) / 2;
						}
					}
				}
			}

			// ---------------------------
            // or, pick up the Window of large PASS Cnt
			else //if(NULL == pWindow)
			{
			    pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

				g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[pWindow->aParam[0].u8_DQS];
				g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].Param.u8_Cell =
					(pWindow->aParam[0].u8_Cell + pWindow->aParam[1].u8_Cell)/2;
			}

			#if 0
			// ---------------------------
			// use 12M for Set.Min
			if(eMMC_FCIE_DDRT_SET_CNT-2 == u8_SetIdx)
				u8_ClkIdx = eMMC_FCIE_VALID_CLK_CNT-2;
			#else
			if(FCIE_SLOW_CLK == g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.TimingTable_t.u8_SetCnt = u8_SetIdx + 1;
			#endif

			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT > eMMC_FCIE_DDRT_SET_CNT ?
	    eMMC_FCIE_DDRT_SET_CNT : eMMC_FCIE_VALID_CLK_CNT;
	u8_tmp = u8_tmp > g_eMMCDrv.TimingTable_t.u8_SetCnt ?
		g_eMMCDrv.TimingTable_t.u8_SetCnt : u8_tmp;

	eMMC_DumpTimingTable();

	// ======================================================
	// CAUTION: expect 48MHz can have valid DDRT parameter
	if(g_eMMCDrv.TimingTable_t.Set[eMMC_TIMING_SET_MAX].u8_Clk == gau8_FCIEClkSel[0]
		&& 0 != g_eMMCDrv.TimingTable_t.u8_SetCnt)
		u32_ret = eMMC_ST_SUCCESS;
	else if(g_eMMCDrv.TimingTable_t.Set[eMMC_TIMING_SET_MAX].u8_Clk != gau8_FCIEClkSel[0])
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: max clk can NOT run DDR\n");
	else if(0 == g_eMMCDrv.TimingTable_t.u8_SetCnt)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: min clk can NOT run DDR\n");
	// ======================================================

	// ---------------------------
	// save DDRT Table
	if(eMMC_ST_SUCCESS == u32_ret)
	{

		g_eMMCDrv.TimingTable_t.u32_ChkSum =
		    eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-4);
		memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t));

		eMMC_FCIE_ApplyTimingSet(eMMC_TIMING_SET_MAX);

		u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
	    u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
	    if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	    {
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: %Xh %Xh\n",
			    u32_err, u32_ret);
		    return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	    }
	}
	else
	{
		gau8_eMMC_SectorBuf[0] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>>24);
		gau8_eMMC_SectorBuf[1] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>>16);
		gau8_eMMC_SectorBuf[2] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>> 8);
		gau8_eMMC_SectorBuf[3] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t))>> 0);

		u32_ret = eMMC_ST_ERR_BUILD_DDRT;
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: fail! please call CAE or RD for helps, %Xh \n",
			u32_ret);
		eMMC_ResetReadyFlag();
		eMMC_Init();
		eMMC_FCIE_EnableSDRMode();

	}

	return u32_ret;
}

#endif // IF_DETECT_eMMC_DDR_TIMING


//---------------------------------------
#if defined(ENABLE_eMMC_INTERRUPT_MODE)&&ENABLE_eMMC_INTERRUPT_MODE

static DECLARE_WAIT_QUEUE_HEAD(fcie_wait);
static volatile U32 fcie_int = 0;

#define eMMC_IRQ_DEBUG    1

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
	volatile u16 u16_Events;

	if((REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE) != BIT_EMMC_ACTIVE)
	{
		return IRQ_NONE;
	}

	// one time enable one bit
	u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

	if(u16_Events & BIT_MIU_LAST_DONE)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CARD_DMA_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
	else if(u16_Events & BIT_SD_CMD_END)
	{
		REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_CMD_END);

		fcie_int = 1;
		wake_up(&fcie_wait);
		return IRQ_HANDLED;
	}

#if eMMC_IRQ_DEBUG
	if(0==fcie_int)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: Int St:%Xh, En:%Xh, Evt:%Xh \n",
		    REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN), u16_Events);
#endif

	return IRQ_NONE;

}


U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
{
	U32 u32_i=0;

    #if eMMC_IRQ_DEBUG
	U32 u32_isr_tmp[2];
	unsigned long long u64_jiffies_tmp, u64_jiffies_now;
	struct timeval time_st;
	time_t sec_tmp;
	suseconds_t us_tmp;

	u32_isr_tmp[0] = fcie_int;
	do_gettimeofday(&time_st);
	sec_tmp = time_st.tv_sec;
	us_tmp = time_st.tv_usec;
	u64_jiffies_tmp = jiffies_64;
    #endif

	//----------------------------------------
	if(wait_event_timeout(fcie_wait, (fcie_int == 1), usecs_to_jiffies(u32_MicroSec)) == 0)
    {
	    #if eMMC_IRQ_DEBUG
		u32_isr_tmp[1] = fcie_int;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh, ISR:%u->%u->%u \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN),
			u32_isr_tmp[0], u32_isr_tmp[1], fcie_int);

		do_gettimeofday(&time_st);
		u64_jiffies_now = jiffies_64;
	    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
		    " PassTime: %lu s, %lu us, %llu jiffies.  WaitTime: %u us, %lu jiffies, HZ:%u.\n",
		    time_st.tv_sec-sec_tmp, time_st.tv_usec-us_tmp, u64_jiffies_now-u64_jiffies_tmp,
		    u32_MicroSec, usecs_to_jiffies(u32_MicroSec), HZ);
	    #else
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN));
	    #endif

		// switch to polling
        for(u32_i=0; u32_i<u32_MicroSec; u32_i++)
        {
            if((REG_FCIE(u32_RegAddr) & u16_WaitEvent) == u16_WaitEvent )
                break;

            eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        }

		if(u32_i == u32_MicroSec)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: events lose, WaitEvent: %Xh \n", u16_WaitEvent);
			eMMC_DumpDriverStatus();  eMMC_DumpPadClk();
		    eMMC_FCIE_DumpRegisters();eMMC_FCIE_DumpDebugBus();
			return eMMC_ST_ERR_INT_TO;
		}
		else
		{	REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
		}

    }

	//----------------------------------------
	if(u16_WaitEvent & BIT_MIU_LAST_DONE)
	{
		for(u32_i=0; u32_i<TIME_WAIT_1_BLK_END; u32_i++)
		{
			if(REG_FCIE(u32_RegAddr) & BIT_CARD_DMA_END)
				break; // should be very fase
			eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
		}

		if(TIME_WAIT_1_BLK_END == u32_i)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no CARD_DMA_END\n");
			eMMC_FCIE_ErrHandler_Stop();
		}
	}

	//----------------------------------------
	fcie_int = 0;
	return eMMC_ST_SUCCESS;

}


#endif

int mstar_mci_Housekeep(void *pData)
{
	#if !(defined(eMMC_HOUSEKEEP_THREAD) && eMMC_HOUSEKEEP_THREAD)
    return 0;
	#endif

	#if eMMC_PROFILE_WR
	g_eMMCDrv.u32_CNT_MinRBlk = 0xFFFFFFFF;
	g_eMMCDrv.u32_CNT_MinWBlk = 0xFFFFFFFF;
	#endif

	while(1)
	{
		#if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
		msleep(1000);
		eMMC_dump_WR_Count();
		#endif

		if(kthread_should_stop())
			break;
	}

	return 0;
}


int mstar_mci_exit_checkdone_Ex(void)
{
    U16 u16Reg = 0;

    u16Reg = REG_FCIE(FCIE_SD_STATUS);

    if(u16Reg & BIT_SD_D0)
        return 1;
    else
        return 0;
}

bool mstar_mci_exit_checkdone_ForCI(void)
{
	#if 0
	U32 u32_Cnt=0;

    // eiffel eMMC_mode_1 share pins
	if(BIT_eMMC_CFG_MODE1 != (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
		return true;

	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			eMMC_debug(0,1,"eMMC Info: PCM check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}
	#endif
	return true;
}

EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForCI);


bool mstar_mci_exit_checkdone_ForSD(void)
{
	U32 u32_Cnt=0;

    // eiffel SD_mode_x does not share pins
	//if(BIT_eMMC_CFG_MODE2 == (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
	//	return TRUE;
    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
			eMMC_debug(0,1,"eMMC Info: SD check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	return true;
}
EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForSD);

U32 mstar_SD_CardChange(void)
{
	return 0;
}

//---------------------------------------
extern struct semaphore	PfModeSem;
extern bool ms_sdmmc_wait_d0_for_emmc(void);

void eMMC_LockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	down(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	if(false == ms_sdmmc_wait_d0_for_emmc())
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SD keep D0 low \n");
		eMMC_FCIE_ErrHandler_Stop();
	}

	eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
	eMMC_pads_switch(g_eMMCDrv.u8_PadType);
	#endif

    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	REG_FCIE_SETBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);
}

void eMMC_UnlockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	REG_FCIE_CLRBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);
	up(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);
}

//---------------------------------------

U32 eMMC_PlatformResetPre(void)
{
	/**((volatile unsigned short *)(0x25007DCC))|=0x02;	// emi mask
	*((volatile unsigned short *)(0x25007C18))|=0x02;	// imi0 mask
	*((volatile unsigned short *)(0x25007C58))|=0x02;	// imi1 mask
	*/
	//pull-high CMD pin
	REG_FCIE_SETBIT(PAD_CHIPTOP_BASE + (0x29 << 2), BIT13);
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	/**((volatile unsigned short *)(0x25007DCC))&=(~0x02);	// emi unmask
	*((volatile unsigned short *)(0x25007C18))&=(~0x02);	// imi0 unmask
	*((volatile unsigned short *)(0x25007C58))&=(~0x02);	// imi1 unmask
	*/
	return eMMC_ST_SUCCESS;
}


U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_DEFAULT_PAD);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformDeinit(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_StartSector += BL_BLK_OFFSET;
		break;

	case eMMC_PART_OTP:
		u32_StartSector += OTP_BLK_OFFSET;
		break;

	case eMMC_PART_SECINFO:
		u32_StartSector += SecInfo_BLK_OFFSET;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"SecAddr: %Xh, SecCnt: %Xh\n", u32_StartSector, u32_SectorCnt);

	if(eMMC_BOOT_PART_W == u8_OP)
		return eMMC_WriteBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);
	else
		return eMMC_ReadBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);

}


U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
	U32 u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end;

	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_eMMCBlkAddr_start = 0;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+BL_BLK_CNT-1;
		break;

	case eMMC_PART_OTP:
		u32_eMMCBlkAddr_start = OTP_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+OTP_BLK_CNT-1;
		break;

	case eMMC_PART_SECINFO:
		u32_eMMCBlkAddr_start = SecInfo_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+SecInfo_BLK_CNT-1;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"BlkAddr_start: %Xh, BlkAddr_end: %Xh\n",
		u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);

    return eMMC_EraseBootPart(u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end, 1);

}


char *gpas8_eMMCPartName[]={
	"e2pbak", "nvrambak", "hwcfgs", "recovery", "os",
	"fdd", "tdd", "blogo", "apanic","misc", "cus",
	"e2p0","e2p1","nvram0","nvram1", "system","cache", "data", "internal sd"};

// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

U32 eMMC_CheckIfMemCorrupt(void)
{
	if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
		return eMMC_ST_ERR_MEM_CORRUPT;

	return eMMC_ST_SUCCESS;
}

#elif (defined(eMMC_DRV_NAPOLI_LINUX)&&eMMC_DRV_NAPOLI_LINUX) // [FIXME] clone for your flatform

// check some fix value, print only when setting wrong
void eMMC_DumpPadClk(void)
{
	U16 u16_i, u16_reg;

	//---------------------------------------------------------------------
	eMMC_debug(eMMC_DEBUG_LEVEL, 1, "reg_ckg_fcie(0x%X):0x%x", reg_ckg_fcie ,REG_FCIE_U16(reg_ckg_fcie));
	eMMC_debug(eMMC_DEBUG_LEVEL, 0,"  FCIE Clk: %uKHz\n", g_eMMCDrv.u32_ClkKHz);
	eMMC_debug(eMMC_DEBUG_LEVEL, 0,"  Reg Val: %Xh\n", g_eMMCDrv.u16_ClkRegVal);

	eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\n\nemmcpll:");

	for(u16_i=0 ; u16_i<0x60; u16_i++)
	{
		if(0 == u16_i%8)
			eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n%02Xh:| ", u16_i);

		REG_FCIE_R(GET_REG_ADDR(EMMC_PLL_BASE, u16_i), u16_reg);
		eMMC_debug(eMMC_DEBUG_LEVEL, 0, "%04Xh ", u16_reg);
	}

	//---------------------------------------------------------------------
	eMMC_debug(0, 0, "\n\n[pad setting]: ");
	switch(g_eMMCDrv.u8_PadType)
	{
		case FCIE_eMMC_BYPASS:	eMMC_debug(0, 0, "BYPASS\n");	break;
		case FCIE_eMMC_SDR: eMMC_debug(0, 0, "SDR\n");	break;
		case FCIE_eMMC_DDR: eMMC_debug(0, 0, "DDR\n");	break;
		case FCIE_eMMC_HS200:	eMMC_debug(0, 0, "HS200\n");	break;
		default:
			eMMC_debug(0, 0, "eMMC Err: Pad unknown, %d\n", g_eMMCDrv.u8_PadType); eMMC_die("\n");
			break;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\nchiptop:");

	for(u16_i=0 ; u16_i<0x80; u16_i++)
	{
		if(0 == u16_i%8)
			eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n%02Xh:| ", u16_i);

		REG_FCIE_R(GET_REG_ADDR(PAD_CHIPTOP_BASE, u16_i), u16_reg);
		eMMC_debug(eMMC_DEBUG_LEVEL, 0, "%04Xh ", u16_reg);
	}

	eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\n");
}


// set pad first, then config clock
U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	REG_FCIE_CLRBIT(reg_test_mode, reg_test_out_mode_mask|reg_test_in_mode_mask);
    REG_FCIE_CLRBIT(reg_chiptop_0x64, BIT_CAADCONFIG|BIT_PCMADCONFIG|BIT_PCM2CTRLCONFIG);
	REG_FCIE_SETBIT(reg_sd_use_bypass, BIT_SD_USE_BYPASS);
	REG_FCIE_CLRBIT(reg_nand_config, BIT_NAND_MODE|BIT_NAND_CS1_EN);
	REG_FCIE_CLRBIT(reg_sd_config, BIT_SD_CONFIG);
    REG_FCIE_CLRBIT(reg_sdio_config, BIT_SDIO_CONFIG);
    REG_FCIE_CLRBIT(reg_emmc_config, BIT_EMMC_CONFIG_MASK);
	REG_FCIE_SETBIT(reg_emmc_config, BIT_EMMC_CONFIG_MODE1);
	REG_FCIE_CLRBIT(reg_chiptop_0x50,BIT_ALL_PAD_IN);

	REG_FCIE_CLRBIT(reg_atop_patch, BIT_ATOP_PATCH_MASK);
	REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_CLK_PH_MASK|BIT_DQ_PH_MASK|BIT_CMD_PH_MASK|BIT_SKEW4_MASK);

	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT10|BIT11|BIT12|BIT14|BIT15);
	REG_FCIE_CLRBIT(FCIE_SM_STS, BIT_DQS_DELAY_CELL_MASK);
	REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_CRC_STATUS_4_HS200|BIT_LATE_DATA0_W_IP_CLK);
	REG_FCIE_CLRBIT(FCIE_HS200_PATCH, BIT_HS200_PATCH_MASK);

	switch (u32_FCIE_IF_Type) {

	case FCIE_eMMC_BYPASS:
		//eMMC_debug(1, 0, "eMMC pads: BYPASS\n");
		eMMC_debug(1, 0, "eMMC Warn: Why are you using bypass mode, Daniel does not allow this!!!\n");
		REG_FCIE_SETBIT(reg_fcie2macro_bypass, BIT_FCIE2MACRO_BYPASS);
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT10|BIT11);
		g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
		break;

	case FCIE_eMMC_SDR:
		//eMMC_debug(1, 0, "eMMC pads: SDR\n");
		REG_FCIE_SETBIT(reg_fcie2macro_bypass, BIT_FCIE2MACRO_BYPASS);
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8);
		g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
		break;

	case FCIE_eMMC_DDR:
		//eMMC_debug(1, 0, "eMMC pads: DDR\n");
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT14);
		REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_CRC_STATUS_4_HS200|BIT_LATE_DATA0_W_IP_CLK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, BIT2);
		REG_FCIE_CLRBIT(reg_fcie2macro_bypass, BIT_FCIE2MACRO_BYPASS); // move to last to prevent glitch
		g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
		REG_FCIE_SETBIT(reg_atop_patch, BIT_HS_RSP_META_PATCH_HW|BIT_HS_D0_META_PATCH_HW|BIT_HS_DIN0_PATCH|BIT_HS_RSP_MASK_PATCH|BIT_DDR_RSP_PATCH);
		REG_FCIE_SETBIT(FCIE_HS200_PATCH, BIT_HS200_NORSP_PATCH|BIT_HS200_WCRC_PATCH|BIT_sbit_lose_patch);
		if(g_eMMCDrv.TimingTable_t.u8_SetCnt)
		    eMMC_FCIE_ApplyTimingSet(eMMC_TIMING_SET_MAX);
		break;

	case FCIE_eMMC_HS200:
		//eMMC_debug(1, 0, "eMMC pads: SDR200\n");
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT14|BIT15);
		REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_CRC_STATUS_4_HS200|BIT_LATE_DATA0_W_IP_CLK);
		REG_FCIE_CLRBIT(reg_fcie2macro_bypass, BIT_FCIE2MACRO_BYPASS); // move to last to prevent glitch
		g_eMMCDrv.u8_PadType = FCIE_eMMC_HS200;
		REG_FCIE_SETBIT(reg_atop_patch, BIT_HS200_PATCH|BIT_HS_RSP_META_PATCH_HW|BIT_HS_D0_META_PATCH_HW|BIT_HS_DIN0_PATCH|BIT_HS_EMMC_DQS_PATCH|BIT_HS_RSP_MASK_PATCH);
		REG_FCIE_SETBIT(FCIE_HS200_PATCH, BIT_HS200_PATCH_MASK);
		if(g_eMMCDrv.TimingTable_t.u8_SetCnt)
		    eMMC_FCIE_ApplyTimingSet(eMMC_TIMING_SET_MAX);
		break;

	default:
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: unknown interface: %X\n", u32_FCIE_IF_Type);
		eMMC_die(" ");
		return eMMC_ST_ERR_INVALID_PARAM;
		break;
	}

	return eMMC_ST_SUCCESS;

}


U32 eMMC_pll_setting(U16 u16_ClkParam)
{
	static U16 u16_OldClkParam=0xFFFF;
	 U32 u32_value_reg_emmc_pll_pdiv;

	 if(u16_ClkParam == u16_OldClkParam)
		 return eMMC_ST_SUCCESS;
	 else
		 u16_OldClkParam = u16_ClkParam;

	 // 1. reset emmc pll
	REG_FCIE_SETBIT(reg_emmc_pll_reset,BIT0);
	REG_FCIE_CLRBIT(reg_emmc_pll_reset,BIT0);

	// 2. synth clock
	switch(u16_ClkParam) {
	    case eMMC_PLL_CLK_200M: // 200M
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			#if 0
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x22); // 20xMHz
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x8F5C);
			#else
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x24); // 195MHz
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x03D8);
			#endif
			u32_value_reg_emmc_pll_pdiv =1;// PostDIV: 2
			break;
		case eMMC_PLL_CLK_160M:
			REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x2B);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x3333);
			u32_value_reg_emmc_pll_pdiv =1;// PostDIV: 2
			break;
		case eMMC_PLL_CLK_140M:
			REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x31);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x5F15);
			u32_value_reg_emmc_pll_pdiv = 1;// PostDIV: 2
			break;
		case eMMC_PLL_CLK_120M:
			REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x39);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x9999);
			u32_value_reg_emmc_pll_pdiv = 1;// PostDIV: 2
			break;
		case eMMC_PLL_CLK_100M:
			REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x45);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x1EB8);
			u32_value_reg_emmc_pll_pdiv = 1;// PostDIV: 2
			break;
		case eMMC_PLL_CLK__86M:
			REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x28);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x2FA0);
			u32_value_reg_emmc_pll_pdiv = 2;// PostDIV: 4
			break;
		case eMMC_PLL_CLK__80M:
			REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x2B);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x3333);
			u32_value_reg_emmc_pll_pdiv = 2;// PostDIV: 4
			break;
		case eMMC_PLL_CLK__72M:
			REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x30);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x0000);
			u32_value_reg_emmc_pll_pdiv = 2;// PostDIV: 4
			break;
		case eMMC_PLL_CLK__62M:
			REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x37);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0xBDEF);
			u32_value_reg_emmc_pll_pdiv = 2;// PostDIV: 4
			break;
		case eMMC_PLL_CLK__52M:
			REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x42);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x7627);
			u32_value_reg_emmc_pll_pdiv = 2;// PostDIV: 4
			break;
		case eMMC_PLL_CLK__48M:
			REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x48);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x0000);
			u32_value_reg_emmc_pll_pdiv = 2;// PostDIV: 4
			break;
		case eMMC_PLL_CLK__40M:
			REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x2B);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x3333);
			u32_value_reg_emmc_pll_pdiv = 4;// PostDIV: 8
			break;
		case eMMC_PLL_CLK__36M:
			REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x30);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x0000);
			u32_value_reg_emmc_pll_pdiv = 4;// PostDIV: 8
			break;
		case eMMC_PLL_CLK__32M:
			REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x36);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x0000);
			u32_value_reg_emmc_pll_pdiv = 4;// PostDIV: 8
			break;
		case eMMC_PLL_CLK__27M:
			REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x40);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x0000);
			u32_value_reg_emmc_pll_pdiv = 4;// PostDIV: 8
			break;
		case eMMC_PLL_CLK__20M:
			REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x2B);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x3333);
			u32_value_reg_emmc_pll_pdiv = 7;// PostDIV: 16
			break;
		default:
			eMMC_debug(0,0,"eMMC Err: emmc PLL not configed %Xh\n", u16_ClkParam);
			eMMC_die(" ");
			return eMMC_ST_ERR_UNKNOWN_CLK;
			break;
	}

	// 3. VCO clock ( loop N = 4 )
	REG_FCIE_CLRBIT(reg_emmcpll_fbdiv,0xffff);
	REG_FCIE_SETBIT(reg_emmcpll_fbdiv,0x6);// PostDIV: 8

	// 4. 1X clock
	REG_FCIE_CLRBIT(reg_emmcpll_pdiv,BIT2|BIT1|BIT0);
	REG_FCIE_SETBIT(reg_emmcpll_pdiv,u32_value_reg_emmc_pll_pdiv);// PostDIV: 8

	if(u16_ClkParam==eMMC_PLL_CLK__20M) {
	    REG_FCIE_SETBIT(reg_emmc_pll_test, BIT10);
	}
	else {
		REG_FCIE_CLRBIT(reg_emmc_pll_test, BIT10);
	}

	return eMMC_ST_SUCCESS;
}


// Notice!!! you need to set pad before config clock
U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam) {
		case eMMC_PLL_CLK__20M	: g_eMMCDrv.u32_ClkKHz =  20000; break;
		case eMMC_PLL_CLK__27M	: g_eMMCDrv.u32_ClkKHz =  27000; break;
		case eMMC_PLL_CLK__32M	: g_eMMCDrv.u32_ClkKHz =  32000; break;
		case eMMC_PLL_CLK__36M	: g_eMMCDrv.u32_ClkKHz =  36000; break;
		case eMMC_PLL_CLK__40M	: g_eMMCDrv.u32_ClkKHz =  40000; break;
		case eMMC_PLL_CLK__48M	: g_eMMCDrv.u32_ClkKHz =  48000; break;
		case eMMC_PLL_CLK__52M	: g_eMMCDrv.u32_ClkKHz =  52000; break;
		case eMMC_PLL_CLK__62M	: g_eMMCDrv.u32_ClkKHz =  62000; break;
		case eMMC_PLL_CLK__72M	: g_eMMCDrv.u32_ClkKHz =  72000; break;
		case eMMC_PLL_CLK__80M	: g_eMMCDrv.u32_ClkKHz =  80000; break;
		case eMMC_PLL_CLK__86M	: g_eMMCDrv.u32_ClkKHz =  86000; break;
		case eMMC_PLL_CLK_100M	: g_eMMCDrv.u32_ClkKHz = 100000; break;
		case eMMC_PLL_CLK_120M	: g_eMMCDrv.u32_ClkKHz = 120000; break;
		case eMMC_PLL_CLK_140M	: g_eMMCDrv.u32_ClkKHz = 140000; break;
		case eMMC_PLL_CLK_160M	: g_eMMCDrv.u32_ClkKHz = 160000; break;
		case eMMC_PLL_CLK_200M	: g_eMMCDrv.u32_ClkKHz = 200000; break;

		case BIT_FCIE_CLK_20M	: g_eMMCDrv.u32_ClkKHz =  20000; break;
		case BIT_FCIE_CLK_27M	: g_eMMCDrv.u32_ClkKHz =  27000; break;
		case BIT_FCIE_CLK_32M	: g_eMMCDrv.u32_ClkKHz =  32000; break;
		case BIT_FCIE_CLK_36M	: g_eMMCDrv.u32_ClkKHz =  36000; break;
		case BIT_FCIE_CLK_40M	: g_eMMCDrv.u32_ClkKHz =  40000; break;
		case BIT_FCIE_CLK_43_2M	: g_eMMCDrv.u32_ClkKHz =  43200; break;
		case BIT_FCIE_CLK_300K	: g_eMMCDrv.u32_ClkKHz =    300; break;
		case BIT_FCIE_CLK_48M	: g_eMMCDrv.u32_ClkKHz =  48000; break;

		default:
			eMMC_debug(1, 1, "eMMC Err: clkgen %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
			eMMC_die(" ");
			return eMMC_ST_ERR_INVALID_PARAM; break;
	}

	if(u16_ClkParam & eMMC_PLL_FLAG) {
		//eMMC_debug(0,0,"eMMC PLL: %Xh\n", u16_ClkParam);
		REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_FCIE_CLK_GATING|BIT_FCIE_CLK_INVERSE|BIT_CLKGEN_FCIE_MASK);
		REG_FCIE_SETBIT(reg_ckg_fcie, BIT_FCIE_CLK_SRC_SEL|BIT_FCIE_CLK_EMMC_PLL<<2);
		eMMC_pll_setting(u16_ClkParam);
	}
	else {
		//eMMC_debug(0,0,"eMMC CLKGEN: %Xh\n", u16_ClkParam);
	    REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_FCIE_CLK_GATING|BIT_FCIE_CLK_INVERSE|BIT_CLKGEN_FCIE_MASK);
        REG_FCIE_SETBIT(reg_ckg_fcie, BIT_FCIE_CLK_SRC_SEL|u16_ClkParam<<2);
	}

	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_gating(void)
{
	//eMMC_debug(0,1,"\n");
	eMMC_PlatformResetPre();
	g_eMMCDrv.u32_ClkKHz = 0;
	REG_FCIE_SETBIT(reg_ckg_fcie, BIT_FCIE_CLK_GATING);// gate clock
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}

U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT] = {
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_36M,
	BIT_FCIE_CLK_20M
};

//---------------------------------------
#if defined(ENABLE_eMMC_INTERRUPT_MODE)&&ENABLE_eMMC_INTERRUPT_MODE

static DECLARE_WAIT_QUEUE_HEAD(fcie_wait);
static volatile U32 fcie_int = 0;

#define eMMC_IRQ_DEBUG    1

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
	volatile u16 u16_Events;

	if((REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE) != BIT_EMMC_ACTIVE)
	{
		return IRQ_NONE;
	}

	// one time enable one bit
	u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

	if(u16_Events & BIT_MIU_LAST_DONE)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CARD_DMA_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
	else if(u16_Events & BIT_SD_CMD_END)
	{
		REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_CMD_END);

		fcie_int = 1;
		wake_up(&fcie_wait);
		return IRQ_HANDLED;
	}

#if eMMC_IRQ_DEBUG
	if(0==fcie_int)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: Int St:%Xh, En:%Xh, Evt:%Xh \n",
		    REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN), u16_Events);
#endif

	return IRQ_NONE;

}


U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
{
	U32 u32_i=0;

    #if eMMC_IRQ_DEBUG
	U32 u32_isr_tmp[2];
	unsigned long long u64_jiffies_tmp, u64_jiffies_now;
	struct timeval time_st;
	time_t sec_tmp;
	suseconds_t us_tmp;

	u32_isr_tmp[0] = fcie_int;
	do_gettimeofday(&time_st);
	sec_tmp = time_st.tv_sec;
	us_tmp = time_st.tv_usec;
	u64_jiffies_tmp = jiffies_64;
    #endif

	//----------------------------------------
	if(wait_event_timeout(fcie_wait, (fcie_int == 1), usecs_to_jiffies(u32_MicroSec)) == 0)
    {
		#if eMMC_IRQ_DEBUG
		u32_isr_tmp[1] = fcie_int;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh, ISR:%u->%u->%u \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN),
			u32_isr_tmp[0], u32_isr_tmp[1], fcie_int);

		do_gettimeofday(&time_st);
		u64_jiffies_now = jiffies_64;
	    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
		    " PassTime: %lu s, %lu us, %llu jiffies.  WaitTime: %u us, %lu jiffies, HZ:%u.\n",
		    time_st.tv_sec-sec_tmp, time_st.tv_usec-us_tmp, u64_jiffies_now-u64_jiffies_tmp,
		    u32_MicroSec, usecs_to_jiffies(u32_MicroSec), HZ);
	    #else
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh \n",
			u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN));
	    #endif

		// switch to polling
        for(u32_i=0; u32_i<u32_MicroSec; u32_i++)
        {
            if((REG_FCIE(u32_RegAddr) & u16_WaitEvent) == u16_WaitEvent )
                break;

            eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        }

		if(u32_i == u32_MicroSec)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: events lose, WaitEvent: %Xh \n", u16_WaitEvent);
			eMMC_DumpDriverStatus();  eMMC_DumpPadClk();
		    eMMC_FCIE_DumpRegisters();eMMC_FCIE_DumpDebugBus();
			return eMMC_ST_ERR_INT_TO;
		}
		else
		{	REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
		}

    }

	//----------------------------------------
	if(u16_WaitEvent & BIT_MIU_LAST_DONE)
	{
		for(u32_i=0; u32_i<TIME_WAIT_1_BLK_END; u32_i++)
		{
			if(REG_FCIE(u32_RegAddr) & BIT_CARD_DMA_END)
				break; // should be very fase
			eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
		}

		if(TIME_WAIT_1_BLK_END == u32_i)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no CARD_DMA_END\n");
			eMMC_FCIE_ErrHandler_Stop();
		}
	}

	//----------------------------------------
	fcie_int = 0;
	return eMMC_ST_SUCCESS;

}
#endif


int mstar_mci_Housekeep(void *pData)
{
	#if !(defined(eMMC_HOUSEKEEP_THREAD) && eMMC_HOUSEKEEP_THREAD)
    return 0;
	#endif

	#if eMMC_PROFILE_WR
	g_eMMCDrv.u32_CNT_MinRBlk = 0xFFFFFFFF;
	g_eMMCDrv.u32_CNT_MinWBlk = 0xFFFFFFFF;
	#endif

	while(1)
	{
		#if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
		msleep(1000);
		eMMC_dump_WR_Count();
		#endif

		if(kthread_should_stop())
			break;
	}

	return 0;
}


int mstar_mci_exit_checkdone_Ex(void)
{
    U16 u16Reg = 0;

    u16Reg = REG_FCIE(FCIE_SD_STATUS);

    if(u16Reg & BIT_SD_D0)
        return 1;
    else
        return 0;
}

bool mstar_mci_exit_checkdone_ForCI(void)
{
	#if 0// no shared pins with CI
	U32 u32_Cnt=0;

    // eiffel eMMC_mode_1 share pins
	if(BIT_eMMC_CFG_MODE1 != (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
		return true;

	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			eMMC_debug(0,1,"eMMC Info: PCM check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}
	#endif
	return true;
}

EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForCI);


bool mstar_mci_exit_checkdone_ForSD(void)
{
	#if 0 // no shared pins with SD
	U32 u32_Cnt=0;

    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
			eMMC_debug(0,1,"eMMC Info: SD check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	#endif
	return true;
}
EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForSD);


U32 mstar_SD_CardChange(void)
{
	return 0;
}

//---------------------------------------
extern struct semaphore	PfModeSem;
extern bool ms_sdmmc_wait_d0_for_emmc(void);

void eMMC_LockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	down(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	if(false == ms_sdmmc_wait_d0_for_emmc())
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SD keep D0 low \n");
		eMMC_FCIE_ErrHandler_Stop();
	}

	eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
	eMMC_pads_switch(g_eMMCDrv.u8_PadType);
	#endif

    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	REG_FCIE_SETBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);
	
}

void eMMC_UnlockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	REG_FCIE_CLRBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);
	up(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);
}

//---------------------------------------

U32 eMMC_PlatformResetPre(void)
{
	/**((volatile unsigned short *)(0x25007DCC))|=0x02;	// emi mask
	*((volatile unsigned short *)(0x25007C18))|=0x02;	// imi0 mask
	*((volatile unsigned short *)(0x25007C58))|=0x02;	// imi1 mask
	*/
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	/**((volatile unsigned short *)(0x25007DCC))&=(~0x02);	// emi unmask
	*((volatile unsigned short *)(0x25007C18))&=(~0x02);	// imi0 unmask
	*((volatile unsigned short *)(0x25007C58))&=(~0x02);	// imi1 unmask
	*/
    #if defined(ENABLE_EMMC_POWER_SAVING_MODE) && ENABLE_EMMC_POWER_SAVING_MODE
	eMMC_Prepare_Power_Saving_Mode_Queue();
    #endif
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
    #if 1	// for eMMC 4.5 HS200 need 1.8V, unify all eMMC IO power to 1.8V
	// works both for eMMC 4.4 & 4.5
	// printf("1.8V IO power for eMMC\n");
	// Irwin Tyan: set this bit to boost IO performance at low power supply.

	if((REG_FCIE(reg_emmc_test) & BIT0) != 1) {
		//eMMC_debug(0, 0, "eMMC: set 1.8V \n");
		REG_FCIE_SETBIT(reg_emmc_test, BIT0); // 1.8V must set this bit
		REG_FCIE_SETBIT(reg_emmc_test, BIT2); // atop patch
		REG_FCIE_CLRBIT(reg_nand_pad_driving, 0xffff);// 1.8V must set this bit
		REG_FCIE_SETBIT(reg_nand_pad_driving, 0xf);// 1.8V must set this bit
	}
    #else
	printf("3.3V IO power for eMMC\n");
	pRegEmmcPll->reg_emmc_test = 0; // 3.3V must clear this bit
    #endif
	
	#if defined(CONFIG_ENABLE_EMMC_ACP) && CONFIG_ENABLE_EMMC_ACP
	//set once only , ACP can't be disabled
	eMMC_debug(0, 0, "eMMC: enable ACP\n");
	
	REG_FCIE_CLRBIT(reg_1239_Rx16, BIT8|BIT9|BIT10);
	REG_FCIE_CLRBIT(reg_1239_Rx26, BIT8|BIT9|BIT10);

	//mask acp request
	REG_FCIE_SETBIT(reg_100E_Rx03, (BIT8-1));
	//start acp
	REG_FCIE_SETBIT(reg_100E_Rx04, BIT6);
	//unmask acp request
	REG_FCIE_CLRBIT(reg_100E_Rx03, (BIT8-1));
	#endif

	eMMC_pads_switch(FCIE_DEFAULT_PAD);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	return eMMC_ST_SUCCESS;
}


U32 eMMC_PlatformDeinit(void)
{
	return eMMC_ST_SUCCESS;
}


// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

U32 eMMC_CheckIfMemCorrupt(void)
{
	if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
		return eMMC_ST_ERR_MEM_CORRUPT;

	return eMMC_ST_SUCCESS;
}
// <-- [FIXME]


//=============================================================
#else
  #error "Error! no platform functions."
#endif
#endif
