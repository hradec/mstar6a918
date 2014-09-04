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
#if defined(UNIFIED_eMMC_DRIVER) && UNIFIED_eMMC_DRIVER

U32 gu32_eMMCDrvExtFlag = 0;

U32 eMMC_hw_timer_delay(U32 u32us)
{
	#if 0
   volatile int i = 0;

	for (i = 0; i < (u32us>>0); i++)
	{
		#if 1
		volatile int j = 0, tmp;
		for (j = 0; j < 0x2; j++)
		{
			tmp = j;
		}
		#endif
	}
	#else
	volatile U32 u32_i=u32us;
    //udelay((u32us>>1)+(u32us>>4)+1);
    while(u32_i>1000)
    {
		udelay(1000);
		u32_i-=1000;
    }
	udelay(u32_i);
	#endif
	return u32us;
}

U32 eMMC_hw_timer_sleep(U32 u32ms)
{
	U32 u32_us = u32ms*1000;

	while(u32_us > 1000)
	{
		eMMC_hw_timer_delay(1000); // uboot has no context switch
		u32_us -= 1000;
	}

	eMMC_hw_timer_delay(u32_us);
	return u32ms;
}

U32 eMMC_hw_timer_start(void)
{
	// Reset PIU Timer1
	REG_FCIE_W(TIMER1_MAX_LOW, 0xFFFF);
	REG_FCIE_W(TIMER1_MAX_HIGH, 0xFFFF);
	REG_FCIE_W(TIMER1_ENABLE, 0);

	// Start PIU Timer1
	REG_FCIE_SETBIT(TIMER1_ENABLE, 0x0001);
	return 0;
}

U32 eMMC_hw_timer_tick(void)
{
	U32 u32HWTimer = 0;
	volatile U16 u16TimerLow = 0;
	volatile U16 u16TimerHigh = 0;

	REG_FCIE_R(TIMER1_CAP_LOW, u16TimerLow);
	REG_FCIE_R(TIMER1_CAP_HIGH, u16TimerHigh);

	u32HWTimer = (u16TimerHigh<<16) | u16TimerLow;

	return u32HWTimer;
}

//------------------------------------
void eMMC_set_WatchDog(U8 u8_IfEnable)
{
	// do nothing
}

void eMMC_reset_WatchDog(void)
{
	// do nothing
}

//--------------------------------
U32 eMMC_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt)
{
	flush_cache(u32_DMAAddr, u32_ByteCnt);
	#ifdef CONFIG_MCU_ARM
	//eMMC_debug(0,0,"MIU0[%Xh], MIU1[%Xh]\n", CONFIG_MIU0_BUSADDR, CONFIG_MIU1_BUSADDR);

	if((MsOS_VA2PA(u32_DMAAddr)& CONFIG_SYS_MIU_INTERVAL)==CONFIG_SYS_MIU_INTERVAL)//MIU1
	{
	    REG_FCIE_SETBIT(FCIE_MIU_DMA_26_16,BIT_MIU1_SELECT);
         return (MsOS_VA2PA(u32_DMAAddr) - CONFIG_SYS_MIU_INTERVAL);
	}
    else//MIU0
    {
        REG_FCIE_CLRBIT(FCIE_MIU_DMA_26_16,BIT_MIU1_SELECT);
        return MsOS_VA2PA(u32_DMAAddr);
    }

	#endif
}

void eMMC_flush_data_cache_buffer(U32 u32_DMAAddr, U32 u32_ByteCnt)
{
    //flush_cache(u32_DMAAddr, u32_ByteCnt);
}

void eMMC_Invalidate_data_cache_buffer(U32 u32_DMAAddr, U32 u32_ByteCnt)
{
	//flush_cache(u32_DMAAddr, u32_ByteCnt);
}

void eMMC_flush_miu_pipe(void)
{

}

void eMMC_CheckPowerCut(void)
{
#if ((defined(eMMC_DRV_EIFFEL_UBOOT) && eMMC_DRV_EIFFEL_UBOOT) || \
     (defined(eMMC_DRV_NIKE_UBOOT) && eMMC_DRV_NIKE_UBOOT) || \
     (defined(eMMC_DRV_MADISON_UBOOT) && eMMC_DRV_MADISON_UBOOT) || \
	 (defined(eMMC_DRV_CLIPPERS_UBOOT) && eMMC_DRV_CLIPPERS_UBOOT) || \
     (defined(eMMC_DRV_MIAMI_UBOOT) && eMMC_DRV_MIAMI_UBOOT) || \
     (defined(eMMC_DRV_NADAL_UBOOT) && eMMC_DRV_NADAL_UBOOT))
	/* SAR5=ON in set_config will enable this feature */

	while(((REG_FCIE(reg_vplug_in_pwrgd) >> 14)&0x0001) == 0)
	{
		eMMC_debug(0,1,"detect power cut\n");
		//eMMC_hw_timer_delay(HW_TIMER_DELAY_500ms);
		while(1);
	}
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
				{   // to ignore "discontinous case"
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
	{   // --------------------------------
		// to ignore "discontinous case"
		u16_cnt = 0;
		for(u16_i=0; u16_i<PLL_SKEW4_CNT*2; u16_i++)
		{
			if(u32_Skew4Result&1<<u16_i)
				u16_cnt++;
			else
			{   u16_StartBit = u16_i-1;
				break;
			}
		}
		for(u16_i=PLL_SKEW4_CNT*2-1; u16_i>0; u16_i--)
		{
			if(u32_Skew4Result&1<<u16_i)
				u16_cnt++;
			else
			{   u16_EndBit = u16_i+1;
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
		{   u32_tmp = u32_Skew4Result;
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
	eMMC_die();
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

			if(!eMMC_CHK_BKG_SCAN_HS200()){
			u32_err = eMMC_CMD24(eMMC_HS200TABLE_BLK_0, gau8_eMMC_SectorBuf);
	        u32_ret = eMMC_CMD24(eMMC_HS200TABLE_BLK_1, gau8_eMMC_SectorBuf);
	        if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	        {
		        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: Save TTable fail: %Xh %Xh\n",
			        u32_err, u32_ret);
		        return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	        }}
			return eMMC_ST_SUCCESS;
		}		
	}	

	eMMC_DumpTimingTable();
	eMMC_die();
	return eMMC_ST_ERR_SKEW4;
}

#endif // IF_DETECT_eMMC_DDR_TIMING

//=============================================================
#if (defined(eMMC_DRV_A3_UBOOT)&&eMMC_DRV_A3_UBOOT)
void eMMC_DumpPadClk(void)
{

	eMMC_debug(0, 1, "\nclk setting: \n");
	eMMC_debug(0, 1, "reg_ckg_fcie(0x%X):0x%x\n", reg_ckg_fcie, REG_FCIE_U16(reg_ckg_fcie));

	eMMC_debug(0, 1, "\npad setting: \n");
	eMMC_debug(0, 1, "FCIE_BOOT_CONFIG(0x%X):0x%x\n", FCIE_BOOT_CONFIG, REG_FCIE_U16(FCIE_BOOT_CONFIG));
	eMMC_debug(0, 1, "reg_all_pad_in(0x%X):0x%x\n", reg_all_pad_in, REG_FCIE_U16(reg_all_pad_in));
	eMMC_debug(0, 1, "reg_sd_pad(0x%X):0x%x\n", reg_sd_pad, REG_FCIE_U16(reg_sd_pad));
	eMMC_debug(0, 1, "reg_emmc_pad(0x%X):0x%x\n", reg_emmc_pad, REG_FCIE_U16(reg_emmc_pad));
	eMMC_debug(0, 1, "reg_nand_pad(0x%X):0x%x\n", reg_nand_pad, REG_FCIE_U16(reg_nand_pad));
	eMMC_debug(0, 1, "reg_pcmcia_pad(0x%X):0x%x\n", reg_pcmcia_pad, REG_FCIE_U16(reg_pcmcia_pad));

	eMMC_debug(0, 1, "\n");
}

#if (MS_BOARD_TYPE_SEL == BD_MST029B_10AFVA_A3 || ENABLE_MSTAR_TITANIA_BD_MS801_MAA2XG_A3 || ENABLE_MSTAR_TITANIA_BD_MST029B_CUS37_A3 || ENABLE_MSTAR_TITANIA_BD_MST029B_CUS18_A3 || ENABLE_MSTAR_TITANIA_BD_MST029B_CUS18_A3_ATSC || ENABLE_MSTAR_TITANIA_BD_MSTCUS23_6A801_A3 || ENABLE_MSTAR_TITANIA_BD_MST029B_CUS36_A3 || ENABLE_MSTAR_TITANIA_BD_CUS28_6A801 || ENABLE_MSTAR_TITANIA_BD_MST029B_BOX_A3)
static U8 eMMC_Platform_DisableBootMode(void)
{
    U16 regval;

    REG_FCIE_SETBIT(FCIE_TEST_MODE, BIT8);
    udelay(100000);

    REG_FCIE_R(FCIE_DEBUG_BUS,regval);

    if( (regval != 0x1F) && (regval != 0x0F) )
    {
        if (regval == 0x0E)
        {
//           FCIE_RIU_16_ON(EMMC_BOOT_CONFIG, WBIT01);
			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG,BIT1);
           while(1)
           {
              udelay(1);
//              regval=FCIE_RIU_R16(MIE_EVENT);
			  REG_FCIE_R(FCIE_MIE_EVENT, regval);
              if (regval&BIT15)
                 break;
           }
//           FCIE_RIU_W16(MIE_PATH_CTL, 0);
			REG_FCIE_W(FCIE_PATH_CTRL, 0);
//           FCIE_RIU_W16(JOB_BL_CNT, 0);
			REG_FCIE_W(FCIE_JOB_BL_CNT, 0);
//           regval=FCIE_RIU_R16(SD_STS);
			REG_FCIE_R(FCIE_SD_STATUS, regval);
//           FCIE_RIU_W16(SD_STS, regval);
			REG_FCIE_W(FCIE_SD_STATUS, regval);
//           regval=FCIE_RIU_R16(MIE_EVENT);
			REG_FCIE_R(FCIE_MIE_EVENT, regval);
//           FCIE_RIU_W16(MIE_EVENT, regval);
			REG_FCIE_W(FCIE_MIE_EVENT, regval);
        }
//        FCIE_RIU_W16(CARD_INT_EN, regval);
		REG_FCIE_W(FCIE_CARD_INT_EN, regval);
        return 1;
    }

//    FCIE_RIU_W16(JOB_BL_CNT, 1);
	REG_FCIE_W(FCIE_JOB_BL_CNT,1);
//    FCIE_RIU_16_ON(SD_MODE, WBIT09);
	REG_FCIE_SETBIT(FCIE_SD_MODE, BIT9);
    udelay(100);
//    FCIE_RIU_W16(SD_CTL, WBIT03);
	REG_FCIE_W(FCIE_SD_CTRL, BIT3);

    while(1)
    {
       udelay(10);
//       regval=FCIE_RIU_R16(MIE_EVENT);
		REG_FCIE_R(FCIE_MIE_EVENT, regval);
       if (regval&BIT2)
           break;
    }

    udelay(1000);
//    FCIE_RIU_16_ON(EMMC_BOOT_CONFIG, WBIT01);
	REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT1);

    while(1)
    {
       udelay(10);
//       regval=FCIE_RIU_R16(MIE_EVENT);
		REG_FCIE_R(FCIE_MIE_EVENT, regval);
       if (regval&BIT15)
           break;
    }

//    FCIE_RIU_W16(MIE_PATH_CTL, 0);
	REG_FCIE_W(FCIE_PATH_CTRL, 0);
//    FCIE_RIU_W16(JOB_BL_CNT, 0);
	REG_FCIE_W(FCIE_JOB_BL_CNT, 0);
//    regval=FCIE_RIU_R16(SD_STS);
	REG_FCIE_R(FCIE_SD_STATUS, regval);
//    FCIE_RIU_W16(SD_STS, regval);
	REG_FCIE_W(FCIE_SD_STATUS, regval);
//    regval=FCIE_RIU_R16(MIE_EVENT);
	REG_FCIE_R(FCIE_MIE_EVENT, regval);
//    FCIE_RIU_W16(MIE_EVENT, regval);
	REG_FCIE_W(FCIE_MIE_EVENT, regval);

