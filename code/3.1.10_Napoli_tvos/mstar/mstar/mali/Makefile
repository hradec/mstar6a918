MALI_TOP_DIR=
include $(src)/$(MALI_TOP_DIR)config.mak

ifneq ($(CONFIG_MALI400),)

MALI_DIR:=$(src)/$(MALI_TOP_DIR)$(MALI_VERSION)/linux/src/devicedrv/mali

ifeq ($(MALI_NEED_UPDATE),1)
$(shell rm -f $(src)/mali.mak)
$(call clean-mali-files,$(src))
endif

MALI_FILES_PREFIX =
MALI_USING_PROFILING ?= 1
MALI_USING_INTERNAL_PROFILING ?= 0
MALI_DMA_BUF_MAP_ON_ATTACH ?= 1
MALI_USING_DUMP_MEMORY_USAGE ?= 1

ifeq ($(MALI_TARGET_PLATFORM), mstar)
	MALI_SHARED_INTERRUPTS = 1
	MALI_PLATFORM = mstar
endif

MALI_PLATFORM ?= $(MALI_PLATFORM-$(MALI_TARGET_PLATFORM))

ifneq ($(CONFIG_UMP),)
CONFIG_MALI400_UMP=y
MALI_EXTRA_DEFINES += -DCONFIG_MALI400_UMP=1
endif

ifneq ($(MALI_PLATFORM),)
MALI_EXTRA_DEFINES += -DMALI_FAKE_PLATFORM_DEVICE=1
MALI_PLATFORM_FILES = $(wildcard $(MALI_DIR)/platform/$(MALI_PLATFORM)/*.c)
MALI_PLATFORM_FILES := $(patsubst $(MALI_DIR)/%,%,$(MALI_PLATFORM_FILES))
endif

ifeq ($(MALI_TARGET_PLATFORM), mstar)
MALI_MSTAR_PLATFORM_FILE = $(wildcard $(MALI_DIR)/platform/mstar/$(shell echo $(MALI_MSTAR_PLATFORM) | tr A-Z a-z)/*.c)
MALI_MSTAR_PLATFORM_FILE := $(patsubst $(MALI_DIR)/%,%,$(MALI_MSTAR_PLATFORM_FILE))
endif

ifeq ($(MALI_USING_PROFILING),1)
ifeq ($(CONFIG_TRACEPOINTS),)
$(warning CONFIG_TRACEPOINTS reqired for profiling)
else
CONFIG_MALI400_PROFILING=y
MALI_EXTRA_DEFINES += -DCONFIG_MALI400_PROFILING=1
ifeq ($(MALI_USING_INTERNAL_PROFILING),1)
CONFIG_MALI400_INTERNAL_PROFILING=y
MALI_EXTRA_DEFINES += -DCONFIG_MALI400_INTERNAL_PROFILING=1
endif
endif
endif

ifeq ($(MALI_USING_DUMP_MEMORY_USAGE),1)
export MALI_EXTRA_DEFINES += -DMSTAR_MALI_DUMP_MEMORY_USAGE=1
endif

ifeq ($(MALI_DMA_BUF_MAP_ON_ATTACH),1)
CONFIG_MALI_DMA_BUF_MAP_ON_ATTACH=y
MALI_EXTRA_DEFINES += -DCONFIG_MALI_DMA_BUF_MAP_ON_ATTACH
endif

ifeq ($(MALI_SHARED_INTERRUPTS),1)
CONFIG_MALI_SHARED_INTERRUPTS=y
MALI_EXTRA_DEFINES += -DCONFIG_MALI_SHARED_INTERRUPTS
endif

include $(src)/$(MALI_TOP_DIR)mali.mak

mali-y := $(foreach file,$(mali-y),$(MALI_VERSION)/linux/src/devicedrv/mali/$(file))
clean-files += mali.mak

$(src)/$(MALI_TOP_DIR)mali.mak: $(src)/$(MALI_TOP_DIR)$(MALI_VERSION)/linux/src/devicedrv/mali/Kbuild $(src)/$(MALI_TOP_DIR)platform.mak $(src)/$(MALI_TOP_DIR)project.mak .config
	@perl $(src)/$(MALI_TOP_DIR)scripts/update_kbuild.pl mali < $< > $@
	-@find $(src)/$(MALI_TOP_DIR)$(MALI_VERSION) \( -name '*.c' \) -type f -print | xargs touch
	@rm -f $(src)/__malidrv_build_info.c

endif # CONFIG_MALI400
