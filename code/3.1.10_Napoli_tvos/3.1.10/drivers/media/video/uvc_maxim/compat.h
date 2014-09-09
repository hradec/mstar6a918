#ifndef _COMPAT_H
#define _COMPAT_H

#include <linux/version.h>

/* <linux/usb.h> fixups */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34)
static inline int usb_enable_autosuspend(void *udev)
{ return 0; }
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34)
#define usb_alloc_coherent usb_buffer_alloc
#define usb_free_coherent usb_buffer_free
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 21)
/* sourced from the 2.6.20 kernel tree */
#include <linux/usb.h>
/* returns 0 if no match, 1 if match */
static inline int usb_match_device(struct usb_device *dev, const struct usb_device_id *id)
{
	if ((id->match_flags & USB_DEVICE_ID_MATCH_VENDOR) &&
	    id->idVendor != le16_to_cpu(dev->descriptor.idVendor))
		return 0;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_PRODUCT) &&
	    id->idProduct != le16_to_cpu(dev->descriptor.idProduct))
		return 0;

	/* No need to test id->bcdDevice_lo != 0, since 0 is never
	   greater than any unsigned number. */
	if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_LO) &&
	    (id->bcdDevice_lo > le16_to_cpu(dev->descriptor.bcdDevice)))
		return 0;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_HI) &&
	    (id->bcdDevice_hi < le16_to_cpu(dev->descriptor.bcdDevice)))
		return 0;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_CLASS) &&
	    (id->bDeviceClass != dev->descriptor.bDeviceClass))
		return 0;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_SUBCLASS) &&
	    (id->bDeviceSubClass!= dev->descriptor.bDeviceSubClass))
		return 0;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_PROTOCOL) &&
	    (id->bDeviceProtocol != dev->descriptor.bDeviceProtocol))
		return 0;

	return 1;
}
/* returns 0 if no match, 1 if match */
static inline int usb_match_one_id(struct usb_interface *interface,
		     const struct usb_device_id *id)
{
	struct usb_host_interface *intf;
	struct usb_device *dev;

	/* proc_connectinfo in devio.c may call us with id == NULL. */
	if (id == NULL)
		return 0;

	intf = interface->cur_altsetting;
	dev = interface_to_usbdev(interface);

	if (!usb_match_device(dev, id))
		return 0;

	/* The interface class, subclass, and protocol should never be
	 * checked for a match if the device class is Vendor Specific,
	 * unless the match record specifies the Vendor ID. */
	if (dev->descriptor.bDeviceClass == USB_CLASS_VENDOR_SPEC &&
			!(id->match_flags & USB_DEVICE_ID_MATCH_VENDOR) &&
			(id->match_flags & (USB_DEVICE_ID_MATCH_INT_CLASS |
				USB_DEVICE_ID_MATCH_INT_SUBCLASS |
				USB_DEVICE_ID_MATCH_INT_PROTOCOL)))
		return 0;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_CLASS) &&
	    (id->bInterfaceClass != intf->desc.bInterfaceClass))
		return 0;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_SUBCLASS) &&
	    (id->bInterfaceSubClass != intf->desc.bInterfaceSubClass))
		return 0;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_PROTOCOL) &&
	    (id->bInterfaceProtocol != intf->desc.bInterfaceProtocol))
		return 0;

	return 1;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
static inline int usb_autopm_get_interface(struct usb_interface *intf)
{ return 0; }

static inline void usb_autopm_put_interface(struct usb_interface *intf)
{ }
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
/*
 * usb_endpoint_* functions
 *
 * Included in Linux 2.6.19
 * Backported to 2.6.18 in Red Hat Enterprise Linux 5.2
 */

#ifdef RHEL_RELEASE_CODE
#if RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(5, 2)
#define RHEL_HAS_USB_ENDPOINT
#endif
#endif

#ifndef RHEL_HAS_USB_ENDPOINT
static inline int
usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd)
{
	return (epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN;
}

static inline int
usb_endpoint_xfer_int(const struct usb_endpoint_descriptor *epd)
{
	return (epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
		USB_ENDPOINT_XFER_INT;
}

static inline int
usb_endpoint_is_int_in(const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_int(epd) && usb_endpoint_dir_in(epd);
}
#endif
#endif

/* v4l2 subsystem fixups */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 29)
#define v4l2_file_operations file_operations
#define uvc_v4l2_open uvc_v4l2_open_compat
#define uvc_v4l2_release uvc_v4l2_release_compat
#include <media/v4l2-dev.h>
#include <media/v4l2-ioctl.h>
#define video_usercopy video_usercopy_compat
long video_usercopy_compat(struct file *file, unsigned int cmd,
				unsigned long arg, void* func);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28)
