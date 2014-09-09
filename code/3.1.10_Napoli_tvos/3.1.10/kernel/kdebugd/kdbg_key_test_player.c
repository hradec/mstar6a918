

#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <asm/uaccess.h>

#include "kdbg_key_test_player.h"
#include "kdbg_elf_sym_debug.h"

/* Enable debug prints */
#define SYM_DEBUG_ON  0

/* Key Test Player Toggle Event Key */
#define KDBG_KEY_TEST_PLAYER_TOGGLE_KEY "987"
#define KDBG_DELAY_TO_START ((CONFIG_LIFETEST_START_SEC)*1000)
#define KDBG_KEY_TEST_PLAYER_FILE_NAME  "/mtd_rwarea/KDebugdLifeTest"
#define KDBG_MAX_KEY_DELAY_LEN 64

static struct task_struct *kdbg_key_test_player_task;
static int kdbg_key_test_player_thread_running;

void kdbg_reg_sys_event_handler(int (*psys_event_handler) (debugd_event_t *));

/* Parse the Key and delay from the file */
int kdbg_read_line(struct file *fp, char *buf, int buf_len)
{
	char ch = 0;
	int i = 0;
	int ret = 0;

	ret = fp->f_op->read(fp, &ch, 1, &fp->f_pos);
	if (ret != 1) {
		*buf = 0;
		return ret;
	}

	while (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') {
		ret = fp->f_op->read(fp, &ch, 1, &fp->f_pos);

		if (ret != 1) {
			*buf = 0;
			return ret;
		}
	}

	while (ch == '#') {
		do {
			ret = fp->f_op->read(fp, &ch, 1, &fp->f_pos);
			if (ret != 1) {
				*buf = 0;
				return ret;
			}
		} while (ch != '\n');

		while (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') {
			ret = fp->f_op->read(fp, &ch, 1, &fp->f_pos);
			if (ret != 1) {
				*buf = 0;
				return ret;
			}
		}
	}

	while (ch != ' ' && ch != '\n' && ch != '\r' && ch != '\t') {
		buf[i++] = ch;
		ret = fp->f_op->read(fp, &ch, 1, &fp->f_pos);
		if (ret != 1)
			break;
	}

	while (ch != '\n') {
		ret = fp->f_op->read(fp, &ch, 1, &fp->f_pos);
		if (ret != 1)
			break;
	}

	buf[i] = '\0';
	return i;
}

/* Key Test Player Thread, which continously read the Keys/delay from the file */
int kdbg_key_test_player_thread(void *arg)
{

	static struct file *key_filp;	/* File pointer to read file */
	static char buf[16];
	int value = 0;
	int ret = 0;
	debugd_event_t event;
	char *ptr = NULL;

	static int first_time = 1;
	mm_segment_t oldfs = get_fs();

	sym_printk("enter\n");

	if (first_time && !kdebugd_running) {
		msleep(KDBG_DELAY_TO_START);
		first_time = 0;
		if (!kdebugd_running) {
			kdebugd_start();
			kdebugd_running = 1;
		} else {
			/* If kdebugd is running, don't start AutoTest */
			return 0;
		}
	}

	set_fs(KERNEL_DS);

	sym_printk("####### Starting Autotest thread #######\n");
	kdbg_key_test_player_thread_running = 1;

	key_filp =
		filp_open(KDBG_KEY_TEST_PLAYER_FILE_NAME, O_RDONLY | O_LARGEFILE,
				0);
	if (IS_ERR(key_filp) || (key_filp == NULL)) {
		PRINT_KD("error opening file OR file not found %s\n",
				KDBG_KEY_TEST_PLAYER_FILE_NAME);
		ret = -1;
		goto auto_out;
	}

	if (key_filp->f_op->read == NULL) {
		PRINT_KD("read not allowed\n");
		ret = -1;
		goto auto_out;
	}

	while (!kthread_should_stop()) {

		ptr = NULL;
		buf[0] = '\0';

		ret = kdbg_read_line(key_filp, buf, KDBG_MAX_KEY_DELAY_LEN);

		if (ret < 0) {
			PRINT_KD("error in reading the file\n");
			ret = -1;
			goto auto_out;
		}

		if (ret == 0) {
			PRINT_KD("reached at EOF\n");

			PRINT_KD("going to repeat\n");
			key_filp->f_pos = 0;
			ret =
			    kdbg_read_line(key_filp, buf,
					KDBG_MAX_KEY_DELAY_LEN);
			if (ret <= 0) {
				PRINT_KD(" ERROR:in reading line=%d\n",
						__LINE__);
				goto auto_out;

			}
		}

		value = simple_strtoul(buf, NULL, 0);

		sym_printk("######### sleeping for %d\n", value);
		msleep(value);
		ret = kdbg_read_line(key_filp, buf, KDBG_MAX_KEY_DELAY_LEN);

		if (ret <= 0) {
			PRINT_KD("ERROR:: in reading the value of scan\n");
			goto auto_out;
		}

		/* remove leading and trailing whitespaces */
		ptr = strstrip(buf);

		/* create a event */
		strncpy(event.input_string, ptr,
				sizeof(event.input_string) - 1);
		event.input_string[sizeof(event.input_string) - 1] = '\0';

		sym_printk("######  Adding event - %s\n", __FUNCTION__,
				__LINE__, event.input_string);
		queue_add_event(&kdebugd_queue, &event);
	}
auto_out:
	if (!IS_ERR(key_filp))
		filp_close(key_filp, NULL);
	set_fs(oldfs);

	kdbg_key_test_player_thread_running = 0;

	PRINT_KD("##### Exiting autotest thread######\n");
	return ret;
}

