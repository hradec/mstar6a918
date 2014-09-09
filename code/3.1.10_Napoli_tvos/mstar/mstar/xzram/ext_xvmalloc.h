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

#ifndef _EXT_XV_MALLOC_H_
#define _EXT_XV_MALLOC_H_

#include <linux/types.h>

#define EXT_XVPOOL_INTERNAL_NO_LIMIT 0
#define EXT_XVPOOL_EXTERNAL          1
#define EXT_XVPOOL_INTERNAL          2

#define XZRAM_PAGE_ATTR (GFP_NOIO | __GFP_HIGHMEM)

struct ext_xv_pool_attr{
     u8 pool_id;
     u8 type;
     unsigned int ext_pool_size;
     unsigned int ext_pool_start;
};

struct ext_xv_backup_page{ 
     void __iomem *page;
     struct ext_xv_backup_page *next_backup_page;
};

struct ext_xv_backuplist{
     struct ext_xv_backup_page *backup_page;
     unsigned int cnt;
};

struct migration{
    unsigned int on_migration;
    unsigned int read_cnt;
    unsigned int write_cnt;
};

struct ext_xv_pool;

struct ext_xv_pool *ext_xv_create_pool(struct ext_xv_pool_attr *attr, u8 pool_id);
void ext_xv_destroy_pool(struct ext_xv_pool *pool);
void __iomem *ext_xv_get_ptr_atomic(void __iomem *page, u16 offset, u32 type, struct ext_xv_pool *pool);
void ext_xv_put_ptr_atomic(void *ptr, u32 type, struct ext_xv_pool *pool);
int ext_xv_malloc(struct ext_xv_pool *pool, u32 size, struct page **page,
			u32 *offset, gfp_t flags);
void ext_xv_free(struct ext_xv_pool *pool, struct page *page, u32 offset);

u32 ext_xv_get_object_size(void *obj);
u64 ext_xv_get_total_size_bytes(struct ext_xv_pool *pool);
bool ext_xv_is_ext_pool(struct ext_xv_pool *pool);
bool ext_xv_is_dyn_int_pool(struct ext_xv_pool *pool);
u64 ext_xv_show_total_pages(struct ext_xv_pool *pool);

void __iomem**  ext_xv_pool_migration(struct ext_xv_pool *pool, u32 type, void __iomem **tmp_array, unsigned int *tmp_xbitmap, struct ext_xv_backuplist *backuplist);
void __iomem** ext_xv_create_pool_array(struct ext_xv_pool *pool, u32 type, unsigned int *tmp_xbitmap);
void ext_xv_delete_pool_array(struct ext_xv_pool *pool, void __iomem **tmp_array);
unsigned int * ext_xv_create_bitmap(struct ext_xv_pool *pool);
void ext_xv_delete_bitmap(unsigned int *bitmap);
unsigned int ext_xv_add_page_to_backuplist(struct ext_xv_backuplist *backuplist, int cnt);
void ext_xv_set_migration(struct ext_xv_pool *pool);
void ext_xv_clear_migration(struct ext_xv_pool *pool);
void ext_xv_add_read_cnt(struct ext_xv_pool *pool, int cnt);
void ext_xv_sub_read_cnt(struct ext_xv_pool *pool, int cnt);
void ext_xv_add_write_cnt(struct ext_xv_pool *pool, int cnt);
void ext_xv_sub_write_cnt(struct ext_xv_pool *pool, int cnt);
void ext_xv_set_migration(struct ext_xv_pool *pool);
void ext_xv_clear_migration(struct ext_xv_pool *pool);
unsigned int ext_xv_get_migration(struct ext_xv_pool *pool);
unsigned int ext_xv_get_read_cnt(struct ext_xv_pool *pool);
unsigned int ext_xv_get_write_cnt(struct ext_xv_pool *pool);
void ext_xv_bitmap_duplicate(struct ext_xv_pool *pool, unsigned int *bitmap);
void ext_xv_show_block_info(struct ext_xv_pool *pool);

#endif
