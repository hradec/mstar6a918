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

#ifndef __MS_XHCI_CTX_H
#define __MS_XHCI_CTX_H


#define LAST_EP_INDEX			30

/* --- Used for a Device or Input Context --- */
struct xhci_container_ctx {
	unsigned type;
	u8 *pBuf;
	dma_addr_t dma_addr;
	int size;
};
// Define context type
#define CTX_TYPE_DEVICE  0x1
#define CTX_TYPE_INPUT   0x2

/*-------------------------------------*/

/* --- Used for a Input Control Context --- */
struct xhci_input_control_ctx {
	u32	drop_ctx_flags;
	u32	add_ctx_flags;
	u32	reserved[6];
};
/*-------------------------------------*/

/* ----- Used for a device slot context ------ */
struct xhci_slot_ctx {
	u32	dev_slot_info1;
	u32	dev_slot_info2;
	u32	dev_slot_tt_info;
	u32	dev_slot_state_info;
	/* offset 0x10 to 0x1f reserved for HC internal use */
	u32	reserved[4];
};

/* --- dev_slot_info1 --- */
/* Bit 0:19 - Route String */
#define DEV_ROUTE_STRING_MASK  (0xfffff)
/* Bit 0:19 - device speed  */
#define DEV_SPEED              (0xf << 20)
/* Bit 24 - reserved */
/* Bit 25 - MTT : LS/FS device under HS hub  */
#define DEV_MTT                (0x1 << 25)
/* Bit 26 -Hub */
#define DEV_HUB                (0x1 << 26)
/* Bit 27:31 - Context Entries : index of the last valid endpoint context */
#define DEV_CTX_ENTRY_MASK     (0x1f << 27)
#define DEV_CTX_ENTRY(x)       ((x) << 27)
#define DEV_CTX_ENTRY_TO_EP_NUM(x) (((x) >> 27) - 1)
/* ctx index */
#define SLOT_FLAG              (1 << 0)
#define EP0_FLAG               (1 << 1)

/* --- dev_slot_info2 --- */
/* Bit 0:15 - Max Exit Latency (ms) */
#define DEV_CTX_MAX_EXIT_LATENCY(x) ((x) & 0xffff)
/* Bit 16:23 -Root hub port number  */
#define DEV_CTX_ROOT_HUB_PORT(x)	(((x) & 0xff) << 16)
#define GET_DEV_CTX_ROOT_HUB_PORT(x)	(((x) >> 16) & 0xff)
/* Bit 16:23 - number of ports supported by the hub */
#define DEV_CTX_HUB_NUM_PORTS(x)	(((x) & 0xff) << 24)

/* --- dev_slot_tt_info --- */
/* Bit 0:7 - TT Hub Slot ID */
#define DEV_CTX_TT_HUB_SLOT_ID(x)   ((x) & 0xff)
/* Bit 8:15 - TT Port Number */
#define DEV_CTX_TT_PORT_NUM(x)      (((x) & 0xff) << 8)
/* Bit 16:17 - TT Think Time */
#define DEV_CTX_TT_THINK_TIME(x)    (((x) & 0x3) << 16)
/* Bit 18:21 - Reserved */
/* Bit 22:31 - Interrupt Target */

/* --- dev_slot_state_info --- */
/* Bit 0:7 -USB device address  */
#define DEV_CTX_DEV_ADDR_MASK	(0xff)
/* Bit 8:26 - Reserved */
/* Bit 27:31 - Slot state */
#define GET_DEV_CTX_SLOT_STATE(x)	(((x) & (0x1f << 27)) >> 27)
#define DEV_CTX_SLOT_STATE_DISABLED     0
#define DEV_CTX_SLOT_STATE_ENABLED      0
#define DEV_CTX_SLOT_STATE_DEFAULT      1
#define DEV_CTX_SLOT_STATE_ADDRESSED    2
#define DEV_CTX_SLOT_STATE_CONFIGURED   3

/*-------------------------------------*/

/* ----- Used for endpoint context ------ */

struct xhci_ep_ctx {
	u32	ep_ctx_field1;
	u32	ep_ctx_field2;
	u64	tr_deq_ptr;
	u32	avg_trb_len;
	u32	reserved[3];
};

