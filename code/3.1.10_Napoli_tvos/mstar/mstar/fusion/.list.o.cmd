cmd_drivers/mstar/fusion/list.o := /tools/arm/arm-2012.09/bin/arm-none-linux-gnueabi-gcc -Wp,-MD,drivers/mstar/fusion/.list.o.d  -nostdinc -isystem /tools/arm/arm-2012.09/bin/../lib/gcc/arm-none-linux-gnueabi/4.7.2/include -I/home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include -Iarch/arm/include/generated -Iinclude  -include /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -I/home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/arm-boards/generic/include/ -Iarch/arm/arm-boards/napoli//include -Iarch/arm/arm-boards/plat-mstar//include -Iarch/arm/arm-boards/napoli/ -Iarch/arm/arm-boards/plat-mstar/ -I drivers/mstar -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Os -marm -fno-dwarf2-cfi-asm -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -Wno-unused-but-set-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -pg -fno-inline-functions-called-once -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -DCC_HAVE_ASM_GOTO -Idrivers/mstar/include -Idrivers/mstar/fusion    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(list)"  -D"KBUILD_MODNAME=KBUILD_STR(fusion)" -c -o drivers/mstar/fusion/list.o drivers/mstar/fusion/list.c

source_drivers/mstar/fusion/list.o := drivers/mstar/fusion/list.c

deps_drivers/mstar/fusion/list.o := \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/include/linux/kconfig.h \
    $(wildcard include/config/h.h) \
    $(wildcard include/config/.h) \
    $(wildcard include/config/foo.h) \
  include/linux/types.h \
    $(wildcard include/config/mp/debug/tool/changelist.h) \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/posix_types.h \
  drivers/mstar/fusion/list.h \

drivers/mstar/fusion/list.o: $(deps_drivers/mstar/fusion/list.o)

$(deps_drivers/mstar/fusion/list.o):
