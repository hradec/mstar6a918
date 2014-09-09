////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    mdrv_mtlb.c
/// @brief  Memory Pool Control Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h> 
#include <linux/timer.h> 
#include <linux/device.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/types.h>
#include <asm/cacheflush.h>
#include <linux/dma-mapping.h>
#include <linux/spinlock.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0)
#include <linux/shrinker.h>
#endif


#if defined(CONFIG_MIPS)
#include <asm/mips-boards/prom.h>
#elif defined(CONFIG_ARM)
#endif

#include "mst_devid.h"
#include "mdrv_mtlb.h"
#include "mdrv_types.h"
#include "mst_platform.h"
#include "mdrv_system.h"
#include "mhal_mtlb.h"


#define USEMPOOL 1
//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define MTLB_DPRINTK(fmt, args...) printk(fmt,## args)
#define MTLB_DEBUG_ASSERT(condition) do  {if( !(condition)) {MTLB_DPRINTK("ASSERT failed: " #condition ); _mtlb_abort();} } while(0)


//#define DEBUG

#ifdef DEBUG
#define MTLB_DEBUG_ASSERT_POINTER(pointer) do  {if( (pointer)== NULL) {MTLB_DPRINTK("NULL pointer " #pointer); _mtlb_abort();} } while(0)
#define MTLB_DEBUG(fmt, args...) printk(KERN_WARNING"%s:%d " fmt,__FUNCTION__,__LINE__,## args)
#else /* DEBUG */
#define MTLB_DEBUG_ASSERT_POINTER(pointer) do {} while(0)
#define MTLB_DEBUG(fmt, args...) do {} while(0)

#endif
//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MTLB_VERSION 1
#define MOD_MTLB_DEVICE_COUNT     1
#ifdef USEMPOOL
#define MOD_MTLB_NAME             "mtlb"
#else
#define MOD_MTLB_NAME             "mpool"
#endif
#define KER_CACHEMODE_CACHE   1
#define KER_CACHEMODE_UNCACHE_NONBUFFERABLE 0
#define KER_CACHEMODE_UNCACHE_BUFFERABLE 2

#define INVALID_PAGE 0xffffffff
#define MAX_ALLOCTION_PAGE (0x3200000 >> PAGE_SHIFT)
//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------;
typedef enum
{
    _MTLB_ERR_OK = 0, /**< Success. */
    _MTLB_ERR_FAULT = -1, /**< General non-success */
    _MTLB_ERR_INVALID_FUNC = -2, /**< Invalid function requested through User/Kernel interface (e.g. bad IOCTL number) */
    _MTLB_ERR_INVALID_ARGS = -3, /**< Invalid arguments passed through User/Kernel interface */
} _mtlb_errcode_t;


typedef  struct
{
	struct vm_area_struct* cookie;
	u32 heap_start;
	u32 heap_end;
}MTLB_FileData;

typedef  struct
{
	int references;
	u32 cookie;
    u32 mTLBData;
}MTLB_VmaData;

struct AllocationList
{
	struct AllocationList *next;
	u32 offset;
	u32 physaddr;
};

typedef struct AllocationList AllocationList;

/* Private structure to store details of a mapping region returned
 */
struct MappingInfo
{
	struct AllocationList *list;
	struct AllocationList *tail;
};

typedef struct
{
    int                         s32MTLBMajor;
    int                         s32MTLBMinor;
    struct cdev                 cDevice;
    struct file_operations      MTLBFop;
} MTLBModHandle;

typedef struct
{
	_mtlb_errcode_t (*map_physical)(MTLB_FileData *mTLBData, u32 offset, u32 phys_addr, u32 size );
	void (*unmap_physical)(MTLB_FileData *mTLBData, u32 offset, u32 size);
} mem_address_manager;

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------  
static struct class *mtlb_class;
struct mutex mtlb_remap_mutex = __MUTEX_INITIALIZER(mtlb_remap_mutex);
extern void (*flush_cache_all)(void);
static struct MappingInfo mappingInfo;
//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
_mtlb_errcode_t 				_process_mem_mapregion_map(MTLB_FileData *mTLBData, u32 offset, u32 phys_addr, u32 size );
void 							_process_mem_mapregion_unmap(MTLB_FileData *mTLBData, u32 offset, u32 size);

