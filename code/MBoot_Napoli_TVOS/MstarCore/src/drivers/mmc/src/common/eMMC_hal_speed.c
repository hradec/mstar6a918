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


// ==========================================================
U32 eMMC_FCIE_EnableSDRMode(void)
{
	U32 u32_ErrSpeed = eMMC_ST_SUCCESS, u32_ErrWidth = eMMC_ST_SUCCESS;

	// ----------------------------------------
	// may call from any other interface status
	if(eMMC_IF_NORMAL_SDR())
	{
	    u32_ErrSpeed = eMMC_SetBusSpeed(eMMC_SPEED_HIGH);
		u32_ErrWidth = eMMC_SetBusWidth(8, 0);
	}
	else if(DRV_FLAG_SPEED_HS200 == eMMC_SPEED_MODE())
	{
		u32_ErrSpeed = eMMC_SetBusSpeed(eMMC_SPEED_HIGH);
	}
	else if(DRV_FLAG_SPEED_HS400 == eMMC_SPEED_MODE())
	{
		u32_ErrWidth = eMMC_SetBusWidth(8, 0);
		u32_ErrSpeed = eMMC_SetBusSpeed(eMMC_SPEED_HIGH);		
	}

	if(eMMC_ST_SUCCESS!=u32_ErrSpeed || eMMC_ST_SUCCESS!=u32_ErrWidth)
	{
	    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh %Xh\n", u32_ErrSpeed, u32_ErrWidth);
	    return u32_ErrSpeed ? u32_ErrSpeed : u32_ErrWidth;
	}
	
	// ----------------------------------------
	// set to normal SDR 48MHz
	eMMC_pads_switch(FCIE_eMMC_SDR);
	eMMC_clock_setting(FCIE_DEFAULT_CLK);

	return eMMC_ST_SUCCESS;
}


// ===========================
#if !(defined(ENABLE_eMMC_ATOP) && ENABLE_eMMC_ATOP)
void eMMC_DumpDDR48TTable(void)
{
	U16 u16_i;

	eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n  eMMC DDR Timing Table: Cnt:%u CurIdx:%u \n",
		g_eMMCDrv.TimingTable_t.u8_SetCnt, g_eMMCDrv.TimingTable_t.u8_CurSetIdx);

	for(u16_i=0; u16_i<g_eMMCDrv.TimingTable_t.u8_SetCnt; u16_i++)
	    eMMC_debug(eMMC_DEBUG_LEVEL,0,"    Set:%u: clk:%02Xh, DQS:%02Xh, Cell:%02Xh \n",
			u16_i, g_eMMCDrv.TimingTable_t.Set[u16_i].u8_Clk,
			g_eMMCDrv.TimingTable_t.Set[u16_i].Param.u8_DQS,
			g_eMMCDrv.TimingTable_t.Set[u16_i].Param.u8_Cell);
}

void eMMC_FCIE_SetDDR48TimingReg(U8 u8_DQS, U8 u8_DelaySel)
{
	REG_FCIE_CLRBIT(FCIE_SM_STS, BIT_DQS_MODE_MASK|BIT_DQS_DELAY_CELL_MASK);

	REG_FCIE_SETBIT(FCIE_SM_STS, u8_DQS<<BIT_DQS_MDOE_SHIFT);
	REG_FCIE_SETBIT(FCIE_SM_STS, u8_DelaySel<<BIT_DQS_DELAY_CELL_SHIFT);
}

// ===========================
#else // DDR52 (ATOP)
void eMMC_DumpATopTable(void)
{
	U16 u16_i;

	eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n  eMMC ATop Timing Table: Ver:%Xh Cnt:%u CurIdx:%u \n",
		g_eMMCDrv.TimingTable_t.u32_VerNo,
		g_eMMCDrv.TimingTable_t.u8_SetCnt, g_eMMCDrv.TimingTable_t.u8_CurSetIdx);

	for(u16_i=0; u16_i<g_eMMCDrv.TimingTable_t.u8_SetCnt; u16_i++)
	    eMMC_debug(eMMC_DEBUG_LEVEL,0,"    Set:%u: Clk: %04Xh, Result: %08Xh, Reg2Ch: %02Xh, Skew4: %02Xh \n",
			u16_i, g_eMMCDrv.TimingTable_t.Set[u16_i].u8_Clk,
			g_eMMCDrv.TimingTable_t.Set[u16_i].u32_ScanResult,
			g_eMMCDrv.TimingTable_t.Set[u16_i].u8_Reg2Ch,
			g_eMMCDrv.TimingTable_t.Set[u16_i].u8_Skew4);
}

