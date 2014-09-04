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

#include <linux/ctype.h>
#include "xhci.h"
#include "../usb/ehci_usb.h"
#include "usbdesc.h"
#include <asm/unaligned.h>

//#define MS_DBG
#ifdef MS_DBG
#define ms_dbg(fmt, args...)   do { printf(fmt , ## args); } while (0)
#else
#define ms_dbg(fmt, args...)   do { } while (0)
#endif

#define ms_err(fmt, args...)   do { printf(fmt , ## args); } while (0)

u8 DescBuf[256]                __attribute__ ((aligned (128)));

 int usb_get_string_u3(struct xhci_hcd *xhci, struct usb_device *dev, unsigned short langid,
			  unsigned char index, void *buf, int size);
struct usb_host_interface *ms_usb_altno_to_alt(
					const struct usb_interface *pIntf,
					unsigned int altno);
extern int  ms_wait_for_completion_timeout(struct xhci_hcd *pXhci,
		int *done ,  unsigned long timeout);
extern struct urb *ms_usb_alloc_urb(gfp_t mem_flags);
void ms_usb_fill_control_urb(struct urb *pUrb,
					struct usb_device *pDev,
					unsigned int pipe,
					unsigned char *pSetup_packet,
					void *pBuf,
					int len);
void ms_usb_fill_bulk_urb(struct urb *pUrb,
				     struct usb_device *pDev,
				     unsigned int pipe,
				     void *pBuf,
				     int len);

void ms_usb_kill_urb(struct urb *pUrb);
int ms_usb_submit_urb( struct urb *pUrb, gfp_t mem_flags);
int ms_usb_get_dev_desc(struct xhci_hcd *pXhci, struct usb_device *pDev, unsigned int len);
extern int ms_xhci_alloc_bandwidth(struct xhci_hcd *pXhci,struct usb_device *pUdev,
		struct usb_host_config *pConfig,
		struct usb_host_interface *pCur_alt,
		struct usb_host_interface *pNew_alt);
extern void usb_free_urb(struct urb *urb);
extern void ms_xhci_reset_ept(struct usb_host_endpoint *ep);
extern int ms_xhci_urb_enqueue( struct urb *pUrb, gfp_t mem_flags);
extern int ms_xhci_urb_dequeue(struct urb *pUrb, int status);

extern unsigned long  gUsbStatusXHC;


#define to_urb(d) container_of(d, struct urb, kref)

#ifndef msleep
#define msleep(x) mdelay(x)
#endif

#if XHCI_FLUSHPIPE_PATCH
extern void Chip_Flush_Memory(void);
#endif

#define SET_ADDRESS_TRIES	3
extern int ms_xhci_address_dev(struct xhci_hcd *pXhci, struct usb_device *pUdev);
//extern int ms_usb_get_dev_desc(struct xhci_hcd *pXhci, struct usb_device *pDev, unsigned int len);
extern int ms_usb_get_config(struct xhci_hcd *pXhci, struct usb_device *pDev);
//extern int ms_usb_set_config_u3(struct xhci_hcd *pXhci,struct usb_device *pDev, int config_num);
extern int ms_xhci_reset_dev(struct xhci_hcd *xhci, struct usb_device *udev);
extern int hub_port_reset(struct usb_device *dev, int port, unsigned short *portstat);

#define	MS_XHCI_PORT_REG_RO	((1<<0) | (1<<3) | (0xf<<10) | (1<<30))
#define MS_XHCI_PORT_REG_RWS	((0xf<<5) | (1<<9) | (0x3<<14) | (0x7<<25))


static int ms_usb_send_urb(struct xhci_hcd *xhci,
			struct urb *urb, int timeout, int *actual_length)
{
	int retval;
	struct api_context ctx;

	init_completion(ctx.done);
	urb->context = &ctx;
	urb->actual_length = 0;
	retval = ms_usb_submit_urb(urb, 0);
	if (unlikely(retval))
		goto out;

