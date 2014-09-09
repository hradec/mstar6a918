#
# Makefile for MStar CPU_setting device drivers.
#

ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
#EXTRA_CFLAGS      +=  -Idrivers/mstar/common
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/CPU_setting
EXTRA_CFLAGS      +=  -Idrivers/mstar/CPU_setting/include


# specific options
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
obj-$(CONFIG_CPU_FREQ)	+= mdrv_CPU_setting.o

obj-$(CONFIG_CPU_FREQ)	+= core.o lm.o
#obj-$(CONFIG_CPU_FREQ)		+= cpu.o
obj-$(CONFIG_CPU_FREQ)		+= impd1.o
#mdrv-CPU_setting-objs    := mdrv_CPU_setting.o


#export header files

