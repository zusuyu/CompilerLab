#include <bits/stdc++.h>

#include "util.hpp"

Result::Result() {}
Result::Result(ResultEnum which_, int val_) {
    which = which_;
    val = val_;
}
std::ostream & operator << (std::ostream &ofs, Result res) {
    if (res.which == Result::ResultEnum::reg)
        ofs << '%';
    ofs << res.val;
    return ofs;
}

Value::Value() {}
Value::Value(ValueEnum which_, int val_) {
    which = which_;
    val = val_;
}

extern std::ofstream koopa_ofs;
extern int RegCount;

Result calc(std::string oprand, Result s1, Result s2) {
    if (s1.which == Result::ResultEnum::reg || s2.which == Result::ResultEnum::reg) {
        Result d = Reg(RegCount++);
        koopa_ofs << d << "= " << oprand << " " << s1 << ", " << s2 << "\n";
        return d;
    }
    if (oprand == "ne") {
        return Imm(s1.val != s2.val);
    }
    if (oprand == "eq") {
        return Imm(s1.val == s2.val);
    }
    if (oprand == "gt") {
        return Imm(s1.val > s2.val);
    }
    if (oprand == "lt") {
        return Imm(s1.val < s2.val);
    }
    if (oprand == "ge") {
        return Imm(s1.val >= s2.val);
    }
    if (oprand == "le") {
        return Imm(s1.val <= s2.val);
    }
    if (oprand == "add") {
        return Imm(s1.val + s2.val);
    }
    if (oprand == "sub") {
        return Imm(s1.val - s2.val);
    }
    if (oprand == "mul") {
        return Imm(s1.val * s2.val);
    }
    if (oprand == "div") {
        return Imm(s1.val / s2.val);
    }
    if (oprand == "mod") {
        return Imm(s1.val % s2.val);
    }
    if (oprand == "and") {
        return Imm(s1.val & s2.val);
    }
    if (oprand == "or") {
        return Imm(s1.val | s2.val);
    }
    if (oprand == "xor") {
        return Imm(s1.val ^ s2.val);
    }
    return Result();    
}

