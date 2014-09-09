cmd_drivers/mstar/sc/napoli/mhal_sc.o := /tools/arm/arm-2012.09/bin/arm-none-linux-gnueabi-gcc -Wp,-MD,drivers/mstar/sc/napoli/.mhal_sc.o.d  -nostdinc -isystem /tools/arm/arm-2012.09/bin/../lib/gcc/arm-none-linux-gnueabi/4.7.2/include -I/home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include -Iarch/arm/include/generated -Iinclude  -include /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -I/home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/arm-boards/generic/include/ -Iarch/arm/arm-boards/napoli//include -Iarch/arm/arm-boards/plat-mstar//include -Iarch/arm/arm-boards/napoli/ -Iarch/arm/arm-boards/plat-mstar/ -I drivers/mstar -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Os -marm -fno-dwarf2-cfi-asm -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -Wno-unused-but-set-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -pg -fno-inline-functions-called-once -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -DCC_HAVE_ASM_GOTO -Idrivers/mstar/include -Idrivers/mstar/sc -Idrivers/mstar/sc/napoli    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(mhal_sc)"  -D"KBUILD_MODNAME=KBUILD_STR(mdrv_sc)" -c -o drivers/mstar/sc/napoli/mhal_sc.o drivers/mstar/sc/napoli/mhal_sc.c

source_drivers/mstar/sc/napoli/mhal_sc.o := drivers/mstar/sc/napoli/mhal_sc.c

deps_drivers/mstar/sc/napoli/mhal_sc.o := \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/include/linux/kconfig.h \
    $(wildcard include/config/h.h) \
    $(wildcard include/config/.h) \
    $(wildcard include/config/foo.h) \
  drivers/mstar/sc/napoli/reg_sc.h \

drivers/mstar/sc/napoli/mhal_sc.o: $(deps_drivers/mstar/sc/napoli/mhal_sc.o)

$(deps_drivers/mstar/sc/napoli/mhal_sc.o):
