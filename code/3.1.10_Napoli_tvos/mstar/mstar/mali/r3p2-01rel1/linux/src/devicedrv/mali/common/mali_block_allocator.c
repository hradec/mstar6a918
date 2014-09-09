/*
 * Copyright (C) 2010-2013 ARM Limited. All rights reserved.
 * 
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 * 
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "mali_kernel_common.h"
#include "mali_kernel_memory_engine.h"
#include "mali_block_allocator.h"
#include "mali_osk.h"
#ifdef BUDDY_ALLOCATOR
#include "mali_osk_list.h"
#endif

#define MALI_BLOCK_SIZE (256UL * 1024UL)  /* 256 kB, remember to keep the ()s */

#ifdef BUDDY_ALLOCATOR
/* Enum uses to store multiple fields in one u32 to keep the memory block struct small */
enum MISC_SHIFT { MISC_SHIFT_FREE = 0, MISC_SHIFT_ORDER = 1, MISC_SHIFT_TOPLEVEL = 6 };
enum MISC_MASK { MISC_MASK_FREE = 0x01, MISC_MASK_ORDER = 0x1F, MISC_MASK_TOPLEVEL = 0x1F };
#endif

#ifdef BUDDY_ALLOCATOR
struct block_allocator;
#endif

typedef struct block_info
{
#ifdef BUDDY_ALLOCATOR
	_mali_osk_list_t link; /* used for freelist and process usage list*/
	struct block_allocator * bank; /* the bank it belongs to */
	u32 misc; /* used while a block is free to track the number blocks it represents */
	int descriptor;
#else
	struct block_info * next;
#endif
} block_info;

/* The structure used as the handle produced by block_allocator_allocate,
 * and removed by block_allocator_release */
typedef struct block_allocator_allocation
{
	/* The list will be released in reverse order */
	block_info *last_allocated;
	mali_allocation_engine * engine;
	mali_memory_allocation * descriptor;
	u32 start_offset;
	u32 mapping_length;
} block_allocator_allocation;


typedef struct block_allocator
{
    _mali_osk_lock_t *mutex;
	block_info * all_blocks;
#ifndef BUDDY_ALLOCATOR
	block_info * first_free;
#endif
	u32 base;
	u32 cpu_usage_adjust;
#ifndef BUDDY_ALLOCATOR
	u32 num_blocks;
#endif

#ifdef BUDDY_ALLOCATOR
	u32 size; /* the effective size */
	u32 real_size; /* the real size of the bank, as given by to the subsystem */
	int min_order;
	int max_order;
	_mali_osk_list_t *freelist;
#endif
} block_allocator;

#ifdef BUDDY_ALLOCATOR
MALI_STATIC_INLINE u32 order_needed_for_size(u32 size, block_allocator * bank);
MALI_STATIC_INLINE u32 maximum_order_which_fits(u32 size);
static block_info * mali_memory_block_get(block_allocator* bank, u32 minimum_size);
MALI_STATIC_INLINE u32 block_mali_addr_get(block_info * block);
MALI_STATIC_INLINE u32 get_block_free(block_info * block);
MALI_STATIC_INLINE void set_block_free(block_info * block, int state);
MALI_STATIC_INLINE void set_block_order(block_info * block, u32 order);
MALI_STATIC_INLINE u32 get_block_order(block_info * block);
MALI_STATIC_INLINE void set_block_toplevel(block_info * block, u32 level);
MALI_STATIC_INLINE u32 get_block_toplevel(block_info * block);
MALI_STATIC_INLINE int block_is_valid_buddy(block_info * block, int order);
MALI_STATIC_INLINE block_info * block_get_buddy(block_info * block, u32 order);
MALI_STATIC_INLINE block_info * block_get_parent(block_info * block, u32 order);
static void block_release(block_info * block);
#endif

