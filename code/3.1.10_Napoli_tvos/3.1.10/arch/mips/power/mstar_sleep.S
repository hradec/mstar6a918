#include <asm/asm-offsets.h>
#include <asm/page.h>
#include <asm/regdef.h>
#include <asm/asm.h>

#include "mstar_hibernate.h"

K_CacheAttrCN       = (0x3  << 0) #Cacheable, noncoherent, write-back, write allocate.
K_UnCache 	    = (0x2  << 0) #UnCache.

M_ConfigK0          = (0x7  << 0)
S_Config1IS         = 22
M_Config1IS         = (0x7  << S_Config1IS)
S_Config1IL         = 19
M_Config1IL         = (0x7  << S_Config1IL)
S_Config1IA         = 16
M_Config1IA         = (0x7  << S_Config1IA)
S_Config1DS         = 13
M_Config1DS         = (0x7  << S_Config1DS)
W_Config1DS		    = 3
S_Config1DL         = 10
W_Config1DL		    = 3
M_Config1DL         = (0x7  << S_Config1DL)
S_Config1DA         = 7
M_Config1DA         = (0x7  << S_Config1DA)

A_KSEG0BASE         = 0x80000000


Index_Load_Tag_I    	= 0x4
Index_Load_Tag_D    	= 0x5
Index_Store_Tag_I   	= 0x8
Index_Store_Tag_D   	= 0x9
Address_Fill_I      	= 0x14
Address_Fetch_lock_I 	= 0x1C
Index_Writeback_Inv_D   = 0x01
Hit_Invalidate_D	= 0x11

MSTAR_SLEEP_MAGIC = 0x4D535452
WAKEUP_ADDR_MASK  =              0x0000FFF0
WAKEUP_FLAG_MASK  =              0x0000000F
WAKEUP_FLAG_INVALID =            0
WAKEUP_FLAG_SLEPT   =            1
WAKEUP_FLAG_WKUP    =            2

.macro swcp0r reg sel dst
    mfc0 t0, $\reg, \sel
    sw   t0, \dst(a0)
.endm

.macro lwcp0r reg sel dst
    lw   t0, \dst(a0)
    mtc0 t0, $\reg, \sel
.endm

.macro swcp0r_c reg sel dst out
    ext  t0, t0, 31, 1
    beq  t0, zero, \out
    nop
    swcp0r \reg, \sel, \dst
.endm
.text
save_magic: .word SAVE_MAGIC
save_gen_reg:
.fill SAVEGEN_LEN/4, 4, 0
save_cp0_reg:
.fill SAVECP0_LEN/4, 4, 0

sleep_wakeup_save_magic:    .long 0
sleep_wakeup_save_addr:     .long 0
sleep_wakeup_save_data:     .long 0, 0
.align 2
SerPrintPhyStk: .fill 256, 4, 0
SerPrintPhyStk_bottom: .long 0,0,0,0
.set push
.set noreorder
/* input a0: reg val; sp: sp */
FEXPORT(__PUTREGVAL)
    addiu sp, -12
    sw    ra, 0(sp)
    sw    s0, 4(sp)
    sw    s1, 8(sp)

    move  s0, a0
    li    s1, 32
30:
    addiu s1, -4
    srl   t0, s0, s1
    andi  t0, t0, 0x0F
    addiu t1, t0, -10
    bgez  t1, 31f
    nop
    b     32f
    addiu a0, t0, '0'
31:
    addiu a0, t0, ('A'-10)
32:
    jal   SerPrintChar
    nop
    bgtz  s1, 30b
    nop

    jal   SerPrintChar      /* new line */
    li    a0, '\r'
    jal   SerPrintChar
    li    a0, '\n'
    lw    s0, 4(sp)
    lw    s1, 8(sp)
    lw    ra, 0(sp)
    jr    ra
    addiu sp, 12
/*------------------------------------------------------------------------------
    Function: sleep_set_wakeup_save_addr_phy

    Description:
        set the phy address where save the wake up addr
    Input: (The arguments were used by caller to input data.)
        a0: phy addr, a1: virt addr
    Output: (The arguments were used by caller to receive data.)
        none
    Return:
        None.
    Remark:
        none
-------------------------------------------------------------------------------*/
FEXPORT(sleep_set_wakeup_save_addr_phy)
    addiu sp, sp, -4
    sw    ra, 0(sp)

    li    t0, (MSTAR_SLEEP_MAGIC)
    la    t1, sleep_wakeup_save_magic
    sw    t0, 0(t1)
    sw    a1, 4(t1)
    lw    t0, 0(a1)
    sw    t0, 8(t1)
    lw    t0, 4(a1)
    sw    t0, 12(t1)
    lui   t0, (WAKEUP_ADDR_MASK)
    and   a0, a0, t0
    srl   a0, a0, 16
    jal   write_wkup_pmu
    nop

    lw    ra, 0(sp)
    addiu sp, sp, 4
    jr    ra
    nop

