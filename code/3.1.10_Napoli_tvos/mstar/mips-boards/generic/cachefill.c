/*
 * Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 1999,2000 MIPS Technologies, Inc.  All rights reserved.
 *
 * This program is free software; you can distribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Kernel command line creation using the prom monitor (YAMON) argc/argv.
 */

#include <linux/module.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/kexec.h>
#include <linux/getcpu.h>
#include <linux/cpu.h>
#include <linux/fs_struct.h>
#include <linux/version.h>

#include "mdrv_mstypes.h"

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/unistd.h>

#define _BITMASK(loc_msb, loc_lsb) ((1U << (loc_msb)) - (1U << (loc_lsb)) + (1U << (loc_msb)))
#define BITMASK(x) _BITMASK(1?x, 0?x)

// for Run in cache

#define CONFIG_KSEG0_MASK (0x00000007)
#define KSEG0_UNCACHED (0x2)
#define KSEG0_CACHE (0x3) // Cacheable, noncoherent, write-back, write allocate
#define KSEG0_BASE  (0x80000000)
#define KSEG1_BASE  (0xA0000000)

#define MIPS_CLOCK_FREQ_XTAL 12000000

#define HAL_MIPS_CACHE_INDEX_LOAD_TAG_I      0x04 /* 1 0 */
#define HAL_MIPS_CACHE_INDEX_LOAD_TAG_D      0x05 /* 1 1 */
#define HAL_MIPS_CACHE_INDEX_STORE_TAG_I     0x08 /* 2 0 */
#define HAL_MIPS_CACHE_INDEX_STORE_TAG_D     0x09 /* 2 1 */
#define HAL_MIPS_CACHE_INDEX_STORE_DATA_I    0x0c /* 7 0 */
#define HAL_MIPS_CACHE_INDEX_STORE_DATA_D    0x0d /* 7 1 */

#define hazard_barrier() ({ asm volatile(".set push\n"\
                                         ".set mips32r2\n"\
                                         "ehb\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"\
                                         ".set pop\n"); 0; })


typedef struct {
        unsigned int waysize;   /* Bytes per way */
        unsigned short sets;    /* Number of lines per set */
        unsigned char ways;     /* Number of ways */
        unsigned char linesz;   /* Size of line in bytes */
        unsigned char waybit;   /* Bits to select in a cache set */
        unsigned char flags;    /* Flags describing cache properties */
}cache_desc;


void read_icache_config(cache_desc *icache_dsc)
{
    unsigned int config1;
    unsigned int lsize;
    unsigned int icache_size;

    asm volatile (       \
            ".set\tmips32\n\t"   \
            "mfc0 %0, $16, 1\n\t"\
            ".set\tmips0\n\t"    \
            : "=r" (config1));
    printk("icache config = %x\n", config1);

    if ((lsize = ((config1 >> 19) & 7)))
    {
        icache_dsc->linesz = 2 << lsize;
    }
    else
    {
        icache_dsc->linesz = lsize;
    }
    icache_dsc->sets = 64 << ((config1 >> 22) & 7);
    icache_dsc->ways = 1 + ((config1 >> 16) & 7);

    icache_size = icache_dsc->sets *
                  icache_dsc->ways *
                  icache_dsc->linesz;
    //icache_dsc->waybit = ffs(icache_size/icache_dsc->ways) - 1;
    icache_dsc->waybit =  __builtin_ffs(icache_size/icache_dsc->ways) - 1;
    printk("icache_dsc->linesz = %x\n", icache_dsc->linesz);
    printk("icache_dsc->sets = %x\n", icache_dsc->sets);
    printk("icache_dsc->ways = %x\n", icache_dsc->ways);
    printk("icache_dsc->waybit = %x\n", icache_dsc->waybit);
    printk("icache_size = %x\n", icache_size);
}

void read_dcache_config(cache_desc *dcache_dsc)
{
    unsigned int config1;
    unsigned int lsize;
    unsigned int dcache_size;

    asm volatile (       \
            ".set\tmips32\n\t"   \
            "mfc0 %0, $16, 1\n\t"\
            ".set\tmips0\n\t"    \
            : "=r" (config1));
    printk("dcache config = %x\n", config1);

    if ((lsize = ((config1 >> 10) & 7)))
    {
        dcache_dsc->linesz = 2 << lsize;
    }
    else
    {
        dcache_dsc->linesz = lsize;
    }
    dcache_dsc->sets = 64 << ((config1 >> 13) & 7);
    dcache_dsc->ways = 1 + ((config1 >> 7) & 7);

    dcache_size = dcache_dsc->sets *
                  dcache_dsc->ways *
                  dcache_dsc->linesz;
    //icache_dsc->waybit = ffs(icache_size/icache_dsc->ways) - 1;
    dcache_dsc->waybit =  __builtin_ffs(dcache_size/dcache_dsc->ways) - 1;
    printk("dcache_dsc->linesz = %x\n", dcache_dsc->linesz);
    printk("dcache_dsc->sets = %x\n", dcache_dsc->sets);
    printk("dcache_dsc->ways = %x\n", dcache_dsc->ways);
    printk("dcache_dsc->waybit = %x\n", dcache_dsc->waybit);
    printk("dcache_size = %x\n", dcache_size);
}

