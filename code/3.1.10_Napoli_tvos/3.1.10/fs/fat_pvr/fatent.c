/*
 * Copyright (C) 2004, OGAWA Hirofumi
 * Released under GPL v2.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/msdos_fs.h>
#include <linux/blkdev.h>
#include "fat.h"

#include <linux/pagemap.h>
#include <linux/mpage.h>
#include <linux/buffer_head.h>
#include <linux/writeback.h>




struct fatent_operations {
	void (*ent_blocknr)(struct super_block *, int, int *, sector_t *);
	void (*ent_set_ptr)(struct fat_entry2 *, int);
	int (*ent_bread)(struct super_block *, struct fat_entry2 *,
			 int, sector_t,u8);
	int (*ent_get)(struct fat_entry2 *);
	void (*ent_put)(struct fat_entry2 *, struct super_block *, int);
	int (*ent_next)(struct fat_entry2 *);
};

s64 fatt_inode_map_read(int *p_errno, struct inode *vi, s64 pos, s64 count, u8 *b);
s64 fatt_inode_map_write(int *p_errno, struct inode *vi, s64 pos, s64 count, u8 *b);
int fattable_map(struct inode *vi, struct page ** mpage, s64 pos, u8 raflag);


static DEFINE_SPINLOCK(fat12_entry_lock);

static int writeback_fatt(struct inode *inode)
{

	int ret;
	struct address_space *mapping = inode->i_mapping;
	struct writeback_control wbc = {
	       .sync_mode = WB_SYNC_NONE,
	      .nr_to_write = 1024,
	};
	/* if we used WB_SYNC_ALL, sync_inode waits for the io for the
	* inode to finish.  So WB_SYNC_NONE is sent down to sync_inode
	* and filemap_fdatawrite is used for the data blocks
	*/
	ret = sync_inode(inode, &wbc);
	if (!ret) {
	       ret = filemap_fdatawrite(mapping);
	}
    return ret;
}


static void fat12_ent_blocknr(struct super_block *sb, int entry,
			      int *offset, sector_t *blocknr)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	int bytes = entry + (entry >> 1);
	WARN_ON(entry < FAT_START_ENT || sbi->max_cluster <= entry);
	*offset = bytes & (PAGE_CACHE_SIZE - 1);
    *blocknr = (bytes / PAGE_CACHE_SIZE) ;//* PAGE_CACHE_SIZE;//(bytes >> PAGE_CACHE_SIZE) << PAGE_CACHE_SIZE;
}

static void fat_ent_blocknr(struct super_block *sb, int entry,
			    int *offset, sector_t *blocknr)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	int bytes = (entry << sbi->fatent_shift);
	WARN_ON(entry < FAT_START_ENT || sbi->max_cluster <= entry);
	*offset = bytes & (PAGE_CACHE_SIZE - 1);
    *blocknr = (bytes / PAGE_CACHE_SIZE) ;//* PAGE_CACHE_SIZE;//(bytes >> PAGE_CACHE_SIZE) << PAGE_CACHE_SIZE;
}

static void fat12_ent_set_ptr(struct fat_entry2 *fatent, int offset)
{
/*FIXME*/
//    u8 * kaddr;
//    kaddr = page_address(fatent->entpage[0]);
	if (fatent->nr_bhs == 1) {
		//WARN_ON(offset >= (bhs[0]->b_size - 1));
		fatent->u.ent12_p[0] = fatent->pageaddress + offset;
		fatent->u.ent12_p[1] = fatent->pageaddress + (offset + 1);
	} else {
		//WARN_ON(offset != (bhs[0]->b_size - 1));
		fatent->u.ent12_p[0] = fatent->pageaddress + offset;
		fatent->u.ent12_p[1] = fatent->pageaddress;
	}

}

static void fat16_ent_set_ptr(struct fat_entry2 *fatent, int offset)
{
	WARN_ON(offset & (2 - 1));
	fatent->u.ent16_p = (__le16 *)(fatent->pageaddress + offset);
}

static void fat32_ent_set_ptr(struct fat_entry2 *fatent, int offset)
{
	WARN_ON(offset & (4 - 1));
	fatent->u.ent32_p = (__le32 *)(fatent->pageaddress+ offset);
}

static int fat12_ent_bread(struct super_block *sb, struct fat_entry2 *fatent,
			   int offset, sector_t blocknr,u8 raflag)
{
#if 0
	struct buffer_head **bhs = fatent->bhs;

	WARN_ON(blocknr < MSDOS_SB(sb)->fat_start);
	fatent->fat_inode = MSDOS_SB(sb)->fat_inode;

	bhs[0] = sb_bread(sb, blocknr);
	if (!bhs[0])
		goto err;

	if ((offset + 1) < sb->s_blocksize)
		fatent->nr_bhs = 1;
	else {
		/* This entry is block boundary, it needs the next block */
		blocknr++;
		bhs[1] = sb_bread(sb, blocknr);
		if (!bhs[1])
			goto err_brelse;
		fatent->nr_bhs = 2;
	}
	fat12_ent_set_ptr(fatent, offset);
	return 0;

err_brelse:
	brelse(bhs[0]);
err:
	fat_msg(sb, KERN_DEBUG, "FAT read failed (blocknr %llu)", (llu)blocknr);
#endif
	return -EIO;
}

