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
#include "../usb/ehci_usb.h"
#include "usbdesc.h"

//#define MS_DBG
#ifdef MS_DBG
#define ms_dbg(fmt, args...)   do { printf(fmt , ## args); } while (0)
#else
#define ms_dbg(fmt, args...)   do { } while (0)
#endif

#define ms_err(fmt, args...)   do { printf(fmt , ## args); } while (0)

struct xhci_dcbaa  xHCI_DevContextArray  __attribute__ ((aligned (128)));
U8 CmdRingBuf[XHCI_SEG_SIZE]   __attribute__ ((aligned (128)));
U8 EventRingBuf[XHCI_SEG_SIZE] __attribute__ ((aligned (128)));
U8 xhc_ERST[16*MS_MAX_ERST_SEGS] __attribute__ ((aligned (128)));
U8 SP_array[5*16]             __attribute__ ((aligned (128)));
U8 SP_buf[5][1024*8]          __attribute__ ((aligned (128)));

// For one device only !!
U8 InCTX[XHCI_MAX_DEV][2176]                __attribute__ ((aligned (128)));
U8 OutCTX[XHCI_MAX_DEV][2048]               __attribute__ ((aligned (128)));
U8 EP0RingBuf[XHCI_MAX_DEV][XHCI_SEG_SIZE]   __attribute__ ((aligned (128)));
U8 CmdCTX[2176]                __attribute__ ((aligned (128)));
U8 BulkRingBuf[XHCI_MAX_DEV][XHCI_MAX_BULK_NUM*2][XHCI_SEG_SIZE]   __attribute__ ((aligned (128)));

static struct xhci_seg *ms_xhci_seg_alloc(void *pBuf)
{
	struct xhci_seg *pSeg;

	if (pBuf == NULL)
	{
		ms_err("%s: pBuf is NULL\n", __func__);
		return NULL;
	}

	pSeg = kzalloc(sizeof *pSeg, 0);
//	printf("pSeg:%x\n",pSeg);
	if (!pSeg)
		return NULL;
	ms_dbg("%s: alloc seg at %p\n", __func__, pSeg);

	pSeg->pTrbs = pBuf;
	//printf("trbs:%x\n",pSeg->pTrbs);

	pSeg->dma = VA2PA((U32)pBuf);

	ms_dbg("%s: buf dma at 0x%llx\n", __func__, (unsigned long long)pSeg->dma);

	memset(pSeg->pTrbs, 0, XHCI_SEG_SIZE);
	pSeg->next = NULL;

	return pSeg;
}

static void ms_xhci_seg_free(struct xhci_seg *pSeg)
{
	if (!pSeg)
		return;

	if (pSeg->pTrbs) {
		ms_dbg("%s: at %p, 0x%llx (DMA)\n",
				__func__, pSeg->pTrbs, (unsigned long long)pSeg->dma);

		pSeg->pTrbs = NULL;
	}
	ms_dbg("%s: at %p\n", __func__, pSeg);
	kfree(pSeg);
}

static void ms_xhci_link_segs(struct xhci_seg *pPreSeg,
		struct xhci_seg *pNextSeg, bool is_link_trbs)
{
	u32 val;

	if (!pPreSeg || !pNextSeg)
		return;

	pPreSeg->next = pNextSeg;
	if (is_link_trbs) {
		pPreSeg->pTrbs[MAX_TRBS_IN_SEG-1].link.ring_segment_ptr = pNextSeg->dma;

		val = pPreSeg->pTrbs[MAX_TRBS_IN_SEG-1].link.trb_info;
		val &= ~TRB_TYPE_MASK;
		val |= TRB_TYPE(TRB_TYPE_LINK);
		pPreSeg->pTrbs[MAX_TRBS_IN_SEG-1].link.trb_info = val;
	}
	ms_dbg("%s: link 0x%llx to 0x%llx\n",
			__func__,
			(unsigned long long)pPreSeg->dma,
			(unsigned long long)pNextSeg->dma);
}

