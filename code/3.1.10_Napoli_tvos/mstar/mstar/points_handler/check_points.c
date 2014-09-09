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
#include <asm/page.h>
#include <asm/pgtable.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/ctype.h>
#include <linux/reboot.h>

#include <asm/setup.h>
#include <asm/cacheflush.h>

/*
 *
 */

/* Use 'M' as magic number */

struct mutex    cp_lock;
#define MAX_SIZE_STR                       64
struct ioctl_arg
{
        unsigned int reg;
        unsigned int val;
        char Str[MAX_SIZE_STR];
};
#define atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0)	/* atio not defined in mips : Namit */

#define MAJOR_NUM                               60
#define MODULE_NAME                           "POINTS_HANDLER"
#define DEV_NAME                                 "boot_points"

#define DIR_SAVE_PATH                         "/mnt/usb/sda1"
#define MBOOT_PREFIX_STR                   "MB_"
#define MBOOT_PREFIX_COUNT_STR        "en_chk_p"
#define POINTS_IOC_MAGIC                    'N'
#define POINTS_IOC_MAXNR                    1
#define MARK_STR                                  "]["
#define MARK_END_STR                           "[AN][finish boot]"
#define END_CHARACTER                         '\0'

#define POINTS_IOC_KIND                       0xaa
#define POINTS_IOC_TYPE_BOOTING        0x01
#define POINTS_IOC_TYPE_SN                 0x02
#define POINTS_IOC_TYPE_AN                 0x03
#define POINTS_IOC_TYPE_IC                  0x04
#define POINTS_IOC_TYPE_NONE             0xff

#define POINTS_SET_SAVE	                       _IOW(POINTS_IOC_MAXNR,  1, struct ioctl_arg)	/* change ramdisk range include: start address, and size*/
#define POINTS_SET_PATH	                       _IOW(POINTS_IOC_MAXNR,  2, struct ioctl_arg)	/* change ramdisk range include: start address, and size*/
#define POINTS_SET_VER	                       _IOW(POINTS_IOC_MAXNR,  3, struct ioctl_arg)	/* change ramdisk range include: start address, and size*/
#define POINTS_GET	                             _IOW(POINTS_IOC_MAXNR,  4, struct ioctl_arg)	/* change ramdisk range include: start address, and size*/
#define BOOTARGS_BUF_SIZE                   512
#define STRING_BUF_SIZE                        128
#define TIME_BUF_SIZE                           10
#define SECS_FOR_A_MIN                        60

static char *pBuf_Bootargs = NULL;

#if (defined(CONFIG_MSTAR_MIPS))
extern unsigned int Chip_Query_MIPS_CLK(void);
#else
extern int query_frequency(void);
#endif

static unsigned char get_bootargs(char*_target)
{
    struct file *filp = NULL;
    mm_segment_t old_fs;
    loff_t off;

    // allocat the buffer for bootargs
    if(pBuf_Bootargs == NULL)
    {
        if (!(pBuf_Bootargs = kmalloc(BOOTARGS_BUF_SIZE, GFP_KERNEL)))
        {
            printk(KERN_NOTICE "SH@ memory alloc failed for pBuf_Bootargs\n");
        }
        else
        {
            pBuf_Bootargs[0] = '[';
            pBuf_Bootargs[1] = ']';
        }
    }
    else if(!(pBuf_Bootargs[0] == '[' && pBuf_Bootargs[1] == ']' ))
    {
        strncpy(_target, pBuf_Bootargs, strlen(pBuf_Bootargs));
        return 1;
    }

    // parsing count number from mboot
    if(filp == NULL)
    {
        filp = filp_open("/proc/cmdline", O_RDONLY, 0644);

        if (IS_ERR(filp))
        {
            printk(KERN_NOTICE "SH@ error occured while opening file //proc//cmdline, exiting...\n");
            return 0;
        }

        old_fs = get_fs();
        set_fs(KERNEL_DS);

        if (!IS_ERR(filp))
        {
            off = 0;
            filp->f_op->read(filp, (char *)_target, BOOTARGS_BUF_SIZE, &filp->f_pos);
            strncpy(pBuf_Bootargs, _target, strlen(_target));
        }

         set_fs(old_fs);

        if(filp != NULL)
        {
             filp_close(filp, NULL);
        }
     }

    //printk(KERN_NOTICE " bootargs=%s \n\n", _target);
    return 1;
}