FEXPORT(sleep_clear_wakeup_save_addr_phy)
    addiu sp, sp, -4
    sw    ra, 0(sp)

    li    t0, (MSTAR_SLEEP_MAGIC)
    la    t1, sleep_wakeup_save_magic
    lw    t2, 0(t1)
    bne   t0, t2, 41f
    nop
    lw    t0, 4(t1)
    beq   t0, zero, 41f
    nop
    bne   t0, a1, 41f
    nop
    lw    t0, 8(t1)
    sw    t0, 0(a1)
    lw    t0, 12(t1)
    sw    t0, 4(a1)
    sw    zero, (t1)
    sw    zero, 4(t1)
    sw    zero, 8(t1)
    sw    zero, 12(t1)
    xor   a0, a0, a0
    jal   write_wkup_pmu
    nop
41:
    jal   asm_flush_d_cache
    nop
    jal   mstar_sleep_cur_cpu_flush
    nop
    lw    ra, 0(sp)
    addiu sp, sp, 4
    jr    ra
    nop

/*------------------------------------------------------------------------------
    Function: sleep_prepare_last

    Description:
        last prepare for sleep,should be called after sleep_set_wakeup_save_addr_phy
    Input: (The arguments were used by caller to input data.)
        a0: wake up phy addr
    Output: (The arguments were used by caller to receive data.)
        none
    Return:
        None.
    Remark:

-------------------------------------------------------------------------------*/
FEXPORT(sleep_prepare_last)
    addiu sp, sp, -4
    sw    ra, 0(sp)
    lw    t1, sleep_wakeup_save_addr
    li    t0, MSTAR_SLEEP_MAGIC
    sw    t0, 0(t1)
    sw    a0, 4(t1)
    jal   asm_flush_d_cache
    nop
    jal      mstar_sleep_cur_cpu_flush
    nop

    /* slept flag */
    jal   read_wkup_pmu
    nop
    li    t0, WAKEUP_FLAG_MASK
    not   t0, t0
    and   a0, v0, t0
    ori   a0, a0, WAKEUP_FLAG_SLEPT
    jal   write_wkup_pmu
    nop

    lw    ra, 0(sp)
    addiu sp, sp, 4
    jr    ra
    nop

FEXPORT(sleep_save_cpu_registers)
.set push
.set noat
    la t0, save_gen_reg
    sw AT, SAVEGEN_OFFSET_AT(t0)
    sw s0, SAVEGEN_OFFSET_S0(t0)
    sw s1, SAVEGEN_OFFSET_S1(t0)
    sw s2, SAVEGEN_OFFSET_S2(t0)
    sw s3, SAVEGEN_OFFSET_S3(t0)
    sw s4, SAVEGEN_OFFSET_S4(t0)
    sw s5, SAVEGEN_OFFSET_S5(t0)
    sw s6, SAVEGEN_OFFSET_S6(t0)
    sw s7, SAVEGEN_OFFSET_S7(t0)
    sw k0, SAVEGEN_OFFSET_K0(t0)
    sw k1, SAVEGEN_OFFSET_K1(t0)
    sw gp, SAVEGEN_OFFSET_GP(t0)
    sw s8, SAVEGEN_OFFSET_S8(t0)
    sw sp, SAVEGEN_OFFSET_SP(t0)
    sw ra, SAVEGEN_OFFSET_RA(t0)
.set pop
    la a0, save_cp0_reg
    bal mstar_save_cp0
    nop
    la t0, save_gen_reg
    lw ra, SAVEGEN_OFFSET_RA(t0)
    jr ra
    nop
