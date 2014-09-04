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

#define MS_USB_MAX_ALT		128	/* Hard limit */
//#define USB_MAXENDPOINTS		30	/* Hard limit */

#define MS_USB_MAX_CONFIG			1	/* Arbitrary limit */
U8 ConfigBuf[256]                __attribute__ ((aligned (128)));
U8 DevConfigBuf[XHCI_MAX_DEV][256]                __attribute__ ((aligned (128)));

extern int ms_usb_get_desc_u3(struct xhci_hcd *pXhci, struct usb_device *pDev, unsigned char type,
		       unsigned char index, void *pBuf, int size);
extern int usb_parse_config(struct usb_device *dev, unsigned char *buffer, int cfgno);

#if 1
static int ms_find_desc(unsigned char *pBuf, int size,
    int type1, int type2)
{
	struct usb_descriptor_header *h;
	unsigned char *buffer0 = pBuf;

	while (size > 0) {
		h = (struct usb_descriptor_header *) pBuf;


		if (h->bDescriptorType == type1 || h->bDescriptorType == type2)
		{
			ms_dbg("find desc type: %x\n", h->bDescriptorType);
			break;
		}

		ms_dbg("skip desc type: %x\n", h->bDescriptorType);
		pBuf += h->bLength;
		size -= h->bLength;
	}

	return pBuf - buffer0;
}

void ms_usb_parse_ss_ept_comp(struct usb_host_endpoint *pEpt,
		unsigned char *pBuf, int size)
{
	struct usb_ss_ep_comp_descriptor *pDesc;
	int max_tx;

	pDesc = (struct usb_ss_ep_comp_descriptor *) pBuf;
	if (pDesc->bDescriptorType != USB_DT_SS_ENDPOINT_COMP ||
			size < USB_DT_SS_EP_COMP_SIZE) {
		ms_err("%s: has no SS ept companion\n", __func__);

		pEpt->ss_ep_comp.bLength = USB_DT_SS_EP_COMP_SIZE;
		pEpt->ss_ep_comp.bDescriptorType = USB_DT_SS_ENDPOINT_COMP;
		if (is_usb_isoc_ept(&pEpt->desc) ||
				is_usb_int_ept(&pEpt->desc))
			pEpt->ss_ep_comp.wBytesPerInterval =
					pEpt->desc.wMaxPacketSize;
		return;
	}

	memcpy(&pEpt->ss_ep_comp, pDesc, USB_DT_SS_EP_COMP_SIZE);

	if (is_usb_control_ept(&pEpt->desc) && pDesc->bMaxBurst != 0) {
		ms_err( "%s: set control bMaxBurst from %d to 0\n", __func__, pDesc->bMaxBurst);
		pEpt->ss_ep_comp.bMaxBurst = 0;
	} else if (pDesc->bMaxBurst > 15) {
		ms_err( "%s: limit bMaxBurst from %d to 15\n", __func__, pDesc->bMaxBurst);
		pEpt->ss_ep_comp.bMaxBurst = 15;
	}

	if ((is_usb_control_ept(&pEpt->desc) ||
			is_usb_int_ept(&pEpt->desc)) &&
				pDesc->bmAttributes != 0) {
		ms_err("%s: set control ept bmAttributes from %d to 0\n",
				__func__, pDesc->bmAttributes);
		pEpt->ss_ep_comp.bmAttributes = 0;
	} else if (is_usb_bulk_ept(&pEpt->desc) &&
			pDesc->bmAttributes > 16) {
		ms_err( "%s: set bulk ept bmAttributes from %d to 16\n",
				__func__, pDesc->bmAttributes);
		pEpt->ss_ep_comp.bmAttributes = 16;
	}

	max_tx = 999999;

	if (pDesc->wBytesPerInterval > max_tx) {
		ms_err( "%s: set ept wBytesPerInterval from %d to %d\n",
					__func__, pDesc->wBytesPerInterval, max_tx);
		pEpt->ss_ep_comp.wBytesPerInterval = max_tx;
	}
}

