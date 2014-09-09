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
#include <linux/vmalloc.h> 
//#include <linux/atomic.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/ctype.h>
#include <linux/delay.h>

#include <asm/setup.h>
//#include "internal.h"
#include <asm/cacheflush.h>


//static struct class *ramlog_class;

typedef struct _IO_RAMLOG_INFO
{
	char* MESSAGE_BUFF;
	char CPUID;
	char  MESSAGE_LEN;
	int MID;	
}IO_RAMLOG_INFO;

//typedef struct RAMLOG_INFO TP_RAMLOG_INFO;

typedef struct _HK_RAMLOG_INFO
{
	//char* MESSAGE_BUFF;
	//char CPUID;
	int mid;
    unsigned int message_time;
	char  message_len;
}HK_RAMLOG_INFO;


typedef struct _HK_RAMLOG_DM
{
    unsigned long u32WaitMs;
	unsigned long u32BufSize;
	char* MESSAGE_BUFF;
	unsigned long u32MessageLen;
}HK_RAMLOG_DM;

static atomic_t proc_is_open = ATOMIC_INIT(0);


#define MAX_DMSG_WRITE_BUFFER	64


#if 1//def RAMLOG

extern int ramlog_buf_adr ;
extern char ramlog_cmdline[COMMAND_LINE_SIZE];
//unsigned long RAMLOG_ADDRESS 0x80000000
#define RAMLOG_SIZE 0x100000

#define write_wpoint(value)     (*(volatile unsigned int *)(ramlog_buf_adr) = value)
#define write_char(addr,value)     (*(volatile unsigned char *)(addr) = value)
#define read_wpoint             (*(volatile unsigned int *)(ramlog_buf_adr))

#define write_rpoint(value)     (*(volatile unsigned int *)(ramlog_buf_adr+4) = value)
#define read_rpoint             (*(volatile unsigned int *)(ramlog_buf_adr+4))
#define read_char(addr)               (*(volatile unsigned char *)(addr))

static int ram_log_buff_start;

static int overtake = 0 ;

//struct mutex ramlog;
static spinlock_t ramlog;

static unsigned long u32LastdataForWpoint;

int bRamlogInit = 0;
#endif

unsigned int ramlog_kernel_printk=1;
EXPORT_SYMBOL(ramlog_kernel_printk);

unsigned int ramlog_kernel_printf=1;
EXPORT_SYMBOL(ramlog_kernel_printf);

unsigned int ramlog_kernel_enable=1;
EXPORT_SYMBOL(ramlog_kernel_enable);

void ram_log_write(int ModuleID,const char* buff,int len)
{
	
	char *p;
	int count=0;
	char printf_buf[256];
	HK_RAMLOG_INFO ramlog_info;
	int log_len = len + sizeof(ramlog_info) ; //MID(2) + len (1) + message_len;
	int tmpRam_log_buff=0;
	
	if(len > 256)
	len=256;
	ramlog_info.mid = ModuleID;
	ramlog_info.message_len= len;
	ramlog_info.message_time=0;
	/*
	if(ModuleID ==2)
	{
	    if (copy_from_user(printf_buf,buff,len)) {
		    printk("ramlog printf error\n");
			return -EFAULT;
		}
	}
	*/
	spin_lock(&ramlog);
    tmpRam_log_buff = read_wpoint;
	
	if((tmpRam_log_buff+log_len)<(ramlog_buf_adr+RAMLOG_SIZE))
	{ 
	    memcpy((void *)tmpRam_log_buff,&ramlog_info,sizeof(ramlog_info));
		tmpRam_log_buff = tmpRam_log_buff+sizeof(ramlog_info);
		memcpy((void *)tmpRam_log_buff,buff,len);
		tmpRam_log_buff = tmpRam_log_buff+len;
	}else
	{
	    overtake = 1;
		u32LastdataForWpoint = read_wpoint;
	    tmpRam_log_buff = (ram_log_buff_start);  //reset buffer pointer
	}
	
	write_wpoint(tmpRam_log_buff);

	if((overtake==1)&&(read_wpoint>read_rpoint))
	{
	    write_rpoint(tmpRam_log_buff);
	}
	/*
	
	for (p = buff; *p ; p++)
  {
  	if(count>len)
  		break;

						   // ram_log_buff = 0+8;
	  if(ram_log_buff<(ramlog_buf_adr+RAMLOG_SIZE))
	  {
	        ram_log_buff++;
	        write_char(ram_log_buff,*p);
	   }else
	   {
	        ram_log_buff = (ramlog_buf_adr+8);
	   }

		  write_wpoint(ram_log_buff);
		  
		  count++;
		
  }
  */
  __cpuc_flush_kern_all();
   spin_unlock(&ramlog);
}

