#include <linux/fs.h>
#include <linux/hugetlb.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/mmzone.h>
#include <linux/proc_fs.h>
#include <linux/quicklist.h>
#include <linux/seq_file.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
//#include <linux/atomic.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/ctype.h>

#include <asm/setup.h>
//#include "internal.h"
#include <asm/cacheflush.h>
#include <linux/delay.h>

#include <include/mstar/mstar_chip.h>
#include <linux/spinlock.h>

#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/smp.h>
#include <linux/io.h>

#include "./include/mach/hardware.h"
#include "./include/mach/platform.h"
#include <asm/mach-types.h>
#include <asm/hardware/icst.h>

extern void change_interval(unsigned int old_freq, unsigned int new_freq);


static atomic_t proc_is_open = ATOMIC_INIT(0);
static DEFINE_SPINLOCK(set_freq_lock);

static struct cpufreq_driver integrator_driver;

#define MAX_DMSG_WRITE_BUFFER	64

#define CM_ID  	IO_ADDRESS(INTEGRATOR_HDR_ID)
#define CM_OSC	IO_ADDRESS(INTEGRATOR_HDR_OSC)
#define CM_STAT IO_ADDRESS(INTEGRATOR_HDR_STAT)
#define CM_LOCK IO_ADDRESS(INTEGRATOR_HDR_LOCK)

int current_freq;

static const struct icst_params lclk_params = {
	.ref		= 24000000,
	.vco_max	= ICST525_VCO_MAX_5V,
	.vco_min	= ICST525_VCO_MIN,
	.vd_min		= 8,
	.vd_max		= 132,
	.rd_min		= 24,
	.rd_max		= 24,
	.s2div		= icst525_s2div,
	.idx2s		= icst525_idx2s,
};

static const struct icst_params cclk_params = {
	.ref		= 24000000,
	.vco_max	= ICST525_VCO_MAX_5V,
	.vco_min	= ICST525_VCO_MIN,
	.vd_min		= 12,
	.vd_max		= 160,
	.rd_min		= 24,
	.rd_max		= 24,
	.s2div		= icst525_s2div,
	.idx2s		= icst525_idx2s,
};

int get_mboot_freq(void)
{
	u32 reg_l, reg_h; 
    unsigned long freq = 0;
	reg_l = REG(REG_FREQ_LOW);
	reg_h =(REG(REG_FREQ_HIGH)) << 16;
	freq = FREQ_PARAMETER *1000 / (reg_h + reg_l);
	freq = freq *1000;
	return freq;
}	

void set_freq(int freq)
{
  unsigned long reg_l, reg_h, temp, temp1;
	unsigned long flags;
	
	temp = FREQ_PARAMETER*1000;
  reg_l = (temp / freq);
  reg_l = reg_l & 0x0000FFFF;
  reg_h = (temp / freq);
  reg_h = reg_h & 0xFFFF0000;
  reg_h = reg_h >> 16;  
  temp1 = freq*1000;
  spin_lock_irqsave(&set_freq_lock, flags);

  if (temp1  > current_freq)
  	{
  		printk("\n current_freq = %d\n",current_freq);
  		printk("\n low to high \n"); 
  		REG(REG_freq_high_bound_l) = reg_l;
  		REG(REG_freq_high_bound_h) = reg_h;
  		REG(REG_0x110C_B0) = 0x0001; //switch to LPF control
  		REG(REG_0x110C_AA) = 0x0007;
  		REG(REG_0x110C_AE) = 0x0008;
  		REG(REG_0x110C_B2) = 0x1330; //low to high
  		REG(REG_0x110C_A8) = 0x0000;
  		REG(REG_0x110C_A8) = 0x0001;
  		printk("\n new freq setting\n"); 		 		
  	}
  else if (temp1 < current_freq)
  	{
  		printk("\n current_freq = %d\n",current_freq);
  		printk("\n high to low \n");
  		REG(REG_freq_low_bound_l) = reg_l;
  		REG(REG_freq_low_bound_h) = reg_h;
  		REG(REG_0x110C_B0) = 0x0001; //switch to LPF control
  		REG(REG_0x110C_AA) = 0x0007;
  		REG(REG_0x110C_AE) = 0x0008;
  		REG(REG_0x110C_B2) = 0x0330; //high to low 
  		REG(REG_0x110C_A8) = 0x0000;
  		REG(REG_0x110C_A8) = 0x0001; 	
  		printk("\n new freq setting\n"); 	
  	}                       

	spin_unlock_irqrestore(&set_freq_lock, flags);
}