static int ms_usb_parse_ept(struct usb_device *pDev,
	struct usb_host_interface *pIntf, int num_ep,
    unsigned char *pBuf, int size)
{
	unsigned char *pBuf_ori = pBuf;
	struct usb_endpoint_descriptor *pEpt_desc;
	struct usb_host_endpoint *pEpt;
	int n, i, j, retval;

	pEpt_desc = (struct usb_endpoint_descriptor *) pBuf;
	pBuf += pEpt_desc->bLength;
	size -= pEpt_desc->bLength;

	if (pEpt_desc->bLength >= USB_DT_ENDPOINT_AUDIO_SIZE)
		n = USB_DT_ENDPOINT_AUDIO_SIZE;
	else if (pEpt_desc->bLength >= USB_DT_ENDPOINT_SIZE)
		n = USB_DT_ENDPOINT_SIZE;
	else {
		ms_err( "%s: bad desc length %d \n", __func__, pEpt_desc->bLength);
		goto skip_to_next;
	}

	i = pEpt_desc->bEndpointAddress & ~USB_ENDPOINT_DIR_MASK;
	if (i >= 16 || i == 0) {
		ms_err( "%s: bad ept address %d\n", __func__, i);
		goto skip_to_next;
	}

	if (pIntf->desc.bNumEndpoints >= num_ep)
		goto skip_to_next;

	pEpt = &pIntf->endpoint[pIntf->desc.bNumEndpoints];
	++pIntf->desc.bNumEndpoints;

	memcpy(&pEpt->desc, pEpt_desc, n);
	INIT_LIST_HEAD(&pEpt->urb_list);

	i = 0;
	j = 255;

	if (pDev->speed == USB_SPEED_HIGH
			&& is_usb_bulk_ept(pEpt_desc)) {
		unsigned maxp;

		maxp = le16_to_cpu(pEpt->desc.wMaxPacketSize) & 0x07ff;
		if (maxp != 512)
			ms_err("%s: bulk ept wMaxPacketSize != 512\n", __func__);
	}

	if (pDev->speed == USB_SPEED_SUPER)
		ms_usb_parse_ss_ept_comp(pEpt, pBuf, size);

	pEpt->extra = pBuf;
	i = ms_find_desc(pBuf, size, USB_DT_ENDPOINT,
			USB_DT_INTERFACE);
	pEpt->extralen = i;
	retval = pBuf - pBuf_ori + i;

	return retval;

skip_to_next:
	i = ms_find_desc(pBuf, size, USB_DT_ENDPOINT,
	    USB_DT_INTERFACE);
	return pBuf - pBuf_ori + i;
}

 int ms_usb_parse_intf(struct usb_device *pDev,
    struct usb_host_config *pConfig, unsigned char *pBuf, int size,
    u8 pIntfs[], u8 pAlts[])
{
	unsigned char *pBuf_ori = pBuf;
	struct usb_interface_descriptor	*pIntf_desc;
	int inum, asnum;
	struct usb_interface_cache *pIntf_cache;
	struct usb_host_interface *pAlt;
	int i, n;
	int retval;
	int ept_count, ept_count_ori;

	pIntf_desc = (struct usb_interface_descriptor *) pBuf;
	pBuf += pIntf_desc->bLength;
	size -= pIntf_desc->bLength;

	if (pIntf_desc->bLength < USB_DT_INTERFACE_SIZE)
		goto skip_to_next;

	pIntf_cache = NULL;
	inum = pIntf_desc->bInterfaceNumber;
	for (i = 0; i < pConfig->desc.bNumInterfaces; ++i) {
		if (pIntfs[i] == inum) {
			pIntf_cache = pConfig->intf_cache[i];
			break;
		}
	}
	if (!pIntf_cache || pIntf_cache->num_altsetting >= pAlts[i])
		goto skip_to_next;

	asnum = pIntf_desc->bAlternateSetting;
	for ((i = 0, pAlt = &pIntf_cache->altsetting[0]);
	      i < pIntf_cache->num_altsetting;
	     (++i, ++pAlt)) {
		if (pAlt->desc.bAlternateSetting == asnum) {
			ms_err( "%s: desc already exist\n", __func__);
			goto skip_to_next;
		}
	}

	++pIntf_cache->num_altsetting;
	memcpy(&pAlt->desc, pIntf_desc, USB_DT_INTERFACE_SIZE);

	pAlt->extra = pBuf;
	i = ms_find_desc(pBuf, size, USB_DT_ENDPOINT,
	    USB_DT_INTERFACE);
	pAlt->extralen = i;
	pBuf += i;
	size -= i;

	ept_count = ept_count_ori = pAlt->desc.bNumEndpoints;
	pAlt->desc.bNumEndpoints = 0;

	n = 0;
	while (size > 0) {
		if (((struct usb_descriptor_header *) pBuf)->bDescriptorType
		     == USB_DT_INTERFACE)
			break;
		retval = ms_usb_parse_ept(pDev, pAlt, ept_count, pBuf, size);
		if (retval < 0)
			return retval;
		++n;

		pBuf += retval;
		size -= retval;
	}

	if (n != ept_count_ori)
		ms_err("%s: retrieve ept desc num (%d) is not equal to interface desc (%d)\n",
		    __func__, n,  ept_count_ori);
	return pBuf - pBuf_ori;

skip_to_next:
	i = ms_find_desc(pBuf, size, USB_DT_INTERFACE,
	    USB_DT_INTERFACE);
	return pBuf - pBuf_ori + i;
}

 int ms_usb_parse_config(struct usb_device *pDev,
    struct usb_host_config *pConfig, unsigned char *pBuf, int size)
{
	//struct device *ddev = &pDev->dev;
	unsigned char *pBuf_ori = pBuf;
	int Intf_count, Intf_count_ori;
	int i, j, n;
	struct usb_interface_cache *pIntf_cache;
	unsigned char *pBuf_tmp;
	int size2;
	struct usb_descriptor_header *pDesc_header;
	int len, retval;
	u8 Intfs[MS_USB_MAX_INTERFACES], Alts[MS_USB_MAX_INTERFACES];
	unsigned assoc_count = 0;

