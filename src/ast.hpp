#pragma once

#include "util.hpp"

/* declarations */
class BaseAST;
class ProgramAST;
class FuncDefAST;
class TypeAST;
class FuncParamAST;
class BlockAST;
class ConstDeclAST;
class VarDeclAST;
class ConstDefAST;
class VarDefAST;
class InitValAST;
class StmtAST;
class LValAST;
class ExpAST;
class LogicalOrExpAST;
class LogicalAndExpAST;
class EqExpAST;
class RelExpAST;
class AddExpAST;
class MulExpAST;
class UnaryExpAST;
class PrimaryExpAST;

/* definitions */
class BaseAST {
public:
    virtual void DumpKoopa() const = 0;
    virtual ~BaseAST() = default;
};
class ProgramAST {
public:
    // FuncDef, ConstDecl and VarDecl
    std::vector<std::unique_ptr<BaseAST>> comp_units;
    void DumpKoopa() const;
};
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<TypeAST> func_type;
    std::string ident;
    std::vector<std::unique_ptr<FuncParamAST>> func_params;
    std::unique_ptr<BlockAST> block;
    void DumpKoopa() const override;
};
class TypeAST {
public:
    // i32 or void
    std::string type;
    // return 1 for i32 and 0 for void
    int DumpKoopa() const;
};
class FuncParamAST {
public:
    // type must be i32
    std::string ident;
    std::vector<std::unique_ptr<ExpAST>> subscripts;
    void DumpKoopa() const;
};
class BlockAST {
public:
    // Stmt, ConstDecl and VarDecl
    std::vector<std::unique_ptr<BaseAST>> block_items;
    void DumpKoopa(const std::vector<std::unique_ptr<FuncParamAST>> &func_params = std::vector<std::unique_ptr<FuncParamAST>>()) const;
};
class ConstDeclAST : public BaseAST {
public:
    // type must be i32
    std::vector<std::unique_ptr<ConstDefAST>> const_defs;
    void DumpKoopa() const override;
};
class VarDeclAST : public BaseAST {
public:
    // type must be i32
    std::vector<std::unique_ptr<VarDefAST>> var_defs;
    void DumpKoopa() const override;
};
class ConstDefAST {
public:
    std::string ident;
    std::vector<std::unique_ptr<ExpAST>> subscripts;
    std::unique_ptr<InitValAST> init_val;
    void DumpKoopa() const;
};
class VarDefAST {
public:
    std::string ident;
    std::vector<std::unique_ptr<ExpAST>> subscripts;
    std::unique_ptr<InitValAST> init_val;
    void DumpKoopa() const;
};
class InitValAST {
public:
    enum class InitValEnum {single, list} which;
    std::unique_ptr<ExpAST> exp;
    std::vector<std::unique_ptr<InitValAST>> init_vals;
    void DumpKoopa(int *array_len, int k, Result *buffer) const;
};
class StmtAST : public BaseAST{
public:
    enum class StmtEnum {assign, if_, while_, break_, continue_, ret, another_block, exp, empty} which;
    std::unique_ptr<LValAST> lval;
    std::unique_ptr<ExpAST> exp;
    std::unique_ptr<StmtAST> then_stmt, else_stmt;
    std::unique_ptr<BlockAST> block;
    void DumpKoopa() const override;
};
class LValAST {
public:
    std::string ident;
    std::vector<std::unique_ptr<ExpAST>> subscripts;
    Result DumpKoopa() const;
    void StoreValue(Result res) const;
};
class ExpAST {
public:
    std::unique_ptr<LogicalOrExpAST> logical_or_exp;
    Result DumpKoopa() const;
};
class LogicalOrExpAST {
public:
    enum class LogicalOrExpEnum {into_logical_and, logical_or} which;
    std::unique_ptr<LogicalOrExpAST> logical_or_exp;
    std::unique_ptr<LogicalAndExpAST> logical_and_exp;
    Result DumpKoopa() const;
};
class LogicalAndExpAST {
public:
    enum class LogicalAndExpEnum {into_eq, logical_and} which;
    std::unique_ptr<LogicalAndExpAST> logical_and_exp;
    std::unique_ptr<EqExpAST> eq_exp;
    Result DumpKoopa() const;
};
class EqExpAST {
public:
    enum class EqExpEnum {into_rel, eq, ne} which;
    std::unique_ptr<EqExpAST> eq_exp;
    std::unique_ptr<RelExpAST> rel_exp;
    Result DumpKoopa() const;
};
class RelExpAST {
public:
    enum class RelExpEnum {into_add, lt, gt, le, ge} which;
    std::unique_ptr<RelExpAST> rel_exp;
    std::unique_ptr<AddExpAST> add_exp;
    Result DumpKoopa() const;
};
class AddExpAST {
public:
    enum class AddExpEnum {into_mul, add, minus} which;
    std::unique_ptr<AddExpAST> add_exp;
    std::unique_ptr<MulExpAST> mul_exp;
    Result DumpKoopa() const;
};
class MulExpAST {
public:
    enum class MulExpEnum {into_unary, mul, div, rem} which;
    std::unique_ptr<MulExpAST> mul_exp;
    std::unique_ptr<UnaryExpAST> unary_exp;
    Result DumpKoopa() const;
};
class UnaryExpAST {
public:
    enum class UnaryExpEnum {into_primary, pos, neg, logical_neg, func_call} which;
    std::unique_ptr<UnaryExpAST> unary_exp;
    std::unique_ptr<PrimaryExpAST> primary_exp;
    std::string ident;
    std::vector<std::unique_ptr<ExpAST>> call_params;
    Result DumpKoopa() const;
};
class PrimaryExpAST {
public:
    enum class PrimaryExpEnum {into_number, into_lval, another_exp} which;
    std::unique_ptr<ExpAST> exp;
    std::unique_ptr<LValAST> lval;
    int number;
    Result DumpKoopa() const;
};

template<typename... Ts>
void koopa_inst(Ts... args);

void koopa_basic_block(std::string label);
void koopa_ret(Result res);
void koopa_ret();

Result calc(std::string oprand, Result s1, Result s2);