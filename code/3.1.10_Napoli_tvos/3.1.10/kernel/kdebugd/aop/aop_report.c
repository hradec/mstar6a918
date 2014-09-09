

#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/oprofile.h>

#include <kdebugd.h>
#include "aop_report.h"
#include "aop_oprofile.h"
#include "aop_kernel.h"
#include "../drivers/oprofile/event_buffer.h"
#include "aop_report_symbol.h"

#ifdef	CONFIG_ELF_MODULE
#include "kdbg_elf_sym_api.h"
#endif /* CONFIG_ELF_MODULE */

#include "aop_debug.h"

/*for chelsea architecture and linux kernel 2.6.18*/
size_t kernel_pointer_size = 4;

/*position from which the value is to be read from the raw data buffer*/
static unsigned long aop_read_buf_pos;

/*samples which are remaining to get processed. Its value is cached with
write position of the buffer at the time of initialization of raw data buffer*/
static int aop_samples_remaining;

/*
 * Transient values used for parsing the raw data buffer aop_cache.
The relevant values are updated when each buffer entry is processed.
 */
static struct op_data aop_trans_data;

/*The samples are processed and the data specific to application which
are run are extracted and collected in this structure. aop_app_list_head is
the head of node of the linked list*/
aop_image_list *aop_app_list_head;

/*The samples are processed and the data specific to libraries which
are referred are extracted and collected in this structure.  aop_lib_list_head is the
head of node of the linked list*/
static aop_image_list *aop_lib_list_head;

/*The samples are processed and the data specific to tgid which
are run are extracted and collected in this structure. aop_tgid_list_head is
the head of node of the linked list*/
static aop_pid_list *aop_tgid_list_head;

/*The samples are processed and the data specific to pid which
are referred are extracted and collected in this structure.  aop_tid_list_head is the
head of node of the linked list*/
static aop_pid_list *aop_tid_list_head;

/*count of the total user samples collected on
processing the buffer aop_cache*/
static unsigned long aop_nr_user_samples;

/*total samples = user+kernel*/
unsigned long aop_nr_total_samples;

/*Thread name for the pid which is not registered in the kernel in task_struct*/
static const char *AOP_IDLE_TH_NAME = "Idle Task";

/*The samples are processed and the data specific to pid which
are referred are extracted and collected in this structure.  aop_dead_list_head is the
head of node of the linked list at the time of mortuary*/
aop_dead_list *aop_dead_list_head;

/*enum for denoting the type of image to be
decoded nd get the symbol information.*/
enum {
	IMAGE_TYPE_APPLICATION,
	IMAGE_TYPE_LIBRARY
};

/*sort the linked list by sample count.
Algorithm used is selection sort.
This sorts the application and library based linked list*/
static void aop_sort_image_list(int type, int cpu)
{
	struct aop_image_list *a = NULL;
	struct aop_image_list *b = NULL;
	struct aop_image_list *c = NULL;
	struct aop_image_list *d = NULL;
	struct aop_image_list *tmp = NULL;
	struct aop_image_list *head = NULL;
	int sample_count_a = 0;
	int sample_count_b = 0;

	BUG_ON(cpu < 0 || cpu > NR_CPUS);

	switch (type) {
	case AOP_TYPE_APP:
		head = aop_app_list_head;
		break;
	case AOP_TYPE_LIB:
		head = aop_lib_list_head;
		break;
	default:
		PRINT_KD("\n");
		PRINT_KD("wrong type(%d)\n", type);
		return;
	}

	a = c = head;

	while (a->next != NULL) {
		d = b = a->next;
		while (b != NULL) {
			/* sample_count_a = a->samples_count[cpu] */
			if (cpu < NR_CPUS) {
				sample_count_a = a->samples_count[cpu];
				sample_count_b = b->samples_count[cpu];
			} else {
				sample_count_a =
				    COUNT_SAMPLES(a, samples_count, NR_CPUS);
				sample_count_b =
				    COUNT_SAMPLES(b, samples_count, NR_CPUS);
			}

			if (sample_count_a < sample_count_b) {
				/* neighboring linked list node */
				if (a->next == b) {
					if (a == head) {
						a->next = b->next;
						b->next = a;
						tmp = a;
						a = b;
						b = tmp;
						head = a;
						c = a;
						d = b;
						b = b->next;
					} else {
						a->next = b->next;
						b->next = a;
						c->next = b;
						tmp = a;
						a = b;
						b = tmp;
						d = b;
						b = b->next;
					}
				} else {
					if (a == head) {
						tmp = b->next;
						b->next = a->next;
						a->next = tmp;
						d->next = a;
						tmp = a;
						a = b;
						b = tmp;
						d = b;
						b = b->next;
						head = a;
					} else {
						tmp = b->next;
						b->next = a->next;
						a->next = tmp;
						c->next = b;
						d->next = a;
						tmp = a;
						a = b;
						b = tmp;
						d = b;
						b = b->next;
					}
				}
			} else {
				d = b;
				b = b->next;
			}
		}
		c = a;
		a = a->next;
	}
	switch (type) {
	case AOP_TYPE_APP:
		aop_app_list_head = head;
		break;
	case AOP_TYPE_LIB:
		aop_lib_list_head = head;
		break;
	default:
		PRINT_KD("\nwrong type(%d)\n", type);
		return;
	}
}

/*sort the linked list by sample count.
Algorithm used is selection sort.
This sorts the TID and IGID based linked list*/
static void aop_sort_pid_list(int type, int cpu)
{
	struct aop_pid_list *a = NULL;
	struct aop_pid_list *b = NULL;
	struct aop_pid_list *c = NULL;
	struct aop_pid_list *d = NULL;
	struct aop_pid_list *tmp = NULL;
	struct aop_pid_list *head = NULL;
	int sample_count_a = 0;
	int sample_count_b = 0;

	BUG_ON(cpu < 0 || cpu > NR_CPUS);

	switch (type) {
	case AOP_TYPE_TGID:
		head = aop_tgid_list_head;
		break;
	case AOP_TYPE_TID:
		head = aop_tid_list_head;
		break;
	default:
		PRINT_KD("\n");
		PRINT_KD("wrong type(%d)\n", type);
		return;
	}

	a = c = head;

	while (a->next != NULL) {
		d = b = a->next;
		while (b != NULL) {
			if (cpu < NR_CPUS) {
				sample_count_a = a->samples_count[cpu];
				sample_count_b = b->samples_count[cpu];
			} else {
				sample_count_a =
				    COUNT_SAMPLES(a, samples_count, NR_CPUS);
				sample_count_b =
				    COUNT_SAMPLES(b, samples_count, NR_CPUS);
			}

			if (sample_count_a < sample_count_b) {
				/* neighboring linked list node */
				if (a->next == b) {
					if (a == head) {
						a->next = b->next;
						b->next = a;
						tmp = a;
						a = b;
						b = tmp;
						head = a;
						c = a;
						d = b;
						b = b->next;
					} else {
						a->next = b->next;
						b->next = a;
						c->next = b;
						tmp = a;
						a = b;
						b = tmp;
						d = b;
						b = b->next;
					}
				} else {
					if (a == head) {
						tmp = b->next;
						b->next = a->next;
						a->next = tmp;
						d->next = a;
						tmp = a;
						a = b;
						b = tmp;
						d = b;
						b = b->next;
						head = a;
					} else {
						tmp = b->next;
						b->next = a->next;
						a->next = tmp;
						c->next = b;
						d->next = a;
						tmp = a;
						a = b;
						b = tmp;
						d = b;
						b = b->next;
					}
				}
			} else {
				d = b;
				b = b->next;
			}
		}
		c = a;
		a = a->next;
	}
	switch (type) {
	case AOP_TYPE_TGID:
		aop_tgid_list_head = head;
		break;
	case AOP_TYPE_TID:
		aop_tid_list_head = head;
		break;
	default:
		PRINT_KD("\n");
		PRINT_KD("wrong type(%d)\n", type);
		return;
	}
}