# -===========================================-
# function      :   mstar_save_cp0
# parameters    :   a0:save address
# parameters    :   none
# return        :   none
# -===========================================-
FEXPORT(mstar_save_cp0)
    swcp0r 0, 0, SAVECP0_OFFSET_IR
    swcp0r 2, 0, SAVECP0_OFFSET_EntryLo0
    swcp0r 3, 0, SAVECP0_OFFSET_EntryLo1
    swcp0r 4, 0, SAVECP0_OFFSET_CTX
    swcp0r 5, 0, SAVECP0_OFFSET_PageMask
    swcp0r 5, 1, SAVECP0_OFFSET_PageGrain
    swcp0r 6, 0, SAVECP0_OFFSET_Wired
    swcp0r 7, 0, SAVECP0_OFFSET_HWREna
    swcp0r 9, 0, SAVECP0_OFFSET_Count
    swcp0r 10, 0, SAVECP0_OFFSET_EntryHi
    swcp0r 11, 0, SAVECP0_OFFSET_Compare
    swcp0r 12, 0, SAVECP0_OFFSET_Status
    swcp0r 12, 1, SAVECP0_OFFSET_IntCtrl
    swcp0r 12, 2, SAVECP0_OFFSET_SRSCtl
    swcp0r 12, 3, SAVECP0_OFFSET_SRSMap
    swcp0r 13, 0, SAVECP0_OFFSET_Cause
    swcp0r 14, 0, SAVECP0_OFFSET_EPC
    swcp0r 15, 1, SAVECP0_OFFSET_EBase

    swcp0r 16, 0, SAVECP0_OFFSET_CFG0
    addiu   t1, zero, 1
    swcp0r_c 16, 1, SAVECP0_OFFSET_CFG1, cfg_end
    addiu   t1, zero, 2
    swcp0r_c 16, 2, SAVECP0_OFFSET_CFG2, cfg_end
    addiu   t1, zero, 3
    swcp0r_c 16, 3, SAVECP0_OFFSET_CFG3, cfg_end
    addiu   t1, zero, 4
cfg_end:
    sw     t1, SAVECP0_OFFSET_CFGCnt(a0)

    swcp0r 30, 0, SAVECP0_OFFSET_EEPC
    swcp0r 21, 0, SAVECP0_OFFSET_FrmMsk

    xor    t1, t1, t1
    lw t0, SAVECP0_OFFSET_CFG1(a0)
    andi t0, t0, 0x08
    beq  t0, zero, wch_end
    nop
    swcp0r 18, 0, SAVECP0_OFFSET_WchL0
    swcp0r 19, 0, SAVECP0_OFFSET_WchH0
    addiu   t1, zero, 1
    swcp0r_c 18, 1, SAVECP0_OFFSET_WchL1, wch_end
    swcp0r 19, 1, SAVECP0_OFFSET_WchH1
    addiu   t1, zero, 2
    swcp0r_c 18, 2, SAVECP0_OFFSET_WchL2, wch_end
    swcp0r 19, 2, SAVECP0_OFFSET_WchH2
    addiu   t1, zero, 3
    swcp0r_c 18, 3, SAVECP0_OFFSET_WchL3, wch_end
    swcp0r 19, 3, SAVECP0_OFFSET_WchH3
    addiu   t1, zero, 4
wch_end:
    sw     t1, SAVECP0_OFFSET_WchCnt(a0)
    swcp0r 23, 0, SAVECP0_OFFSET_DbgCnt

    jr ra
    nop

# -===========================================-
# function      :   msfb_invalid_tlb
# parameters    :   a0: Maximum TLB index
# parameters    :   none
# return        :   none
# -===========================================-
FEXPORT(msfb_invalid_tlb)
    mtc0 zero, $2, 0  # EntryLo0
    mtc0 zero, $3, 0  # EntryLo1
    mtc0 zero, $5, 0  # PageMask

    lui t0, 0x8000
10:
    mtc0 t0, $10, 0  # EntryHi
    ehb
    tlbp
    ehb
    mfc0 t1, $0, 0  # Index
    bgez t1, 10b    # duplicate
    addiu t0, (1<<13)

    mtc0 a0, $0, 0  # Index
    ehb
    tlbwi
    bne a0, zero, 10b
    addiu a0, -1

    mtc0 zero, $0, 0  # Index
    mtc0 zero, $10, 0  # EntryHi
    jr ra
    nop

# -===========================================-
# function      :   asm_invalid_icache
# parameters    :   none
# return        :   none
# -===========================================-
asm_invalid_icache:
    # Clear TagLo & TagHi
    mtc0    zero, $28
    mtc0    zero, $29

    # Config1
    mfc0    t0, $16, 1

    # Line Size t2
    li      t1, M_Config1IL
    and     t1, t0, t1
    srl     t1, t1, S_Config1IL
    beqz    t1, Icache_done    # no Icache
    nop
    li      t2, 2
    sll     t2, t2, t1

    # Sets per way t3
    li      t1, M_Config1IS
    and     t1, t0, t1
    srl     t1, t1, S_Config1IS
    li      t3, 64
    sll     t3, t3, t1

    # Associativity t4
    li      t1, M_Config1IA
    and     t1, t0, t1
    srl     t1, t1, S_Config1IA
    add     t4, t1, 1

    li      t6, A_KSEG0BASE
    move    t7, t3

