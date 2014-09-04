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

int ms_xhci_chk_maxpacket(struct xhci_hcd *pXhci, unsigned int slot_id,
		unsigned int ep_idx, struct urb *pUrb);
u32 xhci_count_num_new_endpoints(struct xhci_hcd *xhci,
	struct xhci_container_ctx *in_ctx);
int ms_xhci_urb_enqueue( struct urb *pUrb, gfp_t mem_flags);
extern void ms_xhci_ept_ctx_zero(struct xhci_hcd *pXhci,
	struct xhci_my_device *pVirt_dev,
	struct usb_host_endpoint *pEpt);
int mx_xhci_config_ept_cmd(struct xhci_hcd *pXhci,
		struct usb_device *pUdev,
		struct xhci_my_cmd *pCmd,
		bool is_ctx_chg);
extern  void ms_usb_hcd_giveback_urb( struct urb *pUrb, int status);
extern struct xhci_ring *ms_xhci_urb_to_ring(struct xhci_hcd *pXhci,
	struct urb *pUrb);

int ms_xhci_halt(struct xhci_hcd *pXhci)
{
	int retval;
	int ii;
	u32 u32tmp;
	u32 mask;
	u32 port_halt;

	ms_dbg(pXhci, "XHCI halt\n");

	mask = ~(MSTAR_XHCI_IRQS);
	port_halt = xhci_readl(pXhci, &pXhci->op_regs->u32UsbSts) & USBSTS_HALT;
	if (!port_halt)
		mask &= ~USBCMD_RUN;

	u32tmp = xhci_readl(pXhci, &pXhci->op_regs->u32UsbCmd);
	u32tmp &= mask;
	xhci_writel(pXhci, u32tmp, &pXhci->op_regs->u32UsbCmd);

	for (ii=0; ii<10; ii++)
	{
		u32tmp = xhci_readl(pXhci, &pXhci->op_regs->u32UsbSts);
		if (u32tmp == ~(u32)0)
			retval = -ENODEV;

		if ( (u32tmp & USBSTS_HALT) == USBSTS_HALT )
		{
			retval = 0;
			goto chk_done;
		}

		udelay(1000);
	}

	retval = -ETIMEDOUT;

chk_done:
	if (!retval)
		pXhci->xhc_state |= XHCI_STATE_HALTED;

	return retval;
}

static int ms_xhci_start(struct xhci_hcd *pXhci)
{
	u32 temp;
	int ret;
	int ii;

	temp = xhci_readl(pXhci, &pXhci->op_regs->u32UsbCmd);
	temp |= (USBCMD_RUN);
	ms_dbg("xhci start (cmd=%x)\n", temp);
	xhci_writel(pXhci, temp, &pXhci->op_regs->u32UsbCmd);

	for (ii=0; ii<10; ii++)
	{
		temp = xhci_readl(pXhci, &pXhci->op_regs->u32UsbSts);
		if (temp == ~(u32)0)
			ret = -ENODEV;

		if ( (temp & USBSTS_HALT) == 0 )
		{
			ret = 0;
			goto chk_done;
		}

		udelay(1000);
	}

	ret = -ETIMEDOUT;

chk_done:

	if (!ret)
		pXhci->xhc_state &= ~XHCI_STATE_HALTED;
	return ret;
}

int ms_xhci_reset(struct xhci_hcd *pXhci)
{
	u32 command;
	u32 state;
	int ret, ii;

	state = xhci_readl(pXhci, &pXhci->op_regs->u32UsbSts);
	if ((state & USBSTS_HALT) == 0) {
		ms_err("%s: xhci not halted\n", __func__);
		return 0;
	}

	ms_dbg("xhci reset\n");
	command = xhci_readl(pXhci, &pXhci->op_regs->u32UsbCmd);
	command |= USBCMD_RST;
	xhci_writel(pXhci, command, &pXhci->op_regs->u32UsbCmd);

	for (ii=0; ii<10; ii++)
	{
		command = xhci_readl(pXhci, &pXhci->op_regs->u32UsbCmd);

		if ( (command & USBCMD_RST) == 0 )
		{
			ret = 0;
			goto chk_reset_done;
		}

		udelay(1000);
	}

	ret = -ETIMEDOUT;

chk_reset_done:
	if (ret)
		return ret;

	ms_dbg("waiting CNR...\n");

	for (ii=0; ii<10; ii++)
	{
		state = xhci_readl(pXhci, &pXhci->op_regs->u32UsbSts);

		if ( (state & USBSTS_CNR) == 0 )
		{
			ret = 0;
			goto chk_cnr_done;
		}

		udelay(1000);
	}

	ret = -ETIMEDOUT;

chk_cnr_done:
	return ret;
}

int ms_xhci_init(struct xhci_hcd *pXhci)
{
	int retval = 0;

	ms_dbg("xhci init\n");

	retval = ms_xhci_mem_init(pXhci);

	ms_dbg("xhci init done\n");

	return retval;
}

