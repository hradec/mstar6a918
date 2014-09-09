#include <linux/device.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/backing-dev.h>
#include <linux/blkpg.h>
#include <linux/vmalloc.h>
#include <linux/cdev.h>
#include <linux/workqueue.h>
#include <linux/mtd/mtd.h>

#define NAND_USE_UNFD					1

#if defined(NAND_USE_UNFD) && NAND_USE_UNFD
#include "inc/common/drvNAND.h"
//#include "mdrv_nand_io.h"

#define MSTAR_NAND_CHAR_MAJOR			283
#define MSTAR_CDEV_NAND_NAME			"nand"
#define SBOOT_SIZE						0x10000
#ifndef HASH0_SIZE
#define HASH0_SIZE						0x2800 // default hash0 size is 10K unless defined in platform-dependent .h
#endif

extern struct semaphore					PfModeSem;
extern void nand_lock_fcie(void);
extern void nand_unlock_fcie(void);

char * vbuf;
char * cur_buf;
int fps;
U8 au8_MainBuf[16384] UNFD_ALIGN1;
U8 au8_SpareBuf[1280] UNFD_ALIGN1;

struct cdev cDevice;

//========================================check Read disturbance========================================
static void drvNAND_CheckReadDisturbance(struct work_struct *work)
{
	NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
	U8	u8_RecoveryFlag, u8_RequireRandomizer = 0, u8_HashIdx;
	U8	*pu8_BlockBuffer = NULL;
	U16 u16_PageIdx, u16_BlkPageCnt;
	U32 u32_Row, u32_Err, u32_BlkIdx, *pu32_V2PBlkMap = NULL, u32_BakBlkIdx;
	U32 u32_VBlkIdx;
	int s32_ECCCount;

	//nand_debug(UNFD_DEBUG_LEVEL, 1, "Begin\n");
	if(pNandDrv->u8_HasPNI == 0)
		return;

	nand_lock_fcie();

	if(pNandDrv->u8_CellType)
		u16_BlkPageCnt = pNandDrv->u16_BlkPageCnt >> 1;
	else
		u16_BlkPageCnt = pNandDrv->u16_BlkPageCnt;

	//check whether cis need to read disturbance storage error bit when search cis in initialization

	//check hash or BL
	//Disable Randomizer
	#if defined(FCIE_LFSR) && FCIE_LFSR
	if(pNandDrv->u8_RequireRandomizer)
	{
		NC_DisableLFSR();
		u8_RequireRandomizer = pNandDrv->u8_RequireRandomizer;
		pNandDrv->u8_RequireRandomizer = 0;
	}
	#endif

	u8_RecoveryFlag = 0;
	//read hash0,1/BL
	if(pNandDrv->u8_BL0PBA != 0 && pNandDrv->u8_BL1PBA != 0)
	{
		// eagle's rom code does not identify blank page when reading bootloader. Disable read disturbance handling in BL
		#if !(defined(CONFIG_MSTAR_EAGLE) || defined(CONFIG_MSTAR_EMERALD) || defined(CONFIG_MSTAR_EDISON))
		//read first BL for read disturbance and backup from backup bootloader
		//trick: BL context is only stored in a block

		for(u16_PageIdx = 0; u16_PageIdx < u16_BlkPageCnt; u16_PageIdx ++)
		{
			u32_Row = (pNandDrv->u8_BL0PBA << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u16_PageIdx].u16_LSB;
			u32_Err = NC_ReadPages(u32_Row, au8_MainBuf, au8_SpareBuf, 1);
			if(u32_Err == UNFD_ST_SUCCESS)
			{
				NC_CheckECC(&s32_ECCCount);
				if(s32_ECCCount != 0)
				{
					//ecc bit is larger than the threshold need to recovery.
					u8_RecoveryFlag = 1;
					break;
				}
			}
		}
		#if 0
		if(u8_RecoveryFlag == 1)
		{
			pu8_BlockBuffer = vmalloc(1 << (pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageByteCntBits));
			if(pu8_BlockBuffer == NULL)
			{
				nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Can't alloc memory for Block Buffer\n");
				goto ERROR;
			}

			//read all context from backup BL and write to original BL
			//only one block context	need full block program ??? or SLC mode???
			for(u16_PageIdx = 0; u16_PageIdx < u16_BlkPageCnt; u16_PageIdx ++)
			{
				u32_Row = (pNandDrv->u8_BL1PBA << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u16_PageIdx].u16_LSB;
				u32_Err = NC_ReadPages(u32_Row, au8_MainBuf, au8_SpareBuf, 1);
				if(u32_Err != UNFD_ST_SUCCESS)
				{
					//read backup BL fail or need recovery ?
					nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Read Backup BL Error\n");
					goto ERROR;
				}
				memcpy(pu8_BlockBuffer + (u16_PageIdx << pNandDrv->u8_PageByteCntBits), au8_MainBuf, pNandDrv->u16_PageByteCnt);
			}

			u32_Err = NC_EraseBlk(pNandDrv->u8_BL0PBA << pNandDrv->u8_BlkPageCntBits);
			if(u32_Err != UNFD_ST_SUCCESS)
			{
				nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Erase BL Error\n");
				drvNAND_MarkBadBlk(pNandDrv->u8_BL0PBA);		//need mark bad?
				goto ERROR;
			}

			au8_SpareBuf[1] = 0x00;
			for(u16_PageIdx = 0; u16_PageIdx < u16_BlkPageCnt; u16_PageIdx ++)
			{
				memcpy(au8_MainBuf, pu8_BlockBuffer + (u16_PageIdx << pNandDrv->u8_PageByteCntBits), pNandDrv->u16_PageByteCnt);
				u32_Row = (pNandDrv->u8_BL0PBA << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u16_PageIdx].u16_LSB;
				u32_Err = NC_WritePages(u32_Row, au8_MainBuf, au8_SpareBuf, 1);
				if(u32_Err != UNFD_ST_SUCCESS)
				{
					//write original BL fail
					nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,"Write BL Error\n");
					drvNAND_MarkBadBlk(pNandDrv->u8_BL0PBA);
					goto ERROR;
				}
			}
			if(pu8_BlockBuffer)
			{
				vfree(pu8_BlockBuffer);
				pu8_BlockBuffer = NULL;
			}
		}
		#else
		if(u8_RecoveryFlag == 1)
		{
			U8 u8_Marker;
			u8_Marker = 0x00;
			nand_ReadDisturbance_BigImg(pNandDrv->u8_BL0PBA << pNandDrv->u8_BlkPageCntBits, pNandDrv->u8_BL1PBA << pNandDrv->u8_BlkPageCntBits, NULL, u8_Marker, 1);
		}
		#endif

		#endif
		//===============================================================================
		//									check uboot
		//===============================================================================
		if(pNandDrv->u8_UBOOTPBA == 0)
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Fatal Error, UBOOTPBA should not be 0\n");
			goto ERROR;
		}
		else
		{
			pu32_V2PBlkMap = (U32*)vmalloc(sizeof(U32) * (pNandDrv->u8_UBOOTPBA - pNandDrv->u8_BL1PBA - 1));
			if(pu32_V2PBlkMap == NULL)
			{
				nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Can't alloc memory for Uboot bad block mapping\n");
				goto ERROR;
			}
			//find backup UBOOT good block
			for(u32_BlkIdx = 0, u32_VBlkIdx = 0; u32_VBlkIdx < (pNandDrv->u8_UBOOTPBA - pNandDrv->u8_BL1PBA - 1);)
			{
				if(drvNAND_IsGoodBlk(u32_BlkIdx + pNandDrv->u8_UBOOTPBA))
				{
					pu32_V2PBlkMap[u32_VBlkIdx] = u32_BlkIdx;
					u32_VBlkIdx ++;
				}
				u32_BlkIdx ++;
			}

			//read first UBOOT
			//for each block between BL1 and UBOOTPBA
			u32_VBlkIdx = 0;
			for(u32_BlkIdx = pNandDrv->u8_BL1PBA + 1, u32_BakBlkIdx = pNandDrv->u8_UBOOTPBA;
				u32_BlkIdx < pNandDrv->u8_UBOOTPBA;
				u32_BlkIdx ++, u32_BakBlkIdx++)
			{
				u8_RecoveryFlag = 0;
			#if 0
				if(!drvNAND_IsGoodBlk(u32_BlkIdx))
					continue;
			#else
				while(!drvNAND_IsGoodBlk(u32_BlkIdx))
				{
					u32_BlkIdx++;
					if(u32_BlkIdx == pNandDrv->u8_UBOOTPBA)
						goto ERROR;
				}
			#endif
				while(!drvNAND_IsGoodBlk(u32_BakBlkIdx))
					u32_BakBlkIdx++;

				for(u16_PageIdx = 0; u16_PageIdx < u16_BlkPageCnt; u16_PageIdx ++)
				{
					u32_Row = (u32_BlkIdx << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u16_PageIdx].u16_LSB;
					u32_Err = NC_ReadPages(u32_Row, au8_MainBuf, au8_SpareBuf, 1);
				//	nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1,"ReadPage %lX\n",u32_Row);
					if(u32_Err == UNFD_ST_SUCCESS)
					{
						NC_CheckECC(&s32_ECCCount);
						if(s32_ECCCount != 0)
						{
							//ecc bit is larger than the threshold need to recovery.
							u8_RecoveryFlag = 1;
							break;
						}
					}
				}
				#if 0
				if(u8_RecoveryFlag == 1)
				{
					pu8_BlockBuffer = vmalloc(1 <<(pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageByteCntBits));
					if(pu8_BlockBuffer == NULL)
					{
						nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Can't alloc memory for Block Buffer\n");
						pNandDrv->u8_RequireRandomizer = u8_RequireRandomizer;
						goto ERROR;
					}

					//One block context	need full block program ??? or SLC mode???
					for(u16_PageIdx = 0; u16_PageIdx < u16_BlkPageCnt; u16_PageIdx ++)
					{
						u32_Row = ((pNandDrv->u8_UBOOTPBA + pu32_V2PBlkMap[u32_VBlkIdx]) << pNandDrv->u8_BlkPageCntBits)
										+ ga_tPairedPageMap[u16_PageIdx].u16_LSB;
						u32_Err = NC_ReadPages(u32_Row, au8_MainBuf, au8_SpareBuf, 1);
				//		nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "ReadPage %lX\n", u32_Row);
						if(u32_Err != UNFD_ST_SUCCESS)
						{
							//read backup UBOOT fail or need recovery ?
							nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Read Backup UBOOT Error\n");
							goto ERROR;
						}
						memcpy(pu8_BlockBuffer + u16_PageIdx * pNandDrv->u16_PageByteCnt, au8_MainBuf, pNandDrv->u16_PageByteCnt);
					}
					//u32_Err = UNFD_ST_SUCCESS;
				//	nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "EraseBlk %lX\n", u32_BlkIdx << pNandDrv->u8_BlkPageCntBits);
					u32_Err = NC_EraseBlk(u32_BlkIdx << pNandDrv->u8_BlkPageCntBits);
					if(u32_Err != UNFD_ST_SUCCESS)
					{
						nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Erase UBOOT Error\n");
						drvNAND_MarkBadBlk(u32_BlkIdx);		//need mark bad?
						goto ERROR;
					}

					for(u16_PageIdx = 0; u16_PageIdx < u16_BlkPageCnt; u16_PageIdx ++)
					{
						memcpy(au8_MainBuf, pu8_BlockBuffer + u16_PageIdx * pNandDrv->u16_PageByteCnt, pNandDrv->u16_PageByteCnt);
						u32_Row = (u32_BlkIdx << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u16_PageIdx].u16_LSB;
						u32_Err = NC_WritePages(u32_Row, au8_MainBuf, au8_SpareBuf, 1);
				//		nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "WritePage %lX\n", u32_Row);
						//u32_Err = UNFD_ST_SUCCESS;
						if(u32_Err != UNFD_ST_SUCCESS)
						{
							//write original UBOOT fail
							nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,"Write UBOOT Error\n");
							drvNAND_MarkBadBlk(u32_BlkIdx);
							goto ERROR;
						}
					}
					if(pu8_BlockBuffer)
					{
						vfree(pu8_BlockBuffer);
						pu8_BlockBuffer = NULL;
					}
				}
				#else
				if(u8_RecoveryFlag == 1)
				{
					U8 u8_Marker;
					u8_Marker = 0xFF;
					nand_ReadDisturbance_BigImg(u32_BlkIdx << pNandDrv->u8_BlkPageCntBits, u32_BakBlkIdx << pNandDrv->u8_BlkPageCntBits, NULL, u8_Marker, 1);
				}
				#endif
				u32_VBlkIdx ++;
			}
			if(pu32_V2PBlkMap)
			{
				vfree(pu32_V2PBlkMap);
				pu32_V2PBlkMap = NULL;
			}
		}

	}
	else if(pNandDrv->u8_HashPBA[0][0] != 0 && pNandDrv->u8_HashPBA[0][1] != 0)
	{
		for(u8_HashIdx = 0; u8_HashIdx < 3; u8_HashIdx ++)
		{
			pu32_V2PBlkMap = (U32*)vmalloc(sizeof(U32) * (pNandDrv->u8_HashPBA[u8_HashIdx][1] - pNandDrv->u8_HashPBA[u8_HashIdx][0]));
			if(pu32_V2PBlkMap == NULL)
			{
				nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Can't alloc memory for Uboot bad block mapping\n");
				goto ERROR;
			}
			//find good block for backup hash
			for(u32_BlkIdx = 0, u32_VBlkIdx = 0; u32_VBlkIdx < (pNandDrv->u8_HashPBA[u8_HashIdx][1] - pNandDrv->u8_HashPBA[u8_HashIdx][0]);)
			{
				if(drvNAND_IsGoodBlk(u32_BlkIdx + pNandDrv->u8_HashPBA[u8_HashIdx][1]))
				{
					pu32_V2PBlkMap[u32_VBlkIdx] = u32_BlkIdx;
					u32_VBlkIdx ++;
				}
				u32_BlkIdx ++;
			}

			//read first hash
			//for each block between hash and backup hash
			u32_VBlkIdx = 0;
			for(u32_BlkIdx = pNandDrv->u8_HashPBA[u8_HashIdx][0], u32_BakBlkIdx = pNandDrv->u8_HashPBA[u8_HashIdx][1];
				u32_BlkIdx < pNandDrv->u8_HashPBA[u8_HashIdx][1]; 
				u32_BlkIdx ++, u32_BakBlkIdx++)
			{
				u8_RecoveryFlag = 0;
			#if 0	
				if(!drvNAND_IsGoodBlk(u32_BlkIdx))
					continue;
			#else
				while(!drvNAND_IsGoodBlk(u32_BlkIdx))
				{
					u32_BlkIdx++;
					if(u32_BlkIdx == pNandDrv->u8_HashPBA[u8_HashIdx][1])
						goto ERROR;
				}
			#endif	
				while(!drvNAND_IsGoodBlk(u32_BakBlkIdx))
					u32_BakBlkIdx++;

				for(u16_PageIdx = 0; u16_PageIdx < u16_BlkPageCnt; u16_PageIdx ++)
				{
					u32_Row = (u32_BlkIdx << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u16_PageIdx].u16_LSB;
					u32_Err = NC_ReadPages(u32_Row, au8_MainBuf, au8_SpareBuf, 1);
			//		nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1,"ReadPage %lX\n",u32_Row);
					if(u32_Err == UNFD_ST_SUCCESS)
					{
						NC_CheckECC(&s32_ECCCount);
						if(s32_ECCCount != 0)
						{
							//ecc bit is larger than the threshold need to recovery.
							u8_RecoveryFlag = 1;
							break;
						}
					}
				}
				#if 0
				//Recovery from backup hash
				if(u8_RecoveryFlag == 1)
				{
					pu8_BlockBuffer = vmalloc(1 <<(pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageByteCntBits));
					if(pu8_BlockBuffer == NULL)
					{
						nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Can't alloc memory for Block Buffer\n");
						pNandDrv->u8_RequireRandomizer = u8_RequireRandomizer;
						goto ERROR;
					}

					//One block context	need full block program ??? or SLC mode???
					for(u16_PageIdx = 0; u16_PageIdx < u16_BlkPageCnt; u16_PageIdx ++)
					{
						u32_Row = ((pNandDrv->u8_HashPBA[u8_HashIdx][1] + pu32_V2PBlkMap[u32_VBlkIdx]) << pNandDrv->u8_BlkPageCntBits)
										+ ga_tPairedPageMap[u16_PageIdx].u16_LSB;
						u32_Err = NC_ReadPages(u32_Row, au8_MainBuf, au8_SpareBuf, 1);
						//nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "ReadPage %lX\n", u32_Row);
						if(u32_Err != UNFD_ST_SUCCESS)
						{
							//read backup Hash fail or need recovery ?
							nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Read Backup Hash %d Error\n", u8_HashIdx);
							goto ERROR;
						}
						memcpy(pu8_BlockBuffer + u16_PageIdx * pNandDrv->u16_PageByteCnt, au8_MainBuf, pNandDrv->u16_PageByteCnt);
					}
					//nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "EraseBlk %lX\n", u32_BlkIdx << pNandDrv->u8_BlkPageCntBits);
					u32_Err = NC_EraseBlk(u32_BlkIdx << pNandDrv->u8_BlkPageCntBits);
					if(u32_Err != UNFD_ST_SUCCESS)
					{
						nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Erase Hash %d Error\n", u8_HashIdx);
						drvNAND_MarkBadBlk(u32_BlkIdx);		//need mark bad?
						goto ERROR;
					}

					if(u32_VBlkIdx == 0)
						au8_SpareBuf[1] = (0x80|u8_HashIdx);
					else
						au8_SpareBuf[1] = 0xFF;

					for(u16_PageIdx = 0; u16_PageIdx < u16_BlkPageCnt; u16_PageIdx ++)
					{
						memcpy(au8_MainBuf, pu8_BlockBuffer + u16_PageIdx * pNandDrv->u16_PageByteCnt, pNandDrv->u16_PageByteCnt);
						u32_Row = (u32_BlkIdx << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u16_PageIdx].u16_LSB;
						u32_Err = NC_WritePages(u32_Row, au8_MainBuf, au8_SpareBuf, 1);
						//nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "WritePage %lX\n", u32_Row);
						if(u32_Err != UNFD_ST_SUCCESS)
						{
							//write original Hash fail
							nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,"Write Hash %d Error\n", u8_HashIdx);
							drvNAND_MarkBadBlk(u32_BlkIdx);
							goto ERROR;
						}
					}
					if(pu8_BlockBuffer)
					{
						vfree(pu8_BlockBuffer);
						pu8_BlockBuffer = NULL;
					}
				}
				#else
				if(u8_RecoveryFlag == 1)
				{
					U8 u8_Marker;
					u8_Marker = (u32_VBlkIdx == 0) ? (0x80|u8_HashIdx) : 0xFF;
					nand_ReadDisturbance_BigImg(u32_BlkIdx << pNandDrv->u8_BlkPageCntBits, u32_BakBlkIdx << pNandDrv->u8_BlkPageCntBits, NULL, u8_Marker, 1);
				}
				#endif
				u32_VBlkIdx ++;
			}
			if(pu32_V2PBlkMap)
			{
				vfree(pu32_V2PBlkMap);
				pu32_V2PBlkMap = NULL;
			}
		}
	}

	//read kernel image need to parse mtd partition table ?
	//impossible nand driver don't know which partition is the kernel partition.
	//done in mboot
	// mtd_info can't get partition's offset > can't access kernel partition in kernel

	pNandDrv->u8_RequireRandomizer = u8_RequireRandomizer;
	nand_unlock_fcie();
	return;

