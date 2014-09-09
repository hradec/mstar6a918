#
# Makefile for MStar POINTS_HANDLER device drivers.
#

ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
#EXTRA_CFLAGS      +=  -Idrivers/mstar/common
EXTRA_CFLAGS      +=  -Idrivers/mstar/include


# specific options
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
obj-$(CONFIG_MSTAR_POINTS_HANDLER)	+= check_points.o

mdrv-check_points-objs    := check_points.o


#export header files

