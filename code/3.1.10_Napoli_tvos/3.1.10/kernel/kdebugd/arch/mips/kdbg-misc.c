

#include <linux/mm.h>
#include <linux/pfn.h>
#include <linux/io.h>
#include <kdebugd.h>

inline int kdbg_chk_pfn_valid(unsigned long pfn)
{
	if (pfn < max_mapnr)
		return 1;
	else
		return -ENXIO;
}

inline int kdbg_chk_vaddr_valid(void *kaddr)
{
	if (kdbg_chk_pfn_valid(PFN_DOWN(virt_to_phys(kaddr))))
		return 1;
	else
		return -ENXIO;
}

int kdbg_elf_chk_machine_type(Elf32_Ehdr Ehdr)
{
	if (Ehdr.e_machine == EM_MIPS_RS3_LE || Ehdr.e_machine == EM_MIPS)
		return 0;
	else
		return -ENXIO;
}
