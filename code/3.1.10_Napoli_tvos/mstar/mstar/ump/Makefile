MALI_TOP_DIR=../mali/

include $(src)/$(MALI_TOP_DIR)config.mak

ifneq ($(CONFIG_UMP),)

ifeq ($(MALI_NEED_UPDATE),1)
$(shell rm -f $(src)/ump.mak)
$(call clean-mali-files,$(src))
endif

include $(src)/ump.mak

$(src)/ump.mak: $(src)/$(MALI_TOP_DIR)$(MALI_VERSION)/linux/src/devicedrv/ump/Kbuild $(src)/$(MALI_TOP_DIR)platform.mak $(src)/$(MALI_TOP_DIR)project.mak .config
	@perl $(src)/$(MALI_TOP_DIR)scripts/update_kbuild.pl ump < $< > $@
	-@find $(src)/$(MALI_TOP_DIR)$(MALI_VERSION) \( -name '*.c' \) -type f -print | xargs touch

clean-files += ump.mak
ump-y := $(foreach file,$(ump-y),../mali/$(MALI_VERSION)/linux/src/devicedrv/ump/$(file))

endif # CONFIG_UMP