void ms_xhci_ring_free(struct xhci_ring *pRing)
{

	if (!pRing)
		return;

	ms_dbg("free ring at %p\n", pRing);
	if (pRing->pFirst_seg) {
		ms_xhci_seg_free(pRing->pFirst_seg);
		pRing->pFirst_seg = NULL;
	}
	kfree(pRing);
}

static struct xhci_ring *ms_xhci_ring_alloc(bool is_link_trbs, void *pBuf)
{
	struct xhci_ring	*ring;
	struct xhci_seg	*prev;

	ring = kzalloc(sizeof *(ring), 0);
	ms_dbg("%s: ring at %p\n", __func__, ring);
	if (!ring)
		return NULL;

	INIT_LIST_HEAD(&ring->td_list);

	ring->pFirst_seg = ms_xhci_seg_alloc(pBuf);
	if (!ring->pFirst_seg)
		goto fail;

	prev = ring->pFirst_seg;
	ms_xhci_link_segs(prev, ring->pFirst_seg, is_link_trbs);

	if (is_link_trbs) {
		prev->pTrbs[MAX_TRBS_IN_SEG-1].link.trb_info |= LINK_TOGGLE;
	}
	ring->pEnq = ring->pFirst_seg->pTrbs;
	ring->pEnq_seg = ring->pFirst_seg;
	ring->pDeq = ring->pEnq;
	ring->pDeq_seg = ring->pFirst_seg;
	ring->cycle_state = 1;

	return ring;

fail:
	ms_err("%s: alloc failed\n", __func__);
	ms_xhci_ring_free(ring);
	return NULL;
}

static void ms_xhci_set_event_deq(struct xhci_hcd *pXhci)
{
	u64 temp;
	dma_addr_t deq;

	deq = ms_xhci_trb_to_dma(pXhci->event_ring->pDeq_seg,
			pXhci->event_ring->pDeq);
	if (deq == 0)
	{
		ms_err("%s: dep is NULL\n", __func__);
	}
	temp = xhci_read_64(pXhci, &pXhci->intr_regs->u32ERDP);
	temp &= ERDP_MASK;
	temp &= ~ERDP_EHB; //Don't clear the EHB bit
	xhci_write_64(pXhci, ((u64) deq & (u64) ~ERDP_MASK) | temp,
			&pXhci->intr_regs->u32ERDP);
}

static int ms_scratchpad_alloc(struct xhci_hcd *pXhci)
{
	int i, num_sp;
	u32 hcs_params2;

	hcs_params2 = xhci_readl(pXhci, &pXhci->cap_regs->u32HcsParams2);
	num_sp = HCS_MAX_SPR(hcs_params2);
	ms_dbg("%s: num_sp = %d\n", __func__, num_sp);

	if (!num_sp)
		return 0;

	pXhci->scratchpad.sp_array = (u64*)KSEG02KSEG1(SP_array);
	pXhci->scratchpad.dma_addr = VA2PA((U32)pXhci->scratchpad.sp_array);

	pXhci->dcbaa->dev_ctx_bases[0] = pXhci->scratchpad.dma_addr;
	for (i = 0; i < num_sp; i++) {
		dma_addr_t dma;
		void *buf = (void*) KSEG02KSEG1(SP_buf[i]);

		dma = VA2PA((U32)buf);
		pXhci->scratchpad.sp_array[i] = dma;
		pXhci->scratchpad.sp_buffers[i] = buf;
		pXhci->scratchpad.sp_dma_buffers[i] = dma;
	}
	return 0;
}