MALI_STATIC_INLINE u32 get_phys(block_allocator * info, block_info * block);
static mali_physical_memory_allocation_result block_allocator_allocate(void* ctx, mali_allocation_engine * engine,  mali_memory_allocation * descriptor, u32* offset, mali_physical_memory_allocation * alloc_info);
static void block_allocator_release(void * ctx, void * handle);
static mali_physical_memory_allocation_result block_allocator_allocate_page_table_block(void * ctx, mali_page_table_block * block);
static void block_allocator_release_page_table_block( mali_page_table_block *page_table_block );
static void block_allocator_destroy(mali_physical_memory_allocator * allocator);
static u32 block_allocator_stat(mali_physical_memory_allocator * allocator);

mali_physical_memory_allocator * mali_block_allocator_create(u32 base_address, u32 cpu_usage_adjust, u32 size, const char *name)
{
	mali_physical_memory_allocator * allocator;
	block_allocator * info;
	u32 usable_size;
#ifdef BUDDY_ALLOCATOR
	u32 left, offset;
#else
	u32 num_blocks;
#endif

	usable_size = size & ~(MALI_BLOCK_SIZE - 1);
	MALI_DEBUG_PRINT(3, ("Mali block allocator create for region starting at 0x%08X length 0x%08X\n", base_address, size));
	MALI_DEBUG_PRINT(4, ("%d usable bytes\n", usable_size));
#ifndef BUDDY_ALLOCATOR
	num_blocks = usable_size / MALI_BLOCK_SIZE;
	MALI_DEBUG_PRINT(4, ("which becomes %d blocks\n", num_blocks));
#endif

	if (usable_size == 0)
	{
		MALI_DEBUG_PRINT(1, ("Memory block of size %d is unusable\n", size));
		return NULL;
	}

#ifdef BUDDY_ALLOCATOR
	/* warn for banks not a muliple of the block size  */
	MALI_DEBUG_PRINT_IF(1, usable_size != size, ("Memory bank @ 0x%X not a multiple of minimum block size. %d bytes wasted\n", base_address, size - usable_size));
#endif

	allocator = _mali_osk_malloc(sizeof(mali_physical_memory_allocator));
	if (NULL != allocator)
	{
		info = _mali_osk_malloc(sizeof(block_allocator));
		if (NULL != info)
		{
#ifdef BUDDY_ALLOCATOR
			info->mutex = _mali_osk_lock_init((_mali_osk_lock_flags_t)(_MALI_OSK_LOCKFLAG_SPINLOCK | _MALI_OSK_LOCKFLAG_NONINTERRUPTABLE), 0, 0);
#else
            info->mutex = _mali_osk_lock_init( _MALI_OSK_LOCKFLAG_ORDERED, 0, _MALI_OSK_LOCK_ORDER_MEM_INFO);
#endif
            if (NULL != info->mutex)
            {
#ifdef BUDDY_ALLOCATOR
				info->all_blocks = _mali_osk_calloc(1, sizeof(block_info) * (usable_size / MALI_BLOCK_SIZE));
#else
        		info->all_blocks = _mali_osk_malloc(sizeof(block_info) * num_blocks);
#endif
			    if (NULL != info->all_blocks)
			    {
				    u32 i;
#ifndef BUDDY_ALLOCATOR
				    info->first_free = NULL;
				    info->num_blocks = num_blocks;
#endif

				    info->base = base_address;
				    info->cpu_usage_adjust = cpu_usage_adjust;
#ifdef BUDDY_ALLOCATOR
					info->size = usable_size;
					info->real_size = size;
					info->min_order = order_needed_for_size(MALI_BLOCK_SIZE, NULL);
					info->max_order = maximum_order_which_fits(usable_size);
#endif

#ifdef BUDDY_ALLOCATOR
					for (i = 0; i < (usable_size / MALI_BLOCK_SIZE); i++)
					{
						info->all_blocks[i].bank = info;
					}

					info->freelist = _mali_osk_calloc(1, sizeof(_mali_osk_list_t) * (info->max_order - info->min_order + 1));
					if (NULL != info->freelist)
					{
						for (i = 0; i < (info->max_order - info->min_order + 1); i++)
						{
							_MALI_OSK_INIT_LIST_HEAD(&info->freelist[i]);
						}

						/* init slot info */
						for (offset = 0, left = usable_size; offset < (usable_size / MALI_BLOCK_SIZE); /* updated inside the body */)
						{
							u32 block_order;
							block_info * block;

							/* the maximum order which fits in the remaining area */
							block_order = maximum_order_which_fits(left);

							/* find the block pointer */
							block = &info->all_blocks[offset];

							/* tag the block as being toplevel */
							set_block_toplevel(block, block_order);

							/* tag it as being free */
							set_block_free(block, 1);

							/* set the order */
							set_block_order(block, block_order);

							_mali_osk_list_addtail(&block->link, info->freelist + (block_order - info->min_order));

							left -= (1 << block_order);
							offset += ((1 << block_order) / MALI_BLOCK_SIZE);
						}
#else
				    for ( i = 0; i < num_blocks; i++)
				    {
					    info->all_blocks[i].next = info->first_free;
					    info->first_free = &info->all_blocks[i];
				    }
#endif

				    allocator->allocate = block_allocator_allocate;
				    allocator->allocate_page_table_block = block_allocator_allocate_page_table_block;
				    allocator->destroy = block_allocator_destroy;
				    allocator->stat = block_allocator_stat;
				    allocator->ctx = info;
					allocator->name = name;

				    return allocator;
#ifdef BUDDY_ALLOCATOR
					}
					_mali_osk_free(info->all_blocks);
#endif
			    }
                _mali_osk_lock_term(info->mutex);
            }
			_mali_osk_free(info);
		}
		_mali_osk_free(allocator);
	}

	return NULL;
}

