#
# Makefile for the linux kernel.
#

obj-y := chip_irq.o chip_arch.o chip_timer.o chip_early_printk.o 
obj-$(CONFIG_SMP) 			+= platsmp.o headsmp.o
obj-$(CONFIG_LOCAL_TIMERS) 	+= localtimer.o
obj-y += platform.o
obj-$(CONFIG_MSTAR_STR) 	+= sleep.o pm.o sleep_helper.o \
							   sleep_helper_asm.o
