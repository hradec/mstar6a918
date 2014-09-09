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


//static struct class *pm_addr_class;


static atomic_t proc_is_open_iaddr = ATOMIC_INIT(0);
static atomic_t proc_is_open_isize = ATOMIC_INIT(0);
static atomic_t proc_is_open_daddr = ATOMIC_INIT(0);
static atomic_t proc_is_open_dsize = ATOMIC_INIT(0);
static atomic_t proc_is_open_run = ATOMIC_INIT(0);

unsigned long g_pm_Iaddress = 0;
unsigned long g_pm_Isize = 0;
unsigned long g_pm_Daddress = 0;
unsigned long g_pm_Dsize = 0;
unsigned long g_pm_run = 0;


#define MAX_DMSG_WRITE_BUFFER	64


static ssize_t pm_write_data(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos, unsigned long *ulData)
{
    char buffer[MAX_DMSG_WRITE_BUFFER];
    unsigned long idx;

    if (!count)
        return count;

    if (count >= MAX_DMSG_WRITE_BUFFER)
        count = MAX_DMSG_WRITE_BUFFER - 1;

	 if (copy_from_user(buffer, buf, count))
        return -EFAULT;

    buffer[count] = '\0';

    if (strict_strtoul(buffer, 0, &idx) != 0)
        return -EINVAL;

    //record input address.
    *ulData = (unsigned long)idx;

	return count;
}

/******************************* I Addr ***************************************/
static ssize_t pm_proc_read_iaddr(struct file *file, char __user *buf, size_t count, loff_t * ppos)
{
    printk("read pm_Iaddr :0x%x\n", (unsigned int)g_pm_Iaddress);
    return 0;
}

static ssize_t pm_proc_write_iaddr(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
    ssize_t ret;
    ret = pm_write_data(file, buf, count, ppos, &g_pm_Iaddress);
    printk("write pm_Iaddr :0x%x\n", (unsigned int)g_pm_Iaddress);
    return ret;
}

static int pm_Iaddr_proc_open(struct inode *inode, struct file *file)
{
	if (atomic_read(&proc_is_open_iaddr))
		return -EACCES;

	atomic_set(&proc_is_open_iaddr, 1);
	return 0;
}
static int pm_Iaddr_proc_release(struct inode *inode, struct file * file)
{
	WARN_ON(!atomic_read(&proc_is_open_iaddr));
	atomic_set(&proc_is_open_iaddr, 0);
	return 0;
}

const struct file_operations proc_pm_Iaddr_operations = {
	.read       = pm_proc_read_iaddr,
	.write      = pm_proc_write_iaddr,
	.open       = pm_Iaddr_proc_open,
	.release    = pm_Iaddr_proc_release,
};

/****************************************************************************/

/******************************* I Size ***************************************/
static ssize_t pm_proc_read_isize(struct file *file, char __user *buf, size_t count, loff_t * ppos)
{
    printk("read pm_Isize :0x%x\n", (unsigned int)g_pm_Isize);
    return 0;
}

static ssize_t pm_proc_write_isize(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
    ssize_t ret;
    ret = pm_write_data(file, buf, count, ppos, &g_pm_Isize);
    printk("write pm_Isize :0x%x\n", (unsigned int)g_pm_Isize);
    return ret;
}

static int pm_Isize_proc_open(struct inode *inode, struct file *file)
{
	if (atomic_read(&proc_is_open_isize))
		return -EACCES;

	atomic_set(&proc_is_open_isize, 1);
	return 0;
}
static int pm_Isize_proc_release(struct inode *inode, struct file * file)
{
	WARN_ON(!atomic_read(&proc_is_open_isize));
	atomic_set(&proc_is_open_isize, 0);
	return 0;
}

const struct file_operations proc_pm_Isize_operations = {
	.read       = pm_proc_read_isize,
	.write      = pm_proc_write_isize,
	.open       = pm_Isize_proc_open,
	.release    = pm_Isize_proc_release,
};
/****************************************************************************/

/******************************* D Addr **************************************/
static ssize_t pm_proc_read_daddr(struct file *file, char __user *buf, size_t count, loff_t * ppos)
{
    printk("read pm_Daddr :0x%x\n", (unsigned int)g_pm_Daddress);
    return 0;
}

