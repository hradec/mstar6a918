#
# Makefile for MStar NAND device drivers.
#
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))
ifdef CONFIG_MSTAR_PROJECT_NAME
    CONFIG_MSTAR_PROJECT_NAME := $(subst ",,$(CONFIG_MSTAR_PROJECT_NAME))
endif

# general options
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/include/sys/common
EXTRA_CFLAGS      +=  -Idrivers/mstar/include/sys/drv
EXTRA_CFLAGS      +=  -Idrivers/mstar/mspi
EXTRA_CFLAGS      +=  -Idrivers/mstar/mspi/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
#EXTRA_CFLAGS      +=  -O0
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
obj-y   += drvMSPI.o $(CONFIG_MSTAR_CHIP_NAME)/
obj-y	+= drvMSPI.o

header-y = drvMSPI.h