	retval=ms_wait_for_completion_timeout(xhci, &(ctx.done), timeout);
	if (retval<0)
	{
		xhci->kill_urb_done = 0;
		ms_usb_kill_urb(urb);
		retval=ms_wait_for_completion_timeout(xhci, &(xhci->kill_urb_done), timeout);
		retval = -ETIMEDOUT;
		gUsbStatusXHC |= USB_ST_TIMEOUT;

		ms_err("%s: urb timed out on len=%u/%u\n", __func__,
		urb->actual_length,
		urb->transfer_buffer_length);
	} else
		retval = ctx.status;

out:
	if (actual_length)
		*actual_length = urb->actual_length;

	kfree(urb);

	return retval;
}

static int ms_usb_control_msg(struct xhci_hcd *pXhci,
		struct usb_device *pDev,
		unsigned int pipe,
		struct usb_ctrlrequest *pCmd,
		void *pBuf,
		int len,
		int timeout)
{
	struct urb *urb;
	int retv;
	int length;

	urb = ms_usb_alloc_urb(0);
	if (!urb)
	{
		ms_err("%s: alloc urb fail\n", __func__);
		return -ENOMEM;
	}

	ms_usb_fill_control_urb(urb, pDev, pipe, (unsigned char *)pCmd, pBuf, len);

	retv = ms_usb_send_urb(pXhci, urb, timeout, &length);

	if (retv < 0)
	{
		ms_err("%s: fail to send control urb (err=%d)\n", __func__, retv);
		return retv;
	}
	else
		return length;
}

int usb_control_msg_u3(struct xhci_hcd *pXhci, struct usb_device *pDev, unsigned int pipe, __u8 u8Req,
		    __u8 u8Req_type, __u16 u16Val, __u16 u16Index, void *pBuf, __u16 u16Size, int timeout)
{
	struct usb_ctrlrequest *pReq;
	int ret;

	pReq = kmalloc(sizeof(struct usb_ctrlrequest), 0);
	if (!pReq)
		return -ENOMEM;

	pReq->bRequestType = u8Req_type;
	pReq->bRequest = u8Req;
	pReq->wValue = u16Val;
	pReq->wIndex = u16Index;
	pReq->wLength = u16Size;

	ret = ms_usb_control_msg(pXhci, pDev, pipe, pReq, pBuf, u16Size, timeout);

	kfree(pReq);

	return ret;
}

void ms_usb_enable_ept(struct usb_device *pDev, struct usb_host_endpoint *pEpt)
{
	int epnum = usb_ept_num(&pEpt->desc);
	int is_out = is_usb_ept_dir_out(&pEpt->desc);
	int is_control = is_usb_control_ept(&pEpt->desc);

	if (is_control)
	{
		pDev->ep_out[epnum] = pEpt;
		pDev->ep_in[epnum] = pEpt;
	}
	else
	{
		if (is_out)
			pDev->ep_out[epnum] = pEpt;
		else
			pDev->ep_in[epnum] = pEpt;
	}

	pEpt->enabled = 1;
	pEpt->hcpriv = pDev;
}


extern struct xhci_hcd     ms_xhci;

int ms_usb_bulk_msg_u3(struct usb_device *pDev, unsigned int pipe,
		 void *pBuf, int len, int *pAct_len, int timeout)
{
	struct urb *urb;

	urb = ms_usb_alloc_urb(0);
	if (!urb)
		return -ENOMEM;

	ms_usb_fill_bulk_urb(urb, pDev, pipe, pBuf, len);

	return ms_usb_send_urb(&ms_xhci, urb, timeout, pAct_len);
}

int ms_usb_get_desc_u3(struct xhci_hcd *pXhci, struct usb_device *pDev, unsigned char type,
		       unsigned char index, void *pBuf, int size)
{
	int i;
	int retval;

	memset(pBuf, 0, size);

	for (i = 0; i < 3; ++i)
	{
		retval = usb_control_msg_u3(pXhci,pDev, usb_rcvctrlpipe(pDev, 0),
				USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
				(type << 8) + index, 0, pBuf, size,
				MS_CTRL_GET_TIMEOUT);
		if (retval <= 0 && retval != -ETIMEDOUT)
			continue;

		if (retval > 1 && ((u8 *)pBuf)[1] != type)
		{
			retval = -ENODATA;
			continue;
		}
		break;
	}

	return retval;
}

