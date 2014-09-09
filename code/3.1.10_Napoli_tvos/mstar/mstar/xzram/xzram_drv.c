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

#define KMSG_COMPONENT "xzram"
#define pr_fmt(fmt) KMSG_COMPONENT ": " fmt

#ifdef CONFIG_XZRAM_DEBUG
#define DEBUG
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/bio.h>
#include <linux/bitops.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/device.h>
#include <linux/genhd.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/vmalloc.h>

#include "xzram_drv.h"
#include "xzram_codec_wrapper.h"

/* Globals */
static int xzram_major;
struct xzram *xzram_devices;

/* Module params (documentation at end) */
unsigned int xzram_num_devices;

static void xzram_stat_inc(u32 *v)
{
	*v = *v + 1;
}

static void xzram_stat_dec(u32 *v)
{
	*v = *v - 1;
}

static void xzram_stat64_add(struct xzram *xzram, u64 *v, u64 inc)
{
	spin_lock(&xzram->stat64_lock);
	*v = *v + inc;
	spin_unlock(&xzram->stat64_lock);
}

static void xzram_stat64_sub(struct xzram *xzram, u64 *v, u64 dec)
{
	spin_lock(&xzram->stat64_lock);
	*v = *v - dec;
	spin_unlock(&xzram->stat64_lock);
}

static void xzram_stat64_inc(struct xzram *xzram, u64 *v)
{
	xzram_stat64_add(xzram, v, 1);
}

int xzram_test_flag(struct xzram *xzram, u32 index,
			enum xzram_pageflags flag)
{
	return xzram->table[index].flags & BIT(flag);
}

static void xzram_set_flag(struct xzram *xzram, u32 index,
			enum xzram_pageflags flag)
{
	xzram->table[index].flags |= BIT(flag);
}

static void xzram_clear_flag(struct xzram *xzram, u32 index,
			enum xzram_pageflags flag)
{
	xzram->table[index].flags &= ~BIT(flag);
}

static int page_zero_filled(void *ptr)
{
	unsigned int pos;
	unsigned long *page;

	page = (unsigned long *)ptr;

	for (pos = 0; pos != PAGE_SIZE / sizeof(*page); pos++) {
		if (page[pos])
			return 0;
	}

	return 1;
}

//extern void Chip_Flush_Cache_Range(unsigned long u32Addr, unsigned long u32Size);

static void __iomem * xzram_map(void __iomem * addr, enum km_type type,unsigned long size, struct xzram *xzram, unsigned int index)
{
        int pool_id = xzram->table[index].pool_id;

        if(xzram_test_flag(xzram, index, XZRAM_UNCOMPRESSED))
            return  kmap_atomic((struct page*)addr, type);
        else
            return  ext_xv_get_ptr_atomic(addr, 0, type, xzram->xv_mem_pool[pool_id]);
}

static void xzram_unmap(void __iomem * addr, enum km_type type, struct xzram * xzram, unsigned int index)
{
        int pool_id = xzram->table[index].pool_id;
        struct ext_xv_pool *pool = xzram->xv_mem_pool[pool_id];

        if(xzram_test_flag(xzram, index, XZRAM_UNCOMPRESSED))
            kunmap_atomic(addr, type);
        else
            ext_xv_put_ptr_atomic(addr, type, pool);
}

static void xzram_set_disksize(struct xzram *xzram, size_t totalram_bytes)
{
	if (!xzram->disksize) {
		pr_info(
		"disk size not provided. You can use disksize_kb module "
		"param to specify size.\nUsing default: (%u%% of RAM).\n",
		default_disksize_perc_ram
		);
		xzram->disksize = default_disksize_perc_ram *
					(totalram_bytes / 100);
	}

	if (xzram->disksize > 2 * (totalram_bytes)) {
		pr_info(
		"There is little point creating a xzram of greater than "
		"twice the size of memory since we expect a 2:1 compression "
		"ratio. Note that xzram uses about 0.1%% of the size of "
		"the disk when not in use so a huge xzram is "
		"wasteful.\n"
		"\tMemory Size: %zu kB\n"
		"\tSize you selected: %llu kB\n"
		"Continuing anyway ...\n",
		totalram_bytes >> 10, xzram->disksize
		);
	}

	xzram->disksize &= PAGE_MASK;
}