	memcpy(&pConfig->desc, pBuf, USB_DT_CONFIG_SIZE);
	if (pConfig->desc.bDescriptorType != USB_DT_CONFIG ||
	    pConfig->desc.bLength < USB_DT_CONFIG_SIZE) {
		ms_err("%s: bad desc type = %d, lentgh = %d\n",
			__func__, pConfig->desc.bDescriptorType, pConfig->desc.bLength);
		return -EINVAL;
	}

	pBuf += pConfig->desc.bLength;
	size -= pConfig->desc.bLength;

	Intf_count = Intf_count_ori = pConfig->desc.bNumInterfaces;
	if (Intf_count > MS_USB_MAX_INTERFACES) {
		ms_err("%s: bad interfaces number %d\n", __func__, Intf_count);
		Intf_count = MS_USB_MAX_INTERFACES;
	}

	n = 0;
	for ((pBuf_tmp = pBuf, size2 = size);
	      size2 > 0;
	     (pBuf_tmp += pDesc_header->bLength, size2 -= pDesc_header->bLength)) {

		if (size2 < sizeof(struct usb_descriptor_header)) {
			ms_err("%s: invalid desc buffer length %d\n", __func__, size2);
			break;
		}

		pDesc_header = (struct usb_descriptor_header *) pBuf_tmp;
		if ((pDesc_header->bLength > size2) || (pDesc_header->bLength < 2)) {
			ms_err( "%s: invalid desc length %d\n", __func__, pDesc_header->bLength);
			break;
		}

		if (pDesc_header->bDescriptorType == USB_DT_INTERFACE) {
			struct usb_interface_descriptor *d;
			int inum;

			d = (struct usb_interface_descriptor *) pDesc_header;
			if (d->bLength < USB_DT_INTERFACE_SIZE) {
				ms_err( "%s: invalid interface desc len %d\n", __func__, d->bLength);
				continue;
			}

			inum = d->bInterfaceNumber;

			for (i = 0; i < n; ++i) {
				if (Intfs[i] == inum)
					break;
			}
			if (i < n) {
				if (Alts[i] < 255)
					++Alts[i];
			} else if (n < MS_USB_MAX_INTERFACES) {
				Intfs[n] = inum;
				Alts[n] = 1;
				++n;
			}

		}
		else if (pDesc_header->bDescriptorType ==
				USB_DT_INTERFACE_ASSOCIATION) {
			if (assoc_count == USB_MAX_INTF_ASSOCS) {
				ms_err("%s: assoc_count == USB_MAX_INTF_ASSOCS\n", __func__);
			} else {
				pConfig->intf_assoc[assoc_count] =
					(struct usb_interface_assoc_descriptor
					*)pDesc_header;
				assoc_count++;
			}

		}

	}

	size = pBuf_tmp - pBuf;
	pConfig->desc.wTotalLength = cpu_to_le16(pBuf_tmp - pBuf_ori);

	pConfig->desc.bNumInterfaces = Intf_count = n;

	for (i = 0; i < Intf_count; ++i) {
		for (j = 0; j < Intf_count; ++j) {
			if (Intfs[j] == i)
				break;
		}
		if (j >= Intf_count)
			ms_err( "%s: can't find interface number %d\n", __func__, i);
	}

	for (i = 0; i < Intf_count; ++i) {
		j = Alts[i];
		if (j > MS_USB_MAX_ALT) {
			ms_err( "%s: too many alt settings %d\n", __func__, j);
			Alts[i] = j = MS_USB_MAX_ALT;
		}

		len = sizeof(*pIntf_cache) + sizeof(struct usb_host_interface) * j;
		pConfig->intf_cache[i] = pIntf_cache = kzalloc(len, GFP_KERNEL);
		//printf("................................alloc intf_cache[%d]: %x\n", i, pConfig->intf_cache[i]);
		if (!pIntf_cache)
			return -ENOMEM;
		pIntf_cache->ref=1;
	}

	i = ms_find_desc(pBuf, size, USB_DT_INTERFACE,
	    USB_DT_INTERFACE);

	pBuf += i;
	size -= i;

