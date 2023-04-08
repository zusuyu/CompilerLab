#include <bits/stdc++.h>

#include "util.hpp"

Result::Result() {
    which = ResultEnum::imm;
    val = 0;
}
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

Value::Value() {
    which = ValueEnum::const_;
    const_val = 0;
}
Value::Value(ValueEnum which_, int const_val_) {
    which = which_;
    const_val = const_val_;
}

extern std::ofstream koopa_ofs;
extern int RegCount;
extern bool BasicBlockEnds;

void koopa_basic_block(std::string label) {
    koopa_print("\n%", label, ":");
    BasicBlockEnds = false;
}
void koopa_array_type(int *len, int k) {
    if (k > 0) {
        koopa_ofs << "[";
        koopa_array_type(len + 1, k - 1);
        koopa_ofs << ", " << len[0] << "]";
    }
    else {
        koopa_ofs << "i32";
    }
}
void koopa_aggregate(int *len, int k, Result *buffer){
    if (k == 0) {
        koopa_ofs << buffer->val;
    }
    else {
        int tot = 1; // size of subarray
        for (int i = 1; i < k; ++i)
            tot *= len[i];
        koopa_ofs << "{";
        for (int i = 0; i < len[0]; ++i) {
            if (i > 0)
                koopa_ofs << ", ";
            koopa_aggregate(len + 1, k - 1, buffer + i * tot);
        }
        koopa_ofs << "}";
    }
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