static void xzram_free_page(struct xzram *xzram, size_t index)
{
	u32 clen;
	void *obj;
        u32 pool_id;

	struct page *page = xzram->table[index].page;
	u32 offset = xzram->table[index].offset;
        pool_id = xzram->table[index].pool_id;

	if (unlikely(!page)) {
		/*
		 * No memory is allocated for zero filled pages.
		 * Simply clear zero page flag.
		 */
		if (xzram_test_flag(xzram, index, XZRAM_ZERO)) {
			xzram_clear_flag(xzram, index, XZRAM_ZERO);
			xzram_stat_dec(&xzram->stats.pages_zero);
		}
		return;
	}

	if (unlikely(xzram_test_flag(xzram, index, XZRAM_UNCOMPRESSED))) {
		clen = PAGE_SIZE;
		__free_page(page);
		xzram_clear_flag(xzram, index, XZRAM_UNCOMPRESSED);
		xzram_stat_dec(&xzram->stats.pages_expand);
		goto out;
	}

        obj = xzram_map( page, KM_USER0, PAGE_SIZE, xzram, index) + offset;

	    clen = ext_xv_get_object_size(obj) - sizeof(struct zobj_header);

        xzram_unmap(obj, KM_USER0, xzram, index);

        ext_xv_free(xzram->xv_mem_pool[pool_id], page, offset);

	if (clen <= PAGE_SIZE / 2)
            xzram_stat_dec(&xzram->stats.good_compress);

out:
	xzram_stat64_sub(xzram, &xzram->stats.compr_size, clen);
	xzram_stat_dec(&xzram->stats.pages_stored);

	xzram->table[index].page = NULL;
	xzram->table[index].offset = 0;
        xzram->table[index].pool_id = NO_POOL_ID;
}

static void handle_zero_page(struct bio_vec *bvec)
{
	struct page *page = bvec->bv_page;
	void *user_mem;

	user_mem = kmap_atomic(page, KM_USER0);
	memset(user_mem + bvec->bv_offset, 0, bvec->bv_len);
	kunmap_atomic(user_mem, KM_USER0);

	flush_dcache_page(page);
}

static void handle_uncompressed_page(struct xzram *xzram, struct bio_vec *bvec,
				     u32 index, int offset)
{
	struct page *page = bvec->bv_page;
	unsigned char *user_mem, *cmem;

	user_mem = kmap_atomic(page, KM_USER0);
	cmem = kmap_atomic(xzram->table[index].page, KM_USER1);

	memcpy(user_mem + bvec->bv_offset, cmem + offset, bvec->bv_len);
	kunmap_atomic(cmem, KM_USER1);
	kunmap_atomic(user_mem, KM_USER0);

	flush_dcache_page(page);
}

static inline int is_partial_io(struct bio_vec *bvec)
{
	return bvec->bv_len != PAGE_SIZE;
}

static int xzram_bvec_read(struct xzram *xzram, struct bio_vec *bvec,
			  u32 index, int offset, struct bio *bio)
{
	int ret;
	size_t clen;
	struct page *page;
	struct zobj_header *zheader;
	unsigned char *user_mem, *cmem, *uncmem = NULL;

	page = bvec->bv_page;

	if (xzram_test_flag(xzram, index, XZRAM_ZERO)) {
		handle_zero_page(bvec);
		return 0;
	}

	/* Requested page is not present in compressed area */
	if (unlikely(!xzram->table[index].page)) {
		pr_debug("Read before write: sector=%lu, size=%u",
			 (ulong)(bio->bi_sector), bio->bi_size);
		handle_zero_page(bvec);
		return 0;
	}

	/* Page is stored uncompressed since it's incompressible */
	if (unlikely(xzram_test_flag(xzram, index, XZRAM_UNCOMPRESSED))) {
		handle_uncompressed_page(xzram, bvec, index, offset);
		return 0;
	}

	if (is_partial_io(bvec)) {
		/* Use  a temporary buffer to decompress the page */
		uncmem = kmalloc(PAGE_SIZE, GFP_KERNEL);
		if (!uncmem) {
			pr_info("Error allocating temp memory!\n");
			return -ENOMEM;
		}
	}

	user_mem = kmap_atomic(page, KM_USER0);
	if (!is_partial_io(bvec))
		uncmem = user_mem;
	clen = PAGE_SIZE;

        cmem = xzram_map((void __iomem *)xzram->table[index].page, KM_USER1, PAGE_SIZE, xzram, index) +
                xzram->table[index].offset;

        #ifdef CONFIG_XZRAM_COMPRESS_PERFORMANCE_STAT
        u64 start = gettime();
        #endif
	ret = decompres_func(cmem + sizeof(*zheader),
				    ext_xv_get_object_size(cmem) - sizeof(*zheader),
				    uncmem, &clen);
        #ifdef CONFIG_XZRAM_COMPRESS_PERFORMANCE_STAT
        xzram_stat64_add(xzram, &xzram->stats.time_dec, gettime() - start);
        xzram_stat64_inc(xzram, &xzram->stats.cnt_dec);
        #endif

	if (is_partial_io(bvec)) {
		memcpy(user_mem + bvec->bv_offset, uncmem + offset,
		       bvec->bv_len);
		kfree(uncmem);
	}

        xzram_unmap((struct page *)cmem, KM_USER1, xzram, index);
	kunmap_atomic(user_mem, KM_USER0);

	/* Should NEVER happen. Return bio error if it does. */
	if (unlikely(ret != ZRAM_CODEC_OK)) {
		pr_err("Decompression failed! err=%d, page=%u\n", ret, index);
		xzram_stat64_inc(xzram, &xzram->stats.failed_reads);
		return ret;
	}

	flush_dcache_page(page);

	return 0;
}

