ifneq ($(CONFIG_MALI400)$(CONFIG_UMP),)

# version
MALI_ALL_VERSIONS := r2p3-01rel0
MALI_ALL_VERSIONS += r2p4-02rel1
MALI_ALL_VERSIONS += r3p1-01rel0
MALI_ALL_VERSIONS += r3p1-01rel1
MALI_ALL_VERSIONS += r3p2-01rel0
MALI_ALL_VERSIONS += r3p2-01rel1
MALI_ALL_VERSIONS += r3p2-01rel2
MALI_ALL_VERSIONS += r3p2-01rel3

# platform
MALI_ALL_PLATFORMS := supernova
MALI_ALL_PLATFORMS += android-ics
MALI_ALL_PLATFORMS += android-jellybean
MALI_ALL_PLATFORMS += android-jellybean_mr1
MALI_ALL_PLATFORMS += android-jellybean_mr2

MALI_TOP_DIR ?=

MALI_VERSION := $(strip $(foreach version,$(MALI_ALL_VERSIONS),\
	$(if $(CONFIG_MALI400_VERSION_$(shell echo -n $(version) | tr a-z- A-Z_)),$(version),)))

MALI_PLATFORM := $(strip $(foreach platform,$(MALI_ALL_PLATFORMS),\
	$(if $(CONFIG_MALI400_PLATFORM_$(shell echo -n $(platform) | tr a-z- A-Z_)),$(platform),)))

ifeq ($(MALI_VERSION),)
	$(error Mali version not specified)
endif

ifeq ($(MALI_PLATFORM),)
	$(error Mali platform not specified)
endif

MALI_CHIP_NAME = $(patsubst "%",%,$(CONFIG_MSTAR_CHIP_NAME))

MALI_CONFIG := $(MALI_CHIP_NAME)-$(MALI_PLATFORM)

MALI_DRV_DIR := $(src)/$(MALI_TOP_DIR)$(MALI_VERSION)/linux/src/devicedrv

ifeq ($(wildcard $(MALI_DRV_DIR)/build_system/project/$(MALI_CONFIG).mak),)
	$(error Mali project config $(MALI_CONFIG) not exist)
endif

MALI_NEED_UPDATE := 0

MALI_BUILD_VERSION := $(MALI_VERSION) $(MALI_CONFIG)

MALI_BUILD_VERSION_FILE := $(src)/build.version
MALI_CURRENT_BUILD_VERSION := $(shell [ -f $(MALI_BUILD_VERSION_FILE) ] && cat $(MALI_BUILD_VERSION_FILE) || echo -n $(MALI_BUILD_VERSION))
ifneq ($(MALI_CURRENT_BUILD_VERSION),$(MALI_BUILD_VERSION))
	MALI_CURRENT_VERSION := $(firstword $(MALI_CURRENT_BUILD_VERSION))
	MALI_NEED_UPDATE := 1
$(shell rm -f $(src)/$(MALI_TOP_DIR)platform.mak)
$(shell rm -f $(src)/$(MALI_TOP_DIR)project.mak)
endif

$(shell echo $(MALI_BUILD_VERSION) > $(MALI_BUILD_VERSION_FILE))

include $(src)/$(MALI_TOP_DIR)platform.mak
include $(src)/$(MALI_TOP_DIR)project.mak

clean-files += $(MALI_TOP_DIR)platform.mak $(MALI_TOP_DIR)project.mak

MALI_CHECK_KERNEL_CONFIG = $(MALI_CHECK_CONFIG_CHIP_NAME)
ifeq ($(MAKECMDGOALS),clean)
MALI_CHECK_KERNEL_CONFIG := 0
endif

$(src)/$(MALI_TOP_DIR)platform.mak: $(MALI_DRV_DIR)/build_system/platform/$(MALI_CHIP_NAME).mak .config
	@perl $(src)/$(MALI_TOP_DIR)scripts/update_platform.pl < $< > $@

$(src)/$(MALI_TOP_DIR)project.mak: $(MALI_DRV_DIR)/build_system/project/$(MALI_CONFIG).mak $(src)/$(MALI_TOP_DIR)platform.mak .config
	@perl $(src)/$(MALI_TOP_DIR)scripts/update_project.pl < $< > $@

define clean-mali-files
$(shell cd $1; rm -f \
	*.[oas] *.ko .*.cmd \
	.*.d .*.tmp *.mod.c \
	*.symtypes modules.order \
	modules.builtin .tmp_*.o.* \
	*.gcno)
endef

define clean-mali-files-recursive
$(shell find $1 \
	\( -name '*.[oas]' -o -name '*.ko' -o -name '.*.cmd' \
	-o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' \
	-o -name '*.symtypes' -o -name 'modules.order' \
	-o -name modules.builtin -o -name '.tmp_*.o.*' \
	-o -name '*.gcno' \) -type f -print | xargs rm -f)
endef

endif # CONFIG_MALI400 || CONFIG_UMP