//    FCIE_RIU_16_OF(SD_MODE, WBIT09);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT9);
//    FCIE_RIU_W16(SD_CTL, 0);
	REG_FCIE_W(FCIE_SD_CTRL, 0);
    return 0;
}
#endif

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{

	static U32 u32_Pre_IF=0xFFFFFFFF;

	// disable macro
	//REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT_SD_DDR_EN|BIT_MACRO_EN);
	// set bypass mode
	//REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_SD_BYPASS_MODE_EN);

	// set emmc pad mode
	REG_FCIE_CLRBIT(reg_emmc_pad, CFG_EMMC_PAD_MASK);
	REG_FCIE_SETBIT(reg_emmc_pad, CFG_EMMC_PAD);

	// clr Pad Ctrl all_pad_in
	REG_FCIE_CLRBIT(reg_all_pad_in, BIT15);

	//clr nand mode
	REG_FCIE_CLRBIT(reg_nand_pad, CFG_NAND_PAD_MASK);

	//clr pcmcia pad
	REG_FCIE_CLRBIT(reg_pcmcia_pad, BIT4);


    #if (MS_BOARD_TYPE_SEL == BD_MST029B_10AFVA_A3 || ENABLE_MSTAR_TITANIA_BD_MS801_MAA2XG_A3 || ENABLE_MSTAR_TITANIA_BD_MST029B_CUS37_A3 || ENABLE_MSTAR_TITANIA_BD_MST029B_CUS18_A3 || ENABLE_MSTAR_TITANIA_BD_MSTCUS23_6A801_A3 || ENABLE_MSTAR_TITANIA_BD_MST029B_CUS36_A3 || ENABLE_MSTAR_TITANIA_BD_MST029B_CUS18_A3_ATSC|| ENABLE_MSTAR_TITANIA_BD_CUS28_6A801 || ENABLE_MSTAR_TITANIA_BD_MST029B_BOX_A3)
    //===================================================
    // PATCH A3 U02 goto boot mode issue
    //===================================================
//    FCIE_RIU_16_ON(EMMC_BOOT_CFG, FORCE_BOOT_FROM_EMMC2);
	REG_FCIE_SETBIT(CHIPTOP_EMMC_BOOT_CFG, FORCE_BOOT_FROM_EMMC2);
	eMMC_Platform_DisableBootMode();
    #endif

    if(u32_Pre_IF == u32_FCIE_IF_Type)
        return eMMC_ST_SUCCESS;

	switch(u32_FCIE_IF_Type){
        case FCIE_eMMC_DDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC pads: DDR\n");
            // disable macro
            REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,
                BIT_MACRO_EN|BIT_SD_DDR_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS|BIT_SD_FROM_TMUX);
            // set DDR mode
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_MACRO_EN|BIT_SD_DDR_EN);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
            break;

        case FCIE_eMMC_SDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC pads: SDR\n");
            // disable macro
            REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,
                BIT_MACRO_EN|BIT_SD_DDR_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS|BIT_SD_FROM_TMUX);
            // set SDR mode
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_MACRO_EN);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
            break;

        case FCIE_eMMC_BYPASS:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC pads: BYPASS\n");
            // disable macro
    		REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,
				BIT_MACRO_EN|BIT_SD_DDR_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS|BIT_SD_FROM_TMUX);
    		// set bypass mode
    		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_SD_BYPASS_MODE_EN);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
            break;

        default:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: unknown interface: %X\n",u32_FCIE_IF_Type);
            return eMMC_ST_ERR_INVALID_PARAM;
    }
	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam) {
    case BIT_FCIE_CLK_300K:		g_eMMCDrv.u32_ClkKHz = 300;		break;
    case BIT_FCIE_CLK_12M:		g_eMMCDrv.u32_ClkKHz = 12000;	break;
    case BIT_FCIE_CLK_20M:		g_eMMCDrv.u32_ClkKHz = 20000;	break;
    case BIT_FCIE_CLK_24M:		g_eMMCDrv.u32_ClkKHz = 24000;	break;
    case BIT_FCIE_CLK_27M:		g_eMMCDrv.u32_ClkKHz = 27000;	break;
    case BIT_FCIE_CLK_32M:		g_eMMCDrv.u32_ClkKHz = 32000;	break;
    case BIT_FCIE_CLK_40M:		g_eMMCDrv.u32_ClkKHz = 40000;	break;
    case BIT_FCIE_CLK_43_2M:	g_eMMCDrv.u32_ClkKHz = 43200;	break;
    case BIT_FCIE_CLK_48M:		g_eMMCDrv.u32_ClkKHz = 48000;	break;
    default:
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
        return eMMC_ST_ERR_INVALID_PARAM;
    }

	REG_FCIE_W(reg_ckg_fcie, 0);
	REG_FCIE_SETBIT(reg_ckg_fcie, BIT_FCIE_CLK_SRC_SEL);

	REG_FCIE_SETBIT(reg_ckg_fcie, (u16_ClkParam<<BIT_FCIE_CLK_SHIFT));

	REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_Inverse);

	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "reg_ckg_fcie(%Xh):%Xh, Clk:%Xh\n", reg_ckg_fcie, REG_FCIE_U16(reg_ckg_fcie), u16_ClkParam);
	//eMMC_debug(0, 1, "reg_gate_miu_fcie(0x%X):0x%x\n", reg_gate_miu_fcie, REG_FCIE_U16(reg_gate_miu_fcie));
	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
    eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}

U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
	BIT_FCIE_CLK_32M,
	BIT_FCIE_CLK_27M,
	BIT_FCIE_CLK_20M,
	BIT_FCIE_CLK_12M,
	BIT_FCIE_CLK_300K};


U32 eMMC_PlatformResetPre(void)
{

	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{

	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_eMMC_BYPASS);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	//*(U16*)0x25020DD4 |= BIT6; // FPGA only

	return eMMC_ST_SUCCESS;
}

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

//=============================================================
#elif (defined(eMMC_DRV_AGATE_UBOOT)&&eMMC_DRV_AGATE_UBOOT)
void eMMC_DumpPadClk(void)
{

	eMMC_debug(0, 1, "\nclk setting: \n");
	eMMC_debug(0, 1, "reg_ckg_fcie(0x%X):0x%x\n", reg_ckg_fcie, REG_FCIE_U16(reg_ckg_fcie));

	eMMC_debug(0, 1, "\npad setting: \n");
	eMMC_debug(0, 1, "FCIE_BOOT_CONFIG(0x%X):0x%x\n", FCIE_BOOT_CONFIG, REG_FCIE_U16(FCIE_BOOT_CONFIG));
	eMMC_debug(0, 1, "reg_all_pad_in(0x%X):0x%x\n", reg_all_pad_in, REG_FCIE_U16(reg_all_pad_in));
	eMMC_debug(0, 1, "reg_sd_pad(0x%X):0x%x\n", reg_sd_pad, REG_FCIE_U16(reg_sd_pad));
	eMMC_debug(0, 1, "reg_emmc_pad(0x%X):0x%x\n", reg_emmc_pad, REG_FCIE_U16(reg_emmc_pad));
	eMMC_debug(0, 1, "reg_nand_pad(0x%X):0x%x\n", reg_nand_pad, REG_FCIE_U16(reg_nand_pad));
	eMMC_debug(0, 1, "reg_pcmcia_pad(0x%X):0x%x\n", reg_pcmcia_pad, REG_FCIE_U16(reg_pcmcia_pad));

	eMMC_debug(0, 1, "\n");
}

#if (MS_BOARD_TYPE_SEL == BD_MST029B_10AFVA_A3 || ENABLE_MSTAR_TITANIA_BD_MS801_MAA2XG_A3 || ENABLE_MSTAR_TITANIA_BD_MST029B_CUS37_A3 || ENABLE_MSTAR_TITANIA_BD_MST029B_CUS18_A3 || ENABLE_MSTAR_TITANIA_BD_MST029B_CUS18_A3_ATSC || ENABLE_MSTAR_TITANIA_BD_MSTCUS23_6A801_A3 || ENABLE_MSTAR_TITANIA_BD_MST029B_CUS36_A3 || ENABLE_MSTAR_TITANIA_BD_CUS28_6A801 || ENABLE_MSTAR_TITANIA_BD_MST029B_BOX_A3)
static U8 eMMC_Platform_DisableBootMode(void)
{
    U16 regval;

    REG_FCIE_SETBIT(FCIE_TEST_MODE, BIT8);
    udelay(100000);

    REG_FCIE_R(FCIE_DEBUG_BUS,regval);

    if( (regval != 0x1F) && (regval != 0x0F) )
    {
        if (regval == 0x0E)
        {
//           FCIE_RIU_16_ON(EMMC_BOOT_CONFIG, WBIT01);
			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG,BIT1);
           while(1)
           {
              udelay(1);
//              regval=FCIE_RIU_R16(MIE_EVENT);
			  REG_FCIE_R(FCIE_MIE_EVENT, regval);
              if (regval&BIT15)
                 break;
           }
//           FCIE_RIU_W16(MIE_PATH_CTL, 0);
			REG_FCIE_W(FCIE_PATH_CTL, 0);
//           FCIE_RIU_W16(JOB_BL_CNT, 0);
			REG_FCIE_W(FCIE_JOB_BL_CNT, 0);
//           regval=FCIE_RIU_R16(SD_STS);
			REG_FCIE_R(FCIE_SD_STATUS, regval);
//           FCIE_RIU_W16(SD_STS, regval);
			REG_FCIE_W(FCIE_SD_STATUS, regval);
//           regval=FCIE_RIU_R16(MIE_EVENT);
			REG_FCIE_R(FCIE_MIE_EVENT, regval);
//           FCIE_RIU_W16(MIE_EVENT, regval);
			REG_FCIE_W(FCIE_MIE_EVENT, regval);
        }
//        FCIE_RIU_W16(CARD_INT_EN, regval);
		REG_FCIE_W(FCIE_CARD_INT_EN, regval);
        return 1;
    }

//    FCIE_RIU_W16(JOB_BL_CNT, 1);
	REG_FCIE_W(FCIE_JOB_BL_CNT,1);
//    FCIE_RIU_16_ON(SD_MODE, WBIT09);
	REG_FCIE_SETBIT(FCIE_SD_MODE, BIT9);
    udelay(100);
//    FCIE_RIU_W16(SD_CTL, WBIT03);
	REG_FCIE_W(FCIE_SD_CTRL, BIT3);

    while(1)
    {
       udelay(10);
//       regval=FCIE_RIU_R16(MIE_EVENT);
		REG_FCIE_R(FCIE_MIE_EVENT, regval);
       if (regval&BIT2)
           break;
    }

    udelay(1000);
//    FCIE_RIU_16_ON(EMMC_BOOT_CONFIG, WBIT01);
	REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT1);

    while(1)
    {
       udelay(10);
//       regval=FCIE_RIU_R16(MIE_EVENT);
		REG_FCIE_R(FCIE_MIE_EVENT, regval);
       if (regval&BIT15)
           break;
    }

//    FCIE_RIU_W16(MIE_PATH_CTL, 0);
	REG_FCIE_W(FCIE_PATH_CTL, 0);
//    FCIE_RIU_W16(JOB_BL_CNT, 0);
	REG_FCIE_W(FCIE_JOB_BL_CNT, 0);
//    regval=FCIE_RIU_R16(SD_STS);
	REG_FCIE_R(FCIE_SD_STATUS, regval);
//    FCIE_RIU_W16(SD_STS, regval);
	REG_FCIE_W(FCIE_SD_STATUS, regval);
//    regval=FCIE_RIU_R16(MIE_EVENT);
	REG_FCIE_R(FCIE_MIE_EVENT, regval);
//    FCIE_RIU_W16(MIE_EVENT, regval);
	REG_FCIE_W(FCIE_MIE_EVENT, regval);