static int xzram_read_before_write(struct xzram *xzram, char *mem, u32 index)
{
	int ret;
	size_t clen = PAGE_SIZE;
	struct zobj_header *zheader;
	unsigned char *cmem;

	if (xzram_test_flag(xzram, index, XZRAM_ZERO) ||
	    !xzram->table[index].page) {
		memset(mem, 0, PAGE_SIZE);
		return 0;
	}

        cmem = xzram_map( (void __iomem *)xzram->table[index].page, KM_USER0, PAGE_SIZE, xzram, index) +
                xzram->table[index].offset;

	/* Page is stored uncompressed since it's incompressible */
	if (unlikely(xzram_test_flag(xzram, index, XZRAM_UNCOMPRESSED))) {
		memcpy(mem, cmem, PAGE_SIZE);
		kunmap_atomic(cmem, KM_USER0);
		return 0;
	}


        #ifdef CONFIG_XZRAM_COMPRESS_PERFORMANCE_STAT
        u64 start = gettime();
        #endif
	ret = decompres_func(cmem + sizeof(*zheader),
				    ext_xv_get_object_size(cmem) - sizeof(*zheader),
				    mem, &clen);
        #ifdef CONFIG_XZRAM_COMPRESS_PERFORMANCE_STAT
        xzram_stat64_add(xzram, &xzram->stats.time_dec, gettime() - start);
        xzram_stat64_inc(xzram, &xzram->stats.cnt_dec);
        #endif

        xzram_unmap((struct page*)cmem, KM_USER0, xzram, index);
	/* Should NEVER happen. Return bio error if it does. */
	if (unlikely(ret != ZRAM_CODEC_OK)) {
		pr_err("Decompression failed! err=%d, page=%u\n", ret, index);
		xzram_stat64_inc(xzram, &xzram->stats.failed_reads);
		return ret;
	}

	return 0;
}

static int xzram_malloc(struct xzram *xzram, u32 size, struct page **page,
                u32 *offset, gfp_t flags)
{
  int i;

  for(i=MAX_POOL_NUM-1;i>=0;i--)
  {
      if(ext_xv_malloc(xzram->xv_mem_pool[i], size,
                          page, offset, flags))
      {
         continue;
      }

      return i;
  }

  return NO_POOL_ID;

}


