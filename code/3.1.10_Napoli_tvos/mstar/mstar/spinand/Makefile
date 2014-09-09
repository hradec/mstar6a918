#
# Makefile for MStar SPI NAND device drivers.
#
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/spinand/
EXTRA_CFLAGS      +=  -Idrivers/mstar/spinand/inc/common
EXTRA_CFLAGS      +=  -Idrivers/mstar/spinand/inc/config/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/mstar/spinand/inc/config
$(warning $(CONFIG_MSTAR_CHIP_NAME))
$(warning $(EXTRA_CFLAGS))

# specific options
obj-y             += spinand.o
obj-y	            += src/common/drvSPINAND.o
obj-y               += src/$(CONFIG_MSTAR_CHIP_NAME)/halSPINAND.o