static void block_allocator_destroy(mali_physical_memory_allocator * allocator)
{
	block_allocator * info;
	MALI_DEBUG_ASSERT_POINTER(allocator);
	MALI_DEBUG_ASSERT_POINTER(allocator->ctx);
	info = (block_allocator*)allocator->ctx;

#ifdef BUDDY_ALLOCATOR
	_mali_osk_free(info->freelist);
#endif
	_mali_osk_free(info->all_blocks);
    _mali_osk_lock_term(info->mutex);
	_mali_osk_free(info);
	_mali_osk_free(allocator);
}

MALI_STATIC_INLINE u32 get_phys(block_allocator * info, block_info * block)
{
	return info->base + ((block - info->all_blocks) * MALI_BLOCK_SIZE);
}

static mali_physical_memory_allocation_result block_allocator_allocate(void* ctx, mali_allocation_engine * engine, mali_memory_allocation * descriptor, u32* offset, mali_physical_memory_allocation * alloc_info)
{
	block_allocator * info;
#ifdef BUDDY_ALLOCATOR
	u32 minimum_size;
	u32 padding;
	u32 phys_addr;
#else
	u32 left;
#endif
	block_info * last_allocated = NULL;
	mali_physical_memory_allocation_result result = MALI_MEM_ALLOC_NONE;
	block_allocator_allocation *ret_allocation;

	MALI_DEBUG_ASSERT_POINTER(ctx);
	MALI_DEBUG_ASSERT_POINTER(descriptor);
	MALI_DEBUG_ASSERT_POINTER(offset);
	MALI_DEBUG_ASSERT_POINTER(alloc_info);

#ifndef BUDDY_ALLOCATOR
	info = (block_allocator*)ctx;
	left = descriptor->size - *offset;
	MALI_DEBUG_ASSERT(0 != left);

	if (_MALI_OSK_ERR_OK != _mali_osk_lock_wait(info->mutex, _MALI_OSK_LOCKMODE_RW)) return MALI_MEM_ALLOC_INTERNAL_FAILURE;
#endif

	ret_allocation = _mali_osk_malloc( sizeof(block_allocator_allocation) );

	if ( NULL == ret_allocation )
	{
		/* Failure; try another allocator by returning MALI_MEM_ALLOC_NONE */
#ifndef BUDDY_ALLOCATOR
		_mali_osk_lock_signal(info->mutex, _MALI_OSK_LOCKMODE_RW);
#endif
		return result;
	}

#ifdef BUDDY_ALLOCATOR
	info = (block_allocator*)ctx;
	minimum_size = descriptor->size;

	/* at least min block size */
	if (MALI_BLOCK_SIZE > minimum_size) minimum_size = MALI_BLOCK_SIZE;

	/* perform the actual allocation */
	last_allocated = mali_memory_block_get(info, minimum_size);
	if ( NULL == last_allocated )
	{
		/* Failure; try another allocator by returning MALI_MEM_ALLOC_NONE */
		return result;
	}

	phys_addr = block_mali_addr_get(last_allocated);
	ret_allocation->start_offset = *offset;
	padding = *offset & (MALI_BLOCK_SIZE-1);
	ret_allocation->mapping_length = descriptor->size - padding;

	if (_MALI_OSK_ERR_OK != mali_allocation_engine_map_physical(engine, descriptor, *offset, phys_addr + padding, info->cpu_usage_adjust, ret_allocation->mapping_length))
	{
		MALI_DEBUG_PRINT(1, ("Mapping of physical memory  failed\n"));
		result = MALI_MEM_ALLOC_INTERNAL_FAILURE;
		mali_allocation_engine_unmap_physical(engine, descriptor, ret_allocation->start_offset, ret_allocation->mapping_length, (_mali_osk_mem_mapregion_flags_t)0);
		block_release(last_allocated);
		_mali_osk_free(ret_allocation);

		return result;
	}

	result = MALI_MEM_ALLOC_FINISHED;

	*offset += ret_allocation->mapping_length;
#else
	ret_allocation->start_offset = *offset;
	ret_allocation->mapping_length = 0;

	while ((left > 0) && (info->first_free))
	{
		block_info * block;
		u32 phys_addr;
		u32 padding;
		u32 current_mapping_size;

		block = info->first_free;
		info->first_free = info->first_free->next;
		block->next = last_allocated;
		last_allocated = block;

		phys_addr = get_phys(info, block);

		padding = *offset & (MALI_BLOCK_SIZE-1);

 		if (MALI_BLOCK_SIZE - padding < left)
		{
			current_mapping_size = MALI_BLOCK_SIZE - padding;
		}
		else
		{
			current_mapping_size = left;
		}

		if (_MALI_OSK_ERR_OK != mali_allocation_engine_map_physical(engine, descriptor, *offset, phys_addr + padding, info->cpu_usage_adjust, current_mapping_size))
		{
			MALI_DEBUG_PRINT(1, ("Mapping of physical memory  failed\n"));
			result = MALI_MEM_ALLOC_INTERNAL_FAILURE;
			mali_allocation_engine_unmap_physical(engine, descriptor, ret_allocation->start_offset, ret_allocation->mapping_length, (_mali_osk_mem_mapregion_flags_t)0);

			/* release all memory back to the pool */
			while (last_allocated)
			{
				/* This relinks every block we've just allocated back into the free-list */
				block = last_allocated->next;
				last_allocated->next = info->first_free;
				info->first_free = last_allocated;
				last_allocated = block;
			}

			break;
		}

		*offset += current_mapping_size;
		left -= current_mapping_size;
		ret_allocation->mapping_length += current_mapping_size;
	}

	_mali_osk_lock_signal(info->mutex, _MALI_OSK_LOCKMODE_RW);
#endif

#ifndef BUDDY_ALLOCATOR
	if (last_allocated)
	{
		if (left) result = MALI_MEM_ALLOC_PARTIAL;
		else result = MALI_MEM_ALLOC_FINISHED;
#endif

		/* Record all the information about this allocation */
		ret_allocation->last_allocated = last_allocated;
		ret_allocation->engine = engine;
		ret_allocation->descriptor = descriptor;

		alloc_info->ctx = info;
		alloc_info->handle = ret_allocation;
		alloc_info->release = block_allocator_release;
#ifndef BUDDY_ALLOCATOR
	}
	else
	{
		/* Free the allocation information - nothing to be passed back */
		_mali_osk_free( ret_allocation );
	}
#endif

	return result;
}

