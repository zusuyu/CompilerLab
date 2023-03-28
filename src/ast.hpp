#pragma once

#include <bits/stdc++.h>

class Result {
public:
    enum class ResultEnum {imm, reg} which;
    int val;
    Result() {}
    Result(ResultEnum which_, int val_): which(which_), val(val_) {}
    friend std::ostream & operator << (std::ostream &ofs, Result res) {
        if (res.which == Result::ResultEnum::reg)
            ofs << '%';
        ofs << res.val;
        return ofs;
    }
};


class BaseAST {
public:
    virtual ~BaseAST() = default;
    virtual Result DumpKoopa(std::ofstream &ofs) const = 0;
};

class CompUnitAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_def;
    Result DumpKoopa(std::ofstream &ofs) const override;
};

class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
    Result DumpKoopa(std::ofstream &ofs) const override;
};

class FuncTypeAST : public BaseAST {
public:
    std::string type;
    Result DumpKoopa(std::ofstream &ofs) const override;
};

class BlockAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> stmt;
    Result DumpKoopa(std::ofstream &ofs) const override;
};

class StmtAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;
    Result DumpKoopa(std::ofstream &ofs) const override;
};

class ExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> logical_or_exp;
    Result DumpKoopa(std::ofstream &ofs) const override;
};

class LogicalOrExpAST : public BaseAST {
public:
    enum class LogicalOrExpEnum {into_logical_and, logical_or} which;
    std::unique_ptr<BaseAST> logical_or_exp, logical_and_exp;
    Result DumpKoopa(std::ofstream &ofs) const override;
};

class LogicalAndExpAST : public BaseAST {
public:
    enum class LogicalAndExpEnum {into_eq, logical_and} which;
    std::unique_ptr<BaseAST> logical_and_exp, eq_exp;
    Result DumpKoopa(std::ofstream &ofs) const override;
};

class EqExpAST : public BaseAST {
public:
    enum class EqExpEnum {into_rel, eq, ne} which;
    std::unique_ptr<BaseAST> eq_exp, rel_exp;
    Result DumpKoopa(std::ofstream &ofs) const override;
};

class RelExpAST : public BaseAST {
public:
    enum class RelExpEnum {into_add, lt, gt, le, ge} which;
    std::unique_ptr<BaseAST> eq_exp, rel_exp;
    Result DumpKoopa(std::ofstream &ofs) const override;
};

class AddExpAST : public BaseAST {
public:
    enum class AddExpEnum {into_mul, add, minus} which;
    std::unique_ptr<BaseAST> add_exp, mul_exp;
    Result DumpKoopa(std::ofstream &ofs) const override;
};

class MulExpAST : public BaseAST {
public:
    enum class MulExpEnum {into_unary, mul, div, rem} which;
    std::unique_ptr<BaseAST> mul_exp, unary_exp;
    Result DumpKoopa(std::ofstream &ofs) const override;
};

class UnaryExpAST : public BaseAST {
public:
    enum class UnaryExpEnum {into_primary, pos, neg, logical_neg} which;
    std::unique_ptr<BaseAST> unary_exp, primary_exp;
    char unary_op;
    Result DumpKoopa(std::ofstream &ofs) const override;
};

class PrimaryExpAST : public BaseAST {
public:
    enum class PrimaryExpEnum {into_number, another_exp} which;
    std::unique_ptr<BaseAST> exp;
    int number;
    Result DumpKoopa(std::ofstream &ofs) const override;
};