ERROR:
	if(pu8_BlockBuffer != NULL)
		vfree(pu8_BlockBuffer);
	if(pu32_V2PBlkMap != NULL)
		vfree(pu32_V2PBlkMap);

	pNandDrv->u8_RequireRandomizer = u8_RequireRandomizer;
	nand_unlock_fcie();
	return;
}

static DECLARE_DELAYED_WORK(rd_work, drvNAND_CheckReadDisturbance);

//======================================================================================================
U32 drvNAND_WriteCIS_for_ROM(NAND_FLASH_INFO_t * pNandInfo)
{
	NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
	PARTITION_INFO_t *pPartInfo = drvNAND_get_DrvContext_PartInfo();
	U8 *au8_PageBuf = (U8*)pNandDrv->PlatCtx_t.pu8_PageDataBuf;
	U8 *au8_SpareBuf = (U8*)pNandDrv->PlatCtx_t.pu8_PageSpareBuf;
	BLK_INFO_t *pBlkInfo = (BLK_INFO_t*)au8_SpareBuf;
	U32 u32_Err = UNFD_ST_SUCCESS;
	U16 u16_PBA;
	U32 u32_PageIdx;
	U8 u8_CISIdx;

	NC_ConfigContext();
	NC_ReInit();
	NC_Config();

	nand_config_clock(pNandInfo->u16_tRC);

	#if defined(FCIE4_DDR) && FCIE4_DDR
	memcpy((void *) &pNandInfo->tDefaultDDR, (const void *) &pNandDrv->tDefaultDDR, sizeof(DDR_TIMING_GROUP_t));
	memcpy((void *) &pNandInfo->tMaxDDR, (const void *) &pNandDrv->tMaxDDR, sizeof(DDR_TIMING_GROUP_t));
	memcpy((void *) &pNandInfo->tMinDDR, (const void *) &pNandDrv->tMinDDR, sizeof(DDR_TIMING_GROUP_t));
	#endif

	u8_CISIdx = 0;

    /* Search for two good blocks within the first 10 physical blocks */
	for (u16_PBA = 0; u16_PBA < 10; u16_PBA++) {

		/* Reeset NAND driver and FCIE to the original settings */
		pNandDrv->u16_SpareByteCnt = pNandInfo->u16_SpareByteCnt;
		pNandDrv->u16_PageByteCnt  = pNandInfo->u16_PageByteCnt;
		pNandDrv->u16_ECCType      = pNandInfo->u16_ECCType;
		NC_ConfigContext();
		NC_ReInit();
		pNandDrv->u16_Reg48_Spare &= ~(1 << 12);
		//Disable Randomizer for nni Read/Write
		#if defined(FCIE_LFSR) && FCIE_LFSR
		if(pNandDrv->u8_RequireRandomizer)
			NC_DisableLFSR();
		#endif
		NC_Config();

		/* Check first page of block */
		u32_PageIdx = u16_PBA << pNandDrv->u8_BlkPageCntBits;
		u32_Err = NC_ReadSectors(u32_PageIdx, 0, au8_PageBuf, au8_SpareBuf, 1);
		if (u32_Err != UNFD_ST_SUCCESS)
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "NC_ReadSectors(0x%lX)=0x%lX\n", u32_PageIdx, u32_Err);
		if (au8_SpareBuf[0] != 0xFF)
		{
			nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "Skip bad blk 0x%04x\n", u16_PBA);
			continue;
		}


		u32_Err = NC_EraseBlk(u16_PBA << pNandDrv->u8_BlkPageCntBits);
		if (u32_Err != UNFD_ST_SUCCESS) {
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Erase blk 0x%04x failed with EC: 0x%08lx\n",
				u16_PBA, u32_Err);

			drvNAND_MarkBadBlk(u16_PBA);
			continue;
		}

		pNandDrv->u16_PageByteCnt = 2048;
		pNandDrv->u16_SpareByteCnt = 256;
		pNandDrv->u16_ECCType = NANDINFO_ECC_TYPE;

		NC_ConfigContext();
		NC_ReInit();

		pNandDrv->u16_Reg48_Spare |= (1 << 12);
		NC_Config();

		memset(au8_PageBuf, '\0', pNandDrv->u16_PageByteCnt);
		memcpy(au8_PageBuf, pNandInfo, 512);
		memset(au8_SpareBuf, 0xFF, pNandDrv->u16_SpareByteCnt);

		pBlkInfo->u8_BadBlkMark = 0xFF;
		pBlkInfo->u8_PartType = 0;

		u32_Err = NC_WriteSectors(u32_PageIdx, 0, au8_PageBuf, au8_SpareBuf, 1);

		if (u32_Err != UNFD_ST_SUCCESS)
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Write Nand Info failed with EC: 0x%08lx\n", u32_Err);
			drvNAND_MarkBadBlk(u16_PBA);
			continue;
		}

		/* Reset NAND driver and FCIE to the original settings */
		pNandDrv->u16_SpareByteCnt = pNandInfo->u16_SpareByteCnt;
		pNandDrv->u16_PageByteCnt  = pNandInfo->u16_PageByteCnt;
		pNandDrv->u16_ECCType      = pNandInfo->u16_ECCType;
		NC_ConfigContext();
		NC_ReInit();

		pNandDrv->u16_Reg48_Spare &= ~BIT_NC_HW_AUTO_RANDOM_CMD_DISABLE;
		NC_Config();

		/*
		**	Write Partition Info the 2nd page
		**/
		if(pNandDrv->u8_HasPNI == 1)
		{
			memset(au8_PageBuf, '\0', pNandDrv->u16_PageByteCnt);
			memcpy(au8_PageBuf, pPartInfo, 512);

			u32_Err = NC_WriteSectors(u32_PageIdx+ga_tPairedPageMap[1].u16_LSB, 0, au8_PageBuf, au8_SpareBuf, 1);
			if (u32_Err != UNFD_ST_SUCCESS)
			{
				nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Write Part Info failed with EC: 0x%08lx\n", u32_Err);
				drvNAND_MarkBadBlk(u16_PBA);
				continue;
			}
		}

		/*
		**  Write Paired Page Map to the 4th page
		**/
		if(pNandDrv->u8_CellType == 1)  // MLC
		{
			memset(au8_PageBuf, '\0', pNandDrv->u16_PageByteCnt);
			memcpy(au8_PageBuf, &ga_tPairedPageMap, 2048);
			u32_Err = NC_WritePages(u32_PageIdx+pNandInfo->u8_PairPageMapLoc, au8_PageBuf, au8_SpareBuf, 1);
			if (u32_Err != UNFD_ST_SUCCESS) {
				nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Write Paired-Page Map failed with EC: 0x%08lx\n", u32_Err);
				drvNAND_MarkBadBlk(u16_PBA);
				continue;
			}
		}

		nand_debug(0,1, "CIS%d is written to blk 0x%04x\n", u8_CISIdx, u16_PBA);

		pNandDrv->u16_CISPBA[u8_CISIdx] = u16_PBA;

		if ((++u8_CISIdx) == 2)
			break;
	}

	/* Reset NAND driver and FCIE to the original settings */
	pNandDrv->u16_SpareByteCnt = pNandInfo->u16_SpareByteCnt;
	pNandDrv->u16_PageByteCnt  = pNandInfo->u16_PageByteCnt;
	pNandDrv->u16_ECCType      = pNandInfo->u16_ECCType;
	NC_ConfigContext();
	NC_ReInit();
	pNandDrv->u16_Reg48_Spare &= ~(1 << 12);
	NC_Config();

	switch (u8_CISIdx) {
		case 0:
			u32_Err = UNFD_ST_ERR_NO_BLK_FOR_CIS0;
			break;
		case 1:
			u32_Err = UNFD_ST_ERR_NO_BLK_FOR_CIS1;
			break;
		case 2:
			u32_Err = UNFD_ST_SUCCESS;
			break;
	}

    return u32_Err;
}