static void block_allocator_release(void * ctx, void * handle)
{
	block_allocator * info;
	block_info * block, * next;
	block_allocator_allocation *allocation;

	MALI_DEBUG_ASSERT_POINTER(ctx);
	MALI_DEBUG_ASSERT_POINTER(handle);

	info = (block_allocator*)ctx;
	allocation = (block_allocator_allocation*)handle;
	block = allocation->last_allocated;

	MALI_DEBUG_ASSERT_POINTER(block);

#ifndef BUDDY_ALLOCATOR
	if (_MALI_OSK_ERR_OK != _mali_osk_lock_wait(info->mutex, _MALI_OSK_LOCKMODE_RW))
	{
		MALI_DEBUG_PRINT(1, ("allocator release: Failed to get mutex\n"));
		return;
	}
#endif

	/* unmap */
	mali_allocation_engine_unmap_physical(allocation->engine, allocation->descriptor, allocation->start_offset, allocation->mapping_length, (_mali_osk_mem_mapregion_flags_t)0);

#ifdef BUDDY_ALLOCATOR
	block_release(block);
#else
	while (block)
	{
		MALI_DEBUG_ASSERT(!((block < info->all_blocks) || (block > (info->all_blocks + info->num_blocks))));

		next = block->next;

		/* relink into free-list */
		block->next = info->first_free;
		info->first_free = block;

		/* advance the loop */
		block = next;
	}

	_mali_osk_lock_signal(info->mutex, _MALI_OSK_LOCKMODE_RW);
#endif

	_mali_osk_free( allocation );
}


