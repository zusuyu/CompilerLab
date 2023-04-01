

3.2 finished

Logical Or / Logical And / Eq / Rel

finished in ast.hpp, but not in sysy.y and ast.cpp

How to implement && and || for integer input?

    a || b  ->  %0 = or a, b
                %1 = ne %0, 0
                ret %1

    a && b  ->  %0 = ne a, 0
                %1 = ne b, 0
                %2 = and %0, %1
                ret %2





docker run -it --rm -v C:\Lecture-Notes\compiler\lab\lab-cpp:/root/compiler maxxing/compiler-dev bash

./compiler -koopa hello.c -o hello.koopa

autotest -koopa -s lv3 /root/compiler/