static unsigned char get_counter(char* _bootargs, char *_target)
{
    char *pStart = NULL, *pMid = NULL, *pEnd = NULL;

    // parsing count number from mboot
    pStart = _bootargs;
    pStart = strstr (pStart, MBOOT_PREFIX_COUNT_STR);
    if (pStart != NULL)
    {
        pMid = strchr (pStart, '=');
        pEnd = strchr (pStart, ' ');

        strncpy(_target, pMid+1, (pEnd - pMid -1));
        _target[(pEnd - pMid - 1)] = END_CHARACTER;
        //printk(KERN_NOTICE "\n count_time=%s...\n", _target);

        if(_target != NULL)
        {
            return 1;
        }
    }

    return 0;
}

static unsigned char add_kr_piuTime(char* pStr, unsigned int new_val)
{
    char *pStart = NULL, *pMid = NULL;
    char str_val[TIME_BUF_SIZE];
    char m_tbuf[STRING_BUF_SIZE];
    unsigned int val;

    //printk(KERN_NOTICE "\n line=%d --> len=%d, (%d, %d)...\n", __LINE__, o_len, strlen((char *)MARK_STR), new_val);
    pStart = pStr;
    pStart = strstr (pStart, MARK_STR); // find first
    pStart = strstr (pStart+strlen((char *)MARK_STR), MARK_STR); // find second

    if (pStart != NULL)
    {
        pMid = strchr (pStart+strlen((char *)MARK_STR), ']');
        strncpy(str_val, pStart+strlen((char *)MARK_STR), (pMid - pStart - strlen((char *)MARK_STR)));
        str_val[(pMid - pStart - strlen((char *)MARK_STR))] = END_CHARACTER;

        val = atoi(str_val);
        val += new_val;

        strncpy(m_tbuf, pStr, (pStart - pStr + 1));
        m_tbuf[(pStart - pStr + 1)] = END_CHARACTER;

        sprintf(m_tbuf, "%s[%d]\n", m_tbuf, val);
        //printk(KERN_NOTICE "\n ori=%s, aft=%s, len=%d...\n", str_val, m_tbuf, strlen(m_tbuf));

        strncpy(pStr, m_tbuf, strlen(m_tbuf));
        pStr[strlen(m_tbuf)] = END_CHARACTER;

        if(pStr != NULL)
        {
            return 1;
        }
    }

    return 0;
}

static unsigned int get_value(char* pStr)
{
    char *pStart = NULL, *pMid = NULL;
    char str_val[TIME_BUF_SIZE];
    unsigned int val = 0;

    pStart = pStr;
    pStart = strstr (pStart, MARK_STR); // find first
    pStart = strstr (pStart+strlen((char *)MARK_STR), MARK_STR); // find second
    if (pStart != NULL)
    {
        pMid = strchr (pStart+strlen((char *)MARK_STR), ']');
        strncpy(str_val, pStart+strlen((char *)MARK_STR), (pMid - pStart - strlen((char *)MARK_STR)));
        str_val[(pMid - pStart - strlen((char *)MARK_STR))] = END_CHARACTER;

        val = atoi(str_val);
    }

    return val;
}