static int fat_ent_bread(struct super_block *sb, struct fat_entry2 *fatent,
			 int offset, sector_t pagenr,u8 raflag)
{
    int errno;

	struct fatent_operations *ops = MSDOS_SB(sb)->fatent_ops;
	struct msdos_sb_info *sbi = MSDOS_SB(sb);

	WARN_ON(pagenr < 0);
	fatent->fat_inode = MSDOS_SB(sb)->fat_inode;
    if(fatent->entpage[0] != NULL)
        fatt_unmap_page(fatent->entpage[0]);
	fatent->pageaddress = NULL;
	errno = fattable_map(sbi->fattab_inode, &fatent->entpage[0], pagenr<<PAGE_CACHE_SHIFT,raflag);
	if (errno < 0) {

		printk(KERN_ERR "FAT: FAT read map fail (blocknr %lld)\n",pagenr);
		return -EIO;
	}
	fatent->b_size = PAGE_CACHE_SIZE;//blocksize;


#if 0
    fatt_inode_map_read(&errno, sbi->fattab_inode, blocknr/*FIXME*/, PAGE_CACHE_SIZE, fatent->bhs[0]);
    //a_ops->readpage();

	if (!fatent->bhs[0]) {
		printk(KERN_ERR "FAT: FAT read failed (blocknr %llu)\n",
		       (llu)blocknr);
		return -EIO;
	}
#endif

	fatent->nr_bhs = 1;
    	fatent->pagenr = pagenr;
    	fatent->pageaddress = page_address(fatent->entpage[0]);;
	ops->ent_set_ptr(fatent, offset);
	return 0;
}

static int fat12_ent_get(struct fat_entry2 *fatent)
{
	u8 **ent12_p = fatent->u.ent12_p;
	int next;

	spin_lock(&fat12_entry_lock);
	if (fatent->entry & 1)
		next = (*ent12_p[0] >> 4) | (*ent12_p[1] << 4);
	else
		next = (*ent12_p[1] << 8) | *ent12_p[0];
	spin_unlock(&fat12_entry_lock);

	next &= 0x0fff;
	if (next >= BAD_FAT12)
		next = FAT_ENT_EOF;
	return next;
}

static int fat16_ent_get(struct fat_entry2 *fatent)
{
	int next = le16_to_cpu(*fatent->u.ent16_p);
	WARN_ON((unsigned long)fatent->u.ent16_p & (2 - 1));
	if (next >= BAD_FAT16)
		next = FAT_ENT_EOF;
	return next;
}

static int fat32_ent_get(struct fat_entry2 *fatent)
{
	int next = le32_to_cpu(*fatent->u.ent32_p) & 0x0fffffff;
	WARN_ON((unsigned long)fatent->u.ent32_p & (4 - 1));
	if (next >= BAD_FAT32)
		next = FAT_ENT_EOF;
	return next;
}

static void fat12_ent_put(struct fat_entry2 *fatent, struct super_block *sb, int new)
{
	u8 **ent12_p = fatent->u.ent12_p;

	if (new == FAT_ENT_EOF)
		new = EOF_FAT12;

	spin_lock(&fat12_entry_lock);
	if (fatent->entry & 1) {
		*ent12_p[0] = (new << 4) | (*ent12_p[0] & 0x0f);
		*ent12_p[1] = new >> 4;
	} else {
		*ent12_p[0] = new & 0xff;
		*ent12_p[1] = (*ent12_p[1] & 0xf0) | (new >> 8);
	}
	spin_unlock(&fat12_entry_lock);

}

static void fat16_ent_put(struct fat_entry2 *fatent, struct super_block *sb, int new)
{
    struct msdos_sb_info *sbi = MSDOS_SB(sb);
    struct fatent_operations *ops = sbi->fatent_ops;
    sector_t blocknr;
    int offset;
    unsigned bh_size = sb->s_blocksize;
    struct page* page = fatent->entpage[0];

    ops->ent_blocknr(sb, fatent->entry, &offset, &blocknr);

    if (new == FAT_ENT_EOF)
    	new = EOF_FAT16;

    *fatent->u.ent16_p = cpu_to_le16(new);

    if(page == NULL) {
        printk("ERROR !!!!! fat16 need remap\n");
        fattable_map(sbi->fattab_inode, &fatent->entpage[0], blocknr,0);
        page = fatent->entpage[0];
    }
    set_page_dirty(page);
    if (likely(page_has_buffers(page))) {
        struct buffer_head *bh, *head;
        unsigned int  bh_ofs;
        bh = head = page_buffers(page);
        do {
            bh_ofs = bh_offset(bh);
            if (bh_ofs + bh_size <= offset)
                continue;
            else {
                set_buffer_dirty(bh);
                break; //only one bh dirty
            }
        } while ((bh = bh->b_this_page) != head);
    }
}

