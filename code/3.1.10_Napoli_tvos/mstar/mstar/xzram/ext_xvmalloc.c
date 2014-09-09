/*
 * xvmalloc memory allocator
 *
 * Copyright (C) 2008, 2009, 2010  Nitin Gupta
 *
 * This code is released using a dual license strategy: BSD/GPL
 * You can choose the licence that better fits your requirements.
 *
 * Released under the terms of 3-clause BSD License
 * Released under the terms of GNU General Public License Version 2.0
 */

#ifdef CONFIG_XZRAM_DEBUG
#define DEBUG
#endif
  
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/highmem.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/io.h>
 
#include "ext_xvmalloc.h"
#include "ext_xvmalloc_int.h"

#define EXT_XV_FAIL             -1
#define PageIndexToOffset(x) ((u32)x-1)
#define OffsetToPageIndex(x) ((u32)x+1)

static void stat_inc(u64 *value)
{
    *value = *value + 1;
}

static void stat_dec(u64 *value)
{
    *value = *value - 1;
}

static int test_flag(struct block_header *block, enum blockflags flag)
{
    return block->prev & BIT(flag);
}

static void set_flag(struct block_header *block, enum blockflags flag)
{
    block->prev |= BIT(flag);
}

static void clear_flag(struct block_header *block, enum blockflags flag)
{
    block->prev &= ~BIT(flag);
}

/*
 * Given <page, offset> pair, provide a dereferencable pointer.
 * This is called from xv_malloc/xv_free path, so it
 * needs to be fast.
 */
static void __iomem *get_ptr_atomic(void __iomem *page, u16 offset, enum km_type type, struct ext_xv_pool *pool)
{
    void __iomem *base, *k_page;

    if(pool->type == EXT_XVPOOL_INTERNAL_NO_LIMIT)
        base = kmap_atomic(page, type);
        
    else if(pool->type == EXT_XVPOOL_INTERNAL) 
    {
        k_page = pool->ext_pool_attr->ext_xv_pool_array[PageIndexToOffset(page)];
        base = kmap_atomic(k_page, type);
    }
    else
        base = pool->ext_pool_attr->ext_xv_pool_array[PageIndexToOffset(page)];
        
    return base + offset;
}

static void put_ptr_atomic(void *ptr, enum km_type type, struct ext_xv_pool *pool)
{
    if(pool->type == EXT_XVPOOL_INTERNAL_NO_LIMIT || pool->type == EXT_XVPOOL_INTERNAL)
        kunmap_atomic(ptr, type);
}

void __iomem *ext_xv_get_ptr_atomic(void __iomem *page, u16 offset, u32 type, struct ext_xv_pool *pool)
{
    void __iomem* va_page;
   
    spin_lock(&pool->lock);
    va_page = get_ptr_atomic(page, offset, (enum km_type)type, pool);
    spin_unlock(&pool->lock);

    return va_page;
}
EXPORT_SYMBOL_GPL(ext_xv_get_ptr_atomic);

void ext_xv_put_ptr_atomic(void *ptr, u32 type, struct ext_xv_pool *pool)
{   
    spin_lock(&pool->lock);
    put_ptr_atomic(ptr, (enum km_type)type, pool);
    spin_unlock(&pool->lock);
}
EXPORT_SYMBOL_GPL(ext_xv_put_ptr_atomic);

static u32 get_blockprev(struct block_header *block)
{
    return block->prev & PREV_MASK;
}

static void set_blockprev(struct block_header *block, u16 new_offset)
{
    block->prev = new_offset | (block->prev & FLAGS_MASK);
}

static struct block_header *BLOCK_NEXT(struct block_header *block)
{
    return (struct block_header *)
        ((char *)block + block->size + XV_ALIGN);
}

/*
 * Get index of free list containing blocks of maximum size
 * which is less than or equal to given size.
 */
static u32 get_index_for_insert(u32 size)
{
    if (unlikely(size > XV_MAX_ALLOC_SIZE))
        size = XV_MAX_ALLOC_SIZE;

    size &= ~FL_DELTA_MASK;

    return (size - XV_MIN_ALLOC_SIZE) >> FL_DELTA_SHIFT;
}

/*
 * Get index of free list having blocks of size greater than
 * or equal to requested size.
 */
static u32 get_index(u32 size)
{
    if (unlikely(size < XV_MIN_ALLOC_SIZE))
        size = XV_MIN_ALLOC_SIZE;
    
    size = ALIGN(size, FL_DELTA);
    
    return (size - XV_MIN_ALLOC_SIZE) >> FL_DELTA_SHIFT;
}

/**
 * find_block - find block of at least given size
 * @pool: memory pool to search from
 * @size: size of block required
 * @page: page containing required block
 * @offset: offset within the page where block is located.
 *
 * Searches two level bitmap to locate block of at least
 * the given size. If such a block is found, it provides
 * <page, offset> to identify this block and returns index
 * in freelist where we found this block.
 * Otherwise, returns 0 and <page, offset> params are not touched.
 */
static u32 find_block(struct ext_xv_pool *pool, u32 size,
            struct page **page, u32 *offset)
{
    ulong flbitmap, slbitmap;
    u32 flindex, slindex, slbitstart;

    /* There are no free blocks in this pool */
    if (!pool->flbitmap)
        return 0;

    /* Get freelist index correspoding to this size */
    slindex = get_index(size);
    slbitmap = pool->slbitmap[slindex / BITS_PER_LONG];
    slbitstart = slindex % BITS_PER_LONG;

    /*
     * If freelist is not empty at this index, we found the
     * block - head of this list. This is approximate best-fit match.
     */
    if (test_bit(slbitstart, &slbitmap)) {
        *page = pool->freelist[slindex].page;
        *offset = pool->freelist[slindex].offset;
        return slindex;
    }

    /*
     * No best-fit found. Search a bit further in bitmap for a free block.
     * Second level bitmap consists of series of 32-bit chunks. Search
     * further in the chunk where we expected a best-fit, starting from
     * index location found above.
     */
    slbitstart++;
    slbitmap >>= slbitstart;