int ms_xhci_run(struct xhci_hcd *pXhci)
{
	u32 temp;
	u64 temp_64;

	ms_dbg("xhci run\n");

	ms_dbg("xhci cmd ring:\n");
	ms_xhci_dump_ring(pXhci->cmd_ring);
	ms_xhci_print_ring_ptrs(pXhci->cmd_ring);
	ms_xhci_print_cmd_ptrs(pXhci);

	ms_dbg("xhci ERST:\n");
	ms_xhci_dump_erst(&pXhci->erst);
	ms_dbg("event ring:\n");
	ms_xhci_dump_ring(pXhci->event_ring);
	ms_xhci_print_ring_ptrs(pXhci->event_ring);
	temp_64 = xhci_read_64(pXhci, &pXhci->intr_regs->u32ERDP);
	temp_64 &= ~ERDP_MASK;
	ms_dbg("ERST deq = %0lx\n", (long unsigned int) temp_64);

	ms_dbg("set interrupt register set:\n");
	temp = xhci_readl(pXhci, &pXhci->intr_regs->u32IMOD);
	temp &= ~IMOD_INTERVAL_MASK;
	temp |= (u32) 160;
	xhci_writel(pXhci, temp, &pXhci->intr_regs->u32IMOD);

	/* Set the HCD state before we enable the irqs */
#if (XHCI_INT_ENABLE)
	temp = xhci_readl(pXhci, &pXhci->op_regs->u32UsbCmd);
	temp |= (USBCMD_INTE);
	xhci_writel(pXhci, temp, &pXhci->op_regs->u32UsbCmd);
#endif

	temp = xhci_readl(pXhci, &pXhci->intr_regs->u32IMAN);
	ms_dbg("enabling event ring interrupter\n");
	xhci_writel(pXhci, IMAN_INT_ENABLE(temp),
			&pXhci->intr_regs->u32IMAN);
	ms_xhci_print_interrupt_reg_set(pXhci, 0);

	if (ms_xhci_start(pXhci)) {
		ms_xhci_halt(pXhci);
		return -ENODEV;
	}

	ms_dbg("ms_xhci_run done\n");
	return 0;
}
int  ms_wait_for_completion_timeout(struct xhci_hcd *pXhci,
		int *done ,  unsigned long timeout)
{
	int timeleft;
	int ret;

	for (timeleft=0;  timeleft < timeout ; timeleft+=1)
	{
		ret = ms_xhci_irq_polling(pXhci);
		if (ret)
		{
			ms_err("%s: return error %d \n", __func__, -ret);
			return ret;
		}
		// printf("done:%x\n",*done);
		if (*done)
			break;

		mdelay(1);
	}


	if (timeleft >= timeout)
		return -ETIME;
	else
		return 1;
}

int ms_xhci_alloc_dev(struct xhci_hcd *pXhci, struct usb_device *pUdev)
{
	//int timeleft, flags;
	int retval;

	pXhci->addr_dev = 0;
	retval = ms_xhci_queue_slot_cmd(pXhci, TRB_TYPE_ENABLE_SLOT, 0);
	if (retval)
	{
		ms_err("%s: fail to queue slot cmd\n", __func__);
		return 0;
	}
	ms_xhci_ring_cmd_db(pXhci);

	retval=ms_wait_for_completion_timeout(pXhci, &(pXhci->addr_dev), MS_CTRL_SET_TIMEOUT);

	if (retval<=0) {
		ms_err("%s: enable slot timeout\n", __func__);
		return -ETIME;
	}

	if (!pXhci->slot_id) {
		ms_err("%s: fail to assign device slot ID\n", __func__);
		return 0;
	}

	ms_dbg("slot id:%x\n",pXhci->slot_id);

	if (!ms_xhci_alloc_virt_dev(pXhci, pXhci->slot_id, pUdev, 0)) {
		ms_err("%s: fail to alloc virt dev\n", __func__);
		goto disable_slot;
	}
	pUdev->slot_id = pXhci->slot_id;

	//assign slot ID
       pUdev->state = USB_STATE_ATTACHED;

	pUdev->route = 0;
	pUdev->ep0.desc.bLength = USB_DT_ENDPOINT_SIZE;
	pUdev->ep0.desc.bDescriptorType = USB_DT_ENDPOINT;

	ms_usb_enable_ept(pUdev, &pUdev->ep0);

	return 1;

disable_slot:
	if (!ms_xhci_queue_slot_cmd(pXhci, TRB_TYPE_DISABLE_SLOT, pUdev->slot_id))
		ms_xhci_ring_cmd_db(pXhci);

	return 0;
}

int ms_xhci_reset_dev(struct xhci_hcd *pXhci, struct usb_device *pUdev)
{
	int retval, i;
	unsigned int slot_id;
	struct xhci_my_device *pVirt_dev;
	struct xhci_my_cmd *pCmd_reset_dev;
	int timeleft;
	struct xhci_slot_ctx *pSlot_ctx;


	slot_id = pUdev->slot_id;
	pVirt_dev = pXhci->devs[slot_id];
	if (!pVirt_dev) {
		ms_err("%s: The virt_dev of slot %d is NULL\n", __func__, slot_id);
		return -EINVAL;
	}

	if (pVirt_dev->pUdev != pUdev) {
		ms_err("%s: virt_dev->udev (%x) != udev (%x)\n", __func__,
				(unsigned int)pVirt_dev->pUdev, (unsigned int)pUdev);
		return -EINVAL;
	}

	pSlot_ctx = ms_xhci_get_slot_ctx(pXhci, pVirt_dev->out_ctx);
	if (GET_DEV_CTX_SLOT_STATE(pSlot_ctx->dev_slot_state_info) == DEV_CTX_SLOT_STATE_DISABLED)
		return 0;

	ms_dbg("reset device with slot ID %u\n", slot_id);
	pCmd_reset_dev = ms_xhci_alloc_cmd(pXhci, true, 0);
	if (!pCmd_reset_dev) {
		ms_err("%s: fail to alloc reset cmd\n", __func__);
		return -ENOMEM;
	}

	pCmd_reset_dev->pCmd_trb = pXhci->cmd_ring->pEnq;

	//if it is a link pointer , move to next trb
	if ((pCmd_reset_dev->pCmd_trb->link.trb_info
	     & TRB_TYPE_MASK) == TRB_TYPE(TRB_TYPE_LINK))
		pCmd_reset_dev->pCmd_trb =
			pXhci->cmd_ring->pEnq_seg->next->pTrbs;

	list_add_tail(&pCmd_reset_dev->cmd_list, &pVirt_dev->cmd_list);
	//printf("issue reset device command\n");
	retval = ms_xhci_queue_reset_dev_cmd(pXhci, slot_id);
	if (retval) {
		ms_err("%s: fail to queue reset cmd\n", __func__);
		list_del(&pCmd_reset_dev->cmd_list);
		goto command_cleanup;
	}
	ms_xhci_ring_cmd_db(pXhci);

	for (timeleft=0;  timeleft<MS_CTRL_SET_TIMEOUT; timeleft+=1)
      {
		retval = ms_xhci_irq_polling(pXhci);
		if (retval)
		{
			ms_err("%s: fail to reset dev (err=%d)\n", __func__, -retval);
			return retval;
		}

		if (pCmd_reset_dev->completion)
			break;	//complete..

		mdelay(1);
	}

	if (timeleft >= MS_CTRL_SET_TIMEOUT)
	{
		ms_err("%s: reset dev cmd timeout\n", __func__);
		retval = -ETIME;
	}

	retval = pCmd_reset_dev->status;
	switch (retval) {
	case TRB_COMP_SLOT_ERR:
	case TRB_COMP_CTX_STATE_ERR:
		ms_err("%s: reset dev failed (comp=%d)\n", __func__, retval);
		retval = 0;
		goto command_cleanup;

	case TRB_COMP_SUCCESS:
		ms_dbg("reset dev ok\n");
		break;

	default:
		if (ms_xhci_is_vendor_code(retval))
			break;
		ms_err("%s: reset dev failed (comp=%d)\n", __func__, retval);
		retval = -EINVAL;
		goto command_cleanup;
	}

	//disable all endpoints except the endpoint 0
	for (i = 1; i < 31; ++i)
	{
		struct xhci_my_ep *ep = &pVirt_dev->eps[i];
		if (ep->ring)
			pVirt_dev->eps[i].ring = NULL;
	}
	retval = 0;

command_cleanup:
	ms_xhci_free_cmd(pCmd_reset_dev);
	return retval;
}

