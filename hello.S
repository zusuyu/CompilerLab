    .text
    .globl main
main:
    sw fp, -4(sp)
    mv fp, sp
    addi sp, sp, -8
entry:
    li t0, 2
    sw t0, -8(fp)
    lw t0, -8(fp)
    bnez t0, then1
    beqz t0, else1
then1:
    lw t0, -8(fp)
    li t1, 1
    add t0, t0, t1
    addi sp, sp, -8
    sw t0, -12(fp)
    lw t0, -12(fp)
    sw t0, -8(fp)
    li a0, 233
    j FuncReturn0
else1:
    li a0, 666
    j FuncReturn0
FuncReturn0:
    addi sp, sp, 16
    mv sp, fp
    lw fp, -4(sp)
    ret