static void swap_str(char* pAfter, char* pBefore)
{
    unsigned int after_val, before_val;

    if (pAfter != NULL && pBefore != NULL)
    {
        // get value
        after_val = get_value(pAfter);
        before_val = get_value(pBefore);

        // compare
        if(before_val > after_val)
        {
            char pBuf[STRING_BUF_SIZE];
            //printk(KERN_NOTICE "\n Before --> updated for (%s and %s)..\n", pAfter, pBefore);

            // swap
            after_val = strlen(pAfter);
            before_val = strlen(pBefore);

            strncpy(pBuf, pAfter, after_val);
            strncpy(pAfter, pBefore, before_val);
            pAfter[before_val] = END_CHARACTER;
            strncpy(pBefore, pBuf, after_val);
            pBefore[after_val] = END_CHARACTER;

            //printk(KERN_NOTICE "\n After --> updated for (%s and %s)..\n", pAfter, pBefore);
        }
    }
}

static ssize_t drv_read(struct file *filp, char *buf, size_t count, loff_t *ppos)
{
    //printk(KERN_NOTICE "SH@ device read\n");
    return count;
}

#define POINTS_NUMBERS                      40
#define MBOOT_CHK_POINTS_NUMBER    3
#define MAX_FILE_SIZE                         1024
#define MAX_SAVING_FILE_NUMBERS     16
#define MB_VER_STR                              "mboot_ver"

extern unsigned int kr_PiuTime;
static char *pPoints[POINTS_NUMBERS];
static int  points_len = 0;
static char *pStr_Path = NULL;
static char *pStr_Ver_SN = NULL;
static char *pStr_Ver_AN = NULL;
static char *pStr_Ver_IC = NULL;
static unsigned char bStop = 0;
static ssize_t drv_write(struct file *filp, const char *buf, size_t count, loff_t *ppos)
{
    char *pbuf = NULL;
    char mbuf[BOOTARGS_BUF_SIZE]="";
    char str_val[TIME_BUF_SIZE];

    if(bStop == 1)
    {
        return 1;
    }

    mutex_lock(&cp_lock);

    // -- is it enabled for check-points? --------------------->
    if(!get_bootargs(mbuf))
    {
        printk(KERN_NOTICE "SH@ open bootargs file failed\n");
        mutex_unlock(&cp_lock);
        return 0;
    }

    // get counter
    if(get_counter(mbuf, str_val) == 0)
    {
        mutex_unlock(&cp_lock);
        return 1;
    }
    // <-------------------------------------------------

    //printk(KERN_NOTICE "SH@ device write, len=%d, buf=%s\n", count, buf);

    /* allocate a buffer */
    if (!(pbuf = kmalloc(STRING_BUF_SIZE, GFP_KERNEL)))   // STRING_BUF_SIZE: sometimes it will swap for sorting, so keep the size to max.
    {
        printk(KERN_NOTICE "SH@ memory alloc failed, %d\n", count+1);
        mutex_unlock(&cp_lock);
        return 0;
    }
    else
    {
        if (copy_from_user(pbuf, buf, count))
        {
            printk(KERN_NOTICE "SH@ get data failed\n");
        }
        else
        {
            pbuf[count] = END_CHARACTER;

            // add the kernel piu time
            if(add_kr_piuTime(pbuf, kr_PiuTime) == 0)
            {
                printk(KERN_NOTICE "add kernel piu time failed\n");
            }
            else
            {
                // printk(KERN_NOTICE "SH@ add KR piu time (%d), data = (%s), ", kr_PiuTime, pbuf);
            }

            // to Queue
            pPoints[points_len] = pbuf;

            // compare and swap
            if(points_len > 0)
            {
                swap_str(pPoints[points_len], pPoints[points_len-1]);
            }

            // get stop flag
            if(!strncmp(pPoints[points_len], MARK_END_STR, strlen(MARK_END_STR)))
            {
                // printk(KERN_NOTICE "SH@ stop stop...\n\n");
                bStop = 1;
            }

            points_len++;
        }
    }

    mutex_unlock(&cp_lock);

    return count;
}

static int drv_open(struct inode *inode, struct file *filp)
{
    //printk(KERN_NOTICE "\n\nSH@ ==========================================> OPEN \n\n");
    //printk(KERN_NOTICE "SH@ device open\n");
    return 0;
}