Icache_loop:
    # Invalidate each tag entry
    cache   Index_Store_Tag_I, 0(t6)
    ehb
    add     t6, t2
    sub     t7, 1
    bnez    t7, Icache_loop
    nop
    move    t7, t3
    sub     t4, 1
    bnez    t4, Icache_loop
    nop

Icache_done:
    jr      ra
    nop


# -===========================================-
# function      :   asm_flush_d_cache
# parameters    :   none
# return        :   none
# -===========================================-
asm_flush_d_cache:

    # Config1
    mfc0    t0, $16, 1

    # Line Size t2
    li      t1, M_Config1DL
    and     t1, t0, t1
    srl     t1, t1, S_Config1DL
    beqz    t1, flush_Dcache_done    # no Dcache
    nop
    add     t1, t1, 1
    li      t2, 1
    sll     t2, t2, t1

    # Sets per way t3
    li      t1, M_Config1DS
    and     t1, t0, t1
    srl     t1, t1, S_Config1DS
    li      t3, 64
    sll     t3, t3, t1

    # Associativity t4
    li      t1, M_Config1DA
    and     t1, t0, t1
    srl     t1, t1, S_Config1DA
    li      t4, 0
    add     t4, t1, 1

    li      t6, A_KSEG0BASE
    move    t7, t3

flush_Dcache_loop:
    # Invalidate each tag entry
    cache   Index_Writeback_Inv_D, 0(t6)
    ehb
    add     t6, t2
    sub     t7, 1
    bnez    t7, flush_Dcache_loop
    nop
    move    t7, t3
    sub     t4, 1
    bnez    t4, flush_Dcache_loop
    nop

flush_Dcache_done:
    jr      ra
    nop



asm_invalid_d_cache:
    # Clear TagLo & TagHi
    mtc0    zero, $28
    mtc0    zero, $29

    # Config1
    mfc0    t0, $16, 1

	# Line Size t2
    li      t1, M_Config1DL
    and     t1, t0, t1
    srl     t1, t1, S_Config1DL
    beqz    t1, Dcache_done    # no Dcache
    nop
    li      t2, 2
    sll     t2, t2, t1

    # Sets per way t3
    li      t1, M_Config1DS
    and     t1, t0, t1
    srl     t1, t1, S_Config1DS
    li      t3, 64
    sll     t3, t3, t1

    # Associativity t4
    li      t1, M_Config1DA
    and     t1, t0, t1
    srl     t1, t1, S_Config1DA
    add     t4, t1, 1

    li      t6, A_KSEG0BASE
    move    t7, t3

Dcache_loop:
    # Invalidate each tag entry
    cache   Index_Store_Tag_D, 0(t6)
    ehb
    add     t6, t2
    sub     t7, 1
    bnez    t7, Dcache_loop
    nop
    move    t7, t3
    sub     t4, 1
    bnez    t4, Dcache_loop
    nop
Dcache_done:
    jr          ra
    nop
FEXPORT(ensure_environment)
    la sp, SerPrintPhyStk_bottom
    jr ra
    nop
FEXPORT(sleep_wakeup_first)
    move    s0, ra
    # should be in kernel mode
    mfc0    t0, $12, 0   # status
    or      t0, t0,  0x1000001f
    xor     t0, t0,  0x0000001f  # disable interrupt
    mtc0    t0, $12, 0

    mfc0    t0, $16, 0   # config
    and     t0, t0,  ~0x07
    or      t0, t0,  0x02  # disable cache
    mtc0    t0, $16, 0

    mfc0    a0, $16, 1
    srl     a0, a0,  25
    and     a0, a0,  0x3F
    addiu   a0, a0,  1
    bal     msfb_invalid_tlb
    nop

    bal     asm_invalid_icache
    nop

    bal     asm_invalid_d_cache
    nop

    move ra, s0
    jr ra
    nop