void enable_cache(MS_BOOL bEnable)
{
    unsigned int config;

    asm volatile("mfc0 %0, $16\n" \
             ".set push\n"    \
             ".set mips32r2\n"\
             "ehb\n"          \
             ".set pop\n" : "=r" (config) : );

    config = (config & ~CONFIG_KSEG0_MASK) | (bEnable ? KSEG0_CACHE : KSEG0_UNCACHED);

    asm volatile("mtc0 %0, $16\n" \
             ".set push\n"    \
             ".set mips32r2\n"\
             "ehb\n"          \
             ".set pop\n" : : "r" (config) );
}

void invalidate_icache(void)
{
    cache_desc c;
    MS_U32 linesize;
    MS_U32 start;
    MS_U32 end;

    MS_U32 addr;
    MS_U8  u8OffsetBit, u8IndexBit, u8WayBit;
    MS_U32 u32Way, u32Index;

    read_icache_config(&c);
    linesize = c.linesz;

    u8OffsetBit = __builtin_ffs(c.linesz) - 1;
    u8IndexBit = __builtin_ffs( (c.sets * c.ways * c.linesz)/ c.ways) - 1;
    u8WayBit = u8IndexBit + (__builtin_ffs(c.ways) - 1);

    start = KSEG0_BASE;
    end = start + (c.sets * c.ways * c.linesz); // cache size

    // Invalidate lines
    asm volatile("mtc0 %0, $26" : : "r" (0x00000000));
    hazard_barrier();

    for(addr = start; addr < end; addr += linesize)
    {

        unsigned int taglo;

        u32Way = addr & BITMASK(u8WayBit-1 : u8IndexBit);
        u32Index = addr & BITMASK(u8IndexBit-1 : u8OffsetBit);

        taglo = (__pa(addr) & 0xFFFFF800) & ~(1 << 7) & ~(1 << 5); // PTaglo[31:11], V[7], L[5]
        asm volatile("mtc0  %0, $28, 0" : : "r" (taglo));
        hazard_barrier();

        asm volatile("cache %0, 0(%1)" : :
                "I" (HAL_MIPS_CACHE_INDEX_STORE_TAG_I),
                "r" (u32Way | u32Index));
        hazard_barrier();
    }
}

MS_BOOL ric_fill_icache(MS_U32 insts[], MS_U32 size)
{
    cache_desc c;
    MS_U32 linesize;
    MS_U32 start;
    MS_U32 end;
    MS_U32 ws_inc;
    MS_U32 ws_end;
    MS_U32 ws, bytes, addr, i;
    MS_U8  u8OffsetBit, u8IndexBit, u8WayBit;
    MS_U32 u32Way, u32Index;

    read_icache_config(&c);
    linesize = c.linesz;

    start = (MS_U32)(((MS_U32)insts&0x1FFFFFFF)|KSEG0_BASE);//0x83412000;
    end = (MS_U32)  (start+size);

    printk("start = 0x%x, end = 0x%x\n", (unsigned int)start, (unsigned int)end);

    ws_inc = 1U << c.waybit;
    ws_end = c.ways << c.waybit;

    u8OffsetBit = __builtin_ffs(c.linesz) - 1;
    u8IndexBit = __builtin_ffs( (c.sets * c.ways * c.linesz)/ c.ways) - 1;
    u8WayBit = u8IndexBit + (__builtin_ffs(c.ways) - 1);
    printk("u8OffsetBit = %d \n", u8OffsetBit);
    printk("u8IndexBit = %d \n", u8IndexBit);
    printk("u8WayBit = %d \n", u8WayBit);

    if(size % 4)
    {
        printk("The size is unalignment\n");
        return FALSE;
    }

    // Fill instruction to each way
    i = 0;
    for(addr = start; addr < end; addr += linesize)
    {
        unsigned int taglo;

        asm volatile("mtc0 %0, $26" : : "r" (0x20000000));
        hazard_barrier();

        u32Way = addr & BITMASK(u8WayBit-1 : u8IndexBit);
        u32Index = addr & BITMASK(u8IndexBit-1 : u8OffsetBit);

        for(bytes = 0; bytes < linesize; bytes += 8)
        {
            unsigned int lo = insts[i++];
            unsigned int hi = insts[i++];
            asm volatile("mtc0  %0, $28, 1" : : "r" (lo));
            asm volatile("mtc0  %0, $29, 1" : : "r" (hi));
            hazard_barrier();

            asm volatile("cache %0, 0(%1)" : :
                "I" (HAL_MIPS_CACHE_INDEX_STORE_DATA_I),
                "r" (u32Way | u32Index | bytes));
            hazard_barrier();
        }

        // Lock line

        asm volatile("mtc0 %0, $26" : : "r" (0x00000000));
        hazard_barrier();

        taglo = (__pa(addr) & 0xFFFFF800) | (1 << 7) | (1 << 5); // PTaglo[31:11], V[7], L[5]
        asm volatile("mtc0  %0, $28, 0" : : "r" (taglo));
        hazard_barrier();

        //u32Index = addr & BITMASK(u8IndexBit-1 : u8OffsetBit);
        asm volatile("cache %0, 0(%1)" : :
                "I" (HAL_MIPS_CACHE_INDEX_STORE_TAG_I),
                "r" (u32Way | u32Index));
        hazard_barrier();
    }


    printk("Clear Way-Select\n");
    // Clear Way-Select
    asm volatile("mtc0 %0, $26" : : "r" (0x20000000));
    hazard_barrier();
    for(addr = start; addr < end; addr += linesize)
    {
        for(ws = 0; ws < ws_end; ws += ws_inc)
        {
            asm volatile("mtc0  %0, $28, 0" : : "r" (0));
            hazard_barrier();

            asm volatile("cache %0, 0(%1)" : :
                    "I" (HAL_MIPS_CACHE_INDEX_STORE_TAG_I),
                    "r" (addr ^ ws));
            hazard_barrier();
        }
    }
    // Disable WST test mode
    asm volatile("mtc0 %0, $26" : : "r" (0x00000000));
    hazard_barrier();

    printk("Disable WST test mode\n");

    return TRUE;
}