    /* Skip this search if we were already at end of this bitmap chunk */
    if ((slbitstart != BITS_PER_LONG) && slbitmap) {
        slindex += __ffs(slbitmap) + 1;
        *page = pool->freelist[slindex].page;
        *offset = pool->freelist[slindex].offset;
        return slindex;
    }

    /* Now do a full two-level bitmap search to find next nearest fit */
    flindex = slindex / BITS_PER_LONG;

    flbitmap = (pool->flbitmap) >> (flindex + 1);
    if (!flbitmap)
        return 0;

    flindex += __ffs(flbitmap) + 1;
    slbitmap = pool->slbitmap[flindex];
    slindex = (flindex * BITS_PER_LONG) + __ffs(slbitmap);
    *page = pool->freelist[slindex].page;
    *offset = pool->freelist[slindex].offset;

    return slindex;
}

/*
 * Insert block at <page, offset> in freelist of given pool.
 * freelist used depends on block size.
 */
static void insert_block(struct ext_xv_pool *pool, struct page *page, u32 offset,
            struct block_header *block)
{
    u32 flindex, slindex;
    struct block_header *nextblock;

    slindex = get_index_for_insert(block->size);
    flindex = slindex / BITS_PER_LONG;

    block->link.prev_page = NULL;
    block->link.prev_offset = 0;
    block->link.next_page = pool->freelist[slindex].page;
    block->link.next_offset = pool->freelist[slindex].offset;
    pool->freelist[slindex].page = page;
    pool->freelist[slindex].offset = offset;
    pool->freelist[slindex].num++;

    if (block->link.next_page) {
        nextblock = get_ptr_atomic(block->link.next_page,
                    block->link.next_offset, KM_USER1, pool);
        nextblock->link.prev_page = page;
        nextblock->link.prev_offset = offset;
        put_ptr_atomic(nextblock, KM_USER1, pool);
        /* If there was a next page then the free bits are set. */
        return;
    }

    __set_bit(slindex % BITS_PER_LONG, &pool->slbitmap[flindex]);
    __set_bit(flindex, &pool->flbitmap);
}

/*
 * Remove block from freelist. Index 'slindex' identifies the freelist.
 */
static void remove_block(struct ext_xv_pool *pool, struct page *page, u32 offset,
            struct block_header *block, u32 slindex)
{
    u32 flindex = slindex / BITS_PER_LONG;
    struct block_header *tmpblock;

    if (block->link.prev_page) {

        tmpblock = get_ptr_atomic(block->link.prev_page,
                block->link.prev_offset, KM_USER1, pool);
        tmpblock->link.next_page = block->link.next_page;
        tmpblock->link.next_offset = block->link.next_offset;
        put_ptr_atomic(tmpblock, KM_USER1, pool);
    }

    if (block->link.next_page) {
        tmpblock = get_ptr_atomic(block->link.next_page,
                block->link.next_offset, KM_USER1, pool);
        tmpblock->link.prev_page = block->link.prev_page;
        tmpblock->link.prev_offset = block->link.prev_offset;
        put_ptr_atomic(tmpblock, KM_USER1, pool);
    }

    /* Is this block is at the head of the freelist? */
    if (pool->freelist[slindex].page == page
       && pool->freelist[slindex].offset == offset) {

        pool->freelist[slindex].page = block->link.next_page;
        pool->freelist[slindex].offset = block->link.next_offset;

        if (pool->freelist[slindex].page) {
            struct block_header *tmpblock;
            tmpblock = get_ptr_atomic(pool->freelist[slindex].page,
                    pool->freelist[slindex].offset,
                    KM_USER1, pool);
            tmpblock->link.prev_page = NULL;
            tmpblock->link.prev_offset = 0;
            put_ptr_atomic(tmpblock, KM_USER1, pool);
        } else {
            /* This freelist bucket is empty */
            __clear_bit(slindex % BITS_PER_LONG,
                    &pool->slbitmap[flindex]);
            if (!pool->slbitmap[flindex])
                __clear_bit(flindex, &pool->flbitmap);
        }
    }

    pool->freelist[slindex].num--;
    block->link.prev_page = NULL;
    block->link.prev_offset = 0;
    block->link.next_page = NULL;
    block->link.next_offset = 0;
}

static unsigned int GetRightMostZero(unsigned int u32Flags)
{
    unsigned int i;

    for(i=0;i<BITS_PER_LONG;i++)
    {
        if((u32Flags & (1<<i))==0)
        return i;
    }

    return EXT_XV_FAIL;
}

static unsigned int GetRightMostOne(unsigned int u32Flags)
{
    unsigned int i;

    for(i=0;i<BITS_PER_LONG;i++)
    {
        if((u32Flags & (1<<i))>0)
        return i;
    }

    return EXT_XV_FAIL;
}


static void __iomem * alloc_xpage(struct ext_xv_pool *pool, gfp_t flag)
{
    int i;
    unsigned int  array_offset;    
    struct page *page;
    unsigned int bmap_entry_num, mod;
    signed int u32bit;

    bmap_entry_num = (pool->ext_pool_attr->size>>PAGE_SHIFT)/BITS_PER_LONG;
    mod = (pool->ext_pool_attr->size>>PAGE_SHIFT)%BITS_PER_LONG;
 

    if(mod != 0)
        bmap_entry_num ++;

    for(i=0;i<bmap_entry_num;i++)
    {
        if(*(pool->ext_pool_attr->xbitmap+i) ==0xffffffff)
           continue;

        u32bit = GetRightMostZero(*(pool->ext_pool_attr->xbitmap+i));

        if((i==bmap_entry_num-1) && (u32bit>=mod) && (mod !=0))
            continue;

        if(u32bit >= 0)
        {
            array_offset = i*BITS_PER_LONG+u32bit;
            *(pool->ext_pool_attr->xbitmap+i) |= (1 <<u32bit);

            if(pool->type == EXT_XVPOOL_INTERNAL)
            {
                page = alloc_page(flag);
                pool->ext_pool_attr->ext_xv_pool_array[array_offset] = (void __iomem *)page;
            }
            else if(pool->type == EXT_XVPOOL_EXTERNAL)
            {
                pool->ext_pool_attr->ext_xv_pool_array[array_offset] = 
                                     (void __iomem *) (pool->ext_pool_attr->ba_start+
                                      (i*BITS_PER_LONG*PAGE_SIZE)+(PAGE_SIZE*u32bit));
            }
            return (void __iomem *) OffsetToPageIndex(array_offset);
        }
    }    
 
    return NULL;
}


