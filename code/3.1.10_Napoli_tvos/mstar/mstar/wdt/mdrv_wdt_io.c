

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>

#include "mdrv_wdt_io.h"
#include "mdrv_wdt.h"

#define DEFAULT_WDT_RESET_TIME  (16)    //unit: sec

static unsigned long driver_open, orphan_timer;
static char expect_close;
static int g_margin;


static void mdrv_wdt_set_timeout(int margin) //unit: sec
{
	MDrv_WDT_SetWDT_MS(margin*1000);
	g_margin = margin;
	printk(KERN_EMERG "[%s] set WDT timeout: %d sec.\n", __FUNCTION__, margin);
}

static long mdrv_wdt_ioctl(struct file *file, unsigned int cmd,
							unsigned long arg)
{

	//void __user *argp = (void __user *)arg;
	//int __user *p = argp;
	int new_margin;
	static const struct watchdog_info ident = {
		.options =		WDIOF_SETTIMEOUT |
					WDIOF_KEEPALIVEPING |
					WDIOF_MAGICCLOSE,
		.firmware_version =	0,
		.identity =		"MStar Watchdog",
	};

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		//return copy_to_user(argp, &ident, sizeof(ident)) ? -EFAULT : 0;
		break;
	case WDIOC_GETSTATUS:
		break;
	case WDIOC_GETBOOTSTATUS:
		return put_user(0, (int __user *) arg);
	case WDIOC_KEEPALIVE:
		MDrv_WDT_ClearWDT();
		return 0;
	case WDIOC_SETTIMEOUT:
		if (get_user(new_margin, (int __user *) arg))
		{
			new_margin = *((int *)arg);
		}
		mdrv_wdt_set_timeout(new_margin);
		break;
	case WDIOC_GETTIMEOUT:
		//return put_user(g_margin, p);
		break;
	default:
		return -ENOTTY;
	}

	return 0;
}


static ssize_t mdrv_wdt_write(struct file *file, const char __user *data,
						size_t len, loff_t *ppos)
{
	/*
	 *	Refresh the timer.
	 */
	if (len) {
		size_t i;
		/* In case it was set long ago */
		expect_close = 0;

		for (i = 0; i != len; i++) {
			char c;	
			if (get_user(c, data + i))
				return -EFAULT;
			if (c == 'V')
			{
				printk(KERN_EMERG "prepare to stop watchdog!\n");
				expect_close = 42;
			}
		}
	}

	MDrv_WDT_ClearWDT();
	return len;
}

static int mdrv_wdt_open(struct inode *inode, struct file *file)
{
	if (test_and_set_bit(0, &driver_open))
	{
		printk(KERN_EMERG "[%s] device is busy \n", __FUNCTION__);
		return -EBUSY;
	}
	if (!test_and_clear_bit(0, &orphan_timer))
		__module_get(THIS_MODULE);
	/*
	 *	Activate timer
	 */
	MDrv_WDT_SetWDTInt(DEFAULT_WDT_RESET_TIME);

	printk(KERN_EMERG "[%s] open , default wdt timeout:%d sec.\n", __FUNCTION__, DEFAULT_WDT_RESET_TIME);
	
	return nonseekable_open(inode, file);

}

static int mdrv_wdt_release(struct inode *inode, struct file *file)
{
	/*
	 *	Shut off the timer.
	 *	Lock it in if it's a module and we set nowayout
	 */
	if (expect_close == 42) {
		MDrv_WDT_DisableWDT();
		module_put(THIS_MODULE);
		printk(KERN_EMERG 
			"[%s] close WDT\n", __FUNCTION__);
	} else {
		printk(KERN_EMERG 
			"Unexpected close, not stopping watchdog!\n");
		set_bit(0, &orphan_timer);
		MDrv_WDT_ClearWDT();
	}
	clear_bit(0, &driver_open);
	expect_close = 0;

	//MDrv_WDT_DisableWDT();
	return 0;
}


static int mdrv_wdt_notify_sys(struct notifier_block *this, unsigned long code,
	void *unused)
{

	if (code == SYS_DOWN || code == SYS_HALT)
		MDrv_WDT_DisableWDT();
	
	return NOTIFY_DONE;
}

static const struct file_operations mstar_wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= mdrv_wdt_write,
	.unlocked_ioctl	= mdrv_wdt_ioctl,
	.open		= mdrv_wdt_open,
	.release	= mdrv_wdt_release,
};

static struct miscdevice mstar_wdt_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &mstar_wdt_fops,
};

static struct notifier_block mstar_wdt_notifier = {
	.notifier_call	= mdrv_wdt_notify_sys,
};

static int __init mstar_watchdog_init(void)
{
	int ret;

	ret = register_reboot_notifier(&mstar_wdt_notifier);
	if (ret) {
		printk(KERN_EMERG 
			"cannot register reboot notifier (err=%d)\n", ret);
		return ret;
	}

	ret = misc_register(&mstar_wdt_miscdev);
	if (ret) {
		printk(KERN_EMERG
			"cannot register miscdev on minor=%d (err=%d)\n",
						WATCHDOG_MINOR, ret);
		unregister_reboot_notifier(&mstar_wdt_notifier);
		return ret;
	}

	printk(KERN_EMERG "[%s] WDT module init!!!!\n", __FUNCTION__);
	
	return 0;
}

static void __exit mstar_watchdog_exit(void)
{
	misc_deregister(&mstar_wdt_miscdev);
	unregister_reboot_notifier(&mstar_wdt_notifier);
	MDrv_WDT_DisableWDT();//diable WDT 
	printk(KERN_EMERG "[%s] WDT module exit!!!!\n", __FUNCTION__);
}

module_init(mstar_watchdog_init);
module_exit(mstar_watchdog_exit);

MODULE_DESCRIPTION("Mstar Watchdog Device Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);


