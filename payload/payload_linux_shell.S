/*#
 *# (c) 2018-2019 Hadrien Barral
 *# SPDX-License-Identifier: Apache-2.0
 */

.section .start
_start:
    .globl _start
    nop
    nop

    # Get a stack
    jal sp, next
next:
    addi sp, sp, -32

    li sp, PAYLOAD_STACK
    jal main
    li a7, 93
    ecall
