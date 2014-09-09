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
EXTRA_CFLAGS      +=  -Idrivers/mstar/spi
EXTRA_CFLAGS      +=  -Idrivers/mstar/spi/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
#EXTRA_CFLAGS      +=  -O0
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
obj-y   += drvSPI.o $(CONFIG_MSTAR_CHIP_NAME)/
obj-y	+= drvSPI.o

header-y = drvSPI.h