#define video_drvdata(file) video_get_drvdata(video_devdata(file))
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26)
#define get_unaligned_be16(a)					\
	be16_to_cpu(get_unaligned((unsigned short *)(a)))
#define put_unaligned_be16(r, a)				\
	put_unaligned(cpu_to_be16(r), ((unsigned short *)(a)))
#define get_unaligned_le16(a)					\
	le16_to_cpu(get_unaligned((unsigned short *)(a)))
#define put_unaligned_le16(r, a)				\
	put_unaligned(cpu_to_le16(r), ((unsigned short *)(a)))
#define get_unaligned_be32(a)					\
	be32_to_cpu(get_unaligned((u32 *)(a)))
#define put_unaligned_be32(r, a)				\
	put_unaligned(cpu_to_be32(r), ((u32 *)(a)))
#define get_unaligned_le32(a)					\
	le32_to_cpu(get_unaligned((u32 *)(a)))
#define put_unaligned_le32(r, a)				\
	put_unaligned(cpu_to_le32(r), ((u32 *)(a)))
#define get_unaligned_le64(a)					\
	le64_to_cpu(get_unaligned((u64 *)(a)))
#define put_unaligned_le64(r, a)				\
	put_unaligned(cpu_to_le64(r), ((u64 *)(a)))
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
#define clamp( x, l, h )        max_t( __typeof__( x ),		\
				      ( l ),			\
				      min_t( __typeof__( x ),	\
					     ( h ),        	\
					     ( x ) ) )
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22)
/*
 * Linked list API
 */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

/*
 * uninitialized_var() macro
 */
#define uninitialized_var(x) x = x
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
/* FIXME - only used in uvc_clock_param_set() */
#define strcasecmp(...) 1
#define strncasecmp(...) 1
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
/**
 * kmemdup - duplicate region of memory
 *
 * @src: memory region to duplicate
 * @len: memory region length
 * @gfp: GFP mask to use
 */
static inline void *kmemdup(const void *src, size_t len, gfp_t gfp)
{
	void *p;

	p = kmalloc(len, gfp);
	if (p)
		memcpy(p, src, len);
	return p;
}
#endif

/* <linux/videodev2.h> fixups */
#include <linux/videodev2.h>

#ifndef VIDIOC_ENUM_FRAMESIZES
enum v4l2_frmsizetypes {
	V4L2_FRMSIZE_TYPE_DISCRETE	= 1,
	V4L2_FRMSIZE_TYPE_CONTINUOUS	= 2,
	V4L2_FRMSIZE_TYPE_STEPWISE	= 3,
};

struct v4l2_frmsize_discrete {
	__u32			width;		/* Frame width [pixel] */
	__u32			height;		/* Frame height [pixel] */
};

struct v4l2_frmsize_stepwise {
	__u32			min_width;	/* Minimum frame width [pixel] */
	__u32			max_width;	/* Maximum frame width [pixel] */
	__u32			step_width;	/* Frame width step size [pixel] */
	__u32			min_height;	/* Minimum frame height [pixel] */
	__u32			max_height;	/* Maximum frame height [pixel] */
	__u32			step_height;	/* Frame height step size [pixel] */
};

struct v4l2_frmsizeenum {
	__u32			index;		/* Frame size number */
	__u32			pixel_format;	/* Pixel format */
	__u32			type;		/* Frame size type the device supports. */

	union {					/* Frame size */
		struct v4l2_frmsize_discrete	discrete;
		struct v4l2_frmsize_stepwise	stepwise;
	};

	__u32   reserved[2];			/* Reserved space for future use */
};
#define VIDIOC_ENUM_FRAMESIZES	_IOWR('V', 74, struct v4l2_frmsizeenum)
#endif

#ifndef VIDIOC_ENUM_FRAMEINTERVALS
enum v4l2_frmivaltypes {
	V4L2_FRMIVAL_TYPE_DISCRETE	= 1,
	V4L2_FRMIVAL_TYPE_CONTINUOUS	= 2,
	V4L2_FRMIVAL_TYPE_STEPWISE	= 3,
};

struct v4l2_frmival_stepwise {
	struct v4l2_fract	min;		/* Minimum frame interval [s] */
	struct v4l2_fract	max;		/* Maximum frame interval [s] */
	struct v4l2_fract	step;		/* Frame interval step size [s] */
};