int ms_xhci_mem_init(struct xhci_hcd *pXhci)
{
	dma_addr_t	dma;
	unsigned int	val, val2;
	u64		val_64;
	struct xhci_seg	*pSeg;
	int i;

	pXhci->page_shift = 12;
	pXhci->page_size = 1 << pXhci->page_shift;
	ms_dbg("%s: xhci page size set to %d\n", __func__, pXhci->page_size);

	val = HCS_MAX_NUM_SLOTS(xhci_readl(pXhci, &pXhci->cap_regs->u32HcsParams1));
	ms_dbg("%s: HW support max slot = %d\n", __func__, (unsigned int) val);
	val2 = xhci_readl(pXhci, &pXhci->op_regs->u32Config);
	val |= (val2 & ~HCS_MAX_SLOTS_MASK);
	ms_dbg("%s: max device slots  = %d\n", __func__, (unsigned int) val);
	xhci_writel(pXhci, val, &pXhci->op_regs->u32Config);

	pXhci->dcbaa = (struct xhci_dcbaa *)KSEG02KSEG1(&xHCI_DevContextArray);
	memset(pXhci->dcbaa, 0, sizeof *(pXhci->dcbaa));
	pXhci->dcbaa->dma = VA2PA((U32)&xHCI_DevContextArray);
	ms_dbg("%s: DCBAA = %p, 0x%llx\n", __func__,
			pXhci->dcbaa,	(unsigned long long)pXhci->dcbaa->dma);
	xhci_write_64(pXhci, pXhci->dcbaa->dma, &pXhci->op_regs->u64DcbaaPtr);

	pXhci->cmd_ring = ms_xhci_ring_alloc(true, (void*)KSEG02KSEG1(CmdRingBuf));
	if (!pXhci->cmd_ring)
		goto fail;
	ms_dbg("%s: alloc cmd ring at %p 1st seg at 0x%llx (DMA)\n",
		__func__,
		pXhci->cmd_ring,
		(unsigned long long)pXhci->cmd_ring->pFirst_seg->dma);

	val_64 = xhci_read_64(pXhci, &pXhci->op_regs->u64CrCr);
	val_64 = (val_64 & (u64) CMD_RING_MASK_BITS) |
		(pXhci->cmd_ring->pFirst_seg->dma & (u64) ~CMD_RING_MASK_BITS) |
		pXhci->cmd_ring->cycle_state;
	ms_dbg("%s: set cmd ring to 0x%llx\n", __func__, val_64);
	xhci_write_64(pXhci, val_64, &pXhci->op_regs->u64CrCr);
	ms_xhci_print_cmd_ptrs(pXhci);

	val = xhci_readl(pXhci, &pXhci->cap_regs->u32DbOff);
	val &= DBOFF_RSVD_MASK;
	ms_dbg("%s: DBOFF = 0x%x\n", __func__, val);
	pXhci->doorbells = (void __iomem *) pXhci->cap_regs + val;
	ms_xhci_print_regs(pXhci);
	ms_xhci_print_run_regs(pXhci);
	pXhci->intr_regs = &pXhci->run_regs->stIrSet[0];

	pXhci->event_ring = ms_xhci_ring_alloc(false, (void*)KSEG02KSEG1(EventRingBuf));
	if (!pXhci->event_ring)
		goto fail;

	pXhci->erst.entries = (void*)KSEG02KSEG1(xhc_ERST);
	if (!pXhci->erst.entries)
		goto fail;

	dma = VA2PA((U32)xhc_ERST);

	ms_dbg("%s: alloc event ring at 0x%llx\n", __func__, (unsigned long long)dma);

	memset(pXhci->erst.entries, 0, sizeof(struct xhci_event_ring_seg_table_entry)*MS_MAX_ERST_SEGS);
	pXhci->erst.entry_count = MS_MAX_ERST_SEGS;
	pXhci->erst.dma_addr = dma;

	for (val = 0, pSeg = pXhci->event_ring->pFirst_seg; val < MS_MAX_ERST_SEGS; val++) {
		struct xhci_event_ring_seg_table_entry *entry = &pXhci->erst.entries[val];
		entry->ring_base_addr = pSeg->dma;
		entry->ring_seg_size = MAX_TRBS_IN_SEG;
		entry->reserved = 0;
		pSeg = pSeg->next;
	}

	val = xhci_readl(pXhci, &pXhci->intr_regs->u32ERSTSZ);
	val &= ERST_SZ_MASK;
	val |= MS_MAX_ERST_SEGS;
	xhci_writel(pXhci, val, &pXhci->intr_regs->u32ERSTSZ);

	val_64 = xhci_read_64(pXhci, &pXhci->intr_regs->u32ERSTBA);
	val_64 &= ERDP_MASK;
	val_64 |= (pXhci->erst.dma_addr & (u64) ~ERDP_MASK);
	xhci_write_64(pXhci, val_64, &pXhci->intr_regs->u32ERSTBA);

	ms_xhci_set_event_deq(pXhci);
	ms_xhci_print_interrupt_reg_set(pXhci, 0);

	for (i = 0; i < XHCI_MAX_SLOTS; ++i)
		pXhci->devs[i] = NULL;

	if (ms_scratchpad_alloc(pXhci))
		goto fail;

	return 0;

fail:
	ms_err("%s: fail to alloc memory\n", __func__);
	return -ENOMEM;
}

