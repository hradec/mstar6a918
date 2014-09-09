/*
 * Compressed RAM block device
 *
 * Copyright (C) 2008, 2009, 2010  Nitin Gupta
 *
 * This code is released using a dual license strategy: BSD/GPL
 * You can choose the licence that better fits your requirements.
 *
 * Released under the terms of 3-clause BSD License
 * Released under the terms of GNU General Public License Version 2.0
 *
 * Project home: http://compcache.googlecode.com/
 */

#include <linux/device.h>
#include <linux/genhd.h>    
#include <linux/mm.h>
#include "xzram_drv.h"

static unsigned set_pool_id = 1;

static u64 xzram_stat64_read(struct xzram *xzram, u64 *v)
{
    u64 val;

    spin_lock(&xzram->stat64_lock);
    val = *v;
    spin_unlock(&xzram->stat64_lock);

    return val;
}

static struct xzram *dev_to_xzram(struct device *dev)
{
    int i;
    struct xzram *xzram = NULL;

    for (i = 0; i < xzram_num_devices; i++) {
        xzram = &xzram_devices[i];
        if (disk_to_dev(xzram->disk) == dev)
            break;
    }

    return xzram;
}

static ssize_t disksize_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct xzram *xzram = dev_to_xzram(dev);

    return sprintf(buf, "%llu\n", xzram->disksize);
}

static ssize_t disksize_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t len)
{
    int ret;
    struct xzram *xzram = dev_to_xzram(dev);

    if (xzram->init_done) {
        pr_info("Cannot change disksize for initialized device\n");
        return -EBUSY;
    }

    ret = strict_strtoull(buf, 10, &xzram->disksize);
    if (ret)
        return ret;

    xzram->disksize = PAGE_ALIGN(xzram->disksize);
    set_capacity(xzram->disk, xzram->disksize >> SECTOR_SHIFT);

    return len;
}

static ssize_t initstate_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct xzram *xzram = dev_to_xzram(dev);

    return sprintf(buf, "%u\n", xzram->init_done);
}

static ssize_t reset_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t len)
{
    int ret;
    unsigned long do_reset;
    struct xzram *xzram;
    struct block_device *bdev;

    xzram = dev_to_xzram(dev);
    bdev = bdget_disk(xzram->disk, 0);

    /* Do not reset an active device! */
    if (bdev->bd_holders)
        return -EBUSY;

    ret = strict_strtoul(buf, 10, &do_reset);
    if (ret)
        return ret;

    if (!do_reset)
        return -EINVAL;

    /* Make sure all pending I/O is finished */
    if (bdev)
        fsync_bdev(bdev);

    if (xzram->init_done)
        xzram_reset_device(xzram);

    return len;
}

static ssize_t num_reads_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct xzram *xzram = dev_to_xzram(dev);

    return sprintf(buf, "%llu\n",
        xzram_stat64_read(xzram, &xzram->stats.num_reads));
}

static ssize_t num_writes_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct xzram *xzram = dev_to_xzram(dev);

    return sprintf(buf, "%llu\n",
        xzram_stat64_read(xzram, &xzram->stats.num_writes));
}

static ssize_t invalid_io_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct xzram *xzram = dev_to_xzram(dev);

    return sprintf(buf, "%llu\n",
        xzram_stat64_read(xzram, &xzram->stats.invalid_io));
}

static ssize_t notify_free_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct xzram *xzram = dev_to_xzram(dev);

    return sprintf(buf, "%llu\n",
        xzram_stat64_read(xzram, &xzram->stats.notify_free));
}

static ssize_t zero_pages_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct xzram *xzram = dev_to_xzram(dev);

    return sprintf(buf, "%u\n", xzram->stats.pages_zero);
}

static ssize_t orig_data_size_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct xzram *xzram = dev_to_xzram(dev);

    return sprintf(buf, "%llu\n",
        (u64)(xzram->stats.pages_stored) << PAGE_SHIFT);
}

static ssize_t compr_data_size_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct xzram *xzram = dev_to_xzram(dev);

    return sprintf(buf, "%llu\n",
        xzram_stat64_read(xzram, &xzram->stats.compr_size));
}

static ssize_t mem_used_total_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    u64 val = 0;
        int i;

    struct xzram *xzram = dev_to_xzram(dev);

    if (xzram->init_done) {

                for(i=0;i<MAX_POOL_NUM;i++)
                {
                    if(xzram->pool_info[i].activate)
                       val += ext_xv_get_total_size_bytes(xzram->xv_mem_pool[i]);
                }

        val +=    ((u64)(xzram->stats.pages_expand) << PAGE_SHIFT);
    }

    return sprintf(buf, "%llu\n", val);
}

