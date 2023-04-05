
(?) 把 @name 也弄进 Result


docker run -it --rm -v C:\Lecture-Notes\compiler\lab\lab-cpp:/root/compiler maxxing/compiler-dev bash

./compiler -koopa ../hello.c -o ../hello.koopa
./compiler -riscv ../hello.c -o ../hello.S

autotest -koopa -s lv4 /root/compiler/

flex -o sysy.lex.cpp ../src/sysy.l
bison -d -o sysy.tab.cpp ../src/sysy.y -Wconflicts-sr -Wconflicts-rr

koopac ../hello.koopa | llc --filetype=obj -o ../hello.o
clang ../hello.o -L$CDE_LIBRARY_PATH/native -lsysy -o ../hello
../hello