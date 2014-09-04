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

//#define MS_DBG
#ifdef MS_DBG
#define ms_dbg(fmt, args...)   do { printf(fmt , ## args); } while (0)
#else
#define ms_dbg(fmt, args...)   do { } while (0)
#endif

#define ms_err(fmt, args...)   do { printf(fmt , ## args); } while (0)

#define XHCI_INIT_VALUE 0x0

void ms_xhci_print_regs(struct xhci_hcd *pXhci)
{
	u32 u32Val;

	ms_dbg("xhci capability regs at %p:\n", pXhci->cap_regs);

	u32Val = xhci_readl(pXhci, &pXhci->cap_regs->caplen_hciver);
	ms_dbg("CAPLENGTH&HCIVERSION %p = 0x%x \n",
			&pXhci->cap_regs->caplen_hciver, u32Val);

	ms_dbg("xhci operational regs at %p:\n", pXhci->op_regs);

	u32Val = xhci_readl(pXhci, &pXhci->cap_regs->u32RtsOff);
	ms_dbg("RTSOFF %p = 0x%x \n",
			&pXhci->cap_regs->u32RtsOff,
			(unsigned int) u32Val & RTSOFF_RSVD_MASK);

	ms_dbg("xhci runtime regss at %p:\n", pXhci->run_regs);

	u32Val = xhci_readl(pXhci, &pXhci->cap_regs->u32DbOff);
	ms_dbg("DBOFF %p = 0x%x \n", &pXhci->cap_regs->u32DbOff, u32Val);
	ms_dbg("doorbell array at %p:\n", pXhci->doorbells);
}

static void ms_xhci_print_cap_regs(struct xhci_hcd *pXhci)
{
	u32 u32Val;

	ms_dbg("xhci capability regs at %p:\n", pXhci->cap_regs);

	u32Val = xhci_readl(pXhci, &pXhci->cap_regs->caplen_hciver);
	ms_dbg("CAPLENGTH&HCIVERSION: 0x%x:\n",	(unsigned int) u32Val);

	u32Val = xhci_readl(pXhci, &pXhci->cap_regs->u32HcsParams1);
	ms_dbg("HCSPARAMS 1: 0x%x\n", (unsigned int) u32Val);

	u32Val = xhci_readl(pXhci, &pXhci->cap_regs->u32HcsParams2);
	ms_dbg("HCSPARAMS 2: 0x%x\n", (unsigned int) u32Val);

	u32Val = xhci_readl(pXhci, &pXhci->cap_regs->u32HcsParams3);
	ms_dbg("HCSPARAMS 3 0x%x:\n", (unsigned int) u32Val);

	u32Val = xhci_readl(pXhci, &pXhci->cap_regs->u32HccParams);
	ms_dbg("HCCPARAMS 0x%x:\n", (unsigned int) u32Val);

	u32Val = xhci_readl(pXhci, &pXhci->cap_regs->u32DbOff);
	ms_dbg("DBOFF %p = 0x%x \n", &pXhci->cap_regs->u32DbOff, u32Val);

	u32Val = xhci_readl(pXhci, &pXhci->cap_regs->u32RtsOff);
	ms_dbg("RTSOFF 0x%x:\n", u32Val);
}

static void ms_xhci_print_cmd_reg(struct xhci_hcd *pXhci)
{
	u32 u32Val;

	u32Val = xhci_readl(pXhci, &pXhci->op_regs->u32UsbCmd);
	ms_dbg("USBCMD: 0x%x:\n", u32Val);
}

static void ms_xhci_print_status_reg(struct xhci_hcd *pXhci)
{
	u32 u32Val;

	u32Val = xhci_readl(pXhci, &pXhci->op_regs->u32UsbSts);
	ms_dbg("USBSTS: 0x%x\n", u32Val);
}

static void ms_xhci_print_op_regs(struct xhci_hcd *xhci)
{
	ms_dbg("xhci operational regs at %p:\n", xhci->op_regs);
	ms_xhci_print_cmd_reg(xhci);
	ms_xhci_print_status_reg(xhci);
}

static void ms_xhci_print_ports(struct xhci_hcd *pXhci)
{
	__le32 __iomem *addr;
	int i;
	int ports;
	u32	hcs_params1;

	hcs_params1 = xhci_readl(pXhci, &pXhci->cap_regs->u32HcsParams1);
	ports = HCS_MAX_NUM_PORTS(hcs_params1);
	addr = &pXhci->op_regs->u32PortSc;
	for (i = 0; i < ports; i++) {
		ms_dbg("port %d (%p):\n", i, addr);
		ms_dbg("PORTSC: 0x%x\n", (unsigned int) xhci_readl(pXhci, addr));
		ms_dbg("PORTPMSC: 0x%x\n", (unsigned int) xhci_readl(pXhci, addr+1));
		ms_dbg("PORTLI: 0x%x\n", (unsigned int) xhci_readl(pXhci, addr+2));
	}
}

