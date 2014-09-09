EXTRA_CFLAGS	:= -include drivers/media/video/uvc_maxim/compat.h
CPPFLAGS        := $(CPPFLAGS)  -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
uvcvideo_maxim-objs  := uvc_driver.o uvc_queue.o uvc_v4l2.o uvc_video.o uvc_ctrl.o \
		  uvc_status.o uvc_isight.o
obj-$(CONFIG_MAXIM_USB_VIDEO_CLASS) += uvcvideo_maxim.o