unsigned int ms_xhci_get_ept_idx(struct usb_endpoint_descriptor *pDesc)
{
	unsigned int uIdx;

	if (is_usb_control_ept(pDesc))
		uIdx = (unsigned int) (usb_ept_num(pDesc)*2);
	else
		uIdx = (unsigned int) (usb_ept_num(pDesc)*2) +
			(is_usb_ept_dir_in(pDesc) ? 1 : 0) - 1;

	return uIdx;
}

unsigned int ms_xhci_get_ept_ctrl_flag(struct usb_endpoint_descriptor *pDesc)
{
	return 1 << (ms_xhci_get_ept_idx(pDesc) + 1);
}

unsigned int ms_xhci_last_valid_ept_idx(u32 add_flags)
{
	return fls(add_flags) - 1;
}

int ms_xhci_address_dev(struct xhci_hcd *pXhci, struct usb_device *pUdev)
{
	struct xhci_my_device *pVirt_dev;
	int retval = 0;
	struct xhci_slot_ctx *pSlot_ctx;
	struct xhci_input_control_ctx *pCtrl_ctx;
	u64 temp_64;

	if (!pUdev->slot_id) {
		ms_err("%s: invalid slot id\n", __func__);
		return -EINVAL;
	}

	pVirt_dev = pXhci->devs[pUdev->slot_id];

	if ((!pVirt_dev)) {
		ms_err("%s: virt dev is null at slot_id %d\n", __func__, pUdev->slot_id);
		return -EINVAL;
	}

	pSlot_ctx = ms_xhci_get_slot_ctx(pXhci, pVirt_dev->in_ctx);
	if (!pSlot_ctx->dev_slot_info1)
		ms_xhci_setup_virt_dev(pXhci, pUdev); //first set address command
	else
		ms_xhci_copy_ep0_deq_to_input_ctx(pXhci, pUdev);
	pCtrl_ctx = ms_xhci_get_input_control_ctx(pVirt_dev->in_ctx);
	pCtrl_ctx->add_ctx_flags = cpu_to_le32(SLOT_FLAG | EP0_FLAG);
	pCtrl_ctx->drop_ctx_flags = 0;

	init_completion(pXhci->addr_dev);			//yuwen add , for xhci_alloc_dev already set this flag

	ms_dbg("slot %d input ctx:\n", pUdev->slot_id);
	ms_xhci_print_ctx(pXhci, pVirt_dev->in_ctx, 2);

	retval = ms_xhci_queue_address_dev_cmd(pXhci, pVirt_dev->in_ctx->dma_addr,
					pUdev->slot_id);
	if (retval) {
		ms_err("%s: fail to queue address dev cmd\n", __func__);
		return retval;
	}
	ms_xhci_ring_cmd_db(pXhci);

	retval=ms_wait_for_completion_timeout(pXhci, &(pXhci->addr_dev), MS_CTRL_SET_TIMEOUT);

	if (retval < 0) {
		ms_err("%s: address device cmd timeout\n", __func__);
		return -ETIME;
	}

	switch (pVirt_dev->cmd_status) {
	case TRB_COMP_CTX_STATE_ERR:
	case TRB_COMP_SLOT_ERR:
		ms_err("%s: fail to address dev on slot %d (comp=%d)\n", __func__,
				pUdev->slot_id, pVirt_dev->cmd_status);
		retval = -EINVAL;
		break;

	case TRB_COMP_TRANS_ERR:
		ms_err("%s: transfer error\n", __func__);
		retval = -EPROTO;
		break;

	case TRB_COMP_INCOMP_DEV_ERR:
		ms_err("%s: incompatible device error\n", __func__);
		retval = -ENODEV;
		break;

	case TRB_COMP_SUCCESS:
		ms_dbg("address device ok\n");
		break;

	default:
		ms_err("%s: fail to address dev on slot %d (comp=%d)\n", __func__,
				pUdev->slot_id, pVirt_dev->cmd_status);
		retval = -EINVAL;
		break;

	}
	if (retval) {
		return retval;
	}
	temp_64 = xhci_read_64(pXhci, &pXhci->op_regs->u64DcbaaPtr);
	ms_dbg("DCBAA = %#016llx\n", temp_64);
	ms_dbg("slot %d dcbaa %p = %#016llx\n", pUdev->slot_id,
		 &pXhci->dcbaa->dev_ctx_bases[pUdev->slot_id],
		 (unsigned long long) pXhci->dcbaa->dev_ctx_bases[pUdev->slot_id]);
	ms_dbg("input ctx:\n");
	ms_xhci_print_ctx(pXhci, pVirt_dev->in_ctx, 2);
	ms_dbg("output ctx %#08llx (DMA):\n",
			(unsigned long long)pVirt_dev->out_ctx->dma_addr);
	ms_xhci_print_ctx(pXhci, pVirt_dev->out_ctx, 2);
	pSlot_ctx = ms_xhci_get_slot_ctx(pXhci, pVirt_dev->out_ctx);

	// store the device address returned from XHC
	pVirt_dev->addr = (pSlot_ctx->dev_slot_state_info & DEV_CTX_DEV_ADDR_MASK) + 1;

	pCtrl_ctx->add_ctx_flags = 0;
	pCtrl_ctx->drop_ctx_flags = 0;

	//ms_dbg("device address = %d\n", pVirt_dev->address);
	return 0;
}