struct v4l2_frmivalenum {
	__u32			index;		/* Frame format index */
	__u32			pixel_format;	/* Pixel format */
	__u32			width;		/* Frame width */
	__u32			height;		/* Frame height */
	__u32			type;		/* Frame interval type the device supports. */

	union {					/* Frame interval */
		struct v4l2_fract		discrete;
		struct v4l2_frmival_stepwise	stepwise;
	};

	__u32	reserved[2];			/* Reserved space for future use */
};
#define VIDIOC_ENUM_FRAMEINTERVALS _IOWR('V', 75, struct v4l2_frmivalenum)
#endif

#ifndef V4L2_CTRL_FLAG_WRITE_ONLY
#define V4L2_CTRL_FLAG_WRITE_ONLY 	0x0040
#endif

#ifndef V4L2_PIX_FMT_Y16
#define V4L2_PIX_FMT_Y16     v4l2_fourcc('Y', '1', '6', ' ') /* 16  Greyscale     */
#endif

#ifndef V4L2_PIX_FMT_H264
#define V4L2_PIX_FMT_H264    v4l2_fourcc('H', '2', '6', '4') /* H.264 Annex-B NAL Units */
#endif

#ifndef V4L2_CID_POWER_LINE_FREQUENCY
#define V4L2_CID_POWER_LINE_FREQUENCY	(V4L2_CID_BASE+24)
enum v4l2_power_line_frequency {
	V4L2_CID_POWER_LINE_FREQUENCY_DISABLED	= 0,
	V4L2_CID_POWER_LINE_FREQUENCY_50HZ	= 1,
	V4L2_CID_POWER_LINE_FREQUENCY_60HZ	= 2,
};
#endif
#ifndef V4L2_CID_HUE_AUTO
#define V4L2_CID_HUE_AUTO			(V4L2_CID_BASE+25)
#endif
#ifndef V4L2_CID_WHITE_BALANCE_TEMPERATURE
#define V4L2_CID_WHITE_BALANCE_TEMPERATURE	(V4L2_CID_BASE+26)
#endif
#ifndef V4L2_CID_SHARPNESS
#define V4L2_CID_SHARPNESS			(V4L2_CID_BASE+27)
#endif
#ifndef V4L2_CID_BACKLIGHT_COMPENSATION
#define V4L2_CID_BACKLIGHT_COMPENSATION 	(V4L2_CID_BASE+28)
#endif
#ifndef V4L2_CID_CHROMA_AGC
#define V4L2_CID_CHROMA_AGC                     (V4L2_CID_BASE+29)
#endif
#ifndef V4L2_CID_COLOR_KILLER
#define V4L2_CID_COLOR_KILLER                   (V4L2_CID_BASE+30)
#endif
#ifndef V4L2_CID_COLORFX
#define V4L2_CID_COLORFX			(V4L2_CID_BASE+31)
enum v4l2_colorfx {
	V4L2_COLORFX_NONE	= 0,
	V4L2_COLORFX_BW		= 1,
	V4L2_COLORFX_SEPIA	= 2,
	V4L2_COLORFX_NEGATIVE = 3,
	V4L2_COLORFX_EMBOSS = 4,
	V4L2_COLORFX_SKETCH = 5,
	V4L2_COLORFX_SKY_BLUE = 6,
	V4L2_COLORFX_GRASS_GREEN = 7,
	V4L2_COLORFX_SKIN_WHITEN = 8,
	V4L2_COLORFX_VIVID = 9,
};
#endif
#ifndef V4L2_CID_AUTOBRIGHTNESS
#define V4L2_CID_AUTOBRIGHTNESS			(V4L2_CID_BASE+32)
#endif
#ifndef V4L2_CID_BAND_STOP_FILTER
#define V4L2_CID_BAND_STOP_FILTER		(V4L2_CID_BASE+33)
#endif

#ifndef V4L2_CID_ROTATE
#define V4L2_CID_ROTATE				(V4L2_CID_BASE+34)
#endif
#ifndef V4L2_CID_BG_COLOR
#define V4L2_CID_BG_COLOR			(V4L2_CID_BASE+35)
#endif

#ifndef V4L2_CID_CHROMA_GAIN
#define V4L2_CID_CHROMA_GAIN                    (V4L2_CID_BASE+36)
#endif

#ifndef V4L2_CID_ILLUMINATORS_1
#define V4L2_CID_ILLUMINATORS_1			(V4L2_CID_BASE+37)
#endif
#ifndef V4L2_CID_ILLUMINATORS_2
#define V4L2_CID_ILLUMINATORS_2			(V4L2_CID_BASE+38)
#endif

