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
#include "usbdesc.h"

//#define MS_DBG
#ifdef MS_DBG
#define ms_dbg(fmt, args...)   do { printf(fmt , ## args); } while (0)
#else
#define ms_dbg(fmt, args...)   do { } while (0)
#endif

#define ms_err(fmt, args...)   do { printf(fmt , ## args); } while (0)

#if XHCI_FLUSHPIPE_PATCH
extern void Chip_Flush_Memory(void);
extern void Chip_Read_Memory(void);
#endif
 u32 ms_xhci_td_remaining(unsigned int remainder);
 u32 ms_xhci_v1_0_td_remaining(int running_len, int trb_buf_len,
		unsigned int total_packets, struct urb *pUrb);
 extern struct xhci_ring *ms_xhci_urb_to_ring(struct xhci_hcd *pXhci,
		struct urb *pUrb);

struct xhci_seg *ms_chk_trb_in_td(struct xhci_seg *pStart_seg,
		union xhci_trb	*pStart_trb,
		union xhci_trb	*pEnd_trb,
		dma_addr_t	chk_dma);
int ms_finish_td(struct xhci_hcd *pXhci, struct xhci_td *pTd,
	union xhci_trb *pEvent_trb, struct xhci_transfer_event *pEvent,
	struct xhci_my_ep *pVirt_ep, int *status, bool skip);

int ms_has_room_on_ring(struct xhci_hcd *xhci, struct xhci_ring *ring,
		unsigned int num_trbs);

void ms_usb_hcd_giveback_urb( struct urb *pUrb, int status);

static int ms_finish_cmd_in_cmd_list(struct xhci_hcd *pXhci,
		struct xhci_my_device *pVirt_dev,
		struct xhci_event_cmd_completion *pEvent);
int ms_queue_set_tr_deq(struct xhci_hcd *pXhci, int slot_id,
		unsigned int ep_idx,
		struct xhci_seg *pDeq_seg,
		union xhci_trb *pDeq_trb, u32 u32Cycle);

extern unsigned long  gUsbStatusXHC;

dma_addr_t ms_xhci_trb_to_dma(struct xhci_seg *pSeg,
		union xhci_trb *pTrb)
{
	unsigned long uOffset;

	if (!pSeg || !pTrb || pTrb < pSeg->pTrbs)
		return 0;

	uOffset = pTrb - pSeg->pTrbs;
	if (uOffset > MAX_TRBS_IN_SEG)
		return 0;
	return pSeg->dma + (uOffset * sizeof(*pTrb));
}

static int ms_last_trb(struct xhci_hcd *pXhci, struct xhci_ring *pRing,
		struct xhci_seg *pSeg, union xhci_trb *pTrb)
{
	if (pRing == pXhci->event_ring)
		return pTrb == &pSeg->pTrbs[MAX_TRBS_IN_SEG];
	else
		return (pTrb->link.trb_info & TRB_TYPE_MASK)
			== TRB_TYPE(TRB_TYPE_LINK);
}

static int ms_enq_is_link_trb(struct xhci_ring *pRing)
{
	struct xhci_link_trb *pLink = &pRing->pEnq->link;
	return ((le32_to_cpu(pLink->trb_info) & TRB_TYPE_MASK) ==
		TRB_TYPE(TRB_TYPE_LINK));
}

static void ms_next_trb(struct xhci_hcd *pXhci,
		struct xhci_ring *pRing,
		struct xhci_seg **pSeg,
		union xhci_trb **pTrb)
{
	if (ms_last_trb(pXhci, pRing, *pSeg, *pTrb)) {
		*pSeg = (*pSeg)->next;
		*pTrb = ((*pSeg)->pTrbs);
	} else {
		(*pTrb)++;
	}
}

static void ms_inc_deq(struct xhci_hcd *pXhci, struct xhci_ring *pRing, bool isConsumer)
{
	union xhci_trb *pNext = ++(pRing->pDeq);

	while (ms_last_trb(pXhci, pRing, pRing->pDeq_seg, pNext))
	{
		if (isConsumer)
		{
			pRing->cycle_state = (pRing->cycle_state ? 0 : 1);
			ms_dbg("ring %p toggle cycle to %i\n", pRing,
						(unsigned int) pRing->cycle_state);
		}
		pRing->pDeq_seg = pRing->pDeq_seg->next;
		pRing->pDeq = pRing->pDeq_seg->pTrbs;
		pNext = pRing->pDeq;
	}
}

int ms_prepare_ring(struct xhci_hcd *pXhci, struct xhci_ring *pEp_ring,
		u32 u32Ep_state, unsigned int num_trbs)
{
	switch (u32Ep_state) {
	case EP_STATE_DISABLED:
		ms_err("%s: ept state is disabled\n", __func__);
		return -ENOENT;

	case EP_STATE_ERROR:
		ms_err("%s: ept state is error\n", __func__);
		return -EINVAL;

	case EP_STATE_HALTED:
		ms_dbg("%s: ept state is halted\n", __func__);
	case EP_STATE_STOPPED:
	case EP_STATE_RUNNING:
		break;

	default:
		ms_err("%s:unknown ept state %d\n", __func__, u32Ep_state);
		return -EINVAL;
	}

	if (!ms_has_room_on_ring(pXhci, pEp_ring, num_trbs)) {
		ms_err("%s: no room on ep ring\n", __func__);
		return -ENOMEM;
	}

	if (ms_enq_is_link_trb(pEp_ring)) {
		struct xhci_ring *ring = pEp_ring;
		union xhci_trb *next;

		next = ring->pEnq;

		while (ms_last_trb(pXhci, ring, ring->pEnq_seg, next))
		{
			next->link.trb_info |= cpu_to_le32(TRB_CHAIN);
			next->link.trb_info ^= cpu_to_le32((u32) TRB_CYCLE);

			//toggle the cycle bit
			ring->cycle_state = (ring->cycle_state ? 0 : 1);
			ms_dbg("ring %p toggle cycle to %d\n", ring,
				(unsigned int)ring->cycle_state);

			ring->pEnq_seg = ring->pEnq_seg->next;
			ring->pEnq = ring->pEnq_seg->pTrbs;
			next = ring->pEnq;
		}
	}

	return 0;
}

int ms_prepare_transfer(struct xhci_hcd *pXhci,
		struct xhci_my_device *pVirt_dev,
		unsigned int ep_index,
		unsigned int stream_id,
		unsigned int num_trbs,
		struct urb *pUrb,
		unsigned int td_index)
{
	int retval;
	struct urb_priv *pUrb_priv;
	struct xhci_td	*pTd;
	struct xhci_ring *pEp_ring;
	struct xhci_ep_ctx *pEp_ctx = ms_xhci_get_ep_ctx(pXhci, pVirt_dev->out_ctx, ep_index);
	struct xhci_my_ep *pVirt_ep = &pVirt_dev->eps[ep_index];

	pEp_ring=pVirt_ep->ring;
	if (!pEp_ring) {
		ms_err("%s: pEp_ring = NULL\n", __func__);
		return -EINVAL;
	}

	retval = ms_prepare_ring(pXhci, pEp_ring,
			   le32_to_cpu(pEp_ctx->ep_ctx_field1) & EP_STATE_MASK,
			   num_trbs);
	if (retval)
		return retval;
	pUrb_priv = pUrb->hcpriv;
	pTd = pUrb_priv->td[td_index];

	INIT_LIST_HEAD(&pTd->td_list);
	INIT_LIST_HEAD(&pTd->cancel_td_list);

	pTd->pUrb = pUrb;

	list_add_tail(&pTd->td_list, &pEp_ring->td_list);
	pTd->pStart_seg = pEp_ring->pEnq_seg;
	pTd->pFirst_trb = pEp_ring->pEnq;

	pUrb_priv->td[td_index] = pTd;

	return 0;
}

static void ms_inc_enq(struct xhci_hcd *pXhci, struct xhci_ring *pRing,
		bool is_consumer, bool is_more_trbs)
{
	u32 chain;
	union xhci_trb *pNext;

	chain = le32_to_cpu(pRing->pEnq->generic.trb_info[3]) & TRB_CHAIN;
	pNext = ++(pRing->pEnq);

	while (ms_last_trb(pXhci, pRing, pRing->pEnq_seg, pNext)) {
		if (!is_consumer) {
			if (pRing != pXhci->event_ring) {
				if (!chain && !is_more_trbs)
					break;

				pNext->link.trb_info &=
					cpu_to_le32(~TRB_CHAIN);
				pNext->link.trb_info |=
					cpu_to_le32(chain);

				pNext->link.trb_info ^= cpu_to_le32(TRB_CYCLE);
			}

			pRing->cycle_state = (pRing->cycle_state ? 0 : 1);
			ms_dbg("ring %p toggle cycle to %d\n",
					pRing,
					(unsigned int) pRing->cycle_state);
		}
		pRing->pEnq_seg = pRing->pEnq_seg->next;
		pRing->pEnq = pRing->pEnq_seg->pTrbs;
		pNext = pRing->pEnq;
	}
}

