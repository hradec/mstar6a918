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

#ifndef __MS_XHCI_H
#define __MS_XHCI_H

#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include "compat.h"
#include <linux/list.h>
#include <usb.h>
#include <asm/errno.h>

#include "xhci-ctx.h"
#include "xhci-reg.h"

#define MSTAR_XHCI_MAX_HALT_USEC	(16*1000)
//Event Interrupt Enable  | Host System Error Interrupt Enable | Enable Wrap Event
#define MSTAR_XHCI_IRQS		((1 << 2) | (1 << 3) | (1 << 10))

#include "xhci-mstar.h"

struct api_context {
	int	done;
	int	status;
};

#define XHCI_MAX_SLOTS		256

struct xhci_dcbaa {
	u64			dev_ctx_bases[XHCI_MAX_SLOTS];
	dma_addr_t	dma;
};

struct xhci_my_cmd {
	struct xhci_container_ctx	*pIn_ctx;  //Input context
	u32				status;
	int					completion;  //completion for waiting
	union xhci_trb			*pCmd_trb;
	struct list_head		cmd_list;
};

struct xhci_my_ep {
	struct xhci_ring		*ring; //endpoint's transfer ring

	unsigned int			ep_state;
	struct list_head	cancel_td_list;  //for URB cancel
	union xhci_trb		*pStop_trb;     //stopped trb
	struct xhci_td		*pStop_td;      //stopped td

	struct xhci_seg	*set_deq_seg;   //new dequeue segment want to be set
	union xhci_trb		*set_deq_ptr;          //new dequeue pointer want to be set

	bool			skip;
};

#define EP_ST_SET_DEQ_PENDING		0x01
#define EP_ST_HALTED				0x02
#define EP_ST_HALT_PENDING			0x04

struct xhci_my_device {
	struct usb_device		*pUdev;

	struct xhci_container_ctx       *out_ctx; //output context of device
	struct xhci_container_ctx       *in_ctx;

	int				addr; //The address assigned to the device
	struct xhci_my_ep		eps[31];
	int         		cmd_completion;
	int   		reset_ep_done;

	u32				cmd_status;
	struct list_head		cmd_list;
};


#define BUS2PA(A)	\
	(((A>=MIU0_BUS_BASE_ADDR)&&(A<(MIU0_BUS_BASE_ADDR+MIU0_SIZE)))?	\
		(A-MIU0_BUS_BASE_ADDR+MIU0_PHY_BASE_ADDR):	\
		(((A>= MIU1_BUS_BASE_ADDR)&&(A<MIU1_BUS_BASE_ADDR+MIU1_SIZE))?	\
			(A-MIU1_BUS_BASE_ADDR+MIU1_PHY_BASE_ADDR):	\
			(0xFFFFFFFF)))

#define PA2BUS(A)	\
	(((A>=MIU0_PHY_BASE_ADDR)&&(A<(MIU0_PHY_BASE_ADDR+MIU0_SIZE)))?	\
		(A-MIU0_PHY_BASE_ADDR+MIU0_BUS_BASE_ADDR):	\
		(((A>= MIU1_PHY_BASE_ADDR)&&(A<MIU1_PHY_BASE_ADDR+MIU1_SIZE))?	\
			(A-MIU1_PHY_BASE_ADDR+MIU1_BUS_BASE_ADDR):	\
			(0xFFFFFFFF)))


#define MAX_TRBS_IN_SEG	64
#define XHCI_SEG_SIZE		(MAX_TRBS_IN_SEG*16)
#define MAX_TRB_BUF_SIZE	0x10000

struct xhci_seg {
	union xhci_trb		*pTrbs;
	struct xhci_seg	*next;
	dma_addr_t		dma;
};

struct xhci_td {
	struct list_head	td_list;
	struct list_head	cancel_td_list;
	struct urb		*pUrb;
	struct xhci_seg	*pStart_seg;
	union xhci_trb		*pFirst_trb;
	union xhci_trb		*pLast_trb;
};

struct xhci_deq_state {
	struct xhci_seg *pDeq_seg;
	union xhci_trb *pDeq_ptr;
	int cycle_state;
};

struct xhci_ring {
	struct xhci_seg	*pFirst_seg;
	union  xhci_trb		*pEnq;
	struct xhci_seg	*pEnq_seg;
	union  xhci_trb		*pDeq;
	struct xhci_seg	*pDeq_seg;
	struct list_head	td_list;
	u32			cycle_state;
	bool			last_td_short;
};

struct xhci_erst {
	struct xhci_event_ring_seg_table_entry	*entries;
	unsigned int		entry_count;
	dma_addr_t		dma_addr;
	unsigned int		size;
};


struct xhci_scratchpad {
	u64 *sp_array;
	dma_addr_t dma_addr;

	void *sp_buffers[5];
	dma_addr_t sp_dma_buffers[5];
};

struct urb_priv {
	int	len;
	int	td_count;
	struct	xhci_td	*td[0];
};

#define	MS_MAX_ERST_SEGS	1