static void fat32_ent_put(struct fat_entry2 *fatent, struct super_block *sb, int new)
{
	//unsigned short blocksize = sbi->cluster_size >> sbi->sec_per_clus;
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct fatent_operations *ops = sbi->fatent_ops;
	sector_t blocknr;
	int offset;
    unsigned bh_size = sb->s_blocksize;
    struct page* page = fatent->entpage[0];

	ops->ent_blocknr(sb, fatent->entry, &offset, &blocknr);

	if (new == FAT_ENT_EOF)
		new = EOF_FAT32;

	WARN_ON(new & 0xf0000000);
	new |= le32_to_cpu(*fatent->u.ent32_p) & ~0x0fffffff;
	*fatent->u.ent32_p = cpu_to_le32(new);

	if(page == NULL) {
        printk("ERROR !!!!! need remap\n");
        fattable_map(sbi->fattab_inode, &fatent->entpage[0], blocknr,0);
        page = fatent->entpage[0];
	}
	set_page_dirty(page);
	if (likely(page_has_buffers(page))) {
		struct buffer_head *bh, *head;
		unsigned int  bh_ofs;
		bh = head = page_buffers(page);
		do {
			bh_ofs = bh_offset(bh);
			if (bh_ofs + bh_size <= offset)
				continue;
            else {
			    set_buffer_dirty(bh);
                break; //only one bh dirty
            }
		} while ((bh = bh->b_this_page) != head);
	}
}

static int fat12_ent_next(struct fat_entry2 *fatent)
{
#if 0
	u8 **ent12_p = fatent->u.ent12_p;
	struct buffer_head **bhs = fatent->bhs;
	u8 *nextp = ent12_p[1] + 1 + (fatent->entry & 1);

	fatent->entry++;
	if (fatent->nr_bhs == 1) {
		WARN_ON(ent12_p[0] > (u8 *)(bhs[0]->b_data + (bhs[0]->b_size - 2)));
		WARN_ON(ent12_p[1] > (u8 *)(bhs[0]->b_data + (bhs[0]->b_size - 1)));
		if (nextp < (u8 *)(bhs[0]->b_data + (bhs[0]->b_size - 1))) {
			ent12_p[0] = nextp - 1;
			ent12_p[1] = nextp;
			return 1;
		}
	} else {
		WARN_ON(ent12_p[0] != (u8 *)(bhs[0]->b_data + (bhs[0]->b_size - 1)));
		WARN_ON(ent12_p[1] != (u8 *)bhs[1]->b_data);
		ent12_p[0] = nextp - 1;
		ent12_p[1] = nextp;
		brelse(bhs[0]);
		bhs[0] = bhs[1];
		fatent->nr_bhs = 1;
		return 1;
	}
	ent12_p[0] = NULL;
	ent12_p[1] = NULL;
#endif    
	return 0;
}

static int fat16_ent_next(struct fat_entry2 *fatent)
{
	fatent->entry++;
	if (fatent->u.ent16_p < (__le16 *)(fatent->pageaddress + (fatent->b_size - 2))) {
		fatent->u.ent16_p++;
		return 1;
	}
	fatent->u.ent16_p = NULL;
	return 0;
}

static int fat32_ent_next(struct fat_entry2 *fatent)
{
	fatent->entry++;
	if (fatent->u.ent32_p < (__le32 *)(fatent->pageaddress + (fatent->b_size - 4))) {
		fatent->u.ent32_p++;
		return 1;
	}
	fatent->u.ent32_p = NULL;
	return 0;
}

static inline int __fatt_get_block(struct inode *inode, sector_t iblock,
				  unsigned long *max_blocks,
				  struct buffer_head *bh_result, int create)
{
	struct super_block *sb = inode->i_sb;
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	unsigned long mapped_blocks;
	sector_t phys;
	//int err, offset;

	WARN_ON(iblock > sbi->fat_length);

	//printk("__fatt_get_block:%X + %X",sbi->fat_start, iblock);
	phys = sbi->fat_start + iblock;//(bytes >> sb->s_blocksize_bits);
	mapped_blocks = 1;


	if (phys) {
		map_bh(bh_result, sb, phys);
		*max_blocks = min(mapped_blocks, *max_blocks);
		return 0;
	}


	BUG_ON(!phys);
	BUG_ON(*max_blocks != mapped_blocks);
	set_buffer_new(bh_result);
	map_bh(bh_result, sb, phys);

	return -1;
}

static int fatt_get_block(struct inode *inode, sector_t iblock,
			 struct buffer_head *bh_result, int create)
{
	struct super_block *sb = inode->i_sb;
	unsigned long max_blocks = bh_result->b_size >> inode->i_blkbits;
	int err;

	err = __fatt_get_block(inode, iblock, &max_blocks, bh_result, create);
	if (err)
		return err;
	bh_result->b_size = max_blocks << sb->s_blocksize_bits;
	return 0;
}