/* -- ep_ctx_field1 -- */
/* bit 0:2 EP State */
#define EP_STATE_MASK		(0xF)
#define EP_STATE_DISABLED	0
#define EP_STATE_RUNNING	1
#define EP_STATE_HALTED		2
#define EP_STATE_STOPPED	3
#define EP_STATE_ERROR		4
    /* EP state 5~7 - reserved */
/* bit 3:7 reserved */
/* bit 8:9 Mult */
#define EP_MULT(x)      (((x) & 0x3) << 8)
/* bit 10:14  MaxPrimStreams */
/* bit 15   LSA */
/* bit 16:23  Interval */
#define EP_INTERVAL(x)  (((x) & 0xFF) << 16)
/* bit 24:31 reserved */

/* -- ep_ctx_field2 -- */
/* bit 0  reserved */
/* bit 1:2 CErr */
#define EP_CERR(x)	(((x) & 0x3) << 1)
/* bit 3:5 EP Type */
#define EP_TYPE(x)	(((x) & 0x7) << 3)
#define EP_TYPE_ISOC_OUT	1
#define EP_TYPE_BULK_OUT	2
#define EP_TYPE_INT_OUT		3
#define EP_TYPE_CTRL		4
#define EP_TYPE_ISOC_IN		5
#define EP_TYPE_BULK_IN		6
#define EP_TYPE_INT_IN		7
/* bit 6  reserved */
/* bit 7  HID */
/* bit 8:15 Max Burst Size */
#define EP_MAX_BURST_SIZE(x)        (((x)&0xFF) << 8)
/* bit 16:31 Max Packet Size */
#define EP_MAX_PACKET_SIZE(x)		(((x)&0xFFFF) << 16)
#define GET_EP_MAX_PACKET_SIZE(x)	(((x) >> 16) & 0xFFFF)
#define EP_MAX_PACKET_SIZE_MASK		(0xFFFF << 16)
/* EP descriptor bit 10..0 : the max packet size  */
#define GET_MAX_PACKET_SIZE(x)		((x) & 0x7FF)

/* -- tr_deq_ptr -- */
/* bit 0  DCS */
#define EP_DCS_MASK					(1 << 0)

/* -- avg_trb_len -- */
/* bit 0:15 Average TRB Length */
#define EP_AVG_TRB_LENGTH(x)		((x) & 0xFFFF)
/* bit 16:31 Max ESIT Payload */
#define EP_MAX_ESIT_PAYLOAD(x)		(((x) & 0xFFFF) << 16)

/*-------------------------------------*/

/* ----- Used for TRB ------ */
struct xhci_generic_trb {
	u32 trb_info[4];
};
/*-------------------------------------*/


/* ----- Used for transfer event ------ */
struct xhci_transfer_event {
	u64	trb_ptr;
	u32	trb_transfer_len;
	u32	trb_info;
};

/* TRB completion code definition */
#define	COMP_CODE_MASK		(0xFF << 24)
#define GET_COMP_CODE(x)	(((x) & COMP_CODE_MASK) >> 24)
#define TRB_COMP_SUCCESS			1	/* Success */
#define TRB_COMP_DATA_BUFFER_ERR	2	/* Data Buffer Error */
#define TRB_COMP_BABBLE_ERR			3	/* Babble Detected Error */
#define TRB_COMP_TRANS_ERR			4	/* USB Transaction Error */
#define TRB_COMP_TRB_ERR			5	/* TRB Error - TRB parameter error */
#define TRB_COMP_STALL_ERR			6	/* Stall Error */
#define TRB_COMP_RESOURCE_ERR		7	/* Resource Error */
#define TRB_COMP_BANDWIDTH_ERR		8	/* Bandwidth Error */
#define TRB_COMP_NOSLOTS_ERR		9	/* No Slots Available Error */
#define TRB_COMP_STREAM_ERR			10	/* Invalid Stream Type Error */
#define TRB_COMP_SLOT_ERR			11	/* Slot Not Enabled Error */
#define TRB_COMP_ENDP_ERR			12	/* Endpoint Not Enabled Error */
#define TRB_COMP_SHORT_PACKET		13	/* Short Packet */
#define TRB_COMP_RING_UNDERRUN		14	/* Ring Underrun */
#define TRB_COMP_RING_OVERRUN		15	/* Ring Overrun */
#define TRB_COMP_VF_FULL_ERR		16	/* VF Event Ring Full Error */
#define TRB_COMP_PARAMETER_ERR		17	/* Parameter Error - Context parameter is invalid */
#define TRB_COMP_BANDWIDTH_OVERRUN	18	/* Bandwidth Overrun Error */
#define TRB_COMP_CTX_STATE_ERR		19	/* Context State Error */
#define TRB_COMP_PING_ERR			20	/* No Ping Response Error */
#define TRB_COMP_ER_FULL_ERR		21	/* Event Ring is full */
#define TRB_COMP_INCOMP_DEV_ERR		22	/* Incompatible Device Error */
#define TRB_COMP_MISSED_SERVICE_ERR	23	/* Missed Service Error */
#define TRB_COMP_CMD_STOP			24	/* Command Ring Stopped */
#define TRB_COMP_CMD_ABORT			25	/* Command Aborted */
#define TRB_COMP_STOPPED			26	/* Stopped - transfer was terminated */
#define TRB_COMP_STOP_LENGTH_INVAL	27	/* Stopped - length invalid */
#define TRB_COMP_DBG_ABORT			28	/* Control Abort Error - Debug Capability */
#define TRB_COMP_MEL_ERR			29	/* Max Exit Latency Too Large Error */
		/* TRB complete code  30 : reserved */
