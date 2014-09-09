#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/vfpmacros.h>
#include "mstar_hibernate.h"

SAVE_IMP_DEFINE0 = 0
SAVE_IMP_DEFINE1 = 1

.text
.align 4
bmstar_fast_boot: .long 0
fastboot_savedlr: .long 0

fastboot_save_mmu_ctr:  .long 0
fastboot_save_mmu_ttb:  .long 0
fastboot_save_mmu_dmn:  .long 0
//@ c1
fastboot_save_mmu_ACTLR: .long 0
fastboot_save_mmu_CPACR: .long 0
//@ c2
fastboot_save_mmu_TTBR1: .long 0
fastboot_save_mmu_TTBCR: .long 0
//@ c10
fastboot_save_mmu_PRRR:  .long 0
fastboot_save_mmu_NMRR:  .long 0
//@ c13
fastboot_save_mmu_FCSEIDR:  .long 0
fastboot_save_mmu_CTXIDR:   .long 0
fastboot_save_mmu_TPIDRURW: .long 0
fastboot_save_mmu_TPIDRURO: .long 0
fastboot_save_mmu_TPIDRPRW: .long 0

fastboot_save_mmu_VBAR:     .long 0

.if (SAVE_IMP_DEFINE0==1)
fastboot_save_imp_define0:   .long 0
.endif

.if (SAVE_IMP_DEFINE1==1)
fastboot_save_imp_define1:   .long 0
.endif

//@
fastboot_save_asimde_FPSCR:  .long 0
fastboot_save_asimde_FPEXEC: .long 0


ENTRY(is_mstar_fastboot)
    adr r0, bmstar_fast_boot
    ldr r0, [r0]
    mov pc, lr

ENTRY(set_mstar_fastboot)
    adr r0, bmstar_fast_boot
    ldr r1, [r0]
    add r1, r1, #1
    str r1, [r0]
    mov pc, lr


ENTRY(unset_mstar_fastboot)
    adr r0, bmstar_fast_boot
    ldr r1, [r0]
    sub r1, r1, #1
    str r1, [r0]
    mov pc, lr

ENTRY(fastboot_put_char)
#ifdef CONFIG_EARLY_PRINTK
    b   prom_putchar
#else
    mov pc, lr
#endif

ENTRY(mstar_fastboot_save_last)
    stmfd   sp!,{r0-r7,lr}
    adrl    r2, fastboot_save_mmu_ctr
    mrc     p15, 0, r3, c1, c0, 0   /* MMU Control */
    str     r3, [r2], #4
    mrc     p15, 0, r3, c2, c0, 0   /* TTB address. */
    str     r3, [r2], #4
    mrc     p15, 0, r3, c3, c0, 0   /* domain access control. */
    str     r3, [r2], #4
    mrc     p15,0,r3,c1,c0,1        /* ACTLR*/
    str     r3, [r2], #4
    mrc     p15,0,r3,c1,c0,2        /* CPACR*/
    str     r3, [r2], #4
    mrc     p15, 0, r3, c2, c0, 1   /* TTBR 1 */
    str     r3, [r2], #4
    mrc     p15, 0, r3, c2, c0, 2   /* TTBCR */
    str     r3, [r2], #4
    mrc     p15, 0, r3, c10, c2, 0   /* PRRR */
    str     r3, [r2], #4
    mrc     p15, 0, r3, c10, c2, 1   /* NMRR */
    str     r3, [r2], #4
    mrc     p15,0,r3,c13,c0,0        /* FCSEIDR */
    str     r3, [r2], #4
    mrc     p15,0,r3,c13,c0,1        /* CTXIDR */
    str     r3, [r2], #4
    mrc     p15,0,r3,c13,c0,2        /* TPIDRURW */
    str     r3, [r2], #4
    mrc     p15,0,r3,c13,c0,3        /* TPIDRURO */
    str     r3, [r2], #4
    mrc     p15,0,r3,c13,c0,4        /* TPIDRPRW */
    str     r3, [r2], #4
    mrc	    p15,0,r3,c12,c0,0	     /* VBAR */
    str     r3, [r2], #4
.if (SAVE_IMP_DEFINE0==1)
    mrc     p15, 1, r3, c9, c0, 2   /* imp defined */
    str     r3, [r2], #4
.endif
.if (SAVE_IMP_DEFINE1==1)
    mrc     p15, 0, r3, c15, c0, 1
    str     r3, [r2], #4