struct xhci_hcd {
	void __iomem		*regs;
	struct xhci_cap_regs __iomem *cap_regs;
	struct xhci_op_regs __iomem *op_regs;
	struct xhci_run_regs __iomem *run_regs;
	struct xhci_doorbell_array __iomem *doorbells;
	struct	xhci_intr_reg __iomem *intr_regs;

	__u32		hcc_params;

	u16		hci_version;

	int		page_size;
	int		page_shift;

	struct xhci_dcbaa *dcbaa;
	struct xhci_ring	*cmd_ring;
	struct xhci_ring	*event_ring;
	struct xhci_erst	erst;

	struct xhci_scratchpad  scratchpad;

	int	addr_dev;
	int   kill_urb_done;
	int slot_id;
	struct xhci_my_device	*devs[XHCI_MAX_SLOTS];

	unsigned int		xhc_state;

	u32			command;

#define XHCI_STATE_DYING	(1 << 0)
#define XHCI_STATE_HALTED	(1 << 1)
	int			error_bitmask;

	u32 	utmi_base;
	u32 	xhci_base;
	u32 	u3phy_d_base;
	u32 	u3phy_a_base;
	u32 	u3top_base;
	u32 	u3indctl_base;

};

static inline unsigned int xhci_readl(const struct xhci_hcd *xhci,
		__le32 __iomem *regs)
{
#if (XHCI_RIU_PATCH)
		__u32 __iomem *tmp;
		tmp = (u32 *)( ((unsigned int)regs & 0xffff0000) + (((unsigned int)regs & 0x0000FFFF)<<2) );
		return readl(tmp);

#else
		return readl(regs);
#endif
}
static inline void xhci_writel(struct xhci_hcd *xhci,
		const unsigned int val, __le32 __iomem *regs)
{
#if (XHCI_RIU_PATCH)
		__u32 __iomem *tmp;
		tmp = (u32 *)( ((unsigned int)regs & 0xffff0000) + (((unsigned int)regs & 0x0000FFFF)<<2) );
		writel(val, tmp);

#else
		writel(val, regs);
#endif
}

static inline u64 xhci_read_64(const struct xhci_hcd *xhci,
		__le64 __iomem *regs)
{
	__u32 __iomem *ptr = (__u32 __iomem *) regs;

#if (XHCI_RIU_PATCH)
	u64 val_lo = xhci_readl(xhci, ptr);
	u64 val_hi = xhci_readl(xhci, ptr + 1);
#else
	u64 val_lo = readl(ptr);
	u64 val_hi = readl(ptr + 1);
#endif
	return val_lo + (val_hi << 32);
}
static inline void xhci_write_64(struct xhci_hcd *xhci,
				 const u64 val, __le64 __iomem *regs)
{
	__u32 __iomem *ptr = (__u32 __iomem *) regs;
	u32 val_lo = val & 0xFFFFFFFF;
	u32 val_hi = (val>>32) & 0xFFFFFFFF;

#if (XHCI_RIU_PATCH)
	xhci_writel(xhci, val_lo, ptr);
	xhci_writel(xhci, val_hi, ptr + 1);
#else
	writel(val_lo, ptr);
	writel(val_hi, ptr + 1);
#endif
}

#define MS_CTRL_SET_TIMEOUT 10000
#define MS_CTRL_GET_TIMEOUT 5000

#define MS_BULK_TIMEOUT      20000

void ms_xhci_print_interrupt_reg_set(struct xhci_hcd *pXhci, int Ir_num);
void ms_xhci_dump_regs(struct xhci_hcd *pXhci);
void ms_xhci_print_regs(struct xhci_hcd *pXhci);
void ms_xhci_print_run_regs(struct xhci_hcd *pXhci);
void ms_xhci_print_trb(union xhci_trb *pTrb);
void ms_xhci_dump_trb(struct xhci_hcd *pXhci, union xhci_trb *pTrb);
void ms_xhci_print_seg(struct xhci_seg *seg);
void ms_xhci_dump_ring(struct xhci_ring *ring);
void ms_xhci_dump_erst(struct xhci_erst *pErst);
void ms_xhci_print_cmd_ptrs(struct xhci_hcd *pXhci);
void ms_xhci_print_ring_ptrs(struct xhci_ring *ring);
void ms_xhci_print_ctx(struct xhci_hcd *pXhci,
		  struct xhci_container_ctx *pCtx,
		  unsigned int last_ep_num);

int ms_xhci_mem_init(struct xhci_hcd *pXhci);
void ms_xhci_free_virt_dev(struct xhci_hcd *pXhci, int slot_id);
int ms_xhci_alloc_virt_dev(struct xhci_hcd *pXhci, int slot_id, struct usb_device *pUdev, gfp_t flags);
int ms_xhci_setup_virt_dev(struct xhci_hcd *xhci, struct usb_device *udev);
void ms_xhci_copy_ep0_deq_to_input_ctx(struct xhci_hcd *pXhci,
		struct usb_device *pUdev);