/*free all the resources that are taken by the system
while processing the tid and tgid data*/
void aop_free_tgid_tid_resources(void)
{
	aop_pid_list *tmp_tgid_data = aop_tgid_list_head;
	aop_pid_list *tmp_pid_data = aop_tid_list_head;

	/*free the TGID data */
	while (tmp_tgid_data) {
		aop_tgid_list_head = tmp_tgid_data->next;
		KDBG_MEM_DBG_KFREE(tmp_tgid_data->thread_name);
		KDBG_MEM_DBG_KFREE(tmp_tgid_data);
		tmp_tgid_data = aop_tgid_list_head;
	}
	/*free the PID data */
	while (tmp_pid_data) {
		aop_tid_list_head = tmp_pid_data->next;
		KDBG_MEM_DBG_KFREE(tmp_pid_data->thread_name);
		KDBG_MEM_DBG_KFREE(tmp_pid_data);
		tmp_pid_data = aop_tid_list_head;
	}
	aop_tgid_list_head = NULL;
	aop_tid_list_head = NULL;
}

/*free all the resources taken by the system while processing the data*/
void aop_free_resources(void)
{
	aop_image_list *tmp_app_data = aop_app_list_head;
	aop_image_list *tmp_lib_data = aop_lib_list_head;

	aop_read_buf_pos = 0;
	aop_samples_remaining = 0;
	memset(&aop_trans_data, 0, sizeof(struct op_data));

	aop_free_kernel_data();	/*free the kernel data */

	aop_sym_report_free_sample_data();	/* free the sym data */

	/*free the application data */
	while (tmp_app_data) {
		aop_app_list_head = tmp_app_data->next;
		KDBG_MEM_DBG_KFREE(tmp_app_data);
		tmp_app_data = aop_app_list_head;
	}
	/*free the library data */
	while (tmp_lib_data) {
		aop_lib_list_head = tmp_lib_data->next;
		KDBG_MEM_DBG_KFREE(tmp_lib_data);
		tmp_lib_data = aop_lib_list_head;
	}

	/*free up the buffer that is taken by the system
	   when collecting and processing by tid and tgid */
	aop_free_tgid_tid_resources();

	aop_app_list_head = NULL;
	aop_lib_list_head = NULL;

	aop_nr_user_samples = 0;
	aop_nr_total_samples = 0;
}

/*initializes the data and structures before processing*/
static void aop_init_processing(void)
{
	/*caching the write offset before processing.
	   Otherwise the race conditions may occur. */
	aop_samples_remaining = aop_cache.wr_offset / AOP_ULONG_SIZE;

	aop_printk("%s:Data entries to process aop_samples_remaining=%d\n",
		   __FUNCTION__, aop_samples_remaining);
}

/*decode the cookie into the path name of application and libraries.*/
static char *aop_decode_cookie(aop_cookie_t cookie, char *buf, size_t buf_size)
{
	/*call the function to decode the cookie value into directory PATH */
	if (buf)
		aop_sys_lookup_dcookie(cookie, buf, buf_size);
	return buf;
}

/*Delete the nodes in the linked list whose sample count is zero.*/
static void aop_clean_tgid_list(void)
{
	aop_pid_list *tmp_tgid_data = NULL;
	aop_pid_list *tmp = NULL;

	/* If no sample recieved, head is not allocated.
	 * Check and return*/
	if (!aop_tgid_list_head) {
		PRINT_KD ("\n");
		PRINT_KD ("TGID List Empty !!!\n");
		return;
	}

	while (!COUNT_SAMPLES(aop_tgid_list_head, samples_count, NR_CPUS)) {
		tmp = aop_tgid_list_head->next;
		KDBG_MEM_DBG_KFREE(aop_tgid_list_head->thread_name);
		KDBG_MEM_DBG_KFREE(aop_tgid_list_head);
		aop_tgid_list_head = tmp;
		if (!aop_tgid_list_head) {
			PRINT_KD("\n");
			PRINT_KD("TGID List Empty !!!\n");
			return;
		}
	}

	tmp = aop_tgid_list_head;
	tmp_tgid_data = tmp->next;

	while (tmp_tgid_data) {
		if (!COUNT_SAMPLES(tmp_tgid_data, samples_count, NR_CPUS)) {
			tmp->next = tmp_tgid_data->next;
			KDBG_MEM_DBG_KFREE(tmp_tgid_data->thread_name);
			KDBG_MEM_DBG_KFREE(tmp_tgid_data);
			tmp_tgid_data = tmp->next;
		} else {
			tmp = tmp->next;
			tmp_tgid_data = tmp->next;
		}
	}
}

/*Delete the nodes in the linked list whose sample count is zero.*/
static void aop_clean_tid_list(void)
{
	aop_pid_list *tmp_tid_data = NULL;
	aop_pid_list *tmp = NULL;

	/* If no sample recieved, head is not allocated.
	 * Check and return*/
	if (!aop_tid_list_head) {
		PRINT_KD ("\n");
		PRINT_KD ("TID List Empty !!!\n");
		return;
	}

	while (!COUNT_SAMPLES(aop_tid_list_head, samples_count, NR_CPUS)) {
		tmp = aop_tid_list_head->next;
		KDBG_MEM_DBG_KFREE(aop_tid_list_head->thread_name);
		KDBG_MEM_DBG_KFREE(aop_tid_list_head);
		aop_tid_list_head = tmp;
		if (!aop_tid_list_head) {
			PRINT_KD("\n");
			PRINT_KD("TID List Empty!!!\n");
			return;
		}
	}

	tmp = aop_tid_list_head;
	tmp_tid_data = tmp->next;
	while (tmp_tid_data) {
		if (!COUNT_SAMPLES(tmp_tid_data, samples_count, NR_CPUS)) {
			tmp->next = tmp_tid_data->next;

			KDBG_MEM_DBG_KFREE(tmp_tid_data->thread_name);
			KDBG_MEM_DBG_KFREE(tmp_tid_data);
			tmp_tid_data = tmp->next;
		} else {
			tmp = tmp->next;
			tmp_tid_data = tmp->next;
		}
	}
}

/* Get the process name for given pid/tgid */
static void aop_get_comm_name(int flag, pid_t pid, char *t_name)
{
	struct task_struct *tsk = NULL;

	if (!t_name)
		return;

	/*Take RCU read lock register can be changed */
	rcu_read_lock();
	/* Refer to kernel/pid.c
	 *  init_pid_ns has bee initialized @ void __init pidmap_init(void)
	 *  PID-map pages start out as NULL, they get allocated upon
	 *  first use and are never deallocated. This way a low pid_max
	 *  value does not cause lots of bitmaps to be allocated, but
	 *  the scheme scales to up to 4 million PIDs, runtime.
	 */
	tsk = find_task_by_pid_ns(pid, &init_pid_ns);

	if (tsk)
		get_task_struct(tsk);

	rcu_read_unlock();

	if (tsk) {
		/*Unlock */
		task_lock(tsk);

		/* Namit: taking lock is safe in printk ?? */
		aop_printk("Given %s %d, TID %d TGID %d comm name %s\n",
			   (flag) ? "tgid" : "pid", pid, tsk->pid, tsk->tgid,
			   tsk->comm);

		strlcpy(t_name, tsk->comm, TASK_COMM_LEN);
		task_unlock(tsk);
		put_task_struct(tsk);

	} else if (pid) {
		/*This is for the thread which are created and died
		   between the time the sampling stops and processing is done */
		strncpy(t_name, "---", 4);
	} else {
		/*This is for idle task having pid = 0 */
		strncpy(t_name, AOP_IDLE_TH_NAME, TASK_COMM_LEN);
	}

}