extern  struct xhci_hcd     ms_xhci;

int ms_xhci_urb_enqueue( struct urb *pUrb, gfp_t mem_flags)
{
	struct xhci_hcd *pXhci = &ms_xhci;
	int retval = 0;
	unsigned int slot_id, ep_idx;
	struct urb_priv	*pUrb_priv;
	int size, i;

	slot_id = pUrb->dev->slot_id;
	ep_idx = ms_xhci_get_ept_idx(&pUrb->ep->desc);

	size = 1;

	pUrb_priv = kzalloc(sizeof(struct urb_priv) +
				  size * sizeof(struct xhci_td *), mem_flags);
	if (!pUrb_priv)
		return -ENOMEM;

	for (i = 0; i < size; i++) {
		pUrb_priv->td[i] = kzalloc(sizeof(struct xhci_td), mem_flags);
		if (!pUrb_priv->td[i]) {
			pUrb_priv->len = i;
			ms_xhci_free_urb_priv(pUrb_priv);
			return -ENOMEM;
		}
	}

	pUrb_priv->len = size;
	pUrb_priv->td_count = 0;
	pUrb->hcpriv = pUrb_priv;

	if (is_usb_control_ept(&pUrb->ep->desc)) {
		//If the maxpacket of endpoint 0 is changed, need to update it.
		if (pUrb->dev->speed == USB_SPEED_FULL) {
			retval = ms_xhci_chk_maxpacket(pXhci, slot_id,
					ep_idx, pUrb);
			if (retval < 0) {
				ms_xhci_free_urb_priv(pUrb_priv);
				pUrb->hcpriv = NULL;
				return retval;
			}
		}

		if (pXhci->xhc_state & XHCI_STATE_DYING)
			goto dying;
		retval = ms_xhci_queue_ctrl_tx(pXhci, GFP_ATOMIC, pUrb,
				slot_id, ep_idx);
		if (retval)
			goto free_priv;
	}

	else if (is_usb_bulk_ept(&pUrb->ep->desc))
	{
		retval = ms_xhci_queue_bulk_tx(pXhci, GFP_ATOMIC, pUrb,
					slot_id, ep_idx);
		if (retval)
			goto free_priv;
	}

	return retval;
dying:
	ms_err("%s: xhci host is dying\n", __func__);
	retval = -ESHUTDOWN;
free_priv:
	ms_xhci_free_urb_priv(pUrb_priv);
	pUrb->hcpriv = NULL;
	return retval;
}

int ms_xhci_urb_dequeue(struct urb *pUrb, int status)
{
	int retval = 0, i;
	u32 temp;
	struct xhci_hcd *pXhci = &ms_xhci;
	struct urb_priv	*pUrb_priv;
	struct xhci_td *pTd;
	unsigned int ep_idx;
	struct xhci_ring *pEp_ring;
	struct xhci_my_ep *pVirt_ep;

	if (!pUrb->hcpriv)
		goto done;

	temp = xhci_readl(pXhci, &pXhci->op_regs->u32UsbSts);
	if (temp == 0xffffffff || (pXhci->xhc_state & XHCI_STATE_HALTED)) {
		ms_dbg("xhci hc halted\n");
		pUrb_priv = pUrb->hcpriv;
		for (i = pUrb_priv->td_count; i < pUrb_priv->len; i++) {
			pTd = pUrb_priv->td[i];
			if (!list_empty(&pTd->td_list))
				list_del_init(&pTd->td_list);
			if (!list_empty(&pTd->cancel_td_list))
				list_del_init(&pTd->cancel_td_list);
		}

		ms_usb_hcd_giveback_urb(pUrb, -ESHUTDOWN);
		ms_xhci_free_urb_priv(pUrb_priv);
		return retval;
	}
	if ((pXhci->xhc_state & XHCI_STATE_DYING) ||
			(pXhci->xhc_state & XHCI_STATE_HALTED)) {
		ms_dbg("xhci hc dying or halted\n");
		goto done;
	}

	ms_dbg("cancel urb %p\n", pUrb);
	ms_dbg("event ring:\n");
	ms_xhci_dump_ring(pXhci->event_ring);
	ep_idx = ms_xhci_get_ept_idx(&pUrb->ep->desc);
	pVirt_ep = &pXhci->devs[pUrb->dev->slot_id]->eps[ep_idx];
	pEp_ring = ms_xhci_urb_to_ring(pXhci, pUrb);
	if (!pEp_ring) {
		retval = -EINVAL;
		goto done;
	}

	ms_dbg("endpoint %d ring: \n", ep_idx);
	ms_xhci_dump_ring(pEp_ring);
	ms_dbg("device context:\n");
	ms_xhci_print_ctx(pXhci, pXhci->devs[pUrb->dev->slot_id]->out_ctx, 2);

	pUrb_priv = pUrb->hcpriv;

	for (i = pUrb_priv->td_count; i < pUrb_priv->len; i++) {
		pTd = pUrb_priv->td[i];
		list_add_tail(&pTd->cancel_td_list, &pVirt_ep->cancel_td_list);
	}

	if (!(pVirt_ep->ep_state & EP_ST_HALT_PENDING)) {
		pVirt_ep->ep_state |= EP_ST_HALT_PENDING;
		ms_dbg("queue stop ept cmd\n");
		ms_xhci_queue_stop_ept_cmd(pXhci, pUrb->dev->slot_id, ep_idx, 0);
		ms_xhci_ring_cmd_db(pXhci);
	}
done:
	return retval;
}