unsigned int ms_xhci_get_ept_idx(struct usb_endpoint_descriptor *pDesc);
unsigned int ms_xhci_get_ept_ctrl_flag(struct usb_endpoint_descriptor *pDesc);
unsigned int ms_xhci_last_valid_ept_idx(u32 add_flags);
void ms_xhci_ep_ctx_copy(struct xhci_hcd *pXhci,
		struct xhci_container_ctx *pIn_ctx,
		struct xhci_container_ctx *pOut_ctx,
		unsigned int ep_index);
void ms_xhci_ring_free(struct xhci_ring *ring);
struct xhci_my_cmd *ms_xhci_alloc_cmd(struct xhci_hcd *pXhci,
		bool is_alloc_completion,
		gfp_t mem_flags);
void ms_xhci_free_urb_priv(struct urb_priv *pUrb_priv);
void ms_xhci_free_cmd(struct xhci_my_cmd *pCmd);

int ms_xhci_halt(struct xhci_hcd *pXhci);
int ms_xhci_reset(struct xhci_hcd *pXhci);
int ms_xhci_init(struct xhci_hcd *pXhci);
int ms_xhci_run(struct xhci_hcd *pXhci);

#define	xhci_suspend	NULL
#define	xhci_resume	NULL

dma_addr_t ms_xhci_trb_to_dma(struct xhci_seg *pSeg, union xhci_trb *pTrb);

struct xhci_input_control_ctx *ms_xhci_get_input_control_ctx(struct xhci_container_ctx *ctx);
struct xhci_slot_ctx *ms_xhci_get_slot_ctx(struct xhci_hcd *xhci, struct xhci_container_ctx *ctx);
struct xhci_ep_ctx *ms_xhci_get_ep_ctx(struct xhci_hcd *pXhci, struct xhci_container_ctx *ctx, unsigned int ep_index);

int ms_xhci_port_connect(struct xhci_hcd *pXhci);
int ms_xhci_dev_enum(struct xhci_hcd *xhci, struct usb_device *dev, int rh_port);
int ms_xhci_alloc_dev(struct xhci_hcd *pXhci, struct usb_device *pUdev);

int ms_xhci_irq_polling(struct xhci_hcd *pXhci);

int ms_xhci_queue_slot_cmd(struct xhci_hcd *pXhci, u32 u32Cmd_type, u32 u32Slot_id);
void ms_xhci_ring_cmd_db(struct xhci_hcd *pXhci);
int ms_xhci_queue_address_dev_cmd(struct xhci_hcd *pXhci, dma_addr_t in_ctx_dma,
		u32 u32Slot_id);
int ms_xhci_queue_stop_ept_cmd(struct xhci_hcd *pXhci, int slot_id,
		unsigned int ep_idx, int suspend);
int ms_xhci_queue_ctrl_tx(struct xhci_hcd *xhci, gfp_t mem_flags, struct urb *urb,
		int slot_id, unsigned int ep_index);
int ms_xhci_queue_bulk_tx(struct xhci_hcd *pXhci, gfp_t mem_flags,
		struct urb *pUrb, int slot_id, unsigned int ep_idx);
int ms_xhci_queue_config_ept_cmd(struct xhci_hcd *pXhci, dma_addr_t in_ctx_dma,
		u32 u32Slot_id);
int ms_xhci_queue_eval_ctx_cmd(struct xhci_hcd *pXhci, dma_addr_t in_ctx_dma,
		u32 u32Slot_id);
int ms_xhci_queue_reset_ept_cmd(struct xhci_hcd *pXhci, int slot_id,
		unsigned int ep_idx);
int ms_xhci_queue_reset_dev_cmd(struct xhci_hcd *pXhci, u32 u32Slot_id);
void ms_xhci_get_new_deq_state(struct xhci_hcd *pXhci,
		unsigned int slot_id, unsigned int ep_idx,
		struct xhci_td *pTd,
		struct xhci_deq_state *pDeq_state);
void ms_xhci_set_new_deq_state(struct xhci_hcd *pXhci,
		unsigned int slot_id, unsigned int ep_idx,
		struct xhci_deq_state *pDeq_state);
void ms_usb_enable_ept(struct usb_device *pDev, struct usb_host_endpoint *pEpt);
int ms_xhci_is_vendor_code(unsigned int comp_code);
int USB3_enumerate(struct usb_device *pdev);
int ms_xhci_ept_init(struct xhci_hcd *xhci,
		struct xhci_my_device *virt_dev,
		struct usb_device *udev,
		struct usb_host_endpoint *ep,
		gfp_t mem_flags);
void ms_xhci_clear_stalled_ring(struct xhci_hcd *xhci,
		struct usb_device *udev, unsigned int ep_index);
int ms_xhci_chk_bandwidth(struct xhci_hcd *pXhci, struct usb_device *pUdev);
struct usb_host_interface *ms_usb_find_alts(
		struct usb_host_config *pConfig,
		unsigned int intf_num,
		unsigned int alt_num);
struct usb_host_interface *ms_usb_altno_to_alt(
					const struct usb_interface *pIntf,
					unsigned int altno);

#endif /* __MS_XHCI_H */