/*
 * Allocate a page and add it to freelist of given pool.
 */
static int grow_pool(struct ext_xv_pool *pool, gfp_t flags)
{
    struct page *page;
    struct block_header *block;

    if(pool->type == EXT_XVPOOL_INTERNAL_NO_LIMIT)
    {
         page = alloc_page(flags);
    }
    else
    {
         page = alloc_xpage(pool, flags);
    }
    if (!page)
    { 
         return -ENOMEM;
    }

    stat_inc(&pool->total_pages);

    spin_lock(&pool->lock);

    if(pool->type != EXT_XVPOOL_INTERNAL_NO_LIMIT)
    {
        if(pool->xv_migration.on_migration)
        {
            pool->xv_migration.write_cnt++;
        }
    }

    block = get_ptr_atomic(page, 0, KM_USER0, pool);

    block->size = PAGE_SIZE - XV_ALIGN;
    set_flag(block, BLOCK_FREE);
    clear_flag(block, PREV_FREE);
    set_blockprev(block, 0);

    insert_block(pool, page, 0, block);
    put_ptr_atomic(block, KM_USER0, pool);
    spin_unlock(&pool->lock);

    return 0;
}


/*
 * Create a memory pool from interal/external memory. Allocates freelist, bitmaps and other
 * per-pool metadata.
 */
struct ext_xv_pool *ext_xv_create_pool(struct ext_xv_pool_attr *attr, u8 pool_id)
{
    u32 ovhd_size;
    struct ext_xv_pool *pool;
        u32 bmap_entry_num;

    ovhd_size = roundup(sizeof(*pool), PAGE_SIZE);
    pool = kzalloc(ovhd_size, GFP_KERNEL|__GFP_REPEAT);

    if (!pool)
        return NULL;

    spin_lock_init(&pool->lock);

        if(attr == NULL)
                return NULL;
        else
        {
            pool->pool_id = pool_id;    

            pool->ext_pool_attr = kzalloc(sizeof(struct xv_ext_memory_attr), GFP_KERNEL|__GFP_REPEAT);
            pool->ext_pool_attr->size = ALIGN(attr->ext_pool_size, PAGE_SIZE);
   
            bmap_entry_num = (pool->ext_pool_attr->size>>PAGE_SHIFT)/BITS_PER_LONG;
            if(((pool->ext_pool_attr->size>>PAGE_SHIFT)%BITS_PER_LONG)!=0)
                bmap_entry_num++;
          
            if(attr->type == EXT_XVPOOL_EXTERNAL)
            {
                printk("Create external memory pool for xzram pool_id = %d!!\n", pool->pool_id);
                pool->type = EXT_XVPOOL_EXTERNAL;        
                pool->ext_pool_attr->ba_start = (unsigned long *)attr->ext_pool_start;
                pool->ext_pool_attr->ba_start = ioremap_nocache((unsigned long)pool->ext_pool_attr->ba_start, 
                                                                                attr->ext_pool_size);
                pool->ext_pool_attr->xbitmap = kzalloc(sizeof(u32)*(bmap_entry_num), GFP_KERNEL|__GFP_REPEAT);
                pool->ext_pool_attr->ext_xv_pool_array = kzalloc( (pool->ext_pool_attr->size>>PAGE_SHIFT)*
                                                                      sizeof(void __iomem *), GFP_KERNEL|__GFP_REPEAT);
            }
            else if (attr->type == EXT_XVPOOL_INTERNAL)
            {
                printk("Create internal memory pool for xzram pool_id = %d!!\n", pool->pool_id);
                pool->type = EXT_XVPOOL_INTERNAL; 
                pool->ext_pool_attr->ba_start = (unsigned long *)attr->ext_pool_start;
                pool->ext_pool_attr->ba_start = ioremap_nocache((unsigned long)pool->ext_pool_attr->ba_start, 
                                                                                attr->ext_pool_size);
                pool->ext_pool_attr->xbitmap = kzalloc(sizeof(u32)*bmap_entry_num, GFP_KERNEL);
                pool->ext_pool_attr->ext_xv_pool_array = kzalloc((pool->ext_pool_attr->size>>PAGE_SHIFT)*
                                                                     sizeof(void __iomem *), GFP_KERNEL|__GFP_REPEAT);
            }
            else if (attr->type == EXT_XVPOOL_INTERNAL_NO_LIMIT)
            {
                printk("Create internal memory pool (default pool) pool_id = %d!!\n", pool->pool_id);
                pool->type = EXT_XVPOOL_INTERNAL_NO_LIMIT;
            }
            else
            {
                printk("Can not recognize the pool type!! ID = %d\n", pool_id);
            }
        }
    
        pool->xv_migration.on_migration = 0;
        pool->xv_migration.read_cnt = 0;
        pool->xv_migration.write_cnt = 0;
        
    return pool;
}
EXPORT_SYMBOL_GPL(ext_xv_create_pool);

void ext_xv_destroy_pool(struct ext_xv_pool *pool)
{
    spin_lock(&pool->lock);
 
    if((pool->ext_pool_attr != NULL))
    {
        if(pool->type == EXT_XVPOOL_EXTERNAL
            || pool->type == EXT_XVPOOL_INTERNAL)
            iounmap(pool->ext_pool_attr->ba_start);
    
            kfree(pool->ext_pool_attr->xbitmap);
            kfree(pool->ext_pool_attr);
            kfree(pool->ext_pool_attr->ext_xv_pool_array);
    }

    kfree(pool);
    spin_unlock(&pool->lock);
}
EXPORT_SYMBOL_GPL(ext_xv_destroy_pool);