struct xhci_ring *ms_xhci_urb_to_ring(struct xhci_hcd *pXhci,
		struct urb *pUrb)
{
	unsigned int slot_id;
	unsigned int ep_idx;
	struct xhci_my_ep *pVirt_ep;

	slot_id = pUrb->dev->slot_id;
	ep_idx = ms_xhci_get_ept_idx(&pUrb->ep->desc);
	pVirt_ep = &pXhci->devs[slot_id]->eps[ep_idx];

	return pVirt_ep->ring;
}

void ms_xhci_clear_stalled_ring(struct xhci_hcd *pXhci,
		struct usb_device *pUdev, unsigned int ep_idx)
{
	struct xhci_deq_state deq_state;
	struct xhci_my_ep *pVirt_ep;

	ms_dbg("clear stalled endpoint ring %d\n", ep_idx);
	pVirt_ep = &pXhci->devs[pUdev->slot_id]->eps[ep_idx];

	//update ring dequeue pointer to over the TD
	ms_xhci_get_new_deq_state(pXhci, pUdev->slot_id,
			ep_idx, pVirt_ep->pStop_td,	&deq_state);

	ms_dbg("queue new dequeue state\n");
	ms_xhci_set_new_deq_state(pXhci, pUdev->slot_id,
			ep_idx, &deq_state);
}

int ms_xhci_chk_maxpacket(struct xhci_hcd *pXhci, unsigned int slot_id,
		unsigned int ep_idx, struct urb *pUrb)
{
	struct xhci_container_ctx *pIn_ctx;
	struct xhci_container_ctx *pOut_ctx;
	struct xhci_input_control_ctx *pCtrl_ctx;
	struct xhci_ep_ctx *pEpt_ctx;
	int ep0_max_packets;
	int hw_max_packets;
	int retval = 0;

	pOut_ctx = pXhci->devs[slot_id]->out_ctx;
	pEpt_ctx = ms_xhci_get_ep_ctx(pXhci, pOut_ctx, ep_idx);
	hw_max_packets = GET_EP_MAX_PACKET_SIZE(pEpt_ctx->ep_ctx_field2);
	ep0_max_packets = pUrb->dev->ep0.desc.wMaxPacketSize;
	if (hw_max_packets != ep0_max_packets) {
		ms_dbg("ep 0 maxpkt: %d xhci hw ctx maxpkt: %d\n",
			ep0_max_packets, hw_max_packets);

		ms_xhci_ep_ctx_copy(pXhci, pXhci->devs[slot_id]->in_ctx,
				pXhci->devs[slot_id]->out_ctx, ep_idx);
		pIn_ctx = pXhci->devs[slot_id]->in_ctx;
		pEpt_ctx = ms_xhci_get_ep_ctx(pXhci, pIn_ctx, ep_idx);
		pEpt_ctx->ep_ctx_field2 &= ~EP_MAX_PACKET_SIZE_MASK;
		pEpt_ctx->ep_ctx_field2 |= EP_MAX_PACKET_SIZE(ep0_max_packets);

		pCtrl_ctx = ms_xhci_get_input_control_ctx(pIn_ctx);
		pCtrl_ctx->add_ctx_flags = EP0_FLAG;
		pCtrl_ctx->drop_ctx_flags = 0;

		ms_dbg("Slot %d input ctx\n", slot_id);
		ms_xhci_print_ctx(pXhci, pIn_ctx, ep_idx);
		ms_dbg("Slot %d output ctx\n", slot_id);
		ms_xhci_print_ctx(pXhci, pOut_ctx, ep_idx);

		ms_dbg(pXhci, "send evaluate context cmd.\n");
		retval = mx_xhci_config_ept_cmd(pXhci, pUrb->dev, NULL,
				true);

		pCtrl_ctx->add_ctx_flags = SLOT_FLAG;
	}
	return retval;
}