void 							_mtlb_vma_open(struct vm_area_struct * vma);
void 							_mtlb_vma_close(struct vm_area_struct * vma);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
int 							_mtlb_cpu_page_fault_handler(struct vm_area_struct *vma, struct vm_fault *vmf);
#else
unsigned long 					_mtlb_cpu_page_fault_handler(struct vm_area_struct * vma, unsigned long address);
#endif

static int                      _MDrv_MTLB_Open (struct inode *inode, struct file *filp);
static int                      _MDrv_MTLB_Release(struct inode *inode, struct file *filp);
static int                      _MDrv_MTLB_MMap(struct file *filp, struct vm_area_struct *vma);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
static long                     _MDrv_MTLB_Ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#else
static int                      _MDrv_MTLB_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#endif
//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------
static MTLBModHandle MTLBDev=
{
#ifndef USEMPOOL
    .s32MTLBMajor=               MDRV_MAJOR_MTLB,
    .s32MTLBMinor=               MDRV_MINOR_MTLB,
    .cDevice=
    {
        .kobj=                  {.name= MOD_MTLB_NAME, },
        .owner  =               THIS_MODULE,
    },

#else
    .s32MTLBMajor=				 MDRV_MAJOR_MPOOL,
    .s32MTLBMinor=				 MDRV_MINOR_MPOOL,
    .cDevice=
    {
        .kobj=                  {.name= MOD_MTLB_NAME, },
        .owner  =               THIS_MODULE,
    },
#endif

    .MTLBFop=
    {
        .open=                  _MDrv_MTLB_Open,
        .release=               _MDrv_MTLB_Release,
        .mmap=                  _MDrv_MTLB_MMap,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
        .unlocked_ioctl=        _MDrv_MTLB_Ioctl,
#else
        .ioctl =  		_MDrv_MTLB_Ioctl,
        #endif
    },
};

static mem_address_manager process_address_manager =
{
	_process_mem_mapregion_map,   /* map_physical */
	_process_mem_mapregion_unmap  /* unmap_physical */
};

static struct vm_operations_struct mtlb_vm_ops =
{
	.open =  _mtlb_vma_open,
	.close = _mtlb_vma_close,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	.fault = _mtlb_cpu_page_fault_handler
#else
	.nopfn = _mtlb_cpu_page_fault_handler
#endif
};


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
void _mtlb_abort(void)
{
	/* make a simple fault by dereferencing a NULL pointer */
	dump_stack();
	*(int *)0 = 0;
}
//================================================================================//
static u32 _mtlb_kernel_page_allocate(void)
{
	struct page *new_page;
	u32 linux_phys_addr;
	new_page = alloc_page(GFP_USER | __GFP_ZERO | __GFP_REPEAT | __GFP_NOWARN | __GFP_COLD | __GFP_NORMAL_MIU0);

	if ( NULL == new_page )
	{
		return INVALID_PAGE;
	}

#if defined(MIPS) && !defined(HIGH_MEMORY_NOT_SUPPORTED)
	/* XXX: GFP_HIUSER would cause 'Segmentation fault' when calling dma_map_page() */
	linux_phys_addr = page_to_phys(new_page);
#else
	/* Ensure page is flushed from CPU caches. */
	linux_phys_addr = dma_map_page(NULL, new_page, 0, PAGE_SIZE, DMA_BIDIRECTIONAL);
#endif

	return linux_phys_addr;
}