void dump_ramlog(void)
{
	//char usr_buf[256];
	int count = 0;
	char* log_buff_read_point;
	int log_buff_write_point = read_wpoint;
	log_buff_read_point = (char* )read_rpoint;
	//return;
	
	if((int)log_buff_read_point == log_buff_write_point)
	{
		printk("no data dump\n");
		return ;
	}

	#if 0
	write_rpoint((int)log_buff_read_point + 1);
	printk("dump ram log:\n");
  write_rpoint((int)log_buff_read_point + 1);
  while((int)log_buff_read_point<log_buff_write_point)
	{    
    for(count=0;count<256;count++)
    {
    	if(read_char((int)log_buff_read_point+count) != 0x0a)   //0x0a = LF
    	{
    		if(isascii(*(log_buff_read_point+count)))
			  {
			  	printk("%c",*(log_buff_read_point+count));
	
			  }
    	  continue;
    	}
    	else
    	{
    		printk("\n");
    		break;
    	}
    }

	    log_buff_read_point = log_buff_read_point+count+1;
		write_rpoint((int)log_buff_read_point);
  	}
    #else//temp

	while((int)log_buff_read_point<log_buff_write_point)
	{    
    for(count=0;count<COMMAND_LINE_SIZE;count++)
    {
    	if(read_char((int)log_buff_read_point+count) != 0x0a)   //0x0a = LF
    	{
    		if(isascii(*(log_buff_read_point+count)))
			  {
			  	printk("%c",*(log_buff_read_point+count));
	
			  }
    	  continue;
    	}
    	else
    	{
    		printk("\n");
    		break;
    	}
    }
	log_buff_read_point = log_buff_read_point+count+1;
    	
		//printk("%s",usr_buf);
	}
	#endif

}

static int ramlog_proc_ioctl(struct file *filp, unsigned int cmd, HK_RAMLOG_DM* arg)
{
	
	
	HK_RAMLOG_DM resoult;
	//char usr_buf[256];
	int ret = 0;
	unsigned long StopTime;
	unsigned long u32LocalBufSize = arg->u32BufSize;
	HK_RAMLOG_INFO* ramlog_info;
	unsigned long u32count = 0;
	char* user_bur = NULL;

    unsigned long u32ForLoopCnt = 0;
    
	unsigned int u32ReadPointer = read_rpoint;
	unsigned int u32WritePointer = read_wpoint;

	if((overtake == 0)&&((u32WritePointer-u32ReadPointer)<u32LocalBufSize))
		msleep(arg->u32WaitMs);

    //printk("ramlog ioctl : u32BufSize = 0x%lx \n",u32LocalBufSize);

	if (copy_from_user(&resoult, arg,sizeof(HK_RAMLOG_DM)))
	{
		printk("ramlog_proc_ioctl error\n");
			return -EFAULT;
		}
		
	user_bur = (char*)vmalloc(u32LocalBufSize);

    spin_lock(&ramlog);

	if((overtake==0)&&(read_wpoint==read_rpoint))
	{
	    resoult.u32MessageLen = 0;
		//printk("ramlog ioctl : ramlog buffer empty \n");
		spin_unlock(&ramlog);
		copy_to_user(arg, &resoult,sizeof(HK_RAMLOG_DM));
		vfree(user_bur);
		return 0;
	}

	if(overtake == 0)
	{
        do
        {
	        ramlog_info = (HK_RAMLOG_INFO*)read_rpoint;
		    if((u32count+ramlog_info->message_len)>u32LocalBufSize)
			   break;
			
		    memcpy((char*)(user_bur+u32count),(char*)(read_rpoint+sizeof(HK_RAMLOG_INFO)),ramlog_info->message_len);
		    u32count = u32count+ramlog_info->message_len;
		    write_rpoint(read_rpoint+sizeof(HK_RAMLOG_INFO)+ramlog_info->message_len);
        }while(read_rpoint<read_wpoint);

		if(read_rpoint==read_wpoint)  //reset pointer;
		{  
		   write_rpoint(ram_log_buff_start);
		   write_wpoint(ram_log_buff_start);
		}
	
	}else
	{
	}

	spin_unlock(&ramlog);

	for(u32ForLoopCnt = 0;u32ForLoopCnt<=u32count;u32ForLoopCnt++)
	{
	    if(!isascii(*(user_bur+u32ForLoopCnt)))
			*(user_bur+u32ForLoopCnt)= ' ';
	}

    resoult.u32MessageLen = u32count;
	if (copy_to_user(arg->MESSAGE_BUFF, user_bur,u32LocalBufSize))
	{
		printk("ramlog copy_to_user error\n");
		vfree(user_bur);
			return -EFAULT;
	}
		
	 //printk("kdriver MESSAGE_BUFF = %lx \n",bb->MESSAGE_BUFF);
	 //printk("kdriver MESSAGE_LEN = %d \n",bb->MESSAGE_LEN);	 
	 
	 //printk("kdriver MESSAGE : ");
	 //for(ii=0;ii<bb->MESSAGE_LEN;ii++)
	 //{
	 //   printk("%x:",usr_buf[ii]);
	 //}
	 //printk("\n");
		
   //ram_log_write(,usr_buf,bb->MESSAGE_LEN);
   
   //while(1);
	copy_to_user(arg, &resoult,sizeof(HK_RAMLOG_DM));
	 vfree(user_bur);
	return 0;
}


