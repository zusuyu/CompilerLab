#pragma once

#include "util.hpp"

class BaseAST {
public:
    virtual ~BaseAST() = default;
    virtual Result DumpKoopa() const = 0;
    virtual void storeValue(Result res) const;
};

class CompUnitAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_def;
    Result DumpKoopa() const override;
};

class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
    Result DumpKoopa() const override;
};

class FuncTypeAST : public BaseAST {
public:
    std::string type;
    Result DumpKoopa() const override;
};

class BlockAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST>> block_item; // could be ConstDecl, ValDecl or Stmt
    Result DumpKoopa() const override;
};

class ConstDeclAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> btype;
    std::vector<std::unique_ptr<BaseAST>> const_def;
    Result DumpKoopa() const override;
};

class VarDeclAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> btype;
    std::vector<std::unique_ptr<BaseAST>> var_def;
    Result DumpKoopa() const override;
};

class BTypeAST : public BaseAST {
public:
    std::string type;
    Result DumpKoopa() const override;
};

class ConstDefAST : public BaseAST {
public:
    std::string ident;
    std::unique_ptr<BaseAST> const_init_val;
    Result DumpKoopa() const override;
};

class VarDefAST : public BaseAST {
public:
    std::string ident;
    std::unique_ptr<BaseAST> init_val;
    Result DumpKoopa() const override;
};

class ConstInitValAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> const_exp;
    Result DumpKoopa() const override;
};

class InitValAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;
    Result DumpKoopa() const override;
};

class ConstExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;
    Result DumpKoopa() const override;
};

class StmtAST : public BaseAST {
public:
    enum class StmtEnum {assign, ret} which;
    std::unique_ptr<BaseAST> lval, exp;
    Result DumpKoopa() const override;
};

class LValAST : public BaseAST {
public:
    std::string ident;
    Result DumpKoopa() const override;
    void storeValue(Result res) const override;
};

class ExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> logical_or_exp;
    Result DumpKoopa() const override;
};

class LogicalOrExpAST : public BaseAST {
public:
    enum class LogicalOrExpEnum {into_logical_and, logical_or} which;
    std::unique_ptr<BaseAST> logical_or_exp, logical_and_exp;
    Result DumpKoopa() const override;
};

class LogicalAndExpAST : public BaseAST {
public:
    enum class LogicalAndExpEnum {into_eq, logical_and} which;
    std::unique_ptr<BaseAST> logical_and_exp, eq_exp;
    Result DumpKoopa() const override;
};

class EqExpAST : public BaseAST {
public:
    enum class EqExpEnum {into_rel, eq, ne} which;
    std::unique_ptr<BaseAST> eq_exp, rel_exp;
    Result DumpKoopa() const override;
};

class RelExpAST : public BaseAST {
public:
    enum class RelExpEnum {into_add, lt, gt, le, ge} which;
    std::unique_ptr<BaseAST> rel_exp, add_exp;
    Result DumpKoopa() const override;
};

class AddExpAST : public BaseAST {
public:
    enum class AddExpEnum {into_mul, add, minus} which;
    std::unique_ptr<BaseAST> add_exp, mul_exp;
    Result DumpKoopa() const override;
};

class MulExpAST : public BaseAST {
public:
    enum class MulExpEnum {into_unary, mul, div, rem} which;
    std::unique_ptr<BaseAST> mul_exp, unary_exp;
    Result DumpKoopa() const override;
};

class UnaryExpAST : public BaseAST {
public:
    enum class UnaryExpEnum {into_primary, pos, neg, logical_neg} which;
    std::unique_ptr<BaseAST> unary_exp, primary_exp;
    Result DumpKoopa() const override;
};

class PrimaryExpAST : public BaseAST {
public:
    enum class PrimaryExpEnum {into_number, into_lval, another_exp} which;
    std::unique_ptr<BaseAST> exp, lval;
    int number;
    Result DumpKoopa() const override;
};