/*
 * Validate the speed policy.
 */
static int integrator_verify_policy(struct cpufreq_policy *policy)
{
	struct icst_vco vco;
	cpufreq_verify_within_limits(policy, 
				     policy->cpuinfo.min_freq, 
				     policy->cpuinfo.max_freq);

	vco = icst_hz_to_vco(&cclk_params, policy->max * 1000);
	policy->max = icst_hz(&cclk_params, vco) / 1000;
	vco = icst_hz_to_vco(&cclk_params, policy->min * 1000);
	policy->min = icst_hz(&cclk_params, vco) / 1000;

	cpufreq_verify_within_limits(policy, 
				     policy->cpuinfo.min_freq, 
				     policy->cpuinfo.max_freq);

	return 0;
}


static int integrator_set_target(struct cpufreq_policy *policy,
				 unsigned int target_freq,
				 unsigned int relation)
{
	cpumask_t cpus_allowed;
	int cpu = policy->cpu;
	struct icst_vco vco;
	struct cpufreq_freqs freqs;
	u_int cm_osc;

	/*
	 * Save this threads cpus_allowed mask.
	 */
	cpus_allowed = current->cpus_allowed;

	/*
	 * Bind to the specified CPU.  When this call returns,
	 * we should be running on the right CPU.
	 */
	set_cpus_allowed(current, cpumask_of_cpu(cpu));
	BUG_ON(cpu != smp_processor_id());

	/* get current setting */
	/*cm_osc = __raw_readl(CM_OSC);

	if (machine_is_integrator()) {
		vco.s = (cm_osc >> 8) & 7;
	} else if (machine_is_cintegrator()) {
		vco.s = 1;
	}
	vco.v = cm_osc & 255;
	vco.r = 22;
	freqs.old = icst_hz(&cclk_params, vco) / 1000; */
  freqs.old = get_mboot_freq(); //  johnson
  
	/* icst_hz_to_vco rounds down -- so we need the next
	 * larger freq in case of CPUFREQ_RELATION_L.
	 */
	if (relation == CPUFREQ_RELATION_L)
		target_freq += 999;
	if (target_freq > policy->max)
		target_freq = policy->max;
	//vco = icst_hz_to_vco(&cclk_params, target_freq * 1000);
	//freqs.new = icst_hz(&cclk_params, vco) / 1000;
	freqs.new = get_mboot_freq(); //johnson

	freqs.cpu = policy->cpu;

	if (freqs.old == freqs.new) {
		set_cpus_allowed(current, cpus_allowed);
		return 0;
	}

	cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
   //set_freq(freqs.new/1000); //johnson
	/*cm_osc = __raw_readl(CM_OSC);

	if (machine_is_integrator()) {
		cm_osc &= 0xfffff800;
		cm_osc |= vco.s << 8;
	} else if (machine_is_cintegrator()) {
		cm_osc &= 0xffffff00;
	}
	cm_osc |= vco.v;

	__raw_writel(0xa05f, CM_LOCK);
	__raw_writel(cm_osc, CM_OSC);
	__raw_writel(0, CM_LOCK); */

	/*
	 * Restore the CPUs allowed mask.
	 */
	set_cpus_allowed(current, cpus_allowed);

	cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);

	return 0;
}

static unsigned int integrator_get(unsigned int cpu)
{
	cpumask_t cpus_allowed;
	unsigned int current_freq;
	u_int cm_osc;
	struct icst_vco vco;
	cpus_allowed = current->cpus_allowed;
	set_cpus_allowed(current, cpumask_of_cpu(cpu));
	BUG_ON(cpu != smp_processor_id());

	/* detect memory etc. */
	/*cm_osc = __raw_readl(CM_OSC);
	

	if (machine_is_integrator()) {
		vco.s = (cm_osc >> 8) & 7;
	} else {
		vco.s = 1;
	}
	vco.v = cm_osc & 255;
	vco.r = 22;

	current_freq = icst_hz(&cclk_params, vco) / 1000; */
	
	current_freq = get_mboot_freq();

	set_cpus_allowed(current, cpus_allowed);

	return current_freq;
}

