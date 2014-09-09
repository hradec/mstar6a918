#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 1995, 1998, 2001, 2002 by Ralf Baechle
# Copyright (C) 2004  Maciej W. Rozycki
#

#
# Some DECstations need all possible sections of an ECOFF executable
#
ifdef CONFIG_MACH_DECSTATION
  e2eflag := -a
endif

#
# Drop some uninteresting sections in the kernel.
# This is only relevant for ELF kernels but doesn't hurt a.out
#
drop-sections := .reginfo .mdebug .comment .note .pdr .options .MIPS.options
strip-flags   := $(addprefix --remove-section=,$(drop-sections))

VMLINUX = vmlinux

MKIMAGE_BIN = $(srctree)/arch/mips/boot/mkimage
LZO_BIN = $(srctree)/arch/mips/boot/lzo
KERNEL_RELEASE_FILE = $(srctree)/include/config/kernel.release

hostprogs-y := elf2ecoff

targets := vmlinux.ecoff
quiet_cmd_ecoff = ECOFF   $@
      cmd_ecoff = $(obj)/elf2ecoff $(VMLINUX) $@ $(e2eflag)
$(obj)/vmlinux.ecoff: $(obj)/elf2ecoff $(VMLINUX) FORCE
	$(call if_changed,ecoff)

targets += vmlinux.bin
quiet_cmd_bin = OBJCOPY $@
      cmd_bin = $(OBJCOPY) -O binary $(strip-flags) $(VMLINUX) $@

ifdef CONFIG_NVT_CHIP
vmlinux.bin: $(VMLINUX)
	$(OBJCOPY) -O binary $(strip-flags) $(VMLINUX) $(obj)/vmlinux.bin
ifndef CONFIG_PULLOUT_INITRAMFS
	@ gzip -9 -c $(obj)/$@ > $(obj)/$@.gz
	@ KERNEL_ENTRY=`gawk '$$3 == "kernel_entry" { print "0x"$$1 }' System.map`; \
	LD_ADDR=`gawk '$$3 == "_text" { print "0x"$$1 }' System.map`; \
	IMGNAME=Linux-`cat $(KERNEL_RELEASE_FILE)`; \
	MKIMAGE=$(MKIMAGE_BIN); \
	if [ -e $${MKIMAGE} ]; then \
		$${MKIMAGE}  -A mips -O linux -T kernel -C none -a $${LD_ADDR} -e $${KERNEL_ENTRY} -n $${IMGNAME} -d $(obj)/$@ $(obj)/uImage; \
		echo; \
		LZO=$(LZO_BIN); \
		if [ -e $${LZO} ] ; then \
		$${LZO} $(obj)/$@ $(obj)/$@.lzo; \
			chmod 777 $(obj)/$@.lzo; \
			$${MKIMAGE}  -A mips -O linux -T kernel -C lzo -a $${LD_ADDR} -e $${KERNEL_ENTRY} -n $${IMGNAME} -d $(obj)/$@.lzo $(obj)/uImage.lzo; \
			echo; \
		fi; \
		$${MKIMAGE}  -A mips -O linux -T kernel -C gzip -a $${LD_ADDR} -e $${KERNEL_ENTRY} -n $${IMGNAME} -d $(obj)/$@.gz $(obj)/uImage.gz; \
		echo; \
	else \
		echo ">> Can't find $${MKIMAGE}. Please check the u-boot path or build u-boot <<"; \
	fi
endif
else
$(obj)/uImage: STARTADDR=$(LOADADDR)

$(obj)/uImage:	$(obj)/zImage FORCE
	$(call if_changed,uimage)
	@echo '  Image $@ is ready'
vmlinux.bin: $(VMLINUX) FORCE
	$(OBJCOPY) -O binary $(strip-flags) $(VMLINUX) $(obj)/vmlinux.bin
	@ gzip -9 -c $(obj)/$@ > $(obj)/$@.gz
	@ KERNEL_ENTRY=`gawk '$$3 == "kernel_entry" { print "0x"$$1 }' System.map`; \
        LD_ADDR=`gawk '$$3 == "_text" { print "0x"$$1 }' System.map`; \
        IMGNAME=Linux-`cat $(KERNEL_RELEASE_FILE)`-mstar; \
    MKIMAGE=$(MKIMAGE_BIN); \
        if [ -e $${MKIMAGE} ]; then \
            $${MKIMAGE}  -A mips -O linux -T kernel -C none -a $${LD_ADDR} -e $${KERNEL_ENTRY} -n $${IMGNAME} -d $(obj)/$@ $(obj)/uImage; \
            echo; \
        LZO=$(LZO_BIN); \
            if [ -e $${LZO} ] ; then \
                $${LZO} $(obj)/$@ $(obj)/$@.lzo; \
                chmod 777 $(obj)/$@.lzo; \
                $${MKIMAGE}  -A mips -O linux -T kernel -C lzo -a $${LD_ADDR} -e $${KERNEL_ENTRY} -n $${IMGNAME} -d $(obj)/$@.lzo $(obj)/uImage.lzo; \
                echo; \
            fi; \
            $${MKIMAGE}  -A mips -O linux -T kernel -C gzip -a $${LD_ADDR} -e $${KERNEL_ENTRY} -n $${IMGNAME} -d $(obj)/$@.gz $(obj)/uImage.gz; \
            echo; \
        else \
            echo ">> Can't find $${MKIMAGE}. Please check the u-boot path or build u-boot <<"; \
        fi
endif
targets += vmlinux.srec
quiet_cmd_srec = OBJCOPY $@
      cmd_srec = $(OBJCOPY) -S -O srec $(strip-flags) $(VMLINUX) $@
$(obj)/vmlinux.srec: $(VMLINUX) FORCE
	$(call if_changed,srec)