int ms_has_room_on_ring(struct xhci_hcd *pXhci, struct xhci_ring *pRing,
		unsigned int num_trbs)
{
	int i;
	union xhci_trb *pEnq = pRing->pEnq;
	struct xhci_seg *pEnq_seg = pRing->pEnq_seg;
	unsigned int free_trbs;

	while (ms_last_trb(pXhci, pRing, pEnq_seg, pEnq)) {
		pEnq_seg = pEnq_seg->next;
		pEnq = pEnq_seg->pTrbs;
	}

	if (pEnq == pRing->pDeq)
	{
		free_trbs = MAX_TRBS_IN_SEG 	 - 1;
		free_trbs -= 1;
		if (num_trbs > free_trbs) {
			ms_err("%s: not enough room on ring %d (left=%d)\n",
					__func__, num_trbs, free_trbs);
			return 0;
		}
		return 1;
	}

	for (i = 0; i <= num_trbs; ++i) {
		if (pEnq == pRing->pDeq)
			return 0;
		pEnq++;
		while (ms_last_trb(pXhci, pRing, pEnq_seg, pEnq)) {
			pEnq_seg = pEnq_seg->next;
			pEnq = pEnq_seg->pTrbs;
		}
	}

	return 1;
}

void ms_xhci_ring_cmd_db(struct xhci_hcd *pXhci)
{
	ms_dbg("!!Ring cmd doorbell!!\n");

#if XHCI_FLUSHPIPE_PATCH
	Chip_Flush_Memory();
#endif

	xhci_writel(pXhci, XHCI_DB_VALUE_HOST, &pXhci->doorbells->u32DoorBellReg[0]);
	xhci_readl(pXhci, &pXhci->doorbells->u32DoorBellReg[0]);
}

void ms_xhci_ring_ep_db(struct xhci_hcd *pXhci,
		unsigned int slot_id,
		unsigned int ep_idx)
{
	__le32 __iomem *pEp_db = &pXhci->doorbells->u32DoorBellReg[slot_id];
	unsigned int ep_state = pXhci->devs[slot_id]->eps[ep_idx].ep_state;

	if ((ep_state & EP_ST_HALT_PENDING) || (ep_state & EP_ST_SET_DEQ_PENDING) ||
	    (ep_state & EP_ST_HALTED))
		return;
	xhci_writel(pXhci, XHCI_DB_VALUE(ep_idx, 0), pEp_db);
}

static void ms_ring_db_for_active(struct xhci_hcd *pXhci,
		unsigned int slot_id,
		unsigned int ep_idx)
{
	struct xhci_my_ep *pVirt_ep;

#if XHCI_FLUSHPIPE_PATCH
	Chip_Flush_Memory();
#endif

	pVirt_ep = &pXhci->devs[slot_id]->eps[ep_idx];

	if (!(list_empty(&pVirt_ep->ring->td_list)))
		ms_xhci_ring_ep_db(pXhci, slot_id, ep_idx);

	return;
}

static struct xhci_seg *ms_find_trb_seg(
		struct xhci_seg *pStart_seg,
		union xhci_trb	*pTrb)
{
	if ( (pStart_seg->pTrbs <= pTrb) && (pTrb <= &pStart_seg->pTrbs[MAX_TRBS_IN_SEG - 1]) )
		return pStart_seg;
	else
		return NULL;
}

void ms_xhci_get_new_deq_state(struct xhci_hcd *pXhci,
		unsigned int slot_id, unsigned int ep_idx,
		struct xhci_td *pTd,
		struct xhci_deq_state *pDeq_state)
{
	struct xhci_my_device *pVirt_dev = pXhci->devs[slot_id];
	struct xhci_ring *pEp_ring;
	struct xhci_generic_trb *pTrb;
	struct xhci_ep_ctx *pEp_ctx;

	pEp_ring = pXhci->devs[slot_id]->eps[ep_idx].ring;
	pDeq_state->cycle_state = 0;
	ms_dbg("find segment containing stopped trb.\n");
	pDeq_state->pDeq_seg = ms_find_trb_seg(pTd->pStart_seg,
			pVirt_dev->eps[ep_idx].pStop_trb);
	if (!pDeq_state->pDeq_seg) {
		ms_err("%s: can't get seg of stopped trb\n", __func__);
		return;
	}

	pEp_ctx = ms_xhci_get_ep_ctx(pXhci, pVirt_dev->out_ctx, ep_idx);
	pDeq_state->cycle_state = 0x1 & le64_to_cpu(pEp_ctx->tr_deq_ptr);

	pDeq_state->pDeq_ptr = pTd->pLast_trb;
	ms_dbg("find segment containing last trb.\n");
	pDeq_state->pDeq_seg = ms_find_trb_seg(pDeq_state->pDeq_seg,
			pDeq_state->pDeq_ptr);
	if (!pDeq_state->pDeq_seg) {
		ms_err("%s: can't get seg of last trb\n", __func__);
		return;
	}

	pTrb = &pDeq_state->pDeq_ptr->generic;
	if ((le32_to_cpu(pTrb->trb_info[3]) & TRB_TYPE_MASK) ==
	    TRB_TYPE(TRB_TYPE_LINK) && (le32_to_cpu(pTrb->trb_info[3]) & LINK_TOGGLE))
		pDeq_state->cycle_state ^= 0x1;
	ms_next_trb(pXhci, pEp_ring, &pDeq_state->pDeq_seg, &pDeq_state->pDeq_ptr);

	if (pEp_ring->pFirst_seg == pEp_ring->pFirst_seg->next &&
			pDeq_state->pDeq_ptr < pVirt_dev->eps[ep_idx].pStop_trb)
		pDeq_state->cycle_state ^= 0x1;

	ms_dbg("new dequence: %p (seg), %llx (deq DMA), %x (cycle)\n",
		pDeq_state->pDeq_seg,
		(unsigned long long) ms_xhci_trb_to_dma(pDeq_state->pDeq_seg, pDeq_state->pDeq_ptr),
		pDeq_state->cycle_state);
}

void ms_xhci_set_new_deq_state(struct xhci_hcd *pXhci,
		unsigned int slot_id, unsigned int ep_idx,
		struct xhci_deq_state *pDeq_state)
{
	ms_queue_set_tr_deq(pXhci, slot_id, ep_idx,
			pDeq_state->pDeq_seg,
			pDeq_state->pDeq_ptr,
			(u32) pDeq_state->cycle_state);

	pXhci->devs[slot_id]->eps[ep_idx].ep_state |= EP_ST_SET_DEQ_PENDING;
}

static void ms_xhci_td_to_noop(struct xhci_hcd *pXhci, struct xhci_ring *pEp_ring,
		struct xhci_td *pTd, bool is_flip_cycle)
{
	struct xhci_seg *pCur_seg;
	union xhci_trb *pCur_trb;

	for (pCur_seg = pTd->pStart_seg, pCur_trb = pTd->pFirst_trb;
			true;
			ms_next_trb(pXhci, pEp_ring, &pCur_seg, &pCur_trb)) {
		if ((le32_to_cpu(pCur_trb->generic.trb_info[3]) & TRB_TYPE_MASK)
		    == TRB_TYPE(TRB_TYPE_LINK))
		{
			pCur_trb->generic.trb_info[3] &= cpu_to_le32(~TRB_CHAIN);
			if (is_flip_cycle)
				pCur_trb->generic.trb_info[3] ^= cpu_to_le32(TRB_CYCLE);

			ms_dbg("unchain link trb\n");
		}
		else
		{
			pCur_trb->generic.trb_info[0] = 0;
			pCur_trb->generic.trb_info[1] = 0;
			pCur_trb->generic.trb_info[2] = 0;
			pCur_trb->generic.trb_info[3] &= cpu_to_le32(TRB_CYCLE);

			if (is_flip_cycle && pCur_trb != pTd->pFirst_trb &&
					pCur_trb != pTd->pLast_trb)
				pCur_trb->generic.trb_info[3] ^=
					cpu_to_le32(TRB_CYCLE);
			pCur_trb->generic.trb_info[3] |= cpu_to_le32(
				TRB_TYPE(TRB_TYPE_NOOP));

			ms_dbg("set trb %p (%llx DMA) to noop\n",
				pCur_trb, (unsigned long long)ms_xhci_trb_to_dma(pCur_seg, pCur_trb));
		}
		if (pCur_trb == pTd->pLast_trb)
			break;
	}
}

static void ms_xhci_giveback_urb(struct xhci_td *pCur_td, int status)
{
	struct urb	*pUrb;
	struct urb_priv	*pUrb_priv;

	pUrb = pCur_td->pUrb;
	pUrb_priv = pUrb->hcpriv;
	pUrb_priv->td_count++;

	if (pUrb_priv->td_count == pUrb_priv->len)
	{
		ms_usb_hcd_giveback_urb(pUrb, status);
		ms_xhci_free_urb_priv(pUrb_priv);
	}
}