static int xzram_bvec_write(struct xzram *xzram, struct bio_vec *bvec, u32 index,
			   int offset)
{
	int ret, pool_id;
	u32 store_offset;
	size_t clen;
	struct zobj_header *zheader;
	struct page *page, *page_store;
	unsigned char *user_mem, *cmem, *src, *uncmem = NULL;

	page = bvec->bv_page;
	src = xzram->compress_buffer;

    xzram->table[index].notify_id = -1;

	if (is_partial_io(bvec)) {
		/*
		 * This is a partial IO. We need to read the full page
		 * before to write the changes.
		 */
		uncmem = kmalloc(PAGE_SIZE, GFP_KERNEL);
		if (!uncmem) {
			pr_info("Error allocating temp memory!\n");
			ret = -ENOMEM;
			goto out;
		}
		ret = xzram_read_before_write(xzram, uncmem, index);
		if (ret) {
			kfree(uncmem);
			goto out;
		}
	}

	/*
	 * System overwrites unused sectors. Free memory associated
	 * with this sector now.
	 */
	if (xzram->table[index].page ||
	    xzram_test_flag(xzram, index, XZRAM_ZERO))
	{
	   
		xzram_free_page(xzram, index);
	    if(xzram->table[index].notify == true)
	    {
		    xzram->table[index].notify = false;
	    }
	}
	user_mem = kmap_atomic(page, KM_USER0);

	if (is_partial_io(bvec))
		memcpy(uncmem + offset, user_mem + bvec->bv_offset,
		       bvec->bv_len);
	else
		uncmem = user_mem;

	if (page_zero_filled(uncmem)) {
		kunmap_atomic(user_mem, KM_USER0);
		if (is_partial_io(bvec))
			kfree(uncmem);
		xzram_stat_inc(&xzram->stats.pages_zero);
		xzram_set_flag(xzram, index, XZRAM_ZERO);
		ret = 0;
		goto out;
	}

        #ifdef CONFIG_XZRAM_COMPRESS_PERFORMANCE_STAT
        u64 start = gettime();
        #endif
	ret = compress_func(uncmem, PAGE_SIZE, src, &clen,
			       xzram->compress_workmem);
        #ifdef CONFIG_XZRAM_COMPRESS_PERFORMANCE_STAT
        xzram_stat64_add(xzram, &xzram->stats.time_enc, gettime() - start);
        xzram_stat64_inc(xzram, &xzram->stats.cnt_enc);
        #endif

#ifdef CONFIG_XZRAM_DEBUG
        xzram->stats.compress_ratio_cnt += clen;
#endif

	kunmap_atomic(user_mem, KM_USER0);
	if (is_partial_io(bvec))
			kfree(uncmem);

	if (unlikely(ret != ZRAM_CODEC_OK)) {
		pr_err("Compression failed! err=%d\n", ret);
		goto out;
	}

	/*
	 * Page is incompressible. Store it as-is (uncompressed)
	 * since we do not want to return too many disk write
	 * errors which has side effect of hanging the system.
	 */
	if (unlikely(clen > max_zpage_size)) {
        #ifdef CONFIG_XZRAM_COMPRESS_PERFORMANCE_STAT
        xzram_stat64_inc(xzram, &xzram->stats.cnt_drop);
        #endif
		clen = PAGE_SIZE;
		page_store = alloc_page(GFP_NOIO | __GFP_HIGHMEM);
		if (unlikely(!page_store)) {
			pr_info("Error allocating memory for "
				"incompressible page: %u\n", index);
			ret = -ENOMEM;
			goto out;
		}

		store_offset = 0;
		xzram_set_flag(xzram, index, XZRAM_UNCOMPRESSED);
		xzram_stat_inc(&xzram->stats.pages_expand);
		xzram->table[index].page = page_store;
		src = kmap_atomic(page, KM_USER0);
		goto memstore;
	}

        if(xzram->table[index].notify == true)
        {
            xzram_free_page(xzram, index);
            xzram_stat64_inc(xzram, &xzram->stats.notify_free);
            xzram->table[index].notify = false;
        }

        pool_id = xzram_malloc(xzram, clen + sizeof(*zheader),
		      &xzram->table[index].page, &store_offset,
                                                      GFP_NOIO | __GFP_HIGHMEM);

	if (pool_id < 0)
        {
		          pr_info("Error allocating memory for compressed "
			          "page: %u, size=%zu\n", index, clen);
		          ret = -ENOMEM;
		          goto out;
        }
        else
        {
            xzram->table[index].pool_id = (s8)pool_id;
        }

memstore:
	xzram->table[index].offset = store_offset;

        cmem = xzram_map((void __iomem *)xzram->table[index].page, KM_USER1, PAGE_SIZE, xzram, index) +
                xzram->table[index].offset;
#if 0
	/* Back-reference needed for memory defragmentation */
	if (!zram_test_flag(zram, index, XZRAM_UNCOMPRESSED)) {
		zheader = (struct zobj_header *)cmem;
		zheader->table_idx = index;
		cmem += sizeof(*zheader);
	}
#endif

	memcpy(cmem, src, clen);

        xzram_unmap((struct page*)cmem, KM_USER1, xzram, index);

	if (unlikely(xzram_test_flag(xzram, index, XZRAM_UNCOMPRESSED)))
		kunmap_atomic(src, KM_USER0);

	/* Update stats */
	xzram_stat64_add(xzram, &xzram->stats.compr_size, clen);
	xzram_stat_inc(&xzram->stats.pages_stored);
	if (clen <= PAGE_SIZE / 2)
		xzram_stat_inc(&xzram->stats.good_compress);

	return 0;

out:
	if (ret)
		xzram_stat64_inc(xzram, &xzram->stats.failed_writes);
	return ret;
}