#define CTX_SIZE(_hcc) (HCC_64BYTE_CONTEXT_SIZE(_hcc) ? 64 : 32)

struct xhci_input_control_ctx inline *ms_xhci_get_input_control_ctx(struct xhci_container_ctx *ctx)
{
	if (ctx->type != CTX_TYPE_INPUT)
	{
		ms_err("%s: invalid ctx type: %d\n", __func__, ctx->type);
	}
	return (struct xhci_input_control_ctx *)ctx->pBuf;
}

struct xhci_slot_ctx *ms_xhci_get_slot_ctx(struct xhci_hcd *pXhci,
					struct xhci_container_ctx *ctx)
{
	if (ctx->type == CTX_TYPE_DEVICE)
		return (struct xhci_slot_ctx *)ctx->pBuf;

	return (struct xhci_slot_ctx *)
		(ctx->pBuf + CTX_SIZE(pXhci->hcc_params));
}

struct xhci_ep_ctx *ms_xhci_get_ep_ctx(struct xhci_hcd *pXhci,
				    struct xhci_container_ctx *ctx,
				    unsigned int ep_index)
{
	/* increment ep index by offset of start of ep ctx array */
	ep_index++;
	if (ctx->type == CTX_TYPE_INPUT)
		ep_index++;

	return (struct xhci_ep_ctx *)
		(ctx->pBuf + (ep_index * CTX_SIZE(pXhci->hcc_params)));
}

static struct xhci_container_ctx *ms_xhci_alloc_ctx(struct xhci_hcd *pXhci,
						    int type, gfp_t flags, void *pBuf)
{
	struct xhci_container_ctx *ctx = kzalloc(sizeof(*ctx), flags);
	if (!ctx)
		return NULL;

	if ((type != CTX_TYPE_DEVICE) && (type != CTX_TYPE_INPUT))
	{
		ms_err("%s: invalid cts type: %d\n", __func__, type);
	}
	ctx->type = type;
	ctx->size = HCC_64BYTE_CONTEXT_SIZE(pXhci->hcc_params) ? 2048 : 1024;
	if (type == CTX_TYPE_INPUT)
		ctx->size += CTX_SIZE(pXhci->hcc_params);

	//ctx->bytes = dma_pool_alloc(pXhci->device_pool, flags, &ctx->dma);
	ctx->pBuf = pBuf;
	ctx->dma_addr = VA2PA((U32)pBuf);
	ms_dbg("alloc context type: %d at 0x%llx(DMA)\n", type, ctx->dma_addr);

	memset(ctx->pBuf, 0, ctx->size);
	return ctx;
}

void ms_xhci_free_virt_dev(struct xhci_hcd *pXhci, int slot_id)
{
	struct xhci_my_device *pDev;
	int i;

	if (slot_id == 0 || !pXhci->devs[slot_id])
		return;

	pXhci->dcbaa->dev_ctx_bases[slot_id] = 0;

	pDev = pXhci->devs[slot_id];
	if (!pDev)
		return;

	for (i = 0; i < 31; ++i) {
		if (pDev->eps[i].ring)
			ms_xhci_ring_free(pDev->eps[i].ring);
	}


	if (pDev->in_ctx)
		kfree(pDev->in_ctx);

	if (pDev->out_ctx)
		kfree(pDev->out_ctx);

	kfree(pXhci->devs[slot_id]);
	pXhci->devs[slot_id] = NULL;
}