void eMMC_FCIE_SetATopTimingReg(U8 u8_SetIdx)
{
	if(g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Reg2Ch)
	    REG_FCIE_SETBIT(FCIE_SM_STS, BIT11);
	else
		REG_FCIE_CLRBIT(FCIE_SM_STS, BIT11);

	REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
	REG_FCIE_SETBIT(reg_emmcpll_0x03, 
		g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Skew4<<12);
}
#endif


// if failed, must be board issue.
U32 eMMC_FCIE_ChooseSpeedMode(void)
{
	U32 u32_err = eMMC_ST_SUCCESS;
	
	#if defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400	
	if(g_eMMCDrv.u8_ECSD196_DevType & eMMC_DEVTYPE_HS400_1_8V)
	{
		eMMC_debug(0,0,"\neMMC: HS400 %uMHz \n", g_eMMCDrv.u32_ClkKHz/1000);
		return u32_err;
	}
	#endif

	#if defined(ENABLE_eMMC_HS200) && ENABLE_eMMC_HS200	
	if(g_eMMCDrv.u8_ECSD196_DevType & eMMC_DEVTYPE_HS200_1_8V)
	{
		u32_err = eMMC_CMD13(g_eMMCDrv.u16_RCA);
	    if(eMMC_ST_SUCCESS != u32_err)
		    return u32_err;
	    if(eMMC_GetR1() & eMMC_R1_DEVICE_IS_LOCKED)
	    {
		    eMMC_debug(eMMC_DEBUG_LEVEL,1,"\neMMC Warn: HS200, but locked\n");
			return u32_err;
	    }
		
		if(eMMC_ST_SUCCESS != eMMC_FCIE_EnableFastMode(FCIE_eMMC_HS200) )
			u32_err = eMMC_FCIE_BuildHS200TimingTable();

		eMMC_debug(0,0,"\neMMC: HS200 %uMHz \n", g_eMMCDrv.u32_ClkKHz/1000);
		return u32_err;
	}
	#endif

	#if (defined(ENABLE_eMMC_ATOP)&&ENABLE_eMMC_ATOP) || (defined(IF_DETECT_eMMC_DDR_TIMING)&&IF_DETECT_eMMC_DDR_TIMING)
	if(g_eMMCDrv.u8_ECSD196_DevType & eMMC_DEVTYPE_DDR)
	{
		if(eMMC_ST_SUCCESS != eMMC_FCIE_EnableFastMode(FCIE_eMMC_DDR))
			u32_err = eMMC_FCIE_BuildDDRTimingTable();

		eMMC_debug(0,0,"\neMMC: DDR %uMHz \n", g_eMMCDrv.u32_ClkKHz/1000);
		return u32_err;
	}	
	#endif

	eMMC_debug(0,0,"\neMMC: SDR %uMHz\n", g_eMMCDrv.u32_ClkKHz/1000);
	return u32_err;
}


void eMMC_FCIE_ApplyTimingSet(U8 u8_Idx)
{
	// make sure a complete outside clock cycle
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); 
	
	#if !(defined(ENABLE_eMMC_ATOP) && ENABLE_eMMC_ATOP)
	// DDR48
	eMMC_clock_setting(g_eMMCDrv.TimingTable_t.Set[u8_Idx].u8_Clk);
	eMMC_FCIE_SetDDR48TimingReg(
		g_eMMCDrv.TimingTable_t.Set[u8_Idx].Param.u8_DQS,
		g_eMMCDrv.TimingTable_t.Set[u8_Idx].Param.u8_Cell);
	#else
    // HS400 or HS200 or DDR52
    eMMC_clock_setting(g_eMMCDrv.TimingTable_t.Set[u8_Idx].u8_Clk);
	eMMC_FCIE_SetATopTimingReg(u8_Idx);
	#endif
	g_eMMCDrv.TimingTable_t.u8_CurSetIdx = u8_Idx;
}