void *aop_create_node(int type)
{
	void *ret_buf = NULL;
	aop_pid_list *tmp_tgid_data = NULL;
	aop_pid_list *tmp_tid_data = NULL;
	switch (type) {
	case AOP_TYPE_TGID:
		tmp_tgid_data =
		    (aop_pid_list *)
		    KDBG_MEM_DBG_KMALLOC(KDBG_MEM_REPORT_MODULE,
					 sizeof(aop_pid_list), GFP_KERNEL);
		if (!tmp_tgid_data) {
			aop_errk("tmp_tgid_data: no memory\n");
			return NULL;	/* no memory!! */
		}

		memset(tmp_tgid_data, 0, sizeof(aop_pid_list));

		tmp_tgid_data->thread_name =
		    (char *)KDBG_MEM_DBG_KMALLOC(KDBG_MEM_REPORT_MODULE,
						 TASK_COMM_LEN, GFP_KERNEL);
		if (!tmp_tgid_data->thread_name) {
			aop_errk("tmp_tgid_data->thread_name: no memory\n");
			KDBG_MEM_DBG_KFREE(tmp_tgid_data);
			return NULL;	/* no memory!! */
		}

		ret_buf = (void *)tmp_tgid_data;
		break;
	case AOP_TYPE_TID:
		tmp_tid_data =
		    (aop_pid_list *)
		    KDBG_MEM_DBG_KMALLOC(KDBG_MEM_REPORT_MODULE,
					 sizeof(aop_pid_list), GFP_KERNEL);
		if (!tmp_tid_data) {
			aop_errk("tmp_tid_data: no memory\n");
			return NULL;	/* no memory!! */
		}
		memset(tmp_tid_data, 0, sizeof(aop_pid_list));

		tmp_tid_data->thread_name =
		    (char *)KDBG_MEM_DBG_KMALLOC(KDBG_MEM_REPORT_MODULE,
						 TASK_COMM_LEN, GFP_KERNEL);
		if (!tmp_tid_data->thread_name) {
			aop_errk("tmp_tid_data->thread_name: no memory\n");
			KDBG_MEM_DBG_KFREE(tmp_tid_data);
			return NULL;	/* no memory!! */
		}
		ret_buf = (void *)tmp_tid_data;
		break;
	default:
		aop_errk("Iinvalid Type....\n");
	}
	return ret_buf;
}

/*add the sample to generate the report by TGID(process ID) wise.*/
void aop_add_sample_tgid(void)
{
	aop_pid_list *tmp_tgid_data = NULL;
	aop_pid_list *tgiditem = NULL;

	/*Check if it is the first sample.If it is, create the head node of
	   the tgid link list. */
	if (!aop_tgid_list_head) {
		aop_tgid_list_head =
		    (aop_pid_list *) aop_create_node(AOP_TYPE_TGID);
		if (!aop_tgid_list_head) {
			aop_errk("\nFailed to create TGID...\n");
			return;
		}

		aop_get_comm_name(1, aop_trans_data.tgid,
				  aop_tgid_list_head->thread_name);
		aop_tgid_list_head->tgid = aop_trans_data.tgid;
		aop_tgid_list_head->pid = aop_trans_data.tid;

		aop_tgid_list_head->samples_count[aop_trans_data.cpu] = 1;
		aop_tgid_list_head->next = NULL;
		return;
	}

	tmp_tgid_data = aop_tgid_list_head;

	/*add the sample in tgid link list and increase the count */
	while (1) {
		if (tmp_tgid_data) {
			if (tmp_tgid_data->tgid == aop_trans_data.tgid) {
				/*if match found, increment the count */
				tmp_tgid_data->samples_count[aop_trans_data.
							     cpu]++;
				break;
			}
			if (tmp_tgid_data->next) {
				tmp_tgid_data = tmp_tgid_data->next;
			} else {
				/*create the node for some pids which are not
				   yet registered in link list */
				tgiditem =
				    (aop_pid_list *)
				    aop_create_node(AOP_TYPE_TGID);
				if (!tgiditem) {
					aop_errk
					    ("\nFailed to create TGID...\n");
					return;
				}

				aop_get_comm_name(1, aop_trans_data.tgid,
						  tgiditem->thread_name);
				tgiditem->tgid = aop_trans_data.tgid;
				tgiditem->pid = aop_trans_data.tid;
				tgiditem->samples_count[aop_trans_data.cpu] = 1;
				tgiditem->next = NULL;
				tmp_tgid_data->next = tgiditem;
				break;
			}
		} else {
			PRINT_KD("\n");
			PRINT_KD
			    ("aop_add_sample_tgid:check head of link list\n");
			break;
		}
	}
}

void aop_create_dead_list(struct task_struct *tsk)
{
	aop_dead_list *node = NULL;

	if (!tsk) {
		aop_errk("NULL task\n");
		return;
	}

	node = (aop_dead_list *) KDBG_MEM_DBG_KMALLOC(KDBG_MEM_REPORT_MODULE,
						      sizeof(aop_dead_list),
						      GFP_KERNEL);
	if (!node) {
		aop_errk("aop_dead_list : node: no memory\n");
		return;		/* no memory!! */
	}
	node->thread_name = (char *)KDBG_MEM_DBG_KMALLOC(KDBG_MEM_REPORT_MODULE,
							 TASK_COMM_LEN,
							 GFP_KERNEL);
	if (!node->thread_name) {
		aop_errk("aop_dead_list : node->thread_name: no memory\n");
		KDBG_MEM_DBG_KFREE(node);
		node = NULL;
		return;		/* no memory!! */
	}

	if (!node) {
		aop_errk("\nFailed to create TID...\n");
		return;
	}

	/* Already called from safe point no need to protect */
	strlcpy(node->thread_name, tsk->comm, TASK_COMM_LEN);
	node->pid = tsk->pid;
	node->tgid = tsk->tgid;

	aop_printk("Created pid %d, TID COMM %s\n", node->pid,
		   node->thread_name);

	node->next = aop_dead_list_head;
	aop_dead_list_head = node;
}

EXPORT_SYMBOL(aop_create_dead_list);

/* prepare tgid and Tid list for the report showing */
static void aop_process_dead_list(void)
{
	aop_dead_list *node = NULL;
	aop_dead_list *prev_node = NULL;
	aop_pid_list *tid_list_node = NULL;
	aop_pid_list *tgid_list_node = NULL;
	int i;

	for (node = aop_dead_list_head; node;) {
		tid_list_node = (aop_pid_list *) aop_create_node(AOP_TYPE_TID);
		if (!tid_list_node) {
			aop_errk("\nFailed to create TID...\n");
			return;
		}
		memcpy(tid_list_node->thread_name, node->thread_name,
		       TASK_COMM_LEN);
		aop_printk("Created pid %d, comm %s & TID COMM %s\n",
			   node->pid, tid_list_node->thread_name,
			   node->thread_name);
		tid_list_node->pid = node->pid;
		tid_list_node->tgid = node->tgid;

		for (i = 0; i < NR_CPUS; i++)
			tid_list_node->samples_count[i] = 0;

		tid_list_node->next = aop_tid_list_head;
		aop_tid_list_head = tid_list_node;

		if (node->pid == node->tgid) {
			tgid_list_node =
			    (aop_pid_list *) aop_create_node(AOP_TYPE_TGID);
			if (!tgid_list_node) {
				aop_errk("\nFailed to create TGID...\n");
				return;
			}

			memcpy(tgid_list_node->thread_name, node->thread_name,
			       TASK_COMM_LEN);
			aop_printk("Created tgid %d, comm %s & TID COMM %s\n",
				   node->tgid, tgid_list_node->thread_name,
				   node->thread_name);
			tgid_list_node->tgid = node->tgid;
			for (i = 0; i < NR_CPUS; i++)
				tgid_list_node->samples_count[i] = 0;

			tgid_list_node->next = aop_tgid_list_head;
			aop_tgid_list_head = tgid_list_node;
		}
		prev_node = node;
		node = node->next;
		/*free the mortuary data */
		KDBG_MEM_DBG_KFREE(prev_node->thread_name);
		KDBG_MEM_DBG_KFREE(prev_node);
		prev_node = NULL;
	}
	aop_dead_list_head = NULL;
}