static void _mtlb_kernel_page_release(u32 physical_address)
{
	struct page *unmap_page;

	#if !(defined(MIPS) && !defined(HIGH_MEMORY_NOT_SUPPORTED))
	dma_unmap_page(NULL, physical_address, PAGE_SIZE, DMA_BIDIRECTIONAL);
	#endif
	
	unmap_page = pfn_to_page( physical_address >> PAGE_SHIFT );
	MTLB_DEBUG_ASSERT_POINTER( unmap_page );
	__free_page( unmap_page );
}

//================================================================================//
_mtlb_errcode_t _process_mem_mapregion_map(MTLB_FileData *mTLBData, u32 offset, u32 phys_addr, u32 size )
{
	_mtlb_errcode_t ret = _MTLB_ERR_OK;
	AllocationList *alloc_item = NULL;
	
	if (NULL == mTLBData->cookie) return _MTLB_ERR_FAULT;

	MTLB_DEBUG_ASSERT( 0 == (size & ~PAGE_MASK) );

	MTLB_DEBUG_ASSERT( 0 == (offset & ~PAGE_MASK));

	if (size > ( (mTLBData->heap_end - mTLBData->heap_start) - offset))
	{
		MTLB_DPRINTK("%s : virtual memory area not large enough to map physical 0x%x size %x into area 0x%x at offset 0x%xr\n",
		                   	__FUNCTION__, phys_addr, size, mTLBData->heap_start, offset);
		return _MTLB_ERR_FAULT;
	}

	MTLB_DEBUG_ASSERT( 0 == ((phys_addr) & ~PAGE_MASK) );
	
	MTLB_DEBUG("\33[0;36m physaddr = %x -> virtaddr = %x\33[m \n",phys_addr,mTLBData->heap_start + offset);

	/*remove ptes mapping the vma*/
	zap_vma_ptes(mTLBData->cookie, mTLBData->heap_start + offset, size);

    /*remap kernel memory to userspace*/
	ret = (remap_pfn_range(mTLBData->cookie, mTLBData->heap_start + offset, phys_addr >> PAGE_SHIFT, size, mTLBData->cookie->vm_page_prot) ) ? _MTLB_ERR_FAULT : _MTLB_ERR_OK;
	
	if ( ret != _MTLB_ERR_OK)
	{
		MTLB_DPRINTK("%s %d could not remap_pfn_range()\n", __FUNCTION__, __LINE__);
		return ret;
	}

	/* Put our alloc_item into the list of allocations on success */
	alloc_item = kzalloc(sizeof(AllocationList), GFP_KERNEL);
	alloc_item->physaddr = phys_addr;
	alloc_item->offset = offset;
	
	if (NULL == mappingInfo.list)
	{
		mappingInfo.list = alloc_item;
	}
	else
	{
		mappingInfo.tail->next = alloc_item;
	}

	mappingInfo.tail = alloc_item;
	alloc_item->next = NULL;
	/* Write out new physical address on success */

	return ret;
}

void _process_mem_mapregion_unmap(MTLB_FileData *mTLBData, u32 offset, u32 size)
{

   if (NULL == mTLBData->cookie) return;

	MTLB_DEBUG_ASSERT( 0 == (size & ~PAGE_MASK) );

	MTLB_DEBUG_ASSERT( 0 == (offset & ~PAGE_MASK) );
	if (size > ( (mTLBData->heap_end- mTLBData->heap_start) - offset))
	{
		MTLB_DPRINTK("%s : virtual memory area not large enough to unmap size %x from area 0x%x at offset 0x%x\n",
							__FUNCTION__,size, mTLBData->heap_start, offset);
		return;
	}
	
	/* This physical RAM was allocated in _process_mem_mapregion_map and
	 * so needs to be unmapped
	 */
	while (size)
	{
		/* First find the allocation in the list of allocations */
		AllocationList *alloc = mappingInfo.list;
		AllocationList *prev = NULL;

		while (NULL != alloc && alloc->offset != offset)
		{
			prev = alloc;
			alloc = alloc->next;
		}

		if (alloc == NULL) {
			MTLB_DPRINTK("Unmapping memory that isn't mapped\n");
			size -= PAGE_SIZE;
			offset += PAGE_SIZE;
			continue;
		}

        /*The first allocation in the list of allocation is to be deleted*/
        if(prev == NULL)
        {
            mappingInfo.list = alloc->next;
        }
        else
        {
		    prev->next = alloc->next;
        }

		_mtlb_kernel_page_release(alloc->physaddr);
		kfree(alloc);
		
		/* Move onto the next allocation */
		size -= PAGE_SIZE;
		offset += PAGE_SIZE;
	}
	/* Linux does the right thing as part of munmap to remove the mapping */

	return;
}