.endif

    /* neon */
    adrl    r2, fastboot_save_asimde_FPSCR
    mrc   p15,0,r7,c1,c0,2
    orr   r3, r7, #0x00F00000
    bic   r3, r3, #0xC0000000
    mcr   p15,0,r3,c1,c0,2
    VFPFMRX  r6, FPEXC
    orr   r3, r6, #0x40000000
    VFPFMXR  FPEXC, r3
    VFPFMRX  r3, FPSCR
    str   r3, [r2], #4
    str   r6, [r2], #4
    VFPFMXR  FPEXC, r6
    mcr   p15,0,r7,c1,c0,2
    /* end neon*/

    ldmfd   sp!,{r0-r7,pc}


ENTRY(mstar_fastboot_restore_first)
    bl ensure_environment
    mov r0, #'K'
    bl PUTCHAR
    adrl    r2, fastboot_save_mmu_ctr
    ldmia   r2!, {r0,r3,r4}    /* r0: mmu ctrl */
    ldr     r1, =fastboot_restore1_ret
	mcr	    p15, 0, r4, c3, c0, 0		@ load domain access register
	mcr	    p15, 0, r3, c2, c0, 0		@ load page table pointer
    ldr     r3, [r2], #4
	mcr     p15,0,r3,c1,c0,1        /* ACTLR*/
    ldr     r3, [r2], #4
    mcr     p15,0,r3,c1,c0,2        /* CPACR*/
    ldr     r3, [r2], #4
    mcr     p15, 0, r3, c2, c0, 1   /* TTBR 1 */
    ldr     r3, [r2], #4
    mcr     p15, 0, r3, c2, c0, 2   /* TTBCR */
    ldr     r3, [r2], #4
    mcr     p15, 0, r3, c10, c2, 0   /* PRRR */
    ldr     r3, [r2], #4
    mcr     p15, 0, r3, c10, c2, 1   /* NMRR */
    ldr     r3, [r2], #4
    mcr     p15,0,r3,c13,c0,0        /* FCSEIDR */
    ldr     r3, [r2], #4
    mcr     p15,0,r3,c13,c0,1        /* CTXIDR */
    ldr     r3, [r2], #4
    mcr     p15,0,r3,c13,c0,2        /* TPIDRURW */
    ldr     r3, [r2], #4
    mcr     p15,0,r3,c13,c0,3        /* TPIDRURO */
    ldr     r3, [r2], #4
    mcr     p15,0,r3,c13,c0,4        /* TPIDRPRW */
    ldr     r3, [r2], #4
    mcr	    p15,0,r3,c12,c0,0	     /* VBAR */
.if (SAVE_IMP_DEFINE0)
    ldr     r3, [r2], #4
    mcr     p15, 1, r3, c9, c0, 2   /* imp defined */
.endif
.if (SAVE_IMP_DEFINE1)
    ldr     r3, [r2], #4
    mcr     p15, 0, r3, c15, c0, 1
.endif

    /* neon */
    adrl    r2, fastboot_save_asimde_FPSCR
    mrc   p15,0,r7,c1,c0,2
    orr   r3, r7, #0x00F00000
    bic   r3, r3, #0xC0000000
    mcr   p15,0,r3,c1,c0,2
    VFPFMRX  r6, FPEXC
    orr   r3, r6, #0x40000000
    VFPFMXR  FPEXC, r3
    ldr   r3, [r2], #4
    VFPFMXR  FPSCR, r3
    ldr   r6, [r2], #4
    VFPFMXR  FPEXC, r6
    mcr   p15,0,r7,c1,c0,2
    /* end neon*/

	b	MMU_ctrl

ENTRY(swsusp_arch_suspend)
    adr r0, fastboot_savedlr
    str lr, [r0]
    bl  mstar_fastboot_save
    bl  mstar_fastboot_save_last
    adr r0, fastboot_savedlr
    ldr lr, [r0]
    b   swsusp_save


ENTRY(swsusp_arch_resume)
    bl  mstar_fastboot_restore_first
fastboot_restore1_ret:
    mov r0, #'L'
    bl  PUTCHAR_VIRT
    bl  mstar_fastboot_restore
    bl  fastboot_reset_onresume
    adr r0, fastboot_savedlr
    ldr lr, [r0]
    eor r0, r0, r0
    mov pc, lr

ENTRY(save_processor_state)
    eor r0, r0, r0
    mov pc, lr

ENTRY(restore_processor_state)
    eor r0, r0, r0
    mov pc, lr

ENTRY(pfn_is_nosave)
    eor r0, r0, r0
    mov pc, lr