/*add the sample to generate the report by tid(thread ID) wise.*/
static void aop_add_sample_tid(void)
{
	aop_pid_list *tmp_tid_data = NULL;
	aop_pid_list *tiditem = NULL;

	/*Check if it is the first sample.If it is, create the head node of
	   the tid link list. */
	if (!aop_tid_list_head) {
		aop_tid_list_head =
		    (aop_pid_list *) aop_create_node(AOP_TYPE_TID);
		if (!aop_tid_list_head) {
			aop_errk("\nFailed to create TID...\n");
			return;
		}
		aop_get_comm_name(0, aop_trans_data.tid,
				  aop_tid_list_head->thread_name);
		aop_tid_list_head->pid = aop_trans_data.tid;
		aop_tid_list_head->tgid = aop_trans_data.tgid;
		aop_tid_list_head->samples_count[aop_trans_data.cpu] = 1;
		aop_tid_list_head->next = NULL;
		return;
	}

	tmp_tid_data = aop_tid_list_head;

	/*add the sample in tid link list and increase the count */
	while (1) {
		if (tmp_tid_data) {
			if (tmp_tid_data->pid == aop_trans_data.tid) {
				/*if match found, increment the count */
				tmp_tid_data->samples_count[aop_trans_data.
							    cpu]++;
				break;
			}
			if (tmp_tid_data->next) {
				tmp_tid_data = tmp_tid_data->next;
			} else {
				/*create the node for some pids which are not
				   yet registered in link list */
				tiditem =
				    (aop_pid_list *)
				    aop_create_node(AOP_TYPE_TID);
				if (!tiditem) {
					aop_errk("\nFailed to create TID...\n");
					return;
				}

				aop_get_comm_name(0, aop_trans_data.tid,
						  tiditem->thread_name);
				tiditem->pid = aop_trans_data.tid;
				tiditem->tgid = aop_trans_data.tgid;
				tiditem->samples_count[aop_trans_data.cpu] = 1;
				tiditem->next = NULL;
				tmp_tid_data->next = tiditem;
				break;
			}
		} else {
			PRINT_KD("\n");
			PRINT_KD
			    ("aop_add_sample_tid:check head of link list\n");
			break;
		}
	}
}

/*context changes are prefixed by an escape code.
This will return if the code is escape code or not.*/
static inline int aop_is_escape_code(uint64_t code)
{
	return kernel_pointer_size == 4 ? code == ~0LU : code == ~0LLU;
}

/*pop the raw data buffer value*/
static int aop_pop_buffer_value(unsigned long *val)
{
	if (!aop_samples_remaining) {
		aop_errk("BUG: popping empty buffer !\n");
		return -ENOBUFS;
	}
	*val = aop_cache.buffer[aop_read_buf_pos++];
	aop_samples_remaining--;

	return 0;
}

/*returns if size number of elements are in data buffer or not.*/
static int aop_enough_remaining(size_t size)
{
	if (aop_samples_remaining >= size)
		return 1;

	PRINT_KD("%s: Dangling ESCAPE_CODE.\n", __FUNCTION__);
	return 0;
}

/*process the pc and event sample*/
static void aop_put_sample(unsigned long pc)
{
	unsigned long event;
	aop_image_list *tmp_app_data = NULL;
	aop_image_list *tmp_lib_data = NULL;
	aop_image_list *appitem = NULL;
	aop_image_list *libitem = NULL;

	/*before popping the value, check if it avaiable in data buffer */
	if (!aop_enough_remaining(1)) {
		aop_samples_remaining = 0;
		return;
	}

	if (aop_pop_buffer_value(&event) == -ENOBUFS) {
		aop_samples_remaining = 0;
		PRINT_KD("%s, Buffer empty...returning\n", __FUNCTION__);
		return;
	}

	aop_nr_total_samples++;

	/*add the sample for tgid */
	aop_add_sample_tgid();

	/*add the sample for pid */
	aop_add_sample_tid();

	if (aop_trans_data.tracing != AOP_TRACING_ON)
		aop_trans_data.event = event;

	aop_trans_data.pc = pc;

	/* to log symbol wise samples */
	aop_sym_report_update_sample_data(&aop_trans_data);

	WARN_ON(aop_trans_data.cpu >= NR_CPUS);

	if ((aop_trans_data.cpu >= NR_CPUS))
		return;

	/*find the context, whether the sample is for kernel context or user */
	if (aop_trans_data.in_kernel) {
		aop_update_kernel_sample(&aop_trans_data);
	} else {
		aop_nr_user_samples++;

		/*Check if it is the first sample.If it is, create the head nodes of
		   the library and application link list. */
		if (!aop_app_list_head) {
			aop_app_list_head =
			    (aop_image_list *)
			    KDBG_MEM_DBG_KMALLOC(KDBG_MEM_REPORT_MODULE,
						 sizeof(aop_image_list),
						 GFP_KERNEL);
			if (!aop_app_list_head) {
				aop_errk
				    ("aop_image_list: aop_app_list_head: no memory\n");
				aop_nr_user_samples = 0;
				return;	/* no memory!! */
			}

			memset(aop_app_list_head, 0, sizeof(aop_image_list));

			aop_app_list_head->cookie_value =
			    aop_trans_data.app_cookie;
			aop_app_list_head->samples_count[aop_trans_data.cpu] =
			    1;
			aop_app_list_head->next = NULL;
		}

		if (!aop_lib_list_head) {
			aop_lib_list_head =
			    (aop_image_list *)
			    KDBG_MEM_DBG_KMALLOC(KDBG_MEM_REPORT_MODULE,
						 sizeof(aop_image_list),
						 GFP_KERNEL);
			if (!aop_lib_list_head) {
				aop_errk
				    ("aop_image_list: aop_lib_list_head: no memory\n");
				aop_nr_user_samples = 0;
				KDBG_MEM_DBG_KFREE(aop_app_list_head);
				return;	/* no memory!! */
			}
			memset(aop_lib_list_head, 0, sizeof(aop_image_list));

			aop_lib_list_head->cookie_value = aop_trans_data.cookie;
			aop_lib_list_head->samples_count[aop_trans_data.cpu] =
			    1;
			aop_lib_list_head->next = NULL;
			return;
		}

		tmp_app_data = aop_app_list_head;
		tmp_lib_data = aop_lib_list_head;

		/*add the sample in application link list and increase the count */
		while (1) {
			if (tmp_app_data) {
				if (tmp_app_data->cookie_value ==
				    aop_trans_data.app_cookie) {
					/*if match found, increment the count */
					tmp_app_data->
					    samples_count[aop_trans_data.cpu]++;
					break;
				}
				if (tmp_app_data->next) {
					tmp_app_data = tmp_app_data->next;
				} else {
					/*create the node */
					appitem =
					    (aop_image_list *)
					    KDBG_MEM_DBG_KMALLOC
					    (KDBG_MEM_REPORT_MODULE,
					     sizeof(aop_image_list),
					     GFP_KERNEL);
					if (!appitem) {
						aop_errk
						    ("aop_image_list: appitem: no memory\n");
						break;	/* no memory!! */
					}
					memset(appitem, 0,
					       sizeof(aop_image_list));

					appitem->cookie_value =
					    aop_trans_data.app_cookie;
					appitem->samples_count[aop_trans_data.
							       cpu] = 1;
					appitem->next = NULL;
					tmp_app_data->next = appitem;
					break;
				}
			}
		}

		/*add the sample in library link list and increase the count */
		while (1) {
			if (tmp_lib_data) {
				if (tmp_lib_data->cookie_value ==
				    aop_trans_data.cookie) {
					/*if match found, increment the count */
					tmp_lib_data->
					    samples_count[aop_trans_data.cpu]++;
					break;
				}
				if (tmp_lib_data->next) {
					tmp_lib_data = tmp_lib_data->next;
				} else {
					/*create the node */
					libitem =
					    (aop_image_list *)
					    KDBG_MEM_DBG_KMALLOC
					    (KDBG_MEM_REPORT_MODULE,
					     sizeof(aop_image_list),
					     GFP_KERNEL);
					if (!libitem) {
						aop_errk
						    ("aop_image_list: libitem: no memory\n");
						break;	/* no memory!! */
					}
					memset(libitem, 0,
					       sizeof(aop_image_list));

					libitem->cookie_value =
					    aop_trans_data.cookie;
					libitem->samples_count[aop_trans_data.
							       cpu] = 1;
					libitem->next = NULL;
					tmp_lib_data->next = libitem;
					break;
				}
			}
		}
	}
}