U32 drvNAND_RefreshCIS(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	NAND_FLASH_INFO_t * pNandInfo = (NAND_FLASH_INFO_t* )vmalloc(512);
	PARTITION_INFO_t *pPartInfo = pNandDrv->pPartInfo;
	U32 u32_BlkIdx, u32_MBootBegin = 0, u32_MBootEnd = 0;
	U32 u32_Err;
	U8	u8_i;
	int bl_count = 0;

	if(!pNandInfo)
	{
		nand_debug(0,1, "Memory Allocate fail for pNandInfo\n");
		return -1;
	}

	//setup pNandInfo for CIS
	memset(pNandInfo, 0, 512);
	memcpy(pNandInfo->au8_Tag, "MSTARSEMIUNFDCIS", 16);
	pNandInfo->u8_IDByteCnt = pNandDrv->u8_IDByteCnt;
	memset(pNandInfo->au8_ID, 0, NAND_ID_BYTE_CNT);
	memcpy(pNandInfo->au8_ID, pNandDrv->au8_ID, pNandDrv->u8_IDByteCnt);
	pNandInfo->u16_SpareByteCnt = pNandDrv->u16_SpareByteCnt;
	pNandInfo->u16_PageByteCnt = pNandDrv->u16_PageByteCnt;
	pNandInfo->u16_BlkPageCnt = pNandDrv->u16_BlkPageCnt;
	pNandInfo->u16_BlkCnt = pNandDrv->u16_BlkCnt;
	pNandInfo->u32_Config = pNandDrv->u32_Config;

	pNandInfo->u16_ECCType = pNandDrv->u16_ECCType;
	pNandInfo->u16_tRC			= pNandDrv->u16_tRC;
	pNandInfo->u8_tRP			= pNandDrv->u8_tRP;
	pNandInfo->u8_tREH			= pNandDrv->u8_tREH;
	pNandInfo->u8_tREA			= pNandDrv->u8_tREA;
	pNandInfo->u8_tRR			= pNandDrv->u8_tRR;
	pNandInfo->u16_tADL 		= pNandDrv->u16_tADL;
	pNandInfo->u16_tRHW 		= pNandDrv->u16_tRHW;
	pNandInfo->u16_tWHR 		= pNandDrv->u16_tWHR;
	pNandInfo->u16_tCCS 		= pNandDrv->u16_tCCS;
	pNandInfo->u8_tCS			= pNandDrv->u8_tCS;
	pNandInfo->u16_tWC			= pNandDrv->u16_tWC;
	pNandInfo->u8_tWP			= pNandDrv->u8_tWP;
	pNandInfo->u8_tWH			= pNandDrv->u8_tWH;
	pNandInfo->u16_tCWAW		= pNandDrv->u16_tCWAW;
	pNandInfo->u8_tCLHZ 		= pNandDrv->u8_tCLHZ;
	pNandInfo->u8_AddrCycleIdx	= pNandDrv->u8_AddrCycleIdx;
	pNandInfo->u16_tWW			= pNandDrv->u16_tWW;

	pNandInfo->u8_PairPageMapLoc = pNandDrv->u8_PairPageMapLoc;
	pNandInfo->u8_ReadRetryType =	pNandDrv->u8_ReadRetryType;
	pNandInfo->u8_BitflipThreshold = pNandDrv->u16_BitflipThreshold;

	pNandInfo->u32_ChkSum		= drvNAND_CheckSum((U8*) (pNandInfo) + 0x24, 0x32 - 0x24);

	memcpy(pNandInfo->u8_Vendor, pNandDrv->u8_Vendor, 16);
	memcpy(pNandInfo->u8_PartNumber, pNandDrv->u8_PartNumber, 16);

	pNandInfo->u8_Hash0PageIdx = pNandDrv->u8_Hash0PageIdx;
	pNandInfo->u8_Hash1PageIdx = pNandDrv->u8_Hash1PageIdx;
	pNandInfo->u32_BootSize = pNandDrv->u32_BootSize;

	//search MBOOT partition in partinfo

	#if defined(FCIE_LFSR) && FCIE_LFSR
	if(pNandDrv->u8_RequireRandomizer)
		NC_DisableLFSR();
	#endif

	if(pNandDrv->u8_HasPNI == 1)
	{
		for(u8_i = 0; u8_i < pPartInfo->u16_PartCnt; u8_i ++)
		{
			if(pPartInfo->records[u8_i].u16_PartType == UNFD_PART_UBOOT)
			{
				u32_MBootBegin = pPartInfo->records[u8_i].u16_StartBlk;
				u32_MBootEnd = pPartInfo->records[u8_i].u16_StartBlk + pPartInfo->records[u8_i].u16_BlkCnt + pPartInfo->records[u8_i].u16_BackupBlkCnt;
				break;
			}
		}
		if(u8_i == pPartInfo->u16_PartCnt)
		{
			nand_debug(0,1,"ERROR: Partition info does not contain MBOOT partition\n");
			vfree(pNandInfo);
			return -1;
		}

		//search sboot uboot/ HashX location for update nni infomation

		if(pNandDrv->u8_BL0PBA != 0)	//for bl uboot
		{
			bl_count = 0;
			//search bl location in MBOOT PARTITION
			for(u32_BlkIdx = u32_MBootBegin; u32_BlkIdx < u32_MBootEnd; u32_BlkIdx ++)
			{
				u32_Err = NC_ReadPages(u32_BlkIdx << pNandDrv->u8_BlkPageCntBits, au8_MainBuf, au8_SpareBuf, 1);
				if(u32_Err != UNFD_ST_SUCCESS || au8_SpareBuf[0] !=0xFF)
					continue;
				if(!drvNAND_CheckAll0xFF(au8_MainBuf, pNandDrv->u16_PageByteCnt))
				{
					if(bl_count == 0)
						pNandInfo->u8_BL0PBA = pNandDrv->u8_BL0PBA = (U8)u32_BlkIdx;
					else if(bl_count == 1)
					{
						pNandInfo->u8_BL1PBA = pNandDrv->u8_BL1PBA = (U8)u32_BlkIdx;
						bl_count ++;
						break;
					}
					bl_count ++;
				}
			}

			nand_debug(0,1,"BL0_PBA %X, BL1_PBA %X\n", pNandDrv->u8_BL0PBA, pNandDrv->u8_BL1PBA);
			if(bl_count != 2)
			{
				nand_debug(0,1,"WARNING: there is no two sboots in NAND Flash, Please Reupgrade Sboot\n");
				vfree(pNandInfo);
				return -1;
			}

			if(pNandDrv->u8_UBOOTPBA != 0)
			{
				bl_count = 0;
				for(u32_BlkIdx = u32_MBootBegin; u32_BlkIdx < u32_MBootEnd; u32_BlkIdx ++)
				{
					u32_Err = NC_ReadPages(u32_BlkIdx << pNandDrv->u8_BlkPageCntBits, au8_MainBuf, au8_SpareBuf, 1);
					if(u32_Err != UNFD_ST_SUCCESS || au8_SpareBuf[0] !=0xFF)
						continue;
					if(((U32 *)au8_MainBuf)[0x7] == 0x0000B007)
					{
						if(bl_count == 1)
						{
							pNandInfo->u8_UBOOTPBA = pNandDrv->u8_UBOOTPBA = (U8)u32_BlkIdx;
							bl_count ++;
							break;
						}
						bl_count ++;
					}
				}
				if(bl_count != 2)
				{
					nand_debug(0,1,"WARNING: there is no two Mboots in NAND Flash, Please Reupgrade Mboot\n");
					vfree(pNandInfo);
					return -1;
				}

				nand_debug(0,1,"UBOOTPBA %X\n", pNandDrv->u8_UBOOTPBA);
			}
		}
		else if(pNandDrv->u8_HashPBA[0][0] != 0)	//for hash
		{
			bl_count = 0;
			//search bl location in MBOOT PARTITION
			for(u32_BlkIdx = u32_MBootBegin; u32_BlkIdx < u32_MBootEnd; u32_BlkIdx ++)
			{
				u32_Err = NC_ReadPages(u32_BlkIdx << pNandDrv->u8_BlkPageCntBits, au8_MainBuf, au8_SpareBuf, 1);
				if(u32_Err != UNFD_ST_SUCCESS || au8_SpareBuf[0] !=0xFF)
					continue;
				if(!drvNAND_CheckAll0xFF(au8_MainBuf, pNandDrv->u16_PageByteCnt))
				{
					pNandInfo->u8_HashPBA[bl_count>>1][bl_count&1] = pNandDrv->u8_HashPBA[bl_count>>1][bl_count&1] = (U8)u32_BlkIdx;
					if(++bl_count == 4)
						break;
				}
			}

			nand_debug(0,1,"HASH00_PBA %X, HASH01_PBA %X\n", pNandInfo->u8_HashPBA[0][0], pNandInfo->u8_HashPBA[0][1]);
			nand_debug(0,1,"HASH10_PBA %X, HASH11_PBA %X\n", pNandInfo->u8_HashPBA[1][0], pNandInfo->u8_HashPBA[1][1]);
			//printf("HASH00_PBA %X, HASH01_PBA %X\n", pNandInfo->u8_HashPBA[0][0], pNandInfo->u8_HashPBA[0][1]);
			if(bl_count != 4)
			{
				nand_debug(0,1,"WARNING: there is no two sboots in NAND Flash, Please Reupgrade Sboot\n");
				vfree(pNandInfo);
				return -1;
			}

			bl_count = 0;
			for(u32_BlkIdx = pNandDrv->u8_HashPBA[1][1]+1; u32_BlkIdx < u32_MBootEnd; u32_BlkIdx ++)
			{
				u32_Err = NC_ReadPages(u32_BlkIdx << pNandDrv->u8_BlkPageCntBits, au8_MainBuf, au8_SpareBuf, 1);
				if(u32_Err != UNFD_ST_SUCCESS || au8_SpareBuf[0] !=0xFF)
					continue;
				if(((U32 *)au8_MainBuf)[0x7] == 0x0000B007)
				{
					pNandInfo->u8_HashPBA[2][bl_count] = pNandDrv->u8_HashPBA[2][bl_count] = (U8)u32_BlkIdx;
					if(++bl_count == 2)
						break;
				}
			}
			if(bl_count != 2)
			{
				nand_debug(0,1,"WARNING: there is no two Mboots in NAND Flash, Please Reupgrade Mboot\n");
				vfree(pNandInfo);
				return -1;
			}

			nand_debug(0,1,"HASH20_PBA %X, HASH21_PBA %X\n", pNandInfo->u8_HashPBA[2][0], pNandInfo->u8_HashPBA[2][1]);
		}
	}

	u32_Err = drvNAND_WriteCIS_for_ROM(pNandInfo);

	vfree(pNandInfo);
	return u32_Err;
}