void eMMC_DumpTimingTable(void)
{
	#if !(defined(ENABLE_eMMC_ATOP) && ENABLE_eMMC_ATOP)
    eMMC_DumpDDR48TTable();
	#else
    eMMC_DumpATopTable();
	#endif
}


U32 eMMC_LoadTimingTable(void)
{
	U32 u32_err;
	U32 u32_ChkSum, u32_eMMCBlkAddr, u32_TryFlag=0;

    // --------------------------------------
    #if 0
	LABEL_TRY_TABLE:
	#endif
	#if defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400
	if(0==(u32_TryFlag & eMMC_DEVTYPE_HS400_1_8V))
	{
		u32_TryFlag |= eMMC_DEVTYPE_HS400_1_8V;
		u32_eMMCBlkAddr = eMMC_HS400TABLE_BLK_0;
		goto LABEL_READ_TABLE;
	}	
	#endif
	#if defined(ENABLE_eMMC_HS200) && ENABLE_eMMC_HS200
	if(0==(u32_TryFlag & eMMC_DEVTYPE_HS200_1_8V))
	{
		u32_TryFlag |= eMMC_DEVTYPE_HS200_1_8V;
		u32_eMMCBlkAddr = eMMC_HS200TABLE_BLK_0;
	    goto LABEL_READ_TABLE;
	}	
	#endif

	u32_TryFlag |= eMMC_DEVTYPE_DDR;
    u32_eMMCBlkAddr = eMMC_DDRTABLE_BLK_0;
	goto LABEL_READ_TABLE;
	
	// --------------------------------------
	LABEL_READ_TABLE:	
	u32_err = eMMC_CMD18(u32_eMMCBlkAddr, gau8_eMMC_SectorBuf, 1);
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: load 1 Table fail, %Xh\n", u32_err);

		u32_err = eMMC_CMD18(u32_eMMCBlkAddr+1, gau8_eMMC_SectorBuf, 1);
		if(eMMC_ST_SUCCESS != u32_err)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: load 2 Tables fail, %Xh\n", u32_err);
			goto LABEL_END_OF_NO_TABLE;
		}
	}

	// --------------------------------------
	memcpy((U8*)&g_eMMCDrv.TimingTable_t, gau8_eMMC_SectorBuf, sizeof(g_eMMCDrv.TimingTable_t));

	u32_ChkSum = eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-eMMC_TIMING_TABLE_CHKSUM_OFFSET);
	if(u32_ChkSum != g_eMMCDrv.TimingTable_t.u32_ChkSum)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: ChkSum error, no Table \n");
		u32_err = eMMC_ST_ERR_DDRT_CHKSUM;
		goto LABEL_END_OF_NO_TABLE;
	}

	if(0==u32_ChkSum && 
		0==memcmp(&g_eMMCDrv.TimingTable_t.Set[0], &g_eMMCDrv.TimingTable_t.Set[1], 2))
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: no Table \n");
		u32_err = eMMC_ST_ERR_DDRT_NONA;
		goto LABEL_END_OF_NO_TABLE;
	}

	// --------------------------------------
	// check if need auto-update
	if(eMMC_TIMING_TABLE_VERSION != g_eMMCDrv.TimingTable_t.u32_VerNo)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Warn: auto update Timng Table ... \n");
		u32_err = eMMC_ST_ERR_DDRT_NONA;
		goto LABEL_END_OF_NO_TABLE;
	}	

	return eMMC_ST_SUCCESS;

	LABEL_END_OF_NO_TABLE:
	g_eMMCDrv.TimingTable_t.u8_SetCnt = 0;

	#if 0
	if(eMMC_DEVTYPE_ALL != (u32_TryFlag & eMMC_DEVTYPE_ALL) )
		goto LABEL_TRY_TABLE;
    #endif
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: load Tables fail, %Xh\n", u32_err);
	return u32_err;

}