int ms_usb_get_dev_desc(struct xhci_hcd *pXhci, struct usb_device *pDev, unsigned int len)
{
	struct usb_device_descriptor *pDesc;
	int retval;

	if (len > sizeof(*pDesc))
	{
		ms_err("%s: bad len for dev desc\n", __func__);
		return -EINVAL;
	}

	pDesc =(struct usb_device_descriptor *)DescBuf;

	retval = ms_usb_get_desc_u3(pXhci, pDev, USB_DT_DEVICE, 0, pDesc, len);
	if (retval >= 0)
		memcpy(&pDev->descriptor, pDesc, len);

	return retval;
}

void ms_usb_reset_ept(struct usb_device *pDev, unsigned int ept_addr)
{
	unsigned int epnum = ept_addr & USB_ENDPOINT_NUMBER_MASK;
	struct usb_host_endpoint *pEpt;

	if (usb_endpoint_out(ept_addr))
		pEpt = pDev->ep_out[epnum];
	else
		pEpt = pDev->ep_in[epnum];

	if (pEpt)
		ms_xhci_reset_ept(pEpt);
}

extern struct xhci_hcd     ms_xhci;

int ms_usb_ctrl_transfer_u3(struct usb_device *pDev, struct devrequest *pReq,  int len,void *pBuf)
{
	int retval;

	retval= usb_control_msg_u3(&ms_xhci,pDev,usb_sndctrlpipe(pDev, 0),pReq->request,
	 	pReq->requesttype,pReq->value,pReq->index, pBuf, len, MS_CTRL_SET_TIMEOUT);

	return retval;
}

int ms_usb_set_config_u3(struct xhci_hcd *pXhci,struct usb_device *pDev, int config_num)
{
	int res, retval, i;
	struct usb_host_config *pConfig = NULL;

	ms_dbg("set config %d\n", config_num);
	/* set setup command */
	for (i = 0; i < pDev->descriptor.bNumConfigurations; i++)
	{
		if (pDev->pconfig[i].desc.bConfigurationValue == config_num)
		{
			pConfig = &pDev->pconfig[i];
			break;

		}
	}

	if ((!pConfig && config_num != 0))
	{
		ms_err("%s: can't find config %d\n", __func__, config_num);
		return -EINVAL;
	}

	retval = ms_xhci_alloc_bandwidth(pXhci, pDev,pConfig, NULL, NULL);
	if (retval < 0) {
		ms_err("%s: allocate bandwidth fail\n", __func__);
		return -1;
	}

	res = usb_control_msg_u3(pXhci,pDev, usb_sndctrlpipe(pDev, 0),
			      USB_REQ_SET_CONFIGURATION, 0, config_num, 0,
			      NULL, 0, MS_CTRL_SET_TIMEOUT);

	if (res == 0)
		return 0;
	else
		return -1;
}

struct usb_host_interface *ms_usb_altno_to_alt(
					const struct usb_interface *pIntf,
					unsigned int altno)
{
	int i;

	for (i = 0; i < pIntf->num_altsetting; i++) {
		if (pIntf->altsetting[i].desc.bAlternateSetting == altno)
			return &pIntf->altsetting[i];
	}
	return NULL;
}

struct urb *ms_usb_alloc_urb(gfp_t mem_flags)
{
	struct urb *urb;

	urb = kmalloc(sizeof(struct urb),
	//	iso_packets * sizeof(struct usb_iso_packet_descriptor),
		mem_flags);
	if (!urb) {
		ms_err("%s: fail to alloc urb\n", __func__);
		return NULL;
	}

	memset(urb, 0, sizeof(*urb));
	return urb;
}

void ms_usb_fill_control_urb(struct urb *pUrb,
					struct usb_device *pDev,
					unsigned int pipe,
					unsigned char *pSetup_packet,
					void *pBuf,
					int len)
{
	pUrb->dev = pDev;
	pUrb->pipe = pipe;
	pUrb->setup_packet = pSetup_packet;
	pUrb->transfer_buffer = pBuf;
	pUrb->transfer_buffer_length = len;
	pUrb->complete = NULL;
	pUrb->context = NULL;
}

