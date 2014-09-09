EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/fusion

obj-$(CONFIG_MSTAR_FUSION) += fusion.o

fusion-y := call.o entries.o fifo.o fusiondev.o fusionee.o list.o property.o reactor.o ref.o skirmish.o shmpool.o
$(obj)/call.o: include/linux/fusion.h
include/linux/fusion.h: FORCE
	@cp ../mstar/mstar/fusion/include/fusion.h  include/linux/fusion.h -f