#define TRB_COMP_ISO_BUFFER_OVERRUN	31	/* Isoc Buffer Overrun */
#define TRB_COMP_EVENT_LOST_ERR		32	/* Event Lost Error */
#define TRB_COMP_UNDEFINED_ERR		33	/* Undefined Error */
#define TRB_COMP_INVAL_STRAM_ID_ERR	34	/* Invalid Stream ID Error */
#define TRB_COMP_2ND_BANDWIDTH_ERR	35	/* Secondary Bandwidth Error */
#define	TRB_COMP_SPLIT_TRANS_ERR	36	/* Split Transaction Error */

/* trb_info : Endpoint ID */
#define	TRB_TO_EP_ID(x)			(((x) >> 16) & 0x1f)

/* TRB type */
#define	TRB_TYPE_MASK			(0xFC00)
#define TRB_TYPE(x)				((x) << 10)
#define GET_TRB_TYPE(x)			(((x) & TRB_TYPE_MASK) >> 10)
/* TRB type IDs */
	/* Transfer Ring */
#define TRB_TYPE_NORMAL			1	/* bulk, interrupt, isoc scatter/gather, and control data stage */
#define TRB_TYPE_SETUP			2	/* setup stage */
#define TRB_TYPE_DATA			3	/* data stage */
#define TRB_TYPE_STATUS			4	/* status stage */
#define TRB_TYPE_ISOC			5	/* isoc */
#define TRB_TYPE_LINK			6	/* Link */
#define TRB_TYPE_EVENT_DATA		7	/* Event data */
#define TRB_TYPE_NOOP			8	/* No-op */
	/* Command TRBs */
#define TRB_TYPE_ENABLE_SLOT	9	/* Enable Slot Command */
#define TRB_TYPE_DISABLE_SLOT	10	/* Disable Slot Command */
#define TRB_TYPE_ADDRESS_DEVICE	11	/* Address Device Command */
#define TRB_TYPE_CONFIG_EP		12	/* Configure Endpoint Command */
#define TRB_TYPE_EVAL_CONTEXT	13	/* Evaluate Context Command */
#define TRB_TYPE_RESET_EP		14	/* Reset Endpoint Command */
#define TRB_TYPE_STOP_RING		15	/* Stop Endpoint Command */
#define TRB_TYPE_SET_TR_DEQ_PTR	16	/* Set TR Dequeue Pointer Command */
#define TRB_TYPE_RESET_DEV		17	/* Reset Device Command */
#define TRB_TYPE_FORCE_EVENT	18	/* Force Event Command  */
#define TRB_TYPE_NEGO_BANDWIDTH	19	/* Negotiate Bandwidth Command */
#define TRB_TYPE_SET_LATENCY	20	/* Set Latency Tolerance Value Command */
#define TRB_TYPE_GET_BANDWIDTH	21	/* Get port bandwidth Command */
#define TRB_TYPE_FORCE_HEADER	22	/* Force Header Command */
#define TRB_TYPE_CMD_NOOP		23	/* No op Command */
	/* TRB ID: 24~31 reserved */
	/* Event TRBS */