void ms_usb_fill_bulk_urb(struct urb *pUrb,
				     struct usb_device *pDev,
				     unsigned int pipe,
				     void *pBuf,
				     int len)
{
	pUrb->dev = pDev;
	pUrb->pipe = pipe;
	pUrb->transfer_buffer = pBuf;
	pUrb->transfer_buffer_length = len;
	pUrb->complete = NULL;
	pUrb->context = NULL;
}


int ms_usb_submit_urb( struct urb *pUrb, gfp_t mem_flags)
{
	int		xfertype, max;
	struct usb_device		*pDev;
	struct usb_host_endpoint	*ep;
	int		is_out;
	int		retval;

	pDev = pUrb->dev;

	ep = ms_usb_pipe_to_ept(pDev, pUrb->pipe);
	//printf("ep:%x\n",ep);
	if (!ep)
		return -ENOENT;

	pUrb->ep = ep;
	pUrb->status = -EINPROGRESS;
	pUrb->actual_length = 0;

	xfertype = usb_ept_type(&ep->desc);
	if (xfertype == USB_ENDPOINT_XFER_CONTROL) {
		struct usb_ctrlrequest *setup =
				(struct usb_ctrlrequest *) pUrb->setup_packet;

		if (!setup)
			return -ENOEXEC;
		is_out = !(setup->bRequestType & USB_DIR_IN) ||
				!setup->wLength;
	} else {
		is_out = is_usb_ept_dir_out(&ep->desc);
	}

	pUrb->transfer_flags &= ~(URB_DIR_MASK | URB_DMA_MAP_SINGLE |
			URB_DMA_MAP_PAGE | URB_DMA_MAP_SG | URB_MAP_LOCAL |
			URB_SETUP_MAP_SINGLE | URB_SETUP_MAP_LOCAL |
			URB_DMA_SG_COMBINED);
	pUrb->transfer_flags |= (is_out ? URB_DIR_OUT : URB_DIR_IN);

	max = le16_to_cpu(ep->desc.wMaxPacketSize);
	if (max <= 0) {
		ms_err("bogus endpoint ep%d%s in %s (bad maxpacket %d)\n",
			usb_ept_num(&ep->desc), is_out ? "out" : "in",
			__func__, max);
		return -EMSGSIZE;
	}

#ifdef DEBUG
	{
	unsigned int	orig_flags = pUrb->transfer_flags;
	unsigned int	allowed;
	static int pipetypes[4] = {
		PIPE_CONTROL, PIPE_ISOCHRONOUS, PIPE_BULK, PIPE_INTERRUPT
	};

	/* Check that the pipe's type matches the endpoint's type */
	if (usb_pipetype(pUrb->pipe) != pipetypes[xfertype]) {
		ms_err( "BOGUS urb xfer, pipe %x != type %x\n",
			usb_pipetype(pUrb->pipe), pipetypes[xfertype]);
		return -EPIPE;		/* The most suitable error code :-) */
	}

	/* enforce simple/standard policy */
	allowed = (URB_NO_TRANSFER_DMA_MAP | URB_NO_INTERRUPT | URB_DIR_MASK |
			URB_FREE_BUFFER);
	switch (xfertype) {
	case USB_ENDPOINT_XFER_BULK:
		if (is_out)
			allowed |= URB_ZERO_PACKET;
		/* FALLTHROUGH */
	case USB_ENDPOINT_XFER_CONTROL:
		allowed |= URB_NO_FSBR;	/* only affects UHCI */
		/* FALLTHROUGH */
	default:			/* all non-iso endpoints */
		if (!is_out)
			allowed |= URB_SHORT_NOT_OK;
		break;
	case USB_ENDPOINT_XFER_ISOC:
		allowed |= URB_ISO_ASAP;
		break;
	}
	pUrb->transfer_flags &= allowed;

	/* fail if submitter gave bogus flags */
	if (pUrb->transfer_flags != orig_flags) {
		ms_err("BOGUS urb flags, %x --> %x\n",
			orig_flags, pUrb->transfer_flags);
		return -EINVAL;
	}
	}
#endif

	flush_cache((unsigned long)pUrb->transfer_buffer, pUrb->transfer_buffer_length);
	pUrb->transfer_dma = VA2PA((U32)pUrb->transfer_buffer);

	retval = ms_xhci_urb_enqueue( pUrb, mem_flags);

	return retval;
}

