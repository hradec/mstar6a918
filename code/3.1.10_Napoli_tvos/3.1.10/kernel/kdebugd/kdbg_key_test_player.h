

#ifndef _LINUX_KDBG_KEY_TEST_PLAYER_H
#define _LINUX_KDBG_KEY_TEST_PLAYER_H

#include "kdebugd.h"
/* Start the Key test Player thread - read from the file */
int kdbg_start_key_test_player_thread(void);
/* Stop the Key test Player thread - read from the file */
int kdbg_stop_key_test_player_thread(void);
/*
 * Key Test Player  Module init function, which initialize Key Test Player Module and start functions
 * and allocateKey Test Player module.
 */
int kdbg_key_test_player_init(void);

#endif /* !_LINUX_KDBG_ELF_SYM_API_H */