//================================================================================//
static _mtlb_errcode_t _mtlb_mem_mmap(MTLB_FileData *mTLBData, u32 offset, u32 size)
{
	u32 left = 0,physaddr = INVALID_PAGE;
	int pages_allocated = 0;
	_mtlb_errcode_t err = _MTLB_ERR_OK;
	AllocationList *alloc = mappingInfo.list;
	AllocationList *prev = NULL;
	
	MTLB_DEBUG_ASSERT( 0 == (size & ~PAGE_MASK) );
	MTLB_DEBUG_ASSERT( 0 == (offset & ~PAGE_MASK));
	
	if (size > ( (mTLBData->heap_end - mTLBData->heap_start) - offset))
	{
		MTLB_DPRINTK("%s : virtual memory area not large enough to map size %x into area 0x%x at offset 0x%xr\n",
		                   	__FUNCTION__, size, mTLBData->heap_start, offset);
		return _MTLB_ERR_FAULT;
	}
	
	left = size;

	/*Check if virtual memory page be mapped more than once */
	while (NULL != alloc)
	{
	    MTLB_DEBUG("alloc->offset = %x, offset = %x, size = %x\n", alloc->offset, offset, size);
        MTLB_DEBUG_ASSERT(((alloc->offset + PAGE_SIZE) <= offset) || (alloc->offset >= (offset+ size)));
		prev = alloc;
		alloc = alloc->next;
	}

	
	while (left > 0)
	{
		physaddr = _mtlb_kernel_page_allocate();
		if ( INVALID_PAGE == physaddr )
		{
			MTLB_DPRINTK("\33[0;36m   %s:%d   INVALID_PAGE == physaddr \33[m \n",__FUNCTION__,__LINE__);
			err = _MTLB_ERR_FAULT;
			break;
		}
		err = process_address_manager.map_physical(mTLBData, offset, physaddr, PAGE_SIZE);
		if ( _MTLB_ERR_OK != err)
			break;
		
		/* Loop iteration */
		if (left < PAGE_SIZE) 
			left = 0;
		else 
			left -= PAGE_SIZE;
		pages_allocated++;

		offset += PAGE_SIZE;
	}
	
	if ( _MTLB_ERR_OK != err)
	{

		MTLB_DPRINTK("Mapping of physical memory failed\n");
		/* Fatal error, cleanup any previous pages allocated. */
		if ( pages_allocated > 0 )
		{
			process_address_manager.unmap_physical(mTLBData, offset, PAGE_SIZE*pages_allocated);
		}
		
		pages_allocated = 0;
	}
	
	if (left) 
	{	
		MTLB_DPRINTK("Out of memory. MTLB memory allocated: %ld kB\n",(pages_allocated * PAGE_SIZE)/1024);	
		return _MTLB_ERR_FAULT;
	}
	
	return _MTLB_ERR_OK;
}
static _mtlb_errcode_t _mtlb_mem_munmap(MTLB_FileData *mTLBData, u32 offset, u32 size)
{
	process_address_manager.unmap_physical(mTLBData, offset, size);
	return _MTLB_ERR_OK;
}