int mx_xhci_config_ept_cmd(struct xhci_hcd *pXhci,
		struct usb_device *pUdev,
		struct xhci_my_cmd *pCmd,
		bool is_ctx_chg)
{
	int retval;
	int timeleft;
	struct xhci_container_ctx *pIn_ctx;
	int *pCmd_comp;
	u32 *pCmd_status;
	struct xhci_my_device *pVirt_dev;

	pVirt_dev = pXhci->devs[pUdev->slot_id];
	if (pCmd) {
		pIn_ctx = pCmd->pIn_ctx;

		pCmd_comp =&pCmd->completion;
		pCmd_status = &pCmd->status;
		pCmd->pCmd_trb = pXhci->cmd_ring->pEnq;

		if ((pCmd->pCmd_trb->link.trb_info
		     & TRB_TYPE_MASK) == TRB_TYPE(TRB_TYPE_LINK))
			pCmd->pCmd_trb =
				pXhci->cmd_ring->pEnq_seg->next->pTrbs;

		list_add_tail(&pCmd->cmd_list, &pVirt_dev->cmd_list);
	}
	else {


		pIn_ctx = pVirt_dev->in_ctx;
		pCmd_comp = &pVirt_dev->cmd_completion;
		pCmd_status = &pVirt_dev->cmd_status;
	}
	init_completion(*pCmd_comp);

	if (!is_ctx_chg)
		retval = ms_xhci_queue_config_ept_cmd(pXhci, pIn_ctx->dma_addr,
				pUdev->slot_id);
	else
		retval = ms_xhci_queue_eval_ctx_cmd(pXhci, pIn_ctx->dma_addr,
				pUdev->slot_id);
	if (retval < 0) {
		if (pCmd)
			list_del(&pCmd->cmd_list);
		ms_err("%s: fail to queue config ept cmd\n", __func__);
		return -ENOMEM;
	}
	ms_xhci_ring_cmd_db(pXhci);


	timeleft=ms_wait_for_completion_timeout(pXhci, pCmd_comp, MS_CTRL_SET_TIMEOUT);

	if (timeleft <= 0) {
		ms_err("%s: wait config ept cmd error (err=%d)\n",
			__func__, timeleft);
		/* FIXME cancel the configure endpoint command */
		return -ETIME;
	}

	if (!is_ctx_chg)
	{
		switch (*pCmd_status) {
		case TRB_COMP_RESOURCE_ERR:
			ms_err("%s: no memory\n", __func__);
			retval = -ENOMEM;
			break;

		case TRB_COMP_BANDWIDTH_ERR:
		case TRB_COMP_2ND_BANDWIDTH_ERR:
			ms_err("%s: bandwidth error\n", __func__);
			retval = -ENOSPC;
			break;

		case TRB_COMP_TRB_ERR:
			ms_err("%s: trb error\n", __func__);
			retval = -EINVAL;
			break;

		case TRB_COMP_INCOMP_DEV_ERR:
			ms_err("%s: incompatible dev error\n", __func__);
			retval = -ENODEV;
			break;

		case TRB_COMP_SUCCESS:
			ms_dbg("config ept cmd ok\n");
			retval = 0;
			break;

		default:
			ms_err("%s: unknow error %d\n", __func__, *pCmd_status);
			retval = -EINVAL;
			break;
		}
	}
	else
	{
		switch (*pCmd_status) {
		case TRB_COMP_PARAMETER_ERR:
			ms_err("%s: parameter error\n", __func__);
			retval = -EINVAL;
			break;

		case TRB_COMP_SLOT_ERR:
			ms_err("%s: slot not enabled\n", __func__);
		case TRB_COMP_CTX_STATE_ERR:
			ms_err("%s: ctx state error\n", __func__);
			ms_xhci_print_ctx(pXhci, pVirt_dev->out_ctx, 1);
			retval = -EINVAL;
			break;

		case TRB_COMP_INCOMP_DEV_ERR:
			ms_err("%s: incompatible dev error\n", __func__);
			retval = -ENODEV;
			break;

		case TRB_COMP_MEL_ERR:
			ms_err("%s: max exit latency to large error\n", __func__);
			retval = -EINVAL;
			break;

		case TRB_COMP_SUCCESS:
			ms_dbg("eval ept ctx ok\n");
			retval = 0;
			break;

		default:
			ms_err("%s: unknow error %d\n", __func__, *pCmd_status);
			retval = -EINVAL;
			break;
		}
	}

	return retval;
}

int ms_xhci_add_ept(struct xhci_hcd *pXhci, struct usb_device *pUdev,
		struct usb_host_endpoint *pEpt)
{
	struct xhci_container_ctx *pIn_ctx, *pOut_ctx;
	unsigned int ep_idx;
	struct xhci_ep_ctx *pEpt_ctx;
	struct xhci_slot_ctx *pSlot_ctx;
	struct xhci_input_control_ctx *pCtrl_ctx;
	u32 add_ctx;
	unsigned int last_ctx;
	struct xhci_my_device *pVirt_dev;

	if (is_usb_isoc_ept(&pEpt->desc))
	{
		ms_err("%s: not support isochronous ept\n", __func__);
		return -EINVAL;
	}

	add_ctx = ms_xhci_get_ept_ctrl_flag(&pEpt->desc);
	last_ctx = ms_xhci_last_valid_ept_idx(add_ctx);
	if (add_ctx == SLOT_FLAG || add_ctx == EP0_FLAG)
	{
		ms_err("%s: add flag can't set for slot or ep0 %x\n",
				__func__, add_ctx);
		return 0;
	}

	pVirt_dev = pXhci->devs[pUdev->slot_id];
	pIn_ctx = pVirt_dev->in_ctx;
	pOut_ctx = pVirt_dev->out_ctx;
	pCtrl_ctx = ms_xhci_get_input_control_ctx(pIn_ctx);
	ep_idx = ms_xhci_get_ept_idx(&pEpt->desc);
	pEpt_ctx = ms_xhci_get_ep_ctx(pXhci, pOut_ctx, ep_idx);

	if (pVirt_dev->eps[ep_idx].ring &&
			!(pCtrl_ctx->drop_ctx_flags &
				ms_xhci_get_ept_ctrl_flag(&pEpt->desc))) {
		ms_err("%s: add a ept already in use without drop\n", __func__);
		return -EINVAL;
	}

	if (pCtrl_ctx->add_ctx_flags & ms_xhci_get_ept_ctrl_flag(&pEpt->desc))
	{
		ms_dbg("add a ept already in use\n");
		return 0;
	}

	if (ms_xhci_ept_init(pXhci, pVirt_dev, pUdev, pEpt, 0) < 0) {
		ms_err("%s: fail to init ept %x\n",
				__func__, pEpt->desc.bEndpointAddress);
		return -ENOMEM;
	}

	pCtrl_ctx->add_ctx_flags |= add_ctx;

