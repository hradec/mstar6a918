#
# Makefile for MStar Video Prealloc Memory device drivers.
#
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))
ifdef CONFIG_MSTAR_PROJECT_NAME
    CONFIG_MSTAR_PROJECT_NAME := $(subst ",,$(CONFIG_MSTAR_PROJECT_NAME))
endif

# general options
#EXTRA_CFLAGS      +=  -Idrivers/mstar/common
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/sata_host
EXTRA_CFLAGS      +=  -Idrivers/mstar/sata_host/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=

# files
obj-$(CONFIG_MSTAR_SATA_HOST)	+= mdrv_sata_host.o
mdrv-sata-host-objs		+= mdrv_sata_host.o
