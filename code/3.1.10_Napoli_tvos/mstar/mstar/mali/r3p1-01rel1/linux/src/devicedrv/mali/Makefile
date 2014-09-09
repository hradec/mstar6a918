#
# Copyright (C) 2010-2012 ARM Limited. All rights reserved.
# 
# This program is free software and is provided to you under the terms of the GNU General Public License version 2
# as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
# 
# A copy of the licence is included with the program, and can also be obtained from Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

USE_UMPV2=0

# The Makefile sets up "arch" based on the CONFIG, creates the version info
# string and the __malidrv_build_info.c file, and then call the Linux build
# system to actually build the driver. After that point the Kbuild file takes
# over.

# set up defaults if not defined by the user
ARCH ?= arm

#OSKOS=linux
OSKOS?=linux
FILES_PREFIX=

check_cc2 = \
	$(shell if $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; \
	then \
		echo "$(2)"; \
	else \
		echo "$(3)"; \
	fi ;)

# MStar project config
include ../build_system/project/$(CONFIG).mak

# This conditional makefile exports the global definition ARM_INTERNAL_BUILD. Customer releases will not include arm_internal.mak
-include ../../../arm_internal.mak

# Check that required parameters are supplied.
ifeq ($(CONFIG),)
$(error "CONFIG must be specified.")
endif
ifeq ($(OSKOS),msos)
KDIR=.  # no KDIR for MSOS actually
else
ifeq ($(CPU)$(KDIR),)
$(error "KDIR or CPU must be specified.")
endif
endif
ifeq ($(CONFIG_CHIP_NAME),)
$(error "CONFIG_CHIP_NAME must be specified.")
endif

ifeq ($(USING_UMP),1)
ifeq ($(USE_UMPV2),1)
UMP_SYMVERS_FILE ?= ../umpv2/Module.symvers
else
UMP_SYMVERS_FILE ?= ../ump/Module.symvers
endif
KBUILD_EXTRA_SYMBOLS = $(realpath $(UMP_SYMVERS_FILE))
$(warning $(KBUILD_EXTRA_SYMBOLS))
endif

# Get any user defined KDIR-<names> or maybe even a hardcoded KDIR
-include KDIR_CONFIGURATION

# Define host system directory
KDIR-$(shell uname -m):=/lib/modules/$(shell uname -r)/build

ifeq ($(ARCH), arm)
# when compiling for ARM we're cross compiling
export CROSS_COMPILE ?= $(call check_cc2, arm-linux-gnueabi-gcc, arm-linux-gnueabi-, arm-none-linux-gnueabi-)
endif

# look up KDIR based om CPU selection
KDIR ?= $(KDIR-$(CPU))

# validate lookup result
ifeq ($(KDIR),)
$(error No KDIR found for platform $(CPU))
endif

# report detected/selected settings
ifdef ARM_INTERNAL_BUILD
$(warning Config $(CONFIG))
$(warning Host CPU $(CPU))
$(warning OS_MEMORY $(USING_OS_MEMORY))
endif

# Set up build config
export CONFIG_MALI400=m

ifeq ($(USING_GPU_UTILIZATION),1)
export EXTRA_DEFINES += -DCONFIG_MALI400_GPU_UTILIZATION=1
export CONFIG_MALI400_GPU_UTILIZATION := y
endif

ifeq ($(OSKOS),msos)

BUILD_SYSTEM_DIR = ../../../build_system
include $(BUILD_SYSTEM_DIR)/toolchains/$(TARGET_TOOLCHAIN).mak
$(eval $(call $(TARGET_TOOLCHAIN)-setup,TARGET))

include Kbuild

objects := $(SRC:.c=.o)
objects := $(objects:.cpp=.o)
objects := $(abspath $(objects))

target = $(abspath mali.a)

clean_files = $(objects) $(target)

%.o: %.c
	@echo "  [CC]" $@
	@$(TARGET_CC) -c -o $@ $< $(ccflags-y)

%.o: %.cpp
	@echo "  [CC]" $@
	@$(TARGET_CC) -c -o $@ $< $(ccflags-y)

all: $(target)
	@rm __malidrv_build_info.c __malidrv_build_info.o

$(target): make-build-info-file $(objects)
	@echo "  [AR]" $@
	@$(TARGET_AR) $(TARGET_ARFLAGS) $@ $(objects)

make-build-info-file:
	@echo 'char *__malidrv_build_info(void) { return "malidrv: $(VERSION_STRINGS)";}' > $(FILES_PREFIX)__malidrv_build_info.c
	@echo "  [CC]" $(abspath $(FILES_PREFIX)__malidrv_build_info.o)
	@$(TARGET_CC) -c -o $(FILES_PREFIX)__malidrv_build_info.o $(FILES_PREFIX)__malidrv_build_info.c $(ccflags-y)

clean:
	@echo "  [CLEAN] *.o mali.a"
	@rm -f $(clean_files)
	@[ -L arch ] && rm arch

else

export CHECK_KERNEL_CONFIG = $(CHECK_CONFIG_CHIP_NAME)

ifeq ($(MAKECMDGOALS),clean)
CHECK_KERNEL_CONFIG := 0
endif

all: $(UMP_SYMVERS_FILE)
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) M=$(CURDIR) modules
	@rm $(FILES_PREFIX)__malidrv_build_info.c $(FILES_PREFIX)__malidrv_build_info.o

clean:
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) M=$(CURDIR) clean
	@[ -L arch ] && rm arch

kernelrelease:
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) kernelrelease

endif

export CONFIG KBUILD_EXTRA_SYMBOLS