static void ms_stopped_ept_completion(struct xhci_hcd *pXhci,
		union xhci_trb *pTrb, struct xhci_event_cmd_completion *pEvent)
{
	unsigned int slot_id;
	unsigned int ep_index;
	struct xhci_my_device *pVirt_dev;
	struct xhci_ring *pEp_ring;
	struct xhci_my_ep *pVirt_ep;
	struct list_head *entry;
	struct xhci_td *pCur_td = NULL;
	struct xhci_td *pLast_td;

	struct xhci_deq_state deq_state;

	if (le32_to_cpu(pXhci->cmd_ring->pDeq->generic.trb_info[3]) & (1<<23) ) //stop on suspend endpoint
	{
		slot_id = GET_TRB_SLOT_ID(
			le32_to_cpu(pXhci->cmd_ring->pDeq->generic.trb_info[3]));
		pVirt_dev = pXhci->devs[slot_id];
		if (pVirt_dev)
			ms_finish_cmd_in_cmd_list(pXhci, pVirt_dev, pEvent);
		else
			ms_err("%s: stop ept cmd on disabled slot %d\n", __func__, slot_id);

		return;
	}

	memset(&deq_state, 0, sizeof(deq_state));
	slot_id = GET_TRB_SLOT_ID(le32_to_cpu(pTrb->generic.trb_info[3]));
	ep_index = GET_TRB_EP_INDEX(le32_to_cpu(pTrb->generic.trb_info[3]));
	pVirt_ep = &pXhci->devs[slot_id]->eps[ep_index];

	list_for_each(entry, &pVirt_ep->cancel_td_list) {
		pCur_td = list_entry(entry, struct xhci_td, cancel_td_list);
		ms_dbg("cancel td %p (%xllx DMA)\n",
				pCur_td->pFirst_trb,
				(unsigned long long)ms_xhci_trb_to_dma(pCur_td->pStart_seg, pCur_td->pFirst_trb));
		pEp_ring = ms_xhci_urb_to_ring(pXhci, pCur_td->pUrb);
		if (!pEp_ring)
		{
			ms_err("%s: pEp_ring = NULL\n", __func__);
			list_del_init(&pCur_td->td_list);
			continue;
		}

		if (pCur_td == pVirt_ep->pStop_td)
			ms_xhci_get_new_deq_state(pXhci, slot_id, ep_index,
					pCur_td, &deq_state);
		else
			ms_xhci_td_to_noop(pXhci, pEp_ring, pCur_td, false);

		list_del_init(&pCur_td->td_list);
	}
	pLast_td = pCur_td;

	if (deq_state.pDeq_ptr && deq_state.pDeq_seg) {
		ms_xhci_set_new_deq_state(pXhci,
				slot_id, ep_index,
				&deq_state);
		ms_xhci_ring_cmd_db(pXhci);
	}
	else
	{
		ms_ring_db_for_active(pXhci, slot_id, ep_index);
	}
	pVirt_ep->pStop_td = NULL;
	pVirt_ep->pStop_trb = NULL;

	do {
		pCur_td = list_entry(pVirt_ep->cancel_td_list.next,
				struct xhci_td, cancel_td_list);
		list_del_init(&pCur_td->cancel_td_list);

		ms_xhci_giveback_urb(pCur_td, 0);
	} while (pCur_td != pLast_td);
}

static void ms_set_deq_completion(struct xhci_hcd *pXhci,
		struct xhci_event_cmd_completion *pEvent,
		union xhci_trb *pTrb)
{
	unsigned int slot_id;
	unsigned int ep_index;
	struct xhci_ring *pEp_ring;
	struct xhci_my_device *pVirt_dev;
	struct xhci_ep_ctx *pEp_ctx;
	struct xhci_slot_ctx *pSlot_ctx;

	slot_id = GET_TRB_SLOT_ID(pTrb->generic.trb_info[3]);
	ep_index = GET_TRB_EP_INDEX(pTrb->generic.trb_info[3]);
	pVirt_dev = pXhci->devs[slot_id];

	pEp_ring = pVirt_dev->eps[ep_index].ring;
	if (!pEp_ring) {
		ms_err("%s: pEp_ring = NULL\n", __func__);

		pVirt_dev->eps[ep_index].ep_state &= ~EP_ST_SET_DEQ_PENDING;
		return;
	}

	pEp_ctx = ms_xhci_get_ep_ctx(pXhci, pVirt_dev->out_ctx, ep_index);
	pSlot_ctx = ms_xhci_get_slot_ctx(pXhci, pVirt_dev->out_ctx);

	if (GET_COMP_CODE(pEvent->status) != TRB_COMP_SUCCESS) {
		ms_err("%s: fail to set deq (err=%d)\n",
			__func__, GET_COMP_CODE(pEvent->status) );
	} else {
		ms_dbg("success to set deq to %llx\n", pEp_ctx->tr_deq_ptr);
		if (ms_xhci_trb_to_dma(pVirt_dev->eps[ep_index].set_deq_seg,
					 pVirt_dev->eps[ep_index].set_deq_ptr) ==
		    (pEp_ctx->tr_deq_ptr & ~(EP_DCS_MASK))) {
			pEp_ring->pDeq_seg = pVirt_dev->eps[ep_index].set_deq_seg;
			pEp_ring->pDeq = pVirt_dev->eps[ep_index].set_deq_ptr;
		} else {
			ms_err("%s: can't find deq in ept ctx\n", __func__);
		}
	}

	pVirt_dev->eps[ep_index].ep_state &= ~EP_ST_SET_DEQ_PENDING;
	pVirt_dev->eps[ep_index].set_deq_seg = NULL;
	pVirt_dev->eps[ep_index].set_deq_ptr = NULL;
	ms_ring_db_for_active(pXhci, slot_id, ep_index);
}

static void ms_reset_ep_completion(struct xhci_hcd *pXhci,
		struct xhci_event_cmd_completion *pEvent,
		union xhci_trb *pTrb)
{
	int slot_id;
	unsigned int ep_index;

	slot_id = GET_TRB_SLOT_ID(le32_to_cpu(pTrb->generic.trb_info[3]));
	ep_index = GET_TRB_EP_INDEX(le32_to_cpu(pTrb->generic.trb_info[3]));

	pXhci->devs[slot_id]->eps[ep_index].ep_state &= ~EP_ST_HALTED;
	ms_ring_db_for_active(pXhci, slot_id, ep_index);
}

static int ms_finish_cmd_in_cmd_list(struct xhci_hcd *pXhci,
		struct xhci_my_device *pVirt_dev,
		struct xhci_event_cmd_completion *pEvent)
{
	struct xhci_my_cmd *pCmd;

	if (list_empty(&pVirt_dev->cmd_list))
		return 0;

	pCmd = list_entry(pVirt_dev->cmd_list.next,
			struct xhci_my_cmd, cmd_list);

	if (pXhci->cmd_ring->pDeq != pCmd->pCmd_trb)
		return 0;

	pCmd->status = GET_COMP_CODE(le32_to_cpu(pEvent->status));
	list_del(&pCmd->cmd_list);

	if (pCmd->completion)
		complete(pCmd->completion);
	else
		ms_xhci_free_cmd(pCmd);

	return 1;
}

static void ms_cmd_completion(struct xhci_hcd *pXhci,
		struct xhci_event_cmd_completion *pEvent)
{
	int slot_id = GET_TRB_SLOT_ID(pEvent->trb_info);
	u64 cmd_dma;
	dma_addr_t cmd_dequeue_dma;
	struct xhci_input_control_ctx *pCtrl_ctx;
	struct xhci_my_device *pVirt_dev;
	unsigned int ep_index;

	cmd_dma = pEvent->cmd_trb_ptr;
	cmd_dequeue_dma = ms_xhci_trb_to_dma(pXhci->cmd_ring->pDeq_seg,
			pXhci->cmd_ring->pDeq);

	if (cmd_dequeue_dma == 0) {
		pXhci->error_bitmask |= 1 << 4;
		return;
	}

	if (cmd_dma != (u64) cmd_dequeue_dma) {
		pXhci->error_bitmask |= 1 << 5;
		return;
	}