int ms_xhci_alloc_virt_dev(struct xhci_hcd *pXhci, int slot_id,
		struct usb_device *pUdev, gfp_t flags)
{
	struct xhci_my_device *dev;
	int i;

	if (slot_id == 0 || pXhci->devs[slot_id]) {
		ms_err("%s: bad slot id = %d\n", __func__, slot_id);
		return 0;
	}

	pXhci->devs[slot_id] = kzalloc(sizeof(struct xhci_my_device), flags);
	if (!pXhci->devs[slot_id])
		return 0;
	dev = pXhci->devs[slot_id];

	dev->out_ctx = ms_xhci_alloc_ctx(pXhci, CTX_TYPE_DEVICE, flags, (void*)KSEG02KSEG1(OutCTX[slot_id-1]));
	if (!dev->out_ctx)
		goto fail;

	ms_dbg("%s: slot %d out ctx = 0x%llx\n", __func__, slot_id,
			(unsigned long long)dev->out_ctx->dma_addr);

	dev->in_ctx = ms_xhci_alloc_ctx(pXhci, CTX_TYPE_INPUT, flags, (void*)KSEG02KSEG1(InCTX[slot_id-1]));
	if (!dev->in_ctx)
		goto fail;

	ms_dbg(pXhci, "%s: Slot %d input ctx = 0x%llx n", __func__, slot_id,
			(unsigned long long)dev->in_ctx->dma_addr);

	for (i = 0; i < 31; i++) {
		INIT_LIST_HEAD(&dev->eps[i].cancel_td_list);
	}

	dev->eps[0].ring = ms_xhci_ring_alloc(true, (void*)KSEG02KSEG1(EP0RingBuf[slot_id-1]));
	if (!dev->eps[0].ring)
		goto fail;

	init_completion(dev->cmd_completion);
	INIT_LIST_HEAD(&dev->cmd_list);
	dev->pUdev = pUdev;

	pXhci->dcbaa->dev_ctx_bases[slot_id] = cpu_to_le64(dev->out_ctx->dma_addr);
	ms_dbg("%s: slot id %d dcbaa entry = 0x%llx\n", __func__, slot_id,
		 (unsigned long long) le64_to_cpu(pXhci->dcbaa->dev_context_ptrs[slot_id]));

	return 1;
fail:
	ms_xhci_free_virt_dev(pXhci, slot_id);
	return 0;
}

struct xhci_my_cmd *ms_xhci_alloc_cmd(struct xhci_hcd *pXhci,
		bool is_alloc_completion,
		gfp_t mem_flags)
{
	struct xhci_my_cmd *pCmd;

	pCmd = kzalloc(sizeof(*pCmd), mem_flags);
	if (!pCmd)
		return NULL;

	if (is_alloc_completion) {
		init_completion(pCmd->completion);
	}

	pCmd->status = 0;
	INIT_LIST_HEAD(&pCmd->cmd_list);
	return pCmd;
}

