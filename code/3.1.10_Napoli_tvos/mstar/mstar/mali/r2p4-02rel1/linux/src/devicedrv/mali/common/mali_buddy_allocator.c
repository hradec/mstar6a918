/*
 * Copyright (C) 2010-2012 ARM Limited. All rights reserved.
 * 
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 * 
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "mali_kernel_common.h"
#include "mali_kernel_core.h"
#include "mali_kernel_memory_engine.h"
#include "mali_buddy_allocator.h"
#include "mali_osk.h"
#include "mali_osk_list.h"

/**
 * Minimum memory allocation size
 */
#define MIN_BLOCK_SIZE (256*1024UL)

/**
 * Enum uses to store multiple fields in one u32 to keep the memory block struct small
 */
enum MISC_SHIFT { MISC_SHIFT_FREE = 0, MISC_SHIFT_ORDER = 1, MISC_SHIFT_TOPLEVEL = 6 };
enum MISC_MASK { MISC_MASK_FREE = 0x01, MISC_MASK_ORDER = 0x1F, MISC_MASK_TOPLEVEL = 0x1F };

/* forward declaration of the block struct */
struct mali_memory_block;

/**
 * Definition of memory bank type.
 * Represents a memory bank (separate address space)
 * Each bank keeps track of its block usage.
 * A buddy system used to track the usage
*/
typedef struct mali_memory_bank
{
    _mali_osk_lock_t *lock;
    u32 base_addr; /* Mali seen address of bank */
    u32 cpu_usage_adjust; /* Adjustment factor for what the CPU sees */
    u32 size; /* the effective size */
    u32 real_size; /* the real size of the bank, as given by to the subsystem */
    int min_order;
    int max_order;
    struct mali_memory_block * blocklist;
    _mali_osk_list_t *freelist;
} mali_memory_bank;

/**
 * Definition of the memory block type
 * Represents a memory block, which is the smallest memory unit operated on.
 * A block keeps info about its mapping, if in use by a user process
 */
typedef struct mali_memory_block
{
    _mali_osk_list_t link; /* used for freelist and process usage list*/
    mali_memory_bank * bank; /* the bank it belongs to */
    u32 misc; /* used while a block is free to track the number blocks it represents */
    int descriptor;
} mali_memory_block;

/* The structure used as the handle produced by block_allocator_allocate,
 * and removed by block_allocator_release */
typedef struct buddy_allocator_allocation
{
    /* The list will be released in reverse order */
    mali_memory_block* block;
    mali_allocation_engine * engine;
    mali_memory_allocation * descriptor;
    u32 start_offset;
    u32 mapping_length;
} buddy_allocator_allocation;

static mali_physical_memory_allocation_result buddy_allocator_allocate(void* ctx, mali_allocation_engine * engine,  mali_memory_allocation * descriptor, u32* offset, mali_physical_memory_allocation * alloc_info);
static void buddy_allocator_release(void * ctx, void * handle);
static mali_physical_memory_allocation_result buddy_allocator_allocate_page_table_block(void * ctx, mali_page_table_block * block);
static void buddy_allocator_release_page_table_block( mali_page_table_block *page_table_block );
static void buddy_allocator_destroy(mali_physical_memory_allocator * allocator);
static u32 buddy_allocator_stat(mali_physical_memory_allocator * allocator);

/**
 * Get a block of mali memory of at least the given size and of the given type
 * This is the backend for get_big_block.
 * @param type_id The type id of memory requested.
 * @param minimum_size The size requested
 * @return Pointer to a block on success, NULL on failure
 */
static mali_memory_block * mali_memory_block_get(mali_memory_bank* bank, u32 minimum_size);

/**
 * Get the mali seen address of the memory described by the block
 * @param block The memory block to return the address of
 * @return The mali seen address of the memory block
 */
MALI_STATIC_INLINE u32 block_mali_addr_get(mali_memory_block * block);

/**
 * Get the cpu seen address of the memory described by the block
 * The cpu_usage_adjust will be used to change the mali seen phys address
 * @param block The memory block to return the address of
 * @return The mali seen address of the memory block
 */
MALI_STATIC_INLINE u32 block_cpu_addr_get(mali_memory_block * block);

/**
 * Get the size of the memory described by the given block
 * @param block The memory block to return the size of
 * @return The size of the memory block described by the object
 */
MALI_STATIC_INLINE u32 block_size_get(mali_memory_block * block);

/**
 * Get a memory block's free status
 * @param block The block to get the state of
 */
MALI_STATIC_INLINE u32 get_block_free(mali_memory_block * block);

/**
 * Set a memory block's free status
 * @param block The block to set the state for
 * @param state The state to set
 */
