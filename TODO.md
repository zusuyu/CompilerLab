
(?) 把 @name 也弄进 Result


docker run -it --rm -v C:\Lecture-Notes\compiler\lab\lab-cpp:/root/compiler maxxing/compiler-dev bash

./compiler -koopa ../hello.c -o ../hello.koopa

autotest -koopa -s lv4 /root/compiler/


koopac ../hello.koopa | llc --filetype=obj -o ../hello.o
clang ../hello.o -L$CDE_LIBRARY_PATH/native -lsysy -o ../hello
../hello