static int nand_write_bootloader(U32 u32_Row,U8 * pu8_addr, U32 u32_size, U8 u8_BootStageId)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();

	U8 * pu8_DataBuf = pu8_addr;
	U16 u16_BlkPageCnt;
	U32 u32_pagecnt, u32_Err;
	U8 u8_IdWritten = 0;

	u16_BlkPageCnt = (pNandDrv->u8_CellType) ? pNandDrv->u16_BlkPageCnt/2: pNandDrv->u16_BlkPageCnt;

	memset(au8_SpareBuf, 0xFF, pNandDrv->u16_SpareByteCnt);

	u32_pagecnt = u32_size >> pNandDrv->u8_PageByteCntBits;

	#if defined(FCIE_LFSR) && FCIE_LFSR
	if(pNandDrv->u8_RequireRandomizer)
		NC_DisableLFSR();
	#endif

	while(u32_pagecnt >= u16_BlkPageCnt)
	{
		while (drvNAND_IsGoodBlk(u32_Row >> pNandDrv->u8_BlkPageCntBits) == 0)
		{
			u32_Row += pNandDrv->u16_BlkPageCnt;
			//bad block jump to next block
			if(u32_Row == (pNandDrv->u16_BlkCnt << pNandDrv->u8_BlkPageCntBits))
			{
				nand_debug(0,1,"Error : There is no available GOOD block in current nand device\n");
				return -1;
			}
		}
		u32_Err = NC_EraseBlk(u32_Row);
		if(u32_Err != UNFD_ST_SUCCESS)
		{
			drvNAND_MarkBadBlk(u32_Row >> pNandDrv->u8_BlkPageCntBits);
			//jump to next block
			u32_Row += pNandDrv->u16_BlkPageCnt;
			continue;
		}
		if(u8_IdWritten == 0)
			au8_SpareBuf[1] = u8_BootStageId;
		else
			au8_SpareBuf[1] = 0xFF;

		{
			U16 u16_i, u16_flag = 0;
			U32 u32_TmpRow;
			for(u16_i =0; u16_i < u16_BlkPageCnt; u16_i ++)
			{
				u32_TmpRow = u32_Row + ga_tPairedPageMap[u16_i].u16_LSB;
				memcpy(au8_MainBuf, pu8_DataBuf, pNandDrv->u16_PageByteCnt);
				u32_Err = NC_WritePages(u32_TmpRow, au8_MainBuf, au8_SpareBuf, 1);
				if(u32_Err != UNFD_ST_SUCCESS)
				{
					drvNAND_MarkBadBlk(u32_Row >> pNandDrv->u8_BlkPageCntBits);
					//jump to next block
					u32_Row += pNandDrv->u16_BlkPageCnt;
					u16_flag = 1;
					break;
				}
				pu8_DataBuf += pNandDrv->u16_PageByteCnt;
			}
			if(u16_flag == 1)
			{
				pu8_DataBuf -= pNandDrv->u16_PageByteCnt * u16_i;
				continue;
			}

			u8_IdWritten = 1;
		}

		u32_pagecnt -= u16_BlkPageCnt;
		u32_size -= (u16_BlkPageCnt << pNandDrv->u8_PageByteCntBits);
		u32_Row += pNandDrv->u16_BlkPageCnt;
	}

	while(u32_size)
	{
		while (drvNAND_IsGoodBlk(u32_Row >> pNandDrv->u8_BlkPageCntBits) == 0)
		{
			u32_Row += pNandDrv->u16_BlkPageCnt;
			//bad block jump to next block
			if(u32_Row == (pNandDrv->u16_BlkCnt << pNandDrv->u8_BlkPageCntBits))
			{
				nand_debug(0,1, "Error : There is no available GOOD block in current nand device\n");
				return -1;
			}
		}

		u32_Err = NC_EraseBlk(u32_Row);
		if(u32_Err != UNFD_ST_SUCCESS)
		{
			drvNAND_MarkBadBlk(u32_Row >> pNandDrv->u8_BlkPageCntBits);
			//jump to next block
			u32_Row += pNandDrv->u16_BlkPageCnt;
			continue;
		}
		if(u8_IdWritten == 0)
			au8_SpareBuf[1] = u8_BootStageId;
		else
			au8_SpareBuf[1] = 0xFF;

		u32_pagecnt = u32_size >> pNandDrv->u8_PageByteCntBits;
		if((u32_size & (pNandDrv->u16_PageByteCnt -1)))
			 u32_pagecnt += 1;

		{
			U16 u16_i, u16_flag = 0;
			U32 u32_TmpRow;
			for(u16_i =0; u16_i < u32_pagecnt; u16_i ++)
			{
				u32_TmpRow = u32_Row + ga_tPairedPageMap[u16_i].u16_LSB;
				memcpy(au8_MainBuf, pu8_DataBuf, pNandDrv->u16_PageByteCnt);
				u32_Err = NC_WritePages(u32_TmpRow, au8_MainBuf, au8_SpareBuf, 1);
				if(u32_Err != UNFD_ST_SUCCESS)
				{
					drvNAND_MarkBadBlk(u32_Row >> pNandDrv->u8_BlkPageCntBits);
					//jump to next block
					u32_Row += pNandDrv->u16_BlkPageCnt;
					u16_flag = 1;
					break;
				}
				pu8_DataBuf += pNandDrv->u16_PageByteCnt;
			}
			if(u16_flag == 1)
			{
				pu8_DataBuf -= pNandDrv->u16_PageByteCnt * u16_i;
				continue;
			}

			u8_IdWritten = 1;
		}

		u32_size-= u32_size;
	}
	return 0;
}

