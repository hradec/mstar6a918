#ifndef _KDBG_TRACE_H_
#define _KDBG_TRACE_H_

#define KDEBUGD_BT_INVALID_VAL 0xffffffff

/* placeholder for the arch registers */
struct kdbgd_bt_regs {
	unsigned int fp;	/* current frame pointer */
	unsigned int pc;	/* current instruction pointer */
	unsigned int lr;	/* current return address */
	unsigned int sp;	/* current stack pointer */
	unsigned int sp_end;	/* The limit of stack address */
};

/*
 * get_user_value
 * Get the address and copy/read the data/instruction from user stack
 */
static inline unsigned int get_user_value(struct mm_struct *bt_mm,
					  unsigned long addr)
{
	unsigned int retval = KDEBUGD_BT_INVALID_VAL;

	if (bt_mm == NULL)
		return retval;

	if (!find_vma(bt_mm, addr)) {
		PRINT_KD("[ALERT] find_vma failed, addr %08lx\n", addr);
		return retval;
	}
	__get_user(retval, (unsigned long *)addr);
	return retval;
}

#endif /* _KDBG_TRACE_H_ */