/**
 * xv_malloc - Allocate block of given size from pool.
 * @pool: pool to allocate from
 * @size: size of block to allocate
 * @page: page no. that holds the object
 * @offset: location of object within page
 *
 * On success, <page, offset> identifies block allocated
 * and 0 is returned. On failure, <page, offset> is set to
 * 0 and -ENOMEM is returned.
 *
 * Allocation requests with size > XV_MAX_ALLOC_SIZE will fail.
 */
int ext_xv_malloc(struct ext_xv_pool *pool, u32 size, struct page **page,
        u32 *offset, gfp_t flags)
{
    int error;
    u32 index, tmpsize, origsize, tmpoffset;
    struct block_header *block, *tmpblock;

    if(pool == NULL)
    {
        return ENOSPC;
    }

    *page = NULL;
    *offset = 0;
    origsize = size;

    if (unlikely(!size || size > XV_MAX_ALLOC_SIZE))
    {
        return -ENOMEM;
    }

    size = ALIGN(size, XV_ALIGN);

    spin_lock(&pool->lock);

    index = find_block(pool, size, page, offset);

    if (!*page) {
        spin_unlock(&pool->lock);

        if (flags & GFP_NOWAIT)
            return -ENOMEM;

        error = grow_pool(pool, flags);

        if (error)
            return error;

        spin_lock(&pool->lock);
        index = find_block(pool, size, page, offset);
    }

    if (!*page) {
        spin_unlock(&pool->lock);
        return -ENOMEM;
    }

    block = get_ptr_atomic(*page, *offset, KM_USER0, pool);
    remove_block(pool, *page, *offset, block, index);

    /* Split the block if required */
    tmpoffset = *offset + size + XV_ALIGN;
    tmpsize = block->size - size;
    tmpblock = (struct block_header *)((char *)block + size + XV_ALIGN);
    if (tmpsize) {
        tmpblock->size = tmpsize - XV_ALIGN;
        set_flag(tmpblock, BLOCK_FREE);
        clear_flag(tmpblock, PREV_FREE);

        set_blockprev(tmpblock, *offset);
        if (tmpblock->size >= XV_MIN_ALLOC_SIZE)
            insert_block(pool, *page, tmpoffset, tmpblock);

        if (tmpoffset + XV_ALIGN + tmpblock->size != PAGE_SIZE) {
            tmpblock = BLOCK_NEXT(tmpblock);
            set_blockprev(tmpblock, tmpoffset);
        }
    } else {
        /* This block is exact fit */
        if (tmpoffset != PAGE_SIZE)
            clear_flag(tmpblock, PREV_FREE);
    }

    block->size = origsize;
    clear_flag(block, BLOCK_FREE);

    put_ptr_atomic(block, KM_USER0, pool);
    spin_unlock(&pool->lock);

    *offset += XV_ALIGN;

    return 0;
}
EXPORT_SYMBOL_GPL(ext_xv_malloc);

/*
 * Free block identified with <page, offset>
 */
void ext_xv_free(struct ext_xv_pool *pool, struct page *page, u32 offset)
{
    void *page_start;
    struct block_header *block, *tmpblock;
    u32 *xbmap;
    u32 boffset,value;
    struct page *int_page;               
    offset -= XV_ALIGN;

    spin_lock(&pool->lock);
    page_start = get_ptr_atomic(page, 0, KM_USER0, pool);
    block = (struct block_header *)((char *)page_start + offset);

    /* Catch double free bugs */
    BUG_ON(test_flag(block, BLOCK_FREE));

    block->size = ALIGN(block->size, XV_ALIGN);

    tmpblock = BLOCK_NEXT(block);
    if (offset + block->size + XV_ALIGN == PAGE_SIZE)
        tmpblock = NULL;

    /* Merge next block if its free */
    if (tmpblock && test_flag(tmpblock, BLOCK_FREE)) {
        /*
         * Blocks smaller than XV_MIN_ALLOC_SIZE
         * are not inserted in any free list.
         */
        if (tmpblock->size >= XV_MIN_ALLOC_SIZE) {
            remove_block(pool, page,
                    offset + block->size + XV_ALIGN, tmpblock,
                    get_index_for_insert(tmpblock->size));
        }
        block->size += tmpblock->size + XV_ALIGN;
    }

    /* Merge previous block if its free */
    if (test_flag(block, PREV_FREE)) {
        tmpblock = (struct block_header *)((char *)(page_start) +
                        get_blockprev(block));
        offset = offset - tmpblock->size - XV_ALIGN;

        if (tmpblock->size >= XV_MIN_ALLOC_SIZE)
            remove_block(pool, page, offset, tmpblock,
                    get_index_for_insert(tmpblock->size));

        tmpblock->size += block->size + XV_ALIGN;
        block = tmpblock;
    }

    /* No used objects in this page. Free it. */
    if (block->size == PAGE_SIZE - XV_ALIGN) {
                
        put_ptr_atomic(page_start, KM_USER0, pool);

                if(pool->type == EXT_XVPOOL_INTERNAL_NO_LIMIT)
                {
                    spin_unlock(&pool->lock);
                    __free_page(page);

                }
                else if(pool->type == EXT_XVPOOL_INTERNAL)
                {
                   int_page = (struct page*)pool->ext_pool_attr->ext_xv_pool_array[PageIndexToOffset(page)];
                   boffset = (PageIndexToOffset(page))/BITS_PER_LONG;
                   xbmap = (pool->ext_pool_attr->xbitmap + boffset);
                   value = ((PageIndexToOffset(page))%BITS_PER_LONG);
                   *xbmap &= ~(1<<value);
                   pool->ext_pool_attr->ext_xv_pool_array[PageIndexToOffset(page)] = 0;
                   if(pool->xv_migration.on_migration)
                       pool->xv_migration.read_cnt++;
                   spin_unlock(&pool->lock);
                   __free_page(int_page);
                }
                else if(pool->type == EXT_XVPOOL_EXTERNAL)
                {
                   boffset = (PageIndexToOffset(page))/BITS_PER_LONG;
                   xbmap = (pool->ext_pool_attr->xbitmap + boffset);
                   value = (PageIndexToOffset(page)%BITS_PER_LONG);
                   *xbmap &= ~(1<<value);
                   pool->ext_pool_attr->ext_xv_pool_array[PageIndexToOffset(page)] = 0;
                   if(pool->xv_migration.on_migration)
                       pool->xv_migration.read_cnt++;
                   spin_unlock(&pool->lock);
                }

        stat_dec(&pool->total_pages);
        return;
    }

    set_flag(block, BLOCK_FREE);
    if (block->size >= XV_MIN_ALLOC_SIZE)
        insert_block(pool, page, offset, block);

    if (offset + block->size + XV_ALIGN != PAGE_SIZE) {
        tmpblock = BLOCK_NEXT(block);
        set_flag(tmpblock, PREV_FREE);
        set_blockprev(tmpblock, offset);
    }

    put_ptr_atomic(page_start, KM_USER0, pool);
    spin_unlock(&pool->lock);
}
EXPORT_SYMBOL_GPL(ext_xv_free);

