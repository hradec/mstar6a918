#
# Makefile for MStar IIC device drivers.
#

ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
# EXTRA_CFLAGS      +=  -Idrivers/mstar/common
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/wdt
EXTRA_CFLAGS      +=  -Idrivers/mstar/wdt/hal/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
obj-$(CONFIG_MSTAR_WDT)	+= mdrv-wdt.o
mdrv-wdt-objs			+= mdrv_wdt.o mdrv_wdt_io.o

mdrv-wdt-objs			+= ./hal/$(CONFIG_MSTAR_CHIP_NAME)/mhal_wdt.o

# export header files
EXPORT_H_FILES    += mdrv_wdt_io.h