#define TRB_TYPE_TRANSFER		32	/* Transfer Event */
#define TRB_TYPE_COMPLETION_EVENT	33	/* Command Completion Event */
#define TRB_TYPE_PORT_CHANGE	34	/* Port Status Change Event */
#define TRB_TYPE_BANDWIDTH_EVENT	35	/* Bandwidth Request Event */
#define TRB_TYPE_DOORBELL_EVENT	36	/* Doorbell Event */
#define TRB_TYPE_HOST_EVENT		37	/* Host Controller Event */
#define TRB_TYPE_DEV_NOTIFICATE	38	/* Device Notification Event */
#define TRB_TYPE_MFINDEX_WRAP	39	/* MFINDEX Wrap Event */
	/* TRB ID: 40~47 reserved */
	/* TRB ID: 48~63 vendor defined */
/*-------------------------------------*/

/* ----- Used for Normal TRB ------ */
#define TRB_CYCLE		(1<<0)	/* Cycle bit */
#define TRB_ENT			(1<<1)	/* Evaltate Next TRB */
#define TRB_ISP			(1<<2)	/* Interrupt on short packet */
#define TRB_NO_SNOOP	(1<<3)
#define TRB_CHAIN		(1<<4)	/* Chain bit */
#define TRB_IOC			(1<<5)	/* Interrupt on completion */
#define TRB_IDT			(1<<6)	/* Immediate Data */
	/* Bit 7:8  reserved */
#define	TRB_BEI			(1<<9)	/* Block Event Interrupt */

/* Bit 0:16 TRB transfer length  */
#define	TRB_TRANSFER_LEN(x)	((x) & 0x1FFFF)
/* Bit 0:16 Interrupter Target  */
#define SET_TRB_INTR_TARGET(x)	(((x) & 0x3FF) << 22)
#define GET_TRB_INTR_TARGET(x)	(((x) >> 22) & 0x3FF)


/* ----- Used for Control Setup Stage TRB ------ */
#define	SET_TRB_TRANSFER_DIR(x)	((x) << 16)
#define	TRB_DATA_OUT		2
#define	TRB_DATA_IN			3
/* ----- Used for Status Stage TRB ------ */
#define TRB_DIR_IN		(1<<16)

/* ----- Used for Command TRB ------ */
#define GET_TRB_EP_INDEX(x)		((((x) & (0x1f << 16)) >> 16) - 1)
#define	SET_TRB_EP_ID(x)		((((x) + 1) & 0x1f) << 16)
/* ----- Used for Stop Endpoint Command TRB ------ */
/* Bit 23  Suspend  */
#define SET_TRB_SUSPEND(x)		(((x) & 1) << 23)
#define GET_TRB_SUSPEND(x)		(((x) & (1 << 23)) >> 23)

/* ----- Used for Command completion event TRB ------ */
struct xhci_event_cmd_completion {
	u64 cmd_trb_ptr;
	u32 status;
	u32 trb_info;
};

/* trb_info */
/* Bit 16:23  virtual function ID */
/* Bit 24:31  slot ID */
#define GET_TRB_SLOT_ID(x)	(((x) & (0xFF<<24)) >> 24)
#define SET_TRB_SLOT_ID(x)	(((x) & 0xFF) << 24)
/*-------------------------------------*/

/* ----- Used for link TRB ------ */
struct xhci_link_trb {
	u64 ring_segment_ptr;
	u32 interrupt_target;
	u32 trb_info;
};

/* trb_info */
#define LINK_TOGGLE	(0x1<<1)
/*-------------------------------------*/

union xhci_trb {
	struct xhci_generic_trb		generic;
	struct xhci_transfer_event	trans_event;
	struct xhci_link_trb		link;
	struct xhci_event_cmd_completion	event_cmd;
};

/* ----- Used for event ring segment table ------ */
struct xhci_event_ring_seg_table_entry {
	u64	ring_base_addr;
	u32	ring_seg_size;
	u32	reserved;
};

#endif //__MS_XHCI_CTX_H