static ssize_t ramlog_proc_write(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
	char buffer[MAX_DMSG_WRITE_BUFFER];
	long idx;

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

#if 0
	printk(KERN_CRIT"[%s] idx = %d\n", __func__, idx);  /* for debug */
#endif

	switch (idx) {
	case 1:
		break;
	case 2:
		dump_ramlog();
		break;
	case 3:

		break;
	case 4:

		break;
	case 5:

		break;
	default:
		printk(KERN_CRIT"\nHow to use :\n");
		printk(KERN_CRIT"    echo 1 >  # show info\n");
		printk(KERN_CRIT"    echo 2 >  # dump log\n");
	}
	
	return count;
}


static int ramlog_proc_open(struct inode *inode, struct file *file)
{
	if (atomic_read(&proc_is_open))
		return -EACCES;

	atomic_set(&proc_is_open, 1);
	return 0;	
}

static int ramlog_proc_release(struct inode *inode, struct file * file)
{
	WARN_ON(!atomic_read(&proc_is_open));
	atomic_set(&proc_is_open, 0);
	return 0;
}


const struct file_operations proc_ramlog_operations = {
	//.read       = ramlog_read,
	.write      = ramlog_proc_write,
	.open       = ramlog_proc_open,
	.release    = ramlog_proc_release,
  .unlocked_ioctl  =ramlog_proc_ioctl,
};

static int __init init_procfs_msg(void)
{

	proc_create("ramlog", S_IRUSR | S_IWUSR, NULL, &proc_ramlog_operations);
	return 0;
}

static int __init ramlog_printk_setup(char *str)
{
   // printk("KernelPrintk= %s\n", str);
    if( str != NULL )
    {
	  if(strcmp(str,"off")==0)
	  {
        ramlog_kernel_printk = 0;
		printk("\nramlog: printk disable\n");
	  }else{
	    ramlog_kernel_printk = 1;
		printk("\nramlog: printk enable\n");
		
	  }
        
    }
    else
    {
	    ramlog_kernel_printk = 1;
        printk("\nkernel_printk not set\n");
    }
    return 0;
}

static int __init ramlog_printf_setup(char *str)
{
    //printk("KernelPrintf= %s\n", str);
    if( str != NULL )
    {
	  if(strcmp(str,"off")==0)
	  {
        ramlog_kernel_printf = 0;
		printk("\nramlog: printf disable\n");
	  }else{
	    ramlog_kernel_printf = 1;
		printk("\nramlog: printf enable\n");
	  }
        
    }
    else
    {
	    ramlog_kernel_printf = 1;
        printk("\nkernel_printf not set\n");
    }
    return 0;
}

static int __init ramlog_init(void)
{
    char consloe_CF = 0x0a;
	ram_log_buff_start=ramlog_buf_adr+8;
    u32LastdataForWpoint= ramlog_buf_adr + RAMLOG_SIZE;
	write_wpoint(ram_log_buff_start);
	write_rpoint(ram_log_buff_start);
	init_procfs_msg();
	
	//mutex_init(&ramlog);
	spin_lock_init(&ramlog);
	
	early_param("KernelPrintk",ramlog_printk_setup);
	early_param("KernelPrintf",ramlog_printf_setup);
	
	bRamlogInit = 1;

	//ramlog_cmdline[COMMAND_LINE_SIZE-1] = 0x0a;
  //ram_log_write(ramlog_cmdline,COMMAND_LINE_SIZE);
  //ram_log_write(&consloe_CF,1);
	return 0;
}


module_init(ramlog_init);