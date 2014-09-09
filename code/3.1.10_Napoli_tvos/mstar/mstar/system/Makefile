#
# Makefile for MStar System device drivers.
#
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))
ifdef CONFIG_MSTAR_PROJECT_NAME
    CONFIG_MSTAR_PROJECT_NAME := $(subst ",,$(CONFIG_MSTAR_PROJECT_NAME))
endif


# general options
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/system
EXTRA_CFLAGS      +=  -Idrivers/mstar/system/$(CONFIG_MSTAR_CHIP_NAME)
ifdef CONFIG_MSTAR_SPI_FLASH
EXTRA_CFLAGS      +=  -Idrivers/mstar/flash
EXTRA_CFLAGS      +=  -Idrivers/mstar/include/sys/common
EXTRA_CFLAGS      +=  -Idrivers/mstar/include/sys/drv
endif
# specific options
EXTRA_CFLAGS      +=

# files
obj-$(CONFIG_MSTAR_SYSTEM)	+= mdrv-system.o
mdrv-system-objs		+= mdrv_system.o
mdrv-system-objs		+= mdrv_system_io.o