//    FCIE_RIU_16_OF(SD_MODE, WBIT09);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT9);
//    FCIE_RIU_W16(SD_CTL, 0);
	REG_FCIE_W(FCIE_SD_CTRL, 0);
    return 0;
}
#endif

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{

	static U32 u32_Pre_IF=0xFFFFFFFF;



    //clr Pad Ctrl sd
	REG_FCIE_CLRBIT(reg_sd_pad, CFG_SD_PAD_MASK);
	// clr Pad Ctrl all_pad_in
	REG_FCIE_CLRBIT(reg_all_pad_in, BIT15);

	//clr nand mode
	REG_FCIE_CLRBIT(reg_nand_pad, CFG_NAND_PAD_MASK);

	//clr pcmcia pad
	REG_FCIE_CLRBIT(reg_pcmcia_pad, CFG_PCMCIA_PAD_MASK);

    //clr Pad Ctrl mmc
    REG_FCIE_CLRBIT(reg_emmc_pad, CFG_EMMC_PAD_MASK);

    //set Pad Ctrl mmc
	REG_FCIE_SETBIT(reg_emmc_pad, CFG_EMMC_PAD_MASK);

    REG_FCIE_CLRBIT(reg_emmc_clk_4X_div, DQS_DLINE_SEL);
	REG_FCIE_SETBIT(reg_emmc_clk_4X_div, SD_USE_BYPASS);


    if(u32_Pre_IF == u32_FCIE_IF_Type)
        return eMMC_ST_SUCCESS;

	switch(u32_FCIE_IF_Type){
        case FCIE_eMMC_DDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC pads: DDR\n");
			REG_FCIE_CLRBIT(reg_emmc_boot_cfg, FCIE2MACRO_SD_BYPASS);
            // disable macro
            REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,
                BIT_MACRO_EN|BIT_SD_DDR_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS|BIT_SD_FROM_TMUX);
            // set DDR mode
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_MACRO_EN|BIT_SD_DDR_EN);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
            break;

        case FCIE_eMMC_SDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC pads: SDR\n");
			REG_FCIE_CLRBIT(reg_emmc_boot_cfg, FCIE2MACRO_SD_BYPASS);
            // disable macro
            REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,
                BIT_MACRO_EN|BIT_SD_DDR_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS|BIT_SD_FROM_TMUX);
            // set SDR mode
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_MACRO_EN);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
            break;

        case FCIE_eMMC_BYPASS:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC pads: BYPASS\n");
			REG_FCIE_SETBIT(reg_emmc_boot_cfg, FCIE2MACRO_SD_BYPASS);
            // disable macro
    		REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,
				BIT_MACRO_EN|BIT_SD_DDR_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS|BIT_SD_FROM_TMUX);
    		// set bypass mode
    		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_MACRO_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
            break;

        default:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: unknown interface: %X\n",u32_FCIE_IF_Type);
            return eMMC_ST_ERR_INVALID_PARAM;
    }
	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam) {
    case BIT_FCIE_CLK_300K:
		g_eMMCDrv.u32_ClkKHz = 300;
		REG_FCIE_CLRBIT(reg_emmc_clk_4X_div, CLK4X_DIV_EN);
		break;
    case BIT_FCIE_CLK_12M:
		g_eMMCDrv.u32_ClkKHz = 12000;
		REG_FCIE_SETBIT(reg_emmc_clk_4X_div, CLK4X_DIV_EN);
		break;
    case BIT_FCIE_CLK_20M:
		g_eMMCDrv.u32_ClkKHz = 20000;
		REG_FCIE_SETBIT(reg_emmc_clk_4X_div, CLK4X_DIV_EN);
		break;
    case BIT_FCIE_CLK_27M:
		g_eMMCDrv.u32_ClkKHz = 27000;
		REG_FCIE_SETBIT(reg_emmc_clk_4X_div, CLK4X_DIV_EN);
		break;
    case BIT_FCIE_CLK_32M:
		g_eMMCDrv.u32_ClkKHz = 32000;
		REG_FCIE_CLRBIT(reg_emmc_clk_4X_div, CLK4X_DIV_EN);
		break;
	case BIT_FCIE_CLK_36M:
		g_eMMCDrv.u32_ClkKHz = 36000;
		REG_FCIE_SETBIT(reg_emmc_clk_4X_div, CLK4X_DIV_EN);
		break;
    case BIT_FCIE_CLK_40M:
		g_eMMCDrv.u32_ClkKHz = 40000;
		REG_FCIE_SETBIT(reg_emmc_clk_4X_div, CLK4X_DIV_EN);
		break;
    case BIT_FCIE_CLK_43_2M:
		g_eMMCDrv.u32_ClkKHz = 43200;
		REG_FCIE_CLRBIT(reg_emmc_clk_4X_div, CLK4X_DIV_EN);
		break;
    case BIT_FCIE_CLK_48M:
		g_eMMCDrv.u32_ClkKHz = 48000;
		REG_FCIE_SETBIT(reg_emmc_clk_4X_div, CLK4X_DIV_EN);
		break;
    default:
        eMMC_debug(eMMC_DEBUG_LEVEL_LOW,1,"eMMC ERROR: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
        return eMMC_ST_ERR_INVALID_PARAM;
    }

	REG_FCIE_W(reg_ckg_fcie, 0);
	REG_FCIE_SETBIT(reg_ckg_fcie, BIT_FCIE_CLK_SRC_SEL);

	REG_FCIE_SETBIT(reg_ckg_fcie, (u16_ClkParam<<BIT_FCIE_CLK_SHIFT));

	REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_Inverse);

	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "reg_ckg_fcie(%Xh):%Xh, Clk:%Xh\n", reg_ckg_fcie, REG_FCIE_U16(reg_ckg_fcie), u16_ClkParam);
	//eMMC_debug(0, 1, "reg_gate_miu_fcie(0x%X):0x%x\n", reg_gate_miu_fcie, REG_FCIE_U16(reg_gate_miu_fcie));
	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
    eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}

U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_43_2M,
	BIT_FCIE_CLK_40M,
	BIT_FCIE_CLK_36M,
	BIT_FCIE_CLK_32M,
	BIT_FCIE_CLK_27M,
	BIT_FCIE_CLK_20M,
	BIT_FCIE_CLK_12M,
	BIT_FCIE_CLK_300K};

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
static char *sgachar_string[]={"  2T","1.5T","2.5T","  0T"};

static U32 eMMC_FCIE_DetectDDRTiming_Ex(U8 u8_DQS, U8 u8_DelaySel)
{
	U32 u32_SectorAddr;

	u32_SectorAddr = eMMC_TEST_BLK_0;

	//eMMC_debug(0,1,"SectorAddr: %Xh\n", u32_SectorAddr);

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
	U8  au8_DQSRegVal[4]={3,1,0,2}; // 0T, 1.5T, 2T, 2.5T
	U8  au8_DQS_10T[4]={0,15,20,25}; // 0T, 1.5T, 2T, 2.5T
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
		    eMMC_debug(0,0,"eMMC Err: set DDR IF fail: %X\n", u32_err);
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

			if(FCIE_SLOW_CLK == g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.TimingTable_t.u8_SetCnt = u8_SetIdx + 1;
			
			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT;
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
		    eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-eMMC_TIMING_TABLE_CHKSUM_OFFSET);
		g_eMMCDrv.TimingTable_t.u32_VerNo = eMMC_TIMING_TABLE_VERSION;
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


U32 eMMC_PlatformResetPre(void)
{

	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{

	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_eMMC_BYPASS);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	//*(U16*)0x25020DD4 |= BIT6; // FPGA only

	return eMMC_ST_SUCCESS;
}

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

//=============================================================

#elif (defined(eMMC_DRV_EAGLE_UBOOT) && eMMC_DRV_EAGLE_UBOOT)

void eMMC_DumpPadClk(void)
{
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

	eMMC_debug(0, 1, "\n");
}

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
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
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
			return eMMC_ST_ERR_INVALID_PARAM;
	}

	// set chiptop
	REG_FCIE_CLRBIT(reg_chiptop_0x5A, BIT0|BIT1|BIT2);
	//REG_FCIE_CLRBIT(reg_chiptop_0x1F, BIT0|BIT1); // don't touch for card-detect
	//REG_FCIE_CLRBIT(reg_chiptop_0x70, BIT2); // don't touch for card-detect
	REG_FCIE_CLRBIT(reg_chiptop_0x64, BIT0);
	REG_FCIE_CLRBIT(reg_chiptop_0x4F, BIT13|BIT14);
	REG_FCIE_CLRBIT(reg_chiptop_0x03, BIT6|BIT7);
	//REG_FCIE_CLRBIT(reg_chiptop_0x51, BIT10|BIT11);  //Cause EAGLE CI card lag issue.
	REG_FCIE_CLRBIT(reg_chiptop_0x6F, BIT14|BIT15);

	REG_FCIE_CLRBIT(reg_chiptop_0x50, BIT_NAND_CS1_EN|BIT_ALL_PAD_IN);
	REG_FCIE_CLRBIT(reg_chiptop_0x0B, BIT_NAND_CS2_EN|BIT_NAND_CS3_EN|
		BIT_SD0_CFG|BIT_SD0_CFG2|BIT_SD1_CFG|BIT_SD1_CFG2|BIT_NAND_CFG);

	#if (MS_BOARD_TYPE_SEL == BD_MST142A_D01A_S_EAGLE) || \
        (MS_BOARD_TYPE_SEL == BD_MST028B_10AFX_EAGLE)
	    //defined(CONFIG_MSTAR_BD_MST028B_10AFX_EAGLE)
    // eMMC Mode1: 142A, 028B
    REG_FCIE_CLRBIT(reg_chiptop_0x5D, BIT_eMMC_CFG_MASK);
    REG_FCIE_SETBIT(reg_chiptop_0x5D, BIT_eMMC_CFG_MODE1);
	//Set pad strength
    REG_FCIE_SETBIT(reg_pcm_drv, CFG_PCM_DRV_STRENGTH);
	// pull-high PCMD
	REG_FCIE_SETBIT(reg_pcm_d_pe, 0xFF);
	#else
    // eMMC Mode2: 152A ,147A, 038B
	REG_FCIE_SETBIT(reg_chiptop_0x5D, BIT_eMMC_CFG_MODE2);
    REG_FCIE_SETBIT(reg_nand_drv, CFG_NAND_DRV_STRENGTH);
    #endif

	REG_FCIE_SETBIT(reg_emmc_drv, CFG_EMMC_CMD_STRENGTH);
	REG_FCIE_SETBIT(reg_emmc_drv, CFG_EMMC_CLK_STRENGTH);

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
		eMMC_debug(eMMC_DEBUG_LEVEL_LOW,1,"eMMC Err: invalid clk: %Xh\n", u16_ClkParam);
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
		    eMMC_debug(0,0,"eMMC Err: set DDR IF fail: %X\n", u32_err);
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

			if(FCIE_SLOW_CLK == g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.TimingTable_t.u8_SetCnt = u8_SetIdx + 1;
			
			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT;
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
		    eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-eMMC_TIMING_TABLE_CHKSUM_OFFSET);
		g_eMMCDrv.TimingTable_t.u32_VerNo = eMMC_TIMING_TABLE_VERSION;
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


U32 eMMC_PlatformResetPre(void)
{

	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
    REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT_csreg_miu_wp_last2_sel);
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_eMMC_SDR);

	#if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
    //eMMC_clock_setting(FCIE_SLOW_CLK);
    eMMC_clock_setting(FCIE_SLOWEST_CLK);
	#else
	// as usual
	eMMC_clock_setting(FCIE_SLOWEST_CLK);
	#endif

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


