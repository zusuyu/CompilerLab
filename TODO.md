
docker run -it --rm -v C:\Lecture-Notes\compiler\lab\lab-cpp:/root/compiler maxxing/compiler-dev bash

./compiler -koopa ../hello.c -o ../hello.koopa
./compiler -riscv ../hello.c -o ../hello.S

autotest -koopa -s lv4 /root/compiler/

flex -o sysy.lex.cpp ../src/sysy.l
bison -d -o sysy.tab.cpp ../src/sysy.y -Wconflicts-sr -Wconflicts-rr

koopac ../hello.koopa | llc --filetype=obj -o ../hello.o
clang ../hello.o -L$CDE_LIBRARY_PATH/native -lsysy -o ../hello
../hello


./compiler -riscv ../hello.c -o ../hello.S && clang ../hello.S -c -o ../hello.o -target riscv32-unknown-linux-elf -march=rv32im -mabi=ilp32 && ld.lld ../hello.o -L$CDE_LIBRARY_PATH/riscv32 -lsysy -o ../hello && qemu-riscv32-static ../hello


怎么你妈的这么多 Bug

Koopa:
lv1 
10_summary1：ONF 
lv6
13_branch2：CTE 
lv7
13_continue1：CTE 
lv8
16_summary1：CTE 
24_gcd3：CTE 
lv9
23_dijkstra1：CTE
24_floyd1：CTE
25_matrix_mul1：CTE
26_summary1：CTE
27_bfs2：CTE
selected
03_exgcd：CTE
05_expr_eval：CTE
08_max_flow：CTE
10_substr：CTE
18_long_code：CTE
22_nested_calls：CTE

RiscV 额外
lv8
14_short_circuit1：RE
selected
02_color：CTE
15_long_func：AE
16_long_array：RE