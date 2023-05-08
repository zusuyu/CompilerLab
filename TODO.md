
docker run -it --rm -v C:\Lecture-Notes\compiler\lab\lab-cpp:/root/compiler maxxing/compiler-dev bash

./compiler -koopa ../hello.c -o ../hello.koopa
./compiler -riscv ../hello.c -o ../hello.S

autotest -koopa -s lv4 /root/compiler/

flex -o sysy.lex.cpp ../src/sysy.l
bison -d -o sysy.tab.cpp ../src/sysy.y -Wconflicts-sr -Wconflicts-rr


./compiler -koopa ../hello.c -o ../hello.koopa && koopac ../hello.koopa | llc --filetype=obj -o ../hello.o && clang ../hello.o -L$CDE_LIBRARY_PATH/native -lsysy -o ../hello && ../hello

./compiler -riscv ../hello.c -o ../hello.S && clang ../hello.S -c -o ../hello.o -target riscv32-unknown-linux-elf -march=rv32im -mabi=ilp32 && ld.lld ../hello.o -L$CDE_LIBRARY_PATH/riscv32 -lsysy -o ../hello && qemu-riscv32-static ../hello


怎么你妈的这么多 Bug

一开始认为每个 int 返回值的函数必须要显式以 return 结尾 但实际上在每一个 if 分支里有 return 也是可以的
所以应该直接不考虑这件事 毕竟没写返回值是 UB
关于 ret, br, jump 和 label: br 和 jump 之后都会有明确的 label 但 ret 不同 所以解决方案应该是在每个 ret 语句后加一个无意义的 label


koopa 过了 riscv 还剩七个点
下辈子再来调吧

RiscV 额外
lv8
14_short_circuit1：RE
lv9
26_summary1：RE
27_bfs2：AE 
selected
02_color：CTE
03_exgcd：RE
15_long_func：AE
16_long_array：RE