//=== =================================================
// <-- [FIXME]
#elif (defined(eMMC_DRV_EDISON_UBOOT)&&eMMC_DRV_EDISON_UBOOT)
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

    // Pad mux
    eMMC_debug(0, 0, "chiptop_0x12 (0x%X):0x%04X\n", reg_chiptop_0x12, REG_FCIE_U16(reg_chiptop_0x12));
    eMMC_debug(0, 0, "chiptop_0x64 (0x%X):0x%04X\n", reg_chiptop_0x64, REG_FCIE_U16(reg_chiptop_0x64));
    eMMC_debug(0, 0, "chiptop_0x40 (0x%X):0x%04X\n", reg_chiptop_0x40, REG_FCIE_U16(reg_chiptop_0x40));
    eMMC_debug(0, 0, "chiptop_0x10 (0x%X):0x%04X\n", reg_chiptop_0x10, REG_FCIE_U16(reg_chiptop_0x10));
    eMMC_debug(0, 0, "chiptop_0x6F (0x%X):0x%04X\n", reg_chiptop_0x6F, REG_FCIE_U16(reg_chiptop_0x6F));
    eMMC_debug(0, 0, "chiptop_0x5A (0x%X):0x%04X\n", reg_chiptop_0x5A, REG_FCIE_U16(reg_chiptop_0x5A));
    eMMC_debug(0, 0, "chiptop_0x6E (0x%X):0x%04X\n", reg_chiptop_0x6E, REG_FCIE_U16(reg_chiptop_0x6E));
    eMMC_debug(0, 0, "chiptop_0x09 (0x%X):0x%04X\n", reg_chiptop_0x09, REG_FCIE_U16(reg_chiptop_0x09));
    eMMC_debug(0, 0, "chiptop_0x0A (0x%X):0x%04X\n", reg_chiptop_0x0A, REG_FCIE_U16(reg_chiptop_0x0A));
    eMMC_debug(0, 0, "chiptop_0x0B (0x%X):0x%04X\n", reg_chiptop_0x0B, REG_FCIE_U16(reg_chiptop_0x0B));
    eMMC_debug(0, 0, "chiptop_0x50 (0x%X):0x%04X\n", reg_chiptop_0x50, REG_FCIE_U16(reg_chiptop_0x50));

    // Pad PU/PD
    eMMC_debug(0, 0, "chiptop_0x0D (0x%X):0x%04X\n", reg_chiptop_0x0D, REG_FCIE_U16(reg_chiptop_0x0D));
    eMMC_debug(0, 0, "chiptop_0x42 (0x%X):0x%04X\n", reg_chiptop_0x40, REG_FCIE_U16(reg_chiptop_0x42));

    // Pad driving
    eMMC_debug(0, 0, "chiptop_0x08 (0x%X):0x%04X\n", reg_chiptop_0x08, REG_FCIE_U16(reg_chiptop_0x08));
    eMMC_debug(0, 0, "chiptop_0x0C (0x%X):0x%04X\n", reg_chiptop_0x0C, REG_FCIE_U16(reg_chiptop_0x0C));

    // Patch
    eMMC_debug(0, 0, "chiptop_0x1D (0x%X):0x%04X\n", reg_chiptop_0x1D, REG_FCIE_U16(reg_chiptop_0x1D));

    // Chip config
    eMMC_debug(0, 0, "chiptop_0x65 (0x%X):0x%04X\n", reg_chiptop_0x65, REG_FCIE_U16(reg_chiptop_0x65));

    eMMC_debug(0, 0, "\n");

}

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT10|BIT11|BIT12);
    REG_FCIE_CLRBIT(reg_chiptop_0x10, BIT8);

    switch(u32_FCIE_IF_Type){
        case FCIE_eMMC_DDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: DDR\n");

            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT9);

            REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT_ddr_timing_patch);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
            break;

        case FCIE_eMMC_SDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: SDR\n");

            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
            break;

        case FCIE_eMMC_BYPASS:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: BYPASS\n");

            REG_FCIE_SETBIT(reg_chiptop_0x10, BIT8);
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT10|BIT11);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
            break;

        default:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
            return eMMC_ST_ERR_INVALID_PARAM;
    }

    // set chiptop
    REG_FCIE_SETBIT(reg_chiptop_0x09, 0x00FF);
    REG_FCIE_SETBIT(reg_chiptop_0x40, BIT0);

    REG_FCIE_SETBIT(reg_chiptop_0x08, BIT2|BIT1);
    REG_FCIE_SETBIT(reg_chiptop_0x0C, 0xFF00);
    REG_FCIE_SETBIT(reg_chiptop_0x0D, 0xFFFF);

    REG_FCIE_SETBIT(reg_chiptop_0x6E, BIT8|BIT9|BIT10|BIT11|BIT13);
    REG_FCIE_CLRBIT(reg_chiptop_0x6E, BIT12);

	REG_FCIE_CLRBIT(reg_chiptop_0x64, BIT4|BIT0);

    REG_FCIE_CLRBIT(reg_chiptop_0x50, BIT15);

	return eMMC_ST_SUCCESS;
}

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
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
#endif

U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam)	{
	case BIT_FCIE_CLK_300K:     g_eMMCDrv.u32_ClkKHz = 300;     break;
	case BIT_FCIE_CLK_20M:      g_eMMCDrv.u32_ClkKHz = 20000;   break;
    case BIT_FCIE_CLK_27M:      g_eMMCDrv.u32_ClkKHz = 27000;   break;
    #if !(defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING)
	case BIT_FCIE_CLK_32M:      g_eMMCDrv.u32_ClkKHz = 32000;   break;
    #endif
    case BIT_FCIE_CLK_36M:      g_eMMCDrv.u32_ClkKHz = 36000;   break;
	case BIT_FCIE_CLK_40M:      g_eMMCDrv.u32_ClkKHz = 40000;   break;
    #if !(defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING)
    case BIT_FCIE_CLK_43_2M:    g_eMMCDrv.u32_ClkKHz = 43200;   break;
    #endif
    case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
	default:
		eMMC_debug(eMMC_DEBUG_LEVEL_LOW,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_SEL|(u16_ClkParam<<BIT_FCIE_CLK_SHIFT));

    #if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
	if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
	{
	    REG_FCIE_CLRBIT(reg_ckg_fcie_4X,
			BIT_FCIE_CLK4X_Gate|BIT_FCIE_CLK4X_MASK);
	    REG_FCIE_SETBIT(reg_ckg_fcie_4X,
			(sgau8_FCIEClk_1X_To_4X_[u16_ClkParam]<<BIT_FCIE_CLK4X_SHIFT));

        REG_FCIE_CLRBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		//REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
		REG_FCIE_SETBIT(reg_chiptop_0x1D, BIT_DDR_TIMING_PATCH);
        REG_FCIE_SETBIT(reg_chiptop_0x1D, BIT_SW_RST_Z_EN);
		REG_FCIE_SETBIT(reg_chiptop_0x1D, BIT_SW_RST_Z);
		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        REG_FCIE_CLRBIT(reg_chiptop_0x1D, BIT_SW_RST_Z);
		REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
	}
    #endif

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
		    eMMC_debug(0,0,"eMMC Err: set DDR IF fail: %X\n", u32_err);
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

			if(FCIE_SLOW_CLK == g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.TimingTable_t.u8_SetCnt = u8_SetIdx + 1;
			
			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT;
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
		    eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-eMMC_TIMING_TABLE_CHKSUM_OFFSET);
		g_eMMCDrv.TimingTable_t.u32_VerNo = eMMC_TIMING_TABLE_VERSION;
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

U32 eMMC_PlatformResetPre(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT_csreg_miu_wp_last2_sel);
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_DEFAULT_PAD);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	return eMMC_ST_SUCCESS;
}

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


// ====================================================
#elif (defined(eMMC_DRV_EIFFEL_UBOOT)&&eMMC_DRV_EIFFEL_UBOOT)
void eMMC_DumpPadClk(void)
{
    //----------------------------------------------
    eMMC_debug(0, 0, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
    eMMC_debug(0, 0, "FCIE 1X (0x%X):0x%04X\n", reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X));
    eMMC_debug(0, 0, "FCIE 4X (0x%X):0x%04X\n", reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));
    eMMC_debug(0, 0, "MIU (0x%X):0x%04X\n", reg_ckg_MIU, REG_FCIE_U16(reg_ckg_MIU));
    eMMC_debug(0, 0, "MCU (0x%X):0x%04X\n", reg_ckg_MCU, REG_FCIE_U16(reg_ckg_MCU));

    //----------------------------------------------
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

    eMMC_debug(0, 0, "reg_test_in_out_mode (0x%08X):0x%04X\n", reg_test_in_out_mode,REG_FCIE_U16(reg_test_in_out_mode));
    eMMC_debug(0, 0, "reg_pcmcia_pad (0x%08X):0x%04X\n", reg_pcmcia_pad, REG_FCIE_U16(reg_pcmcia_pad));
    eMMC_debug(0, 0, "reg_nand_pad (0x%08X):0x%04X\n", reg_nand_pad, REG_FCIE_U16(reg_nand_pad));
    eMMC_debug(0, 0, "reg_sd_pad (0x%08X):0x%04X\n", reg_sd_pad, REG_FCIE_U16(reg_sd_pad));

    eMMC_debug(0, 0, "\n");

}

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
    u16 u16_eMMCMode;
	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT10|BIT11|BIT12);

	REG_FCIE_CLRBIT(reg_fcie2macro_sd_bypass, BIT_FCIE2MACRO_SD_BYPASS);
	REG_FCIE_SETBIT(reg_sd_use_bypass, BIT_SD_USE_BYPASS);// reg_sd_use_bypass

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

            REG_FCIE_SETBIT(reg_fcie2macro_sd_bypass, BIT_FCIE2MACRO_SD_BYPASS);    // reg_fcie2marco_sd_bypass
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT10|BIT11);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
            break;

        default:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
            return eMMC_ST_ERR_INVALID_PARAM;
    }

    // set chiptop
    REG_FCIE_R(reg_emmc_pad,u16_eMMCMode);
    u16_eMMCMode &= CFG_EMMC_PAD_MASK;
	if(u16_eMMCMode == CFG_EMMC_PAD_Mode1)
	{
        REG_FCIE_SETBIT(reg_pcm_a_pe, 0x00FF);
	}

    REG_FCIE_CLRBIT(reg_test_in_out_mode, reg_test_in_out_mode_mask);
	REG_FCIE_CLRBIT(reg_pcmcia_pad, CFG_PCMCIA_PAD_MASK);
    REG_FCIE_CLRBIT(reg_sd_pad, CFG_SD_PAD_MASK);
    REG_FCIE_CLRBIT(reg_nand_pad, CFG_NAND_PAD_MASK);
    REG_FCIE_CLRBIT(reg_all_pad_in, BIT_ALL_PAD_IN);

	return eMMC_ST_SUCCESS;
}

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
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
#endif