FEXPORT(sleep_restore_cpu_registers)
    # restore cp0
    la     a0, save_cp0_reg
    lwcp0r 0, 0, SAVECP0_OFFSET_IR
    lwcp0r 2, 0, SAVECP0_OFFSET_EntryLo0
    lwcp0r 3, 0, SAVECP0_OFFSET_EntryLo1
    lwcp0r 4, 0, SAVECP0_OFFSET_CTX
    lwcp0r 5, 0, SAVECP0_OFFSET_PageMask
    lwcp0r 5, 1, SAVECP0_OFFSET_PageGrain
    lwcp0r 6, 0, SAVECP0_OFFSET_Wired
    lwcp0r 7, 0, SAVECP0_OFFSET_HWREna
    lwcp0r 9, 0, SAVECP0_OFFSET_Count
    lwcp0r 10, 0, SAVECP0_OFFSET_EntryHi
    lwcp0r 11, 0, SAVECP0_OFFSET_Compare
    lwcp0r 12, 1, SAVECP0_OFFSET_IntCtrl
    lwcp0r 12, 2, SAVECP0_OFFSET_SRSCtl
    lwcp0r 12, 3, SAVECP0_OFFSET_SRSMap
    lwcp0r 14, 0, SAVECP0_OFFSET_EPC
    lwcp0r 15, 1, SAVECP0_OFFSET_EBase
    lwcp0r 23, 0, SAVECP0_OFFSET_DbgCnt
    lwcp0r 30, 0, SAVECP0_OFFSET_EEPC
    lwcp0r 21, 0, SAVECP0_OFFSET_FrmMsk

    la     t2, wch_rst_end
    la     t1, wch_rst_start
    subu   t2, t2, t1
    srl    t2, t2, 2
    lw     t1, SAVECP0_OFFSET_WchCnt(a0)
    multu  t2, t1
    la     t1, wch_rst_end
    mflo   t2
    subu   t1, t1, t2
    jr     t1
    mfhi   t2
wch_rst_start:
    lw     t0, SAVECP0_OFFSET_WchH3(a0)
    mtc0   t0, $19, 3
    lw     t0, SAVECP0_OFFSET_WchL3(a0)
    mtc0   t0, $18, 3
    lw     t0, SAVECP0_OFFSET_WchH2(a0)
    mtc0   t0, $19, 2
    lw     t0, SAVECP0_OFFSET_WchL2(a0)
    mtc0   t0, $18, 2
    lw     t0, SAVECP0_OFFSET_WchH1(a0)
    mtc0   t0, $19, 1
    lw     t0, SAVECP0_OFFSET_WchL1(a0)
    mtc0   t0, $18, 1
    lw     t0, SAVECP0_OFFSET_WchH0(a0)
    mtc0   t0, $19, 0
    lw     t0, SAVECP0_OFFSET_WchL0(a0)
    mtc0   t0, $18, 0
wch_rst_end:
    la     t2, cfg_rst_end
    la     t1, cfg_rst_start
    subu   t2, t2, t1
    srl    t2, t2, 2
    lw     t1, SAVECP0_OFFSET_CFGCnt(a0)
    multu  t2, t1
    la     t1, cfg_rst_end
    mflo   t2
    subu   t1, t1, t2
    jr     t1
    mfhi   t2
cfg_rst_start:
    lw     t0, SAVECP0_OFFSET_CFG3(a0)
    mtc0   t0, $16, 3
    lw     t0, SAVECP0_OFFSET_CFG2(a0)
    mtc0   t0, $16, 2
    lw     t0, SAVECP0_OFFSET_CFG1(a0)
    mtc0   t0, $16, 1
    lw     t0, SAVECP0_OFFSET_CFG0(a0)
    mtc0   t0, $16, 0
cfg_rst_end:
    lwcp0r 13, 0, SAVECP0_OFFSET_Cause
    lwcp0r 12, 0, SAVECP0_OFFSET_Status
    # restore gprs
.set push
.set noat
    la t0, save_gen_reg
    lw s0, SAVEGEN_OFFSET_S0(t0)
    lw s1, SAVEGEN_OFFSET_S1(t0)
    lw s2, SAVEGEN_OFFSET_S2(t0)
    lw s3, SAVEGEN_OFFSET_S3(t0)
    lw s4, SAVEGEN_OFFSET_S4(t0)
    lw s5, SAVEGEN_OFFSET_S5(t0)
    lw s6, SAVEGEN_OFFSET_S6(t0)
    lw s7, SAVEGEN_OFFSET_S7(t0)
    lw k0, SAVEGEN_OFFSET_K0(t0)
    lw k1, SAVEGEN_OFFSET_K1(t0)
    lw gp, SAVEGEN_OFFSET_GP(t0)
    lw s8, SAVEGEN_OFFSET_S8(t0)
    lw sp, SAVEGEN_OFFSET_SP(t0)
    #lw ra, SAVEGEN_OFFSET_RA(t0)
    lw AT, SAVEGEN_OFFSET_AT(t0)
.set pop
    xor v0, v0, v0
    jr ra
    nop

.set pop