void ms_usb_kill_urb(struct urb *pUrb)
{
	int	retval = -EIDRM;

	if (!(pUrb && pUrb->dev && pUrb->ep))
		return;

	retval = ms_xhci_urb_dequeue(pUrb, -ENOENT);

	if (retval == 0)
		retval = -EINPROGRESS;
	else if (retval != -EIDRM && retval != -EBUSY)
		ms_err("%s: faill to kill urb %p (err=%d)\n", __func__, pUrb, retval);
}

u32 ms_xhci_port_reg_mask(u32 state)
{
	return (state & MS_XHCI_PORT_REG_RO) | (state & MS_XHCI_PORT_REG_RWS);
}

int ms_xhci_port_connect(struct xhci_hcd *pXhci)
{
	u32 temp;
	u32 mask;
	int i;
	int ports;
	u32 __iomem *addr;
	u32	hcs_params1;

	hcs_params1 = xhci_readl(pXhci, &pXhci->cap_regs->u32HcsParams1);
	ports = HCS_MAX_NUM_PORTS(hcs_params1);
	mask = PORTSC_CCS;

	/* For each port, did anything change?  If so, set that bit in buf. */
	for (i = 0; i < ports; i++) {
		addr = &pXhci->op_regs->u32PortSc +
			PORT_REGS_NUM*i;
		temp = xhci_readl(pXhci, addr);
		if ((temp & mask) != 0 ) {
			return (i+1);
		}
	}

	return 0;
}

int ms_xhci_port_reset(struct xhci_hcd *xhci, int port,
				struct usb_device *udev)
{

	int status, delay_time;
	u32 __iomem *addr;
	u32 temp;

	addr = &xhci->op_regs->u32PortSc + PORT_REGS_NUM*port;

	temp = xhci_readl(xhci, addr);
	ms_dbg("port status:%x\n", temp);
	if (!(temp & PORTSC_CCS))
		return -ENOTCONN;

	temp = xhci_readl(xhci, addr);
	temp = ms_xhci_port_reg_mask(temp);
	temp = (temp | PORTSC_RST);
	xhci_writel(xhci, temp, addr);

	//wait reset done
	for (delay_time = 0; delay_time < 500; delay_time += 50)
	{
		msleep(50);

		temp = xhci_readl(xhci, addr);
		ms_dbg("temp:%x\n",temp);

		if (!(temp & PORTSC_RST) &&
		    (temp & PORTSC_PED)) {
			if (DEV_IS_SUPERSPEED(temp))
			{
				printf("USB super speed device \n");
				udev->speed = USB_SPEED_SUPER;
			}
			else if (DEV_IS_HIGHSPEED(temp))
			{
				printf("USB high speed device \n");
				udev->speed = USB_SPEED_HIGH;
			}
			else if (DEV_IS_FULLSPEED(temp))
			{
				printf("USB full speed device \n");
				udev->speed = USB_SPEED_FULL;
			}
			else if (DEV_IS_LOWSPEED(temp))
			{
				printf("USB low speed device \n");
				udev->speed = USB_SPEED_LOW;
			}
			else
			{
				printf("USB unknow speed !!\n");
				return -ENODEV;
			}
			break;
		}

	}

	msleep(10 + 40);
	udev->devnum = 0;

	status = ms_xhci_reset_dev(xhci, udev);

 	if (status < 0)
	{
	 	ms_err("%s:Cannot reset HCD device state\n", __func__);
 	}

	return status;
}