	while (size > 0) {
		retval = ms_usb_parse_intf(pDev, pConfig,
		    pBuf, size, Intfs, Alts);
		if (retval < 0)
			return retval;

		pBuf += retval;
		size -= retval;
	}

	return 0;
}

void ms_usb_free_config(struct usb_device *dev)
{
	int ii, jj;

	if (!dev->pconfig)
		return;

	for (ii = 0; ii < dev->descriptor.bNumConfigurations; ii++) {
		struct usb_host_config *cf = &dev->pconfig[ii];

		for (jj = 0; jj < cf->desc.bNumInterfaces; jj++) {
			if (cf->intf_cache[jj])
			{
				//printf("................................free intf_cache[%d]: %x\n", jj, cf->intf_cache[jj]);
				free(cf->intf_cache[jj]);
			}
		}
	}
	dev->pconfig = NULL;
}
#endif

struct usb_host_interface *ms_usb_find_alts(
		struct usb_host_config *pConfig,
		unsigned int intf_num,
		unsigned int alt_num)
{
	struct usb_interface_cache *pIntf_cache = NULL;
	int i, j;

	for (i = 0; i < pConfig->desc.bNumInterfaces; i++)
	{
		if (pConfig->intf_cache[i]->altsetting[0].desc.bInterfaceNumber	== intf_num)
		{
			pIntf_cache = pConfig->intf_cache[i];
			if (!pIntf_cache)
				return NULL;

			for (j = 0; j < pIntf_cache->num_altsetting; j++)
				if (pIntf_cache->altsetting[j].desc.bAlternateSetting == alt_num)
					return &pIntf_cache->altsetting[j];

			break;
		}
	}

	ms_dbg("%s: can't find alt setting\n");
	return NULL;
}

int ms_usb_get_config(struct xhci_hcd *pXhci, struct usb_device *pDev)
{
	int result = 0;
	unsigned int /*cfg_num,*/ len;
	unsigned char *pBuf_tmp;
	struct ms_usb_config_desc *pConfig_desc;
	struct usb_interface *pIntf;
	int i, j;

	if (pDev->descriptor.bNumConfigurations > MS_USB_MAX_CONFIG)
	{
		ms_err("%s: too many configs %d\n", __func__, pDev->descriptor.bNumConfigurations);
		pDev->descriptor.bNumConfigurations = MS_USB_MAX_CONFIG;
	}


	pDev->pconfig =(struct usb_host_config *)DevConfigBuf[pDev->devnum-1];

	pConfig_desc = (struct ms_usb_config_desc *)ConfigBuf;

	result = 0;
	{
		result = ms_usb_get_desc_u3(pXhci, pDev, USB_DT_CONFIG, 0,
		    pConfig_desc, USB_DT_CONFIG_SIZE);
		if (result < 0) {
			ms_err("%s: can't read config 0 (err=%d)\n", __func__, result);
			result = -EINVAL;
			goto err;
		} else if (result < 4) {
			ms_err("%s: invalid length %d for config 0\n", __func__, result);
			result = -EINVAL;
			goto err;
		}
		len = max((int) pConfig_desc->wTotalLength, USB_DT_CONFIG_SIZE);

		pBuf_tmp =(unsigned char *)ConfigBuf;

		//printf("config len: %d\n", length);
		result = ms_usb_get_desc_u3(pXhci, pDev, USB_DT_CONFIG, 0,  pBuf_tmp, len);
		if (result < 0) {
			ms_err("%s: can't read full config 0 (err=%d)\n", __func__, result);
			goto err;
		}
		if (result < len) {
			ms_err("%s: invalid config length returned %d (expect to %d)\n",
				__func__, result, len);
			len = result;
		}

		 usb_parse_config(pDev, pBuf_tmp, 0);

		for (i=0, j=0; i<pDev->config.if_desc[0].no_of_ep; i++)
		{
			if (is_usb_bulk_ept(&pDev->config.if_desc[0].ep_desc[i]))
			{
				if (j==0)
				{
					memcpy(&pDev->bulk1.desc, &pDev->config.if_desc[0].ep_desc[0],7);
					ms_usb_enable_ept(pDev, &pDev->bulk1);
					j++;
				}
				else
				{
					memcpy(&pDev->bulk2.desc, &pDev->config.if_desc[0].ep_desc[1],7);
					ms_usb_enable_ept(pDev, &pDev->bulk2);
				}
			}
		}

		pIntf = &pDev->config.if_desc[0];					//yuwen
		result = ms_usb_parse_config(pDev, &pDev->pconfig[0], pBuf_tmp, len);
		if (result < 0) {
			goto err;
		}
	}
	result = 0;

err:
	//pDev->descriptor.bNumConfigurations = cfg_num;

	if (result == -ENOMEM)
		ms_err("%s: out of memory\n", __func__);
	return result;
}