static int xzram_bvec_rw(struct xzram *xzram, struct bio_vec *bvec, u32 index,
			int offset, struct bio *bio, int rw)
{
	int ret;

	if (rw == READ) {
		down_read(&xzram->lock);
		ret = xzram_bvec_read(xzram, bvec, index, offset, bio);
		up_read(&xzram->lock);
	} else {
		down_write(&xzram->lock);
		ret = xzram_bvec_write(xzram, bvec, index, offset);
		up_write(&xzram->lock);
	}

	return ret;
}

static void update_position(u32 *index, int *offset, struct bio_vec *bvec)
{
	if (*offset + bvec->bv_len >= PAGE_SIZE)
		(*index)++;
	*offset = (*offset + bvec->bv_len) % PAGE_SIZE;
}

static void __xzram_make_request(struct xzram *xzram, struct bio *bio, int rw)
{
	int i, offset;
	u32 index;
	struct bio_vec *bvec;

	switch (rw) {
	case READ:
		xzram_stat64_inc(xzram, &xzram->stats.num_reads);
		break;
	case WRITE:
		xzram_stat64_inc(xzram, &xzram->stats.num_writes);
		break;
	}

	index = bio->bi_sector >> SECTORS_PER_PAGE_SHIFT;
	offset = (bio->bi_sector & (SECTORS_PER_PAGE - 1)) << SECTOR_SHIFT;

	bio_for_each_segment(bvec, bio, i) {
		int max_transfer_size = PAGE_SIZE - offset;

		if (bvec->bv_len > max_transfer_size) {
			/*
			 * xzram_bvec_rw() can only make operation on a single
			 * xzram page. Split the bio vector.
			 */
			struct bio_vec bv;

			bv.bv_page = bvec->bv_page;
			bv.bv_len = max_transfer_size;
			bv.bv_offset = bvec->bv_offset;

			if (xzram_bvec_rw(xzram, &bv, index, offset, bio, rw) < 0)
				goto out;

			bv.bv_len = bvec->bv_len - max_transfer_size;
			bv.bv_offset += max_transfer_size;
			if (xzram_bvec_rw(xzram, &bv, index+1, 0, bio, rw) < 0)
				goto out;
		} else
			if (xzram_bvec_rw(xzram, bvec, index, offset, bio, rw)
			    < 0)
				goto out;

		update_position(&index, &offset, bvec);
	}

	set_bit(BIO_UPTODATE, &bio->bi_flags);
	bio_endio(bio, 0);
	return;

out:
	bio_io_error(bio);
}

/*
 * Check if request is within bounds and aligned on xzram logical blocks.
 */
static inline int valid_io_request(struct xzram *xzram, struct bio *bio)
{
	if (unlikely(
		(bio->bi_sector >= (xzram->disksize >> SECTOR_SHIFT)) ||
		(bio->bi_sector & (XZRAM_SECTOR_PER_LOGICAL_BLOCK - 1)) ||
		(bio->bi_size & (XZRAM_LOGICAL_BLOCK_SIZE - 1)))) {

		return 0;
	}

	/* I/O request is valid */
	return 1;
}

/*
 * Handler function for all xzram I/O requests.
 */
static int xzram_make_request(struct request_queue *queue, struct bio *bio)
{
#ifdef CONFIG_XZRAM_COMPRESS_PERFORMANCE_STAT
	u64 start = gettime();
#endif

	struct xzram *xzram = queue->queuedata;

	if (!valid_io_request(xzram, bio)) {
		xzram_stat64_inc(xzram, &xzram->stats.invalid_io);
		bio_io_error(bio);
		return 0;
	}

	if (unlikely(!xzram->init_done) && xzram_init_device(xzram)) {
		bio_io_error(bio);
		return 0;
	}

	__xzram_make_request(xzram, bio, bio_data_dir(bio));

#ifdef CONFIG_XZRAM_COMPRESS_PERFORMANCE_STAT
    if (bio_data_dir(bio) == READ) {
       xzram_stat64_add(xzram, &xzram->stats.time_rd, gettime() - start);
    } else {
       xzram_stat64_add(xzram, &xzram->stats.time_wr, gettime() - start);
    }
#endif

	return 0;
}

static void xzram_destroy_pools(struct xzram *xzram)
{
     int i;

     for (i=0;i<MAX_POOL_NUM;i++)
     {
        if(xzram->xv_mem_pool[i] == NULL)
             continue;

         ext_xv_destroy_pool(xzram->xv_mem_pool[i]);
         xzram->xv_mem_pool[i] = NULL;
     }

}

