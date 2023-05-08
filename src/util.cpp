#include <bits/stdc++.h>

#include "ast.hpp"
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

DataType::DataType() {
    which = DataTypeEnum::const_;
    val = 0;
}
DataType::DataType(DataTypeEnum which_, int val_) {
    which = which_;
    val = val_;
}

extern std::ofstream koopa_ofs;
extern int RegCount;

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
/* 
 * dereference a pointer ident on subs
 *     ident is a (ty.val + 1)-level pointer, and subs.size() <= ty.val
 *     notice that there is a 1-level dereference by default
 *     
 *     when ty.which == pointer_, ident is something like *[...]
 *     but when ty.which == param_pointer_, it is like **[...]
 *     
 *     so in the later case, we use "getptr" instead of "getelemptr" in the first round of dereference
 * 
 *     in the last round of dereference, use "load %ptr" instead of "getelemptr %ptr, 0" for i32
 */
Result koopa_dereference(std::string ident, std::vector<Result> &subs, DataType ty) {
    Result ptr;
    for (int i = 0; i < subs.size(); ++i) {
        Result new_ptr = Reg(RegCount++);
        if (i == 0) {
            if (ty.which == DataType::DataTypeEnum::pointer_) {
                koopa_inst(new_ptr, " = getelemptr @", ident, ", ", subs[i]);
            }
            else {
                Result tmp = Reg(RegCount++);
                koopa_inst(tmp, " = load @", ident);
                koopa_inst(new_ptr, " = getptr ", tmp, ", ", subs[i]);
            }
        }
        else
            koopa_inst(new_ptr, " = getelemptr ", ptr, ", ", subs[i]);
        ptr = new_ptr;
    }
    Result res = Reg(RegCount++);
    if (subs.size() == ty.val) {
        // completely dereferenced
        koopa_inst(res, " = load ", ptr);
    }
    else {
        // partially dereferenced
        if (subs.size() == 0) {
            if (ty.which == DataType::DataTypeEnum::pointer_)
                koopa_inst(res, " = getelemptr @", ident, ", 0");
            else {
                Result tmp = Reg(RegCount++);
                koopa_inst(tmp, " = load @", ident);
                koopa_inst(res, " = getptr ", tmp, ", 0");
            }
        }
        else
            koopa_inst(res, " = getelemptr ", ptr, ", 0");
    }
    return res;
}
void koopa_br(Result cond, std::string label1, std::string label2, std::string next_label) {
    koopa_print("  br ", cond, ", ", label1, ", ", label2);
    koopa_print(next_label, ":");
}
void koopa_jump(std::string dst_label, std::string next_label) {
    koopa_print("  jump ", dst_label);
    koopa_print(next_label, ":");
}
int RetCount = 0;
void koopa_ret(Result res, bool need_label) {
    koopa_print("  ret ", res);
    if (need_label)
        koopa_print("%ret_label", ++RetCount, ":");
}
void koopa_ret(bool need_label) {
    koopa_print("  ret");
    if (need_label)
        koopa_print("%ret_label", ++RetCount, ":");
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