MALI_STATIC_INLINE void set_block_free(mali_memory_block * block, int state);

/**
 * Set a memory block's order
 * @param block The block to set the order for
 * @param order The order to set
 */
MALI_STATIC_INLINE void set_block_order(mali_memory_block * block, u32 order);

/**
 * Get a memory block's order
 * @param block The block to get the order for
 * @return The order this block exists on
 */
MALI_STATIC_INLINE u32 get_block_order(mali_memory_block * block);

/**
 * Tag a block as being a toplevel block.
 * A toplevel block has no buddy and no parent
 * @param block The block to tag as being toplevel
 */
MALI_STATIC_INLINE void set_block_toplevel(mali_memory_block * block, u32 level);

/**
 * Check if a block is a toplevel block
 * @param block The block to check
 * @return 1 if toplevel, 0 else
 */
MALI_STATIC_INLINE u32 get_block_toplevel(mali_memory_block * block);

/**
 * Checks if the given block is a buddy at the given order and that it's free
 * @param block The block to check
 * @param order The order to check against
 * @return 0 if not valid, else 1
 */
MALI_STATIC_INLINE int block_is_valid_buddy(mali_memory_block * block, int order);

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
MALI_STATIC_INLINE mali_memory_block * block_get_buddy(mali_memory_block * block, u32 order);

/**
 * Get a blocks parent
 * @param block The block to find the parent for
 * @param order The order to operate on
 * @return Pointer to the parent block
 */
MALI_STATIC_INLINE mali_memory_block * block_get_parent(mali_memory_block * block, u32 order);

/**
 * Release mali memory
 * Backend for free_big_block.
 * Will release the mali memory described by the given block struct.
 * @param block Memory block to free
 */
static void block_release(mali_memory_block * block);

/* end interface implementation */

MALI_STATIC_INLINE u32 order_needed_for_size(u32 size, struct mali_memory_bank * bank)
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

mali_physical_memory_allocator * mali_buddy_allocator_create(u32 base_address, u32 cpu_usage_adjust, u32 size, const char *name)
{
    mali_physical_memory_allocator * allocator;
    mali_memory_bank * bank;
    u32 usable_size;
    u32 left, offset;

    /* Only a multiple of MIN_BLOCK_SIZE is usable */
    usable_size = size & ~(MIN_BLOCK_SIZE - 1);

    /* handle zero sized banks and bank smaller than the fixed block size */
    if (usable_size == 0)
    {
        MALI_PRINT(("Usable size == 0\n"));
        return NULL;
    }

    /* warn for banks not a muliple of the block size  */
    MALI_DEBUG_PRINT_IF(1, usable_size != size, ("Memory bank @ 0x%X not a multiple of minimum block size. %d bytes wasted\n", base_address, size - usable_size));

    allocator = _mali_osk_malloc(sizeof(mali_physical_memory_allocator));
    if (NULL != allocator)
    {
        bank = _mali_osk_malloc(sizeof(mali_memory_bank));
        if (NULL != bank)
        {
            bank->base_addr = base_address;
            bank->cpu_usage_adjust = cpu_usage_adjust;
            bank->size = usable_size;
            bank->real_size = size;
            bank->min_order = order_needed_for_size(MIN_BLOCK_SIZE, NULL);
            bank->max_order = maximum_order_which_fits(usable_size);

            bank->lock = _mali_osk_lock_init(
                    (_mali_osk_lock_flags_t)(_MALI_OSK_LOCKFLAG_SPINLOCK | _MALI_OSK_LOCKFLAG_NONINTERRUPTABLE), 0, 0);
            if (NULL != bank->lock)
            {
                bank->blocklist = _mali_osk_calloc(1, sizeof(struct mali_memory_block) * (usable_size / MIN_BLOCK_SIZE));
                if (NULL != bank->blocklist)
                {
                    u32 i;

                    for (i = 0; i < (usable_size / MIN_BLOCK_SIZE); i++)
                    {
                        bank->blocklist[i].bank = bank;
                    }

                    bank->freelist = _mali_osk_calloc(1, sizeof(_mali_osk_list_t) * (bank->max_order - bank->min_order + 1));
                    if (NULL != bank->freelist)
                    {
                        for (i = 0; i < (bank->max_order - bank->min_order + 1); i++)
                        {
                            _MALI_OSK_INIT_LIST_HEAD(&bank->freelist[i]);
                        }

                        /* init slot info */
                        for (offset = 0, left = usable_size; 
                                offset < (usable_size / MIN_BLOCK_SIZE); /* updated inside the body */)
                        {
                            u32 block_order;
                            mali_memory_block * block;

                            /* the maximum order which fits in the remaining area */
                            block_order = maximum_order_which_fits(left);

                            /* find the block pointer */
                            block = &bank->blocklist[offset];

                            /* tag the block as being toplevel */
                            set_block_toplevel(block, block_order);

                            /* tag it as being free */
                            set_block_free(block, 1);

                            /* set the order */
                            set_block_order(block, block_order);

                            _mali_osk_list_addtail(&block->link, bank->freelist + (block_order - bank->min_order));

                            left -= (1 << block_order);
                            offset += ((1 << block_order) / MIN_BLOCK_SIZE);
                        }

                        allocator->allocate = buddy_allocator_allocate;
                        allocator->allocate_page_table_block = buddy_allocator_allocate_page_table_block;
                        allocator->destroy = buddy_allocator_destroy;
                        allocator->stat = buddy_allocator_stat;
                        allocator->ctx = bank;
                        allocator->name = name;

                        return allocator;
                    }
                    _mali_osk_free(bank->blocklist);
                }
                _mali_osk_lock_term(bank->lock);
            }
            _mali_osk_free(bank);
        }
        _mali_osk_free(allocator);
    }

    return NULL;
}