static void aop_code_unknown(void)
{
	aop_printk("enter\n");
}

static void aop_code_ctx_switch(void)
{
	unsigned long val;

	aop_printk("enter\n");

	/*This handler would require 5 samples in the data buffer.
	   check if 5 elements exists in buffer. */
	if (!aop_enough_remaining(5)) {
		aop_samples_remaining = 0;
		return;
	}

	if (aop_pop_buffer_value(&val) == -ENOBUFS) {
		aop_samples_remaining = 0;
		PRINT_KD("%s, TID Buffer empty... returning ---\n",
			 __FUNCTION__);
		return;
	}
	aop_trans_data.tid = val;
	aop_printk("tid %d ", aop_trans_data.tid);

	if (aop_pop_buffer_value(&val) == -ENOBUFS) {
		aop_samples_remaining = 0;
		PRINT_KD("%s, APP_COOKIE Buffer empty...returning ---\n",
			 __FUNCTION__);
		return;
	}
	aop_trans_data.app_cookie = val;

	aop_printk("app_cookie %lu ", (unsigned long)aop_trans_data.app_cookie);

	/*
	   must be ESCAPE_CODE, CTX_TGID_CODE, tgid. Like this
	   because tgid was added later in a compatible manner.
	 */
	if (aop_pop_buffer_value(&val) == -ENOBUFS) {
		aop_samples_remaining = 0;
		PRINT_KD("%s, ESCAPE_CODE Buffer empty...returning ---\n",
			 __FUNCTION__);
		return;
	}
	if (aop_pop_buffer_value(&val) == -ENOBUFS) {
		aop_samples_remaining = 0;
		PRINT_KD("%s, CTX_TGID_CODE Buffer empty...returning ---\n",
			 __FUNCTION__);
		return;
	}

	if (aop_pop_buffer_value(&val) == -ENOBUFS) {
		aop_samples_remaining = 0;
		PRINT_KD("%s, TGID Buffer empty...returning ---\n",
			 __FUNCTION__);
		return;
	}
	aop_trans_data.tgid = val;
	aop_printk("tgid %d\n", aop_trans_data.tgid);
}

static void aop_code_cpu_switch(void)
{
	unsigned long val;
	if (!aop_enough_remaining(1)) {
		aop_samples_remaining = 0;
		return;
	}

	if (aop_pop_buffer_value(&val) == -ENOBUFS) {
		aop_samples_remaining = 0;
		PRINT_KD("%s, Buffer empty...returning\n", __FUNCTION__);
		return;
	}
	aop_trans_data.cpu = val;

}

static void aop_code_cookie_switch(void)
{
	unsigned long val;
	aop_printk("enter\n");

	if (!aop_enough_remaining(1)) {
		aop_samples_remaining = 0;
		return;
	}

	if (aop_pop_buffer_value(&val) == -ENOBUFS) {
		aop_samples_remaining = 0;
		PRINT_KD("%s, Buffer empty...returning\n", __FUNCTION__);
		return;
	}
	aop_trans_data.cookie = val;

	aop_printk("cookie 0x%lx\n", (unsigned long)aop_trans_data.cookie);
}

static void aop_code_kernel_enter(void)
{
	aop_printk("enter\n");
	aop_trans_data.in_kernel = 1;
}

static void aop_code_user_enter(void)
{
	aop_printk("enter\n");
	aop_trans_data.in_kernel = 0;
}

static void aop_code_module_loaded(void)
{
	aop_printk("enter\n");
}

static void aop_code_trace_begin(void)
{
	aop_printk("TRACE_BEGIN\n");
	aop_trans_data.tracing = AOP_TRACING_START;
}

/*handlers are registered which are responsible for every type of code*/
aop_handler_t handlers[TRACE_END_CODE + 1] = {
	&aop_code_unknown,
	&aop_code_ctx_switch,
	&aop_code_cpu_switch,
	&aop_code_cookie_switch,
	&aop_code_kernel_enter,
	&aop_code_user_enter,
	&aop_code_module_loaded,
	&aop_code_unknown,
	&aop_code_trace_begin,
};

#if defined(AOP_DEBUG_ON) && (AOP_DEBUG_ON != 0)
void aop_chk_resources(void)
{
	if (aop_app_list_head != NULL)
		aop_errk("aop_app_list_head is not NULL\n");
	if (aop_lib_list_head != NULL)
		aop_errk("aop_lib_list_head is not NULL\n");

	if (aop_nr_user_samples != 0)
		aop_errk("aop_nr_user_samples is not NULL\n");

	if (aop_nr_total_samples != 0)
		aop_errk("aop_nr_total_samples is not NULL\n");

	if (aop_tgid_list_head != NULL)
		aop_errk("aop_tgid_list_head is not NULL\n");

	if (aop_tid_list_head != NULL)
		aop_errk("aop_tid_list_head is not NULL\n");

	if (aop_dead_list_head != NULL)
		aop_errk("aop_dead_list_head is not NULL\n");
}
#endif

