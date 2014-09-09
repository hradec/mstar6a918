cmd_arch/arm/arm-boards/generic/memory.o := /tools/arm/arm-2012.09/bin/arm-none-linux-gnueabi-gcc -Wp,-MD,arch/arm/arm-boards/generic/.memory.o.d  -nostdinc -isystem /tools/arm/arm-2012.09/bin/../lib/gcc/arm-none-linux-gnueabi/4.7.2/include -I/home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include -Iarch/arm/include/generated -Iinclude  -include /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -I/home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/arm-boards/generic/include/ -Iarch/arm/arm-boards/napoli//include -Iarch/arm/arm-boards/plat-mstar//include -Iarch/arm/arm-boards/napoli/ -Iarch/arm/arm-boards/plat-mstar/ -I drivers/mstar -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Os -marm -fno-dwarf2-cfi-asm -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -Wno-unused-but-set-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -pg -fno-inline-functions-called-once -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -DCC_HAVE_ASM_GOTO -Werror -Iarch/arm/arm-boards/napoli -Iarch/arm/arm-boards/napoli/board    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(memory)"  -D"KBUILD_MODNAME=KBUILD_STR(memory)" -c -o arch/arm/arm-boards/generic/memory.o arch/arm/arm-boards/generic/memory.c

source_arch/arm/arm-boards/generic/memory.o := arch/arm/arm-boards/generic/memory.c