static void buddy_allocator_destroy(mali_physical_memory_allocator * allocator)
{
    mali_memory_bank * bank;
    MALI_DEBUG_ASSERT_POINTER(allocator);
    MALI_DEBUG_ASSERT_POINTER(allocator->ctx);

    bank = (mali_memory_bank*)allocator->ctx;

    _mali_osk_lock_term(bank->lock);

    /* remove all resources used to represent this bank*/
    _mali_osk_free(bank->freelist);
    _mali_osk_free(bank->blocklist);

    /* destroy the bank object itself */
    _mali_osk_free(bank);

    _mali_osk_free(allocator);
}

static u32 buddy_allocator_stat(mali_physical_memory_allocator * allocator)
{
    mali_memory_block * block, * temp;
    mali_memory_bank * bank;
    u32 size = 0;
    int i;

    MALI_DEBUG_ASSERT_POINTER(allocator);
    MALI_DEBUG_ASSERT_POINTER(allocator->ctx);

    bank = (mali_memory_bank*)allocator->ctx;

    _mali_osk_lock_wait(bank->lock, _MALI_OSK_LOCKMODE_RW);

    for (i = 0; i < (bank->max_order - bank->min_order + 1); i++)
    {
        u32 block_size = 1 << (bank->min_order+i);
        _MALI_OSK_LIST_FOREACHENTRY(block, temp, &bank->freelist[i], mali_memory_block, link)
        {
            size += block_size;
        }
    }

    _mali_osk_lock_signal(bank->lock, _MALI_OSK_LOCKMODE_RW);

    return size;
}

static mali_physical_memory_allocation_result buddy_allocator_allocate(void* ctx, mali_allocation_engine * engine, mali_memory_allocation * descriptor, u32* offset, mali_physical_memory_allocation * alloc_info)
{
    mali_memory_bank * bank;
    mali_physical_memory_allocation_result result = MALI_MEM_ALLOC_NONE;
    mali_memory_block * block;
    buddy_allocator_allocation *ret_allocation;
    u32 minimum_size;
    u32 padding;
    u32 phys_addr;

    MALI_DEBUG_ASSERT_POINTER(ctx);
    MALI_DEBUG_ASSERT_POINTER(descriptor);
    MALI_DEBUG_ASSERT_POINTER(offset);
    MALI_DEBUG_ASSERT_POINTER(alloc_info);

    ret_allocation = _mali_osk_malloc( sizeof(buddy_allocator_allocation) );

    if ( NULL == ret_allocation )
    {
        /* Failure; try another allocator by returning MALI_MEM_ALLOC_NONE */
        return result;
    }

    bank = (mali_memory_bank*)ctx;
    minimum_size = descriptor->size;

    /* at least min block size */
    if (MIN_BLOCK_SIZE > minimum_size) minimum_size = MIN_BLOCK_SIZE;

    /* perform the actual allocation */
    block = mali_memory_block_get(bank, minimum_size);
    if ( NULL == block )
    {
        /* Failure; try another allocator by returning MALI_MEM_ALLOC_NONE */
        return result;
    }

    phys_addr = block_mali_addr_get(block);
    ret_allocation->start_offset = *offset;
    padding = *offset & (MIN_BLOCK_SIZE-1);
    ret_allocation->mapping_length = descriptor->size - padding;

    if (_MALI_OSK_ERR_OK != mali_allocation_engine_map_physical(engine, descriptor, *offset, phys_addr + padding, bank->cpu_usage_adjust, ret_allocation->mapping_length))
    {
        MALI_DEBUG_PRINT(1, ("Mapping of physical memory  failed\n"));
        result = MALI_MEM_ALLOC_INTERNAL_FAILURE;
        mali_allocation_engine_unmap_physical(engine, descriptor, ret_allocation->start_offset, ret_allocation->mapping_length, (_mali_osk_mem_mapregion_flags_t)0);
        block_release(block);
        _mali_osk_free(ret_allocation);

        return result;
    }

    result = MALI_MEM_ALLOC_FINISHED;

    *offset += ret_allocation->mapping_length;;

    /* Record all the information about this allocation */
    ret_allocation->block = block;
    ret_allocation->engine = engine;
    ret_allocation->descriptor = descriptor;

    alloc_info->ctx = bank;
    alloc_info->handle = ret_allocation;
    alloc_info->release = buddy_allocator_release;

    return result;
}