U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam)	{
	case BIT_FCIE_CLK_300K:     g_eMMCDrv.u32_ClkKHz = 300;     break;
	case BIT_FCIE_CLK_20M:      g_eMMCDrv.u32_ClkKHz = 20000;   break;
    case BIT_FCIE_CLK_27M:      g_eMMCDrv.u32_ClkKHz = 27000;   break;
    #if !(defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING)
	case BIT_FCIE_CLK_32M:      g_eMMCDrv.u32_ClkKHz = 32000;   break;
    #endif
    case BIT_FCIE_CLK_36M:      g_eMMCDrv.u32_ClkKHz = 36000;   break;
	case BIT_FCIE_CLK_40M:      g_eMMCDrv.u32_ClkKHz = 40000;   break;
    #if !(defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING)
    case BIT_FCIE_CLK_43_2M:    g_eMMCDrv.u32_ClkKHz = 43200;   break;
    #endif
    case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
	default:
		eMMC_debug(eMMC_DEBUG_LEVEL_LOW,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_SEL|(u16_ClkParam<<BIT_FCIE_CLK_SHIFT));

    #if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
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
    #endif

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
		    eMMC_debug(0,0,"eMMC Err: set DDR IF fail: %X\n", u32_err);
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

			if(FCIE_SLOW_CLK == g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.TimingTable_t.u8_SetCnt = u8_SetIdx + 1;
			
			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT;
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
		    eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-eMMC_TIMING_TABLE_CHKSUM_OFFSET);
		g_eMMCDrv.TimingTable_t.u32_VerNo = eMMC_TIMING_TABLE_VERSION;
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


U32 eMMC_PlatformResetPre(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_eMMC_SDR);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

    // Set power good status de-glitch
    REG_FCIE_SETBIT(reg_pwrgd_int_glirm, BIT_PWRGD_INT_GLIRM_EN);

	return eMMC_ST_SUCCESS;
}

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
// =============================================
#elif (defined(eMMC_DRV_NIKE_UBOOT) && eMMC_DRV_NIKE_UBOOT)
void eMMC_DumpPadClk(void)
{
    //----------------------------------------------
    eMMC_debug(0, 0, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
    eMMC_debug(0, 0, "FCIE CLK(0x%X):0x%04X\n", reg_ckg_fcie, REG_FCIE_U16(reg_ckg_fcie));
    eMMC_debug(0, 0, "reg_ckg_nfie_div4_en (0x%X):0x%04X\n", reg_ckg_nfie_div4_en, REG_FCIE_U16(reg_ckg_nfie_div4_en));

    //----------------------------------------------
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

    eMMC_debug(0, 0, "reg_chiptop_0x50 (0x%08X):0x%04X\n", reg_chiptop_0x50, REG_FCIE_U16(reg_chiptop_0x50));
    eMMC_debug(0, 0, "reg_chiptop_0x43 (0x%08X):0x%04X\n", reg_chiptop_0x43, REG_FCIE_U16(reg_chiptop_0x43));
    eMMC_debug(0, 0, "reg_chiptop_0x5D (0x%08X):0x%04X\n", reg_chiptop_0x5D, REG_FCIE_U16(reg_chiptop_0x5D));

    eMMC_debug(0, 0, "reg_chiptop_0x0B (0x%08X):0x%04X\n", reg_chiptop_0x0B, REG_FCIE_U16(reg_chiptop_0x0B));
    eMMC_debug(0, 0, "\n");

}

void eMMC_pad_switch_4_tSD(void)
{
	// 0x0C
	REG_FCIE_SETBIT(reg_chiptop_0x0C, BIT7|BIT6|BIT5|BIT4);

	// 0x42
	REG_FCIE_SETBIT(reg_chiptop_0x42, BIT6|BIT5|BIT4);

	// 0x43
	REG_FCIE_SETBIT(reg_chiptop_0x43, BIT_PAD_BYPASS_MACRO);

	// 0x50
	// eMMC pad switch did in eMMC_pads_switch()

	// 0x5B
	REG_FCIE_CLRBIT(reg_chiptop_0x5B, BIT9);
	REG_FCIE_SETBIT(reg_chiptop_0x5B, BIT8);//kernel disapeer

	// 0x5D
	REG_FCIE_CLRBIT(reg_chiptop_0x5D, BIT15|BIT14);

	// 0x12
	REG_FCIE_CLRBIT(reg_chiptop_0x12, BIT6|BIT5|BIT4|BIT2|BIT1|BIT0);

	// 0x4F
	REG_FCIE_CLRBIT(reg_chiptop_0x4F, BIT14|BIT13);

	// 0x64
	REG_FCIE_CLRBIT(reg_chiptop_0x64, BIT0);

	// 0x5B
	REG_FCIE_CLRBIT(reg_chiptop_0x5B, BIT11|BIT10);

	// 0x51
	REG_FCIE_CLRBIT(reg_chiptop_0x51, BIT0);

	// 0x1F
	REG_FCIE_CLRBIT(reg_chiptop_0x1F, BIT1);

	// 0x77
	REG_FCIE_CLRBIT(reg_chiptop_0x77, BIT8);
}

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT10|BIT11|BIT12);

	REG_FCIE_CLRBIT(reg_chiptop_0x43, BIT_PAD_BYPASS_MACRO);

	switch(u32_FCIE_IF_Type){
	case FCIE_eMMC_DDR:
		eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: DDR\n");

		// Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT9);

		REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT_ddr_timing_patch);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_DDR_TIMING_PATCH);

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

		REG_FCIE_SETBIT(reg_chiptop_0x43, BIT_PAD_BYPASS_MACRO);    // reg_fcie2marco_sd_bypass
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT10|BIT11);

		g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
		break;

#if defined USE_SD_PAD_2_DEVELOP_TSD && USE_SD_PAD_2_DEVELOP_TSD

	case FCIE_tSD_BYPASS: // use for debug & develop tSD internal storage use only
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT10);
		REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT15);
		eMMC_pad_switch_4_tSD();
		g_eMMCDrv.u8_PadType = FCIE_tSD_BYPASS;
		break;
	case FCIE_tSD_SDR:
		REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT10);
		REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT15);
		eMMC_pad_switch_4_tSD();
		g_eMMCDrv.u8_PadType = FCIE_tSD_SDR;
		break;
#endif

	default:
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: unknown interface: %X\n", u32_FCIE_IF_Type);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

    // set chiptop
    REG_FCIE_CLRBIT(reg_chiptop_0x0B, BIT_NAND_CFG_MASK);
    REG_FCIE_CLRBIT(reg_chiptop_0x50, BIT_ALL_PAD_IN);
    // fcie driving strength
    //REG_FCIE_SETBIT(reg_emmc_drv, CFG_EMMC_CMD_STRENGTH);
    //REG_FCIE_SETBIT(reg_emmc_drv, CFG_EMMC_CLK_STRENGTH);
    //REG_FCIE_SETBIT(reg_nand_drv, CFG_NAND_DRV_STRENGTH);

	return eMMC_ST_SUCCESS;
}


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
    case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
	default:
		eMMC_debug(eMMC_DEBUG_LEVEL_LOW,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie, u16_ClkParam<<BIT_FCIE_CLK_SHIFT);
	REG_FCIE_SETBIT(reg_ckg_nfie_div4_en, BIT_FCIE_CLK_DIV4_EN);

	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "clk:%uKHz, Param:%Xh, fcie(%Xh):%Xh, fcie_4X_en(%Xh):%Xh\n",
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
		    eMMC_debug(0,0,"eMMC Err: set DDR IF fail: %X\n", u32_err);
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

			if(FCIE_SLOW_CLK == g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.TimingTable_t.u8_SetCnt = u8_SetIdx + 1;
			
			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT;
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
		    eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-eMMC_TIMING_TABLE_CHKSUM_OFFSET);
		g_eMMCDrv.TimingTable_t.u32_VerNo = eMMC_TIMING_TABLE_VERSION;
		memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t));

		eMMC_FCIE_ApplyTimingSet(eMMC_TIMING_SET_MAX);

		#if 1
		u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
	    u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
	    if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	    {
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: %Xh %Xh\n",
			    u32_err, u32_ret);
		    return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	    }
		#else
		eMMC_debug(0,1,"eMMC: does not save t_table\n");
		#endif
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


U32 eMMC_PlatformResetPre(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_eMMC_BYPASS);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

    // Set power good status de-glitch
    REG_FCIE_SETBIT(reg_pwrgd_int_glirm, BIT_PWRGD_INT_GLIRM_EN);

	return eMMC_ST_SUCCESS;
}

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

// =============================================
#elif (defined(eMMC_DRV_MADISON_UBOOT) && eMMC_DRV_MADISON_UBOOT)
void eMMC_DumpPadClk(void)
{
    //----------------------------------------------
    eMMC_debug(0, 0, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
    eMMC_debug(0, 0, "FCIE CLK(0x%X):0x%04X\n", reg_ckg_fcie, REG_FCIE_U16(reg_ckg_fcie));
    eMMC_debug(0, 0, "reg_ckg_nfie_div4_en (0x%X):0x%04X\n", reg_ckg_nfie_div4_en, REG_FCIE_U16(reg_ckg_nfie_div4_en));
	eMMC_debug(0, 0, "XTAL CLK SEL(0x%X):0x%04X\n", reg_ckg_nfie, REG_FCIE_U16(reg_ckg_nfie));

    //----------------------------------------------
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

    eMMC_debug(0, 0, "reg_chiptop_0x50 (0x%08X):0x%04X\n", reg_chiptop_0x50, REG_FCIE_U16(reg_chiptop_0x50));
    eMMC_debug(0, 0, "reg_chiptop_0x43 (0x%08X):0x%04X\n", reg_chiptop_0x43, REG_FCIE_U16(reg_chiptop_0x43));
	eMMC_debug(0, 0, "reg_chiptop_0x5B (0x%08X):0x%04X\n", reg_sd_config,    REG_FCIE_U16(reg_sd_config));
    eMMC_debug(0, 0, "reg_chiptop_0x5D (0x%08X):0x%04X\n", reg_emmc_config, REG_FCIE_U16(reg_emmc_config));
    eMMC_debug(0, 0, "\n");

    

}


U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT10|BIT11|BIT12);

	REG_FCIE_CLRBIT(reg_chiptop_0x43, BIT_PAD_BYPASS_MACRO);
	REG_FCIE_CLRBIT(FCIE_SD_MACRO_REDNT2, BIT_DDR_TIMING_LATCH);
	REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT_ddr_timing_patch);

	switch(u32_FCIE_IF_Type){
	case FCIE_eMMC_DDR:
		eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: DDR\n");

		// Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT9);
		REG_FCIE_SETBIT(FCIE_SD_MACRO_REDNT2, BIT_DDR_TIMING_LATCH);
		REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT_ddr_timing_patch);

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

		REG_FCIE_SETBIT(reg_chiptop_0x43, BIT_PAD_BYPASS_MACRO);    // reg_fcie2marco_sd_bypass
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT10|BIT11);

		g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
		break;

#if defined USE_SD_PAD_2_DEVELOP_TSD && USE_SD_PAD_2_DEVELOP_TSD

	case FCIE_tSD_BYPASS: // use for debug & develop tSD internal storage use only
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT10);
		REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT15);
		eMMC_pad_switch_4_tSD();
		g_eMMCDrv.u8_PadType = FCIE_tSD_BYPASS;
		break;
	case FCIE_tSD_SDR:
		REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT10);
		REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT15);
		eMMC_pad_switch_4_tSD();
		g_eMMCDrv.u8_PadType = FCIE_tSD_SDR;
		break;
#endif

	default:
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: unknown interface: %X\n", u32_FCIE_IF_Type);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

    // set chiptop
    REG_FCIE_CLRBIT(reg_chiptop_0x0B, BIT_NAND_CFG_MASK);
    REG_FCIE_CLRBIT(reg_sd_config, BIT_SD_CFG_MASK);
	REG_FCIE_CLRBIT(reg_emmc_config, BIT_EMMC_CFG_MASK);
	REG_FCIE_SETBIT(reg_emmc_config, BIT_EMMC_MODE1);
    REG_FCIE_CLRBIT(reg_chiptop_0x50, BIT_ALL_PAD_IN|BIT_NAND_CS1_EN);


	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	REG_FCIE_CLRBIT(reg_ckg_nfie, BIT_XTAL_CLK_SEL);

	switch(u16_ClkParam)	{
	case BIT_FCIE_CLK_300K:     g_eMMCDrv.u32_ClkKHz = 300;     break;
	case BIT_FCIE_CLK_18M:      g_eMMCDrv.u32_ClkKHz = 18000;   break;
    case BIT_FCIE_CLK_27M:      g_eMMCDrv.u32_ClkKHz = 27000;   break;
    case BIT_FCIE_CLK_36M:      g_eMMCDrv.u32_ClkKHz = 36000;   break;
	case BIT_FCIE_CLK_40M:      g_eMMCDrv.u32_ClkKHz = 40000;   break;
    case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
	default:
		eMMC_debug(eMMC_DEBUG_LEVEL_LOW,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie, u16_ClkParam<<BIT_FCIE_CLK_SHIFT);
	REG_FCIE_SETBIT(reg_ckg_nfie_div4_en, BIT_FCIE_CLK_DIV4_EN);
	REG_FCIE_SETBIT(reg_ckg_nfie, BIT_XTAL_CLK_SEL);

	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "clk:%uKHz, Param:%Xh, fcie(%Xh):%Xh, fcie_4X_en(%Xh):%Xh\n",
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
		    eMMC_debug(0,0,"eMMC Err: set DDR IF fail: %X\n", u32_err);
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

			if(FCIE_SLOW_CLK == g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.TimingTable_t.u8_SetCnt = u8_SetIdx + 1;
			
			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT;
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
		    eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-eMMC_TIMING_TABLE_CHKSUM_OFFSET);
		g_eMMCDrv.TimingTable_t.u32_VerNo = eMMC_TIMING_TABLE_VERSION;
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


U32 eMMC_PlatformResetPre(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_DEFAULT_PAD);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

    // Set power good status de-glitch
    REG_FCIE_SETBIT(reg_pwrgd_int_glirm, BIT_PWRGD_INT_GLIRM_EN);

	return eMMC_ST_SUCCESS;
}

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

