    .data
    .globl a_0
a_0:
    .word 1

    .globl b_0
b_0:
    .word 2

    .text
    .globl f
f:
    sw ra, -4(sp)
    sw fp, -8(sp)
    mv fp, sp
    addi sp, sp, -32
entry_f:
    la t0, b_0
    lw t0, (t0)
    sw t0, -12(fp)
    lw t0, -12(fp)
    la t1, a_0
    sw t0, (t1)
    j FuncReturn8
FuncReturn8:
    addi sp, sp, 32
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
    addi sp, sp, -32
entry_main:
    call f
    sw a0, -12(fp)
    la t0, a_0
    lw t0, (t0)
    sw t0, -16(fp)
    lw a0, -16(fp)
    j FuncReturn9
FuncReturn9:
    addi sp, sp, 32
    mv sp, fp
    lw fp, -8(sp)
    lw ra, -4(sp)
    ret