u32 ext_xv_get_object_size(void *obj)
{
    struct block_header *blk;

    blk = (struct block_header *)((char *)(obj) - XV_ALIGN);
    return blk->size;
}

/*
 * Returns total memory used by allocator (userdata + metadata)
 */
u64 ext_xv_get_total_size_bytes(struct ext_xv_pool *pool)
{
    return pool->total_pages << PAGE_SHIFT;
}
EXPORT_SYMBOL_GPL(ext_xv_get_total_size_bytes);

bool ext_xv_is_ext_pool(struct ext_xv_pool *pool)
{
    if(pool->type == EXT_XVPOOL_EXTERNAL)
        return true;
    else 
        return false;
}
EXPORT_SYMBOL_GPL(ext_xv_is_ext_pool);

bool ext_xv_is_dyn_int_pool(struct ext_xv_pool *pool)
{
    if(pool->type == EXT_XVPOOL_INTERNAL)
        return true;
    else
        return false;
}

EXPORT_SYMBOL_GPL(ext_xv_is_dyn_int_pool);

u64 ext_xv_show_total_pages(struct ext_xv_pool *pool)
{
    if(pool == NULL)
        return EXT_XV_FAIL;

    return pool->total_pages;
}
EXPORT_SYMBOL_GPL(ext_xv_show_total_pages);

static u32 array_check_sum_int( void __iomem ** array, u32 len)
{
    u32 i, j, sum=0;
    u32 *page;

    /* 
       Check sum:
       sum of every index number + sum of vulue from each pages
     */
    
    for(i=0;i<len;i++)
    {
        if(array[i] != 0)
        {
            
            page = kmap_atomic(array[i], KM_USER0);
            
            for(j=0;j<PAGE_SIZE/4;j++)
            {
                sum += *(page+j);    
            }
            
            sum = sum*(i+1);            

            kunmap_atomic(page, KM_USER0);
        }
    }
    
    return sum;
}

static u32 array_check_sum_ext( void __iomem ** array, u32 len)
{
    u32 i, j, sum=0;
    u32 *page;

    /* 
       Check sum:
       sum of every index number + sum of vulue from each pages
     */
        
    for(i=0;i<len;i++)
    {
        if(array[i] != 0)
        {
            page = array[i];
            
            for(j=0;j<PAGE_SIZE/4;j++)
            {
                sum += *(page+j);    
            }

            sum = sum*(i+1);
        }
    }
    
    return sum;
}

 void __iomem *get_page_from_backup_list(struct ext_xv_backuplist *backuplist)
{
    void __iomem * page=NULL;
    struct ext_xv_backup_page *tmp_backup_page;

    tmp_backup_page = backuplist->backup_page;

    while(backuplist->cnt > 0)
    {
        page = tmp_backup_page->page;
        if(!page)
        {   
            tmp_backup_page = tmp_backup_page->next_backup_page;
            continue;
        }
        else
        {
            tmp_backup_page->page = NULL;
            tmp_backup_page = tmp_backup_page->next_backup_page;
            backuplist->cnt--;
	    break;
        }
    }

    return page;
}