//#define SET_ADDRESS_TRIES	2
#define GET_DESCRIPTOR_TRIES	2
int ms_xhci_dev_enum(struct xhci_hcd *xhci, struct usb_device *dev, int rh_port)
{
	int retval, i,j;
	int port = -1;
	u32 __iomem *addr;
	u32 temp;
	int			devnum = dev->devnum;
       int ret;
	struct usb_device *parent = dev->parent;
	unsigned short portstatus;

	ms_dbg("enum on port: %d\n", rh_port);

	retval = ms_xhci_alloc_dev(xhci, dev);
	if (retval != 1)
		return (-1);

	if (!dev->parent)
	{
		addr = &xhci->op_regs->u32PortSc + PORT_REGS_NUM*rh_port;

		temp = xhci_readl(xhci, addr);

		if ((temp & PORTSC_CCS))
		{
			dev->portnum=rh_port;
			ms_xhci_port_reset(xhci, rh_port,dev);
		}
	}
	else
	{
		int j;

		for (j = 0; j < parent->maxchild; j++) {
			if (parent->children[j] == dev) {
				port = j;
				break;
			}
		}
		if (port < 0) {
			ms_err("%s: cannot get device's port.\n", __func__);
			return -1;
		}

		if (hub_port_reset(dev->parent, port, &portstatus) < 0) {
			ms_err("%s: fail to reset port %i\n", __func__, port);
			return -1;
		}

		dev->route = parent->route + (rh_port << ((parent->level - 1)*4));
	}

	ms_dbg("usb speed: %x\n", dev->speed);
      switch (dev->speed)
	{
	case USB_SPEED_SUPER:
		dev->ep0.desc.wMaxPacketSize = 512;
		break;
	case USB_SPEED_HIGH:		/* fixed at 64 */
		dev->ep0.desc.wMaxPacketSize = 64;
		break;
	case USB_SPEED_FULL:		/* 8, 16, 32, or 64 */
		dev->ep0.desc.wMaxPacketSize = 64;
		break;
	case USB_SPEED_LOW:		/* fixed at 8 */
		dev->ep0.desc.wMaxPacketSize = 8;
		break;
	default:
		goto fail;
	}

	for (j = 0; j < SET_ADDRESS_TRIES; j++)
	{
		retval = ms_xhci_address_dev(xhci, dev);
		if (retval >= 0)	break;

		msleep(200);

		if (retval < 0)
		{
			ms_err("%s: fail to address device at %d (err= %d)\n", __func__, devnum, retval);
			return retval;
		}
	}

	for (i = 0; i < GET_DESCRIPTOR_TRIES; (++i, msleep(100)))
	   {

		retval = ms_usb_get_dev_desc(xhci, dev, 8);
		if (retval < 8)
		{
			ms_err("%s: fail to get dev desc (err= %d)\n",	__func__, retval);
			if (retval >= 0)
			{
				retval = -EMSGSIZE;
				return retval;
			}

		}
		else
		{
			retval = 0;
			break;
		}
	}

	if (dev->descriptor.bMaxPacketSize0 == 0xff ||
			dev->speed == USB_SPEED_SUPER)
		i = 512;
	else
		i = dev->descriptor.bMaxPacketSize0;
	if (le16_to_cpu(dev->ep0.desc.wMaxPacketSize) != i) {
		if (dev->speed == USB_SPEED_LOW ||
				!(i == 8 || i == 16 || i == 32 || i == 64)) {
			ms_err("%s: bad ep0 maxpacket: %d\n", __func__, i);
			retval = -EMSGSIZE;
			return retval;
		}

		dev->ep0.desc.wMaxPacketSize = i;
		//usb_ep0_reinit(udev);
	}

	retval = ms_usb_get_dev_desc(xhci, dev, USB_DT_DEVICE_SIZE);
	if (retval < (signed)sizeof(dev->descriptor)) {
		ms_err("%s: fail to get dev desc (err = %d)\n", __func__, retval);

		return retval;
	}

	ret = ms_usb_get_config(xhci, dev);

      ms_usb_set_config_u3(xhci,dev,dev->config.desc.bConfigurationValue);

  return 0;
fail:
	return retval;
}