/*process all the samples from the buffer*/
int aop_process_all_samples(void)
{
	unsigned long code;
	int count_sample, total_no_of_samples, prev_per = 0;
	int total_escape_code = 0;

	AOP_PRINT_TID_LIST(__FUNCTION__);
	AOP_PRINT_TGID_LIST(__FUNCTION__);

	/*reset all the resiurces taken for processing */
	/* aop_free_resources(); */
	aop_chk_resources();

	/* prepare tgid and Tid list for the report showing */
	aop_process_dead_list();

	/*initialize the data that is required for processing */
	aop_init_processing();

	/* init kernel data with/without vmlinux */
	aop_create_vmlinux();

	/* allocate memory for symbol report */
	if (aop_sym_report_init() != 0) {
		aop_printk("Failed to init symbol info head list\n");
		return 1;
	}

	total_no_of_samples = aop_samples_remaining;

	/*process all the samples one by one. */
	while (aop_samples_remaining) {
		count_sample = total_no_of_samples - aop_samples_remaining;
		if (count_sample) {
			int per = ((count_sample * 100) / total_no_of_samples);
			if (!(per % 10) && (prev_per != per)) {
				prev_per = per;
				PRINT_KD("Processing Samples ...%d%%\r", per);
			}
		}

		if (aop_pop_buffer_value(&code) == -ENOBUFS) {
			aop_samples_remaining = 0;
			PRINT_KD("\n");
			PRINT_KD("%s, Buffer empty...returning\n",
				 __FUNCTION__);
			return -ENOBUFS;
		}

		if (!aop_is_escape_code(code)) {
			aop_put_sample(code);
			continue;
		} else {
			total_escape_code++;
		}

		if (!aop_samples_remaining) {
			PRINT_KD("\n");
			PRINT_KD("%s: Dangling ESCAPE_CODE.\n", __FUNCTION__);
			break;
		}

		/*started with ESCAPE_CODE, next is type */
		if (aop_pop_buffer_value(&code) == -ENOBUFS) {
			aop_samples_remaining = 0;
			PRINT_KD("\n");
			PRINT_KD("%s, Buffer empty...returning\n",
				 __FUNCTION__);
			return -ENOBUFS;
		}

		if (code >= TRACE_END_CODE) {
			PRINT_KD("\n");
			PRINT_KD("%s: Unknown code %lu\n", __FUNCTION__, code);
			continue;
		}

		handlers[code] ();
	}

	/*Delete the nodes in the linked list whose sample count is zero. */
	aop_clean_tgid_list();
	aop_clean_tid_list();

	/* loop will quit before calculate the processing level, so updated here */
	PRINT_KD("Processing Samples ...100%%\n");
	aop_printk("Total Samples processed=%lu\n", aop_nr_total_samples);
	aop_printk("Total user Samples processed=%lu\n", aop_nr_user_samples);
	aop_printk("Total kernel Samples processed=%lu\n",
		   aop_nr_kernel_samples);
	aop_printk("Total escape code =%d\n", total_escape_code);
	aop_printk("Processing Done...\n");

	AOP_PRINT_TID_LIST(__FUNCTION__);
	AOP_PRINT_TGID_LIST(__FUNCTION__);

	return 0;
}

/*Dump the application data*/
int aop_op_generate_app_samples(void)
{
	char *buf;
	size_t buf_size = AOP_MAX_SYM_NAME_LENGTH;
	aop_image_list *tmp_app_data = NULL;
	int perc = 0;
	int index = 1;
	unsigned int choice = 0;
	aop_cookie_t app_cookie = 0;
	int sample_count = 0;

	buf = (char *)KDBG_MEM_DBG_KMALLOC(KDBG_MEM_REPORT_MODULE,
					   buf_size, GFP_KERNEL);
	if (!buf) {
		aop_errk("buf: no memory\n");
		return 0;
	}
	aop_printk("Total user Samples collected (%lu)\n", aop_nr_user_samples);
	PRINT_KD("Total Samples (%lu)\n", aop_nr_total_samples);

	if (aop_nr_user_samples) {
		PRINT_KD("Report Generation For [ALL]\n");
		PRINT_KD("\n");
		PRINT_KD("Index\t  Samples  %%\tApplication Image\n");
		PRINT_KD("----------------------------------------\n");
		if (aop_config_sort_option == AOP_SORT_BY_SAMPLES)
			aop_sort_image_list(AOP_TYPE_APP, NR_CPUS);
		tmp_app_data = aop_app_list_head;
		while (tmp_app_data) {
			sample_count =
			    COUNT_SAMPLES(tmp_app_data, samples_count, NR_CPUS);
			perc = sample_count * 100 / aop_nr_total_samples;
			PRINT_KD("%d\t%8u %3d%%\t%s\n", index, sample_count,
				 perc,
				 aop_decode_cookie(tmp_app_data->cookie_value,
						   buf, buf_size));
			tmp_app_data = tmp_app_data->next;
			++index;
		}
		PRINT_KD("[9999] Exit\n");

		while (1) {
			PRINT_KD("\n");
			PRINT_KD("Select Option (1 to %d & Exit - 9999)==>",
				 index - 1);
			choice = debugd_get_event_as_numeric(NULL, NULL);

			if (choice == 9999) {
				PRINT_KD ("\n");
				break;
			}

			if (choice >= index || choice < 1) {
				PRINT_KD("\n");
				PRINT_KD("Invalid choice\n");
				continue;
			}
			tmp_app_data = aop_app_list_head;
			while ((--choice) != 0)
				tmp_app_data = tmp_app_data->next;

			app_cookie = tmp_app_data->cookie_value;

			/* No need to put  in the continuation of previouse PRINT_KD */
			PRINT_KD("\n");
			PRINT_KD("Symbol profiling for Application %s\n",
				 aop_decode_cookie(tmp_app_data->cookie_value,
						   buf, buf_size));
			aop_sym_report_per_image_user_samples
			    (IMAGE_TYPE_APPLICATION, app_cookie);
		}
		choice = 0;
		index = 1;
	}
	KDBG_MEM_DBG_KFREE(buf);
	return 0;
}

/*Dump the library data*/
int aop_op_generate_lib_samples(void)
{
	char *buf;
	size_t buf_size = AOP_MAX_SYM_NAME_LENGTH;
	aop_image_list *tmp_lib_data = NULL;
	int perc = 0;
	int index = 1;
	unsigned int choice = 0;
	aop_cookie_t lib_cookie = 0;
	int sample_count = 0;

	buf =
	    (char *)KDBG_MEM_DBG_KMALLOC(KDBG_MEM_REPORT_MODULE, buf_size,
					 GFP_KERNEL);
	if (!buf) {
		aop_errk("buf: no memory\n");
		return 0;
	}

	aop_printk("Total user Samples collected (%lu)\n", aop_nr_user_samples);
	PRINT_KD("Total Samples (%lu)\n", aop_nr_total_samples);

	if (aop_nr_user_samples) {
		PRINT_KD("Report Generation For [ALL]\n");
		PRINT_KD("\n");
		PRINT_KD("Index\t  Samples  %%\tLibrary Image\n");
		PRINT_KD("----------------------------------------\n");
		if (aop_config_sort_option == AOP_SORT_BY_SAMPLES)
			aop_sort_image_list(AOP_TYPE_LIB, NR_CPUS);
		tmp_lib_data = aop_lib_list_head;
		while (tmp_lib_data) {
			sample_count =
			    COUNT_SAMPLES(tmp_lib_data, samples_count, NR_CPUS);
			perc = sample_count * 100 / aop_nr_total_samples;
			PRINT_KD("%d\t%8u %3d%%\t%s\n", index, sample_count,
				 perc,
				 aop_decode_cookie(tmp_lib_data->cookie_value,
						   buf, buf_size));
			tmp_lib_data = tmp_lib_data->next;
			++index;
		}
		PRINT_KD("[9999] Exit\n");

		while (1) {
			PRINT_KD("\n");
			PRINT_KD("Select Option (1 to %d & Exit - 9999)==>",
				 index - 1);
			choice = debugd_get_event_as_numeric(NULL, NULL);

			if (choice == 9999) {
				PRINT_KD ("\n");
				break;
			}

			if (choice >= index || choice < 1) {
				PRINT_KD("\n");
				PRINT_KD("Invalid choice\n");
				continue;
			}

			PRINT_KD("Report Generation For [ALL]\n");
			tmp_lib_data = aop_lib_list_head;
			while ((--choice) != 0)
				tmp_lib_data = tmp_lib_data->next;

			lib_cookie = tmp_lib_data->cookie_value;

			/* No need to put  in the continuation of previouse PRINT_KD */
			PRINT_KD("\n");
			PRINT_KD("Symbol profiling for Library %s\n",
				 aop_decode_cookie(tmp_lib_data->cookie_value,
						   buf, buf_size));
			aop_sym_report_per_image_user_samples
			    (IMAGE_TYPE_LIBRARY, lib_cookie);
		}
		choice = 0;
		index = 1;
	}

	KDBG_MEM_DBG_KFREE(buf);
	return 0;
}

