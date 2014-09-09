/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 1995 Waldorf Electronics
 * Written by Ralf Baechle and Andreas Busse
 * Copyright (C) 1995 - 1999 Ralf Baechle
 * Copyright (C) 1996 Paul M. Antoine
 * Modified for DECStation and hence R3000 support by Paul M. Antoine
 * Further modifications by David S. Miller and Harald Koerfgen
 * Copyright (C) 1999 Silicon Graphics, Inc.
 */

#include <asm/asm.h>
#include <asm/regdef.h>


#if (CONFIG_MP_PLATFORM_MIPS == 1)	
# ================================================================
# Dean add
# ================================================================
M_DebugCOUNTDM      = (0x1  << 25)
INITIAL_STATUS      = 0x10400000
INITIAL_CONFIG0     = 0x00000002 #Uncached

S_Config1IL         = 19
M_Config1IL         = (0x7  << S_Config1IL)
S_Config1IS         = 22
M_Config1IS         = (0x7  << S_Config1IS)
S_Config1IA         = 16
M_Config1IA         = (0x7  << S_Config1IA)

S_Config1DL         = 10
M_Config1DL         = (0x7  << S_Config1DL)
S_Config1DS         = 13
M_Config1DS         = (0x7  << S_Config1DS)
S_Config1DA         = 7
M_Config1DA         = (0x7  << S_Config1DA)

Index_Store_Tag_I   = 0x8
Index_Store_Tag_D   = 0x9
A_KSEG0BASE         = 0x80000000

M_ConfigK0          = (0x7  << 0)
## #ifdef CONFIG_MSTAR_EUCLID
## K_CacheAttrCN       = (0x2  << 0)
## #else
K_CacheAttrCN       = (0x3  << 0) #Cacheable, noncoherent, write-back, write allocate.
## #endif

.macro WREG reg, value
lui     $8, \reg >> 16
li      $9, \value
sw      $9, (\reg & 0xFFFF)($8)
.endm

#-------------------------------------------------------------------------------------------------
# BOOT_InitReg
# @param  None
# @return None
# @note   Init CPU Registers
#-------------------------------------------------------------------------------------------------
#BOOT_InitReg:
.macro BOOT_InitReg

    # INIT DEBUG
    mfc0    $8, $23
    and     $8, (~M_DebugCOUNTDM)
    mtc0    $8, $23

    # CLEAR COUNT
    mtc0    $0, $9

    # INIT STATUS
    li      $8, INITIAL_STATUS
    mtc0    $8, $12

    # INIT CAUSE
    mtc0    $0, $13

    # INIT CONFIG
	li	    $8, INITIAL_CONFIG0
	mtc0	$8, $16

#    jr      $31
#    nop
.endm

#-------------------------------------------------------------------------------------------------
# BOOT_InitCache
# @param  None
# @return None
# @note   Init I-cache and D-cache
#-------------------------------------------------------------------------------------------------
# BOOT_InitCache:
.macro BOOT_InitCache

    # Clear TagLo & TagHi
    mtc0    $0, $28
    mtc0    $0, $29

    # Config1
    mfc0    $8, $16, 1

    ### INIT I-CACHE ###

    # Line Size $10
    li      $9, M_Config1IL
    and     $9, $8, $9
    srl     $9, $9, S_Config1IL
    beqz    $9, Icache_done    # no Icache
    nop
    li      $10, 2
    sll     $10, $10, $9

    # Sets per way $11
    li      $9, M_Config1IS
    and     $9, $8, $9
    srl     $9, $9, S_Config1IS
    li      $11, 64
    sll     $11, $11, $9

    # Associativity $12
    li      $9, M_Config1IA
    and     $9, $8, $9
    srl     $9, $9, S_Config1IA
    add     $12, $9, 1

    li      $14, A_KSEG0BASE
    move    $15, $11

Icache_loop:
    # Invalidate each tag entry
    cache   Index_Store_Tag_I, 0($14)
    add     $14, $10
    sub     $15, 1
    bnez    $15, Icache_loop
    nop
    move    $15, $11
    sub     $12, 1
    bnez    $12, Icache_loop
    nop

Icache_done:


    ### INIT D-CACHE ###

    # Line Size $10
    li      $9, M_Config1DL
    and     $9, $8, $9
    srl     $9, $9, S_Config1DL
    beqz    $9, Dcache_done    # no Dcache
    nop
    li      $10, 2
    sll     $10, $10, $9

    # Sets per way $11
    li      $9, M_Config1DS
    and     $9, $8, $9
    srl     $9, $9, S_Config1DS
    li      $11, 64
    sll     $11, $11, $9

    # Associativity $12
    li      $9, M_Config1DA
    and     $9, $8, $9
    srl     $9, $9, S_Config1DA
    add     $12, $9, 1

    li      $14, A_KSEG0BASE
    move    $15, $11

Dcache_loop:
    # Invalidate each tag entry
    cache   Index_Store_Tag_D, 0($14)
    add     $14, $10
    sub     $15, 1
    bnez    $15, Dcache_loop
    nop
    move    $15, $11
    sub     $12, 1
    bnez    $12, Dcache_loop
    nop

Dcache_done:

    # Enable cache
    mfc0    $8, $16
    and     $8, (~M_ConfigK0)
## #ifdef CONFIG_MSTAR_EUCLID
##     addi    $8, $8, (K_CacheAttrCN)
## #else
    li      $8, (K_CacheAttrCN)
## #endif
    mtc0    $8, $16

#    jr      $31
#    nop
.endm
#endif // CONFIG_MP_PLATFORM_MIPS

	.set noreorder
	.cprestore
	LEAF(start)
start:
#if (CONFIG_MP_PLATFORM_MIPS == 1)
    /* Cache Initial */

	BOOT_InitReg
	nop

	# Initialize Cache
	BOOT_InitCache
	nop
#endif
	/* Save boot rom start args */
	move	s0, a0
	move	s1, a1
	move	s2, a2
	move	s3, a3

	/* Clear BSS */
	PTR_LA	a0, _edata
	PTR_LA	a2, _end
1:	sw	zero, 0(a0)
	bne	a2, a0, 1b
	 addiu	a0, a0, 4

	PTR_LA	a0, (.heap)          /* heap address */
	PTR_LA  sp, (.stack + 8192)  /* stack address */

	PTR_LA	ra, 2f

	PTR_LA	k0, decompress_kernel
	jr	k0
	 nop

2:
	move	a0, s0
	move	a1, s1
	move	a2, s2
	move	a3, s3
	PTR_LI	k0, KERNEL_ENTRY
	jr	k0
	 nop
3:
	b	3b
	 nop
	END(start)

	.comm .heap,BOOT_HEAP_SIZE,4
	.comm .stack,4096*2,4