static ssize_t  pool_set_id_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t len)
{
    int ret;
        u64 id;

    struct xzram *xzram = dev_to_xzram(dev);

    down_write(&xzram->lock);

    ret = strict_strtoull(buf, 10, &id);

        set_pool_id = (u32) id;

        if(id == 0)
        {
            pr_info("Pool id 0 is default set to internal, do not change it\n");
            up_write(&xzram->lock);
            return -EINVAL;
        }

        xzram->pool_info[set_pool_id].pool_id = set_pool_id;

        pr_info("Change set pool id to %d\n", set_pool_id);

        if ( set_pool_id > MAX_POOL_NUM ) {
                pr_info("Warnning max pool id is = %d!!\n", MAX_POOL_NUM);
                up_write(&xzram->lock);
                return -EINVAL;
        }

        up_write(&xzram->lock);

    return len;
}

static ssize_t  pool_set_type_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t len)
{
    int ret;
    struct xzram *xzram = dev_to_xzram(dev);
        u64 type;

    if (xzram->init_done) {
        pr_info("Cannot set pool type for initialized device\n");
        return -EBUSY;
    }

    ret = strict_strtoull(buf, 10, &type);

        if((type > 1) || (type < 0))
        {
             pr_info("Warnning type can be only assign to 0(internal) or 1(external), default set to internal\n");
             xzram->pool_info[set_pool_id].type = 0;
        }
        else
        {
            if(type == 0)
                xzram->pool_info[set_pool_id].type = 0;   // internal pool with limited size
            else
                xzram->pool_info[set_pool_id].type = 1;   // external pool with limited size
        }

    return len;
}

static ssize_t  pool_set_ba_start_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t len)
{
    int ret;
    struct xzram *xzram = dev_to_xzram(dev);

    if (xzram->init_done) {
        pr_info("Cannot set bus address for initialized device\n");
        return -EBUSY;
    }

    ret = strict_strtoull(buf, 10, &xzram->pool_info[set_pool_id].ext_ba_start);

    return len;
}

static ssize_t  pool_set_size_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t len)
{
    int ret;
    struct xzram *xzram = dev_to_xzram(dev);
        u64 size;

    if (xzram->init_done) {
        pr_info("Cannot set pool size for initialized device\n");
        return -EBUSY;
    }

    ret = strict_strtoull(buf, 10, &size);

        xzram->pool_info[set_pool_id].ext_size = (u32) size;

    return len;
}

