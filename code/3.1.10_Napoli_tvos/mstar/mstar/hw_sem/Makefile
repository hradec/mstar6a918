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
EXTRA_CFLAGS      +=  -Idrivers/mstar/hw_sem
EXTRA_CFLAGS      +=  -Idrivers/mstar/hw_sem/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
obj-$(CONFIG_MSTAR_HW_SEM)	+= mdrv-hw-sem.o
mdrv-hw-sem-objs			+= mdrv_hw_sem.o
mdrv-hw-sem-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_hw_sem.o

# export header files
