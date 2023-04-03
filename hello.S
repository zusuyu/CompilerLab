    .text
    .globl main
main:
    addi sp, sp, -256
entry:
    li t0, 2
    sw t0, 0(sp)
    lw t0, 0(sp)
    bnez t0, then1
    beqz t0, else1
then1:
    lw t0, 0(sp)
    li t1, 1
    add t0, t0, t1
    sw t0, 4(sp)
    lw t0, 4(sp)
    sw t0, 0(sp)
    li a0, 233
    j .FuncReturn0
else1:
    li a0, 666
    j .FuncReturn0
.FuncReturn0:
    addi sp, sp, 256
    ret