deps_arch/arm/arm-boards/generic/memory.o := \
    $(wildcard include/config/cpu/big/endian.h) \
    $(wildcard include/config/mips/malta.h) \
    $(wildcard include/config/mstar/str/crc.h) \
    $(wildcard include/config/mstar/offset/for/sboot.h) \
    $(wildcard include/config/mp/platform/verify/lx/mem/align.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/include/linux/kconfig.h \
    $(wildcard include/config/h.h) \
    $(wildcard include/config/.h) \
    $(wildcard include/config/foo.h) \
  include/linux/init.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
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
  include/linux/mm.h \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/sysctl.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/stack/growsup.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/transparent/hugepage.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/sparsemem.h) \
    $(wildcard include/config/sparsemem/vmemmap.h) \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/ksm.h) \
    $(wildcard include/config/arch/populates/node/map.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/debug/pagealloc.h) \
    $(wildcard include/config/hibernation.h) \
    $(wildcard include/config/hugetlbfs.h) \
  include/linux/errno.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/errno.h \
  include/asm-generic/errno.h \
  include/asm-generic/errno-base.h \
  include/linux/gfp.h \
    $(wildcard include/config/zone/dma32.h) \
    $(wildcard include/config/kmemcheck.h) \
    $(wildcard include/config/mp/platform/arm.h) \
    $(wildcard include/config/mips.h) \
    $(wildcard include/config/zone/dma.h) \
  include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/arm.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/compaction.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/cgroup/mem/res/ctlr.h) \
    $(wildcard include/config/no/bootmem.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/have/memoryless/nodes.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
    $(wildcard include/config/have/arch/pfn/valid.h) \
    $(wildcard include/config/nodes/span/other/nodes.h) \
    $(wildcard include/config/holes/in/zone.h) \
    $(wildcard include/config/arch/has/holes/memorymodel.h) \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  include/linux/typecheck.h \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/preempt/count.h) \
    $(wildcard include/config/preempt/notifiers.h) \
  include/linux/thread_info.h \
    $(wildcard include/config/compat.h) \
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
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/posix_types.h \
  include/linux/bitops.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/bitops.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/system.h \
    $(wildcard include/config/function/graph/tracer.h) \
    $(wildcard include/config/cpu/32v6k.h) \
    $(wildcard include/config/cpu/xsc3.h) \
    $(wildcard include/config/cpu/fa526.h) \
    $(wildcard include/config/arch/has/barriers.h) \
    $(wildcard include/config/arm/dma/mem/bufferable.h) \
    $(wildcard include/config/cpu/sa1100.h) \
    $(wildcard include/config/cpu/sa110.h) \
    $(wildcard include/config/cpu/v6.h) \
  include/linux/linkage.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/linkage.h \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/irqflags.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
    $(wildcard include/config/arm/thumb.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/hwcap.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/outercache.h \
    $(wildcard include/config/outer/cache/sync.h) \
    $(wildcard include/config/outer/cache.h) \
  include/mstar/mpatch_macro.h \
    $(wildcard include/config/mp/platform/arch/general.h) \
    $(wildcard include/config/mp/platform/arch/general/debug.h) \
    $(wildcard include/config/mp/platform/arm/debug.h) \
    $(wildcard include/config/mp/platform/mips.h) \
    $(wildcard include/config/mp/platform/mips/debug.h) \
    $(wildcard include/config/mp/platform/fixme.h) \
    $(wildcard include/config/mp/platform/fixme/debug.h) \
    $(wildcard include/config/mp/platform/arm/pmu.h) \
    $(wildcard include/config/mp/platform/arm/pmu/debug.h) \
    $(wildcard include/config/mp/platform/pm.h) \
    $(wildcard include/config/mp/platform/pm/debug.h) \
    $(wildcard include/config/mp/platform/arm/errata/775420.h) \
    $(wildcard include/config/mp/platform/arm/errata/775420/debug.h) \
    $(wildcard include/config/mp/platform/mstar/legancy/intr.h) \
    $(wildcard include/config/mp/platform/mstar/legancy/intr/debug.h) \
    $(wildcard include/config/mp/platform/sw/patch/l2/sram/rma.h) \
    $(wildcard include/config/mp/platform/sw/patch/l2/sram/rma/debug.h) \
    $(wildcard include/config/mp/platform/mips74k/timer.h) \
    $(wildcard include/config/mp/platform/mips74k/timer/debug.h) \
    $(wildcard include/config/mp/platform/arm/mstar/etm.h) \
    $(wildcard include/config/mp/platform/arm/mstar/etm/debug.h) \
    $(wildcard include/config/mp/platform/int/1/to/1/spi.h) \
    $(wildcard include/config/mp/platform/int/1/to/1/spi/debug.h) \
    $(wildcard include/config/mp/platform/disable/jiffies/overflow/debug.h) \
    $(wildcard include/config/mp/platform/disable/jiffies/overflow/debug/debug.h) \
    $(wildcard include/config/mp/platform/lzma/bin/compressed.h) \
    $(wildcard include/config/mp/platform/lzma/bin/compressed/debug.h) \
    $(wildcard include/config/mp/platform/cpu/setting.h) \
    $(wildcard include/config/mp/platform/cpu/setting/debug.h) \
    $(wildcard include/config/mp/platform/mips/system/call/get/version.h) \
    $(wildcard include/config/mp/platform/mips/system/call/get/version/debug.h) \
    $(wildcard include/config/mp/platform/verify/lx/mem/align/debug.h) \
    $(wildcard include/config/mp/nand/ubi.h) \
    $(wildcard include/config/mp/nand/ubi/debug.h) \
    $(wildcard include/config/mp/nand/mtd.h) \
    $(wildcard include/config/mp/nand/mtd/debug.h) \
    $(wildcard include/config/mp/nand/ubifs.h) \
    $(wildcard include/config/mp/nand/ubifs/debug.h) \
    $(wildcard include/config/mp/nand/spansion.h) \
    $(wildcard include/config/mp/nand/spansion/debug.h) \
    $(wildcard include/config/mp/nand/bbt.h) \
    $(wildcard include/config/mp/nand/bbt/debug.h) \
    $(wildcard include/config/mp/nand/bbt/size.h) \
    $(wildcard include/config/mp/nand/bbt/size/debug.h) \
    $(wildcard include/config/mp/usb/mstar.h) \
    $(wildcard include/config/mp/usb/mstar/debug.h) \
    $(wildcard include/config/mp/sd/mstar.h) \
    $(wildcard include/config/mp/sd/mstar/debug.h) \
    $(wildcard include/config/mp/sd/plug.h) \
    $(wildcard include/config/mp/sd/plug/debug.h) \
    $(wildcard include/config/mp/emmc/partition.h) \
    $(wildcard include/config/mp/emmc/partition/debug.h) \
    $(wildcard include/config/mp/emmc/mmc/patch.h) \
    $(wildcard include/config/mp/emmc/mmc/patch/debug.h) \
    $(wildcard include/config/mp/emmc/trim.h) \
    $(wildcard include/config/mp/emmc/trim/debug.h) \
    $(wildcard include/config/mp/emmc/cache.h) \
    $(wildcard include/config/mp/emmc/cache/debug.h) \
    $(wildcard include/config/mp/emmc/datatag.h) \
    $(wildcard include/config/mp/emmc/datatag/debug.h) \
    $(wildcard include/config/mp/jbd2/commit/num/limit.h) \
    $(wildcard include/config/mp/jbd2/commit/num/limit/debug.h) \
    $(wildcard include/config/mp/jbd2/reset/journal/sb.h) \
    $(wildcard include/config/mp/jbd2/reset/journal/sb/debug.h) \
    $(wildcard include/config/mp/mstar/str/base.h) \
    $(wildcard include/config/mp/mstar/str/base/debug.h) \
    $(wildcard include/config/mp/android/low/mem/killer/include/cachemem.h) \
    $(wildcard include/config/mp/android/low/mem/killer/include/cachemem/debug.h) \
    $(wildcard include/config/mp/android/dummy/mstar/rtc.h) \
    $(wildcard include/config/mp/android/dummy/mstar/rtc/debug.h) \
    $(wildcard include/config/mp/android/alarm/clock/boottime/patch.h) \
    $(wildcard include/config/mp/android/alarm/clock/boottime/patch/debug.h) \
    $(wildcard include/config/mp/android/mstar/rc/map/define.h) \
    $(wildcard include/config/mp/android/mstar/rc/map/define/debug.h) \
    $(wildcard include/config/mp/android/mstar/hardcode/lpj.h) \
    $(wildcard include/config/mp/android/mstar/hardcode/lpj/debug.h) \
    $(wildcard include/config/mp/compiler/error.h) \
    $(wildcard include/config/mp/compiler/error/debug.h) \
    $(wildcard include/config/mp/debug/tool/watchdog.h) \
    $(wildcard include/config/mp/debug/tool/watchdog/debug.h) \
    $(wildcard include/config/mp/debug/tool/codedump.h) \
    $(wildcard include/config/mp/debug/tool/codedump/debug.h) \
    $(wildcard include/config/mp/debug/tool/codedump/data/sync.h) \
    $(wildcard include/config/mp/debug/tool/codedump/data/sync/debug.h) \
    $(wildcard include/config/mp/debug/tool/coredump/path.h) \
    $(wildcard include/config/mp/debug/tool/coredump/path/debug.h) \
    $(wildcard include/config/mp/debug/tool/coredump/path/option.h) \
    $(wildcard include/config/mp/debug/tool/coredump/path/option/debug.h) \
    $(wildcard include/config/mp/debug/tool/coredump/detect/usb/plugin.h) \
    $(wildcard include/config/mp/debug/tool/coredump/detect/usb/plugin/debug.h) \
    $(wildcard include/config/mp/debug/tool/kdebug.h) \
    $(wildcard include/config/mp/debug/tool/kdebug/debug.h) \
    $(wildcard include/config/mp/debug/tool/changelist/debug.h) \
    $(wildcard include/config/mp/debug/tool/oprofile.h) \
    $(wildcard include/config/mp/debug/tool/oprofile/debug.h) \
    $(wildcard include/config/mp/debug/tool/ubi.h) \
    $(wildcard include/config/mp/debug/tool/ubi/debug.h) \
    $(wildcard include/config/mp/debug/tool/oom.h) \
    $(wildcard include/config/mp/debug/tool/oom/debug.h) \
    $(wildcard include/config/mp/debug/tool/log.h) \
    $(wildcard include/config/mp/debug/tool/log/debug.h) \
    $(wildcard include/config/mp/debug/tool/rm.h) \
    $(wildcard include/config/mp/debug/tool/rm/debug.h) \
    $(wildcard include/config/mp/debug/tool/force/ignored/core/dump/path/bootargs/when/usb/plugin.h) \
    $(wildcard include/config/mp/debug/tool/force/ignored/core/dump/path/bootargs/when/usb/plugin/debug.h) \
    $(wildcard include/config/mp/debug/tool/ramlog.h) \
    $(wildcard include/config/mp/debug/tool/ramlog/debug.h) \
    $(wildcard include/config/mp/debug/tool/rtp/trace.h) \
    $(wildcard include/config/mp/debug/tool/rtp/trace/debug.h) \
    $(wildcard include/config/mp/debug/tool/skip/pulling/running/rt/task.h) \
    $(wildcard include/config/mp/debug/tool/skip/pulling/running/rt/task/debug.h) \
    $(wildcard include/config/mp/debug/tool/interrupt/debug.h) \
    $(wildcard include/config/mp/debug/tool/interrupt/debug/debug.h) \
    $(wildcard include/config/mp/remote/control/rc/register/retry.h) \
    $(wildcard include/config/mp/remote/control/rc/register/retry/debug.h) \
    $(wildcard include/config/mp/scsi/mstar/sd/card/hotplug.h) \
    $(wildcard include/config/mp/scsi/mstar/sd/card/hotplug/debug.h) \
    $(wildcard include/config/mp/scsi/hd/suspend.h) \
    $(wildcard include/config/mp/scsi/hd/suspend/debug.h) \
    $(wildcard include/config/mp/scsi/multi/lun.h) \
    $(wildcard include/config/mp/scsi/multi/lun/debug.h) \
    $(wildcard include/config/mp/mm/mstar/3dalloc/miu0/1.h) \
    $(wildcard include/config/mp/mm/mstar/3dalloc/miu0/1/debug.h) \
    $(wildcard include/config/mp/mm/mali/mmu/notifier.h) \
    $(wildcard include/config/mp/mm/mali/mmu/notifier/debug.h) \
    $(wildcard include/config/mp/mm/mali/reserve.h) \
    $(wildcard include/config/mp/mm/mali/reserve/debug.h) \
    $(wildcard include/config/mp/mmap/bufferable.h) \
    $(wildcard include/config/mp/mmap/bufferable/debug.h) \
    $(wildcard include/config/mp/mmap/mmap/boundary/protect.h) \
    $(wildcard include/config/mp/mmap/mmap/boundary/protect/debug.h) \
    $(wildcard include/config/mp/miu/mapping.h) \
    $(wildcard include/config/mp/miu/mapping/debug.h) \
    $(wildcard include/config/mp/mips/l2/catch.h) \
    $(wildcard include/config/mp/mips/l2/catch/debug.h) \
    $(wildcard include/config/mp/wdt/off/dbg.h) \
    $(wildcard include/config/mp/wdt/off/dbg/debug.h) \
    $(wildcard include/config/mp/camera/plug/out.h) \
    $(wildcard include/config/mp/camera/plug/out/debug.h) \
    $(wildcard include/config/mp/sysattr/show.h) \
    $(wildcard include/config/mp/sysattr/show/debug.h) \
    $(wildcard include/config/mp/mips/highmem/cache/alias/patch.h) \
    $(wildcard include/config/mp/mips/highmem/cache/alias/patch/debug.h) \
    $(wildcard include/config/mp/mips/highmem/memory/present/patch.h) \
    $(wildcard include/config/mp/mips/highmem/memory/present/patch/debug.h) \
    $(wildcard include/config/mp/checkpt/boot.h) \
    $(wildcard include/config/mp/checkpt/boot/debug.h) \
    $(wildcard include/config/mp/webcam/init.h) \
    $(wildcard include/config/mp/webcam/init/debug.h) \
    $(wildcard include/config/mp/mips/mips16e/unaligned/access.h) \
    $(wildcard include/config/mp/mips/mips16e/unaligned/access/debug.h) \
    $(wildcard include/config/mp/ntfs3g/wrap.h) \
    $(wildcard include/config/mp/ntfs3g/wrap/debug.h) \
    $(wildcard include/config/mp/bdi/mstar/bdi/patch.h) \
    $(wildcard include/config/mp/bdi/mstar/bdi/patch/debug.h) \
    $(wildcard include/config/mp/smp/startup.h) \
    $(wildcard include/config/mp/smp/startup/debug.h) \
    $(wildcard include/config/mp/uart/serial/8250/riu/base.h) \
    $(wildcard include/config/mp/uart/serial/8250/riu/base/debug.h) \
    $(wildcard include/config/mp/uart/serial/open/set/baudrate.h) \
    $(wildcard include/config/mp/uart/serial/open/set/baudrate/debug.h) \
    $(wildcard include/config/mp/ntfs/ioctl.h) \
    $(wildcard include/config/mp/ntfs/ioctl/debug.h) \
    $(wildcard include/config/mp/ntfs/volume/label.h) \
    $(wildcard include/config/mp/ntfs/volume/label/debug.h) \
    $(wildcard include/config/mp/ntfs/volume/id.h) \
    $(wildcard include/config/mp/ntfs/volume/id/debug.h) \
    $(wildcard include/config/mp/ntfs/read/pages.h) \
    $(wildcard include/config/mp/ntfs/read/pages/debug.h) \
    $(wildcard include/config/mp/ntfs/refine/readdir.h) \
    $(wildcard include/config/mp/ntfs/refine/readdir/debug.h) \
    $(wildcard include/config/mp/kernel/compat/from/2/6/35/11/to/3/1/10.h) \
    $(wildcard include/config/mp/kernel/compat/from/2/6/35/11/to/3/1/10/debug.h) \
    $(wildcard include/config/mp/kernel/compat/patch/fix/inode/cluster/list.h) \
    $(wildcard include/config/mp/kernel/compat/patch/fix/inode/cluster/list/debug.h) \
    $(wildcard include/config/mp/ethernet/mstar/icmp/enhance.h) \
    $(wildcard include/config/mp/ethernet/mstar/icmp/enhance/debug.h) \
    $(wildcard include/config/mp/usb/str/patch.h) \
    $(wildcard include/config/mp/usb/str/patch/debug.h) \
    $(wildcard include/config/mp/fat/volume/label.h) \
    $(wildcard include/config/mp/fat/volume/label/debug.h) \
    $(wildcard include/config/mp/ca7/quad/core/patch.h) \
    $(wildcard include/config/mp/ca7/quad/core/patch/debug.h) \
    $(wildcard include/config/mp/ca7/hw/break/points/enable.h) \
    $(wildcard include/config/mp/ca7/hw/break/points/enable/debug.h) \
    $(wildcard include/config/mp/ca7/performance/monitor/enable.h) \
    $(wildcard include/config/mp/ca7/performance/monitor/enable/debug.h) \
    $(wildcard include/config/mp/kernel/bug/patch/remove.h) \
    $(wildcard include/config/mp/kernel/bug/patch/remove/debug.h) \
    $(wildcard include/config/mp/hid/hidraw/patch.h) \
    $(wildcard include/config/mp/hid/hidraw/patch/debug.h) \
    $(wildcard include/config/mp/hid/hidraw/opensrc.h) \
    $(wildcard include/config/mp/hid/hidraw/opensrc/debug.h) \
    $(wildcard include/config/mp/hid/hidraw/trylock.h) \
    $(wildcard include/config/mp/hid/hidraw/trylock/debug.h) \
    $(wildcard include/config/mp/blcr/compile/patch.h) \
    $(wildcard include/config/mp/blcr/compile/patch/debug.h) \
    $(wildcard include/config/mp/wireless/mstar.h) \
    $(wildcard include/config/mp/wireless/mstar/debug.h) \
    $(wildcard include/config/mp/sata/ata/core/patch.h) \
    $(wildcard include/config/mp/sata/ata/core/patch/debug.h) \
    $(wildcard include/config/mp/acp/l2.h) \
    $(wildcard include/config/mp/acp/l2/debug.h) \
    $(wildcard include/config/mp/acp/direction.h) \
    $(wildcard include/config/mp/acp/direction/debug.h) \
    $(wildcard include/config/mp/temp/debug/eiffel/netflix.h) \
    $(wildcard include/config/mp/temp/debug/eiffel/netflix/debug.h) \
    $(wildcard include/config/mp/schd/debug/re/sched/nor.h) \
    $(wildcard include/config/mp/schd/debug/re/sched/nor/debug.h) \
    $(wildcard include/config/mp/antutu/benchmark/rise/performance.h) \
    $(wildcard include/config/mp/antutu/benchmark/rise/performance/debug.h) \
  include/asm-generic/cmpxchg-local.h \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/arch_hweight.h \
  include/asm-generic/bitops/const_hweight.h \
  include/asm-generic/bitops/lock.h \
  include/asm-generic/bitops/le.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/linux/swab.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/swab.h \
  include/linux/byteorder/generic.h \
  include/asm-generic/bitops/ext2-atomic-setbit.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/thread_info.h \
    $(wildcard include/config/arm/thumbee.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/fpstate.h \
    $(wildcard include/config/vfpv3.h) \
    $(wildcard include/config/iwmmxt.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/domain.h \
    $(wildcard include/config/io/36.h) \
    $(wildcard include/config/cpu/use/domains.h) \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  include/linux/poison.h \
    $(wildcard include/config/illegal/pointer/value.h) \
  include/linux/const.h \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/atomic/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/ring/buffer.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  /tools/arm/arm-2012.09/bin/../lib/gcc/arm-none-linux-gnueabi/4.7.2/include/stdarg.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/printk.h \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
  include/linux/dynamic_debug.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/div64.h \
  include/linux/stringify.h \
  include/linux/bottom_half.h \
  include/linux/spinlock_types.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/spinlock_types.h \
  include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
    $(wildcard include/config/prove/rcu.h) \
  include/linux/rwlock_types.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/spinlock.h \
    $(wildcard include/config/thumb2/kernel.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/processor.h \
    $(wildcard include/config/have/hw/breakpoint.h) \
    $(wildcard include/config/arm/errata/754327.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/hw_breakpoint.h \
  include/linux/rwlock.h \
  include/linux/spinlock_api_smp.h \
    $(wildcard include/config/inline/spin/lock.h) \
    $(wildcard include/config/inline/spin/lock/bh.h) \
    $(wildcard include/config/inline/spin/lock/irq.h) \
    $(wildcard include/config/inline/spin/lock/irqsave.h) \
    $(wildcard include/config/inline/spin/trylock.h) \
    $(wildcard include/config/inline/spin/trylock/bh.h) \
    $(wildcard include/config/inline/spin/unlock.h) \
    $(wildcard include/config/inline/spin/unlock/bh.h) \
    $(wildcard include/config/inline/spin/unlock/irq.h) \
    $(wildcard include/config/inline/spin/unlock/irqrestore.h) \
  include/linux/rwlock_api_smp.h \
    $(wildcard include/config/inline/read/lock.h) \
    $(wildcard include/config/inline/write/lock.h) \
    $(wildcard include/config/inline/read/lock/bh.h) \
    $(wildcard include/config/inline/write/lock/bh.h) \
    $(wildcard include/config/inline/read/lock/irq.h) \
    $(wildcard include/config/inline/write/lock/irq.h) \
    $(wildcard include/config/inline/read/lock/irqsave.h) \
    $(wildcard include/config/inline/write/lock/irqsave.h) \
    $(wildcard include/config/inline/read/trylock.h) \
    $(wildcard include/config/inline/write/trylock.h) \
    $(wildcard include/config/inline/read/unlock.h) \
    $(wildcard include/config/inline/write/unlock.h) \
    $(wildcard include/config/inline/read/unlock/bh.h) \
    $(wildcard include/config/inline/write/unlock/bh.h) \
    $(wildcard include/config/inline/read/unlock/irq.h) \
    $(wildcard include/config/inline/write/unlock/irq.h) \
    $(wildcard include/config/inline/read/unlock/irqrestore.h) \
    $(wildcard include/config/inline/write/unlock/irqrestore.h) \
  include/linux/atomic.h \
    $(wildcard include/config/arch/has/atomic/or.h) \
    $(wildcard include/config/generic/atomic64.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/atomic.h \
  include/asm-generic/atomic-long.h \
  include/linux/wait.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/current.h \
  include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/cache.h \
    $(wildcard include/config/arm/l1/cache/shift.h) \
    $(wildcard include/config/aeabi.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  include/linux/seqlock.h \
  include/linux/nodemask.h \
  include/linux/bitmap.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/string.h \
  include/linux/pageblock-flags.h \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/hugetlb/page/size/variable.h) \
  include/generated/bounds.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/page.h \
    $(wildcard include/config/cpu/copy/v3.h) \
    $(wildcard include/config/cpu/copy/v4wt.h) \
    $(wildcard include/config/cpu/copy/v4wb.h) \
    $(wildcard include/config/cpu/copy/feroceon.h) \
    $(wildcard include/config/cpu/copy/fa.h) \
    $(wildcard include/config/cpu/xscale.h) \
    $(wildcard include/config/cpu/copy/v6.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/glue.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/memory.h \
    $(wildcard include/config/page/offset.h) \
    $(wildcard include/config/task/size.h) \
    $(wildcard include/config/dram/size.h) \
    $(wildcard include/config/dram/base.h) \
    $(wildcard include/config/have/tcm.h) \
    $(wildcard include/config/arm/patch/phys/virt.h) \
    $(wildcard include/config/arm/patch/phys/virt/16bit.h) \
  arch/arm/arm-boards/napoli//include/mach/memory.h \
    $(wildcard include/config/memory/start/address.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/sizes.h \
  include/asm-generic/sizes.h \
  include/asm-generic/memory_model.h \
  include/asm-generic/getorder.h \
  include/linux/memory_hotplug.h \
    $(wildcard include/config/memory/hotremove.h) \
    $(wildcard include/config/have/arch/nodedata/extension.h) \
  include/linux/notifier.h \
  include/linux/mutex.h \
    $(wildcard include/config/debug/mutexes.h) \
    $(wildcard include/config/have/arch/mutex/cpu/relax.h) \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/linux/rwsem-spinlock.h \
  include/linux/srcu.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/sparsemem.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
    $(wildcard include/config/sched/mc.h) \
    $(wildcard include/config/sched/book.h) \
    $(wildcard include/config/use/percpu/numa/node/id.h) \
  include/linux/cpumask.h \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
    $(wildcard include/config/disable/obsolete/cpumask/functions.h) \
  include/linux/smp.h \
    $(wildcard include/config/use/generic/smp/helpers.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/smp.h \
  include/linux/percpu.h \
    $(wildcard include/config/need/per/cpu/embed/first/chunk.h) \
    $(wildcard include/config/need/per/cpu/page/first/chunk.h) \
    $(wildcard include/config/have/setup/per/cpu/area.h) \
  include/linux/pfn.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/percpu.h \
  include/asm-generic/percpu.h \
  include/linux/percpu-defs.h \
    $(wildcard include/config/debug/force/weak/per/cpu.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/topology.h \
  include/asm-generic/topology.h \
  include/linux/mmdebug.h \
    $(wildcard include/config/debug/vm.h) \
    $(wildcard include/config/debug/virtual.h) \
  include/linux/rbtree.h \
  include/linux/prio_tree.h \
  include/linux/debug_locks.h \
    $(wildcard include/config/debug/locking/api/selftests.h) \
  include/linux/mm_types.h \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/want/page/debug/flags.h) \
    $(wildcard include/config/slub.h) \
    $(wildcard include/config/cmpxchg/local.h) \
    $(wildcard include/config/aio.h) \
    $(wildcard include/config/mm/owner.h) \
  include/linux/auxvec.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/auxvec.h \
  include/linux/completion.h \
  include/linux/page-debug-flags.h \
    $(wildcard include/config/page/poisoning.h) \
    $(wildcard include/config/page/debug/something/else.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/mmu.h \
    $(wildcard include/config/cpu/has/asid.h) \
  include/linux/range.h \
  include/linux/bit_spinlock.h \
  include/linux/shrinker.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/pgtable.h \
    $(wildcard include/config/highpte.h) \
  include/asm-generic/4level-fixup.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/proc-fns.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/glue-proc.h \
    $(wildcard include/config/cpu/arm610.h) \
    $(wildcard include/config/cpu/arm7tdmi.h) \
    $(wildcard include/config/cpu/arm710.h) \
    $(wildcard include/config/cpu/arm720t.h) \
    $(wildcard include/config/cpu/arm740t.h) \
    $(wildcard include/config/cpu/arm9tdmi.h) \
    $(wildcard include/config/cpu/arm920t.h) \
    $(wildcard include/config/cpu/arm922t.h) \
    $(wildcard include/config/cpu/arm925t.h) \
    $(wildcard include/config/cpu/arm926t.h) \
    $(wildcard include/config/cpu/arm940t.h) \
    $(wildcard include/config/cpu/arm946e.h) \
    $(wildcard include/config/cpu/arm1020.h) \
    $(wildcard include/config/cpu/arm1020e.h) \
    $(wildcard include/config/cpu/arm1022.h) \
    $(wildcard include/config/cpu/arm1026.h) \
    $(wildcard include/config/cpu/mohawk.h) \
    $(wildcard include/config/cpu/feroceon.h) \
    $(wildcard include/config/cpu/v6k.h) \
    $(wildcard include/config/cpu/v7.h) \
  arch/arm/arm-boards/napoli//include/mach/vmalloc.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/pgtable-hwdef.h \
  include/asm-generic/pgtable.h \
  include/linux/page-flags.h \
    $(wildcard include/config/pageflags/extended.h) \
    $(wildcard include/config/arch/uses/pg/uncached.h) \
    $(wildcard include/config/memory/failure.h) \
    $(wildcard include/config/swap.h) \
    $(wildcard include/config/s390.h) \
  include/linux/huge_mm.h \
  include/linux/vmstat.h \
    $(wildcard include/config/vm/event/counters.h) \
  include/linux/vm_event_item.h \
  include/linux/bootmem.h \
    $(wildcard include/config/have/arch/bootmem/node.h) \
    $(wildcard include/config/have/arch/alloc/remap.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/dma.h \
    $(wildcard include/config/isa/dma/api.h) \
    $(wildcard include/config/pci.h) \
  include/linux/module.h \
    $(wildcard include/config/sysfs.h) \
    $(wildcard include/config/unused/symbols.h) \
    $(wildcard include/config/kallsyms.h) \
    $(wildcard include/config/tracepoints.h) \
    $(wildcard include/config/event/tracing.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/constructors.h) \
    $(wildcard include/config/debug/set/module/ronx.h) \
  include/linux/stat.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/stat.h \
  include/linux/time.h \
    $(wildcard include/config/arch/uses/gettimeoffset.h) \
  include/linux/math64.h \
  include/linux/kmod.h \
  include/linux/workqueue.h \
    $(wildcard include/config/debug/objects/work.h) \
    $(wildcard include/config/freezer.h) \
  include/linux/timer.h \
    $(wildcard include/config/timer/stats.h) \
    $(wildcard include/config/debug/objects/timers.h) \
  include/linux/ktime.h \
    $(wildcard include/config/ktime/scalar.h) \
  include/linux/jiffies.h \
  include/linux/timex.h \
  include/linux/param.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/param.h \
    $(wildcard include/config/hz.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/timex.h \
  arch/arm/arm-boards/napoli//include/mach/timex.h \
  include/linux/debugobjects.h \
    $(wildcard include/config/debug/objects.h) \
    $(wildcard include/config/debug/objects/free.h) \
  include/linux/sysctl.h \
  include/linux/rcupdate.h \
    $(wildcard include/config/rcu/torture/test.h) \
    $(wildcard include/config/tree/rcu.h) \
    $(wildcard include/config/tree/preempt/rcu.h) \
    $(wildcard include/config/preempt/rcu.h) \
    $(wildcard include/config/no/hz.h) \
    $(wildcard include/config/tiny/rcu.h) \
    $(wildcard include/config/tiny/preempt/rcu.h) \
    $(wildcard include/config/debug/objects/rcu/head.h) \
    $(wildcard include/config/preempt/rt.h) \
  include/linux/rcutree.h \
  include/linux/elf.h \
  include/linux/elf-em.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/elf.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/user.h \
  include/linux/kobject.h \
  include/linux/sysfs.h \
  include/linux/kobject_ns.h \
  include/linux/kref.h \
  include/linux/moduleparam.h \
    $(wildcard include/config/alpha.h) \
    $(wildcard include/config/ppc64.h) \
  include/linux/tracepoint.h \
  include/linux/jump_label.h \
    $(wildcard include/config/jump/label.h) \
  include/linux/export.h \
    $(wildcard include/config/symbol/prefix.h) \
    $(wildcard include/config/modversions.h) \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/module.h \
    $(wildcard include/config/arm/unwind.h) \
  include/trace/events/module.h \
  include/trace/define_trace.h \
  include/linux/binfmts.h \
  include/linux/capability.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/sections.h \
  include/asm-generic/sections.h \
  /home/cax/workdir/mstar6a918/code/3.1.10_Napoli_tvos/3.1.10/arch/arm/include/asm/arm-boards/prom.h \
  arch/arm/arm-boards/napoli/board/Board.h \
    $(wildcard include/config/mstar/arm/bd/fpga.h) \
    $(wildcard include/config/mstar/arm/bd/generic.h) \
  arch/arm/arm-boards/napoli/board/BD_GENERIC.h \
    $(wildcard include/config/mstar/arm/mmap/128mb/128mb.h) \
    $(wildcard include/config/mstar/arm/mmap/256mb/256mb.h) \
    $(wildcard include/config/mstar/mmap/128mb/128mb/default.h) \
  arch/arm/arm-boards/napoli/board/mmap/mmap_128mb_128mb.h \
    $(wildcard include/config/mstar/reserve/mem/for/aeon.h) \
    $(wildcard include/config/mstar/reserve/mem/for/aeon/size.h) \
    $(wildcard include/config/mstar/titania/bd/t2/cus03/mbrd/board/atsc/1.h) \
    $(wildcard include/config/mstar/titania/bd/t2/cus03/mibrd/board/atsc/1.h) \
  arch/arm/arm-boards/napoli//include/mach/io.h \
    $(wildcard include/config/oc/etm.h) \
  include/linux/version.h \

arch/arm/arm-boards/generic/memory.o: $(deps_arch/arm/arm-boards/generic/memory.o)

$(deps_arch/arm/arm-boards/generic/memory.o):