void ms_xhci_print_interrupt_reg_set(struct xhci_hcd *pXhci, int Ir_num)
{
	struct xhci_intr_reg __iomem *stIrSet = &pXhci->run_regs->stIrSet[Ir_num];
	void __iomem *addr;
	u32 u32Val;
	u64 u64Val;

	addr = &stIrSet->u32IMAN;
	u32Val = xhci_readl(pXhci, addr);
	if (u32Val == XHCI_INIT_VALUE)
		return;

	ms_dbg("interrupt reg set at %p\n", stIrSet);
	ms_dbg("IMAN: 0x%x\n", (unsigned int)u32Val);

	addr = &stIrSet->u32IMOD;
	u32Val = xhci_readl(pXhci, addr);
	ms_dbg("IMOD: 0x%x\n", (unsigned int)u32Val);

	addr = &stIrSet->u32ERSTSZ;
	u32Val = xhci_readl(pXhci, addr);
	ms_dbg("ERSTSZ: 0x%x\n", (unsigned int)u32Val);

	addr = &stIrSet->u32ERSTBA;
	u64Val = xhci_read_64(pXhci, addr);
	ms_dbg("ERSTBA:  0x%08llx\n",u64Val);

	addr = &stIrSet->u32ERDP;
	u64Val = xhci_read_64(pXhci, addr);
	ms_dbg("ERDP: 0x%08llx\n", u64Val);
}

void ms_xhci_print_run_regs(struct xhci_hcd *pXhci)
{
	u32 u32Val;

	ms_dbg("xhci runtime regs at %p:\n", pXhci->run_regs);
	u32Val = xhci_readl(pXhci, &pXhci->run_regs->u32MfIndex);
	ms_dbg("MFINDEX: 0x%x\n", (unsigned int) u32Val);
}

void ms_xhci_dump_regs(struct xhci_hcd *pXhci)
{
	ms_xhci_print_cap_regs(pXhci);
	ms_xhci_print_op_regs(pXhci);
	ms_xhci_print_ports(pXhci);
}

void ms_xhci_print_trb(union xhci_trb *pTrb)
{
	int i;
	for (i = 0; i < 4; ++i)
		ms_dbg("offset 0x%x = 0x%x\n",
				i*4, pTrb->generic.field[i]);
}

void ms_xhci_dump_trb(struct xhci_hcd *pXhci, union xhci_trb *pTrb)
{
	u64	addr;
	u32	trb_type = le32_to_cpu(pTrb->link.trb_info) & TRB_TYPE_MASK;

	switch (trb_type) {
	case TRB_TYPE(TRB_TYPE_LINK):
		ms_dbg("link trb:\n");

		addr = pTrb->link.ring_segment_ptr;
		ms_dbg("ring segment pointer: 0x%llx\n", addr);

		ms_dbg("interrupter target: 0x%x\n",
			(pTrb->link.interrupt_target >> 22) & 0x3ff);
		ms_dbg("cycle bit: %d\n", pTrb->link.trb_info & (0x1<<0));
		ms_dbg("toggle cycle: %d\n", pTrb->link.trb_info & (0x1<<1));
		ms_dbg("chain bit: %d\n", pTrb->link.trb_info & (0x1<<4));
		break;

	case TRB_TYPE(TRB_TYPE_TRANSFER):
		ms_dbg("transfer trb:\n");
		addr = le64_to_cpu(pTrb->trans_event.trb_ptr);
		break;

	case TRB_TYPE(TRB_TYPE_COMPLETION_EVENT):
		ms_dbg("command completion event trb:\n");
		addr = pTrb->event_cmd.cmd_trb_ptr;
		ms_dbg("command trb pointer: %llu\n", addr);
		ms_dbg("completion code: %d\n",
			(pTrb->event_cmd.status & (0xff << 24)) >> 24);
		break;

	default:
		ms_dbg("unknown trb type %u\n", (unsigned int) trb_type>>10);
		break;
	}

	ms_xhci_print_trb(pTrb);
}

void ms_xhci_print_seg(struct xhci_seg *pSeg)
{
	int i;
	u64 addr = pSeg->dma;
	union xhci_trb *pTrb = pSeg->pTrbs;

	for (i = 0; i < MAX_TRBS_IN_SEG; ++i) {
		pTrb = &pSeg->pTrbs[i];
		ms_dbg("%016llx %08x %08x %08x %08x\n", addr,
			 (u32)(pTrb->generic.field[0]),
			 (u32)(pTrb->generic.field[1]),
			 (u32)(pTrb->generic.field[2]),
			 (u32)(pTrb->generic.field[3]));
		addr += sizeof(*pTrb);
	}
}

void ms_xhci_print_ring_ptrs(struct xhci_ring *ring)
{
	ms_dbg("ring deq: %p, 0x%llx (dma)\n", ring->pDeq,
			(unsigned long long)ms_xhci_trb_to_dma(ring->pDeq_seg,  ring->pDeq));

	ms_dbg("ring enq: %p, 0x%llx (dma)\n", ring->pEnq,
			(unsigned long long)ms_xhci_trb_to_dma(ring->pEnq_seg,  ring->pEnq));
}