MS_BOOL ric_fill_dcache(MS_U32 insts[], MS_U32 size)
{
    cache_desc c;
    MS_U32 linesize;
    MS_U32 start;
    MS_U32 end;
    MS_U32 ws_inc;
    MS_U32 ws_end;
    MS_U32 ws, bytes, addr, i;
    MS_U8  u8OffsetBit, u8IndexBit, u8WayBit;
    MS_U32 u32Way, u32Index;

    read_dcache_config(&c);
    linesize = c.linesz;

    start = (MS_U32)(((MS_U32)insts&0x1FFFFFFF)|KSEG0_BASE);//0x8341A000;
    end = (MS_U32)  (start+size);

    printk("start = 0x%x, end = 0x%x\n", (unsigned int)start, (unsigned int)end);

    ws_inc = 1U << c.waybit;
    ws_end = c.ways << c.waybit;

    u8OffsetBit = __builtin_ffs(c.linesz) - 1;
    u8IndexBit = __builtin_ffs( (c.sets * c.ways * c.linesz)/ c.ways) - 1;
    u8WayBit = u8IndexBit + (__builtin_ffs(c.ways) - 1);
    printk("u8OffsetBit = %d \n", u8OffsetBit);
    printk("u8IndexBit = %d \n", u8IndexBit);
    printk("u8WayBit = %d \n", u8WayBit);

    if(size % 4)
    {
        printk("The size is unalignment\n");
        return FALSE;
    }

    // Fill instruction to each way
    i = 0;
    for(addr = start; addr < end; addr += linesize)
    {
        unsigned int taglo;

        asm volatile("mtc0 %0, $26" : : "r" (0x20000000));
        hazard_barrier();

        u32Way = addr & BITMASK(u8WayBit-1 : u8IndexBit);
        u32Index = addr & BITMASK(u8IndexBit-1 : u8OffsetBit);

        for(bytes = 0; bytes < linesize; bytes += 4)
        {
            unsigned int lo = insts[i++];
            //unsigned int hi = insts[i++];
            asm volatile("mtc0  %0, $28, 3" : : "r" (lo));
            //asm volatile("mtc0  %0, $29, 1" : : "r" (hi));
            hazard_barrier();

            asm volatile("cache %0, 0(%1)" : :
                "I" (HAL_MIPS_CACHE_INDEX_STORE_DATA_D),
                "r" (u32Way | u32Index | bytes));
            hazard_barrier();
        }

        // Lock line

        asm volatile("mtc0 %0, $26" : : "r" (0x00000000));
        hazard_barrier();

        taglo = (__pa(addr) & 0xFFFFF800) | (1 << 7);// | (1 << 5); // PTaglo[31:11], V[7], L[5]
        asm volatile("mtc0  %0, $28, 2" : : "r" (taglo));
        hazard_barrier();

        //u32Index = addr & BITMASK(u8IndexBit-1 : u8OffsetBit);
        asm volatile("cache %0, 0(%1)" : :
                "I" (HAL_MIPS_CACHE_INDEX_STORE_TAG_D),
                "r" (u32Way | u32Index));
        hazard_barrier();
    }


    printk("Clear Way-Select\n");
    // Clear Way-Select
    asm volatile("mtc0 %0, $26" : : "r" (0x20000000));
    hazard_barrier();
    for(addr = start; addr < end; addr += linesize)
    {
        for(ws = 0; ws < ws_end; ws += ws_inc)
        {
            asm volatile("mtc0  %0, $28, 2" : : "r" (0));
            hazard_barrier();

            asm volatile("cache %0, 0(%1)" : :
                    "I" (HAL_MIPS_CACHE_INDEX_STORE_TAG_D),
                    "r" (addr ^ ws));
            hazard_barrier();
        }
    }
    // Disable WST test mode
    asm volatile("mtc0 %0, $26" : : "r" (0x00000000));
    hazard_barrier();

    printk("Disable WST test mode\n");

    return TRUE;
}


