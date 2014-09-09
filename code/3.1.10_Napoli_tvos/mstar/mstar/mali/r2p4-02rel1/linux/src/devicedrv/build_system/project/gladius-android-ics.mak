#
# Copyright (C) 2010-2012 MStar Semiconductor, Inc. All rights reserved.
# 
# This program is free software and is provided to you under the terms of the GNU General Public License version 2
# as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
# 
# A copy of the licence is included with the program, and can also be obtained from Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

include $(if $(KBUILD_EXTMOD),$(KBUILD_EXTMOD)/,)../build_system/platform/gladius.mak

# Mali build options
USING_UMP = 1
USING_PMM = 1
USING_MALI_RUN_TIME_PM ?= 1

BUILD ?= release

# toolchain
export CROSS_COMPILE ?= arm-none-linux-gnueabi-

# project build flags
CONFIG_BUILDFLAGS :=
CONFIG_BUILDFLAGS += -I$(KDIR)/mstar/include
CONFIG_BUILDFLAGS += -I$(KDIR)/mstar/drv/clkgen/pub
CONFIG_BUILDFLAGS += -I$(KDIR)/mstar/drv/wrapper/pub
CONFIG_BUILDFLAGS += -I$(KDIR)/mstar/hal/msw8x68/clkgen/pub
CONFIG_BUILDFLAGS += -I$(KDIR)/mstar/hal/msw8x68/pmu/pub

# IRQ
MALI_IRQ := E_IRQ_DBB_GPU2MCU
