/*#
 *# (c) 2018-2019 Hadrien Barral
 *# SPDX-License-Identifier: Apache-2.0
 */

/*# Allowed registers: s0;a0-a5 (also s1 in specific cases)
 *# (exception: XS can take any value)
 */
#define XA s0
#define XB a5
#define XP a1
#define XQ a2
#define XS t5
#define XJ a3

    fence.i
    lui  XQ, 0xfffff    /*# Negative offset to get back somewhere in pool */
                        /*#  'insert' needs to sync with this */
    add  XQ, XQ, sp     /*# Start addr to store to */
    addi XS, XQ, 0x400  /*# Upper store limit */
    mv   XP, XQ         /*# Start addr to load from */
    mv   XJ, XQ         /*# Where to jump */
/*# Decoding loop. See paper for explanation */
next:
    lw   XA, 4(XP)
    mv   XB, XA
    srli XB, XB, 4
    addi XP, XP, 2
    xor  XA, XA, XB
    sw   XA, 0(XQ)
    addi XQ, XQ, 1
    bltu  XQ, XS, next
done:
    fence.i
    jalr XJ