// set eMMC device & pad registers (no macro timing registers, since also involved in tuning procedure)
U32 eMMC_FCIE_EnableFastMode_Ex(U8 u8_PadType)
{
	U32 u32_err=eMMC_ST_SUCCESS;

	switch(u8_PadType)
	{
		case FCIE_eMMC_DDR:
			u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HIGH);
			if(eMMC_ST_SUCCESS!=u32_err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: enable HighSpeed fail: %Xh\n", u32_err);
				return u32_err;
			}
			u32_err = eMMC_SetBusWidth(8, 1);
			if(eMMC_ST_SUCCESS!=u32_err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: enable DDR fail: %Xh\n", u32_err);
				return u32_err;
			}

			if(0==g_eMMCDrv.TimingTable_t.u8_SetCnt)
				#if defined(ENABLE_eMMC_ATOP)&&ENABLE_eMMC_ATOP			
				eMMC_clock_setting(gau8_eMMCPLLSel_52[0]);
				#else
                eMMC_clock_setting(gau8_FCIEClkSel[0]);
				#endif

			break;

		#if defined(ENABLE_eMMC_HS200)&&ENABLE_eMMC_HS200
		case FCIE_eMMC_HS200:
			u32_err = eMMC_SetBusWidth(8, 0); // disable DDR
			if(eMMC_ST_SUCCESS!=u32_err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: HS200 disable DDR fail: %Xh\n", u32_err);
				return u32_err;
			}
			u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HS200);
			if(eMMC_ST_SUCCESS!=u32_err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: enable HS200 fail: %Xh\n", u32_err);
				return u32_err;
			}
			if(0==g_eMMCDrv.TimingTable_t.u8_SetCnt)
				eMMC_clock_setting(gau8_eMMCPLLSel_200[0]);
			
			break;
		#endif

		#if defined(ENABLE_eMMC_HS400)&&ENABLE_eMMC_HS400
		case FCIE_eMMC_HS400:
			while(1); // TODO
			break;
		#endif
	}

	// --------------------------------------
	if(eMMC_ST_SUCCESS != u32_err) 
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			"eMMC Err: set ECSD fail: %Xh\n", u32_err);
		return u32_err;
	}
	
	g_eMMCDrv.u16_Reg10_Mode &= ~BIT_SD_DATA_SYNC;

	if(g_eMMCDrv.TimingTable_t.u8_SetCnt)
		eMMC_FCIE_ApplyTimingSet(eMMC_TIMING_SET_MAX);
	eMMC_pads_switch(u8_PadType);
	
	return u32_err;
}


U32 eMMC_FCIE_EnableFastMode(U8 u8_PadType)
{
	U32 u32_err;

	if(u8_PadType == g_eMMCDrv.u8_PadType)
		return eMMC_ST_SUCCESS;

	u32_err = eMMC_LoadTimingTable();
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: no Timing Table, %Xh\n", u32_err);
		return u32_err;
	}

	#if defined(ENABLE_eMMC_ATOP) && ENABLE_eMMC_ATOP
	{
		U16 u16_OldClkRegVal = g_eMMCDrv.u16_ClkRegVal;
	    // --------------------------------
	    // ATOP: must init emmpll before pad
	    eMMC_clock_setting(g_eMMCDrv.TimingTable_t.Set[0].u8_Clk);
        // --------------------------------
        eMMC_clock_setting(u16_OldClkRegVal);
	}
	#endif
	
	u32_err = eMMC_FCIE_EnableFastMode_Ex(u8_PadType);
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableDDRMode_Ex fail, %Xh\n", u32_err);
		eMMC_die("");
		return u32_err;
	}

	return eMMC_ST_SUCCESS;
}



#if defined(ENABLE_eMMC_ATOP) && ENABLE_eMMC_ATOP
  
static const U8 hs200_tunning_pattern_128[128] = {
	0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 
	0xFF, 0xFF, 0xCC, 0xCC, 0xCC, 0x33, 0xCC, 0xCC,
    0xCC, 0x33, 0x33, 0xCC, 0xCC, 0xCC, 0xFF, 0xFF, 
    0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF,
    0xFF, 0xFF, 0xDD, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD, 
    0xFF, 0xFF, 0xFF, 0xBB, 0xFF, 0xFF, 0xFF, 0xBB,
    0xBB, 0xFF, 0xFF, 0xFF, 0x77, 0xFF, 0xFF, 0xFF, 
    0x77, 0x77, 0xFF, 0x77, 0xBB, 0xDD, 0xEE, 0xFF,
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0x00, 0xFF, 0xFF, 0xCC, 0xCC, 0xCC, 0x33, 0xCC,
    0xCC, 0xCC, 0x33, 0x33, 0xCC, 0xCC, 0xCC, 0xFF, 
    0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE,
    0xFF, 0xFF, 0xFF, 0xDD, 0xFF, 0xFF, 0xFF, 0xDD, 
    0xDD, 0xFF, 0xFF, 0xFF, 0xBB, 0xFF, 0xFF, 0xFF,
    0xBB, 0xBB, 0xFF, 0xFF, 0xFF, 0x77, 0xFF, 0xFF, 
    0xFF, 0x77, 0x77, 0xFF, 0x77, 0xBB, 0xDD, 0xEE
};