/* to prepare img list to show at report all symbol report */
static int aop_report_prepare_img_list(struct list_head *img_list_head)
{
	aop_image_list *tmp_lib_data = aop_lib_list_head;
	int sample_count = 0;

	while (tmp_lib_data) {
		sample_count =
		    COUNT_SAMPLES(tmp_lib_data, samples_count, NR_CPUS);
		if (sample_count) {
			struct aop_report_all_list *img_data;
			img_data =
			    (struct aop_report_all_list *)
			    KDBG_MEM_DBG_KMALLOC(KDBG_MEM_REPORT_MODULE,
						 sizeof(struct
							aop_report_all_list),
						 GFP_KERNEL);
			if (!img_data) {
				aop_errk
				    ("Add image data failed: aop_report_all_list: img_data: no memory\n");
				return 0;	/* no memory!!, at the same time we report other data  */
			}

			/* assign the sample information to  new img data */
			img_data->is_kernel = 0;
			img_data->report_type.cookie_value =
			    tmp_lib_data->cookie_value;
			img_data->samples_count = sample_count;
			list_add_tail(&img_data->report_list, img_list_head);
		}
		tmp_lib_data = tmp_lib_data->next;
	}

	return 0;
}

/*Dump whole data*/
int aop_op_generate_all_samples(void)
{
	char *buf;
	size_t buf_size = AOP_MAX_SYM_NAME_LENGTH;
	struct list_head *all_list_head;
	struct aop_report_all_list *plist;
	struct list_head *pos, *q;

	if (!aop_nr_kernel_samples && !aop_nr_user_samples) {
		PRINT_KD("No Samples found\n");
		return 1;
	}

	all_list_head = (struct list_head *)KDBG_MEM_DBG_KMALLOC(KDBG_MEM_REPORT_MODULE,
						      sizeof(struct list_head), GFP_KERNEL);
	if (!all_list_head) {
		return 1;
	}

	INIT_LIST_HEAD(all_list_head);

	buf =
	    (char *)KDBG_MEM_DBG_KMALLOC(KDBG_MEM_REPORT_MODULE, buf_size,
					 GFP_KERNEL);
	if (!buf) {
		aop_errk("buf: no memory\n");
		KDBG_MEM_DBG_KFREE(all_list_head);
		return 1;
	}

	aop_printk("\nTotal kernel Samples collected (%lu)\n",
		   aop_nr_kernel_samples);
	aop_printk("\nTotal user Samples collected (%lu)\n",
		   aop_nr_user_samples);

	PRINT_KD("Total Samples (%lu)\n", aop_nr_total_samples);
	PRINT_KD("Samples\t  %%\tImage [module] name\n");
	PRINT_KD("-----------------------------------------\n");

	if (aop_nr_kernel_samples)
		aop_kernel_prepare_report(all_list_head, NR_CPUS);

	if (aop_nr_user_samples)
		aop_report_prepare_img_list(all_list_head);

	aop_list_sort(all_list_head, aop_kernel_report_cmp);

	/* print kernel module samples */
	list_for_each_safe(pos, q, all_list_head) {
		/* loop thru all the nodes */
		plist =
		    list_entry(pos, struct aop_report_all_list, report_list);
		if (plist) {
			int perc =
			    ((plist->samples_count * 100) /
			     aop_nr_total_samples);
			PRINT_KD("%8u %3d%%\t%s\n", plist->samples_count, perc,
				 (plist->is_kernel) ? plist->report_type.
				 kernel_name : aop_decode_cookie(plist->
								 report_type.
								 cookie_value,
								 buf,
								 buf_size));

			list_del(pos);

			/* free all report item memory */
			KDBG_MEM_DBG_KFREE(plist);
		}
	}
	PRINT_KD("-----------------------------------------\n");

	KDBG_MEM_DBG_KFREE(all_list_head);
	KDBG_MEM_DBG_KFREE(buf);
	return 1;
}

/*Dump data- TGID wise*/
int aop_op_generate_report_tgid(void)
{
	aop_pid_list *tmp_tgid_data = NULL;
	int perc = 0;
	int sample_count = 0;

	AOP_PRINT_TID_LIST(__FUNCTION__);
	AOP_PRINT_TGID_LIST(__FUNCTION__);

	PRINT_KD("Total Samples (%lu)\n", aop_nr_total_samples);
	if (aop_nr_total_samples) {

		if (aop_config_sort_option == AOP_SORT_BY_SAMPLES)
			aop_sort_pid_list(AOP_TYPE_TGID, NR_CPUS);

		tmp_tgid_data = aop_tgid_list_head;
		PRINT_KD("\n");
		PRINT_KD("Samples\t  %%\tPid\tProcess Name\n");
		PRINT_KD("--------------------------------------------\n");
		while (tmp_tgid_data) {
			sample_count =
			    COUNT_SAMPLES(tmp_tgid_data, samples_count,
					  NR_CPUS);
			perc = sample_count * 100 / aop_nr_total_samples;
			PRINT_KD("%8u %3d%%\t%d\t%.20s\n", sample_count,
				 perc, tmp_tgid_data->tgid,
				 tmp_tgid_data->thread_name);
			tmp_tgid_data = tmp_tgid_data->next;
		}
	}
	return 1;		/* to show the kdebug menu */
}