static _mtlb_errcode_t _mtlb_mem_traverse(MTLB_FileData *mTLBData)
{
	/* First find the allocation in the list of allocations */
	AllocationList *alloc = mappingInfo.list;
	AllocationList *prev = NULL;
	
    while (NULL != alloc)
    {
        prev = alloc;
        alloc = alloc->next;

        MTLB_DPRINTK("\33[0;36m physaddr = %x -> virtaddr = %x\33[m \n", prev->physaddr, mTLBData->heap_start + prev->offset);
    }

	return _MTLB_ERR_OK;
}

//================================================================================//

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
int _mtlb_cpu_page_fault_handler(struct vm_area_struct *vma, struct vm_fault *vmf)
#else
unsigned long _mtlb_cpu_page_fault_handler(struct vm_area_struct * vma, unsigned long address)
#endif
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	void __user * address;
	address = vmf->virtual_address;
#endif

	MTLB_DPRINTK("\33[0;36m   %s:%d    \33[m \n",__FUNCTION__,__LINE__);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	return VM_FAULT_SIGBUS;
#else
	return NOPFN_SIGBUS;
#endif
}

void _mtlb_vma_open(struct vm_area_struct * vma)
{
	MTLB_VmaData * vma_data;
	vma_data = (MTLB_VmaData*)vma->vm_private_data;
	vma_data->references++;
	MTLB_DPRINTK("\33[0;36m   %s:%d  vma_data->references = %d  \33[m \n",__FUNCTION__,__LINE__,vma_data->references);
	return;
}

void _mtlb_vma_close(struct vm_area_struct * vma)
{
	MTLB_VmaData * vma_data = (MTLB_VmaData*)vma->vm_private_data;
	struct vm_area_struct * vma_priv;
	MTLB_FileData *mTLBData = (MTLB_FileData *)(vma_data->mTLBData);
	
	vma_data->references--;
	vma_priv = (struct vm_area_struct *)vma_data->cookie;
	MTLB_DPRINTK("\33[0;36m   %s:%d  vma_data->references = %d  \33[m \n",__FUNCTION__,__LINE__,vma_data->references);
	
	if (0 != vma_data->references)
	{
		MTLB_DPRINTK("Ignoring this close, %d references still exists\n", vma_data->references);
		return;
	}

	/** @note args->context unused, initialized to 0.
	 * Instead, we use the memory_session from the cookie */
	_mtlb_mem_munmap(mTLBData, 0, mTLBData->heap_end - mTLBData->heap_start);

	kfree(vma_data);
}

//================================================================================//
static int _MDrv_MTLB_Open (struct inode *inode, struct file *filp)
{

    MTLB_FileData *mTLBData;
	
	MTLB_DPRINTK("\33[0;36m   %s:%d    \33[m \n",__FUNCTION__,__LINE__);
    mTLBData = kzalloc(sizeof(*mTLBData), GFP_KERNEL);
    if (mTLBData == NULL)
          return -ENOMEM;

    filp->private_data = mTLBData;
	
    return 0;
}

static int _MDrv_MTLB_Release(struct inode *inode, struct file *filp)
{
    MTLB_FileData *mTLBData = (MTLB_FileData *)filp->private_data ;
	
	MTLB_DPRINTK("\33[0;36m   %s:%d    \33[m \n",__FUNCTION__,__LINE__);
    kfree(mTLBData);
    return 0;
}