static int fatt_writepage(struct page *page, struct writeback_control *wbc)
{
	return block_write_full_page(page, fatt_get_block, wbc);
}

static int fatt_writepages(struct address_space *mapping,
			  struct writeback_control *wbc)
{
	return mpage_writepages(mapping, wbc, fatt_get_block);
}

static int fatt_readpage(struct file *file, struct page *page)
{
	return mpage_readpage(page, fatt_get_block);
}

static int fatt_readpages(struct file *file, struct address_space *mapping,
			 struct list_head *pages, unsigned nr_pages)
{
	return mpage_readpages(mapping, pages, nr_pages, fatt_get_block);
}

static int fatt_write_begin(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned flags,
			struct page **pagep, void **fsdata)
{
	*pagep = NULL;
	return cont_write_begin(file, mapping, pos, len, flags, pagep, fsdata,
				fatt_get_block,
				&MSDOS_I(mapping->host)->mmu_private);
}

static int fatt_write_end(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned copied,
			struct page *pagep, void *fsdata)
{
	struct inode *inode = mapping->host;
	int err;
	err = generic_write_end(file, mapping, pos, len, copied, pagep, fsdata);
	if (!(err < 0) && !(MSDOS_I(inode)->i_attrs & ATTR_ARCH)) {
		inode->i_mtime = inode->i_ctime = CURRENT_TIME_SEC;
		MSDOS_I(inode)->i_attrs |= ATTR_ARCH;
		mark_inode_dirty(inode);
	}
	return err;
}
/*
void tblock_sync_page(struct page *page)
{
	struct address_space *mapping;

	smp_mb();
	mapping = page_mapping(page);
	if (mapping)
		blk_run_backing_dev(mapping->backing_dev_info, page);
}
*/




static struct fatent_operations fat12_ops = {
	.ent_blocknr	= fat12_ent_blocknr,
	.ent_set_ptr	= fat12_ent_set_ptr,
	.ent_bread	= fat12_ent_bread,
	.ent_get	= fat12_ent_get,
	.ent_put	= fat12_ent_put,
	.ent_next	= fat12_ent_next,
};

static struct fatent_operations fat16_ops = {
	.ent_blocknr	= fat_ent_blocknr,
	.ent_set_ptr	= fat16_ent_set_ptr,
	.ent_bread	= fat_ent_bread,
	.ent_get	= fat16_ent_get,
	.ent_put	= fat16_ent_put,
	.ent_next	= fat16_ent_next,
};

static struct fatent_operations fat32_ops = {
	.ent_blocknr	= fat_ent_blocknr,
	.ent_set_ptr	= fat32_ent_set_ptr,
	.ent_bread	= fat_ent_bread,
	.ent_get	= fat32_ent_get,
	.ent_put	= fat32_ent_put,
	.ent_next	= fat32_ent_next,
};

static const struct address_space_operations fatt_aops = {
	.readpage	= fatt_readpage,
	.readpages	= fatt_readpages,
	.writepage	= fatt_writepage,
	.writepages	= fatt_writepages,
	/*.sync_page	= tblock_sync_page,*/
	.write_begin	= fatt_write_begin,
	.write_end	= fatt_write_end,
	.direct_IO	= 0,//fat_direct_IO,
	.bmap		= 0//_fat_bmap
};

s64 fatt_inode_map_write(int *p_errno, struct inode *vi, s64 pos, s64 count, u8 *b)
{
    u32 offset = 0;
    s64 t;
    u32 br, total = 0;
    u32 index;
    struct address_space *mapping;
    struct page *page;
    u8 *kaddr;
    s64 isize;
    //s64 fill_zero_start, fill_zero_length;

    unsigned bh_size = vi->i_sb->s_blocksize;
    struct inode *ni;

    if (!vi  || !b || pos < 0 || count < 0) {
        *p_errno = EINVAL;
        printk("%s:   b=%p  pos=%lld  count=%lld",
               __FUNCTION__,  b, (long long)pos,
               (long long)count);
        return -1;
    }
       ni = vi;//NTFS_I(vi);
    mapping = vi->i_mapping;
	isize =  vi->i_size;//i_size_read(vi);
    index = (u32)(pos/PAGE_CACHE_SIZE);
	while(count) {
		/* Update @index and get the next page. */
		page = fatt_map_page(mapping, index);
		if (IS_ERR(page))
			break;

		kaddr = page_address(page);
		t = pos;
		offset = do_div(t, PAGE_CACHE_SIZE);
		br = PAGE_CACHE_SIZE-offset;
        //printk("\nt:%lld,offset:%d,br:%d\n",t,offset,br);
		if(br > count)
			br = count;
        memcpy(kaddr+offset, b, br);
        total += br;
		b += br;
		pos +=br;
		count -= br;
		set_page_dirty(page);
		if (likely(page_has_buffers(page))) {
			struct buffer_head *bh, *head;
			unsigned int  bh_ofs;
			bh = head = page_buffers(page);
			do {
				bh_ofs = bh_offset(bh);
				if (bh_ofs + bh_size <= offset)
					continue;
				if (unlikely(bh_ofs >= offset+br))
					break;
				set_buffer_dirty(bh);
			} while ((bh = bh->b_this_page) != head);
		}
		fatt_unmap_page(page);
		index++;
	}

	if(count && total == 0) {
		*p_errno = EIO;
		return -1;
	}


	return total;
}


