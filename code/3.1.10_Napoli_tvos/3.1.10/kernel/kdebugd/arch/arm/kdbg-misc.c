

#include <kdebugd.h>
#include <mstar/mpatch_macro.h>

#if (MP_DEBUG_TOOL_RM == 1)
/*
 * kdbg-misc.c
 *
 * Copyright (C) 2009 Mstar Semiconductor
 *
 * NOTE:
 *
 */
#endif

void kdbg_unwind_mem_stack_kernel(const char *str, unsigned long bottom,
				  unsigned long top)
{
	unsigned long p = bottom & ~31;
	int i;

	PRINT_KD("%s\n", str);

	for (p = bottom & ~31; p <= top;) {

		for (i = 0; i < 8; i++, p += 4) {

			if (p < bottom || p > top)
				PRINT_KD("         ");
			else
				PRINT_KD("%08lx ", *(unsigned long *)p);
		}
		PRINT_KD("\n");
	}
}

int kdbg_elf_chk_machine_type(Elf32_Ehdr Ehdr)
{
	if (Ehdr.e_machine == EM_ARM)
		return 0;
	else
		return -ENXIO;
}