static int _MDrv_MTLB_MMap(struct file *filp, struct vm_area_struct *vma)
{
    MTLB_FileData *mTLBData = (MTLB_FileData *)filp->private_data;
	MTLB_VmaData * vma_data;

    mutex_lock(&mtlb_remap_mutex);
    
	if (NULL == vma) 
    {
        mutex_unlock(&mtlb_remap_mutex);
        return _MTLB_ERR_FAULT;
    }

	vma_data = kzalloc(sizeof(* vma_data), GFP_KERNEL);
	if (NULL == vma_data)
	{
		MTLB_DPRINTK("Failed to allocate memory to track memory usage\n");
        mutex_unlock(&mtlb_remap_mutex);
		return _MTLB_ERR_FAULT;
	}
	
	vma->vm_flags |= VM_IO;
	vma->vm_flags |= VM_RESERVED;
	vma->vm_flags |= VM_DONTCOPY;
	vma->vm_flags |= VM_PFNMAP;

#if (defined(MIPS) && !defined(pgprot_writecombine))
	vma->vm_page_prot.pgprot = (vma->vm_page_prot.pgprot & ~_CACHE_MASK) | _CACHE_UNCACHED; /* MIPS's equivalents of 'pgprot_writecombine' */
#else
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
#endif
	vma->vm_ops = &mtlb_vm_ops; /* Operations used on any memory system */

	mTLBData->cookie = vma;
	mTLBData->heap_start = vma->vm_start;
	mTLBData->heap_end = vma->vm_end;

    vma_data->references = 1; /* set initial reference count to be 1 as vma_open won't be called for the first mmap call */
	vma_data->cookie = (u32)vma; /* cookie for munmap */
	vma_data->mTLBData = (u32)mTLBData;
	
	vma->vm_private_data = vma_data;
	MTLB_DEBUG("\33[0;36m   %s:%d   vma->vm_start = %lx,vma->vm_end = %lx \33[m \n",__FUNCTION__,__LINE__,vma->vm_start,vma->vm_end);

    mutex_unlock(&mtlb_remap_mutex);
	
    return 0;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
static long _MDrv_MTLB_Ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int _MDrv_MTLB_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
    int         err= 0;
	int         ret= 0;
    MTLB_FileData *mTLBData = (MTLB_FileData *)filp->private_data ;
	
	mutex_lock(&mtlb_remap_mutex);
    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if (MTLB_IOC_MAGIC!= _IOC_TYPE(cmd))
    {
        mutex_unlock(&mtlb_remap_mutex);
        return -ENOTTY;
    }

    /*
     * the direction is a bitmask, and VERIFY_WRITE catches R/W
     * transfers. `Type' is user-oriented, while
     * access_ok is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }
    
    if (err)
    {
        mutex_unlock(&mtlb_remap_mutex);
        return -EFAULT;
    }

    // @FIXME: Use a array of function pointer for program readable and code size later
    switch(cmd)
    {
    //------------------------------------------------------------------------------
    // Signal
    //------------------------------------------------------------------------------
    case MTLB_IOC_MAP:
    {
		DevMtlb_Map_Info_t stMtlbMapDes;
		ret= copy_from_user(&stMtlbMapDes, (void __user *)arg, sizeof(stMtlbMapDes));
		MTLB_DEBUG("Mtlbmap-> u32Addr:0x%x u32Size:0x%x\n", stMtlbMapDes.u32Addr, stMtlbMapDes.u32Size);
		_mtlb_mem_mmap(mTLBData, (stMtlbMapDes.u32Addr - mTLBData->heap_start), stMtlbMapDes.u32Size);
    }
    break;

    case MTLB_IOC_UNMAP:
    {
		DevMtlb_Map_Info_t stMtlbMapDes;
		ret= copy_from_user(&stMtlbMapDes, (void __user *)arg, sizeof(stMtlbMapDes));
		MTLB_DEBUG("Mtlbunmap->u32Addr:0x%x u32Size:0x%x\n", stMtlbMapDes.u32Addr, stMtlbMapDes.u32Size);
		_mtlb_mem_munmap(mTLBData, (stMtlbMapDes.u32Addr - mTLBData->heap_start), stMtlbMapDes.u32Size);
    }
    break;

    case MTLB_IOC_TRAVERSE:
    {
		_mtlb_mem_traverse(mTLBData);
    }
    break;

    default:
        MTLB_DPRINTK("Unknown ioctl command %d\n", cmd);
        mutex_unlock(&mtlb_remap_mutex);
        return -ENOTTY;
    }
     mutex_unlock(&mtlb_remap_mutex);
    return 0;
}

void _MDrv_MTLB_BIST (void)
{	
    u32 u32va;
    u32 u32pa;
    int i;
	
    MHal_MTLB_Init();
    MHal_MTLB_Mapping(E_MTLB_MIU_0, 0x0000, 0x200000, 0x2000);
    MHal_MTLB_Mapping(E_MTLB_MIU_0, 0x6000, 0x300000, 0x2000);
    MHal_MTLB_Mapping(E_MTLB_MIU_0, 0x2000, 0x900000, 0x2000);
    MHal_MTLB_Mapping(E_MTLB_MIU_0, 0x0000, 0x7100000, 0x4000);
    MHal_MTLB_Mapping(E_MTLB_MIU_0, 0x4000, 0x7100000, 0x2000);
    MHal_MTLB_Mapping(E_MTLB_MIU_0, 0x0000, 0x200000, 0x2000);
    MHal_MTLB_Mapping(E_MTLB_MIU_0, 0x2000, 0x900000, 0x2000);

    for(i=0; i<8; i++)
    {
        u32va = 0x1000*i;
        MHal_MTLB_Dump(E_MTLB_MIU_0, &u32va, &u32pa);
        printk("u32va= 0x%x, u32pa = 0x%x\n", u32va, u32pa);
    }
}

MSYSTEM_STATIC int __init mod_mtlb_init(void)
{
    int s32Ret;
    dev_t dev;
    mtlb_class = class_create(THIS_MODULE, "mtlb");

    if (IS_ERR(mtlb_class))
    {
        return PTR_ERR(mtlb_class);
    }

    if (MTLBDev.s32MTLBMajor)
    {
        dev = MKDEV(MTLBDev.s32MTLBMajor, MTLBDev.s32MTLBMinor);
        s32Ret = register_chrdev_region(dev, MOD_MTLB_DEVICE_COUNT, MOD_MTLB_NAME);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, MTLBDev.s32MTLBMinor, MOD_MTLB_DEVICE_COUNT, MOD_MTLB_NAME);
        MTLBDev.s32MTLBMajor = MAJOR(dev);
    }

    if ( 0 > s32Ret)
    {
        MTLB_DPRINTK("Unable to get major %d\n", MTLBDev.s32MTLBMajor);
        class_destroy(mtlb_class);
        return s32Ret;
    }

    cdev_init(&MTLBDev.cDevice, &MTLBDev.MTLBFop);
    if (0!= (s32Ret= cdev_add(&MTLBDev.cDevice, dev, MOD_MTLB_DEVICE_COUNT)))
    {
        MTLB_DPRINTK("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_MTLB_DEVICE_COUNT);
        class_destroy(mtlb_class);
        return s32Ret;
    }

#if 0  
    _MDrv_MTLB_BIST();
#endif

    device_create(mtlb_class, NULL, dev, NULL, MOD_MTLB_NAME);
    return 0;
}


MSYSTEM_STATIC void __exit mod_mtlb_exit(void)
{
    cdev_del(&MTLBDev.cDevice);
    unregister_chrdev_region(MKDEV(MTLBDev.s32MTLBMajor, MTLBDev.s32MTLBMinor), MOD_MTLB_DEVICE_COUNT);

    device_destroy(mtlb_class, MKDEV(MTLBDev.s32MTLBMajor, MTLBDev.s32MTLBMinor));
    class_destroy(mtlb_class);
}
#if defined(CONFIG_MSTAR_MTLB) || defined(CONFIG_MSTAR_MTLB_MODULE)
module_init(mod_mtlb_init);
module_exit(mod_mtlb_exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("MTLB driver");
MODULE_LICENSE("GPL");
#endif//#if defined(CONFIG_MSTAR_MTLB) || defined(CONFIG_MSTAR_MTLB_MODULE)