int flush(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	U8* pSboot, *pMboot;
	U8	u8_RefreshCis = 0;
	U32 u32_BLPBA0, u32_BLPBA1, u32_UbootPBA0, u32_UbootPBA1, u32_BlkIdx = 0xA, u32_tmp, u32_size;
	size_t MbootSize = fps - SBOOT_SIZE;
	//The following variables are used by hash
    U8* u8_hash_buf[3] = {(U8*)vbuf, (U8*)(vbuf+HASH0_SIZE), (U8*)(vbuf+HASH0_SIZE+SBOOT_SIZE)};
    U32 u32_block_hash[3] = {0, 0 ,0};
    U32 u32_hash_size[3] = {HASH0_SIZE, SBOOT_SIZE, (fps-HASH0_SIZE-SBOOT_SIZE)};
    int i;

	pSboot = (U8*)vbuf;
	pMboot = (U8*)(((U32)vbuf) + SBOOT_SIZE);
	if(pNandDrv->u8_HasPNI == 0)
		return 0;

	nand_lock_fcie();
	#if defined(FCIE_LFSR) && FCIE_LFSR
	if(pNandDrv->u8_RequireRandomizer)
		NC_DisableLFSR();
	#endif

	//for BL, UBOOT PBA
	if(pNandDrv->u8_BL0PBA != 0 && pNandDrv->u8_BL1PBA != 0)
	{
		//skip bad block
		while(drvNAND_IsGoodBlk(u32_BlkIdx) == 0)
			u32_BlkIdx ++;

		u32_BLPBA0 = u32_BlkIdx;

		//check current block with cis records
		if(pNandDrv->u8_BL0PBA != u32_BLPBA0)
		{
			pNandDrv->u8_BL0PBA = u32_BLPBA0;
			u8_RefreshCis = 1;
		}

		u32_size = SBOOT_SIZE;
		nand_write_bootloader(u32_BLPBA0 << pNandDrv->u8_BlkPageCntBits, (U8*) pSboot, u32_size, 0x00);

		nand_debug(UNFD_DEBUG_LEVEL, 1, "write sboot @ block %lX\n", u32_BLPBA0);

		u32_BlkIdx = u32_BLPBA0;

		u32_tmp = (u32_size +  (1<<(pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageByteCntBits-pNandDrv->u8_CellType))-1)
							>> (pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageByteCntBits - pNandDrv->u8_CellType);
		while(u32_tmp > 0)
		{
			if(drvNAND_IsGoodBlk(u32_BlkIdx))
			{
				u32_tmp --;
			}
			u32_BlkIdx ++;
		}

		//next BL
		while(drvNAND_IsGoodBlk(u32_BlkIdx) == 0)
			u32_BlkIdx ++;

		u32_BLPBA1 = u32_BlkIdx;

		//check current block with cis records
		if(pNandDrv->u8_BL1PBA != u32_BLPBA1)
		{
			pNandDrv->u8_BL1PBA = u32_BLPBA1;
			u8_RefreshCis = 1;
		}

		u32_size = SBOOT_SIZE;
		nand_write_bootloader(u32_BLPBA1 << pNandDrv->u8_BlkPageCntBits, (U8*) pSboot, u32_size, 0x00);

		nand_debug(UNFD_DEBUG_LEVEL, 1, "write sboot backup @ block %lX\n", u32_BLPBA1);

		u32_BlkIdx = u32_BLPBA1;

		u32_tmp = (u32_size +  (1<<(pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageByteCntBits-pNandDrv->u8_CellType))-1)
							>> (pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageByteCntBits - pNandDrv->u8_CellType);
		while(u32_tmp > 0)
		{
			if(drvNAND_IsGoodBlk(u32_BlkIdx))
			{
				u32_tmp --;
			}
			u32_BlkIdx ++;
		}

		//write uboot
		while(drvNAND_IsGoodBlk(u32_BlkIdx) == 0)
			u32_BlkIdx ++;

		u32_UbootPBA0 = u32_BlkIdx;
		u32_size = MbootSize;
		nand_write_bootloader(u32_UbootPBA0 << pNandDrv->u8_BlkPageCntBits, (U8*) pMboot, u32_size, 0xFF);

		nand_debug(UNFD_DEBUG_LEVEL, 1, "write uboot @ block %lX\n", u32_UbootPBA0);

		u32_BlkIdx = u32_UbootPBA0;

		u32_tmp = (u32_size +  (1<<(pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageByteCntBits-pNandDrv->u8_CellType))-1)
							>> (pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageByteCntBits - pNandDrv->u8_CellType);

		while(u32_tmp > 0)
		{
			if(drvNAND_IsGoodBlk(u32_BlkIdx))
			{
				u32_tmp --;
			}
			u32_BlkIdx ++;
		}

		u32_UbootPBA1 = u32_BlkIdx;
		if(pNandDrv->u8_UBOOTPBA != u32_UbootPBA1)
		{
			pNandDrv->u8_UBOOTPBA = u32_UbootPBA1;
			u8_RefreshCis = 1;
		}

		nand_write_bootloader(u32_UbootPBA1 << pNandDrv->u8_BlkPageCntBits, (U8*) pMboot, MbootSize, 0xFF);

		nand_debug(UNFD_DEBUG_LEVEL, 1, "write uboot backup @ block %lX\n", u32_UbootPBA1);
	}
	else if(pNandDrv->u8_HashPBA[0][0] != 0)
	{
		for(i=0; i<3; i++)
		{
			//Search good block for hash[0-2]
			while(drvNAND_IsGoodBlk(u32_BlkIdx) == 0)
				u32_BlkIdx++;
			u32_block_hash[i] = u32_BlkIdx;

			//check current block with cis records
			if(pNandDrv->u8_HashPBA[i][0] != u32_block_hash[i])
			{
		        pNandDrv->u8_HashPBA[i][0] = u32_block_hash[i];
				u8_RefreshCis = 1;
			}

			//Write hash[0-2]
			nand_debug(UNFD_DEBUG_LEVEL, 1, "write hash%d @ block %lX\n", i, u32_block_hash[i]);
			nand_write_bootloader(u32_block_hash[i]<< pNandDrv->u8_BlkPageCntBits, u8_hash_buf[i], u32_hash_size[i], (0x80|i));

			u32_tmp = (u32_hash_size[i]+(1<<(pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageByteCntBits-pNandDrv->u8_CellType))-1)
						>> (pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageByteCntBits-pNandDrv->u8_CellType);
			while(u32_tmp > 0)
			{
				if(drvNAND_IsGoodBlk(u32_BlkIdx))
				{
					u32_tmp --;
				}
				u32_BlkIdx ++;
			}

			//Search good block for hash[0-2] backup
			while(drvNAND_IsGoodBlk(u32_BlkIdx) == 0)
				u32_BlkIdx ++;
			u32_block_hash[i] = u32_BlkIdx;

			//check current block with cis records
			if(pNandDrv->u8_HashPBA[i][1] != u32_block_hash[i])
			{
				pNandDrv->u8_HashPBA[i][1] = u32_block_hash[i];
				u8_RefreshCis = 1;
			}

			//Write hash[0-2] backup
			nand_debug(UNFD_DEBUG_LEVEL, 1, "write hash%d backup @ block %lX\n", i, u32_block_hash[i]);
			nand_write_bootloader(u32_block_hash[i]<< pNandDrv->u8_BlkPageCntBits, u8_hash_buf[i], u32_hash_size[i], (0x80|i));

			u32_tmp = (u32_hash_size[i]+(1<<(pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageByteCntBits-pNandDrv->u8_CellType))-1)
						>> (pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageByteCntBits-pNandDrv->u8_CellType);
			while(u32_tmp > 0)
			{
				if(drvNAND_IsGoodBlk(u32_BlkIdx))
				{
					u32_tmp --;
				}
				u32_BlkIdx ++;
			}
		}
	}

	if(u8_RefreshCis == 1)
	{
		drvNAND_RefreshCIS();
	}

	nand_unlock_fcie();

	return 0;
}

