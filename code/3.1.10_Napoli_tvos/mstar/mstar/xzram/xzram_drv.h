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
 * Project home: http://compcache.googlecode.com
 */

#ifndef _XZRAM_DRV_H_
#define _XZRAM_DRV_H_

#include <linux/spinlock.h>
#include <linux/mutex.h>

#include "ext_xvmalloc.h"

/*
 * Some arbitrary value. This is just to catch
 * invalid value for num_devices module parameter.
 */
static const unsigned max_num_devices = 32;


#define MIGRATION_STAT_NO_JOB  0
#define MIGRATION_STAT_ONGO    1
#define MIGRATION_STAT_DONE    2
#define MIGRATION_STAT_FAIL    3

#define NO_POOL_ID            -1

/*
 * Stored at beginning of each compressed object.
 *
 * It stores back-reference to table entry which points to this
 * object. This is required to support memory defragmentation.
 */
struct zobj_header {
#if 0
	u32 table_idx;
#endif
};

/*-- Configurable parameters */

/* Default zram disk size: 25% of total RAM */
static const unsigned default_disksize_perc_ram = 25;

/*
 * Pages that compress to size greater than this are stored
 * uncompressed in memory.
 */
static const unsigned max_zpage_size = PAGE_SIZE / 4 * 3;
//static const unsigned max_zpage_size = PAGE_SIZE;

#define MAX_POOL_NUM  4

/*
 * NOTE: max_zpage_size must be less than or equal to:
 *   XV_MAX_ALLOC_SIZE - sizeof(struct zobj_header)
 * otherwise, xv_malloc() would always return failure.
 */

/*-- End of configurable params */

#define SECTOR_SHIFT		9
#define SECTOR_SIZE		(1 << SECTOR_SHIFT)
#define SECTORS_PER_PAGE_SHIFT	(PAGE_SHIFT - SECTOR_SHIFT)
#define SECTORS_PER_PAGE	(1 << SECTORS_PER_PAGE_SHIFT)
#define XZRAM_LOGICAL_BLOCK_SHIFT 12
#define XZRAM_LOGICAL_BLOCK_SIZE	(1 << XZRAM_LOGICAL_BLOCK_SHIFT)
#define XZRAM_SECTOR_PER_LOGICAL_BLOCK	\
	(1 << (XZRAM_LOGICAL_BLOCK_SHIFT - SECTOR_SHIFT))

/* Flags for zram pages (table[page_no].flags) */
enum xzram_pageflags {
	/* Page is stored uncompressed */
	XZRAM_UNCOMPRESSED,

	/* Page consists entirely of zeros */
	XZRAM_ZERO,

	__NR_XZRAM_PAGEFLAGS,
};

/*-- Data structures */

/* Allocated for each disk page */
struct table {
	struct page *page;
	u16 offset;
	u8 count;	/* object ref count (not yet used) */
	u8 flags;
    s8 pool_id;
    bool notify;
	u32 notify_id;
} __attribute__((aligned(4)));

struct xzram_stats {
	u64 compr_size;		/* compressed size of pages stored */
	u64 num_reads;		/* failed + successful */
	u64 num_writes;		/* --do-- */
	u64 failed_reads;	/* should NEVER! happen */
	u64 failed_writes;	/* can happen when memory is too low */
	u64 invalid_io;		/* non-page-aligned I/O requests */
	u64 notify_free;	/* no. of swap slot free notifications */
	u32 pages_zero;		/* no. of zero filled pages */
	u32 pages_stored;	/* no. of pages currently stored */
	u32 good_compress;	/* % of pages with compression ratio<=50% */
	u32 pages_expand;	/* % of incompressible pages */
 
#ifdef CONFIG_XZRAM_DEBUG
    u64 compress_ratio_cnt;
#endif

#ifdef CONFIG_XZRAM_COMPRESS_PERFORMANCE_STAT
    u64 cnt_enc;
    u64 cnt_dec;
    u64 cnt_drop;
    u64 time_rd;
    u64 time_wr;
    u64 time_enc;
    u64 time_dec;
#endif
};

struct  ext_pool_info{
        s8 pool_id;
        u8  type;
        u64 ext_ba_start;   /*0 for internal*/
        u32 ext_size;       /*0 for internal*/
        u8 activate;
};

struct notify_handler{    
	struct xzram *xzram;       
	u32 index;       
	struct work_struct       wq_work;       
	struct workqueue_struct *wq;
};

#define ENQUEUE_OK 1
#define ENQUEUE_FAIL -1
#define DEQUEUE_EMPTY -1

struct xzram {
  struct ext_pool_info  *pool_info;
	struct ext_xv_pool    **xv_mem_pool;
	void *compress_workmem;
	void *compress_buffer;
	struct table *table;
	spinlock_t stat64_lock;	/* protect 64-bit stats */
	struct rw_semaphore lock; /* protect compression buffers and table
				   * against concurrent read and writes */
  spinlock_t notify_lock;
	struct request_queue *queue;
	struct gendisk *disk;
	int init_done;
	/* Prevent concurrent execution of device init and reset */
	struct mutex init_lock;
	/*
	 * This is the limit on amount of *uncompressed* worth of data
	 * we can store in a disk.
	 */
	u64 disksize;	/* bytes */
	struct xzram_stats stats;
	struct notify_handler *free_handler;
	int index_num;
  u8 pool_migration_stat;
};

extern struct xzram *xzram_devices;
extern unsigned int xzram_num_devices;
#ifdef CONFIG_SYSFS
extern struct attribute_group xzram_disk_attr_group;
#endif

extern int xzram_init_device(struct xzram *xzram);
extern void xzram_reset_device(struct xzram *xzram);
extern int xzram_test_flag(struct xzram *xzram, u32 index,
                        enum xzram_pageflags flag);

#endif
