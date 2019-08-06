/*#
 *# (c) 2018-2019 Hadrien Barral
 *# SPDX-License-Identifier: Apache-2.0
 */

.section .start
_start:
    .globl _start
    li sp, PAYLOAD_STACK
    jal main
    j .