static mali_physical_memory_allocation_result block_allocator_allocate_page_table_block(void * ctx, mali_page_table_block * block)
{
	block_allocator * info;
#ifdef BUDDY_ALLOCATOR
	block_info * alloc;
#endif
	mali_physical_memory_allocation_result result = MALI_MEM_ALLOC_INTERNAL_FAILURE;

	MALI_DEBUG_ASSERT_POINTER(ctx);
	MALI_DEBUG_ASSERT_POINTER(block);
	info = (block_allocator*)ctx;

#ifdef BUDDY_ALLOCATOR
	alloc = mali_memory_block_get(info, MALI_BLOCK_SIZE);
#else
	if (_MALI_OSK_ERR_OK != _mali_osk_lock_wait(info->mutex, _MALI_OSK_LOCKMODE_RW)) return MALI_MEM_ALLOC_INTERNAL_FAILURE;
#endif

#ifdef BUDDY_ALLOCATOR
	if (NULL != alloc)
#else
	if (NULL != info->first_free)
#endif
	{
		void * virt;
		u32 phys;
		u32 size;
#ifndef BUDDY_ALLOCATOR
		block_info * alloc;
		alloc = info->first_free;
#endif

#ifdef BUDDY_ALLOCATOR
		phys = block_mali_addr_get(alloc);
#else
		phys = get_phys(info, alloc); /* Does not modify info or alloc */
#endif
		size = MALI_BLOCK_SIZE; /* Must be multiple of MALI_MMU_PAGE_SIZE */
#ifdef MSTAR
		virt = _mali_osk_mem_mapioregion( phys + info->cpu_usage_adjust, size, "Mali block allocator page tables" );
#else
		virt = _mali_osk_mem_mapioregion( phys, size, "Mali block allocator page tables" );
#endif

		/* Failure of _mali_osk_mem_mapioregion will result in MALI_MEM_ALLOC_INTERNAL_FAILURE,
		 * because it's unlikely another allocator will be able to map in. */

		if ( NULL != virt )
		{
			block->ctx = info; /* same as incoming ctx */
			block->handle = alloc;
			block->phys_base = phys;
			block->size = size;
			block->release = block_allocator_release_page_table_block;
			block->mapping = virt;

#ifndef BUDDY_ALLOCATOR
			info->first_free = alloc->next;

			alloc->next = NULL; /* Could potentially link many blocks together instead */
#endif
			_mali_osk_memset(block->mapping, 0, size);

			result = MALI_MEM_ALLOC_FINISHED;
		}
	}
	else result = MALI_MEM_ALLOC_NONE;

#ifndef BUDDY_ALLOCATOR
	_mali_osk_lock_signal(info->mutex, _MALI_OSK_LOCKMODE_RW);
#endif

	return result;
}