static ssize_t  pool_set_activate_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t len)
{
    int ret;
    struct xzram *xzram = dev_to_xzram(dev);
    u64 activate;

    if (xzram->init_done) {
        pr_info("Cannot set pool size for initialized device\n");
        return -EBUSY;
    }

    ret = strict_strtoull(buf, 10, &activate);

    if(activate ==1)
    {
        xzram->pool_info[set_pool_id].activate = (u8) 1;
    }

    return len;
}
static ssize_t  pool_set_migrate_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t len)
{
    int ret = 0;
    struct xzram *xzram = dev_to_xzram(dev);
    u64 migration;
    void __iomem **tmp_array = NULL;
    unsigned int *tmp_bitmap = NULL;
    u32 migration_type, pool_type;
    struct ext_xv_backuplist backuplist;
    ret = strict_strtoull(buf, 10, &migration);
    unsigned int firstrun=0, cnt;

    backuplist.backup_page = NULL;
    backuplist.cnt = 0;

    if (!xzram->init_done) {
        pr_info("Device not been initialed!\n");
        return -EBUSY;
    }

    if(xzram->pool_migration_stat != MIGRATION_STAT_NO_JOB)
    {
        pr_info("xzram migration is busy [%d]\n", xzram->pool_migration_stat);
        goto migration_fail_init;	
    }

    down_write(&xzram->lock);
    xzram->pool_migration_stat = MIGRATION_STAT_ONGO;
    up_write(&xzram->lock);

    switch (migration)
    {
        case 0:
        {
            if(ext_xv_is_ext_pool(xzram->xv_mem_pool[set_pool_id]))
            {
                migration_type = (u32)EXT_XVPOOL_INTERNAL;
                pool_type = 0;
            }
            else
            {
                pr_info("Pool [%d] type is already been internal\n", set_pool_id);
                goto migration_fail_init;
            }

            break;
        }
        case 1:
        {
            if(ext_xv_is_dyn_int_pool(xzram->xv_mem_pool[set_pool_id]))
            {
                migration_type = (u32)EXT_XVPOOL_EXTERNAL;
                pool_type = 1;
            }
            else
            {
                pr_info("Pool [%d] type is already been internal\n", set_pool_id);
                goto migration_fail_init;
            }

            break;
        }
        default:
            pr_info("error command %d\n", (int) migration);
            goto migration_fail_init;
        break;
    }

    tmp_bitmap = ext_xv_create_bitmap(xzram->xv_mem_pool[set_pool_id]);

    if(!tmp_bitmap)
    {
        pr_info("create bitmap fail...\n");
        goto migration_fail_nolock;
    }

    down_write(&xzram->lock);
    ext_xv_set_migration(xzram->xv_mem_pool[set_pool_id]);
    ext_xv_bitmap_duplicate(xzram->xv_mem_pool[set_pool_id], tmp_bitmap);
    up_write(&xzram->lock);

    tmp_array = ext_xv_create_pool_array(xzram->xv_mem_pool[set_pool_id], migration_type, tmp_bitmap);

    if(!tmp_array)
    {
        pr_info("create tmp_array fail...\n");
        goto migration_end_nolock;
    }

fill_backuplist:
    if(firstrun++)
    {
        if(!ext_xv_add_page_to_backuplist(&backuplist, cnt))
        {
            pr_info("add page to backuplist fail...\n");
            goto migration_backup_fail;
        }

        ext_xv_sub_write_cnt(xzram->xv_mem_pool[set_pool_id], cnt);
    }

    down_write(&xzram->lock);

    cnt = ext_xv_get_write_cnt(xzram->xv_mem_pool[set_pool_id]);

    if(cnt)
    {
        up_write(&xzram->lock);
        goto fill_backuplist;
    }

    tmp_array = ext_xv_pool_migration(xzram->xv_mem_pool[set_pool_id],
                    migration_type, tmp_array, tmp_bitmap, &backuplist);

    if(!tmp_array)
    {
        pr_info("migrate fail...\n");
        xzram->pool_migration_stat = MIGRATION_STAT_FAIL;
        goto migration_end;
    }
    
    xzram->pool_migration_stat = MIGRATION_STAT_DONE;
    xzram->pool_info[set_pool_id].type = pool_type;
    
migration_end:
    ext_xv_delete_pool_array(xzram->xv_mem_pool[set_pool_id], tmp_array);

    ext_xv_delete_bitmap(tmp_bitmap);
    cnt = ext_xv_get_write_cnt(xzram->xv_mem_pool[set_pool_id]);

    ext_xv_sub_write_cnt(xzram->xv_mem_pool[set_pool_id], cnt);
    cnt = ext_xv_get_read_cnt(xzram->xv_mem_pool[set_pool_id]);
    //if(cnt != 0)
    //    pr_info("XXX read cnt = %d\n", cnt);

    ext_xv_sub_read_cnt(xzram->xv_mem_pool[set_pool_id], cnt);
    ext_xv_clear_migration(xzram->xv_mem_pool[set_pool_id]);
  
    up_write(&xzram->lock);
    return len;

migration_backup_fail:
    ext_xv_delete_pool_array(xzram->xv_mem_pool[set_pool_id], tmp_array);

migration_end_nolock:
    ext_xv_delete_bitmap(tmp_bitmap);

    cnt = ext_xv_get_write_cnt(xzram->xv_mem_pool[set_pool_id]);
    ext_xv_sub_write_cnt(xzram->xv_mem_pool[set_pool_id], cnt);
    cnt = ext_xv_get_read_cnt(xzram->xv_mem_pool[set_pool_id]);
    ext_xv_sub_read_cnt(xzram->xv_mem_pool[set_pool_id], cnt);
    ext_xv_clear_migration(xzram->xv_mem_pool[set_pool_id]);

migration_fail_nolock:
migration_fail_init:
	  xzram->pool_migration_stat = MIGRATION_STAT_FAIL;
    return len;
}

enum xzram_info_num{
     XZRAM_INFO_HELP,                    //0
     XZRAM_INFO_PAGE_USAGE,
     XZRAM_INFO_COMPRESS_RATIO,
     XZRAM_INFO_FREELIST_BLOCK,
     XZRAM_INFO_TEST,
};

