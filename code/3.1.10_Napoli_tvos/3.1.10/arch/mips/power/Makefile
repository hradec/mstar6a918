obj-$(CONFIG_HIBERNATION) += cpu.o
ifeq (y,$(CONFIG_MSTAR_FASTBOOT))
obj-$(CONFIG_MSTAR_FASTBOOT) += mstar_hibernate.o
else
ifeq (y,$(CONFIG_MP_MSTAR_STR_BASE))
obj-$(CONFIG_MP_MSTAR_STR_BASE) += mstar_sleep.o
else
obj-$(CONFIG_HIBERNATION) += hibernate.o
endif
endif
