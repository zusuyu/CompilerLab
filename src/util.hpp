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
    enum class ValueEnum {const_, var_} which;
    int val;
    Value();
    Value(ValueEnum which_, int val_);
};

#define Const(v) (Value(Value::ValueEnum::const_, (v)))
#define Var (Value(Value::ValueEnum::var_, 0))

extern std::ofstream koopa_ofs;
extern bool BasicBlockEnds;

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
void koopa_ret(Result res);
void koopa_ret();

Result calc(std::string oprand, Result s1, Result s2);