static ssize_t  xzram_info(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t len)
{
    int ret;
    u64 val;
    struct xzram *xzram = dev_to_xzram(dev);

        if (!xzram->init_done) {
            pr_info("Cannot query xzram information before initialation\n");
            return -EBUSY;
       }

        ret = strict_strtoull(buf, 10, &val);
        down_read(&xzram->lock);

        switch(val)
        {
            case XZRAM_INFO_PAGE_USAGE:
            {
                int i;
                u64 cnt=0;
                        pr_info("=========================================================\n");
                        pr_info("Pool Id = 0\n");
                        pr_info("Type default internal\n");
                        pr_info("Page usaged = %llu\n", ext_xv_show_total_pages(xzram->xv_mem_pool[0]));

                for(i=1;i<MAX_POOL_NUM;i++)
                {
                    if(xzram->pool_info[i].activate)
                    {
                        pr_info("--------------------------------------------------------\n");
                        pr_info("Pool Id = %d\n", xzram->pool_info[i].pool_id);
                        pr_info("Type = %s\n", xzram->pool_info[i].type? "Externl":"Internal");

                        if(xzram->pool_info[i].type)
                            pr_info("Bus address = %llx\n", xzram->pool_info[i].ext_ba_start);

                        pr_info("Pool size = %d\n", xzram->pool_info[i].ext_size);
                        pr_info("Page usaged = %llu\n", ext_xv_show_total_pages(xzram->xv_mem_pool[i]));

                    }
                }

                for(i=0;i<xzram->disksize>>PAGE_SHIFT;i++)
                {
                    if(xzram_test_flag(xzram, i, XZRAM_UNCOMPRESSED))
                    {
                       cnt++;
                    }
                }
                       pr_info("=========================================================\n");
                       pr_info("Page from kernel(uncompressed pages) = %llu pages\n", cnt);
                       pr_info("=========================================================\n");

                break;
            }
#ifdef CONFIG_XZRAM_DEBUG
            case XZRAM_INFO_COMPRESS_RATIO:
            {
                u64 ratio = 0, num_w, ratio_cnt;

                spin_lock(&xzram->stat64_lock);
                num_w = xzram->stats.num_writes;
                ratio_cnt = ratio = xzram->stats.compress_ratio_cnt;
                spin_unlock(&xzram->stat64_lock);

                do_div(ratio, num_w);
                pr_info("===================================================\n");
                pr_info("Total write number = %llu \n", num_w);
                pr_info("Currently compressed cnt = %llu \n", ratio_cnt);
                pr_info("Compress ratio = %llu %%\n", 100 - (100*ratio/PAGE_SIZE));
                pr_info("===================================================\n");

                break;
}
#endif
            case XZRAM_INFO_FREELIST_BLOCK:
            {
                pr_info("===================================================\n");
                pr_info("Pool Id = %d\n", set_pool_id);
                ext_xv_show_block_info(xzram->xv_mem_pool[set_pool_id]);                
                pr_info("===================================================\n");
                break;
            }

            case XZRAM_INFO_TEST:
            {

                break;
            }
                
            case XZRAM_INFO_HELP:

            default:
                pr_info("===================================================\n");
                pr_info("xZram xzram_info helper\n");
                pr_info("write [n] to xzram_info means:\n");
                pr_info("%d: Helper\n", XZRAM_INFO_HELP);
                pr_info("%d: xZram Internal/external memory usage\n", XZRAM_INFO_PAGE_USAGE);
                #ifdef CONFIG_XZRAM_DEBUG
                pr_info("%d: Dump zram compress ratio\n", XZRAM_INFO_COMPRESS_RATIO);
                #endif 
                pr_info("%d: Dump current block usage\n", XZRAM_INFO_FREELIST_BLOCK);
                pr_info("===================================================\n");

            break;
        }

        up_read(&xzram->lock);
    return len;
}

#ifdef CONFIG_XZRAM_COMPRESS_PERFORMANCE_STAT
static ssize_t  compress_performance_stat_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct xzram *xzram = dev_to_xzram(dev);

    return sprintf(buf, "write count:\t%llu\n" "read count:\t%llu\n"
                           "encode count:\t%llu\n" "decode count:\t%llu\n"
                          "drop count:\t%llu\n"
                          "write time:\t%llu us\n" "read time:\t%llu us\n"
                          "encode time:\t%llu us\n" "decode time:\t%llu us\n",
                           xzram->stats.num_writes, xzram->stats.num_reads,
                           xzram->stats.cnt_enc, xzram->stats.cnt_dec,
                           xzram->stats.cnt_drop,
                           xzram->stats.time_wr, xzram->stats.time_rd,
                           xzram->stats.time_enc, xzram->stats.time_dec);
}
#endif