#define MAX_READAHEAD_PAGES	64
static int fat_tab_readahead(struct inode *inode,pgoff_t offset,int nr_pages)
{
	int ret;
	struct address_space *mapping = inode->i_mapping;

	ret = do_page_cache_readahead(mapping,NULL,offset,nr_pages);

	return ret;
}


int fattable_map(struct inode *vi, struct page ** mpage, s64 pos, u8 raflag)
{
	u32 index;
	struct address_space *mapping;
	s64 isize;
	struct inode *ni;
	int ret = 0;
//	int start = 0;
//	static int s_last_ra_index = 0;

	if (!vi || pos < 0 ) {
		printk("1111%s:  pos=%lld  vi=%X\n",
				__FUNCTION__,(long long)pos,
				(unsigned int)vi);
        return -1;
    }
    ni = vi;//NTFS_I(vi);
    isize =  vi->i_size;//i_size_read(vi);

    if(pos > isize) {
		printk("2222%s: pos=%lld, isize = %lld ",	__FUNCTION__, pos, isize);
		return -1;
	}

    mapping = vi->i_mapping;

    index = (u32)(pos/PAGE_CACHE_SIZE);

/*
	if(raflag)
	{
		if(s_last_ra_index - index < MAX_READAHEAD_PAGES / 8)	//backward
		{
			if(index & ~0)
				start = index + MAX_READAHEAD_PAGES / 8;
			ret = fat_tab_readahead(ni, start, MAX_READAHEAD_PAGES);
			s_last_ra_index += MAX_READAHEAD_PAGES;
		}
		else if(s_last_ra_index - index > MAX_READAHEAD_PAGES )	//forward
		{
			int start = max(0,index - MAX_READAHEAD_PAGES/2);
			ret = fat_tab_readahead(ni, start, MAX_READAHEAD_PAGES);
			s_last_ra_index = start + MAX_READAHEAD_PAGES;
		}
	}
*/

 	if(raflag)
 	{
		*mpage = find_get_page(mapping, index);

		if(!*mpage)
		{
			ret = fat_tab_readahead(ni, index, MAX_READAHEAD_PAGES);
		}
		else
		{
			page_cache_release(*mpage);
		}
	}
	*mpage = fatt_map_page(mapping, index);
	if (IS_ERR(*mpage)) {
		printk("page map error!!!!!!!!!!\n");
		return -1;
	}

    return 0;


}

s64 fatt_inode_map_read(int *p_errno, struct inode *vi, s64 pos, s64 count, u8 *b)
{
    u32 offset = 0;
    s64 t;
	u32 br, total = 0;
//	int i,j;
//	unsigned int * tmp;

    u32 index;
	struct address_space *mapping;
    struct page *page;
    u8 *kaddr;
    s64 isize;
    struct inode *ni;

	if (!vi  || !b || pos < 0 || count < 0) {
		*p_errno = EINVAL;
		printk("1111%s:   b=%p  pos=%lld  count=%lld vi=%X\n",
				__FUNCTION__,  b, (long long)pos,
				(long long)count, (unsigned int)vi);
        return -1;
    }
    ni = vi;//NTFS_I(vi);
    isize =  vi->i_size;//i_size_read(vi);
    if(pos > isize) {
	    *p_errno = EINVAL;
		printk("2222%s:   b=%p  pos=%lld  count=%lld",
				__FUNCTION__,  b, (long long)pos,
				(long long)count);
		return -1;
	}
    if(pos+count >  isize)
        count =  isize-pos;

    mapping = vi->i_mapping;

    index = (u32)(pos/PAGE_CACHE_SIZE);

    while(count) {
		/* Update @index and get the next page. */
		page = fatt_map_page(mapping, index);
		if (IS_ERR(page)) {
			printk("page map error!!!!!!!!!!\n");
			break;
		}
        kaddr = page_address(page);

        t = pos;
        offset = do_div(t, PAGE_CACHE_SIZE);
        br = PAGE_CACHE_SIZE-offset;
        if(br > count)
            br = count;

        memcpy(b, kaddr+offset, br);
		total += br;
        b += br;
        pos +=br;
        count -= br;
        fatt_unmap_page(page);
        index++;
    }

    if(count && total == 0)
        return -1;
	return total;
}



static inline void lock_fat(struct msdos_sb_info *sbi)
{
	mutex_lock(&sbi->fat_lock);
}

static inline void unlock_fat(struct msdos_sb_info *sbi)
{
	mutex_unlock(&sbi->fat_lock);
}