	switch (pXhci->cmd_ring->pDeq->generic.trb_info[3] & TRB_TYPE_MASK)
	{
		case TRB_TYPE(TRB_TYPE_ENABLE_SLOT):
			ms_dbg("enable slot %d complete (ret=%x)\n",slot_id, GET_COMP_CODE(pEvent->status));
			if (GET_COMP_CODE(pEvent->status) == TRB_COMP_SUCCESS)
				pXhci->slot_id = slot_id;
			else
				pXhci->slot_id = 0;

			pXhci->addr_dev = 1;				//enable slot -> addr is assigned
			break;

		case TRB_TYPE(TRB_TYPE_DISABLE_SLOT):
			ms_dbg("disable slot %d complete (ret=%x)\n", slot_id, GET_COMP_CODE(pEvent->status));
			if (pXhci->devs[slot_id]) {
				ms_xhci_free_virt_dev(pXhci, slot_id);
			}
			break;

		case TRB_TYPE(TRB_TYPE_CONFIG_EP):
			ms_dbg("config ept complete (ret=%x)\n", GET_COMP_CODE(pEvent->status));
			pVirt_dev = pXhci->devs[slot_id];
			if (ms_finish_cmd_in_cmd_list(pXhci, pVirt_dev, pEvent))
				break;

			pCtrl_ctx = ms_xhci_get_input_control_ctx(pVirt_dev->in_ctx);
			ep_index = ms_xhci_last_valid_ept_idx(pCtrl_ctx->add_ctx_flags) - 1;

			//bandwidth_change:
			pXhci->devs[slot_id]->cmd_status = GET_COMP_CODE(pEvent->status);
			ms_dbg("complete config ept cmd\n");
			complete(pXhci->devs[slot_id]->cmd_completion);
			break;

		case TRB_TYPE(TRB_TYPE_EVAL_CONTEXT):
			ms_dbg("evaluate ctx complete (ret=%x)\n", GET_COMP_CODE(pEvent->status));
			pVirt_dev = pXhci->devs[slot_id];
			if (ms_finish_cmd_in_cmd_list(pXhci, pVirt_dev, pEvent))
				break;

			pXhci->devs[slot_id]->cmd_status = GET_COMP_CODE(pEvent->status);
			ms_dbg("complete eval ctx cmd\n");
			complete(pXhci->devs[slot_id]->cmd_completion);
			break;

		case TRB_TYPE(TRB_TYPE_ADDRESS_DEVICE):
			ms_dbg("address dev complete (ret=%x)\n", GET_COMP_CODE(pEvent->status));
			pXhci->devs[slot_id]->cmd_status = GET_COMP_CODE(pEvent->status);
			pXhci->addr_dev = 1;
			break;

#ifndef TEMP_DISABLE_FOR_DEVELOP
		case TRB_TYPE(TRB_TYPE_CMD_NOOP):
			break;
#endif

		case TRB_TYPE(TRB_TYPE_STOP_RING):
			ms_dbg("stop ring complete (ret=%x)\n", GET_COMP_CODE(pEvent->status));
			ms_stopped_ept_completion(pXhci, pXhci->cmd_ring->pDeq, pEvent);
			pXhci->kill_urb_done = 1;
			break;

		case TRB_TYPE(TRB_TYPE_SET_TR_DEQ_PTR):
			ms_dbg("set deq complete (ret=%x)\n", GET_COMP_CODE(pEvent->status));
			ms_set_deq_completion(pXhci, pEvent, pXhci->cmd_ring->pDeq);
			break;

		case TRB_TYPE(TRB_TYPE_RESET_EP):
			ms_dbg("reset ept complete (ret=%x)\n", GET_COMP_CODE(pEvent->status));
			ms_reset_ep_completion(pXhci, pEvent, pXhci->cmd_ring->pDeq);
			pXhci->devs[slot_id]->reset_ep_done = 1;
			break;

		case TRB_TYPE(TRB_TYPE_RESET_DEV):
			ms_dbg("reset dev complete (ret=%x)\n", GET_COMP_CODE(pEvent->status));
			slot_id = GET_TRB_SLOT_ID(pXhci->cmd_ring->pDeq->generic.trb_info[3]);
			pVirt_dev = pXhci->devs[slot_id];
			if (pVirt_dev)
				ms_finish_cmd_in_cmd_list(pXhci, pVirt_dev, pEvent);
			else
				ms_err("%s: pVirt_dev = NULL in reset dev complete\n", __func__);
			break;

		default:
			ms_dbg("unkonow cmd (ret=%x)\n", GET_COMP_CODE(pEvent->status));
			pXhci->error_bitmask |= 1 << 6;
			break;
	}
	ms_inc_deq(pXhci, pXhci->cmd_ring, false);
}

int ms_xhci_is_vendor_code(unsigned int comp_code)
{
	if (comp_code >= 224 && comp_code <= 255) {
		ms_dbg("completion code %d is vendor code\n", comp_code);
		return 1;
	}
	return 0;
}

int ms_xhci_need_halt_cleanup(struct xhci_ep_ctx *pEp_ctx,
		unsigned int comp_code)
{
	if (comp_code == TRB_COMP_TRANS_ERR ||
			comp_code == TRB_COMP_BABBLE_ERR ||
			comp_code == TRB_COMP_SPLIT_TRANS_ERR)
		if ((pEp_ctx->ep_ctx_field1 & EP_STATE_MASK) == EP_STATE_HALTED)
		{
			ms_dbg("manual cleanup halt to ept (comp_code=%d)\n", comp_code);
			return 1;
		}

	return 0;
}

void ms_xhci_cleanup_halt_ept(struct xhci_hcd *pXhci,
		unsigned int slot_id, unsigned int ep_idx,
		struct xhci_td *pTd, union xhci_trb *pEvent_trb)
{
	struct xhci_my_ep *pEpt = &pXhci->devs[slot_id]->eps[ep_idx];

	pEpt->ep_state |= EP_ST_HALTED;
	pEpt->pStop_td = pTd;
	pEpt->pStop_trb = pEvent_trb;

	ms_xhci_queue_reset_ept_cmd(pXhci, slot_id, ep_idx);
	ms_xhci_clear_stalled_ring(pXhci, pTd->pUrb->dev, ep_idx);

	pEpt->pStop_td = NULL;
	pEpt->pStop_trb = NULL;

	ms_xhci_ring_cmd_db(pXhci);
}

int ms_process_ctrl_td(struct xhci_hcd *pXhci, struct xhci_td *pTd,
	union xhci_trb *pEvent_trb, struct xhci_transfer_event *pEvent,
	struct xhci_my_ep *pVirt_ep, int *status)
{
	struct xhci_my_device *pVirt_dev;
	struct xhci_ring *pEp_ring;
	unsigned int slot_id;
	int ep_idx;
	struct xhci_ep_ctx *pEp_ctx;
	u32 comp_code;

	slot_id = GET_TRB_SLOT_ID(pEvent->trb_info);
	pVirt_dev = pXhci->devs[slot_id];
	ep_idx = TRB_TO_EP_ID(pEvent->trb_info) - 1;
	pEp_ring = pVirt_ep->ring;
	pEp_ctx = ms_xhci_get_ep_ctx(pXhci, pVirt_dev->out_ctx, ep_idx);
	comp_code = GET_COMP_CODE(pEvent->trb_transfer_len);

	ms_xhci_dump_trb(pXhci, pXhci->event_ring->pDeq);

	ms_dbg("ctrl td comp code: %d\n", comp_code);

	switch (comp_code) {
	case TRB_COMP_SUCCESS:
		if (pEvent_trb == pEp_ring->pDeq) {
			ms_err("%s: event trb == ept ring deq\n", __func__);
			*status = -ESHUTDOWN;
		} else if (pEvent_trb != pTd->pLast_trb) {
			ms_err("%s: event trb != last trb\n", __func__);
			*status = -ESHUTDOWN;
		} else {
			*status = 0;
		}
		break;

	case TRB_COMP_SHORT_PACKET:
		ms_err("%s: short tx on control ept\n", __func__);
		if (pTd->pUrb->transfer_flags & URB_SHORT_NOT_OK)
			*status = -EREMOTEIO;
		else
			*status = 0;
		break;

	case TRB_COMP_STOP_LENGTH_INVAL:
	case TRB_COMP_STOPPED:
		return ms_finish_td(pXhci, pTd, pEvent_trb, pEvent, pVirt_ep, status, false);
	default:
		if (!ms_xhci_need_halt_cleanup(pEp_ctx, comp_code))
			break;

		ms_dbg("ctrl trb completion code %d on ept index %d\n", comp_code, ep_idx);
	case TRB_COMP_STALL_ERR:
		if (pEvent_trb != pEp_ring->pDeq &&
				pEvent_trb != pTd->pLast_trb)
			pTd->pUrb->actual_length =
				pTd->pUrb->transfer_buffer_length
				- TRB_TRANSFER_LEN(pEvent->trb_transfer_len);
		else
			pTd->pUrb->actual_length = 0;

		ms_xhci_cleanup_halt_ept(pXhci,
			slot_id, ep_idx, pTd, pEvent_trb);
		return ms_finish_td(pXhci, pTd, pEvent_trb, pEvent, pVirt_ep, status, true);
	}

	if (pEvent_trb != pEp_ring->pDeq) {
		if (pEvent_trb == pTd->pLast_trb) {
			if (pTd->pUrb->actual_length != 0)
			{
				if ((*status == -EINPROGRESS || *status == 0) &&
						(pTd->pUrb->transfer_flags
						 & URB_SHORT_NOT_OK))
					*status = -EREMOTEIO;
			}
			else
				pTd->pUrb->actual_length = pTd->pUrb->transfer_buffer_length;
		}
		else
		{
			pTd->pUrb->actual_length =
				pTd->pUrb->transfer_buffer_length -
				TRB_TRANSFER_LEN(pEvent->trb_transfer_len);
			ms_dbg("waiting status stage event\n");

			return 0;
		}
	}