// =============================================
#elif (defined(eMMC_DRV_MIAMI_UBOOT) && eMMC_DRV_MIAMI_UBOOT)
void eMMC_DumpPadClk(void)
{
    //----------------------------------------------
    eMMC_debug(0, 0, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
    eMMC_debug(0, 0, "FCIE CLK(0x%X):0x%04X\n", reg_ckg_fcie, REG_FCIE_U16(reg_ckg_fcie));
    eMMC_debug(0, 0, "reg_ckg_nfie_div4_en (0x%X):0x%04X\n", reg_ckg_nfie_div4_en, REG_FCIE_U16(reg_ckg_nfie_div4_en));

    //----------------------------------------------
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

    eMMC_debug(0, 0, "reg_chiptop_0x50 (0x%08X):0x%04X\n", reg_chiptop_0x50, REG_FCIE_U16(reg_chiptop_0x50));
    eMMC_debug(0, 0, "reg_chiptop_0x43 (0x%08X):0x%04X\n", reg_chiptop_0x43, REG_FCIE_U16(reg_chiptop_0x43));
    eMMC_debug(0, 0, "reg_chiptop_0x5D (0x%08X):0x%04X\n", reg_chiptop_0x5D, REG_FCIE_U16(reg_chiptop_0x5D));

    eMMC_debug(0, 0, "reg_chiptop_0x0B (0x%08X):0x%04X\n", reg_chiptop_0x0B, REG_FCIE_U16(reg_chiptop_0x0B));
    eMMC_debug(0, 0, "\n");

}

void eMMC_pad_switch_4_tSD(void)
{
	// 0x0C
	REG_FCIE_SETBIT(reg_chiptop_0x0C, BIT7|BIT6|BIT5|BIT4);

	// 0x42
	REG_FCIE_SETBIT(reg_chiptop_0x42, BIT6|BIT5|BIT4);

	// 0x43
	REG_FCIE_SETBIT(reg_chiptop_0x43, BIT_PAD_BYPASS_MACRO);

	// 0x50
	// eMMC pad switch did in eMMC_pads_switch()

	// 0x5B
	REG_FCIE_CLRBIT(reg_chiptop_0x5B, BIT9);
	REG_FCIE_SETBIT(reg_chiptop_0x5B, BIT8);//kernel disapeer

	// 0x5D
	REG_FCIE_CLRBIT(reg_chiptop_0x5D, BIT15|BIT14);

	// 0x12
	REG_FCIE_CLRBIT(reg_chiptop_0x12, BIT6|BIT5|BIT4|BIT2|BIT1|BIT0);

	// 0x4F
	REG_FCIE_CLRBIT(reg_chiptop_0x4F, BIT14|BIT13);

	// 0x64
	REG_FCIE_CLRBIT(reg_chiptop_0x64, BIT0);

	// 0x5B
	REG_FCIE_CLRBIT(reg_chiptop_0x5B, BIT11|BIT10);

	// 0x51
	REG_FCIE_CLRBIT(reg_chiptop_0x51, BIT0);

	// 0x1F
	REG_FCIE_CLRBIT(reg_chiptop_0x1F, BIT1);

	// 0x77
	REG_FCIE_CLRBIT(reg_chiptop_0x77, BIT8);
}

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT10|BIT11|BIT12);

	REG_FCIE_CLRBIT(reg_chiptop_0x43, BIT_PAD_BYPASS_MACRO);

	switch(u32_FCIE_IF_Type){
	case FCIE_eMMC_DDR:
		eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: DDR\n");

		// Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT9);

		REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT_ddr_timing_patch);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_DDR_TIMING_PATCH);

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

		REG_FCIE_SETBIT(reg_chiptop_0x43, BIT_PAD_BYPASS_MACRO);    // reg_fcie2marco_sd_bypass
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT10|BIT11);

		g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
		break;

#if defined USE_SD_PAD_2_DEVELOP_TSD && USE_SD_PAD_2_DEVELOP_TSD

	case FCIE_tSD_BYPASS: // use for debug & develop tSD internal storage use only
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT10);
		REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT15);
		eMMC_pad_switch_4_tSD();
		g_eMMCDrv.u8_PadType = FCIE_tSD_BYPASS;
		break;
	case FCIE_tSD_SDR:
		REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT10);
		REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT15);
		eMMC_pad_switch_4_tSD();
		g_eMMCDrv.u8_PadType = FCIE_tSD_SDR;
		break;
#endif

	default:
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: unknown interface: %X\n", u32_FCIE_IF_Type);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

    // set chiptop
    REG_FCIE_CLRBIT(reg_chiptop_0x0B, BIT_NAND_CFG_MASK);
    REG_FCIE_CLRBIT(reg_chiptop_0x50, BIT_ALL_PAD_IN);
    // fcie driving strength
    //REG_FCIE_SETBIT(reg_emmc_drv, CFG_EMMC_CMD_STRENGTH);
    //REG_FCIE_SETBIT(reg_emmc_drv, CFG_EMMC_CLK_STRENGTH);
    //REG_FCIE_SETBIT(reg_nand_drv, CFG_NAND_DRV_STRENGTH);

	return eMMC_ST_SUCCESS;
}


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
    case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
	default:
		eMMC_debug(eMMC_DEBUG_LEVEL_LOW,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie, u16_ClkParam<<BIT_FCIE_CLK_SHIFT);
	REG_FCIE_SETBIT(reg_ckg_nfie_div4_en, BIT_FCIE_CLK_DIV4_EN);

	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "clk:%uKHz, Param:%Xh, fcie(%Xh):%Xh, fcie_4X_en(%Xh):%Xh\n",
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
		    eMMC_debug(0,0,"eMMC Err: set DDR IF fail: %X\n", u32_err);
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

			if(FCIE_SLOW_CLK == g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.TimingTable_t.u8_SetCnt = u8_SetIdx + 1;
			
			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT;
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
		    eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-eMMC_TIMING_TABLE_CHKSUM_OFFSET);
		g_eMMCDrv.TimingTable_t.u32_VerNo = eMMC_TIMING_TABLE_VERSION;
		memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t));

		eMMC_FCIE_ApplyTimingSet(eMMC_TIMING_SET_MAX);

		#if 1
		u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
	    u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
	    if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	    {
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: %Xh %Xh\n",
			    u32_err, u32_ret);
		    return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	    }
		#else
		eMMC_debug(0,1,"eMMC: does not save t_table\n");
		#endif
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


U32 eMMC_PlatformResetPre(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_eMMC_BYPASS);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

    // Set power good status de-glitch
    REG_FCIE_SETBIT(reg_pwrgd_int_glirm, BIT_PWRGD_INT_GLIRM_EN);

	return eMMC_ST_SUCCESS;
}

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

// =============================================
#elif (defined(eMMC_DRV_NADAL_UBOOT) && eMMC_DRV_NADAL_UBOOT)
void eMMC_DumpPadClk(void)
{
    //----------------------------------------------
    eMMC_debug(0, 0, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
    eMMC_debug(0, 0, "FCIE CLK(0x%X):0x%04X\n", reg_ckg_fcie, REG_FCIE_U16(reg_ckg_fcie));
    eMMC_debug(0, 0, "reg_ckg_nfie_div4_en (0x%X):0x%04X\n", reg_ckg_nfie_div4_en, REG_FCIE_U16(reg_ckg_nfie_div4_en));

    //----------------------------------------------
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

    eMMC_debug(0, 0, "reg_chiptop_0x50 (0x%08X):0x%04X\n", reg_chiptop_0x50, REG_FCIE_U16(reg_chiptop_0x50));
    eMMC_debug(0, 0, "reg_chiptop_0x43 (0x%08X):0x%04X\n", reg_chiptop_0x43, REG_FCIE_U16(reg_chiptop_0x43));
    eMMC_debug(0, 0, "reg_chiptop_0x5D (0x%08X):0x%04X\n", reg_chiptop_0x5D, REG_FCIE_U16(reg_chiptop_0x5D));

    eMMC_debug(0, 0, "reg_chiptop_0x0B (0x%08X):0x%04X\n", reg_chiptop_0x0B, REG_FCIE_U16(reg_chiptop_0x0B));
    eMMC_debug(0, 0, "\n");

}

void eMMC_pad_switch_4_tSD(void)
{
	// 0x0C
	REG_FCIE_SETBIT(reg_chiptop_0x0C, BIT7|BIT6|BIT5|BIT4);

	// 0x42
	REG_FCIE_SETBIT(reg_chiptop_0x42, BIT6|BIT5|BIT4);

	// 0x43
	REG_FCIE_SETBIT(reg_chiptop_0x43, BIT_PAD_BYPASS_MACRO);

	// 0x50
	// eMMC pad switch did in eMMC_pads_switch()

	// 0x5B
	REG_FCIE_CLRBIT(reg_chiptop_0x5B, BIT9);
	REG_FCIE_SETBIT(reg_chiptop_0x5B, BIT8);//kernel disapeer

	// 0x5D
	REG_FCIE_CLRBIT(reg_chiptop_0x5D, BIT15|BIT14);

	// 0x12
	REG_FCIE_CLRBIT(reg_chiptop_0x12, BIT6|BIT5|BIT4|BIT2|BIT1|BIT0);

	// 0x4F
	REG_FCIE_CLRBIT(reg_chiptop_0x4F, BIT14|BIT13);

	// 0x64
	REG_FCIE_CLRBIT(reg_chiptop_0x64, BIT0);

	// 0x5B
	REG_FCIE_CLRBIT(reg_chiptop_0x5B, BIT11|BIT10);

	// 0x51
	REG_FCIE_CLRBIT(reg_chiptop_0x51, BIT0);

	// 0x1F
	REG_FCIE_CLRBIT(reg_chiptop_0x1F, BIT1);

	// 0x77
	REG_FCIE_CLRBIT(reg_chiptop_0x77, BIT8);
}

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT10|BIT11|BIT12);

	REG_FCIE_CLRBIT(reg_chiptop_0x43, BIT_PAD_BYPASS_MACRO);

	switch(u32_FCIE_IF_Type){
	case FCIE_eMMC_DDR:
		eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: DDR\n");

		// Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT9);

		REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT_ddr_timing_patch);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_DDR_TIMING_PATCH);

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

		REG_FCIE_SETBIT(reg_chiptop_0x43, BIT_PAD_BYPASS_MACRO);    // reg_fcie2marco_sd_bypass
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT10|BIT11);

		g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
		break;

#if defined USE_SD_PAD_2_DEVELOP_TSD && USE_SD_PAD_2_DEVELOP_TSD

	case FCIE_tSD_BYPASS: // use for debug & develop tSD internal storage use only
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT10);
		REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT15);
		eMMC_pad_switch_4_tSD();
		g_eMMCDrv.u8_PadType = FCIE_tSD_BYPASS;
		break;
	case FCIE_tSD_SDR:
		REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT10);
		REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT15);
		eMMC_pad_switch_4_tSD();
		g_eMMCDrv.u8_PadType = FCIE_tSD_SDR;
		break;
#endif

	default:
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: unknown interface: %X\n", u32_FCIE_IF_Type);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

    // set chiptop
    REG_FCIE_CLRBIT(reg_chiptop_0x0B, BIT_NAND_CFG_MASK);
    REG_FCIE_CLRBIT(reg_chiptop_0x50, BIT_ALL_PAD_IN);
    // fcie driving strength
    //REG_FCIE_SETBIT(reg_emmc_drv, CFG_EMMC_CMD_STRENGTH);
    //REG_FCIE_SETBIT(reg_emmc_drv, CFG_EMMC_CLK_STRENGTH);
    //REG_FCIE_SETBIT(reg_nand_drv, CFG_NAND_DRV_STRENGTH);

	return eMMC_ST_SUCCESS;
}


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
    case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
	default:
		eMMC_debug(eMMC_DEBUG_LEVEL_LOW,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie, u16_ClkParam<<BIT_FCIE_CLK_SHIFT);
	REG_FCIE_SETBIT(reg_ckg_nfie_div4_en, BIT_FCIE_CLK_DIV4_EN);

	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "clk:%uKHz, Param:%Xh, fcie(%Xh):%Xh, fcie_4X_en(%Xh):%Xh\n",
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
		    eMMC_debug(0,0,"eMMC Err: set DDR IF fail: %X\n", u32_err);
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

			if(FCIE_SLOW_CLK == g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.TimingTable_t.u8_SetCnt = u8_SetIdx + 1;
			
			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT;
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
		    eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-eMMC_TIMING_TABLE_CHKSUM_OFFSET);
		g_eMMCDrv.TimingTable_t.u32_VerNo = eMMC_TIMING_TABLE_VERSION;
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