#if 0
static int mstar_nand_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long argp)
{
	unsigned long size;

	nand_debug(UNFD_DEBUG_LEVEL, 1,"%s is invoked\n", __func__);
	size = (cmd & IOCSIZE_MASK) >> IOCSIZE_SHIFT;
	if (cmd & IOC_IN) {
		if (!access_ok(VERIFY_READ, argp, size))
			return -EFAULT;
	}
	if (cmd & IOC_OUT) {
		if (!access_ok(VERIFY_WRITE, argp, size))
			return -EFAULT;
	}

	switch(cmd)
	{

	}
	return 0;
}
#endif

static ssize_t mstar_nand_write(struct file *file, const char __user *buf, size_t count,loff_t *ppos)
{
	if(copy_from_user(cur_buf, buf, count))
	{
		return -EFAULT;
	}
	cur_buf += count;
	fps += count;
//	nand_debug(UNFD_DEBUG_LEVEL, 1, "%d\n", count);
	return count;
}

static ssize_t mstar_nand_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
	nand_debug(UNFD_DEBUG_LEVEL, 1, "\n");
    return 0;
}

static int mstar_nand_open (struct inode *inode, struct file *filp)
{
	nand_debug(UNFD_DEBUG_LEVEL, 1, "\n");
	vbuf= vmalloc(0x200000);
	if(!vbuf)
		return -ENOMEM;
	cur_buf = vbuf;
	fps = 0;
    return 0;
}

