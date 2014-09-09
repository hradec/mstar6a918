

#ifndef __KDBG_ARCH_WRAPPER_H_
#define __KDBG_ARCH_WRAPPER_H_

/********************************************************************
  INCLUDE FILES
 ********************************************************************/
#include <kdebugd.h>

inline void show_pid_maps_wr(struct task_struct *tsk)
{
	dump_pid_maps(tsk);
}

inline void show_regs_wr(struct pt_regs *regs)
{
	show_regs(regs);
}

#endif /* !__KDBG_ARCH_WRAPPER_H_ */