	return ms_finish_td(pXhci, pTd, pEvent_trb, pEvent, pVirt_ep, status, false);
}

int ms_process_bulk_intr_td(struct xhci_hcd *pXhci, struct xhci_td *pTd,
	union xhci_trb *pEvent_trb, struct xhci_transfer_event *pEvent,
	struct xhci_my_ep *pEpt, int *status)
{
	struct xhci_ring *pEp_ring;
	union xhci_trb *pCur_trb;
	struct xhci_seg *pCur_seg;
	u32 comp_code;

	pEp_ring = pEpt->ring;
	comp_code = GET_COMP_CODE(pEvent->trb_transfer_len);

	if (comp_code == TRB_COMP_SUCCESS)
	{
		*status = 0;
		if (pEvent_trb != pTd->pLast_trb) {
			ms_dbg("transfer successfully but pEvent_trb != pTd->last_trb\n");
			if (pTd->pUrb->transfer_flags & URB_SHORT_NOT_OK)
				*status = -EREMOTEIO;
		}
	}
	else if (comp_code == TRB_COMP_SHORT_PACKET)
	{
		ms_dbg("short transfer %d (ask %d)\n",
			TRB_TRANSFER_LEN(pEvent->trb_transfer_len),
			pTd->pUrb->transfer_buffer_length);

		if (pTd->pUrb->transfer_flags & URB_SHORT_NOT_OK)
			*status = -EREMOTEIO;
		else
			*status = 0;
	}

	if (pEvent_trb == pTd->pLast_trb) {
		if (TRB_TRANSFER_LEN(pEvent->trb_transfer_len) != 0) {
			pTd->pUrb->actual_length =
				pTd->pUrb->transfer_buffer_length -
				TRB_TRANSFER_LEN(pEvent->trb_transfer_len);
			if (pTd->pUrb->transfer_buffer_length <
					pTd->pUrb->actual_length) {
				ms_err("%s: ask len %d < actual_length %d\n", __func__,
					pTd->pUrb->transfer_buffer_length,
					pTd->pUrb->actual_length);
				pTd->pUrb->actual_length = 0;
				if (pTd->pUrb->transfer_flags & URB_SHORT_NOT_OK)
					*status = -EREMOTEIO;
				else
					*status = 0;
			}

			if (*status == -EINPROGRESS) {
				if (pTd->pUrb->transfer_flags & URB_SHORT_NOT_OK)
					*status = -EREMOTEIO;
				else
					*status = 0;
			}
		} else {
			pTd->pUrb->actual_length = pTd->pUrb->transfer_buffer_length;
			if (*status == -EREMOTEIO)
				*status = 0;
		}
	}
	else
	{
		pTd->pUrb->actual_length = 0;
		for (pCur_trb = pEp_ring->pDeq, pCur_seg = pEp_ring->pDeq_seg;
				pCur_trb != pEvent_trb;
				ms_next_trb(pXhci, pEp_ring, &pCur_seg, &pCur_trb)) {
			if ((pCur_trb->generic.trb_info[3] &
			 TRB_TYPE_MASK) != TRB_TYPE(TRB_TYPE_NOOP) &&
			    (pCur_trb->generic.trb_info[3] &
			 TRB_TYPE_MASK) != TRB_TYPE(TRB_TYPE_LINK))
				pTd->pUrb->actual_length +=
					TRB_TRANSFER_LEN(pCur_trb->generic.trb_info[2]);
		}

		if (comp_code != TRB_COMP_STOP_LENGTH_INVAL)
			pTd->pUrb->actual_length +=
				TRB_TRANSFER_LEN(pCur_trb->generic.trb_info[2]) -
				TRB_TRANSFER_LEN(pEvent->trb_transfer_len);
	}

	return ms_finish_td(pXhci, pTd, pEvent_trb, pEvent, pEpt, status, false);
}

void ms_usb_hcd_giveback_urb( struct urb *pUrb, int status)
{
	struct api_context *pComp;

	pUrb->hcpriv = NULL;
	if (unlikely(pUrb->unlinked))
		status = pUrb->unlinked;
	else if (unlikely((pUrb->transfer_flags & URB_SHORT_NOT_OK) &&
			pUrb->actual_length < pUrb->transfer_buffer_length &&
			!status))
		status = -EREMOTEIO;

	pUrb->status = status;
	pComp=(struct api_context *) pUrb->context;		//done the urb
	pComp->done=1;
	pComp->status=status;
}

static int ms_handle_tx_event(struct xhci_hcd *pXhci, struct xhci_transfer_event *pEvent)
{
	struct xhci_my_device *pVirt_dev;
	struct xhci_my_ep *pVirt_ep;
	struct xhci_ring *pEp_ring;
	unsigned int slot_id;
	int ep_idx;
	struct xhci_td *pTd = NULL;
	dma_addr_t event_dma;
	struct xhci_seg *pEvent_seg;
	union xhci_trb *pEvent_trb;
	struct urb *pUrb = NULL;
	int status = -EINPROGRESS;
	struct urb_priv *pUrb_priv;
	struct xhci_ep_ctx *pEpt_ctx;
	struct list_head *tmp;
	u32 comp_code;
	int ret = 0;
	int td_num = 0;

	slot_id = GET_TRB_SLOT_ID(pEvent->trb_info);
	pVirt_dev = pXhci->devs[slot_id];
	if (!pVirt_dev) {
		ms_err("%s: pVirt_dev is null at slot %d\n", __func__, slot_id);
		return -ENODEV;
	}

	ep_idx = TRB_TO_EP_ID(pEvent->trb_info) - 1;
	pVirt_ep = &pVirt_dev->eps[ep_idx];
	pEp_ring = pVirt_ep->ring;
	pEpt_ctx = ms_xhci_get_ep_ctx(pXhci, pVirt_dev->out_ctx, ep_idx);
	if (!pEp_ring ||
	    (pEpt_ctx->ep_ctx_field1 & EP_STATE_MASK) == EP_STATE_DISABLED) {
		ms_err("%s: Transfer on disabled ept (idx=%d) or ept ring is null\n",
			__func__, ep_idx);
		return -ENODEV;
	}

	if (pVirt_ep->skip) {
		list_for_each(tmp, &pEp_ring->td_list)
			td_num++;
	}

	event_dma = pEvent->trb_ptr;
	comp_code = GET_COMP_CODE(pEvent->trb_transfer_len);
	gUsbStatusXHC = 0;

	switch (comp_code)
	{
	case TRB_COMP_SUCCESS:
	case TRB_COMP_SHORT_PACKET:
		//printf("COMP_SUCCESS\n");
		break;

	case TRB_COMP_STOPPED:
		ms_dbg("tx stop\n");
		break;

	case TRB_COMP_STOP_LENGTH_INVAL:
		ms_dbg("tx stop and length is invalid\n");
		break;

	case TRB_COMP_STALL_ERR:
		ms_dbg("tx stall\n");
		pVirt_ep->ep_state |= EP_ST_HALTED;
		status = -EPIPE;
		gUsbStatusXHC |= USB_ST_STALLED;

		break;
	case TRB_COMP_TRB_ERR:
		ms_dbg("trb error\n");
		status = -EILSEQ;
		break;

	case TRB_COMP_SPLIT_TRANS_ERR:
	case TRB_COMP_TRANS_ERR:
		ms_err("tx error\n");
		status = -EPROTO;

		ms_xhci_dump_ring(pXhci->event_ring);
		ms_dbg("Endpoint ring: %x\n", ep_idx);
		ms_xhci_dump_ring(pEp_ring);
		break;

	case TRB_COMP_BABBLE_ERR:
		ms_err("tx babble\n");
		status = -EOVERFLOW;
		gUsbStatusXHC |= USB_ST_BABBLE_DET;
		break;

	case TRB_COMP_DATA_BUFFER_ERR:
		ms_err("tx data buffer error\n");
		status = -ENOSR;
		break;

	case TRB_COMP_BANDWIDTH_OVERRUN:
		ms_err("tx bandwidth overrun error\n");
		break;

	case TRB_COMP_RING_UNDERRUN:
		ms_dbg("tx underrun\n");
		if (!list_empty(&pEp_ring->td_list))
			ms_dbg( "underrun for slot %d ep %d \n",
				 GET_TRB_SLOT_ID(pEvent->trb_info),
				 ep_idx);
		goto cleanup;

	case TRB_COMP_RING_OVERRUN:
		ms_dbg("tx overrun\n");
		if (!list_empty(&pEp_ring->td_list))
			ms_dbg("overrun for slot %d ep %d \n"
				 GET_TRB_SLOT_ID(pEvent->trb_info),
				 ep_idx);
		goto cleanup;

	case TRB_COMP_INCOMP_DEV_ERR:
		ms_err("tx incompatible device error");
		status = -EPROTO;
		break;

	case TRB_COMP_MISSED_SERVICE_ERR:
		pVirt_ep->skip = true;
		ms_dbg("tx miss service error\n");
		goto cleanup;

	default:
		if (ms_xhci_is_vendor_code(comp_code)) {
			status = 0;
			break;
		}
		ms_err("tx unknown event %d\n", comp_code);
		goto cleanup;
	}

	do {
		if (list_empty(&pEp_ring->td_list)) {
			ms_err("event for slot %d ep %d but td is empty\n",
				  GET_TRB_SLOT_ID(pEvent->trb_info),
				  ep_idx);

			ms_xhci_print_trb((union xhci_trb *) pEvent);
			if (pVirt_ep->skip) {
				pVirt_ep->skip = false;
				ms_dbg("clear skip flag if td_list is empty\n");
			}
			ret = 0;
			goto cleanup;
		}

		if (pVirt_ep->skip && td_num == 0) {
			pVirt_ep->skip = false;
			ms_dbg("clear skip if all tds are skipped\n");
			ret = 0;
			goto cleanup;
		}

		pTd = list_entry(pEp_ring->td_list.next, struct xhci_td, td_list);
		if (pVirt_ep->skip)
			td_num--;

		pEvent_seg = ms_chk_trb_in_td(pEp_ring->pDeq_seg, pEp_ring->pDeq,
				pTd->pLast_trb, event_dma);

		if (!pEvent_seg && comp_code == TRB_COMP_STOP_LENGTH_INVAL) {
			ret = 0;
			goto cleanup;
		}

		if (comp_code == TRB_COMP_SHORT_PACKET)
			pEp_ring->last_td_short = true;
		else
			pEp_ring->last_td_short = false;

		if (pVirt_ep->skip) {
			ms_dbg("clear skip flag.\n");
			pVirt_ep->skip = false;
		}

		pEvent_trb = &pEvent_seg->pTrbs[(event_dma - pEvent_seg->dma) /
						sizeof(*pEvent_trb)];
		if ((pEvent_trb->generic.trb_info[3] & TRB_TYPE_MASK)
				 == TRB_TYPE(TRB_TYPE_NOOP)) {
			ms_dbg("event for a no-op trb\n");
			goto cleanup;
		}

		if (is_usb_control_ept(&pTd->pUrb->ep->desc))
			ret = ms_process_ctrl_td(pXhci, pTd, pEvent_trb, pEvent, pVirt_ep,
						 &status);

		else if(is_usb_bulk_ept(&pTd->pUrb->ep->desc))
			ret = ms_process_bulk_intr_td(pXhci, pTd, pEvent_trb, pEvent,
						 pVirt_ep, &status);

cleanup:
		if (comp_code == TRB_COMP_MISSED_SERVICE_ERR || !pVirt_ep->skip) {
			ms_inc_deq(pXhci, pXhci->event_ring, true);
		}

		if (ret) {
			pUrb = pTd->pUrb;
			pUrb_priv = pUrb->hcpriv;

			if (is_usb_control_ept(&pUrb->ep->desc) ||
				(comp_code != TRB_COMP_STALL_ERR &&
					comp_code != TRB_COMP_BABBLE_ERR))
				ms_xhci_free_urb_priv(pUrb_priv);
#ifdef NEED_TO_DO
			usb_hcd_unlink_urb_from_ep(bus_to_hcd(pUrb->dev->bus), pUrb);
#endif
			if ((pUrb->actual_length != pUrb->transfer_buffer_length &&
						(pUrb->transfer_flags &
						 URB_SHORT_NOT_OK)) ||
					status != 0)
				ms_dbg("actual len = %d != ask len %d \n",
						pUrb->actual_length,
						pUrb->transfer_buffer_length);

			if (usb_pipetype(pUrb->pipe) == PIPE_ISOCHRONOUS)
				status = 0;
			ms_usb_hcd_giveback_urb( pUrb, status);

		}

	} while (pVirt_ep->skip && comp_code != TRB_COMP_MISSED_SERVICE_ERR);

	return 0;
}

