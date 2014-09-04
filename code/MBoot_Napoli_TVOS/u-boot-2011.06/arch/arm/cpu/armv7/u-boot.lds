/*
 * January 2004 - Changed to support H4 device
 * Copyright (c) 2004-2008 Texas Instruments
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

STACK_SIZE  = 0x20000;

SECTIONS
{
    . = 0x00000000;

    . = ALIGN(4);
    .text :
    {
        __config_text_start = .;
        arch/arm/cpu/armv7/start.o (.text)
        *(.text*)
        *(COMMON)
        . = ALIGN(4);
        __config_text_end = .;
    }

    .rodata :
    {
        __config_rodata_start = .;
        *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
        . = ALIGN(4);
        __config_rodata_end = .;
    }

    .data :
    {
        __config_data_start = .;
        *(.data)
        . = ALIGN(4);
        __config_data_end = .;
    }

    .u_boot_cmd :
    {
        __config_uboot_cmd_start = .;
        __u_boot_cmd_start = .;
        *(.u_boot_cmd)
        . = ALIGN(4);
        __u_boot_cmd_end = .;
        __config_uboot_cmd_end = .;
    }

    .rel.dyn :
    {
        __config_rel_dyn_start = .;
        __rel_dyn_start = .;
        *(.rel*)
        . = ALIGN(4);
        __rel_dyn_end = .;
        __config_rel_dyn_end = .;
    }

    .dynsym :
    {
        __config_dynsym_start = .;
        __dynsym_start = .;
        *(.dynsym*)
        . = ALIGN(4);
        __config_dynsym_end = .;
    }

    _end = .;

    .bss __rel_dyn_start (OVERLAY) :
    {
        __config_bss_start = .;
        __bss_start = .;
        *(.bss)
        *(.bss*)
        . = ALIGN(4);
        __config_bss_end = .;
        __bss_end__ = .;
    }


    /* for sbrk.c */
    __heap = .;
    __heap_end = __heap + 0x640000;

    _ld_irq_stack_end = . ; . = . + STACK_SIZE; _ld_irq_stack_start = . ;
    _ld_fiq_stack_end = . ; . = . + STACK_SIZE; _ld_fiq_stack_start = . ;
    _ld_svc_stack_end = . ; . = . + STACK_SIZE; _ld_svc_stack_start = . ;

    /DISCARD/ : { *(.dynstr*) }
    /DISCARD/ : { *(.dynamic*) }
    /DISCARD/ : { *(.plt*) }
    /DISCARD/ : { *(.interp*) }
    /DISCARD/ : { *(.gnu*) }
}