int fat_table_init(struct inode *inode)
{
	struct super_block *sb = inode->i_sb;
	struct msdos_sb_info *sbi = MSDOS_SB(sb);

	MSDOS_I(inode)->i_pos = 0;
	inode->i_uid = current_uid();//sbi->options.fs_uid;
	inode->i_gid = current_gid();//sbi->options.fs_gid;
	inode->i_version++;
	inode->i_generation = 0;
	inode->i_mode = fat_make_mode(sbi, ATTR_SYS, S_IRWXUGO);
	inode->i_op = NULL;//sbi->dir_ops;
	inode->i_fop = NULL;//&fat_dir_operations;
	inode->i_mapping->a_ops = &fatt_aops;
	if (sbi->fat_bits == 32) {
		MSDOS_I(inode)->i_start = 0;//sbi->root_cluster;
		inode->i_size = sbi->fat_length * (sbi->cluster_size / sbi->sec_per_clus);//sbi->dir_entries * sizeof(struct msdos_dir_entry);
		
	} else {
		MSDOS_I(inode)->i_start = 0;
		inode->i_size = sbi->fat_length * (sbi->cluster_size / sbi->sec_per_clus);//sbi->dir_entries * sizeof(struct msdos_dir_entry);
	}

	//printk("\nfat_length:%d,cluster_size:%d,secPclus:%d,isize:%ld,isizehex:%X\n",
		//sbi->fat_length,sbi->cluster_size,sbi->sec_per_clus,inode->i_size,inode->i_size);
	inode->i_blocks = ((inode->i_size + (sbi->cluster_size - 1))
			   & ~((loff_t)sbi->cluster_size - 1)) >> 9;
	MSDOS_I(inode)->i_logstart = 0;
	MSDOS_I(inode)->mmu_private = inode->i_size;

	fat_save_attrs(inode, ATTR_SYS);
	inode->i_mtime.tv_sec = inode->i_atime.tv_sec = inode->i_ctime.tv_sec = 0;
	inode->i_mtime.tv_nsec = inode->i_atime.tv_nsec = inode->i_ctime.tv_nsec = 0;

	return 0;
}



void fat_ent_access_init(struct super_block *sb)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);

	mutex_init(&sbi->fat_lock);

	switch (sbi->fat_bits) {
	case 32:
		sbi->fatent_shift = 2;
		sbi->fatent_ops = &fat32_ops;
		break;
	case 16:
		sbi->fatent_shift = 1;
		sbi->fatent_ops = &fat16_ops;
		break;
	case 12:
		sbi->fatent_shift = -1;
		sbi->fatent_ops = &fat12_ops;
		break;
	}
}

static inline int fat_ent_update_ptr(struct super_block *sb,
				     struct fat_entry2 *fatent,
				     int offset, unsigned pagenr)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct fatent_operations *ops = sbi->fatent_ops;

	/* Is this fatent's blocks including this entry? */
	if (!fatent->nr_bhs || fatent->pagenr != pagenr)
		return 0;
#if 0    
	if (sbi->fat_bits == 12) {
		if ((offset + 1) < sb->s_blocksize) {
			/* This entry is on bhs[0]. */
			if (fatent->nr_bhs == 2) {
				brelse(bhs[1]);
				fatent->nr_bhs = 1;
			}
		} else {
			/* This entry needs the next block. */
			if (fatent->nr_bhs != 2)
				return 0;
			if (bhs[1]->b_blocknr != (blocknr + 1))
				return 0;
		}
	}
#endif    
	ops->ent_set_ptr(fatent, offset);
	return 1;
}

int fat_ent_read(struct inode *inode, struct fat_entry2 *fatent, int entry)
{
	struct super_block *sb = inode->i_sb;
	struct msdos_sb_info *sbi = MSDOS_SB(inode->i_sb);
	struct fatent_operations *ops = sbi->fatent_ops;
	int err, offset;
	sector_t blocknr;

	if (entry < FAT_START_ENT || sbi->max_cluster <= entry) {
		fatent_brelse2(fatent);
		fat_fs_error(sb, "invalid access to FAT (entry 0x%08x)", entry);
		return -EIO;
	}

	fatent_set_entry2(fatent, entry);
	ops->ent_blocknr(sb, entry, &offset, &blocknr);

	if (!fat_ent_update_ptr(sb, fatent, offset, blocknr)) {
		fatent_brelse2(fatent);
		err = ops->ent_bread(sb, fatent, offset, blocknr,0);
		if (err)
			return err;
	}
	return ops->ent_get(fatent);
}

#if 0
/* FIXME: We can write the blocks as more big chunk. */
static int fat_mirror_bhs(struct super_block *sb, struct buffer_head **bhs,
			  int nr_bhs)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct buffer_head *c_bh;
	int err, n, copy;

	err = 0;
	for (copy = 1; copy < sbi->fats; copy++) {
		sector_t backup_fat = sbi->fat_length * copy;

		for (n = 0; n < nr_bhs; n++) {
			c_bh = sb_getblk(sb, backup_fat + bhs[n]->b_blocknr);
			if (!c_bh) {
				err = -ENOMEM;
				goto error;
			}
			memcpy(c_bh->b_data, bhs[n]->b_data, sb->s_blocksize);
			set_buffer_uptodate(c_bh);
			mark_buffer_dirty_inode(c_bh, sbi->fat_inode);
			if (sb->s_flags & MS_SYNCHRONOUS)
				err = sync_dirty_buffer(c_bh);
			brelse(c_bh);
			if (err)
				goto error;
		}
	}
