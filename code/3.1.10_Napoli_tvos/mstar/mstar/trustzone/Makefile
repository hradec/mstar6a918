EXTRA_CFLAGS      +=  -Idrivers/trustzone/
EXTRA_CFLAGS      +=  -Idrivers/mstar/include/

mname := trustzone_modules
$(mname)-objs := mdrv_trustzone.o mdrv_smc.o
obj-y	 += mdrv_trustzone.o  
obj-y	 += mdrv_smc.o
obj-y	 += proc.o
#obj-$(CONFIG_TZ_WORLD_TALK_SUPPORT)	 += mdrv_trustzone.o  
