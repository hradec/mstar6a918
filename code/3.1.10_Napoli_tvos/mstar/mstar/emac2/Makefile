#
# Makefile for MStar EMAC device drivers.
#
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))
ifdef CONFIG_MSTAR_PROJECT_NAME
    CONFIG_MSTAR_PROJECT_NAME := $(subst ",,$(CONFIG_MSTAR_PROJECT_NAME))
endif

# general options
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/emac2
EXTRA_CFLAGS      +=  -Idrivers/mstar/emac2/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iinclude/linux

# specific options
EXTRA_CFLAGS      +=  -DRED_LION
# files
obj-$(CONFIG_MSTAR_EMAC2)             := mdrv-emac.o

mdrv-emac-objs    := mdrv_emac.o $(CONFIG_MSTAR_CHIP_NAME)/mhal_emac.o