int ms_finish_td(struct xhci_hcd *pXhci, struct xhci_td *pTd,
	union xhci_trb *pEvent_trb, struct xhci_transfer_event *pEvent,
	struct xhci_my_ep *pVirt_ep, int *status, bool skip)
{
	struct xhci_my_device *pVirt_dev;
	struct xhci_ring *pEp_ring;
	unsigned int slot_id;
	int ep_idx;
	struct urb *pUrb = NULL;
	struct xhci_ep_ctx *pEpt_ctx;
	int retval = 0;
	struct urb_priv	*pUrb_priv;
	u32 comp_code;

	slot_id = GET_TRB_SLOT_ID(pEvent->trb_info);
	pVirt_dev = pXhci->devs[slot_id];
	ep_idx = TRB_TO_EP_ID(pEvent->trb_info) - 1;
	pEp_ring = pVirt_ep->ring;
	pEpt_ctx = ms_xhci_get_ep_ctx(pXhci, pVirt_dev->out_ctx, ep_idx);
	comp_code = GET_COMP_CODE(pEvent->trb_transfer_len);

	if (skip)
		goto td_cleanup;

	if (comp_code == TRB_COMP_STOP_LENGTH_INVAL ||
			comp_code == TRB_COMP_STOPPED)
	{
		pVirt_ep->pStop_td = pTd;
		//printf("ep: %x, ep->stopped_td: %x\n", pVirt_ep, (unsigned int)pVirt_ep->stopped_td);
		pVirt_ep->pStop_trb = pEvent_trb;
		return 0;
	}
	else
	{
		if (comp_code == TRB_COMP_STALL_ERR) {
			pVirt_ep->pStop_td = pTd;
			pVirt_ep->pStop_trb = pEvent_trb;
		} else if (ms_xhci_need_halt_cleanup(pEpt_ctx, comp_code)) {
			ms_xhci_cleanup_halt_ept(pXhci,
					slot_id, ep_idx, pTd, pEvent_trb);
		} else {
			while (pEp_ring->pDeq != pTd->pLast_trb)
				ms_inc_deq(pXhci, pEp_ring, false);
			ms_inc_deq(pXhci, pEp_ring, false);
		}

td_cleanup:
		pUrb = pTd->pUrb;
		pUrb_priv = pUrb->hcpriv;

		if (pUrb->actual_length > pUrb->transfer_buffer_length) {
			ms_err("%s: actual len %d > ask len %d\n", __func__,
					pUrb->actual_length,
					pUrb->transfer_buffer_length);
			pUrb->actual_length = 0;
			if (pTd->pUrb->transfer_flags & URB_SHORT_NOT_OK)
				*status = -EREMOTEIO;
			else
				*status = 0;
		}
		list_del_init(&pTd->td_list);
		if (!list_empty(&pTd->cancel_td_list))
			list_del_init(&pTd->cancel_td_list);

		pUrb_priv->td_count++;
		if (pUrb_priv->td_count == pUrb_priv->len)
			retval = 1;
	}

	return retval;
}

struct xhci_seg *ms_chk_trb_in_td(struct xhci_seg *pStart_seg,
		union xhci_trb	*pStart_trb,
		union xhci_trb	*pEnd_trb,
		dma_addr_t	chk_dma)
{
	dma_addr_t td_start_dma;
	dma_addr_t td_end_dma;
	dma_addr_t seg_end_dma;

	td_start_dma = ms_xhci_trb_to_dma(pStart_seg, pStart_trb);

	if (td_start_dma == 0)
		return NULL;

	seg_end_dma = ms_xhci_trb_to_dma(pStart_seg,
			&pStart_seg->pTrbs[MAX_TRBS_IN_SEG - 1]);

	td_end_dma = ms_xhci_trb_to_dma(pStart_seg, pEnd_trb);

	if (td_end_dma > 0) {
		if (td_start_dma <= td_end_dma) {
			if (chk_dma >= td_start_dma && chk_dma <= td_end_dma)
				return pStart_seg;
		} else {
			if ((chk_dma >= td_start_dma &&
						chk_dma <= seg_end_dma) ||
					(chk_dma >= pStart_seg->dma &&
					 chk_dma <= td_end_dma))
				return pStart_seg;
		}
		return NULL;
	} else {
		if (chk_dma >= td_start_dma && chk_dma <= seg_end_dma)
			return pStart_seg;
	}

	return NULL;
}