U32 eMMC_PlatformResetPre(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_eMMC_BYPASS);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

    // Set power good status de-glitch
    REG_FCIE_SETBIT(reg_pwrgd_int_glirm, BIT_PWRGD_INT_GLIRM_EN);

	return eMMC_ST_SUCCESS;
}

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

// =============================================
#elif (defined(eMMC_DRV_A5_UBOOT)&&eMMC_DRV_A5_UBOOT)

void eMMC_DumpPadClk(void)
{

	eMMC_debug(0, 1, "\nclk setting: \n");
	eMMC_debug(0, 1, "reg_ckg_fcie(0x%X):0x%x\n", reg_ckg_fcie, REG_FCIE_U16(reg_ckg_fcie));

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


	eMMC_debug(0, 1, "\npad setting: \n");
	eMMC_debug(0, 1, "FCIE_BOOT_CONFIG(0x%X):0x%x\n", FCIE_BOOT_CONFIG, REG_FCIE_U16(FCIE_BOOT_CONFIG));
	eMMC_debug(0, 1, "reg_all_pad_in(0x%X):0x%x\n", reg_all_pad_in, REG_FCIE_U16(reg_all_pad_in));
	eMMC_debug(0, 1, "reg_sd_pad(0x%X):0x%x\n", reg_sd_pad, REG_FCIE_U16(reg_sd_pad));
	eMMC_debug(0, 1, "reg_emmc_pad(0x%X):0x%x\n", reg_emmc_pad, REG_FCIE_U16(reg_emmc_pad));
	eMMC_debug(0, 1, "reg_nand_pad(0x%X):0x%x\n", reg_nand_pad, REG_FCIE_U16(reg_nand_pad));
	eMMC_debug(0, 1, "reg_pcmcia_pad(0x%X):0x%x\n", reg_pcmcia_pad, REG_FCIE_U16(reg_pcmcia_pad));

	eMMC_debug(0, 1, "\n");
}


U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	// disable macro
	//REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT_SD_DDR_EN|BIT_MACRO_EN);
	// set bypass mode
	//REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_SD_BYPASS_MODE_EN);

	// set emmc pad mode
	//REG_FCIE_CLRBIT(reg_emmc_pad, CFG_EMMC_PAD_MASK);
	REG_FCIE_SETBIT(reg_emmc_pad, CFG_EMMC_PAD);

	// clr Pad Ctrl all_pad_in
	REG_FCIE_CLRBIT(reg_all_pad_in, BIT15);	//OK

	//clr SD PAD switch
    REG_FCIE_CLRBIT(reg_sd_pad, CFG_SD_PAD);//OK

	//clr nand mode
	REG_FCIE_CLRBIT(reg_nand_pad, CFG_NAND_PAD_MASK);	//OK

	//clr pcmcia pad
	REG_FCIE_CLRBIT(reg_pcmcia_pad, REG_PCMCIA_PAD_MASK);	//OK


	switch(u32_FCIE_IF_Type){
        case FCIE_eMMC_DDR:
            eMMC_debug(0,1,"eMMC pads: DDR no supported!!\n");
            break;

        case FCIE_eMMC_SDR:
            eMMC_debug(0,1,"eMMC pads: DDR no supported!!\n");
            break;

        case FCIE_eMMC_BYPASS:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC pads: BYPASS\n");

			g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
            break;

        default:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: unknown interface: %X\n",u32_FCIE_IF_Type);
            return eMMC_ST_ERR_INVALID_PARAM;
    }
	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	switch(u16_ClkParam) {
    case BIT_FCIE_CLK_300K:		g_eMMCDrv.u32_ClkKHz = 300;		break;
    case BIT_FCIE_CLK_12M:		g_eMMCDrv.u32_ClkKHz = 12000;	break;
    case BIT_FCIE_CLK_20M:		g_eMMCDrv.u32_ClkKHz = 20000;	break;
    case BIT_FCIE_CLK_24M:		g_eMMCDrv.u32_ClkKHz = 24000;	break;
    case BIT_FCIE_CLK_27M:		g_eMMCDrv.u32_ClkKHz = 27000;	break;
    case BIT_FCIE_CLK_32M:		g_eMMCDrv.u32_ClkKHz = 32000;	break;
    case BIT_FCIE_CLK_40M:		g_eMMCDrv.u32_ClkKHz = 40000;	break;
    case BIT_FCIE_CLK_43_2M:	g_eMMCDrv.u32_ClkKHz = 43200;	break;
    case BIT_FCIE_CLK_48M:		g_eMMCDrv.u32_ClkKHz = 48000;	break;
    default:
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
        return eMMC_ST_ERR_INVALID_PARAM;
    }

	REG_FCIE_W(reg_ckg_fcie, 0);
	REG_FCIE_SETBIT(reg_ckg_fcie, BIT_FCIE_CLK_SRC_SEL);

	REG_FCIE_SETBIT(reg_ckg_fcie, (u16_ClkParam<<BIT_FCIE_CLK_SHIFT));

	REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_Inverse);

	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "reg_ckg_fcie(%Xh):%Xh, Clk:%Xh\n", reg_ckg_fcie, REG_FCIE_U16(reg_ckg_fcie), u16_ClkParam);
	//eMMC_debug(0, 1, "reg_gate_miu_fcie(0x%X):0x%x\n", reg_gate_miu_fcie, REG_FCIE_U16(reg_gate_miu_fcie));
	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
    eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}

U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_43_2M,
	BIT_FCIE_CLK_40M,
	BIT_FCIE_CLK_36M,
	BIT_FCIE_CLK_32M,
	BIT_FCIE_CLK_27M,
	BIT_FCIE_CLK_20M,
	BIT_FCIE_CLK_12M,
	BIT_FCIE_CLK_300K};

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
static char *sgachar_string[]={"  2T","1.5T","2.5T","  0T"};

static U32 eMMC_FCIE_DetectDDRTiming_Ex(U8 u8_DQS, U8 u8_DelaySel)
{
	eMMC_debug(0,1,"eMMC pads: DDR no supported!!\n");
	return 0;
}

#define FCIE_DELAY_CELL_ts         700 // 0.7ns
static eMMC_FCIE_DDRT_WINDOW_t sg_DDRTWindow[2];

U32 eMMC_FCIE_DetectDDRTiming(void)
{
	eMMC_debug(0,1,"eMMC pads: DDR no supported!!\n");
	return 0;
}


U32 eMMC_FCIE_BuildDDRTimingTable(void)
{
	eMMC_debug(0,1,"eMMC pads: DDR no supported!!\n");
	return 0;
}

#endif // IF_DETECT_eMMC_DDR_TIMING


U32 eMMC_PlatformResetPre(void)
{

	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{

	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_eMMC_BYPASS);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	//*(U16*)0x25020DD4 |= BIT6; // FPGA only

	return eMMC_ST_SUCCESS;
}

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

///////////////////////////////////////////////////////////////////////////////

#elif (defined(eMMC_DRV_EINSTEIN_UBOOT)&&eMMC_DRV_EINSTEIN_UBOOT)

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
    REG_FCIE_CLRBIT(reg_emmc_config,BIT_EMMC_CONFIG);
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
		eMMC_die();
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
	REG_FCIE_SETBIT(reg_emmcpll_reset,BIT0);
	eMMC_hw_timer_delay(1);
	REG_FCIE_CLRBIT(reg_emmcpll_reset,BIT0);
	eMMC_hw_timer_delay(1);

	// 2. synth clock
	switch(u16_ClkParam) {
		case eMMC_PLL_CLK_200M: // 200M
		    REG_FCIE_CLRBIT(reg_ddfset_23_16,0xffff);
			REG_FCIE_CLRBIT(reg_ddfset_15_00,0xffff);
			REG_FCIE_SETBIT(reg_ddfset_23_16,0x22);
			REG_FCIE_SETBIT(reg_ddfset_15_00,0x8F5C);
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
			eMMC_debug(0,0,"eMMC Err: emmcpll not configed %Xh\n", u16_ClkParam);
			eMMC_die();
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
		REG_FCIE_SETBIT(reg_emmcpll_test, BIT10);
	}
	else {
		REG_FCIE_CLRBIT(reg_emmcpll_test, BIT10);
	}
	
	eMMC_hw_timer_delay(HW_TIMER_DELAY_1ms);
	//eMMC_debug(0,1," emmcpll: %Xh\n", u16_ClkParam);
	return eMMC_ST_SUCCESS;
}


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
			eMMC_die();
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


U32 eMMC_PlatformResetPre(void)
{
	//pull-high CMD pin
	REG_FCIE_SETBIT(PAD_CHIPTOP_BASE + (0x29 << 2), BIT13);

	return eMMC_ST_SUCCESS;
}

U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT] = {
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_36M,
	BIT_FCIE_CLK_20M,
};

U32 eMMC_PlatformResetPost(void)
{
	REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT_eco_d0_busy_check);
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
    #if 1	
    // for eMMC 4.5 HS200 need 1.8V, unify all eMMC IO power to 1.8V
	// works both for eMMC 4.4 & 4.5
	// Irwin Tyan: set this bit to boost IO performance at low power supply.
	if((REG_FCIE(reg_emmc_test)&BIT0) == 0) {
		//eMMC_debug(0, 0, "eMMC Err: not 1.8V IO setting\n");
		REG_FCIE_SETBIT(reg_emmc_test, BIT0);// 1.8V must set this bit
		REG_FCIE_CLRBIT(reg_nand_pad_driving, 0xfff);// 1.8V must set this bit
		REG_FCIE_SETBIT(reg_nand_pad_driving, 0xf);// 1.8V must set this bit
	}
    #else
	eMMC_debug(0,0,"3.3V IO power for eMMC\n");
	REG_FCIE_CLRBIT(reg_emmc_test, BIT0);// 3.3V must clear this bit
    #endif
	eMMC_pads_switch(FCIE_eMMC_SDR);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);
	
    // Set power good status de-glitch
    REG_FCIE_SETBIT(reg_pwrgd_int_glirm, BIT_PWRGD_INT_GLIRM_EN);

	return eMMC_ST_SUCCESS;
}

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

///////////////////////////////////////////////////////////////////////////////

#elif (defined(eMMC_DRV_NAPOLI_UBOOT)&&eMMC_DRV_NAPOLI_UBOOT)

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
		eMMC_die();
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
			eMMC_die();
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

	eMMC_hw_timer_delay(HW_TIMER_DELAY_100us); // asked by Irwin

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
			eMMC_die();
			return eMMC_ST_ERR_INVALID_PARAM; break;
	}
	
	if(u16_ClkParam & eMMC_PLL_FLAG) {
		//eMMC_debug(0,0,"%s(%Xh) EMMC PLL CLOCK\n", __FUNCTION__, u16_ClkParam);
		REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_FCIE_CLK_GATING|BIT_FCIE_CLK_INVERSE|BIT_CLKGEN_FCIE_MASK);
		REG_FCIE_SETBIT(reg_ckg_fcie, BIT_FCIE_CLK_SRC_SEL|BIT_FCIE_CLK_EMMC_PLL<<2);
		eMMC_pll_setting(u16_ClkParam);
	}
	else {
		//eMMC_debug(0,0,"%s(%Xh) CLKGEN CLOCK\n", __FUNCTION__, u16_ClkParam);
	    REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_FCIE_CLK_GATING|BIT_FCIE_CLK_INVERSE|BIT_CLKGEN_FCIE_MASK);
        REG_FCIE_SETBIT(reg_ckg_fcie, BIT_FCIE_CLK_SRC_SEL|u16_ClkParam<<2);
	}

	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;	
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
	BIT_FCIE_CLK_20M,
};