static ssize_t pool_set_migrate_stat_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t len)
{
    int ret;
    struct xzram *xzram = dev_to_xzram(dev);
    int val;

    if (!xzram->init_done) {
        pr_info("Cannot set migration store while device is not ready\n");
        return -EBUSY;
    }
  
    ret = strict_strtoull(buf, 10, &val);
 
    down_write(&xzram->lock);
    
    if(val != MIGRATION_STAT_NO_JOB)
    {
        pr_info("Only value 0 is acceptable \n");
        up_write(&xzram->lock);
        return len;
    }
    
    if(xzram->pool_migration_stat == MIGRATION_STAT_ONGO)
    {
        pr_info("migration is on process!! \n");
        up_write(&xzram->lock);
        return len;    	
    }
    
    if((xzram->pool_migration_stat == MIGRATION_STAT_DONE)|| 
    	 (xzram->pool_migration_stat == MIGRATION_STAT_FAIL))
    {
        xzram->pool_migration_stat = MIGRATION_STAT_NO_JOB;
    }
    
    up_write(&xzram->lock);

    return len;
}

static ssize_t pool_set_migrate_stat_show(struct device *dev,
                     struct device_attribute *attr, char *buf)
{
    struct xzram *xzram = dev_to_xzram(dev);
    return sprintf(buf, "%ld\n", xzram->pool_migration_stat);	
}                     

static DEVICE_ATTR(disksize, S_IRUGO | S_IWUSR, disksize_show, disksize_store);
static DEVICE_ATTR(initstate, S_IRUGO, initstate_show, NULL);
static DEVICE_ATTR(reset, S_IWUSR, NULL, reset_store);
static DEVICE_ATTR(num_reads, S_IRUGO, num_reads_show, NULL);
static DEVICE_ATTR(num_writes, S_IRUGO, num_writes_show, NULL);
static DEVICE_ATTR(invalid_io, S_IRUGO, invalid_io_show, NULL);
static DEVICE_ATTR(notify_free, S_IRUGO, notify_free_show, NULL);
static DEVICE_ATTR(zero_pages, S_IRUGO, zero_pages_show, NULL);
static DEVICE_ATTR(orig_data_size, S_IRUGO, orig_data_size_show, NULL);
static DEVICE_ATTR(compr_data_size, S_IRUGO, compr_data_size_show, NULL);
static DEVICE_ATTR(mem_used_total, S_IRUGO, mem_used_total_show, NULL);
static DEVICE_ATTR(xzram_info, S_IWUSR, NULL, xzram_info);
static DEVICE_ATTR(pool_set_id, S_IWUSR, NULL, pool_set_id_store);
static DEVICE_ATTR(pool_set_type, S_IWUSR, NULL, pool_set_type_store);
static DEVICE_ATTR(pool_set_ba_start, S_IWUSR, NULL, pool_set_ba_start_store);
static DEVICE_ATTR(pool_set_size, S_IWUSR, NULL, pool_set_size_store);
static DEVICE_ATTR(pool_set_activate, S_IWUSR, NULL, pool_set_activate_store);
static DEVICE_ATTR(pool_set_migrate, S_IWUSR, NULL, pool_set_migrate_store);
static DEVICE_ATTR(pool_set_migrate_stat, S_IRUGO | S_IWUSR, pool_set_migrate_stat_show, pool_set_migrate_stat_store);

#ifdef CONFIG_XZRAM_COMPRESS_PERFORMANCE_STAT
static DEVICE_ATTR(compress_performance_stat, S_IRUGO, compress_performance_stat_show, NULL);
#endif

static struct attribute *xzram_disk_attrs[] = {
    &dev_attr_disksize.attr,
    &dev_attr_initstate.attr,
    &dev_attr_reset.attr,
    &dev_attr_num_reads.attr,
    &dev_attr_num_writes.attr,
    &dev_attr_invalid_io.attr,
    &dev_attr_notify_free.attr,
    &dev_attr_zero_pages.attr,
    &dev_attr_orig_data_size.attr,
    &dev_attr_compr_data_size.attr,
    &dev_attr_mem_used_total.attr,
    &dev_attr_xzram_info.attr,
    &dev_attr_pool_set_id.attr,
    &dev_attr_pool_set_type.attr,
    &dev_attr_pool_set_ba_start.attr,
    &dev_attr_pool_set_size.attr,
    &dev_attr_pool_set_activate.attr,
    &dev_attr_pool_set_migrate.attr,
    &dev_attr_pool_set_migrate_stat.attr,
#ifdef CONFIG_XZRAM_COMPRESS_PERFORMANCE_STAT
    &dev_attr_compress_performance_stat.attr,
#endif
    NULL,
};

struct attribute_group xzram_disk_attr_group = {
    .attrs = xzram_disk_attrs,
};
