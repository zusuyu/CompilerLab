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
extern bool BasicBlockEnds;

void koopa_basic_block(std::string label) {
    koopa_print("\n%", label, ":");
    BasicBlockEnds = false;
}
void koopa_ret(Result res) {
    if (!BasicBlockEnds)
        koopa_print("  ret ", res);
    BasicBlockEnds = true;
}
void koopa_ret() {
    if (!BasicBlockEnds)
        koopa_print("  ret");
    BasicBlockEnds = true;
}

Result calc(std::string op, Result s1, Result s2) {
    if (s1.which == Result::ResultEnum::reg || s2.which == Result::ResultEnum::reg) {
        Result d = Reg(RegCount++);
        koopa_inst(d, " = ", op, " ", s1, ", ", s2);
        return d;
    }
    if (op == "ne") {
        return Imm(s1.val != s2.val);
    }
    if (op == "eq") {
        return Imm(s1.val == s2.val);
    }
    if (op == "gt") {
        return Imm(s1.val > s2.val);
    }
    if (op == "lt") {
        return Imm(s1.val < s2.val);
    }
    if (op == "ge") {
        return Imm(s1.val >= s2.val);
    }
    if (op == "le") {
        return Imm(s1.val <= s2.val);
    }
    if (op == "add") {
        return Imm(s1.val + s2.val);
    }
    if (op == "sub") {
        return Imm(s1.val - s2.val);
    }
    if (op == "mul") {
        return Imm(s1.val * s2.val);
    }
    if (op == "div") {
        return Imm(s1.val / s2.val);
    }
    if (op == "mod") {
        return Imm(s1.val % s2.val);
    }
    if (op == "and") {
        return Imm(s1.val & s2.val);
    }
    if (op == "or") {
        return Imm(s1.val | s2.val);
    }
    if (op == "xor") {
        return Imm(s1.val ^ s2.val);
    }
    return Result();    
}

