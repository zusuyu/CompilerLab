    .text
    .globl f
f:
    sw ra, -4(sp)
    sw fp, -8(sp)
    sw a0,-12(sp)
    sw a1,-16(sp)
    mv fp, sp
    addi sp, sp, -64
entry_f:
    li t0, -20
    add t0, t0, fp
    li t1, -24
    add t1, t1, fp
    sw t0, (t1)
    li t1, -24
    add t1, t1, fp
    lw t1, (t1)
    lw t0, -12(fp)
    sw t0, (t1)
    li t0, -28
    add t0, t0, fp
    li t1, -32
    add t1, t1, fp
    sw t0, (t1)
    li t1, -32
    add t1, t1, fp
    lw t1, (t1)
    lw t0, -16(fp)
    sw t0, (t1)
    li t0, -32
    add t0, t0, fp
    lw t0, (t0)
    lw t0, (t0)
    li t1, -36
    add t1, t1, fp
    sw t0, (t1)
    li t0, -24
    add t0, t0, fp
    lw t0, (t0)
    lw t0, (t0)
    li t1, -40
    add t1, t1, fp
    sw t0, (t1)
    li t0, -40
    add t0, t0, fp
    lw t0, (t0)
    li t1, -36
    add t1, t1, fp
    lw t1, (t1)
    li t2, 4
    mul t1, t1, t2
    add t0, t0, t1
    li t3, -44
    add t3, t3, fp
    sw t0, (t3)
    li t1, -44
    add t1, t1, fp
    lw t1, (t1)
    li t0, 114
    sw t0, (t1)
    j func_return_f
func_return_f:
    addi sp, sp, 64
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
    li t0, -20
    add t0, t0, fp
    li t1, -24
    add t1, t1, fp
    sw t0, (t1)
    li t1, -24
    add t1, t1, fp
    lw t1, (t1)
    li t0, 0
    sw t0, (t1)
    add t1, t1, 4
    li t0, 0
    sw t0, (t1)
    add t1, t1, 4
    li t0, 0
    sw t0, (t1)
    add t1, t1, 4
    li t0, -24
    add t0, t0, fp
    lw t0, (t0)
    li t1, 0
    li t2, 4
    mul t1, t1, t2
    add t0, t0, t1
    li t3, -28
    add t3, t3, fp
    sw t0, (t3)
    li a0, -28
    add a0, a0, fp
    lw a0, (a0)
    li a1, 1
    call f
    li t0, -32
    add t0, t0, fp
    sw a0, (t0)
    li t0, -24
    add t0, t0, fp
    lw t0, (t0)
    li t1, 1
    li t2, 4
    mul t1, t1, t2
    add t0, t0, t1
    li t3, -36
    add t3, t3, fp
    sw t0, (t3)
    li t0, -36
    add t0, t0, fp
    lw t0, (t0)
    lw t0, (t0)
    li t1, -40
    add t1, t1, fp
    sw t0, (t1)
    li a0, -40
    add a0, a0, fp
    lw a0, (a0)
    call putint
    li t0, -44
    add t0, t0, fp
    sw a0, (t0)
    li a0, 233
    j func_return_main
func_return_main:
    addi sp, sp, 64
    mv sp, fp
    lw fp, -8(sp)
    lw ra, -4(sp)
    ret