	pSlot_ctx = ms_xhci_get_slot_ctx(pXhci, pIn_ctx);
	if ((pSlot_ctx->dev_slot_info1 & DEV_CTX_ENTRY_MASK) <
	    DEV_CTX_ENTRY(last_ctx)) {
		pSlot_ctx->dev_slot_info1 &= ~DEV_CTX_ENTRY_MASK;
		pSlot_ctx->dev_slot_info1 |= DEV_CTX_ENTRY(last_ctx);
	}

	pEpt->hcpriv = pUdev;

	ms_dbg("add ep %x with new add flag %x, drop flag %x, slot info %x to slot id: %d\n",
			(unsigned int) pEpt->desc.bEndpointAddress,
			pCtrl_ctx->add_flags,
			pCtrl_ctx->drop_flags,
			pSlot_ctx->dev_slot_info1,
			pUdev->slot_id);
	return 0;
}

int ms_xhci_drop_ept(struct xhci_hcd *pXhci, struct usb_device *pUdev,
		struct usb_host_endpoint *pEpt)
{
	struct xhci_container_ctx *pIn_ctx, *pOut_ctx;
	struct xhci_input_control_ctx *pCtrl_ctx;
	struct xhci_slot_ctx *pSlot_ctx;
	unsigned int last_ctx;
	unsigned int ep_idx;
	struct xhci_ep_ctx *pEpt_ctx;
	u32 drop_flag;

	drop_flag = ms_xhci_get_ept_ctrl_flag(&pEpt->desc);
	if (drop_flag == SLOT_FLAG || drop_flag == EP0_FLAG) {
		ms_err("%s: drop flag can't set for slot or ep0 %x\n",
				__func__, drop_flag);
		return 0;
	}

	pIn_ctx = pXhci->devs[pUdev->slot_id]->in_ctx;
	pOut_ctx = pXhci->devs[pUdev->slot_id]->out_ctx;
	pCtrl_ctx = ms_xhci_get_input_control_ctx(pIn_ctx);
	ep_idx = ms_xhci_get_ept_idx(&pEpt->desc);
	pEpt_ctx = ms_xhci_get_ep_ctx(pXhci, pOut_ctx, ep_idx);

	if ((pEpt_ctx->ep_ctx_field1 & EP_STATE_MASK) == EP_STATE_DISABLED ||
	    pCtrl_ctx->drop_ctx_flags & ms_xhci_get_ept_ctrl_flag(&pEpt->desc))
	{
		ms_dbg("%s: drop a ept already disabled\n", __func__);
		return 0;
	}

	pCtrl_ctx->drop_ctx_flags |= drop_flag;

	pCtrl_ctx->add_ctx_flags &= ~drop_flag;

	last_ctx = ms_xhci_last_valid_ept_idx(pCtrl_ctx->add_ctx_flags);
	pSlot_ctx = ms_xhci_get_slot_ctx(pXhci, pIn_ctx);

	if ((pSlot_ctx->dev_slot_info1 & DEV_CTX_ENTRY_MASK) > DEV_CTX_ENTRY(last_ctx))
	{
		pSlot_ctx->dev_slot_info1 &= ~DEV_CTX_ENTRY_MASK;
		pSlot_ctx->dev_slot_info1 |= DEV_CTX_ENTRY(last_ctx);
	}

	ms_xhci_ept_ctx_zero(pXhci, pXhci->devs[pUdev->slot_id], pEpt);

	ms_dbg("drop ep %x with new add flag %x, drop flag %x, slot info %x to slot id: %d\n",
			(unsigned int) pEpt->desc.bEndpointAddress,
			pCtrl_ctx->add_flags,
			pCtrl_ctx->drop_flags,
			pSlot_ctx->dev_slot_info1,
			pUdev->slot_id);
	return 0;
}


void ms_xhci_zero_input_ctx(struct xhci_hcd *pXhci, struct xhci_my_device *pVirt_dev)
{
	struct xhci_input_control_ctx *pCtrl_ctx;
	struct xhci_ep_ctx *pEpt_ctx;
	struct xhci_slot_ctx *pSlot_ctx;
	int i;

	pCtrl_ctx = ms_xhci_get_input_control_ctx(pVirt_dev->in_ctx);
	pCtrl_ctx->drop_ctx_flags = 0;
	pCtrl_ctx->add_ctx_flags = 0;
	pSlot_ctx = ms_xhci_get_slot_ctx(pXhci, pVirt_dev->in_ctx);
	pSlot_ctx->dev_slot_info1 &= ~DEV_CTX_ENTRY_MASK;

	pSlot_ctx->dev_slot_info1 |= DEV_CTX_ENTRY(1);
	for (i = 1; i < 31; ++i) {
		pEpt_ctx = ms_xhci_get_ep_ctx(pXhci, pVirt_dev->in_ctx, i);
		pEpt_ctx->ep_ctx_field1 = 0;
		pEpt_ctx->ep_ctx_field2 = 0;
		pEpt_ctx->tr_deq_ptr = 0;
		pEpt_ctx->avg_trb_len = 0;
	}
}

void ms_xhci_reset_bandwidth(struct xhci_hcd *pXhci, struct usb_device *pUdev)
{
	struct xhci_my_device	*pVirt_dev;
	int i;

	pVirt_dev = pXhci->devs[pUdev->slot_id];
	for (i = 0; i < 31; ++i) {
		if (pVirt_dev->eps[i].ring) {
			ms_xhci_ring_free(pVirt_dev->eps[i].ring);
			pVirt_dev->eps[i].ring = NULL;
		}
	}
	ms_xhci_zero_input_ctx(pXhci, pVirt_dev);
}

int ms_xhci_alloc_bandwidth(struct xhci_hcd *pXhci,struct usb_device *pUdev,
		struct usb_host_config *pConfig,
		struct usb_host_interface *pCur_alt,
		struct usb_host_interface *pNew_alt)
{
	int num_intfs, i, j;
	struct usb_host_interface *pTmp_alt = NULL;
	int retval = 0;
	struct usb_host_endpoint *pEpt;

