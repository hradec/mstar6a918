obj-y	:= clock.o
obj-$(CONFIG_LOCAL_TIMERS) += localtimer.o
obj-$(CONFIG_PLAT_VERSATILE_CLCD) += clcd.o
obj-$(CONFIG_PLAT_VERSATILE_FPGA_IRQ) += fpga-irq.o
obj-$(CONFIG_PLAT_VERSATILE_LEDS) += leds.o
ifeq ($(KERNELVERSION),3.8.13)
obj-$(CONFIG_PLAT_VERSATILE_SCHED_CLOCK) += 3.8.13-sched-clock.o
else
obj-$(CONFIG_PLAT_VERSATILE_SCHED_CLOCK) += sched-clock.o
endif
obj-$(CONFIG_SMP) += headsmp.o platsmp.o
obj-$(CONFIG_HOTPLUG_CPU) += hotplug.o