void xzram_reset_device(struct xzram *xzram)
{
	size_t index;
  u32 pool_id;

	mutex_lock(&xzram->init_lock);
	xzram->init_done = 0;

	/* Free various per-device buffers */
	kfree(xzram->compress_workmem);
	free_pages((unsigned long)xzram->compress_buffer, 1);

	xzram->compress_workmem = NULL;
	xzram->compress_buffer = NULL;

	  /* Free all pages that are still in this xzram device */
	  for (index = 0; index < xzram->disksize >> PAGE_SHIFT; index++) {
		struct page *page;
		u16 offset;

		page = xzram->table[index].page;
		offset = xzram->table[index].offset;
                pool_id = xzram->table[index].pool_id;

		if (!page)
			continue;

		if (unlikely(xzram_test_flag(xzram, index, XZRAM_UNCOMPRESSED)))
			__free_page(page);
		else
                {
		        ext_xv_free(xzram->xv_mem_pool[pool_id], page, offset);
                }
	}

	vfree(xzram->table);
	xzram->table = NULL;

        xzram_destroy_pools(xzram);

	/* Reset stats */
	memset(&xzram->stats, 0, sizeof(xzram->stats));

	xzram->disksize = 0;
	mutex_unlock(&xzram->init_lock);
}

static int xzram_create_pools(struct xzram * xzram)
{
        struct ext_xv_pool_attr *attr=NULL;
        unsigned int i;

        xzram->xv_mem_pool = kzalloc(MAX_POOL_NUM*sizeof(struct ext_xv_pool *), GFP_KERNEL);

        attr = kzalloc(sizeof(struct ext_xv_pool_attr), GFP_KERNEL);

        /*Create default internal pool 0 for xzram*/
        xzram->pool_info[0].activate = 1;
        xzram->pool_info[0].pool_id  = 0;
        attr->type = EXT_XVPOOL_INTERNAL_NO_LIMIT;
        attr->ext_pool_start = 0;
        attr->ext_pool_size = 0;
        xzram->xv_mem_pool[0] = ext_xv_create_pool(attr, xzram->pool_info[0].pool_id);

        for(i=1;i<MAX_POOL_NUM;i++)
        {
            if(xzram->pool_info[i].activate == 0)
            {
                xzram->xv_mem_pool[i] = NULL;
                continue;
            }

            attr->ext_pool_size = xzram->pool_info[i].ext_size;

            if(xzram->pool_info[i].type == 0)
            {
                attr->ext_pool_start = xzram->pool_info[i].ext_ba_start;
                attr->type = EXT_XVPOOL_INTERNAL;
            }
            else
            {
                attr->ext_pool_start = xzram->pool_info[i].ext_ba_start;
                attr->type = EXT_XVPOOL_EXTERNAL;
            }

            attr->pool_id = xzram->pool_info[i].pool_id;

            xzram->xv_mem_pool[i] = ext_xv_create_pool(attr, xzram->pool_info[i].pool_id);
        }

        kfree(attr);

        return 0;
}

int xzram_init_device(struct xzram *xzram)
{
	int ret;
	size_t num_pages;

	mutex_lock(&xzram->init_lock);

	if (xzram->init_done) {
		mutex_unlock(&xzram->init_lock);
		return 0;
	}

	xzram_set_disksize(xzram, totalram_pages << PAGE_SHIFT);

	xzram->compress_workmem = kzalloc(ZRAM_CODEC_MEM, GFP_KERNEL);
	if (!xzram->compress_workmem) {
		pr_err("Error allocating compressor working memory!\n");
		ret = -ENOMEM;
		goto fail;
	}

	xzram->compress_buffer = (void *)__get_free_pages(__GFP_ZERO, 1);
	if (!xzram->compress_buffer) {
		pr_err("Error allocating compressor buffer space\n");
		ret = -ENOMEM;
		goto fail;
	}

	num_pages = xzram->disksize >> PAGE_SHIFT;
	xzram->table = vzalloc(num_pages * sizeof(*xzram->table));
	if (!xzram->table) {
		pr_err("Error allocating xzram address table\n");
		/* To prevent accessing table entries during cleanup */
		xzram->disksize = 0;
		ret = -ENOMEM;
		goto fail;
	}

	set_capacity(xzram->disk, xzram->disksize >> SECTOR_SHIFT);

	/* xzram devices sort of resembles non-rotational disks */
	queue_flag_set_unlocked(QUEUE_FLAG_NONROT, xzram->disk->queue);



  if(xzram_create_pools(xzram))
  {
      pr_err("Error create mem pool!\n");
      ret = -ENOMEM;
      goto fail;
  }

  xzram->pool_migration_stat = MIGRATION_STAT_NO_JOB;

	xzram->init_done = 1;

	mutex_unlock(&xzram->init_lock);

	pr_debug("Initialization done!\n");
	return 0;

fail:
	mutex_unlock(&xzram->init_lock);
	xzram_reset_device(xzram);

	pr_err("Initialization failed: err=%d\n", ret);
	return ret;
}