error:
	return err;
}
#endif



int fat_ent_write(struct inode *inode, struct fat_entry2 *fatent,
		  int new, int wait)
{
	struct super_block *sb = inode->i_sb;
	struct fatent_operations *ops = MSDOS_SB(sb)->fatent_ops;


	ops->ent_put(fatent, sb, new);
	if (wait) {
		printk("\nhit!!\n");
	}
	return 0;//fat_mirror_bhs(sb, fatent->bhs, fatent->nr_bhs);
}

static inline int fat_ent_next(struct msdos_sb_info *sbi,
			       struct fat_entry2 *fatent)
{
	if (sbi->fatent_ops->ent_next(fatent)) {
		if (fatent->entry < sbi->max_cluster)
			return 1;
	}
	return 0;
}

static inline int fat_ent_read_block(struct super_block *sb,
				     struct fat_entry2 *fatent,u8 raflag)
{
	struct fatent_operations *ops = MSDOS_SB(sb)->fatent_ops;
	sector_t blocknr;
	int offset;

	fatent_brelse2(fatent);
	ops->ent_blocknr(sb, fatent->entry, &offset, &blocknr);
	return ops->ent_bread(sb, fatent, offset, blocknr,raflag);
}
#if 0
static void fat_collect_bhs(struct buffer_head **bhs, int *nr_bhs,
			    struct fat_entry *fatent)
{
	int n, i;

	for (n = 0; n < fatent->nr_bhs; n++) {
		for (i = 0; i < *nr_bhs; i++) {
			if (fatent->bhs[n] == bhs[i])
				break;
		}
		if (i == *nr_bhs) {
			get_bh(fatent->bhs[n]);
			bhs[i] = fatent->bhs[n];
			(*nr_bhs)++;
		}
	}
}
#endif
int fat_alloc_clusters(struct inode *inode, int *cluster, int nr_cluster)
{
	struct super_block *sb = inode->i_sb;
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct fatent_operations *ops = sbi->fatent_ops;
	struct fat_entry2 fatent, prev_ent;
	int count, err, nr_bhs, idx_clus;

	BUG_ON(nr_cluster > (MAX_BUF_PER_PAGE / 2));	/* fixed limit */

	lock_fat(sbi);
	if (sbi->free_clusters != -1 && sbi->free_clus_valid &&
	    sbi->free_clusters < nr_cluster) {
		unlock_fat(sbi);
		return -ENOSPC;
	}

	err = nr_bhs = idx_clus = 0;
	count = FAT_START_ENT;
	fatent_init(&prev_ent);
	fatent_init(&fatent);
	fatent_set_entry2(&fatent, sbi->prev_free + 1);
	while (count < sbi->max_cluster) {
		if (fatent.entry >= sbi->max_cluster)
			fatent.entry = FAT_START_ENT;
		fatent_set_entry2(&fatent, fatent.entry);
		err = fat_ent_read_block(sb, &fatent,0);
		if (err)
			goto out;

		/* Find the free entries in a block */
		do {
			if (ops->ent_get(&fatent) == FAT_ENT_FREE) {
				int entry = fatent.entry;

				/* make the cluster chain */
				ops->ent_put(&fatent,sb, FAT_ENT_EOF);
				if (prev_ent.nr_bhs)
					ops->ent_put(&prev_ent,sb, entry);


				sbi->prev_free = entry;
				if (sbi->free_clusters != -1)
					sbi->free_clusters--;
				sb->s_dirt = 1;

				cluster[idx_clus] = entry;
				idx_clus++;
				if (idx_clus == nr_cluster)
					goto out;

				/*
				 * fat_collect_bhs() gets ref-count of bhs,
				 * so we can still use the prev_ent.
				 */
				prev_ent = fatent;
			}
			count++;
			if (count == sbi->max_cluster)
				break;
		} while (fat_ent_next(sbi, &fatent));
	}

	/* Couldn't allocate the free entries */
	sbi->free_clusters = 0;
	sbi->free_clus_valid = 1;
	sb->s_dirt = 1;
	err = -ENOSPC;

out:
    fatent_brelse2(&fatent);
	unlock_fat(sbi);
	
	if (!err) {
        if ( inode_needs_sync(inode) ) {
            writeback_fatt(sbi->fattab_inode);
        }

	}


	if (err && idx_clus)
		fat_free_clusters(inode, cluster[0]);

	return err;
}