static void block_allocator_release_page_table_block( mali_page_table_block *page_table_block )
{
	block_allocator * info;
	block_info * block, * next;

	MALI_DEBUG_ASSERT_POINTER( page_table_block );

	info = (block_allocator*)page_table_block->ctx;
	block = (block_info*)page_table_block->handle;

	MALI_DEBUG_ASSERT_POINTER(info);
	MALI_DEBUG_ASSERT_POINTER(block);


#ifndef BUDDY_ALLOCATOR
	if (_MALI_OSK_ERR_OK != _mali_osk_lock_wait(info->mutex, _MALI_OSK_LOCKMODE_RW))
	{
		MALI_DEBUG_PRINT(1, ("allocator release: Failed to get mutex\n"));
		return;
	}
#endif

	/* Unmap all the physical memory at once */
#ifdef MSTAR
	_mali_osk_mem_unmapioregion( page_table_block->phys_base + info->cpu_usage_adjust, page_table_block->size, page_table_block->mapping );
#else
	_mali_osk_mem_unmapioregion( page_table_block->phys_base, page_table_block->size, page_table_block->mapping );
#endif

#ifdef BUDDY_ALLOCATOR
	block_release(block);
#else
	/** @note This loop handles the case where more than one block_info was linked.
	 * Probably unnecessary for page table block releasing. */
	while (block)
	{
		next = block->next;

		MALI_DEBUG_ASSERT(!((block < info->all_blocks) || (block > (info->all_blocks + info->num_blocks))));

		block->next = info->first_free;
		info->first_free = block;

		block = next;
	}

	_mali_osk_lock_signal(info->mutex, _MALI_OSK_LOCKMODE_RW);
#endif
}

static u32 block_allocator_stat(mali_physical_memory_allocator * allocator)
{
	block_allocator * info;
	block_info *block;
#ifdef BUDDY_ALLOCATOR
	block_info *temp;
	u32 size = 0;
	int i;
#else
	u32 free_blocks = 0;
#endif

	MALI_DEBUG_ASSERT_POINTER(allocator);
#ifdef BUDDY_ALLOCATOR
	MALI_DEBUG_ASSERT_POINTER(allocator->ctx);
#endif

	info = (block_allocator*)allocator->ctx;
#ifdef BUDDY_ALLOCATOR
	_mali_osk_lock_wait(info->mutex, _MALI_OSK_LOCKMODE_RW);

	for (i = 0; i < (info->max_order - info->min_order + 1); i++)
	{
		u32 block_size = 1 << (info->min_order+i);
		_MALI_OSK_LIST_FOREACHENTRY(block, temp, &info->freelist[i], block_info, link)
		{
			size += block_size;
		}
	}

	_mali_osk_lock_signal(info->mutex, _MALI_OSK_LOCKMODE_RW);

	return size;
#else
	block = info->first_free;

	while(block)
	{
		free_blocks++;
		block = block->next;
	}
	return (info->num_blocks - free_blocks) * MALI_BLOCK_SIZE;
#endif
}

#ifdef BUDDY_ALLOCATOR
MALI_STATIC_INLINE u32 order_needed_for_size(u32 size, block_allocator * bank)
{
	u32 order = 0;

	if (0 < size)
	{
		for ( order = sizeof(u32)*8 - 1; ((1UL<<order) & size) == 0; --order)
			/* nothing */;

		/* check if size is pow2, if not we need increment order by one */
		if (0 != (size & ((1UL<<order)-1))) ++order;
	}

	if ((NULL != bank) && (order < bank->min_order)) order = bank->min_order;
	/* Not capped to max order, that doesn't make sense */

	return order;
}

MALI_STATIC_INLINE u32 maximum_order_which_fits(u32 size)
{
	u32 order = 0;
	u32 powsize = 1;
	while (powsize < size)
	{
		powsize <<= 1;
		if (powsize > size) break;
		order++;
	}

	return order;
}