int ms_xhci_setup_virt_dev(struct xhci_hcd *pXhci, struct usb_device *pUdev)
{
	struct xhci_my_device *pDev;
	struct xhci_ep_ctx	*ep0_ctx;
	struct xhci_slot_ctx    *slot_ctx;
	u32			port_num, tt_port_num;
	struct usb_device *pPdev;
	int 			ii;
	u32		slot_speed = 0;

	pDev = pXhci->devs[pUdev->slot_id];
	if (pUdev->slot_id == 0 || !pDev) {
		ms_err("%s: bad slot is %d\n", __func__, pUdev->slot_id);
		return -EINVAL;
	}
	ep0_ctx = ms_xhci_get_ep_ctx(pXhci, pDev->in_ctx, 0);
	slot_ctx = ms_xhci_get_slot_ctx(pXhci, pDev->in_ctx);

	slot_ctx->dev_slot_info1 |= cpu_to_le32(DEV_CTX_ENTRY(1) | (u32) pUdev->route);

	if (pUdev->speed == USB_SPEED_SUPER)
		slot_speed = (u32) SLOT_CTX_SPEED_SS;
	else if (pUdev->speed == USB_SPEED_HIGH)
		slot_speed = (u32) SLOT_CTX_SPEED_HS;
	else if (pUdev->speed == USB_SPEED_FULL)
		slot_speed = (u32) SLOT_CTX_SPEED_FS;
	else if (pUdev->speed == USB_SPEED_LOW)
		slot_speed = (u32) SLOT_CTX_SPEED_LS;

	slot_ctx->dev_slot_info1 |= slot_speed;

	port_num = pUdev->portnum + 1 ;

	if (!port_num)
		return -EINVAL;
	slot_ctx->dev_slot_info2 |= cpu_to_le32((u32) DEV_CTX_ROOT_HUB_PORT(port_num));

	ms_dbg("%s: root hub port num = %d\n", __func__, port_num);

	if ( pUdev->parent &&
		((pUdev->speed ==USB_SPEED_FULL) || (pUdev->speed ==USB_SPEED_LOW)) )
	{
		pPdev = pUdev->parent;
		for (ii=0; ii<pPdev->maxchild; ii++)
		{
			if (pPdev->children[ii] == pUdev)
				break;
		}

		if (ii < pPdev->maxchild)
			tt_port_num = ii + 1;
		else
			tt_port_num = 0;

		ms_dbg("tt port num: %d\n", tt_port_num);
		slot_ctx->dev_slot_tt_info = cpu_to_le32(pUdev->parent->slot_id |
						(tt_port_num<< 8));
	}

	ep0_ctx->ep_ctx_field2 = cpu_to_le32(EP_TYPE(EP_TYPE_CTRL));		//control endpoint
	if (pUdev->speed == USB_SPEED_SUPER)
		ep0_ctx->ep_ctx_field2 |= EP_MAX_PACKET_SIZE(512);
	else if ( (pUdev->speed == USB_SPEED_HIGH) || (pUdev->speed == USB_SPEED_FULL) )
		ep0_ctx->ep_ctx_field2 |= EP_MAX_PACKET_SIZE(64);
	else if (pUdev->speed == USB_SPEED_LOW)
		ep0_ctx->ep_ctx_field2 |= EP_MAX_PACKET_SIZE(8);

	ep0_ctx->ep_ctx_field2 |= cpu_to_le32(EP_MAX_BURST_SIZE(0) | EP_CERR(3));

	ep0_ctx->tr_deq_ptr = cpu_to_le64(pDev->eps[0].ring->pFirst_seg->dma |
				   pDev->eps[0].ring->cycle_state);

	return 0;
}

void ms_xhci_copy_ep0_deq_to_input_ctx(struct xhci_hcd *pXhci,
		struct usb_device *pUdev)
{
	struct xhci_my_device *pVirt_dev;
	struct xhci_ep_ctx	*pEp0_ctx;
	struct xhci_ring	*pEp_ring;

	pVirt_dev = pXhci->devs[pUdev->slot_id];
	pEp0_ctx = ms_xhci_get_ep_ctx(pXhci, pVirt_dev->in_ctx, 0);
	pEp_ring = pVirt_dev->eps[0].ring;
	pEp0_ctx->tr_deq_ptr = cpu_to_le64(ms_xhci_trb_to_dma(pEp_ring->pEnq_seg,
							pEp_ring->pEnq)
				   | pEp_ring->cycle_state);
}

void ms_xhci_free_urb_priv(struct urb_priv *pUrb_priv)
{
	int td_size;

	if (!pUrb_priv)
		return;

	td_size = pUrb_priv->len - 1;
	if (td_size >= 0) {
		int	i;
		for (i = 0; i <= td_size; i++)
			kfree(pUrb_priv->td[i]);
	}
	kfree(pUrb_priv);
}