/*  Camera class control IDs */
#ifndef V4L2_CTRL_CLASS_CAMERA
#define V4L2_CTRL_CLASS_CAMERA 0x009a0000	/* Camera class controls */
#endif
#ifndef V4L2_CID_CAMERA_CLASS_BASE
#define V4L2_CID_CAMERA_CLASS_BASE 	(V4L2_CTRL_CLASS_CAMERA | 0x900)
#endif
#ifndef V4L2_CID_CAMERA_CLASS
#define V4L2_CID_CAMERA_CLASS 		(V4L2_CTRL_CLASS_CAMERA | 1)
#endif

#ifndef V4L2_CID_EXPOSURE_AUTO
#define V4L2_CID_EXPOSURE_AUTO			(V4L2_CID_CAMERA_CLASS_BASE+1)
enum  v4l2_exposure_auto_type {
	V4L2_EXPOSURE_AUTO = 0,
	V4L2_EXPOSURE_MANUAL = 1,
	V4L2_EXPOSURE_SHUTTER_PRIORITY = 2,
	V4L2_EXPOSURE_APERTURE_PRIORITY = 3
};
#endif
#ifndef V4L2_CID_EXPOSURE_ABSOLUTE
#define V4L2_CID_EXPOSURE_ABSOLUTE		(V4L2_CID_CAMERA_CLASS_BASE+2)
#endif
#ifndef V4L2_CID_EXPOSURE_AUTO_PRIORITY
#define V4L2_CID_EXPOSURE_AUTO_PRIORITY		(V4L2_CID_CAMERA_CLASS_BASE+3)
#endif

#ifndef V4L2_CID_PAN_RELATIVE
#define V4L2_CID_PAN_RELATIVE			(V4L2_CID_CAMERA_CLASS_BASE+4)
#endif
#ifndef V4L2_CID_TILT_RELATIVE
#define V4L2_CID_TILT_RELATIVE			(V4L2_CID_CAMERA_CLASS_BASE+5)
#endif
#ifndef V4L2_CID_PAN_RESET
#define V4L2_CID_PAN_RESET			(V4L2_CID_CAMERA_CLASS_BASE+6)
#endif
#ifndef V4L2_CID_TILT_RESET
#define V4L2_CID_TILT_RESET			(V4L2_CID_CAMERA_CLASS_BASE+7)
#endif

#ifndef V4L2_CID_PAN_ABSOLUTE
#define V4L2_CID_PAN_ABSOLUTE			(V4L2_CID_CAMERA_CLASS_BASE+8)
#endif
#ifndef V4L2_CID_TILT_ABSOLUTE
#define V4L2_CID_TILT_ABSOLUTE			(V4L2_CID_CAMERA_CLASS_BASE+9)
#endif

#ifndef V4L2_CID_FOCUS_ABSOLUTE
#define V4L2_CID_FOCUS_ABSOLUTE			(V4L2_CID_CAMERA_CLASS_BASE+10)
#endif
#ifndef V4L2_CID_FOCUS_RELATIVE
#define V4L2_CID_FOCUS_RELATIVE			(V4L2_CID_CAMERA_CLASS_BASE+11)
#endif
#ifndef V4L2_CID_FOCUS_AUTO
#define V4L2_CID_FOCUS_AUTO			(V4L2_CID_CAMERA_CLASS_BASE+12)
#endif

#ifndef V4L2_CID_ZOOM_ABSOLUTE
#define V4L2_CID_ZOOM_ABSOLUTE			(V4L2_CID_CAMERA_CLASS_BASE+13)
#endif
#ifndef V4L2_CID_ZOOM_RELATIVE
#define V4L2_CID_ZOOM_RELATIVE			(V4L2_CID_CAMERA_CLASS_BASE+14)
#endif
#ifndef V4L2_CID_ZOOM_CONTINUOUS
#define V4L2_CID_ZOOM_CONTINUOUS		(V4L2_CID_CAMERA_CLASS_BASE+15)
#endif

#ifndef V4L2_CID_PRIVACY
#define V4L2_CID_PRIVACY			(V4L2_CID_CAMERA_CLASS_BASE+16)
#endif

#ifndef V4L2_CID_IRIS_ABSOLUTE
#define V4L2_CID_IRIS_ABSOLUTE			(V4L2_CID_CAMERA_CLASS_BASE+17)
#endif
#ifndef V4L2_CID_IRIS_RELATIVE
#define V4L2_CID_IRIS_RELATIVE			(V4L2_CID_CAMERA_CLASS_BASE+18)
#endif

#endif  /* _COMPAT_H */