U32 eMMC_PlatformResetPre(void)
{
	//pull-high CMD pin
	REG_FCIE_SETBIT(PAD_CHIPTOP_BASE + (0x29 << 2), BIT13);

	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
    #if 1
	// for eMMC 4.5 HS200 need 1.8V, unify all eMMC IO power to 1.8V
	// works both for eMMC 4.4 & 4.5
	// eMMC_debug(0,0,"1.8V IO power for eMMC\n");
	// Irwin Tyan: set this bit to boost IO performance at low power supply.
	if((REG_FCIE(reg_emmc_test)&BIT0) == 0) {
		//eMMC_debug(0, 0, "eMMC Err: not 1.8V IO setting\n");
		REG_FCIE_SETBIT(reg_emmc_test, BIT0);// 1.8V must set this bit
		REG_FCIE_SETBIT(reg_emmc_test, BIT2); // atop patch
		REG_FCIE_CLRBIT(reg_nand_pad_driving, 0xffff);// 1.8V must set this bit
		REG_FCIE_SETBIT(reg_nand_pad_driving, 0xf);// 1.8V must set this bit
	}
    #else
	eMMC_debug(0,0,"3.3V IO power for eMMC\n");
	REG_FCIE_CLRBIT(reg_emmc_test, BIT0);// 3.3V must clear this bit
    #endif
	eMMC_pads_switch(FCIE_eMMC_SDR);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	// Set power good status de-glitch
    REG_FCIE_SETBIT(reg_pwrgd_int_glirm, BIT_PWRGD_INT_GLIRM_EN);

	return eMMC_ST_SUCCESS;
}

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
// ======================================================
void eMMC_dump_eMMCPLL(void)
{
	U16 u16_i, u16_reg;
	eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\n emmcpll RIU bank:");
	for(u16_i=0 ; u16_i<0x60; u16_i++)
	{
		if(0 == u16_i%8)
			eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n%02Xh:| ", u16_i);
	
		REG_FCIE_R(GET_REG_ADDR(EMMC_PLL_BASE, u16_i), u16_reg);
		eMMC_debug(eMMC_DEBUG_LEVEL, 0, "%04Xh ", u16_reg);
	}
}

// trigger level
#define Trig_lvl_0        ((0<<6)|(0<<3)|0)
#define Trig_lvl_1        ((0<<6)|(0<<3)|7)
#define Trig_lvl_2        ((0<<6)|(7<<3)|0)
#define Trig_lvl_3        ((0<<6)|(7<<3)|7)
#define Trig_lvl_4        ((7<<6)|(0<<3)|0)
#define Trig_lvl_5        ((7<<6)|(0<<3)|7)
#define Trig_lvl_6        ((7<<6)|(7<<3)|0)
#define Trig_lvl_7        ((7<<6)|(7<<3)|7)
#define Trig_lvl_MASK     Trig_lvl_7
#define reg_emmcpll_0x20  GET_REG_ADDR(EMMC_PLL_BASE, 0x20)

#define Trig_lvl_CNT      8
static  U16 sgau16_TrigLvl[Trig_lvl_CNT]=
  {Trig_lvl_0, Trig_lvl_1, Trig_lvl_2, Trig_lvl_3, 
   Trig_lvl_4, Trig_lvl_5, Trig_lvl_6, Trig_lvl_7};

// skew1
#define reg_emmcpll_0x1C  GET_REG_ADDR(EMMC_PLL_BASE, 0x1C)
#define emmcpll_dqs_patch BIT6 // switch skew1

#define IRWIN_TEST_FLAG_CMD21    1
#define IRWIN_TEST_FLAG_WRC      2

void eMMC_dump_Phase(U8 au8_Phase[PLL_SKEW4_CNT*2][PLL_SKEW4_CNT*2], U8 u8_TestFlag, U16 u16_Reg03h)
{
	U8 u8_i, u8_j;

	eMMC_debug(eMMC_DEBUG_LEVEL,0,"[Skew1\\Skew4] ");
	for(u8_i=PLL_SKEW4_CNT*2; u8_i > 0; u8_i--) // print Skew4 indices
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"%02u ", u8_i-1);
	eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n");

	for(u8_i=0; u8_i<PLL_SKEW4_CNT*2; u8_i++)
	{
		//--------------------------------
		// Read Phase has no need to change Skew1
		if(IRWIN_TEST_FLAG_CMD21 == u8_TestFlag)
		{
			if(u8_i != (u16_Reg03h&BIT_CLK_PH_MASK))
				continue;
		}
		//--------------------------------
		
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"  [%02u]        ", u8_i);
		for(u8_j=PLL_SKEW4_CNT*2; u8_j > 0; u8_j--)
			eMMC_debug(eMMC_DEBUG_LEVEL,0,"%2u ", au8_Phase[u8_i][u8_j-1]);
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n");
	}
}


U32 eMMC_FCIE_HS200_IrwinTest(void)
{	
	U8  au8_Phase[PLL_SKEW4_CNT*2][PLL_SKEW4_CNT*2], u8_Skew4Idx, u8_Skew1Idx, u8_TrigLvlIdx;
    U16 u16_reg;
	static U16 u16_Reg03h=0xFFFF, u16_Reg1Ch=0xFFFF;
	U32 u32_err;
	U8  u8_TestFlag=IRWIN_TEST_FLAG_CMD21;

	if(0xFFFF==u16_Reg03h && 0xFFFF==u16_Reg1Ch) // for original Skew1
	{
		REG_FCIE_R(reg_emmcpll_0x03, u16_Reg03h);
		REG_FCIE_R(reg_emmcpll_0x1C, u16_Reg1Ch);
		eMMC_debug(0,0,"Reg03h: %Xh, Reg1Ch: %Xh \n", u16_Reg03h, u16_Reg1Ch);
	}
	// ----------------------------------
	// dump emmcpll
	eMMC_dump_eMMCPLL();

	// ----------------------------------
	LABEL_TEST_START:
	eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n\n==================== \n");
	switch(u8_TestFlag)
	{
		case IRWIN_TEST_FLAG_CMD21:
			eMMC_debug(eMMC_DEBUG_LEVEL,0,"CMD21 Read Phase \n");
			break;
		case IRWIN_TEST_FLAG_WRC:
			eMMC_debug(eMMC_DEBUG_LEVEL,0,"Write Phase \n");
			break;			
	}	
	eMMC_debug(eMMC_DEBUG_LEVEL,0,"==================== \n");
	
	for(u8_TrigLvlIdx=0; u8_TrigLvlIdx<Trig_lvl_CNT; u8_TrigLvlIdx++)
	{
		REG_FCIE_CLRBIT(reg_emmcpll_0x20, Trig_lvl_MASK); 
		REG_FCIE_SETBIT(reg_emmcpll_0x20, sgau16_TrigLvl[u8_TrigLvlIdx]);
		REG_FCIE_R(reg_emmcpll_0x20, u16_reg);
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"\nTrigger Level: %u, Reg.20h: %Xh \n", u8_TrigLvlIdx, u16_reg);

		for(u8_Skew1Idx=0; u8_Skew1Idx<PLL_SKEW4_CNT*2; u8_Skew1Idx++)
		{
			//--------------------------------
		    // Read Phase has no need to change Skew1
		    if(IRWIN_TEST_FLAG_CMD21 == u8_TestFlag)
		    {
			    if(u8_Skew1Idx != (u16_Reg03h&BIT_CLK_PH_MASK))
				    continue;
		    }
		    
			for(u8_Skew4Idx=0; u8_Skew4Idx<PLL_SKEW4_CNT*2; u8_Skew4Idx++)
			{
				if(IRWIN_TEST_FLAG_CMD21 != u8_TestFlag)
				{if(u8_Skew1Idx<PLL_SKEW4_CNT)
			    {
			        REG_FCIE_CLRBIT(reg_emmcpll_0x1C, emmcpll_dqs_patch);
				    REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_CLK_PH_MASK);
		            REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_Skew1Idx);
			    }
			     else
			    {   REG_FCIE_SETBIT(reg_emmcpll_0x1C, emmcpll_dqs_patch); 
			        REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_CLK_PH_MASK);
				    REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_Skew1Idx-PLL_SKEW4_CNT);
			    }}
				
				if(u8_Skew4Idx<PLL_SKEW4_CNT)
			    {
			        REG_FCIE_CLRBIT(FCIE_SM_STS, BIT11);
				    REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		            REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_Skew4Idx<<12);
			    }
			    else
			    {   REG_FCIE_SETBIT(FCIE_SM_STS, BIT11);
			        REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
				    REG_FCIE_SETBIT(reg_emmcpll_0x03, (u8_Skew4Idx-PLL_SKEW4_CNT)<<12);
			    }                
				g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_TUNING_TTABLE;
				//--------------------------------
				switch(u8_TestFlag)
	            {
		            case IRWIN_TEST_FLAG_CMD21:
			            u32_err = eMMC_CMD21();  
						break;
		            case IRWIN_TEST_FLAG_WRC:
			            u32_err = eMMCTest_BlkWRC_ProbeTiming(eMMC_TEST_BLK_0);
			            break;			
	            }
				//--------------------------------
				g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_TUNING_TTABLE;
				if(eMMC_ST_SUCCESS == u32_err)
				{
					au8_Phase[u8_Skew1Idx][u8_Skew4Idx]=1;
					//REG_FCIE_R(reg_emmcpll_0x03, u16_reg);
		            //eMMC_debug(eMMC_DEBUG_LEVEL,0,"Reg.03h: %04Xh, ok \n", u16_reg);
				}
				else
				{   //REG_FCIE_R(reg_emmcpll_0x03, u16_reg);
		            //eMMC_debug(eMMC_DEBUG_LEVEL,0,"Reg.03h: %04Xh, fail \n", u16_reg);
					eMMC_FCIE_ResetToHS200((U8)g_eMMCDrv.u16_ClkRegVal);
					au8_Phase[u8_Skew1Idx][u8_Skew4Idx]=0;
				}
		    }
		}
		
		eMMC_dump_Phase(au8_Phase, u8_TestFlag, u16_Reg03h);		
	}

	// ----------------------------------
	switch(u8_TestFlag)
	{
		case IRWIN_TEST_FLAG_CMD21:
			u8_TestFlag = IRWIN_TEST_FLAG_WRC;
			goto LABEL_TEST_START;
			
		case IRWIN_TEST_FLAG_WRC:
			break;			
	}

	// restore
	REG_FCIE_W(reg_emmcpll_0x03, u16_Reg03h);
	REG_FCIE_W(reg_emmcpll_0x1C, u16_Reg1Ch);
	eMMC_FCIE_ResetToHS200((U8)g_eMMCDrv.u16_ClkRegVal);
	
	return eMMC_ST_SUCCESS;
}

#elif (defined(eMMC_DRV_KAISER_UBOOT)&&eMMC_DRV_KAISER_UBOOT)

void eMMC_DumpPadClk(void)
{
    //----------------------------------------------
    eMMC_debug(0, 0, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
    eMMC_debug(0, 0, "FCIE 1X (0x%X):0x%04X\n", reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X));
    eMMC_debug(0, 0, "FCIE 4X (0x%X):0x%04X\n", reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));
    eMMC_debug(0, 0, "MIU (0x%X):0x%04X\n", reg_ckg_MIU, REG_FCIE_U16(reg_ckg_MIU));
    eMMC_debug(0, 0, "MCU (0x%X):0x%04X\n", reg_ckg_MCU, REG_FCIE_U16(reg_ckg_MCU));

    //----------------------------------------------
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
			g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk = gau8_FCIEClkSel[u8_ClkIdx];

			// ---------------------------
		 	// select Window
			pWindow = NULL;

			// pick up the Window of Cell=0 case
			if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS == sg_DDRTWindow[1].aParam[1].u8_DQS)
			{
			   pWindow = &sg_DDRTWindow[0];
			}
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS == sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			{
			   pWindow = &sg_DDRTWindow[1];
			}
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			{
			   pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];
			}
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

			if(FCIE_SLOW_CLK == g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.TimingTable_t.u8_SetCnt = u8_SetIdx + 1;
			
			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT;
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
		    eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-eMMC_TIMING_TABLE_CHKSUM_OFFSET);
		g_eMMCDrv.TimingTable_t.u32_VerNo = eMMC_TIMING_TABLE_VERSION;
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



U32 eMMC_PlatformResetPre(void)
{
	//pull-high CMD pin
	REG_FCIE_SETBIT(PAD_CHIPTOP_BASE + (0x29 << 2), BIT13);

	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_eMMC_SDR);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	return eMMC_ST_SUCCESS;
}

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











#else
  #error "Error! no platform functions."
#endif
#endif