// read out from RIU then compare pattern is slow
U32 eMMC_CMD21_CIFD(void)
{
	U32 u32_err = 0;
	U16 u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN, u16_reg;
	U16 u16_mode;
	U8 i;

	//eMMC_debug(0,0,"%s()\n", __FUNCTION__);

	eMMC_FCIE_ClearEvents();

	if(g_eMMCDrv.u8_BUS_WIDTH == BIT_SD_DATA_WIDTH_4) {
		REG_FCIE_W(FCIE_SDIO_CTRL, BIT_SDIO_BLK_MODE | 64); // 64 bytes tuning pattern
	} else if(g_eMMCDrv.u8_BUS_WIDTH == BIT_SD_DATA_WIDTH_8) {
		REG_FCIE_W(FCIE_SDIO_CTRL, BIT_SDIO_BLK_MODE | 128); // 128 bytes tuning pattern
	} else {
		u32_err = eMMC_ST_ERR_CMD21_ONE_BIT;
		eMMC_debug(0,0,"eMMC Warn: g_eMMCDrv.u8_BUS_WIDTH = %02Xh\n", g_eMMCDrv.u8_BUS_WIDTH);
		goto ErrorHandle;
	}

	u16_mode = BIT_SD_DATA_CIFD | g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;

	u32_err = eMMC_FCIE_SendCmd(u16_mode, u16_ctrl, 0, 21, eMMC_R1_BYTE_CNT);
	if(eMMC_ST_SUCCESS != u32_err) {
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: CMD55 send CMD fail: %08Xh\n", u32_err);
		goto ErrorHandle;
	}

	u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_SD_DATA_END, TIME_WAIT_1_BLK_END);
	if(u32_err) 
		goto ErrorHandle;
	
	// check status
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

	if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR)) {

		u32_err = eMMC_ST_ERR_CMD21_NO_RSP;
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: CMD21 no Rsp, Reg.12: %04Xh \n", u16_reg);
		goto ErrorHandle;

	} else if(u16_reg &BIT_SD_R_CRC_ERR) {

		u32_err = eMMC_ST_ERR_CMD21_DATA_CRC;
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: CMD21 data CRC err, Reg.12: %04Xh \n", u16_reg);
		goto ErrorHandle;

	} else {
		for(i=0; i<128; i++) 
			if(hs200_tunning_pattern_128[i]!=eMMC_FCIE_DataFifoGet(i)) 
				break;
		
		if(i!=128) { // error
			eMMC_debug(0,0,"got pattern:");
			for(i=0; i<128; i++) {
				if(i%16==0) eMMC_debug(0,0,"\n\t");
				eMMC_debug(0,0,"%02X ", eMMC_FCIE_DataFifoGet(i));
			}
			eMMC_debug(0,0,"\n");
			u32_err = eMMC_ST_ERR_CMD21_DATA_CMP;
			goto ErrorHandle;
		}
	}

    ErrorHandle:
	REG_FCIE_W(FCIE_SDIO_CTRL, BIT_SDIO_BLK_MODE | eMMC_SECTOR_512BYTE); // restore anyway...

	if(u32_err)
	    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: %08Xh\n", u32_err);
	
	return u32_err;

}