static void __iomem ** pool_migration(struct ext_xv_pool *pool, u32 type, void __iomem **tmp_array, 
                                                 unsigned int *tmp_xbitmap, struct ext_xv_backuplist *backuplist)
{
    int i, j;
    unsigned int array_offset;    
    unsigned int bmap_entry_num, mod;
    void __iomem *va_src, *va_dest;
    unsigned int u32bit, tmp_bitmap;
    unsigned int notify_free_num=0;
    void __iomem **orig_array;
    unsigned int m_wcnt=0, orig_backuplist_num=0, m_rcnt=0;

    //u32 check_sum1, check_sum2;
    
    if( (pool->type == EXT_XVPOOL_EXTERNAL) && (type == EXT_XVPOOL_INTERNAL))
    {
        //printk("Original pool type is external switch to internal !!!\n");

    }else if(pool->type == EXT_XVPOOL_INTERNAL && (type == EXT_XVPOOL_EXTERNAL))
    {
        //printk("Original pool type is internal switch to external !!!\n");

    }else
    {
        printk("unknown command!! pool type = %d, type assigned %d\n", pool->type, type);
        return 0;
    }

    if(tmp_array == NULL)
    {
        printk("**tmp_array is NULL!!\n");
        return 0;
    }

    /* Update pool ext_xv_pool_array*/
    bmap_entry_num = (pool->ext_pool_attr->size>>PAGE_SHIFT)/BITS_PER_LONG;
    mod = (pool->ext_pool_attr->size>>PAGE_SHIFT)%BITS_PER_LONG;
  
    if(mod != 0)
        bmap_entry_num ++;
    
    spin_lock(&pool->lock); 
    memcpy(tmp_xbitmap, pool->ext_pool_attr->xbitmap, sizeof(u32)*bmap_entry_num);
    spin_unlock(&pool->lock);

    orig_backuplist_num = backuplist->cnt;
    
    for(i=0;i<bmap_entry_num;i++)
    {
        for(j=0;j<BITS_PER_LONG;j++)
        {
            if((i==bmap_entry_num-1) && (j>mod) && (mod !=0))
                break;
			
            array_offset = i*BITS_PER_LONG+j;
		    if((pool->ext_pool_attr->ext_xv_pool_array[array_offset]!=0) && (tmp_array[array_offset]==0))  // xzram write request while migration
            {
                m_wcnt ++;
				
			    if( (pool->type == EXT_XVPOOL_EXTERNAL) && (type == EXT_XVPOOL_INTERNAL))
			    {
                    tmp_array[array_offset] = get_page_from_backup_list(backuplist);
                }
                else if(pool->type == EXT_XVPOOL_INTERNAL && (type == EXT_XVPOOL_EXTERNAL))
    {
                    tmp_array[array_offset] = (void __iomem *)(pool->ext_pool_attr->ba_start+
                               (i*BITS_PER_LONG*PAGE_SIZE)+(PAGE_SIZE*j));
			    }
		    }

            if((pool->ext_pool_attr->ext_xv_pool_array[array_offset]==0) && (tmp_array[array_offset]!=0))  // xzram write request while migration
            {
                m_rcnt++;

                if( (pool->type == EXT_XVPOOL_EXTERNAL) && (type == EXT_XVPOOL_INTERNAL))
			    {
                    __free_page(tmp_array[array_offset]);
					tmp_array[array_offset] = 0;
                }
				else if(pool->type == EXT_XVPOOL_INTERNAL && (type == EXT_XVPOOL_EXTERNAL))
				{
				    tmp_array[array_offset] = 0;
				}
            }
		}
    }

    if((orig_backuplist_num != m_wcnt))
    {
        //printk("XX11 m_wcnt = %d, orig_backuplist num = %d, type = %d\n", m_wcnt, orig_backuplist_num, pool->type);
    }
    if((pool->xv_migration.read_cnt != m_rcnt))
    {
        //printk("XX22 m_rcnt = %d, pool->xv_migration.read_cnt = %d, type = %d\n", m_rcnt, pool->xv_migration.read_cnt, pool->type);
    }

    if( (pool->type == EXT_XVPOOL_EXTERNAL) && (type == EXT_XVPOOL_INTERNAL))
    { 
        spin_lock(&pool->lock);

        //check_sum1 = array_check_sum_ext(pool->ext_pool_attr->ext_xv_pool_array, bmap_entry_num*BITS_PER_LONG);

        for(i=0;i<bmap_entry_num;i++)
        {
            tmp_bitmap = *(tmp_xbitmap+i);

            for(j=0;j<BITS_PER_LONG;j++)
            {
                u32bit = GetRightMostOne(tmp_bitmap);
            
                if(tmp_bitmap == 0x0)
                    break;

                if((i==bmap_entry_num-1) && (u32bit>mod) && (mod !=0))
                    break;

                if(u32bit >= 0)
                {
                    array_offset = i*BITS_PER_LONG+u32bit;  
                        tmp_bitmap &= ~(1 << u32bit);
                    va_dest = kmap_atomic(tmp_array[array_offset], KM_USER1);
                    va_src  = get_ptr_atomic((void __iomem *)OffsetToPageIndex(array_offset), 0, KM_USER0, pool);
                    memcpy(va_dest, va_src, PAGE_SIZE);
                    kunmap_atomic(va_dest, KM_USER1);
                    put_ptr_atomic(va_src, KM_USER0, pool);
                }    
            }
        }

        //check_sum2 = array_check_sum_int(tmp_array, bmap_entry_num*BITS_PER_LONG);
        //BUG_ON(check_sum1 != check_sum2);
           
        orig_array = pool->ext_pool_attr->ext_xv_pool_array;
        pool->ext_pool_attr->ext_xv_pool_array = tmp_array;
        tmp_array = orig_array; 

        /* Update pool info and to the array switch, 
           because the page array whould change by notify 
           free during migration period */
                 
        pool->type = EXT_XVPOOL_INTERNAL;        
        spin_unlock(&pool->lock);
        
    }
    else if(pool->type == EXT_XVPOOL_INTERNAL && (type == EXT_XVPOOL_EXTERNAL))
    {
        spin_lock(&pool->lock);

        //check_sum1 = array_check_sum_int(pool->ext_pool_attr->ext_xv_pool_array, bmap_entry_num*BITS_PER_LONG);

        for(i=0;i<bmap_entry_num;i++)
        {
            tmp_bitmap = *(tmp_xbitmap+i);

            for(j=0;j<BITS_PER_LONG;j++)
            {
                u32bit = GetRightMostOne(tmp_bitmap);

                if(tmp_bitmap == 0x0)
                    break;

                if((i==bmap_entry_num-1) && (u32bit>mod) && (mod !=0))
                    break;

                if(u32bit >= 0)
                {
                    array_offset = i*BITS_PER_LONG+u32bit;
                    tmp_bitmap &= ~(1 <<u32bit);
                    va_dest = (void __iomem *) (pool->ext_pool_attr->ba_start+
                              (i*BITS_PER_LONG*PAGE_SIZE)+(PAGE_SIZE*u32bit));
                    va_src  = get_ptr_atomic((void __iomem *)OffsetToPageIndex(array_offset), 0, KM_USER1, pool);
                    memcpy(va_dest, va_src, PAGE_SIZE);
                    put_ptr_atomic(va_src, KM_USER1, pool);
                }
            }
        }

        //check_sum2 = array_check_sum_ext(tmp_array, bmap_entry_num*BITS_PER_LONG);   
        //BUG_ON(check_sum1 != check_sum2);
        orig_array = pool->ext_pool_attr->ext_xv_pool_array;
        pool->ext_pool_attr->ext_xv_pool_array = tmp_array;
        tmp_array = orig_array;

        pool->type = EXT_XVPOOL_EXTERNAL;

        spin_unlock(&pool->lock);

    } 
    else
    {
        BUG_ON(1);
    }

    return tmp_array;
}