	if (pConfig) {
		num_intfs = pConfig->desc.bNumInterfaces;

		for (i = 1; i < 16; ++i) {
			pEpt = pUdev->ep_out[i];
			if (pEpt) {
				retval = ms_xhci_drop_ept(pXhci, pUdev, pEpt);
				if (retval < 0)
					goto reset;
			}
			pEpt = pUdev->ep_in[i];
			if (pEpt) {
				retval = ms_xhci_drop_ept(pXhci, pUdev, pEpt);
				if (retval < 0)
					goto reset;
			}
		}
		for (i = 0; i < num_intfs; ++i) {
			struct usb_host_interface *first_alt;
			int iface_num;

			first_alt = &pConfig->intf_cache[i]->altsetting[0];
			iface_num = first_alt->desc.bInterfaceNumber;

			pTmp_alt = ms_usb_find_alts(pConfig, iface_num, 0);
			if (!pTmp_alt)
				pTmp_alt = first_alt;

			for (j = 0; j < pTmp_alt->desc.bNumEndpoints; j++) {
				retval = ms_xhci_add_ept(pXhci, pUdev, &pTmp_alt->endpoint[j]);
				if (retval < 0)
					goto reset;
			}
		}
	}
	retval = ms_xhci_chk_bandwidth(pXhci, pUdev);
reset:
	if (retval < 0)
	           ms_xhci_reset_bandwidth(pXhci,pUdev);
	return retval;
}

int ms_xhci_chk_bandwidth(struct xhci_hcd *pXhci, struct usb_device *pUdev)
{
	int i;
	int retval = 0;
	struct xhci_my_device	*pVirt_dev;
	struct xhci_input_control_ctx *pCtrl_ctx;
	struct xhci_slot_ctx *pSlot_ctx;

	pVirt_dev = pXhci->devs[pUdev->slot_id];

	ms_dbg("check bandwidth for Slot %d virt dev:%x input ctx = 0x%x n",
		pUdev->slot_id, (u32)pVirt_dev,	(u32)pVirt_dev->in_ctx);

	pCtrl_ctx = ms_xhci_get_input_control_ctx(pVirt_dev->in_ctx);
	pCtrl_ctx->add_ctx_flags |= SLOT_FLAG;
	pCtrl_ctx->add_ctx_flags &= ~EP0_FLAG;
	pCtrl_ctx->drop_ctx_flags &= ~(SLOT_FLAG | EP0_FLAG);

	if (pCtrl_ctx->add_ctx_flags == SLOT_FLAG && pCtrl_ctx->drop_ctx_flags == 0)
		return 0;

	ms_dbg("input control ctx:\n");
	pSlot_ctx = ms_xhci_get_slot_ctx(pXhci, pVirt_dev->in_ctx);
	ms_xhci_print_ctx(pXhci, pVirt_dev->in_ctx,
		     DEV_CTX_ENTRY_TO_EP_NUM(le32_to_cpu(pSlot_ctx->dev_slot_info1)));

	retval = mx_xhci_config_ept_cmd(pXhci, pUdev, NULL,
			false);

	if (retval) {
		ms_err("%s: config ept cmd fail (err=%d)\n", __func__, retval);
		return retval;
	}

	ms_dbg("output ctx:\n");
	ms_xhci_print_ctx(pXhci, pVirt_dev->out_ctx,
		     DEV_CTX_ENTRY_TO_EP_NUM(le32_to_cpu(pSlot_ctx->dev_slot_info1)));

	for (i = 1; i < 31; ++i) {
		if ((pCtrl_ctx->drop_ctx_flags & (1 << (i + 1))) &&
		    !(pCtrl_ctx->add_ctx_flags & (1 << (i + 1))))
			pVirt_dev->eps[i].ring = NULL;
	}
	ms_xhci_zero_input_ctx(pXhci, pVirt_dev);

	return retval;
}

void ms_xhci_reset_ept(struct usb_host_endpoint *pEpt)
{
	struct xhci_hcd *pXhci = &ms_xhci;
	struct usb_device *pUdev;
	unsigned int ep_idx;
	int retval;
	struct xhci_my_ep *pVirt_ep;
	int timeleft;
	struct xhci_my_device *pVirt_dev;

	if (!pEpt->hcpriv)
		return;
	pUdev = (struct usb_device *) pEpt->hcpriv;
	pVirt_dev = pXhci->devs[pUdev->slot_id];
	ep_idx = ms_xhci_get_ept_idx(&pEpt->desc);
	pVirt_ep = &pXhci->devs[pUdev->slot_id]->eps[ep_idx];
	if (!pVirt_ep->pStop_td) {
		ms_dbg("ept %x has no stopped td, don't reset it.\n", pEpt->desc.bEndpointAddress);
		return;
	}
	if (is_usb_control_ept(&pEpt->desc)) {
		ms_dbg("no need to reset control ept.\n");
		return;
	}

	ms_dbg("queue reset ept cmd\n");
	retval = ms_xhci_queue_reset_ept_cmd(pXhci, pUdev->slot_id, ep_idx);

	if (!retval) {
		ms_xhci_clear_stalled_ring(pXhci, pUdev, ep_idx);
		kfree(pVirt_ep->pStop_td);
		pVirt_dev->reset_ep_done = 0;
		ms_xhci_ring_cmd_db(pXhci);

		timeleft=ms_wait_for_completion_timeout(pXhci, &(pVirt_dev->reset_ep_done), MS_CTRL_SET_TIMEOUT);
		if (timeleft <= 0) {
			ms_err("%s: wait reset ept cmd error (err=%d)\n",	__func__, timeleft);

			return;
		}
	}
	pVirt_ep->pStop_td = NULL;
	pVirt_ep->pStop_trb = NULL;

	if (retval)
		ms_err("%s: fail to queue reset ept cmd (err=%d)\n", __func__, retval);
}