static int ms_xhci_handle_event(struct xhci_hcd *pXhci)
{
	union xhci_trb *pEvent;
	int is_update_deq = 1;
	int retval;

	if (!pXhci->event_ring || !pXhci->event_ring->pDeq) {
		pXhci->error_bitmask |= 1 << 1;
		return 0;
	}

	pEvent = pXhci->event_ring->pDeq;
	if ((pEvent->event_cmd.trb_info & TRB_CYCLE) !=
	    pXhci->event_ring->cycle_state) {
		pXhci->error_bitmask |= 1 << 2;
		return 0;
	}

	switch ((pEvent->event_cmd.trb_info & TRB_TYPE_MASK)) {
	case TRB_TYPE(TRB_TYPE_COMPLETION_EVENT):

		ms_cmd_completion(pXhci, &pEvent->event_cmd);
		break;


	case TRB_TYPE(TRB_TYPE_PORT_CHANGE):
		ms_dbg("event for port status change\n");
		is_update_deq = 1;

		break;

	case TRB_TYPE(TRB_TYPE_TRANSFER):
		retval = ms_handle_tx_event(pXhci, &pEvent->trans_event);
		if (retval < 0)
			pXhci->error_bitmask |= 1 << 9;
		else
			is_update_deq = 0;
		break;


	default:
		if ((pEvent->event_cmd.trb_info & TRB_TYPE_MASK) >= TRB_TYPE(48))
			ms_err("%s: unknown TRB type: %d \n", __func__,
				GET_TRB_TYPE(pEvent->event_cmd.trb_info));
		else
			pXhci->error_bitmask |= 1 << 3;
	}

	if (is_update_deq)
		ms_inc_deq(pXhci, pXhci->event_ring, true);

	return 1;
}

int ms_xhci_irq_polling(struct xhci_hcd *pXhci)
{
	u32 status;
	union xhci_trb *pTrb;
	u64 temp_64;
	union xhci_trb *pEvent_ring_deq;
	dma_addr_t deq_dma;

#if XHCI_FLUSHPIPE_PATCH
	Chip_Read_Memory(); //Flush Read buffer when H/W finished
#endif

	pTrb = pXhci->event_ring->pDeq;

	status = xhci_readl(pXhci, &pXhci->op_regs->u32UsbSts);
	if (status == 0xffffffff)
		goto hw_died;

	if (status & USBSTS_HSE)
	{
		ms_err("%s: Host System Error\n", __func__);
		ms_xhci_halt(pXhci);
hw_died:
		return -ESHUTDOWN;
	}

	status |= USBSTS_EINT;
	xhci_writel(pXhci, status, &pXhci->op_regs->u32UsbSts);
	{
		u32 irq_pending;

		irq_pending = xhci_readl(pXhci, &pXhci->intr_regs->u32IMAN);
		irq_pending |= 0x3;
		xhci_writel(pXhci, irq_pending, &pXhci->intr_regs->u32IMAN);
	}

	pEvent_ring_deq = pXhci->event_ring->pDeq;

	while (ms_xhci_handle_event(pXhci) > 0) {}

	temp_64 = xhci_read_64(pXhci, &pXhci->intr_regs->u32ERDP);
	if (pEvent_ring_deq != pXhci->event_ring->pDeq) {
		deq_dma = ms_xhci_trb_to_dma(pXhci->event_ring->pDeq_seg,
				pXhci->event_ring->pDeq);
		if (deq_dma == 0)
			ms_err("invalid event ring deq address\n");

		temp_64 &= ERDP_MASK;
		temp_64 |= ((u64) deq_dma & (u64) ~ERDP_MASK);
	}

	temp_64 |= ERDP_EHB;
	xhci_write_64(pXhci, temp_64, &pXhci->intr_regs->u32ERDP);

	return 0;
}

static void ms_queue_trb(struct xhci_hcd *pXhci, struct xhci_ring *pRing,
		bool is_consumer, bool more_trbs,
		u32 u32field1, u32 u32field2, u32 u32field3, u32 u32field4)
{
	struct xhci_generic_trb *trb;

	trb = &pRing->pEnq->generic;
	trb->trb_info[0] = cpu_to_le32(u32field1);
	trb->trb_info[1] = cpu_to_le32(u32field2);
	trb->trb_info[2] = cpu_to_le32(u32field3);
	trb->trb_info[3] = cpu_to_le32(u32field4);
	ms_inc_enq(pXhci, pRing, is_consumer, more_trbs);
}

void ms_giveback_first_trb(struct xhci_hcd *pXhci, int slot_id,
		unsigned int ep_idx, int start_cycle,
		struct xhci_generic_trb *pTrb)
{
	if (start_cycle)
		pTrb->trb_info[3] |= cpu_to_le32(start_cycle);
	else
		pTrb->trb_info[3] &= cpu_to_le32(~TRB_CYCLE);

#if XHCI_FLUSHPIPE_PATCH
	Chip_Flush_Memory();
#endif
	//printf("ring door bell\n");
	ms_xhci_ring_ep_db(pXhci, slot_id, ep_idx);
}

int ms_xhci_queue_ctrl_tx(struct xhci_hcd *pXhci, gfp_t mem_flags,
		struct urb *pUrb, int slot_id, unsigned int ep_idx)
{
	struct xhci_ring *pEp_ring;
	int num_trbs;
	int retval;
	struct usb_ctrlrequest *pSetup_req;
	struct xhci_generic_trb *pStart_trb;
	int start_cycle;
	u32 field, length_field;
	struct urb_priv *pUrb_priv;
	struct xhci_td *pTd;

	pEp_ring = ms_xhci_urb_to_ring(pXhci, pUrb);
	if (!pEp_ring)
		return -EINVAL;

	if (!pUrb->setup_packet)
		return -EINVAL;

	ms_dbg("queue ctrl tx for slot id %d, ep %d\n", slot_id, ep_idx);
	num_trbs = 2; //setup and status packet

	if (pUrb->transfer_buffer_length > 0)
		num_trbs++;
	retval = ms_prepare_transfer(pXhci, pXhci->devs[slot_id],
			ep_idx, pUrb->stream_id,
			num_trbs, pUrb, 0);
	if (retval < 0)
		return retval;

	pUrb_priv = pUrb->hcpriv;
	pTd = pUrb_priv->td[0];

	pStart_trb = &pEp_ring->pEnq->generic;
	start_cycle = pEp_ring->cycle_state;

	pSetup_req = (struct usb_ctrlrequest *) pUrb->setup_packet;
	field = 0;
	field |= TRB_IDT | TRB_TYPE(TRB_TYPE_SETUP);
	if (start_cycle == 0)
		field |= 0x1;

	if (pXhci->hci_version == 0x100) {
		if (pUrb->transfer_buffer_length > 0) {
			if (pSetup_req->bRequestType & USB_DIR_IN)
				field |= SET_TRB_TRANSFER_DIR(TRB_DATA_IN);
			else
				field |= SET_TRB_TRANSFER_DIR(TRB_DATA_OUT);
		}
	}

	ms_queue_trb(pXhci, pEp_ring, false, true,
		  pSetup_req->bRequestType | pSetup_req->bRequest << 8 | pSetup_req->wValue << 16,
		  pSetup_req->wIndex | pSetup_req->wLength << 16,
		  TRB_TRANSFER_LEN(8) | SET_TRB_INTR_TARGET(0),
		  field);

	if (is_usb_urb_dir_in(pUrb))
		field = TRB_ISP | TRB_TYPE(TRB_TYPE_DATA);
	else
		field = TRB_TYPE(TRB_TYPE_DATA);

	length_field = TRB_TRANSFER_LEN(pUrb->transfer_buffer_length) |
		ms_xhci_td_remaining(pUrb->transfer_buffer_length) |
		SET_TRB_INTR_TARGET(0);
	if (pUrb->transfer_buffer_length > 0) {
		if (pSetup_req->bRequestType & USB_DIR_IN)
			field |= TRB_DIR_IN;
		ms_queue_trb(pXhci, pEp_ring, false, true,
				lower_32_bits(pUrb->transfer_dma),
				upper_32_bits(pUrb->transfer_dma),
				length_field,
				field | pEp_ring->cycle_state);
	}

	pTd->pLast_trb = pEp_ring->pEnq;

	if (pUrb->transfer_buffer_length > 0 && pSetup_req->bRequestType & USB_DIR_IN)
		field = 0;
	else
		field = TRB_DIR_IN;
	ms_queue_trb(pXhci, pEp_ring, false, false,
			0,
			0,
			SET_TRB_INTR_TARGET(0),
			/* Event on completion */
			field | TRB_IOC | TRB_TYPE(TRB_TYPE_STATUS) | pEp_ring->cycle_state);

	ms_giveback_first_trb(pXhci, slot_id, ep_idx, start_cycle, pStart_trb);

	return 0;
}

 u32 ms_xhci_td_remaining(unsigned int uRemain)
{
	u32 max = (1 << (21 - 17 + 1)) - 1;

	if ((uRemain >> 10) >= max)
		return max << 17;
	else
		return (uRemain >> 10) << 17;
}

int ms_xhci_queue_bulk_tx(struct xhci_hcd *pXhci, gfp_t mem_flags,
		struct urb *pUrb, int slot_id, unsigned int ep_idx)
{
	struct xhci_ring *pEp_ring;
	struct urb_priv *pUrb_priv;
	struct xhci_td *pTd;
	int num_trbs;
	struct xhci_generic_trb *pStart_trb;
	bool is_first_trb;
	bool more_trbs;
	int start_cycle;
	u32 u32Field, u32Len_field;