static ssize_t pm_proc_write_daddr(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
    ssize_t ret;
    ret = pm_write_data(file, buf, count, ppos, &g_pm_Daddress);
    printk("write pm_Daddr :0x%x\n", (unsigned int)g_pm_Daddress);
    return ret;
}

static int pm_Daddr_proc_open(struct inode *inode, struct file *file)
{
	if (atomic_read(&proc_is_open_daddr))
		return -EACCES;

	atomic_set(&proc_is_open_daddr, 1);
	return 0;
}
static int pm_Daddr_proc_release(struct inode *inode, struct file * file)
{
	WARN_ON(!atomic_read(&proc_is_open_daddr));
	atomic_set(&proc_is_open_daddr, 0);
	return 0;
}

const struct file_operations proc_pm_Daddr_operations = {
	.read       = pm_proc_read_daddr,
	.write      = pm_proc_write_daddr,
	.open       = pm_Daddr_proc_open,
	.release    = pm_Daddr_proc_release,
};
/****************************************************************************/

/******************************* D Size **************************************/
static ssize_t pm_proc_read_dsize(struct file *file, char __user *buf, size_t count, loff_t * ppos)
{
    printk("read pm_Dsize :0x%x\n", (unsigned int)g_pm_Dsize);
    return 0;
}

static ssize_t pm_proc_write_dsize(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
    ssize_t ret;
    ret = pm_write_data(file, buf, count, ppos, &g_pm_Dsize);
    printk("write pm_Dsize :0x%x\n", (unsigned int)g_pm_Dsize);
    return ret;
}

static int pm_Dsize_proc_open(struct inode *inode, struct file *file)
{
	if (atomic_read(&proc_is_open_dsize))
		return -EACCES;

	atomic_set(&proc_is_open_dsize, 1);
	return 0;
}
static int pm_Dsize_proc_release(struct inode *inode, struct file * file)
{
	WARN_ON(!atomic_read(&proc_is_open_dsize));
	atomic_set(&proc_is_open_dsize, 0);
	return 0;
}

const struct file_operations proc_pm_Dsize_operations = {
	.read       = pm_proc_read_dsize,
	.write      = pm_proc_write_dsize,
	.open       = pm_Dsize_proc_open,
	.release    = pm_Dsize_proc_release,
};
/****************************************************************************/

/******************************* Run Addr ************************************/
static ssize_t pm_run_proc_read(struct file *file, char __user *buf, size_t count, loff_t * ppos)
{
    printk("read pm_run :0x%x\n", (unsigned int)g_pm_run);
    return 0;
}
static ssize_t pm_run_proc_write(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
    ssize_t ret;
    ret = pm_write_data(file, buf, count, ppos, &g_pm_run);
    printk("write pm_run :0x%x\n", (unsigned int)g_pm_run);
    return ret;
}
static int pm_run_proc_open(struct inode *inode, struct file *file)
{
	if (atomic_read(&proc_is_open_run))
		return -EACCES;

	atomic_set(&proc_is_open_run, 1);
	return 0;
}
static int pm_run_proc_release(struct inode *inode, struct file * file)
{
	WARN_ON(!atomic_read(&proc_is_open_run));
	atomic_set(&proc_is_open_run, 0);
	return 0;
}

const struct file_operations proc_pm_run_operations = {
	.read       = pm_run_proc_read,
	.write      = pm_run_proc_write,
	.open       = pm_run_proc_open,
	.release    = pm_run_proc_release,
};
/****************************************************************************/

static int __init init_procfs_msg(void)
{
    proc_mkdir("pm", NULL);
	proc_create("pm/pm_Iaddr", S_IRUSR | S_IWUSR, NULL, &proc_pm_Iaddr_operations);
	proc_create("pm/pm_Isize", S_IRUSR | S_IWUSR, NULL, &proc_pm_Isize_operations);
	proc_create("pm/pm_Daddr", S_IRUSR | S_IWUSR, NULL, &proc_pm_Daddr_operations);
	proc_create("pm/pm_Dsize", S_IRUSR | S_IWUSR, NULL, &proc_pm_Dsize_operations);
	proc_create("pm/pm_run", S_IRUSR | S_IWUSR, NULL, &proc_pm_run_operations);
	return 0;
}

static int __init pm_addr_init(void)
{
	init_procfs_msg();

	return 0;
}


module_init(pm_addr_init);