static void buddy_allocator_release(void * ctx, void * handle)
{
    mali_memory_bank * bank;
    buddy_allocator_allocation *allocation;
    mali_memory_block * block;

    MALI_DEBUG_ASSERT_POINTER(ctx);
    MALI_DEBUG_ASSERT_POINTER(handle);

    bank = (mali_memory_bank*)ctx;
    allocation = (buddy_allocator_allocation*)handle;
    block = allocation->block;

    MALI_DEBUG_ASSERT_POINTER(block);

    /* unmap */
    mali_allocation_engine_unmap_physical(allocation->engine, allocation->descriptor, allocation->start_offset, allocation->mapping_length, (_mali_osk_mem_mapregion_flags_t)0);

    block_release(block);
    _mali_osk_free(allocation);
}

static mali_physical_memory_allocation_result buddy_allocator_allocate_page_table_block(void * ctx, mali_page_table_block * page_table_block)
{
    mali_memory_bank * bank;
    mali_physical_memory_allocation_result result = MALI_MEM_ALLOC_INTERNAL_FAILURE;
    mali_memory_block * block;

    MALI_DEBUG_ASSERT_POINTER(ctx);
    MALI_DEBUG_ASSERT_POINTER(block);
    bank = (mali_memory_bank*)ctx;

    block = mali_memory_block_get(bank, MIN_BLOCK_SIZE);
    if ( NULL != block )
    {
        void * virt;
        u32 phys;
        u32 size;

        phys = block_mali_addr_get(block);
        size = MIN_BLOCK_SIZE; /* Must be multiple of MALI_MMU_PAGE_SIZE */
#ifdef MSTAR
        virt = _mali_osk_mem_mapioregion( phys + bank->cpu_usage_adjust, size, "Mali block allocator page tables" );
#else
        virt = _mali_osk_mem_mapioregion( phys, size, "Mali block allocator page tables" );
#endif

        /* Failure of _mali_osk_mem_mapioregion will result in MALI_MEM_ALLOC_INTERNAL_FAILURE,
         * because it's unlikely another allocator will be able to map in. */

        if ( NULL != virt )
        {
            page_table_block->ctx = bank; /* same as incoming ctx */
            page_table_block->handle = block;
            page_table_block->phys_base = phys;
            page_table_block->size = size;
            page_table_block->release = buddy_allocator_release_page_table_block;
            page_table_block->mapping = virt;

            result = MALI_MEM_ALLOC_FINISHED;
        }
    }
    else result = MALI_MEM_ALLOC_NONE;

    return result;

}

static void buddy_allocator_release_page_table_block( mali_page_table_block *page_table_block )
{
    mali_memory_bank * bank;
    mali_memory_block * block;

    MALI_DEBUG_ASSERT_POINTER( page_table_block );

    bank = (mali_memory_bank*)page_table_block->ctx;
    block = (mali_memory_block*)page_table_block->handle;

    MALI_DEBUG_ASSERT_POINTER(bank);
    MALI_DEBUG_ASSERT_POINTER(block);

    /* Unmap all the physical memory at once */
#ifdef MSTAR
    _mali_osk_mem_unmapioregion( page_table_block->phys_base + bank->cpu_usage_adjust, page_table_block->size, page_table_block->mapping );
#else
    _mali_osk_mem_unmapioregion( page_table_block->phys_base, page_table_block->size, page_table_block->mapping );
#endif

    block_release(block);
}