static int integrator_cpufreq_init(struct cpufreq_policy *policy)
{
printk("\n%s,%d \n",__FUNCTION__,__LINE__);
	/* set default policy and cpuinfo */
	policy->cpuinfo.max_freq = 160000;
	policy->cpuinfo.min_freq = 12000;
	policy->cpuinfo.transition_latency = 1000000; /* 1 ms, assumed */
	policy->cur = policy->min = policy->max = integrator_get(policy->cpu);
	return 0;
}

static struct cpufreq_driver integrator_driver = {
	.verify		= integrator_verify_policy,
	.target		= integrator_set_target,
	.get		= integrator_get,
	.init		= integrator_cpufreq_init,
	.name		= "integrator",
};


typedef struct _IO_CPU_setting_INFO
{
	char* MESSAGE_BUFF;
	char CPUID;
	char  MESSAGE_LEN;
	int MID;	
}IO_CPU_setting_INFO;

static int CPU_setting_proc_ioctl(struct file *filp, unsigned int cmd, IO_CPU_setting_INFO* message_buf)
{
	
	
	IO_CPU_setting_INFO* bb = message_buf;
	char usr_buf[256];
	//int ii = 0;

	if (copy_from_user(usr_buf, bb->MESSAGE_BUFF, bb->MESSAGE_LEN)) {
		printk("setgreq_proc_ioctl error\n");
			return -EFAULT;
		}
			
	return 0;
}

extern int ORI_FREQ_KHZ;
static int cunt=0;
static ssize_t CPU_setting_proc_write(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
	char buffer[MAX_DMSG_WRITE_BUFFER];
	long idx;
	int i=0; 
	struct cpufreq_freqs freqs;
 
	if (!count)
		return count;

	if (count >= MAX_DMSG_WRITE_BUFFER)
		count = MAX_DMSG_WRITE_BUFFER - 1;

	/*
	 * Prevent Tainted Scalar Warning:
	 * Buffer can't be tainted because:
	 * 1. The count never exceeds MAX_DMSG_WRITE_BUFFER i.e. buffer size.
	 * 2. copy_from_user returns 0 in case of correct copy.
	 *So, we don't need to sanitize buffer.
	 *
	 */
	if (copy_from_user(buffer, buf, count))
		return -EFAULT;

	buffer[count] = '\0';

	if (buffer[0] == '/')
		idx = 3;
	else if (strict_strtol(buffer, 0, &idx) != 0)
		return -EINVAL;

  cunt++;
  if(cunt == 1)
  	{
  		change_interval(1300000,ORI_FREQ_KHZ);
    }
    
	for(i=0;i<4;i++)
	{   
		freqs.cpu=i;
		freqs.old=current_freq;
		freqs.new=idx;
		cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
		if(i==0){
	             set_freq(freqs.new/1000);
			}
		cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
		
	}
	current_freq=idx;
	
	change_interval(freqs.old,freqs.new);	
	
		
	return count;
}


static int CPU_setting_proc_open(struct inode *inode, struct file *file)
{
	long idx;

	if (atomic_read(&proc_is_open))
		return -EACCES;

	atomic_set(&proc_is_open, 1);
	return 0;	
}

static int CPU_setting_proc_release(struct inode *inode, struct file * file)
{

	WARN_ON(!atomic_read(&proc_is_open));
	atomic_set(&proc_is_open, 0);
	return 0;
}


const struct file_operations proc_CPU_setting_operations = {
	//.read       = setfreq_read,
	.write      = CPU_setting_proc_write,
	.open       = CPU_setting_proc_open,
	.release    = CPU_setting_proc_release,
  .unlocked_ioctl  = CPU_setting_proc_ioctl,
};


static int __init init_procfs_msg(void)
{
  current_freq= get_mboot_freq();
  printk("\n current_freq = %d\n",current_freq);
  REG(REG_freq_low_bound_l) = 0x78D4; //1.008GHZ
  REG(REG_freq_low_bound_h) = 0x0029; //1.008GHZ
  REG(REG_freq_high_bound_l) = 0xE6CB; //1.3GHZ
  REG(REG_freq_high_bound_h) = 0x001F; //1.3GHZ
  
  cpufreq_register_driver(&integrator_driver);
	proc_create("CPU_setting", S_IRUSR | S_IWUSR, NULL, &proc_CPU_setting_operations);
	return 0;
}


static int __init CPU_setting_init(void)
{

	init_procfs_msg();

	
	return 0;
}


module_init(CPU_setting_init);
