    .text
    .globl f
f:
    sw ra, -4(sp)
    sw fp, -8(sp)
    sw a0,-12(sp)
    sw a1,-16(sp)
    sw a2,-20(sp)
    sw a3,-24(sp)
    sw a4,-28(sp)
    sw a5,-32(sp)
    sw a6,-36(sp)
    sw a7,-40(sp)
    mv fp, sp
    addi sp, sp, -64
entry_f:
    li t0, -44
    add t0, t0, fp
    li t1, -48
    add t1, t1, fp
    sw t0, (t1)
    li t1, -48
    add t1, t1, fp
    lw t1, (t1)
    lw t0, -12(fp)
    sw t0, (t1)
    li t0, -52
    add t0, t0, fp
    li t1, -56
    add t1, t1, fp
    sw t0, (t1)
    li t1, -56
    add t1, t1, fp
    lw t1, (t1)
    lw t0, -16(fp)
    sw t0, (t1)
    li t0, -60
    add t0, t0, fp
    li t1, -64
    add t1, t1, fp
    sw t0, (t1)
    li t1, -64
    add t1, t1, fp
    lw t1, (t1)
    lw t0, -20(fp)
    sw t0, (t1)
    addi sp, sp, -64
    li t0, -68
    add t0, t0, fp
    li t1, -72
    add t1, t1, fp
    sw t0, (t1)
    li t1, -72
    add t1, t1, fp
    lw t1, (t1)
    lw t0, -24(fp)
    sw t0, (t1)
    li t0, -76
    add t0, t0, fp
    li t1, -80
    add t1, t1, fp
    sw t0, (t1)
    li t1, -80
    add t1, t1, fp
    lw t1, (t1)
    lw t0, -28(fp)
    sw t0, (t1)
    li t0, -84
    add t0, t0, fp
    li t1, -88
    add t1, t1, fp
    sw t0, (t1)
    li t1, -88
    add t1, t1, fp
    lw t1, (t1)
    lw t0, -32(fp)
    sw t0, (t1)
    li t0, -92
    add t0, t0, fp
    li t1, -96
    add t1, t1, fp
    sw t0, (t1)
    li t1, -96
    add t1, t1, fp
    lw t1, (t1)
    lw t0, -36(fp)
    sw t0, (t1)
    li t0, -100
    add t0, t0, fp
    li t1, -104
    add t1, t1, fp
    sw t0, (t1)
    li t1, -104
    add t1, t1, fp
    lw t1, (t1)
    lw t0, -40(fp)
    sw t0, (t1)
    li t0, -108
    add t0, t0, fp
    li t1, -112
    add t1, t1, fp
    sw t0, (t1)
    li t1, -112
    add t1, t1, fp
    lw t1, (t1)
    lw t0, 0(fp)
    sw t0, (t1)
    li t0, -112
    add t0, t0, fp
    lw t0, (t0)
    lw t0, (t0)
    li t1, -116
    add t1, t1, fp
    sw t0, (t1)
    li a0, -116
    add a0, a0, fp
    lw a0, (a0)
    j func_return_f
func_return_f:
    addi sp, sp, 128
    mv sp, fp
    lw fp, -8(sp)
    lw ra, -4(sp)
    ret

    .text
    .globl main
main:
    sw ra, -4(sp)
    sw fp, -8(sp)
    mv fp, sp
    addi sp, sp, -64
entry_main:
    addi sp, sp, -64
    li a0, 0
    li a1, 0
    li a2, 0
    li a3, 0
    li a4, 0
    li a5, 0
    li a6, 0
    li a7, 0
    li t0, 114514
    li t1, 0
    add t1, t1, sp
    sw t0, (t1)
    call f
    addi sp, sp, 64
    li t0, -12
    add t0, t0, fp
    sw a0, (t0)
    li a0, -12
    add a0, a0, fp
    lw a0, (a0)
    call putint
    li t0, -16
    add t0, t0, fp
    sw a0, (t0)
    li a0, 114514
    j func_return_main
func_return_main:
    addi sp, sp, 64
    mv sp, fp
    lw fp, -8(sp)
    lw ra, -4(sp)
    ret

