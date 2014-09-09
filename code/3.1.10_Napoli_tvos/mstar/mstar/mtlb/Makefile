#
# Makefile for MStar device drivers.
#
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))
ifdef CONFIG_MSTAR_PROJECT_NAME
    CONFIG_MSTAR_PROJECT_NAME := $(subst ",,$(CONFIG_MSTAR_PROJECT_NAME))
endif

# general options
#EXTRA_CFLAGS     +=  -Idrivers/mstar/common
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/mtlb
EXTRA_CFLAGS      +=  -Idrivers/mstar/mtlb/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  -DRED_LION

# files
obj-$(CONFIG_MSTAR_MTLB)       += mdrv_mtlb.o 
obj-$(CONFIG_MSTAR_MTLB)       += $(CONFIG_MSTAR_CHIP_NAME)/mhal_mtlb.o  