/**
 * Get a block of mali memory of at least the given size and of the given type
 * This is the backend for get_big_block.
 * @param type_id The type id of memory requested.
 * @param minimum_size The size requested
 * @return Pointer to a block on success, NULL on failure
 */
static block_info * mali_memory_block_get(block_allocator* bank, u32 minimum_size)
{
	block_info * block = NULL;
	u32 requested_order, current_order;

	/* input validation */
	if (0 == minimum_size)
	{
		/* bad size */
		MALI_DEBUG_PRINT(2, ("Zero size block requested by mali_memory_block_get\n"));
		return NULL;
}

	requested_order = order_needed_for_size(minimum_size, bank);

	MALI_DEBUG_PRINT(4, ("For size %d we need order %d (%d)\n", minimum_size, requested_order, 1 << requested_order));

	_mali_osk_lock_wait(bank->mutex, _MALI_OSK_LOCKMODE_RW);
	/* ! critical section begin */

	MALI_DEBUG_PRINT(7, ("Bank 0x%x locked\n", bank));

	for (current_order = requested_order; current_order <= bank->max_order; ++current_order)
	{
		_mali_osk_list_t * list = bank->freelist + (current_order - bank->min_order);
		MALI_DEBUG_PRINT(7, ("Checking freelist 0x%x for order %d\n", list, current_order));
		if (0 != _mali_osk_list_empty(list)) continue; /* empty list */

		MALI_DEBUG_PRINT(7, ("Found an entry on the freelist for order %d\n", current_order));


		block = _MALI_OSK_LIST_ENTRY(list->next, block_info, link);
		_mali_osk_list_delinit(&block->link);

		while (current_order > requested_order)
		{
			block_info * buddy_block;
			MALI_DEBUG_PRINT(7, ("Splitting block 0x%x\n", block));
			current_order--;
			list--;
			buddy_block = block_get_buddy(block, current_order - bank->min_order);
			set_block_order(buddy_block, current_order);
			set_block_free(buddy_block, 1);
			_mali_osk_list_add(&buddy_block->link, list);
		}

		set_block_order(block, current_order);
		set_block_free(block, 0);

		break;
	}

	/* ! critical section end */
	_mali_osk_lock_signal(bank->mutex, _MALI_OSK_LOCKMODE_RW);

	MALI_DEBUG_PRINT(7, ("Lock released for bank 0x%x\n", bank));

	MALI_DEBUG_PRINT_IF(7, NULL != block, ("Block 0x%x allocated\n", block));

	return block;
}

/**
 * Get the mali seen address of the memory described by the block
 * @param block The memory block to return the address of
 * @return The mali seen address of the memory block
 */
MALI_STATIC_INLINE u32 block_get_offset(block_info * block)
{
	return block - block->bank->all_blocks;
}

MALI_STATIC_INLINE u32 block_mali_addr_get(block_info * block)
{
	if (NULL != block) return block->bank->base + MALI_BLOCK_SIZE * block_get_offset(block);
	else return 0;
}

/**
 * Get a memory block's free status
 * @param block The block to get the state of
 */
MALI_STATIC_INLINE u32 get_block_free(block_info * block)
{
	return (block->misc >> MISC_SHIFT_FREE) & MISC_MASK_FREE;
}

/**
 * Set a memory block's free status
 * @param block The block to set the state for
 * @param state The state to set
 */
MALI_STATIC_INLINE void set_block_free(block_info * block, int state)
{
	if (state) block->misc |= (MISC_MASK_FREE << MISC_SHIFT_FREE);
	else block->misc &= ~(MISC_MASK_FREE << MISC_SHIFT_FREE);
}

/**
 * Set a memory block's order
 * @param block The block to set the order for
 * @param order The order to set
 */
MALI_STATIC_INLINE void set_block_order(block_info * block, u32 order)
{
	block->misc &= ~(MISC_MASK_ORDER << MISC_SHIFT_ORDER);
	block->misc |= ((order & MISC_MASK_ORDER) << MISC_SHIFT_ORDER);
}