unsigned int * ext_xv_create_bitmap(struct ext_xv_pool *pool)
{
    unsigned int bmap_entry_num, mod;

    bmap_entry_num = (pool->ext_pool_attr->size>>PAGE_SHIFT)/BITS_PER_LONG;
    mod = (pool->ext_pool_attr->size>>PAGE_SHIFT)%BITS_PER_LONG;

    if(mod != 0)
        bmap_entry_num ++;

    return kzalloc(sizeof(u32)*bmap_entry_num , GFP_KERNEL|__GFP_REPEAT);
}
EXPORT_SYMBOL_GPL(ext_xv_create_bitmap);

void ext_xv_delete_bitmap(unsigned int *bitmap)
{
    kfree(bitmap);
}
EXPORT_SYMBOL_GPL(ext_xv_delete_bitmap);

void __iomem** ext_xv_create_pool_array(struct ext_xv_pool *pool, u32 type, unsigned int *tmp_xbitmap)
{
    int i,j,k;
    unsigned int array_offset;    
    struct page *page ;
    unsigned int bmap_entry_num, mod;
    unsigned int u32bit, tmp_bitmap;
    void __iomem** tmp_array = NULL;

    if( (pool->type == EXT_XVPOOL_EXTERNAL) && (type == EXT_XVPOOL_INTERNAL))
    {
        //printk("Original pool type is external switch to internal !!!\n");

    }else if(pool->type == EXT_XVPOOL_INTERNAL && (type == EXT_XVPOOL_EXTERNAL))
    {
        //printk("Original pool type is internal switch to external !!!\n");

    }else
    {
        printk("unknown command!! pool type = %d, type assigned %d\n", pool->type, type);
        return NULL;
    }

    if(tmp_xbitmap == NULL)
    {
        printk("migration fail, tmp_array not exist\n");
        return NULL;
    }

    tmp_array = kzalloc((pool->ext_pool_attr->size >> PAGE_SHIFT)*
                                             sizeof(void __iomem *), GFP_KERNEL|__GFP_REPEAT);

    if(tmp_array == NULL)
    {
        printk("migration fail, create pool array fail\n");
        return NULL;
    }

    bmap_entry_num = (pool->ext_pool_attr->size>>PAGE_SHIFT)/BITS_PER_LONG;
    mod = (pool->ext_pool_attr->size>>PAGE_SHIFT)%BITS_PER_LONG;

    if(mod != 0)
        bmap_entry_num ++;
    
    for(i=0;i<bmap_entry_num;i++)
    {
        spin_lock(&pool->lock);
        tmp_bitmap = *(tmp_xbitmap+i);
        spin_unlock(&pool->lock);

        for(j=0;j<BITS_PER_LONG;j++)
        {
            u32bit = GetRightMostOne(tmp_bitmap);
            
            if(tmp_bitmap == 0x0)
                break;

            if((i==bmap_entry_num-1) && (u32bit>mod) && (mod !=0))
                break;

            if(u32bit >= 0)
            {
                array_offset = i*BITS_PER_LONG+u32bit;
                tmp_bitmap &= ~(1 <<u32bit);

                if(pool->type == EXT_XVPOOL_EXTERNAL)
                {
                    page = alloc_page(GFP_NOIO | __GFP_WAIT | __GFP_HIGHMEM | __GFP_FS);  // page alloc fail case?
                    //page = alloc_page(GFP_ATOMIC);  // page alloc fail case?

                    if(page == NULL)
                    {
                        printk("Page allocate fail in migration!\n");
                        
                        for(k=0;k<(pool->ext_pool_attr->size >> PAGE_SHIFT);k++)
                        {
                            if(tmp_array[k] != 0)
                            {
                                __free_page(tmp_array[k]);
                            }
                        }
 
                        kfree(tmp_array);
                        return NULL; // handler??
                    }

                    tmp_array[array_offset] = (void __iomem *)page;
                }
                else if(pool->type == EXT_XVPOOL_INTERNAL)
                {
                    tmp_array[array_offset] = 
                               (void __iomem *)(pool->ext_pool_attr->ba_start+
                               (i*BITS_PER_LONG*PAGE_SIZE)+(PAGE_SIZE*u32bit));
                }
                else
                {
                    BUG_ON(1);
                }
            }
        }
    }   

    return tmp_array;

}
EXPORT_SYMBOL_GPL(ext_xv_create_pool_array);

void ext_xv_delete_pool_array(struct ext_xv_pool *pool, void __iomem **tmp_array)
{
    int i,j;
    unsigned int bmap_entry_num, mod, array_offset;

    bmap_entry_num = (pool->ext_pool_attr->size>>PAGE_SHIFT)/BITS_PER_LONG;
    mod = (pool->ext_pool_attr->size>>PAGE_SHIFT)%BITS_PER_LONG;

    if(!tmp_array)
    {
        printk("tmp_array should not be NULL!\n");
        return;
    }

    if(mod != 0)
        bmap_entry_num ++;

    if(pool->type == EXT_XVPOOL_EXTERNAL)
    {	
        for(i=0;i<=bmap_entry_num;i++)
        {
            for(j=0;j<BITS_PER_LONG;j++)
            {
                array_offset = i*BITS_PER_LONG+j; 
                if(tmp_array[array_offset] != NULL)
                    __free_page(tmp_array[array_offset]);
            }
        }
    }
    kfree(tmp_array);
}
EXPORT_SYMBOL_GPL(ext_xv_delete_pool_array);


void __iomem ** ext_xv_pool_migration(struct ext_xv_pool *pool, u32 type, void __iomem **tmp_array, unsigned int *tmp_xbitmap, struct ext_xv_backuplist *backuplist)
{
    int ret=0;

    ret = pool_migration(pool, type, tmp_array, tmp_xbitmap, backuplist);

    return ret;
}
EXPORT_SYMBOL_GPL(ext_xv_pool_migration);

