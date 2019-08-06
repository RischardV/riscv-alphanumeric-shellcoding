/*#
 *# (c) 2018-2019 Hadrien Barral
 *# SPDX-License-Identifier: Apache-2.0
 */

    fence.i            # 0 1 # FIXED
    lui  XQ, 0xfffff   # 2   # FIXED
    add  XQ, XQ, sp    # 3   # FIXED

                       #     # These 3 can be interchanged freely
    addi XS, XQ, 0x400 # 4 5
    mv   XP, XQ        # 6
    mv   XJ, XQ        # 7

next:
    addi XP, XP, 2     # 8    # Can move quite freely in block
    lw   XA, 4(XP)     # 9
    mv   XB, XA        # 10
    srli XB, XB, 4     # 11
    xor  XA, XA, XB    # 12
    sw   XA, 0(XQ)     # 13
    addi XQ, XQ, 1     # 14
    bltu  XQ, XS, next # 15 16
done:
    fence.i            # 17 18 # Can be moved up
    jalr XJ            # 19
