

#ifndef _KDBG_ELF_DWARF_H_
#define _KDBG_ELF_DWARF_H_

#include "kdbg_elf_def.h"
#include "kdbg_util.h"

int kdbg_elf_read_debug_line_table(kdbg_elf_usb_elf_list_item *plist);

/*
 *  Search the table made in read table ..
 * //TODO: write more comments
 * */
int kdbg_elf_dbg_line_search_table(kdbg_elf_usb_elf_list_item *plist,
				   unsigned long addr,
				   struct aop_df_info *pdf_info);

void kdbg_elf_delete_line_info_table(struct line_table_info *table_info);
#endif /* _KDBG_ELF_DWARF_H_ */