unsigned int ext_xv_add_page_to_backuplist(struct ext_xv_backuplist *backuplist, int cnt)
{
    int i;
    struct ext_xv_backup_page *tmp_backup_page=NULL;

    if(cnt==0)
        printk("infeasible number cnt = %d\n", cnt);

    for(i=0;i<cnt;i++)
    {
        if(backuplist->backup_page == NULL) // first alloc
        {   
            backuplist->backup_page = kzalloc(sizeof(struct ext_xv_backup_page), GFP_KERNEL|__GFP_REPEAT);
            
            if(!backuplist->backup_page)
                goto backup_page_withdraw;
            
            backuplist->backup_page->page = alloc_page(GFP_NOIO | __GFP_WAIT | __GFP_HIGHMEM | __GFP_FS);

            if(!backuplist->backup_page->page)
                goto backup_page_withdraw;
            backuplist->backup_page->next_backup_page = NULL;
        }
        else
        {
            tmp_backup_page = kzalloc(sizeof(struct ext_xv_backup_page), GFP_KERNEL|__GFP_REPEAT);
            
            if(!tmp_backup_page)
                goto backup_page_withdraw;
                
            tmp_backup_page->page = alloc_page(GFP_NOIO | __GFP_WAIT | __GFP_HIGHMEM | __GFP_FS);

            if(!tmp_backup_page->page)
                goto backup_page_withdraw;

            tmp_backup_page->next_backup_page = backuplist->backup_page;
            backuplist->backup_page = tmp_backup_page;
        }
 
        if(backuplist->backup_page == NULL)
        {
            printk("alloc backup_page fail while add page to backuplist\n");
		        goto backup_page_withdraw;
		    }

		    if(backuplist->backup_page->page == NULL)
		    {
		        printk("alloc page fail while add page to backuplist\n");
		        goto backup_page_withdraw;
		    }

		    backuplist->cnt++;
    }
	  return 1;

backup_page_withdraw:
    while(backuplist->cnt > 0)
    {
        tmp_backup_page = backuplist->backup_page;
        backuplist->backup_page = backuplist->backup_page->next_backup_page;
        __free_page(tmp_backup_page->page);
        kfree(tmp_backup_page);
        backuplist->cnt--;

    }

    return 0;
}
EXPORT_SYMBOL_GPL(ext_xv_add_page_to_backuplist);

void ext_xv_add_read_cnt(struct ext_xv_pool *pool, int cnt)
{
    spin_lock(&pool->lock);   
    pool->xv_migration.read_cnt += cnt;
    spin_unlock(&pool->lock);
}
EXPORT_SYMBOL_GPL(ext_xv_add_read_cnt);

void ext_xv_sub_read_cnt(struct ext_xv_pool *pool, int cnt)
{
    spin_lock(&pool->lock);
    pool->xv_migration.read_cnt -= cnt;
    spin_unlock(&pool->lock);
}
EXPORT_SYMBOL_GPL(ext_xv_sub_read_cnt);

void ext_xv_add_write_cnt(struct ext_xv_pool *pool, int cnt)
{
    spin_lock(&pool->lock);
    pool->xv_migration.write_cnt += cnt;
    spin_unlock(&pool->lock);
}
EXPORT_SYMBOL_GPL(ext_xv_add_write_cnt);

void ext_xv_sub_write_cnt(struct ext_xv_pool *pool, int cnt)
{
    spin_lock(&pool->lock);
    pool->xv_migration.write_cnt -= cnt;
    spin_unlock(&pool->lock);
}
EXPORT_SYMBOL_GPL(ext_xv_sub_write_cnt);


void ext_xv_set_migration(struct ext_xv_pool *pool)
{
    spin_lock(&pool->lock);
    pool->xv_migration.on_migration = 1;
    spin_unlock(&pool->lock);
}
EXPORT_SYMBOL_GPL(ext_xv_set_migration);

void ext_xv_clear_migration(struct ext_xv_pool *pool)
{
    spin_lock(&pool->lock);
    pool->xv_migration.on_migration = 0;
    spin_unlock(&pool->lock);
}
EXPORT_SYMBOL_GPL(ext_xv_clear_migration);

unsigned int ext_xv_get_migration(struct ext_xv_pool *pool)
{
    unsigned int cnt;

    spin_lock(&pool->lock);
    cnt = pool->xv_migration.on_migration;
    spin_unlock(&pool->lock);

    return cnt;
}
EXPORT_SYMBOL_GPL(ext_xv_get_migration);

unsigned int ext_xv_get_read_cnt(struct ext_xv_pool *pool)
{
    unsigned int cnt;

    spin_lock(&pool->lock);
    cnt = pool->xv_migration.read_cnt;
    spin_unlock(&pool->lock);

    return cnt;
}
EXPORT_SYMBOL_GPL(ext_xv_get_read_cnt);

unsigned int ext_xv_get_write_cnt(struct ext_xv_pool *pool)
{
    unsigned int cnt;

    spin_lock(&pool->lock);
    cnt = pool->xv_migration.write_cnt;
    spin_unlock(&pool->lock);

    return cnt;
}
EXPORT_SYMBOL_GPL(ext_xv_get_write_cnt);

void ext_xv_bitmap_duplicate(struct ext_xv_pool *pool, unsigned int *tmp_xbitmap)
{ 
    unsigned int bmap_entry_num, mod;

    bmap_entry_num = (pool->ext_pool_attr->size>>PAGE_SHIFT)/BITS_PER_LONG;
    mod = (pool->ext_pool_attr->size>>PAGE_SHIFT)%BITS_PER_LONG;

    if(mod != 0)
        bmap_entry_num ++;

    spin_lock(&pool->lock); 
    memcpy(tmp_xbitmap, pool->ext_pool_attr->xbitmap, sizeof(u32)*bmap_entry_num);
    spin_unlock(&pool->lock);

}
EXPORT_SYMBOL_GPL(ext_xv_bitmap_duplicate);

void ext_xv_show_block_info(struct ext_xv_pool *pool)
{
    unsigned int i=0;
	unsigned int size=0;

    size = XV_MIN_ALLOC_SIZE;
  
    for(i=0;i<NUM_FREE_LISTS;i++)
    {
		if(pool->freelist[i].num)
    	    printk("size %d: num = %d \n", size, pool->freelist[i].num);
        size+=FL_DELTA;  
    }
	//printk("\n");
	
}
EXPORT_SYMBOL_GPL(ext_xv_show_block_info);