static int xzram_enqueue_task(struct xzram *xzram, unsigned int num)
{
    int *tmp;
    int index, index_next;

    spin_lock(&xzram->notify_lock);

    if( xzram->index_num == -1 )
    {
        xzram->index_num = num;
		    xzram->table[num].notify_id = -1;
		
        spin_unlock(&xzram->notify_lock);
		    return ENQUEUE_OK;
    }

    index = xzram->index_num;

	index_next = xzram->table[index].notify_id;
    
	for(;;)
	{ 
        if(index_next==-1)
        {
            break;
        }
	    index = index_next;
 	    index_next = xzram->table[index_next].notify_id;
	}

	xzram->table[index].notify_id = num;
	xzram->table[num].notify_id = -1;
		
	spin_unlock(&xzram->notify_lock);	
	return ENQUEUE_OK;
	
}

static int xzram_dequeue_task(struct xzram *xzram)
{
    int *tmp;
    int index, index_next;
	unsigned int num=-1;

    spin_lock(&xzram->notify_lock);
	
	if( xzram->index_num == -1)
	{
	    spin_unlock(&xzram->notify_lock);
	    return DEQUEUE_EMPTY;
	}
	
    index = xzram->index_num;
    index_next = xzram->table[index].notify_id;

	if( xzram->index_num && (index_next == -1))
	{
	    num = xzram->index_num;
		xzram->index_num = -1;
		spin_unlock(&xzram->notify_lock);
        return num;      
	}

	num = xzram->index_num;
    xzram->table[num].notify_id = -1;
    xzram->index_num = index_next;
	
	spin_unlock(&xzram->notify_lock);
	return num;
}

void wrap_xzram_notify(struct work_struct *taskp) 
{         
    struct notify_handler *notify = container_of(taskp, struct notify_handler, wq_work); 
	struct xzram *xzram = notify->xzram; 
	u32 index = notify->index;      
    int num=0;

    num = xzram_dequeue_task(xzram);

    while( num != DEQUEUE_EMPTY){

	    down_write(&xzram->lock);
		if(xzram->table[num].notify == true)
		{
		    xzram_free_page(xzram, num);	
		    xzram_stat64_inc(xzram, &xzram->stats.notify_free);  
		}

	    xzram->table[num].notify = false;
		up_write(&xzram->lock);
		
        num = xzram_dequeue_task(xzram);
	};

	

} 


void xzram_slot_free_notify(struct block_device *bdev, unsigned long index)
{
	struct xzram *xzram;

	xzram = bdev->bd_disk->private_data;
    xzram->free_handler->xzram = xzram;
    xzram->free_handler->index = index;
	
    xzram->table[index].notify = true;

	xzram_enqueue_task(xzram, index);
	queue_work(xzram->free_handler->wq, &xzram->free_handler->wq_work);
}

static const struct block_device_operations xzram_devops = {
	.swap_slot_free_notify = xzram_slot_free_notify,
	.owner = THIS_MODULE
};