/**
 * Get a memory block's order
 * @param block The block to get the order for
 * @return The order this block exists on
 */
MALI_STATIC_INLINE u32 get_block_order(block_info * block)
{
	return (block->misc >> MISC_SHIFT_ORDER) & MISC_MASK_ORDER;
}

/**
 * Tag a block as being a toplevel block.
 * A toplevel block has no buddy and no parent
 * @param block The block to tag as being toplevel
 */
MALI_STATIC_INLINE void set_block_toplevel(block_info * block, u32 level)
{
	block->misc |= ((level & MISC_MASK_TOPLEVEL) << MISC_SHIFT_TOPLEVEL);
}

/**
 * Check if a block is a toplevel block
 * @param block The block to check
 * @return 1 if toplevel, 0 else
 */
MALI_STATIC_INLINE u32 get_block_toplevel(block_info * block)
{
	return (block->misc >> MISC_SHIFT_TOPLEVEL) & MISC_MASK_TOPLEVEL;
}

/**
 * Checks if the given block is a buddy at the given order and that it's free
 * @param block The block to check
 * @param order The order to check against
 * @return 0 if not valid, else 1
 */
MALI_STATIC_INLINE int block_is_valid_buddy(block_info * block, int order)
{
	if (get_block_free(block) && (get_block_order(block) == order)) return 1;
	else return 0;
}

/*
 The buddy system uses the following rules to quickly find a blocks buddy
 and parent (block representing this block at a higher order level):
 - Given a block with index i the blocks buddy is at index i ^ ( 1 << order)
 - Given a block with index i the blocks parent is at i & ~(1 << order)
*/

/**
 * Get a blocks buddy
 * @param block The block to find the buddy for
 * @param order The order to operate on
 * @return Pointer to the buddy block
 */
MALI_STATIC_INLINE block_info * block_get_buddy(block_info * block, u32 order)
{
	return block + ( (block_get_offset(block) ^ (1 << order)) - block_get_offset(block));
}

/**
 * Get a blocks parent
 * @param block The block to find the parent for
 * @param order The order to operate on
 * @return Pointer to the parent block
 */
MALI_STATIC_INLINE block_info * block_get_parent(block_info * block, u32 order)
{
	return block + ((block_get_offset(block) & ~(1 << order)) - block_get_offset(block));
}


/**
 * Release mali memory
 * Backend for free_big_block.
 * Will release the mali memory described by the given block struct.
 * @param block Memory block to free
 */
static void block_release(block_info * block)
{
	block_allocator * bank;
	u32 current_order;

	if (NULL == block) return;

	bank = block->bank;

	/* we're manipulating the free list, so we need to lock it */
	_mali_osk_lock_wait(bank->mutex, _MALI_OSK_LOCKMODE_RW);
	/* ! critical section begin */

	set_block_free(block, 1);
	current_order = get_block_order(block);

	while (current_order <= bank->max_order)
	{
		block_info * buddy_block;
		buddy_block = block_get_buddy(block, current_order - bank->min_order);
		if (!block_is_valid_buddy(buddy_block, current_order)) break;
		_mali_osk_list_delinit(&buddy_block->link); /* remove from free list */
		/* clear tracked data in both blocks */
		set_block_order(block, 0);
		set_block_free(block, 0);
		set_block_order(buddy_block, 0);
		set_block_free(buddy_block, 0);
		/* make the parent control the new state */
		block = block_get_parent(block, current_order - bank->min_order);
		set_block_order(block, current_order + 1); /* merged has a higher order */
		set_block_free(block, 1); /* mark it as free */
		current_order++;
		if (get_block_toplevel(block) == current_order) break; /* stop the merge if we've arrived at a toplevel block */
	}

	_mali_osk_list_add(&block->link, &bank->freelist[current_order - bank->min_order]);

	/* !critical section end */
	_mali_osk_lock_signal(bank->mutex, _MALI_OSK_LOCKMODE_RW);

	return;
}
#endif