int fat_free_clusters(struct inode *inode, int cluster)
{
	struct super_block *sb = inode->i_sb;
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct fatent_operations *ops = sbi->fatent_ops;
	struct fat_entry2 fatent;
	int err=0, nr_bhs;


	nr_bhs = 0;
	fatent_init(&fatent);
	lock_fat(sbi);
	do {
		cluster = fat_ent_read(inode, &fatent, cluster);
		if (cluster < 0) {
			err = cluster;
			goto error;
		} else if (cluster == FAT_ENT_FREE) {
			fat_fs_error(sb, "%s: deleting FAT entry beyond EOF",
				     __func__);
			err = -EIO;
			goto error;
		}

		if (sbi->options.discard) {
			/*
			 * Issue discard for the sectors we no longer
			 * care about, batching contiguous clusters
			 * into one request
			 */
#if 0			 
			if (cluster != fatent.entry + 1) {
				int nr_clus = fatent.entry - first_cl + 1;

				sb_issue_discard(sb,
					fat_clus_to_blknr(sbi, first_cl),
					nr_clus * sbi->sec_per_clus,
					GFP_NOFS, 0);

				first_cl = cluster;
			}
#endif
		}

		ops->ent_put(&fatent,sb ,FAT_ENT_FREE);
		if (sbi->free_clusters != -1) {
			sbi->free_clusters++;
			sb->s_dirt = 1;
		}
#if 0
		if (nr_bhs + fatent.nr_bhs > MAX_BUF_PER_PAGE) {
			if (sb->s_flags & MS_SYNCHRONOUS) {
				err = fat_sync_bhs(bhs, nr_bhs);
				if (err)
					goto error;
			}
			err = fat_mirror_bhs(sb, bhs, nr_bhs);
			if (err)
				goto error;
			for (i = 0; i < nr_bhs; i++)
				brelse(bhs[i]);
			nr_bhs = 0;
		}
#endif        
	} while (cluster != FAT_ENT_EOF);

	if (sb->s_flags & MS_SYNCHRONOUS) {
		if (err)
			goto error;
	}
error:
	fatent_brelse2(&fatent);
	unlock_fat(sbi);

	return err;
}

EXPORT_SYMBOL_GPL(fat_free_clusters);

/* 128kb is the whole sectors for FAT12 and FAT16 */
#define FAT_READA_SIZE		(128 * 1024)


#if 0
static void fat_ent_reada(struct super_block *sb, struct fat_entry *fatent,
			  unsigned long reada_blocks)
{
	struct fatent_operations *ops = MSDOS_SB(sb)->fatent_ops;
	sector_t blocknr;
	int i, offset;

	ops->ent_blocknr(sb, fatent->entry, &offset, &blocknr);

	for (i = 0; i < reada_blocks; i++)
		sb_breadahead(sb, blocknr + i);
}
static void fat_ent_reada2(struct super_block *sb, struct fat_entry2 *fatent,
			  unsigned long reada_blocks)
{
	struct fatent_operations *ops = MSDOS_SB(sb)->fatent_ops;
	sector_t blocknr;
	int i, offset;

	ops->ent_blocknr(sb, fatent->entry, &offset, &blocknr);

	for (i = 0; i < reada_blocks; i++)
		sb_breadahead(sb, blocknr + i);
}

#endif


int fat_count_free_clusters(struct super_block *sb)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct fatent_operations *ops = sbi->fatent_ops;
	struct fat_entry2 fatent;
	unsigned long reada_blocks, reada_mask, cur_block;
	int err = 0, free,overflow = 0;

	lock_fat(sbi);
	if (sbi->free_clusters != -1 && sbi->free_clus_valid)
		goto out;

	reada_blocks = FAT_READA_SIZE >> sb->s_blocksize_bits;
	reada_mask = reada_blocks - 1;
	cur_block = 0;

	free = 0;
	fatent_init(&fatent);
	fatent_set_entry2(&fatent, FAT_START_ENT);
	while (fatent.entry < sbi->max_cluster) {
#if 0     
		/* readahead of fat blocks */
		if ((cur_block & reada_mask) == 0) {
			unsigned long rest = sbi->fat_length - cur_block;
			fat_ent_reada(sb, &fatent, min(reada_blocks, rest));
		}
		cur_block++;
#endif
		overflow = sbi->max_cluster - fatent.entry - 1;
		if(overflow < PAGE_CACHE_SIZE>>2)
			break;
		err = fat_ent_read_block(sb, &fatent,1);
		if (err)
			goto out;

		do {
			if (ops->ent_get(&fatent) == FAT_ENT_FREE)
				free++;
		} while (fat32_ent_next (&fatent));
	}
	//last page may overflow the fat table
	if(overflow > 0)
	{
		err = fat_ent_read_block(sb, &fatent, 0);
		if (err)
			goto out;

		do {
			if (ops->ent_get(&fatent) == FAT_ENT_FREE)
				free++;
            if(overflow -- <=0)
				break;

		} while (fat32_ent_next(&fatent));
	}
	sbi->free_clusters = free;
	sbi->free_clus_valid = 1;
	sb->s_dirt = 1;
	
out:
    fatent_brelse2(&fatent);
	unlock_fat(sbi);
	return err;
}
