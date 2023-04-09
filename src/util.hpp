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

class DataType {
public:
    enum class DataTypeEnum {const_, var_, pointer_, param_pointer_, func_int, func_void} which;
    int val;
    DataType();
    DataType(DataTypeEnum which_, int const_val_);
};

#define Const(v)        (DataType(DataType::DataTypeEnum::const_, (v)))
#define Var             (DataType(DataType::DataTypeEnum::var_, 0))
#define Pointer(k)      (DataType(DataType::DataTypeEnum::pointer_, (k)))
#define ParamPointer(k) (DataType(DataType::DataTypeEnum::param_pointer_, (k)))
#define FuncInt         (DataType(DataType::DataTypeEnum::func_int, 0))
#define FuncVoid        (DataType(DataType::DataTypeEnum::func_void, 0))

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
Result koopa_dereference(std::string ident, std::vector<Result> &subs, DataType ty);
void koopa_ret(Result res);
void koopa_ret();

Result calc(std::string oprand, Result s1, Result s2);