static int mstar_nand_close(struct inode *inode, struct file *filp)
{
	nand_debug(UNFD_DEBUG_LEVEL, 1, "%d\n", fps);
	flush();
	if(vbuf)
		vfree(vbuf);
    return 0;
}

static const struct file_operations mstar_nand_fops = {
	.owner		= THIS_MODULE,
	.write		= mstar_nand_write,
	.read		= mstar_nand_read,
//	.ioctl		= mstar_nand_ioctl,
	.open		= mstar_nand_open,
	.release	= mstar_nand_close,
};

static int __init init_mstar_nand_char(void)
{
	int ret;
	dev_t dev;
    u16 u16_regval = 0;

    u16_regval = REG(NC_REG_16h);

    if( (u16_regval & BIT_KERN_CHK_NAND_EMMC) == BIT_KERN_CHK_NAND_EMMC )
    {
        if( (u16_regval & BIT_KERN_NAND) != BIT_KERN_NAND )
            return 0;
    }

	nand_debug(UNFD_DEBUG_LEVEL, 1, "\n");
	//ret = register_chrdev(MSTAR_NAND_CHAR_MAJOR,"mstar_nand", &mstar_nand_fops);
	dev = MKDEV(MSTAR_NAND_CHAR_MAJOR, 0);
	ret = register_chrdev_region(dev, 1, MSTAR_CDEV_NAND_NAME);
	if (ret < 0) {
		pr_notice("Can't allocate major number %d for "
				"Memory Technology Devices.\n", MSTAR_NAND_CHAR_MAJOR);
		return ret;
	}

	//add cdev
	cdev_init(&cDevice, &mstar_nand_fops);
	cDevice.owner = THIS_MODULE;
	cDevice.ops = &mstar_nand_fops;

	ret = cdev_add(&cDevice, dev, 1);
	if(ret != 0)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Unable to add a char deivce\n");
		unregister_chrdev_region(dev, 1);
	}

	schedule_delayed_work(&rd_work, 30*HZ);
	return ret;
}


static void __exit cleanup_mstar_nand_char(void)
{
	cdev_del(&cDevice);

	unregister_chrdev_region( MKDEV(MSTAR_NAND_CHAR_MAJOR, 0), 1);
}

module_init(init_mstar_nand_char);
module_exit(cleanup_mstar_nand_char);
MODULE_ALIAS_CHARDEV_MAJOR(MSTAR_NAND_CHAR_MAJOR);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("mstar");
MODULE_DESCRIPTION("Direct access to mstar nand hidden partition");
MODULE_ALIAS_CHARDEV_MAJOR(MSTAR_NAND_CHAR_MAJOR);

#endif
