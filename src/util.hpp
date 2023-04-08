#pragma once

#include <bits/stdc++.h>

class Result {
public:
    enum class ResultEnum {imm, reg} which;
    int val;
    Result();
    Result(ResultEnum which_, int val_);
    friend std::ostream & operator << (std::ostream &ofs, Result res);
};

#define Imm(v) (Result(Result::ResultEnum::imm, (v)))
#define Reg(v) (Result(Result::ResultEnum::reg, (v)))

class Value {
public:
    enum class ValueEnum {const_, var_, pointer_, func_int, func_void} which;
    int const_val;
    Value();
    Value(ValueEnum which_, int const_val_);
};

#define Const(v) (Value(Value::ValueEnum::const_, (v)))
#define Var      (Value(Value::ValueEnum::var_, 0))
#define Pointer  (Value(Value::ValueEnum::pointer_, 0))
#define FuncInt  (Value(Value::ValueEnum::func_int, 0))
#define FuncVoid (Value(Value::ValueEnum::func_void, 0))

extern std::ofstream koopa_ofs;
extern bool BasicBlockEnds;

#define WRAP(ident, sub) ((ident) + "_" + std::to_string(sub))

template<typename T>
void koopa_print(T arg) {
    koopa_ofs << arg << "\n";
}
template<typename T, typename... Ts>
void koopa_print(T arg0, Ts... args) {
    koopa_ofs << arg0;
    koopa_print(args...);
}
template<typename... Ts>
void koopa_inst(Ts... args) {
    if (!BasicBlockEnds)
        koopa_print("  ", args...);
}
void koopa_basic_block(std::string label);
void koopa_array_type(int *len, int k);
void koopa_aggregate(int *len, int k, Result *buffer);
void koopa_ret(Result res);
void koopa_ret();

Result calc(std::string oprand, Result s1, Result s2);