static long drv_ioctl(struct file *_filp, unsigned int cmd, unsigned long arg)
{
    static unsigned char m_once = 0;   // only get mboot info once, MB_xxx
    struct file *filp = NULL;
    int i = 0;
    char *pMPoints[MBOOT_CHK_POINTS_NUMBER];
    int  MPoints_len = 0;
    char mbuf[BOOTARGS_BUF_SIZE]="", m_tbuf[STRING_BUF_SIZE]="";
    mm_segment_t old_fs;
    loff_t off;
    char *pStart = NULL, *pMid = NULL, *pEnd = NULL;
    char str_val[TIME_BUF_SIZE], str_cnt[TIME_BUF_SIZE];
    char str_mb_ver[TIME_BUF_SIZE];
    struct ioctl_arg data;

    mutex_lock(&cp_lock);

    // -- is it enabled for check-points? --------------------->
    if(!get_bootargs(mbuf))
    {
        printk(KERN_NOTICE "SH@ open bootargs file failed\n");
        mutex_unlock(&cp_lock);
        return 0;
    }

    // get counter
    if(get_counter(mbuf, str_cnt) == 0)
    {
        mutex_unlock(&cp_lock);
        return 1;
    }
    // <-------------------------------------------------

    //printk(KERN_NOTICE "SH@ device ioctl\n");
    if(cmd == POINTS_GET)
    {
        int cnt;
        struct ioctl_arg temp;

        data.reg =  POINTS_IOC_KIND;
        cnt = atoi(str_cnt);
        if(cnt > 0 && cnt < MAX_SAVING_FILE_NUMBERS)
        {
            data.val =  POINTS_IOC_TYPE_BOOTING;   // valid for booting check-points
        }
        else
        {
            data.val =  0;   // invalid for booting check-points
        }

        if (copy_from_user(&temp, (void __user *)arg, sizeof(temp)) )
        {
            printk(KERN_NOTICE "SH@ ioctl...copy_from_user failed(POINTS_GET)...\n");
            mutex_unlock(&cp_lock);
            return -EFAULT;
        }

        if (copy_to_user((void __user *)arg, &data, sizeof(data)) )
        {
            mutex_unlock(&cp_lock);
            return -EFAULT;
        }
    }
    else if(cmd == POINTS_SET_PATH)
    {
        struct ioctl_arg temp;

        if (copy_from_user(&temp, (void __user *)arg, sizeof(temp)) )
        {
            printk(KERN_NOTICE "SH@ ioctl...copy_from_user failed(POINTS_SET_PATH)...\n");
            mutex_unlock(&cp_lock);
            return -EFAULT;
        }

        printk(KERN_NOTICE "SH@ kernel device ioctl, 0x%x, %d, path=%s\n", temp.reg, temp.val, temp.Str);
        if (!(pStr_Path = kmalloc(sizeof(temp.Str) + 1, GFP_KERNEL)))
        {
            printk(KERN_NOTICE "SH@ memory alloc failed, %s\n", temp.Str);
            mutex_unlock(&cp_lock);
            return 0;
        }
        else
        {
            strncpy(pStr_Path, temp.Str, sizeof(temp.Str));
            pStr_Path[sizeof(temp.Str)] = END_CHARACTER;
        }
    }
    else if(cmd == POINTS_SET_VER)
    {
        struct ioctl_arg temp;

        if (copy_from_user(&temp, (void __user *)arg, sizeof(temp)) )
        {
            printk(KERN_NOTICE "SH@ ioctl...copy_from_user failed(POINTS_SET_PATH)...\n");
            mutex_unlock(&cp_lock);
            return -EFAULT;
        }

        printk(KERN_NOTICE "SH@ kernel device ioctl, 0x%x, %d, path=%s\n", temp.reg, temp.val, temp.Str);

        if(temp.reg == POINTS_IOC_KIND)
        {
            if(temp.val == POINTS_IOC_TYPE_SN)    // SN
            {
                if (!(pStr_Ver_SN = kmalloc(sizeof(temp.Str) + 1, GFP_KERNEL)))
                {
                    printk(KERN_NOTICE "SH@ memory alloc failed, %s\n", temp.Str);
                    mutex_unlock(&cp_lock);
                    return 0;
                }
                else
                {
                    strncpy(pStr_Ver_SN, temp.Str, sizeof(temp.Str));
                    pStr_Ver_SN[sizeof(temp.Str)] = END_CHARACTER;
                }
            }
            else if(temp.val == POINTS_IOC_TYPE_AN)    // AN
            {
                if (!(pStr_Ver_AN = kmalloc(sizeof(temp.Str) + 1, GFP_KERNEL)))
                {
                    printk(KERN_NOTICE "SH@ memory alloc failed, %s\n", temp.Str);
                    mutex_unlock(&cp_lock);
                    return 0;
                }
                else
                {
                    strncpy(pStr_Ver_AN, temp.Str, sizeof(temp.Str));
                    pStr_Ver_AN[sizeof(temp.Str)] = END_CHARACTER;
                }
            }
            else if(temp.val == POINTS_IOC_TYPE_IC)    // IC
            {
                if (!(pStr_Ver_IC = kmalloc(sizeof(temp.Str) + 1, GFP_KERNEL)))
                {
                    printk(KERN_NOTICE "SH@ memory alloc failed, %s\n", temp.Str);
                    mutex_unlock(&cp_lock);
                    return 0;
                }
                else
                {
                    strncpy(pStr_Ver_IC, temp.Str, sizeof(temp.Str));
                    pStr_Ver_IC[sizeof(temp.Str)] = END_CHARACTER;
                }
            }
        }
    }
    else if(cmd == POINTS_SET_SAVE)
    {
        // printk(KERN_NOTICE "SH@ ioctl...@ len=%d...\n", points_len);
        if(points_len > MBOOT_CHK_POINTS_NUMBER && m_once == 0)
        {
            // -- parsing mboot time --------------------------------------------->
            // --
            {
                char str[STRING_BUF_SIZE], *pbuf = NULL;
                int count = 0;

                str_mb_ver[0] = END_CHARACTER;
                pStart = mbuf;
                for(i = 0; i < MBOOT_CHK_POINTS_NUMBER; i++)
                {
                    pStart = strstr (pStart, MBOOT_PREFIX_STR);
                    if (pStart != NULL)
                    {
                        pMid = strchr (pStart, '=');
                        pEnd = strchr (pStart, ' ');

                        memset(str, 0x00, STRING_BUF_SIZE);
                        strncpy(str, pStart+strlen(MBOOT_PREFIX_STR), (pMid - (pStart+strlen(MBOOT_PREFIX_STR))));
                        str[(pMid - (pStart+strlen(MBOOT_PREFIX_STR)))] = END_CHARACTER;

                        strncpy(str_val, pMid+1, (pEnd - pMid -1));
                        str_val[(pEnd - pMid - 1)] = END_CHARACTER;

                        //printk(KERN_NOTICE "SH@ MB ver i=%d, %s=%s\n", i, str, str_val);

                        if(!strcmp(str, (char *)MB_VER_STR))
                        {
                            //printk(KERN_NOTICE "SH@ get MB ver succeed\n");
                            strncpy(str_mb_ver, str_val, strlen(str_val)+1);
                            pStart = pEnd;
                            continue;
                        }

                        // put to array
                        sprintf(m_tbuf, "[MB][%s][%s]\n", str, str_val);
                        count = strlen(m_tbuf);
                        if (!(pbuf = kmalloc(count + 1, GFP_KERNEL)))
                        {
                            printk(KERN_NOTICE "SH@ memory alloc failed, %d\n", count+1);
                            mutex_unlock(&cp_lock);
                            return 0;
                        }
                        else
                        {
                            strncpy(pbuf, m_tbuf, count);
                            pbuf[count] = END_CHARACTER;
                            pMPoints[MPoints_len++] = pbuf;
                        }

                        //printk(KERN_NOTICE "\n i=%d, str=%s...\n", i, pMPoints[i]);
                        pStart = pEnd;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            // -- output to a file ------------------------------------------------->
            // --
            {
                char buf[STRING_BUF_SIZE]="";
                int sec = 0, min = 0;
                struct timespec ts;

                // get current sec and msec
                getnstimeofday(&ts);

                min = (ts.tv_sec / SECS_FOR_A_MIN) % SECS_FOR_A_MIN;
                sec = ts.tv_sec % SECS_FOR_A_MIN;

                if(pStr_Path != NULL)
                {
                    sprintf(buf, "%s/boot_%02d_%02d_%s.log", pStr_Path, min, sec, str_cnt);
                }
                else
                {
                    sprintf(buf, "%s/boot_%02d_%02d_%s.log", DIR_SAVE_PATH, min, sec, str_cnt);
                }

                filp = filp_open(buf, O_CREAT|O_TRUNC|O_WRONLY, 0644);      // open in USB
                if (IS_ERR(filp))
                {
                    printk("error occured while opening file %s, exiting...\n", buf);
                    mutex_unlock(&cp_lock);

                    //do system reset/reboot below
                    kernel_restart(NULL);

                    return 0;
                }

                // printk(KERN_NOTICE "SH@ array_len=%d, filename=%s...\n", points_len, buf);

                old_fs = get_fs();
                set_fs(KERNEL_DS);

                if (!IS_ERR(filp))
                {
                    // -- write info --

                    /* CPU freq */
                    #if (defined(CONFIG_MSTAR_MIPS))
                    sprintf(buf, "[CPU freq]%d MHz\n", Chip_Query_MIPS_CLK());
                    #else
                    sprintf(buf, "[CPU freq]%d MHz\n", query_frequency());
                    #endif
                    filp->f_op->write(filp, (char *)buf, strlen(buf), &filp->f_pos);

                    // -- write version --

                    /* IC */
                    if(pStr_Ver_IC == NULL)
                    {
                        sprintf(buf, "[IC ver]%s\n", (char *)"none");
                    }
                    else
                    {
                        sprintf(buf, "[IC ver]%s\n", pStr_Ver_IC);
                    }
                    filp->f_op->write(filp, (char *)buf, strlen(buf), &filp->f_pos);

                    /* mboot */
                    sprintf(buf, "[MB ver]%s\n", str_mb_ver);
                    filp->f_op->write(filp, (char *)buf, strlen(buf), &filp->f_pos);

                    /* kernel */
                    sprintf(buf, "[KR ver]%s", (char *)KERN_CL);    // not written '\n', it has in 'KERN_CL'
                    filp->f_op->write(filp, (char *)buf, strlen(buf), &filp->f_pos);

                    /* supernova */
                    if(pStr_Ver_SN == NULL)
                    {
                        sprintf(buf, "[SN ver]%s\n", (char *)"none");
                    }
                    else
                    {
                        sprintf(buf, "[SN ver]%s\n", pStr_Ver_SN);
                    }
                    filp->f_op->write(filp, (char *)buf, strlen(buf), &filp->f_pos);

                    /* android */
                    if(pStr_Ver_AN == NULL)
                    {
                        sprintf(buf, "[AN ver]%s\n\n", (char *)"none");
                    }
                    else
                    {
                        sprintf(buf, "[AN ver]%s\n\n", pStr_Ver_AN);
                    }
                    filp->f_op->write(filp, (char *)buf, strlen(buf), &filp->f_pos);

                    // -- for Mboot --
                    for(i = 0; i < MBOOT_CHK_POINTS_NUMBER; i++)
                    {
                        if(i >= MPoints_len)
                        {
                            break;
                        }
                        else
                        {
                            off = 0;
                            // printk(KERN_NOTICE "SH@, mboot --> (%d), (%s)\n", i, pMPoints[i]);
                            filp->f_op->write(filp, (char *)pMPoints[i], strlen(pMPoints[i]), &filp->f_pos);
                        }
                    }

                    // -- for Kernel --
                    sprintf(buf, "[KR][start init][%d]\n", kr_PiuTime);
                    filp->f_op->write(filp, (char *)buf, strlen(buf), &filp->f_pos);

                    // -- for SN and AN --
                    for(i = 0; i < POINTS_NUMBERS; i++)
                    {
                        if(i >= points_len)
                        {
                            break;
                        }
                        else
                        {
                            off = 0;
                            // printk(KERN_NOTICE "SH@, SN,AN --> (%d), (%s)\n", i, pPoints[i]);
                            filp->f_op->write(filp, (char *)pPoints[i], strlen(pPoints[i]), &filp->f_pos);
                        }
                    }

                    filp->f_op->fsync(filp, 0, MAX_FILE_SIZE, 0);
                }

                set_fs(old_fs);

                if(filp != NULL)
                {
                    filp_close(filp, NULL);
                }
            }

            // -- free --------------------------------------------------------------->
            // --

            // for Mboot
            for(i = 0; i < POINTS_NUMBERS; i++)
            {
                if(i >= MPoints_len)
                {
                    MPoints_len = 0;
                    break;
                }

                // free
                if(pMPoints[i] != NULL)
                {
                    kfree(pMPoints[i]);
                    pMPoints[i] = NULL;
                }
            }

            // for Kernel, SN and AN
            for(i = 0; i < POINTS_NUMBERS; i++)
            {
                if(i >= points_len)
                {
                    points_len = 0;
                    break;
                }

                // free
                if(pPoints[i] != NULL)
                {
                    kfree(pPoints[i]);
                    pPoints[i] = NULL;
                }
            }

            // free
            if(pStr_Path != NULL)
            {
                kfree(pStr_Path);
                pStr_Path = NULL;
            }

            if(pStr_Ver_SN != NULL)
            {
                kfree(pStr_Ver_SN);
                pStr_Ver_SN = NULL;
            }

            if(pStr_Ver_AN != NULL)
            {
                kfree(pStr_Ver_AN);
                pStr_Ver_AN = NULL;
            }

            if(pStr_Ver_IC != NULL)
            {
                kfree(pStr_Ver_IC);
                pStr_Ver_IC = NULL;
            }

            if(pBuf_Bootargs != NULL)
            {
                kfree(pBuf_Bootargs);
                pBuf_Bootargs = NULL;
            }

            m_once = 1;

            // reboot
            printk(KERN_NOTICE "SH@ reboot for write boot check-points (cnt: %s)\n", str_cnt);
            //do system reset/reboot below
            kernel_restart(NULL);
        }
    }
    else
    {
        printk(KERN_NOTICE "SH@ failed for device ioctl\n");
        mutex_unlock(&cp_lock);
        return -ENOTTY;
    }

    mutex_unlock(&cp_lock);
    return 0;
}

static int drv_release(struct inode *inode, struct file *filp)
{
    //printk(KERN_NOTICE "SH@ device close\n");
    //printk(KERN_NOTICE "\n\nSH@ CLOSE <==========================================>\n\n");
    return 0;
}

struct file_operations drv_fops =
{
    read:                   drv_read,
    write:                  drv_write,
    unlocked_ioctl:       drv_ioctl,
    open:                  drv_open,
    release:               drv_release,
};


/*
 *
 */

static int points_handler_init(void)
{
    if (register_chrdev(MAJOR_NUM, DEV_NAME, &drv_fops) < 0)
    {
        printk(KERN_NOTICE "<1>%s: can't get major %d (SH@)\n", MODULE_NAME, MAJOR_NUM);
        return (-EBUSY);
    }

    mutex_init(&cp_lock);
    printk(KERN_NOTICE "<1>%s: started (SH@)\n", MODULE_NAME);

    return 0;
}

static void points_handler_exit(void)
{
    mutex_destroy(&cp_lock);

    unregister_chrdev(MAJOR_NUM, DEV_NAME);
    printk(KERN_NOTICE "<1>%s: removed (SH@)\n", MODULE_NAME);
}

/*
 *
 */

module_init(points_handler_init);
module_exit(points_handler_exit);