/* Dump data- TID wise */
int aop_op_generate_report_tid(int cpu_wise)
{
	aop_pid_list *tmp_tid_data = NULL;
	int perc = 0;
	pid_t pid;
	int cpu_option = 0;
	int sample_count = 0;
	int i = 0;
	int cpu_wise_perc[NR_CPUS] = { 0,};
	int cpu_wise_samples[NR_CPUS] = { 0,};

	AOP_PRINT_TID_LIST(__FUNCTION__);
	AOP_PRINT_TGID_LIST(__FUNCTION__);

	PRINT_KD("Total Samples (%lu)\n", aop_nr_total_samples);
	if (aop_nr_total_samples) {
		while (cpu_option != 9999) {
			if (cpu_wise) {

				for (i = 0; i < NR_CPUS; i++) {
					cpu_wise_perc[i] = 0;
					cpu_wise_samples[i] = 0;
				}

				if (aop_config_sort_option ==
				    AOP_SORT_BY_SAMPLES)
					aop_sort_pid_list(AOP_TYPE_TID,
							  cpu_option);

				PRINT_KD(" Samples    %%\tCPU\n");

				tmp_tid_data = aop_tid_list_head;
				while (tmp_tid_data) {
					for (i = 0; i < NR_CPUS; i++) {
						cpu_wise_perc[i] +=
						    tmp_tid_data->
						    samples_count[i] * 100 /
						    aop_nr_total_samples;
						cpu_wise_samples[i] +=
						    tmp_tid_data->
						    samples_count[i];
					}
					tmp_tid_data = tmp_tid_data->next;
				}

				for (i = 0; i < NR_CPUS; i++) {
					PRINT_KD("%8u %3d%%\t%d\n",
						 cpu_wise_samples[i],
						 cpu_wise_perc[i], i);
				}

				PRINT_KD("Select(9999 for Exit)==>  ");
				cpu_option =
				    debugd_get_event_as_numeric(NULL, NULL);

				if (cpu_option < 0
				    || cpu_option > (NR_CPUS - 1)) {
					PRINT_KD("\n");
					PRINT_KD("Invalid choice\n");
					if (cpu_option == 9999)
						break;
					cpu_option = 0;
					continue;
				}
			}

			PRINT_KD("\n");

			if (!cpu_wise)
				cpu_option = NR_CPUS;

			if (cpu_option < NR_CPUS) {
				while (1) {
					PRINT_KD
					    ("Report Generation For [CPU - %d]\n",
					     cpu_option);
					PRINT_KD("\n");
					PRINT_KD
					    (" Samples    %%\tTID\tTGID\tProcess Name\n");
					PRINT_KD
					    ("--------------------------------------------\n");

					if (aop_config_sort_option ==
					    AOP_SORT_BY_SAMPLES)
						aop_sort_pid_list(AOP_TYPE_TID,
								  cpu_option);

					tmp_tid_data = aop_tid_list_head;
					while (tmp_tid_data) {
						if (tmp_tid_data->
						    samples_count[cpu_option]) {
							perc =
							    tmp_tid_data->
							    samples_count
							    [cpu_option] * 100 /
							    aop_nr_total_samples;
							PRINT_KD
							    ("%8u %3d%%\t%d\t%d\t%.20s\n",
							     tmp_tid_data->
							     samples_count
							     [cpu_option], perc,
							     tmp_tid_data->pid,
							     tmp_tid_data->tgid,
							     tmp_tid_data->
							     thread_name);
						}
						tmp_tid_data =
						    tmp_tid_data->next;
					}
					PRINT_KD("\n");
					PRINT_KD
					    ("Enter TID for symbol wise report(9999 for Exit) ==>");
					pid =
					    debugd_get_event_as_numeric(NULL,
									NULL);
					PRINT_KD("\n");
					if (pid == 9999)
						break;

					aop_sym_report_per_tid(pid);
				}
			} else {
				while (1) {
					PRINT_KD
					    ("Report Generation For [ALL]\n");
					PRINT_KD("\n");
					PRINT_KD
					    (" Samples    %%\tTID\tTGID\tProcess Name\n");
					PRINT_KD
					    ("--------------------------------------------\n");

					if (aop_config_sort_option ==
					    AOP_SORT_BY_SAMPLES)
						aop_sort_pid_list(AOP_TYPE_TID,
								  NR_CPUS);

					tmp_tid_data = aop_tid_list_head;
					while (tmp_tid_data) {
						sample_count =
						    COUNT_SAMPLES(tmp_tid_data,
								  samples_count,
								  NR_CPUS);
						perc =
						    sample_count * 100 /
						    aop_nr_total_samples;
						PRINT_KD
						    ("%8u %3d%%\t%d\t%d\t%.20s\n",
						     sample_count, perc,
						     tmp_tid_data->pid,
						     tmp_tid_data->tgid,
						     tmp_tid_data->thread_name);
						tmp_tid_data =
						    tmp_tid_data->next;
					}
					PRINT_KD("\n");
					PRINT_KD
					    ("Enter TID for symbol wise report(9999 for Exit) ==>");
					pid =
					    debugd_get_event_as_numeric(NULL,
									NULL);
					PRINT_KD("\n");
					if (pid == 9999)
						break;

					aop_sym_report_per_tid(pid);
				}
			}

			if (!cpu_wise)
				cpu_option = 9999;
		}
	}
	return 1;		/* to show the kdebug menu */
}

#if defined(AOP_DEBUG_ON) && (AOP_DEBUG_ON != 0)
/*Dump the raw data from buffer aop_cache*/
static int aop_op_dump_all_samples(void)
{
	/*Read from the zeroth entry to maximum filled.
	   Cache the write offset. */
	int tmp_buf_write_pos = aop_cache.wr_offset;
	int count = 0;
	PRINT_KD("\n");
	PRINT_KD("Data available %d\n", tmp_buf_write_pos / AOP_ULONG_SIZE);
	for (count = 0; count < tmp_buf_write_pos / AOP_ULONG_SIZE; count++)
		PRINT_KD("%lu\n", aop_cache.buffer[count]);

	return 0;
}
#endif

/*
  * oprofile report init function
  */
#if defined(AOP_DEBUG_ON) && (AOP_DEBUG_ON != 0)
static int __init aop_opreport_kdebug_init(void)
{
	kdbg_register("PROFILE: process All Samples",
		      aop_process_all_samples, NULL,
		      KDBG_MENU_AOP_PROCESS_ALL_SAMPLES);

	kdbg_register("PROFILE: dump All raw data Samples",
		      aop_op_dump_all_samples, NULL,
		      KDBG_MENU_AOP_DUMP_ALL_SAMPLES);
	return 0;
}

__initcall(aop_opreport_kdebug_init);
#endif

/* collect all the process id and collect elf files belongs to the process
and load the elf database */
int aop_load_elf_db_for_all_samples(void)
{
	char *buf = NULL;
	char *filename = NULL;
	size_t buf_size = AOP_MAX_SYM_NAME_LENGTH;
	aop_image_list *tmp_lib_data = NULL;

	buf =
	    (char *)KDBG_MEM_DBG_KMALLOC(KDBG_MEM_REPORT_MODULE, buf_size,
					 GFP_KERNEL);
	if (!buf) {
		PRINT_KD("buf: no memory\n");
		return 0;
	}

	if (!aop_nr_user_samples) {
		aop_printk(" No Samples\n");
		KDBG_MEM_DBG_KFREE(buf);
		return 0;
	}

	tmp_lib_data = aop_lib_list_head;
	while (tmp_lib_data) {
		filename =
		    aop_decode_cookie(tmp_lib_data->cookie_value, buf,
				      buf_size);

#ifdef CONFIG_ELF_MODULE
		kdbg_elf_load_elf_db_by_elf_file(filename);
#endif /* CONFIG_ELF_MODULE */
		tmp_lib_data = tmp_lib_data->next;
	}

	KDBG_MEM_DBG_KFREE(buf);
	return 0;
}

#if defined(AOP_DEBUG_ON) && (AOP_DEBUG_ON != 0)
void AOP_PRINT_TID_LIST(const char *msg)
{
	aop_pid_list *tmp_tid_data = NULL;
	tmp_tid_data = aop_tid_list_head;

	PRINT_KD("TID LIST: %s\n", msg);
	while (tmp_tid_data) {
		PRINT_KD("pid= 0x%x, tgid= 0x%x, [%s]\n", tmp_tid_data->pid,
			 tmp_tid_data->tgid, tmp_tid_data->thread_name);
		tmp_tid_data = tmp_tid_data->next;
	}
}

void AOP_PRINT_TGID_LIST(const char *msg)
{
	aop_pid_list *tmp_tgid_data = NULL;
	tmp_tgid_data = aop_tgid_list_head;

	PRINT_KD("TGID LIST: %s\n", msg);
	while (tmp_tgid_data) {
		PRINT_KD("pid= 0x%x, tgid= 0x%x, [%s]\n", tmp_tgid_data->pid,
			 tmp_tgid_data->tgid, tmp_tgid_data->thread_name);
		tmp_tgid_data = tmp_tgid_data->next;
	}
}
#endif