static mali_memory_block * mali_memory_block_get(mali_memory_bank* bank, u32 minimum_size)
{
    mali_memory_block * block = NULL;
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

    _mali_osk_lock_wait(bank->lock, _MALI_OSK_LOCKMODE_RW);
    /* ! critical section begin */

    MALI_DEBUG_PRINT(7, ("Bank 0x%x locked\n", bank));

    for (current_order = requested_order; current_order <= bank->max_order; ++current_order)
    {
        _mali_osk_list_t * list = bank->freelist + (current_order - bank->min_order);
        MALI_DEBUG_PRINT(7, ("Checking freelist 0x%x for order %d\n", list, current_order));
        if (0 != _mali_osk_list_empty(list)) continue; /* empty list */

        MALI_DEBUG_PRINT(7, ("Found an entry on the freelist for order %d\n", current_order));


        block = _MALI_OSK_LIST_ENTRY(list->next, mali_memory_block, link);
        _mali_osk_list_delinit(&block->link);

        while (current_order > requested_order)
        {
            mali_memory_block * buddy_block;
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
    _mali_osk_lock_signal(bank->lock, _MALI_OSK_LOCKMODE_RW);

    MALI_DEBUG_PRINT(7, ("Lock released for bank 0x%x\n", bank));

    MALI_DEBUG_PRINT_IF(7, NULL != block, ("Block 0x%x allocated\n", block));

    return block;
}

static void block_release(mali_memory_block * block)
{
    mali_memory_bank * bank;
    u32 current_order;

    if (NULL == block) return;

    bank = block->bank;

    /* we're manipulating the free list, so we need to lock it */
    _mali_osk_lock_wait(bank->lock, _MALI_OSK_LOCKMODE_RW);
    /* ! critical section begin */

    set_block_free(block, 1);
    current_order = get_block_order(block);

    while (current_order <= bank->max_order)
    {
        mali_memory_block * buddy_block;
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
    _mali_osk_lock_signal(bank->lock, _MALI_OSK_LOCKMODE_RW);

    return;
}

MALI_STATIC_INLINE u32 block_get_offset(mali_memory_block * block)
{
    return block - block->bank->blocklist;
}

MALI_STATIC_INLINE u32 block_mali_addr_get(mali_memory_block * block)
{
    if (NULL != block) return block->bank->base_addr + MIN_BLOCK_SIZE * block_get_offset(block);
    else return 0;
}

MALI_STATIC_INLINE u32 block_cpu_addr_get(mali_memory_block * block)
{
    if (NULL != block) return (block->bank->base_addr + MIN_BLOCK_SIZE * block_get_offset(block)) + block->bank->cpu_usage_adjust;
    else return 0;
}

MALI_STATIC_INLINE u32 block_size_get(mali_memory_block * block)
{
    if (NULL != block) return 1 << get_block_order(block);
    else return 0;
}

MALI_STATIC_INLINE u32 get_block_free(mali_memory_block * block)
{
    return (block->misc >> MISC_SHIFT_FREE) & MISC_MASK_FREE;
}

MALI_STATIC_INLINE void set_block_free(mali_memory_block * block, int state)
{
    if (state) block->misc |= (MISC_MASK_FREE << MISC_SHIFT_FREE);
    else block->misc &= ~(MISC_MASK_FREE << MISC_SHIFT_FREE);
}

MALI_STATIC_INLINE void set_block_order(mali_memory_block * block, u32 order)
{
    block->misc &= ~(MISC_MASK_ORDER << MISC_SHIFT_ORDER);
    block->misc |= ((order & MISC_MASK_ORDER) << MISC_SHIFT_ORDER);
}

MALI_STATIC_INLINE u32 get_block_order(mali_memory_block * block)
{
    return (block->misc >> MISC_SHIFT_ORDER) & MISC_MASK_ORDER;
}

MALI_STATIC_INLINE void set_block_toplevel(mali_memory_block * block, u32 level)
{
    block->misc |= ((level & MISC_MASK_TOPLEVEL) << MISC_SHIFT_TOPLEVEL);
}

MALI_STATIC_INLINE u32 get_block_toplevel(mali_memory_block * block)
{
    return (block->misc >> MISC_SHIFT_TOPLEVEL) & MISC_MASK_TOPLEVEL;
}

MALI_STATIC_INLINE int block_is_valid_buddy(mali_memory_block * block, int order)
{
    if (get_block_free(block) && (get_block_order(block) == order)) return 1;
    else return 0;
}

MALI_STATIC_INLINE mali_memory_block * block_get_buddy(mali_memory_block * block, u32 order)
{
    return block + ( (block_get_offset(block) ^ (1 << order)) - block_get_offset(block));
}

MALI_STATIC_INLINE mali_memory_block * block_get_parent(mali_memory_block * block, u32 order)
{
    return block + ((block_get_offset(block) & ~(1 << order)) - block_get_offset(block));
}