void ms_xhci_dump_ring(struct xhci_ring *pRing)
{
	struct xhci_seg *first_seg = pRing->pFirst_seg;

	ms_xhci_print_seg(first_seg);

}

void ms_xhci_dump_erst(struct xhci_erst *pErst)
{
	u64 addr = pErst->dma_addr;
	int i;
	struct xhci_event_ring_seg_table_entry *pErst_entry;

	for (i = 0; i < pErst->entry_count; ++i) {
		pErst_entry = &pErst->entries[i];
		ms_dbg("%016llx %08x %08x %08x %08x\n",
			 addr,
			 lower_32_bits(pErst_entry->ring_base_addr),
			 upper_32_bits(pErst_entry->ring_base_addr),
			 (unsigned int) pErst_entry->ring_seg_size,
			 (unsigned int) pErst_entry->reserved);
		addr += sizeof(*pErst_entry);
	}
}

void ms_xhci_print_cmd_ptrs(struct xhci_hcd *pXhci)
{
	u64 u64Val;

	u64Val = xhci_read_64(pXhci, &pXhci->op_regs->u64CrCr);
	ms_dbg("command ring control reg Lo %08x\n", lower_32_bits(u64Val));
	ms_dbg("command ring control reg Hi %08x\n", upper_32_bits(u64Val));
}

static void ms_xhci_print_slot_ctx(struct xhci_hcd *pXhci, struct xhci_container_ctx *pCtx)
{
	/* Fields are 32 bits wide, DMA addresses are in bytes */
	int field_size = 32 / 8;
//	int i;

	struct xhci_slot_ctx *pSlot_ctx = ms_xhci_get_slot_ctx(pXhci, pCtx);
	dma_addr_t dma = pCtx->dma_addr +
		((unsigned long)pSlot_ctx - (unsigned long)pCtx->pBuf);

	ms_dbg("slot ctx: %p %08llx (dma)\n",
			&pSlot_ctx->dev_slot_info1, (unsigned long long)dma);
	ms_dbg("offset 0: 0x%x\n", pSlot_ctx->dev_slot_info1);
	ms_dbg("offset 4: 0x%x\n", pSlot_ctx->dev_slot_info2);
	ms_dbg("offset 8: 0x%x\n", pSlot_ctx->dev_slot_tt_info);
	ms_dbg("offset C: 0x%x\n", pSlot_ctx->dev_slot_state_info);

	dma += field_size;
}

static void ms_xhci_print_ep_ctx(struct xhci_hcd *pXhci,
		     struct xhci_container_ctx *pCtx,
		     unsigned int last_ep_num)
{
	int i;
	int last_ep_ctx;
	int field_size = 32 / 8;

	last_ep_ctx = last_ep_num + 1;

	for (i = 0; i < last_ep_ctx; ++i) {
		struct xhci_ep_ctx *pEpt_ctx = ms_xhci_get_ep_ctx(pXhci, pCtx, i);
		dma_addr_t dma = pCtx->dma_addr +
			((unsigned long)pEpt_ctx - (unsigned long)pCtx->pBuf);

		ms_dbg("endpoint %d ctx: %p %08llx (dma)\n",
				i, &pEpt_ctx->ep_ctx_field1, (unsigned long long)dma);
		ms_dbg("offset 0: 0x%x\n", pEpt_ctx->ep_ctx_field1);
		ms_dbg("offset 4: 0x%x\n", pEpt_ctx->ep_ctx_field2);
		ms_dbg("offset 8: 0x%llx\n", pEpt_ctx->tr_deq_ptr);
		ms_dbg("offset 10: 0x%x\n", pEpt_ctx->avg_trb_len);

		dma += field_size;
	}
}

void ms_xhci_print_ctx(struct xhci_hcd *pXhci,
		  struct xhci_container_ctx *pCtx,
		  unsigned int last_ep_num)
{
	/* Fields are 32 bits wide, DMA addresses are in bytes */
	int field_size = 32 / 8;
	struct xhci_slot_ctx *pSlot_ctx;
	dma_addr_t dma = pCtx->dma_addr;

	if (pCtx->type == CTX_TYPE_INPUT) {
	#ifdef MS_DBG
		struct xhci_input_control_ctx *pCtrl_ctx =
			ms_xhci_get_input_control_ctx(pCtx);
	#endif

		ms_dbg("input ctx: %p %08llx (dma)\n",
			&pCtrl_ctx->drop_ctx_flags, (unsigned long long)dma);
		ms_dbg("offset 0: 0x%x\n", pCtrl_ctx->drop_ctx_flags);
		ms_dbg("offset 4: 0x%x\n", pCtrl_ctx->add_ctx_flags);

		dma += field_size;
	}

	pSlot_ctx = ms_xhci_get_slot_ctx(pXhci, pCtx);
	ms_xhci_print_slot_ctx(pXhci, pCtx);
	ms_xhci_print_ep_ctx(pXhci, pCtx, last_ep_num);
}