	int running_total, trb_buff_len, retval;
	unsigned int total_packets;
	u64 u64Addr;

       pEp_ring = ms_xhci_urb_to_ring(pXhci, pUrb);
	if (!pEp_ring)
	{
		ms_err("%s: no ep ring\n", __func__);
		return -EINVAL;
	}
	num_trbs = 0;
	running_total = MAX_TRB_BUF_SIZE -
		(pUrb->transfer_dma & (MAX_TRB_BUF_SIZE - 1));
	running_total &= MAX_TRB_BUF_SIZE - 1;

	if (running_total != 0 || pUrb->transfer_buffer_length == 0)
		num_trbs++;

	while (running_total < pUrb->transfer_buffer_length) {
		num_trbs++;
		running_total += MAX_TRB_BUF_SIZE;
	}

	retval = ms_prepare_transfer(pXhci, pXhci->devs[slot_id],
			ep_idx, pUrb->stream_id,
			num_trbs, pUrb, 0);
	if (retval < 0)
		return retval;

	pUrb_priv = pUrb->hcpriv;
	pTd = pUrb_priv->td[0];

	pStart_trb = &pEp_ring->pEnq->generic;
	start_cycle = pEp_ring->cycle_state;

	running_total = 0;
	total_packets = roundup(pUrb->transfer_buffer_length,
			pUrb->ep->desc.wMaxPacketSize);

	u64Addr = (u64) pUrb->transfer_dma;
	trb_buff_len = MAX_TRB_BUF_SIZE -
		(pUrb->transfer_dma & (MAX_TRB_BUF_SIZE - 1));
	if (trb_buff_len > pUrb->transfer_buffer_length)
		trb_buff_len = pUrb->transfer_buffer_length;

	is_first_trb = true;

	do {
		u32 remainder = 0;
		u32Field = 0;

		if (is_first_trb) {
			is_first_trb = false;
			if (start_cycle == 0)
				u32Field |= 0x1;
		} else
			u32Field |= pEp_ring->cycle_state;

		if (num_trbs > 1) {
			u32Field |= TRB_CHAIN;
		} else {
			pTd->pLast_trb = pEp_ring->pEnq;
			u32Field |= TRB_IOC;
		}

		if (is_usb_urb_dir_in(pUrb))
			u32Field |= TRB_ISP;

		if (pXhci->hci_version < 0x100) {
			remainder = ms_xhci_td_remaining(
					pUrb->transfer_buffer_length -
					running_total);
		} else {
			remainder = ms_xhci_v1_0_td_remaining(running_total,
					trb_buff_len, total_packets, pUrb);
		}
		u32Len_field = TRB_TRANSFER_LEN(trb_buff_len) |
			remainder |
			SET_TRB_INTR_TARGET(0);

		if (num_trbs > 1)
			more_trbs = true;
		else
			more_trbs = false;
		ms_queue_trb(pXhci, pEp_ring, false, more_trbs,
				lower_32_bits(u64Addr),
				upper_32_bits(u64Addr),
				u32Len_field,
				u32Field | TRB_TYPE(TRB_TYPE_NORMAL));
		--num_trbs;
		running_total += trb_buff_len;

		u64Addr += trb_buff_len;
		trb_buff_len = pUrb->transfer_buffer_length - running_total;
		if (trb_buff_len > MAX_TRB_BUF_SIZE)
			trb_buff_len = MAX_TRB_BUF_SIZE;
	} while (running_total < pUrb->transfer_buffer_length);

	ms_giveback_first_trb(pXhci, slot_id, ep_idx,
			start_cycle, pStart_trb);
	return 0;
}

u32 ms_xhci_v1_0_td_remaining(int running_len, int trb_buf_len,
		unsigned int total_packets, struct urb *pUrb)
{
	int actual_packets;

	if (running_len == 0 && trb_buf_len == 0)
		return 0;

	actual_packets = (running_len + trb_buf_len) /
		le16_to_cpu(pUrb->ep->desc.wMaxPacketSize);

	return ms_xhci_td_remaining(total_packets - actual_packets);
}

static int ms_queue_cmd(struct xhci_hcd *pXhci, u32 u32Field1, u32 u32Field2,
		u32 u32Field3, u32 u32Field4)
{
	int retval;

	retval = ms_prepare_ring(pXhci, pXhci->cmd_ring, EP_STATE_RUNNING, 1);

	if (retval < 0) {
		ms_err("%s: No room for command on command ring\n", __func__);
		return retval;
	}
	ms_queue_trb(pXhci, pXhci->cmd_ring, false, false, u32Field1, u32Field2,
			u32Field3,	u32Field4 | pXhci->cmd_ring->cycle_state);

	return 0;
}

int ms_xhci_queue_slot_cmd(struct xhci_hcd *pXhci, u32 u32Cmd_type, u32 u32Slot_id)
{
	return ms_queue_cmd(pXhci, 0, 0, 0,
			TRB_TYPE(u32Cmd_type) | SET_TRB_SLOT_ID(u32Slot_id));
}

int ms_xhci_queue_address_dev_cmd(struct xhci_hcd *pXhci, dma_addr_t in_ctx_dma,
		u32 u32Slot_id)
{
	return ms_queue_cmd(pXhci, lower_32_bits(in_ctx_dma),
			upper_32_bits(in_ctx_dma), 0,
			TRB_TYPE(TRB_TYPE_ADDRESS_DEVICE) | SET_TRB_SLOT_ID(u32Slot_id));
}

int ms_xhci_queue_reset_dev_cmd(struct xhci_hcd *pXhci, u32 u32Slot_id)
{
	return ms_queue_cmd(pXhci, 0, 0, 0,
			TRB_TYPE(TRB_TYPE_RESET_DEV) | SET_TRB_SLOT_ID(u32Slot_id));
}

int ms_xhci_queue_config_ept_cmd(struct xhci_hcd *pXhci, dma_addr_t in_ctx_dma,
		u32 u32Slot_id)
{
	return ms_queue_cmd(pXhci, lower_32_bits(in_ctx_dma),
			upper_32_bits(in_ctx_dma), 0,
			TRB_TYPE(TRB_TYPE_CONFIG_EP) | SET_TRB_SLOT_ID(u32Slot_id));
}

int ms_xhci_queue_eval_ctx_cmd(struct xhci_hcd *pXhci, dma_addr_t in_ctx_dma,
		u32 u32Slot_id)
{
	return ms_queue_cmd(pXhci, lower_32_bits(in_ctx_dma),
			upper_32_bits(in_ctx_dma), 0,
			TRB_TYPE(TRB_TYPE_EVAL_CONTEXT) | SET_TRB_SLOT_ID(u32Slot_id));
}

int ms_xhci_queue_stop_ept_cmd(struct xhci_hcd *pXhci, int slot_id,
		unsigned int ep_idx, int suspend)
{
	return ms_queue_cmd(pXhci, 0, 0, 0,
			SET_TRB_SLOT_ID(slot_id) |
			SET_TRB_EP_ID(ep_idx) |
			TRB_TYPE(TRB_TYPE_STOP_RING) |
			SET_TRB_SUSPEND(suspend));
}

 int ms_queue_set_tr_deq(struct xhci_hcd *pXhci, int slot_id,
		unsigned int ep_idx,
		struct xhci_seg *pDeq_seg,
		union xhci_trb *pDeq_trb, u32 u32Cycle)
{
	dma_addr_t addr;
	struct xhci_my_ep *pVirt_ep;

	addr = ms_xhci_trb_to_dma(pDeq_seg, pDeq_trb);
	if (addr == 0) {
		ms_err("%s: deq trb is null\n", __func__);
		return 0;
	}
	pVirt_ep = &pXhci->devs[slot_id]->eps[ep_idx];
	if ((pVirt_ep->ep_state & EP_ST_SET_DEQ_PENDING)) {
		ms_dbg("ept state is pending when set deq\n");
		return 0;
	}
	pVirt_ep->set_deq_seg = pDeq_seg;
	pVirt_ep->set_deq_ptr = pDeq_trb;
	return ms_queue_cmd(pXhci, lower_32_bits(addr) | u32Cycle,
			upper_32_bits(addr), 0,
			SET_TRB_SLOT_ID(slot_id) |
			SET_TRB_EP_ID(ep_idx) |
			TRB_TYPE(TRB_TYPE_SET_TR_DEQ_PTR));
}

int ms_xhci_queue_reset_ept_cmd(struct xhci_hcd *pXhci, int slot_id,
		unsigned int ep_idx)
{
	return ms_queue_cmd(pXhci, 0, 0, 0,
		SET_TRB_SLOT_ID(slot_id) | SET_TRB_EP_ID(ep_idx) |
		TRB_TYPE(TRB_TYPE_RESET_EP));
}


