/*
 * (c) 2018-2019 Hadrien Barral
 * SPDX-License-Identifier: Apache-2.0
 */

ENTRY (_start)

SECTIONS
{
    . = 0;
    .text . : { *(.start) *(.text*) *(.rodata*) }
    .data : { *(.data*) }
    .bss : { *(.bss*) }
}