void ms_xhci_ep_ctx_copy(struct xhci_hcd *pXhci,
		struct xhci_container_ctx *pIn_ctx,
		struct xhci_container_ctx *pOut_ctx,
		unsigned int ep_index)
{
	struct xhci_ep_ctx *pOut_ep_ctx;
	struct xhci_ep_ctx *pIn_ep_ctx;

	pOut_ep_ctx = ms_xhci_get_ep_ctx(pXhci, pOut_ctx, ep_index);
	pIn_ep_ctx = ms_xhci_get_ep_ctx(pXhci, pIn_ctx, ep_index);

	pIn_ep_ctx->ep_ctx_field1 = pOut_ep_ctx->ep_ctx_field1;
	pIn_ep_ctx->ep_ctx_field2 = pOut_ep_ctx->ep_ctx_field2;
	pIn_ep_ctx->tr_deq_ptr = pOut_ep_ctx->tr_deq_ptr;
	pIn_ep_ctx->avg_trb_len = pOut_ep_ctx->avg_trb_len;
}

static u32 ms_xhci_get_ept_type(struct usb_host_endpoint *pEpt)
{
	int is_in;
	u32 ept_type=0;

	is_in = is_usb_ept_dir_in(&pEpt->desc);
	if (is_usb_control_ept(&pEpt->desc)) {
		ept_type = EP_TYPE(EP_TYPE_CTRL);
	} else if (is_usb_bulk_ept(&pEpt->desc)) {
		if (is_in)
			ept_type = EP_TYPE(EP_TYPE_BULK_IN);
		else
			ept_type = EP_TYPE(EP_TYPE_BULK_OUT);
	} else if (is_usb_int_ept(&pEpt->desc)) {
		if (is_in)
			ept_type = EP_TYPE(EP_TYPE_INT_IN);
		else
			ept_type = EP_TYPE(EP_TYPE_INT_OUT);
	} else {
		ms_err("%s: unknow ept type\n", __func__);
	}
	return ept_type;
}

static unsigned int ms_xhci_parse_frame_interval(struct usb_host_endpoint *pEpt)
{
	unsigned int interval;

	interval = fls(8 * pEpt->desc.bInterval) - 1;

	if (interval < 3)
		interval = 3;
	else if (interval > 10)
		interval  = 10;

	return interval;
}

static unsigned int ms_xhci_parse_ept_interval(struct usb_device *pUdev,
		struct usb_host_endpoint *pEpt)
{
	unsigned int interval;

	interval = pEpt->desc.bInterval;
	if (interval < 1)
		interval = 1;
	else if (interval > 16)
		interval = 16;

	interval--;

	if (pUdev->speed == USB_SPEED_FULL) {
		interval += 3;	/* 1 frame = 2^3 uframes */
	}

	return interval;
}

 unsigned int ms_xhci_get_ept_interval(struct usb_device *pUdev,
		struct usb_host_endpoint *pEpt)
{
	unsigned int interval = 0;

	if ( (pUdev->speed == USB_SPEED_HIGH) || (pUdev->speed == USB_SPEED_SUPER) )
	{
		if ( (pUdev->speed == USB_SPEED_HIGH) &&
			(is_usb_control_ept(&pEpt->desc) ||is_usb_bulk_ept(&pEpt->desc)) )
			interval = pEpt->desc.bInterval;

		if (is_usb_int_ept(&pEpt->desc) ||is_usb_isoc_ept(&pEpt->desc))
			interval = ms_xhci_parse_ept_interval(pUdev, pEpt);
	}
	else if ( (pUdev->speed == USB_SPEED_FULL) || (pUdev->speed == USB_SPEED_LOW) )
	{
		if ( (pUdev->speed == USB_SPEED_FULL) && is_usb_isoc_ept(&pEpt->desc) )
			interval = ms_xhci_parse_ept_interval(pUdev, pEpt);

		if (is_usb_int_ept(&pEpt->desc) ||is_usb_isoc_ept(&pEpt->desc))
			interval = ms_xhci_parse_frame_interval(pEpt);
	}

	return EP_INTERVAL(interval);
}