// use memcmp to confirm tuning pattern
U32 eMMC_CMD21_MIU(void)
{
	U32 u32_err = 0;
	U16 u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN, u16_reg;
	U16 u16_mode;
	U8 i;
	U32 u32_dma_addr;

	REG_FCIE_W(FCIE_TOGGLE_CNT, TOGGLE_CNT_128_CLK_R);
	REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
	REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
	eMMC_hw_timer_delay(TIME_WAIT_FCIE_RST_TOGGLE_CNT); // Brian needs 2T
	REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);

	eMMC_FCIE_ClearEvents();

	if(g_eMMCDrv.u8_BUS_WIDTH == BIT_SD_DATA_WIDTH_4) {
		REG_FCIE_W(FCIE_SDIO_CTRL, BIT_SDIO_BLK_MODE | 64); // 64 bytes tuning pattern
	} else if(g_eMMCDrv.u8_BUS_WIDTH == BIT_SD_DATA_WIDTH_8) {
		REG_FCIE_W(FCIE_SDIO_CTRL, BIT_SDIO_BLK_MODE | 128); // 128 bytes tuning pattern
	} else {
		u32_err = eMMC_ST_ERR_CMD21_ONE_BIT;
		eMMC_debug(0,0,"eMMC Warn: g_eMMCDrv.u8_BUS_WIDTH = %02Xh\n", g_eMMCDrv.u8_BUS_WIDTH);
		goto ErrorHandle;
	}

	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	u32_dma_addr = eMMC_translate_DMA_address_Ex((U32)gau8_eMMC_SectorBuf, eMMC_SECTOR_512BYTE);
    #if FICE_BYTE_MODE_ENABLE
	REG_FCIE_W(FCIE_SDIO_ADDR0, u32_dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_SDIO_ADDR1, u32_dma_addr >> 16);
    #else
	REG_FCIE_W(FCIE_MIU_DMA_15_0, (u32_dma_addr>>MIU_BUS_WIDTH_BITS)&0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_26_16,(u32_dma_addr>>MIU_BUS_WIDTH_BITS)>>16);
    #endif
	REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_DMA_DIR_W);
	u32_err = eMMC_FCIE_FifoClkRdy(0);
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CMD21 wait FIFOClk fail: %08Xh\n", u32_err);
		eMMC_FCIE_ErrHandler_Stop();
		goto ErrorHandle;
	}
	REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);
	
    
	u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
	u32_err = eMMC_FCIE_SendCmd(u16_mode, u16_ctrl, 0, 21, eMMC_R1_BYTE_CNT);
	if(eMMC_ST_SUCCESS != u32_err) 
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD21 send CMD fail: %08Xh\n", u32_err);
		goto ErrorHandle;
	}
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR)) 
	{
		u32_err = eMMC_ST_ERR_CMD21_NO_RSP;
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD21 no Rsp, Reg.12: %04Xh \n", u16_reg);
		goto ErrorHandle;
	}

	u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_MIU_LAST_DONE|BIT_CARD_DMA_END, HW_TIMER_DELAY_100ms); // edit for MIU
	if(u32_err)
		goto ErrorHandle;
	
	// check status
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if(u16_reg &BIT_SD_R_CRC_ERR) {

		u32_err = eMMC_ST_ERR_CMD21_DATA_CRC;
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD21 data CRC err, Reg.12: %04Xh \n", u16_reg);
		goto ErrorHandle;

	} else {
		if(memcmp((void*)hs200_tunning_pattern_128, (void*)gau8_eMMC_SectorBuf, 128)) 
		{
			eMMC_debug(0,0,"got pattern:");
			for(i=0; i<128; i++) {
				if(i%16==0) eMMC_debug(0,0,"\n\t");
				eMMC_debug(0,0,"%02X ", gau8_eMMC_SectorBuf[i]);
			}
			eMMC_debug(0,0,"\n");
			u32_err = eMMC_ST_ERR_CMD21_DATA_CMP;
			goto ErrorHandle;
		}
	}

    ErrorHandle:
	REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
	REG_FCIE_W(FCIE_SDIO_CTRL, BIT_SDIO_BLK_MODE | eMMC_SECTOR_512BYTE); // restore anyway...
	if(u32_err)
	    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: %08Xh\n", u32_err);

	return u32_err;
}

// eMMC CMD21 adtc, R1, fix 128 clock, for HS200 only
//  64 bytes in 4 bits mode
// 128 bytes in 8 bits mode

U32 eMMC_CMD21(void) // send tuning block
{
    #if defined(ENABLE_eMMC_RIU_MODE)&&ENABLE_eMMC_RIU_MODE
	return eMMC_CMD21_CIFD(); // slow
    #else
	return eMMC_CMD21_MIU(); // fast
    #endif
}

#endif// ENABLE_eMMC_ATOP


