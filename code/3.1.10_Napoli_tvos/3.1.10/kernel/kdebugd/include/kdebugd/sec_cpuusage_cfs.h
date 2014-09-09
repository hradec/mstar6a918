

#ifndef __LINUX_CPUUSAGE_H__
#define __LINUX_CPUUSAGE_H__

#define USAGE_BUF_SIZE 4096
#define USAGE_TEMP_SIZE 128
#define CPU_USAGE_TICK  HZ
#define CPU_USAGE_TASK_CNT      150


void cpu_usage_display(int cpu);


#if 0
static unsigned int cpu_usage_tick_cnt[CONFIG_NR_CPUS] = {0,};
static unsigned int cpu_usage_idle_cnt[CONFIG_NR_CPUS] = {0,};
static unsigned int sq_idx[CONFIG_NR_CPUS] = {0,};
static unsigned int ctx_queue_idx[CONFIG_NR_CPUS] = {0,};

static unsigned int ctx_switch_cnt[CONFIG_NR_CPUS] = {0,};
static unsigned int cpu_usage_cnt[CONFIG_NR_CPUS] = {0,};
#endif

struct cpu_usage_param{
	unsigned int cpu_usage_tick_cnt;
	unsigned int cpu_usage_idle_cnt;
	unsigned int sq_idx;
	unsigned int ctx_queue_idx;
	unsigned int ctx_switch_cnt;
	unsigned int cpu_usage_cnt;
};



/* ALERT : DO NOT change member variable sequence */
/* With sort(), cmpint function checks 1st member fo sched_debugd_info */
struct sched_debug_info{
	unsigned int tick_cnt;
	int pid;
};

#endif /*__LINUX_CPUUSAGE_H__*/