int ms_xhci_ept_init(struct xhci_hcd *pXhci,
		struct xhci_my_device *pVirt_dev,
		struct usb_device *pUdev,
		struct usb_host_endpoint *pEpt,
		gfp_t mem_flags)
{
	unsigned int ep_index;
	struct xhci_ep_ctx *pEpt_ctx;
	struct xhci_ring *pEpt_ring;
	unsigned int max_packet;
	unsigned int max_burst;
	u32 max_esit_payload;
	int ep_num;

	ep_index = ms_xhci_get_ept_idx(&pEpt->desc);
	pEpt_ctx = ms_xhci_get_ep_ctx(pXhci, pVirt_dev->in_ctx, ep_index);

	ep_num = usb_ept_num(&pEpt->desc);
	if (ep_num > XHCI_MAX_BULK_NUM)
	{
		ms_err("%s: ep_num %d is over %d..\n", __func__, ep_num, XHCI_MAX_BULK_NUM);
	}

	{
		pVirt_dev->eps[ep_index].ring =
			ms_xhci_ring_alloc(true, (void*) KSEG02KSEG1(BulkRingBuf[pVirt_dev->pUdev->slot_id-1][ep_index-1]));
	}

	if (!pVirt_dev->eps[ep_index].ring)
		return -ENOMEM;

	pVirt_dev->eps[ep_index].skip = false;
	pEpt_ring = pVirt_dev->eps[ep_index].ring;
	pEpt_ctx->tr_deq_ptr = pEpt_ring->pFirst_seg->dma | pEpt_ring->cycle_state;
	pEpt_ctx->ep_ctx_field1 = ms_xhci_get_ept_interval(pUdev, pEpt);

	pEpt_ctx->ep_ctx_field2 = EP_CERR(3);
	pEpt_ctx->ep_ctx_field2 |= ms_xhci_get_ept_type(pEpt);

	switch (pUdev->speed) {
	case USB_SPEED_SUPER:
		max_packet = pEpt->desc.wMaxPacketSize;
		pEpt_ctx->ep_ctx_field2 |= EP_MAX_PACKET_SIZE(max_packet);
		max_packet = pEpt->ss_ep_comp.bMaxBurst;
		pEpt_ctx->ep_ctx_field2 |= EP_MAX_BURST_SIZE(max_packet);
		break;

	case USB_SPEED_HIGH:
		if (is_usb_isoc_ept(&pEpt->desc) ||
				is_usb_int_ept(&pEpt->desc))
		{
			max_burst = (le16_to_cpu(pEpt->desc.wMaxPacketSize)
				     & 0x1800) >> 11;
			pEpt_ctx->ep_ctx_field2 |= cpu_to_le32(EP_MAX_BURST_SIZE(max_burst));
		}
	case USB_SPEED_FULL:
	case USB_SPEED_LOW:
		max_packet = GET_MAX_PACKET_SIZE(pEpt->desc.wMaxPacketSize);
		pEpt_ctx->ep_ctx_field2 |= EP_MAX_PACKET_SIZE(max_packet);
		break;

	default:
		BUG();
	}

	max_esit_payload = 0; //Set 0 for control and bulk endpoint

	if (is_usb_control_ept(&pEpt->desc) && pXhci->hci_version == 0x100)
		pEpt_ctx->avg_trb_len |= EP_AVG_TRB_LENGTH(8);
	else
		pEpt_ctx->avg_trb_len |= EP_AVG_TRB_LENGTH(max_esit_payload);

	return 0;
}

void ms_xhci_ept_ctx_zero(struct xhci_hcd *pXhci,
		struct xhci_my_device *pVirt_dev,
		struct usb_host_endpoint *pEpt)
{
	unsigned int ep_index;
	struct xhci_ep_ctx *pEpt_ctx;

	ep_index = ms_xhci_get_ept_idx(&pEpt->desc);
	pEpt_ctx = ms_xhci_get_ep_ctx(pXhci, pVirt_dev->in_ctx, ep_index);

	memset(pEpt_ctx, 0, sizeof(struct xhci_ep_ctx));
}

void ms_xhci_free_cmd(struct xhci_my_cmd *pCmd)
{
	if (pCmd->pIn_ctx != NULL)
		kfree(pCmd->pIn_ctx);

	kfree(pCmd);
}