/* Key test player stop handler */
int kdbg_key_test_stop_handler(debugd_event_t *event)
{
	if (!strcmp(event->input_string, KDBG_KEY_TEST_PLAYER_TOGGLE_KEY)) {
		kdbg_stop_key_test_player_thread();
		return 0;
	} else
		return 1;
}

/* Start the Key test Player thread - read from the file */
int kdbg_start_key_test_player_thread(void)
{
	int ret = 0;

	PRINT_KD("#####  Starting key test player thread #####\n");
	kdbg_reg_sys_event_handler(kdbg_key_test_stop_handler);
	kdbg_key_test_player_task =
	    kthread_create((void *)kdbg_key_test_player_thread, NULL,
			   "Autotest Thread");
	if (IS_ERR(kdbg_key_test_player_task)) {
		ret = PTR_ERR(kdbg_key_test_player_task);
		kdbg_key_test_player_task = NULL;
		return ret;
	}

	kdbg_key_test_player_task->flags |= PF_NOFREEZE;
	wake_up_process(kdbg_key_test_player_task);

	return ret;
}

/* Stop the Key test Player thread - read from the file */
int kdbg_stop_key_test_player_thread(void)
{
	if (kdbg_key_test_player_thread_running) {
		kdbg_reg_sys_event_handler(NULL);
		PRINT_KD("#####  Stoping key test player thread #####\n");
		kthread_stop(kdbg_key_test_player_task);
		kdbg_key_test_player_task = NULL;
		kdbg_key_test_player_thread_running = 0;
	}
	return 0;
}

/*
FUNCTION NAME	 	:	kdbg_key_test_player_kdmenu
DESCRIPTION			:	main entry routine for the Key Test Player
ARGUMENTS			:	option , File Name
RETURN VALUE	 	:	0 for success
AUTHOR			 	:	Gaurav Jindal
 **********************************************/
int kdbg_key_test_player_kdmenu(void)
{
	int operation = 0;
	int ret = 1;
	do {
		if (ret) {
			PRINT_KD("\n");
			PRINT_KD("Options are:\n");
			PRINT_KD
			    ("------------------------------------------------"
			     "--------------------\n");
			PRINT_KD(" 1. Start Key Test Player\n");
			PRINT_KD(" 2. Stop Key Test Player\n");

			PRINT_KD
			    ("------------------------------------------------"
			     "--------------------\n");
			PRINT_KD(" 99 Key Test Player: Exit Menu\n");
			PRINT_KD
			    ("------------------------------------------------"
			     "--------------------\n");
			PRINT_KD("[Key Test Player] Option ==>  ");
		}

		operation = debugd_get_event_as_numeric(NULL, NULL);
		PRINT_KD("\n");

		switch (operation) {
		case 1:
			if (!kdbg_key_test_player_thread_running) {
				/* Start the thread */
				kdbg_start_key_test_player_thread();
			} else
				PRINT_KD("Key Test Player alreay started\n");
			break;
		case 2:
			if (kdbg_key_test_player_thread_running) {
				/* Stop the thread */
				kdbg_stop_key_test_player_thread();
			} else
				PRINT_KD("Key Test Player not started\n");
			break;
		case 99:
			/* Key Test Player Menu Exit */
			break;

		default:
			PRINT_KD("Key Test Player invalid option....\n");
			ret = 1;	/* to show menu */
			break;
		}
	} while (operation != 99);

	PRINT_KD("Key Test Player menu exit....\n");
	/* as this return value is mean to show or not show the kdebugd menu options */
	return ret;
}

/*
 * Key Test Player  Module init function, which initialize Key Test Player Module and start functions
 * and allocateKey Test Player module.
 */
int kdbg_key_test_player_init(void)
{
	/* Kdbg Key Test Player menu options */
	kdbg_register("KEY DBG: Key Test Player",
		      kdbg_key_test_player_kdmenu, NULL,
		      KDBG_MENU_KEY_TEST_PLAYER);

	return 0;
}