static int create_device(struct xzram *xzram, int device_id)
{
	int ret = 0;

	init_rwsem(&xzram->lock);
	spin_lock_init(&xzram->notify_lock);
	mutex_init(&xzram->init_lock);
	spin_lock_init(&xzram->stat64_lock);
    xzram->index_num = -1;
 
	xzram->queue = blk_alloc_queue(GFP_KERNEL);
	if (!xzram->queue) {
		pr_err("Error allocating disk queue for device %d\n",
			device_id);
		ret = -ENOMEM;
		goto out;
	}

	blk_queue_make_request(xzram->queue, xzram_make_request);
	xzram->queue->queuedata = xzram;

	 /* gendisk structure */
	xzram->disk = alloc_disk(1);
	if (!xzram->disk) {
		blk_cleanup_queue(xzram->queue);
		pr_warning("Error allocating disk structure for device %d\n",
			device_id);
		ret = -ENOMEM;
		goto out;
	}

	xzram->disk->major = xzram_major;
	xzram->disk->first_minor = device_id;
	xzram->disk->fops = &xzram_devops;
	xzram->disk->queue = xzram->queue;
	xzram->disk->private_data = xzram;

#ifdef CONFIG_XZRAM_DEBUG
    xzram->stats.compress_ratio_cnt = 0;
#endif
    xzram->pool_info   = kzalloc(MAX_POOL_NUM*sizeof(struct ext_pool_info), GFP_KERNEL);

	xzram->free_handler = kzalloc(sizeof(struct notify_handler), GFP_KERNEL);		 
	xzram->free_handler->wq = create_workqueue("free_notify"); 	   
	if(!xzram->free_handler->wq) 		   
		printk("Create work queue fail!!!\n");		 
	  
	INIT_WORK(&xzram->free_handler->wq_work, wrap_xzram_notify);


	snprintf(xzram->disk->disk_name, 16, "xzram%d", device_id);

	/* Actual capacity set using syfs (/sys/block/zram<id>/disksize */
	set_capacity(xzram->disk, 0);

	/*
	 * To ensure that we always get PAGE_SIZE aligned
	 * and n*PAGE_SIZED sized I/O requests.
	 */
	blk_queue_physical_block_size(xzram->disk->queue, PAGE_SIZE);
	blk_queue_logical_block_size(xzram->disk->queue,
					XZRAM_LOGICAL_BLOCK_SIZE);
	blk_queue_io_min(xzram->disk->queue, PAGE_SIZE);
	blk_queue_io_opt(xzram->disk->queue, PAGE_SIZE);

	add_disk(xzram->disk);

	ret = sysfs_create_group(&disk_to_dev(xzram->disk)->kobj,
				&xzram_disk_attr_group);
	if (ret < 0) {
		pr_warning("Error creating sysfs group");
		goto out;
	}

	xzram->init_done = 0;

out:
	return ret;
}

static void destroy_device(struct xzram *xzram)
{
	sysfs_remove_group(&disk_to_dev(xzram->disk)->kobj,
			&xzram_disk_attr_group);

	if (xzram->disk) {
		del_gendisk(xzram->disk);
		put_disk(xzram->disk);
	}

	if (xzram->queue)
		blk_cleanup_queue(xzram->queue);

        kfree(xzram->pool_info);
        kfree(xzram->xv_mem_pool);
}

static int __init xzram_init(void)
{
	int ret, dev_id;

#ifdef XZRAM_COMPRESS_LZ4
		if (MEMORY_USAGE == 12)
			printk("xzram: %s, %s, %d: using lz4 with 4k\n", __FILE__, __FUNCTION__, __LINE__);
		else
			printk("xzram: %s, %s, %d: using lz4 with 16k\n", __FILE__, __FUNCTION__, __LINE__);
#else
		printk("xzram: %s, %s, %d: using lzo\n", __FILE__, __FUNCTION__, __LINE__);
#endif

	if (xzram_num_devices > max_num_devices) {
		pr_warning("Invalid value for xzram_num_devices: %u\n",
				xzram_num_devices);
		ret = -EINVAL;
		goto out;
	}

	xzram_major = register_blkdev(0, "xzram");

	if (xzram_major <= 0) {
		pr_warning("Unable to get major number\n");
		ret = -EBUSY;
		goto out;
	}

	if (!xzram_num_devices) {
		pr_info("xzram_num_devices not specified. Using default: 4\n");
		xzram_num_devices = 4;
	}

	/* Allocate the device array and initialize each one */
	pr_info("Creating %u devices ... xzram_major = %d\n", xzram_num_devices, xzram_major);
	xzram_devices = kzalloc(xzram_num_devices * sizeof(struct xzram), GFP_KERNEL);
	if (!xzram_devices) {
		ret = -ENOMEM;
		goto unregister;
	}

	for (dev_id = 0; dev_id < xzram_num_devices; dev_id++) {
		ret = create_device(&xzram_devices[dev_id], dev_id);
		if (ret)
			goto free_devices;
	}

	return 0;

free_devices:
	while (dev_id)
		destroy_device(&xzram_devices[--dev_id]);
	kfree(xzram_devices);
unregister:
	unregister_blkdev(xzram_major, "xzram");
out:
	return ret;
}

static void __exit xzram_exit(void)
{
	int i;
	struct xzram *xzram;

	for (i = 0; i < xzram_num_devices; i++) {
		xzram = &xzram_devices[i];

		destroy_device(xzram);
		if (xzram->init_done)
			xzram_reset_device(xzram);
	}

	unregister_blkdev(xzram_major, "xzram");

	kfree(xzram_devices);
	pr_debug("Cleanup done!\n");
}

module_param(xzram_num_devices, uint, 0);
MODULE_PARM_DESC(num_devices, "Number of xzram devices");

module_init(xzram_init);
module_exit(xzram_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Nitin Gupta <ngupta@vflare.org>");
MODULE_DESCRIPTION("Compressed RAM Block Device");
