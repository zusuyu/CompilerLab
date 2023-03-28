3.2 finished

Logical Or / Logical And / Eq / Rel

finished in ast.hpp, but not in sysy.l and ast.cpp

How to implement && and || for integer input?

    a || b  ->  %0 = or a, b
                %1 = ne %0, 0
                ret %1

    a && b  ->  %0 = ne a, 0
                %1 = ne b, 0
                %2 = and %0, %1
                ret %2