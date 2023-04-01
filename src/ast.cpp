#include <bits/stdc++.h>
#include "ast.hpp"


static int RegCount = 0;

#define OP(d, oprand, s1, s2) \
    ofs << d << " = " << oprand << " " << s1 << ", " << s2 << "\n";


Result CompUnitAST::DumpKoopa(std::ofstream &ofs) const {
    this->func_def->DumpKoopa(ofs);
    return Result();
}

Result FuncDefAST::DumpKoopa(std::ofstream &ofs) const {
    ofs << "fun @" << this->ident << "(): ";
    this->func_type->DumpKoopa(ofs);
    ofs << " {\n" << "%entry:\n";
    this->block->DumpKoopa(ofs);
    ofs << "}\n";
    return Result();
}

Result FuncTypeAST::DumpKoopa(std::ofstream &ofs) const {
    ofs << this->type;
    return Result();
}

Result BlockAST::DumpKoopa(std::ofstream &ofs) const {
    this->stmt->DumpKoopa(ofs);
    return Result();
}

Result StmtAST::DumpKoopa(std::ofstream &ofs) const {
    Result res = this->exp->DumpKoopa(ofs);
    ofs << "ret " << res << "\n";
    return Result();
}

Result ExpAST::DumpKoopa(std::ofstream &ofs) const {
    return this->logical_or_exp->DumpKoopa(ofs);
}

Result LogicalOrExpAST::DumpKoopa(std::ofstream &ofs) const {
    if (this->which == LogicalOrExpAST::LogicalOrExpEnum::into_logical_and) {
        return this->logical_and_exp->DumpKoopa(ofs);
    }
    Result s1 = this->logical_or_exp->DumpKoopa(ofs);
    Result s2 = this->logical_and_exp->DumpKoopa(ofs);
    Result d1 = Result(Result::ResultEnum::reg, RegCount++);
    Result d2 = Result(Result::ResultEnum::reg, RegCount++);
    OP(d1, "or", s1, s2);
    OP(d2, "ne", d1, "0");
    return d2;
}

Result LogicalAndExpAST::DumpKoopa(std::ofstream &ofs) const {
    if (this->which == LogicalAndExpAST::LogicalAndExpEnum::into_eq) {
        return this->eq_exp->DumpKoopa(ofs);
    }
    Result s1 = this->logical_and_exp->DumpKoopa(ofs);
    Result s2 = this->eq_exp->DumpKoopa(ofs);
    Result d1 = Result(Result::ResultEnum::reg, RegCount++);
    Result d2 = Result(Result::ResultEnum::reg, RegCount++);
    Result d3 = Result(Result::ResultEnum::reg, RegCount++);
    OP(d1, "ne", s1, "0");
    OP(d2, "ne", s2, "0");
    OP(d3, "and", d1, d2);
    return d3;
}

Result EqExpAST::DumpKoopa(std::ofstream &ofs) const {
    if (this->which == EqExpAST::EqExpEnum::into_rel) {
        return this->rel_exp->DumpKoopa(ofs);
    }
    Result s1 = this->eq_exp->DumpKoopa(ofs);
    Result s2 = this->rel_exp->DumpKoopa(ofs);
    Result d = Result(Result::ResultEnum::reg, RegCount++);
    if (this->which == EqExpAST::EqExpEnum::eq) {
        OP(d, "eq", s1, s2);
    } else {
        OP(d, "ne", s1, s2);
    }
    return d;
}

Result RelExpAST::DumpKoopa(std::ofstream &ofs) const {
    if (this->which == RelExpAST::RelExpEnum::into_add) {
        return this->add_exp->DumpKoopa(ofs);
    }
    Result s1 = this->rel_exp->DumpKoopa(ofs);
    Result s2 = this->add_exp->DumpKoopa(ofs);
    Result d = Result(Result::ResultEnum::reg, RegCount++);
    if (this->which == RelExpAST::RelExpEnum::lt) {
        OP(d, "lt", s1, s2);
    } else if (this->which == RelExpAST::RelExpEnum::gt) {
        OP(d, "gt", s1, s2);
    } else if (this->which == RelExpAST::RelExpEnum::le) {
        OP(d, "le", s1, s2);
    } else {
        OP(d, "ge", s1, s2);
    }
    return d;
}

Result AddExpAST::DumpKoopa(std::ofstream &ofs) const {
    if (this->which == AddExpAST::AddExpEnum::into_mul) {
        return this->mul_exp->DumpKoopa(ofs);
    }
    Result s1 = this->add_exp->DumpKoopa(ofs);
    Result s2 = this->mul_exp->DumpKoopa(ofs);
    Result d = Result(Result::ResultEnum::reg, RegCount++);
    if (this->which == AddExpAST::AddExpEnum::add) {
        OP(d, "add", s1, s2);
    } else {
        OP(d, "sub", s1, s2);
    }
    return d;
}

Result MulExpAST::DumpKoopa(std::ofstream &ofs) const {
    if (this->which == MulExpAST::MulExpEnum::into_unary) {
        return this->unary_exp->DumpKoopa(ofs);
    }
    Result s1 = this->mul_exp->DumpKoopa(ofs);
    Result s2 = this->unary_exp->DumpKoopa(ofs);
    Result d = Result(Result::ResultEnum::reg, RegCount++);
    if (this->which == MulExpAST::MulExpEnum::mul) {
        OP(d, "mul", s1, s2);
    } else if (this->which == MulExpAST::MulExpEnum::div) {
        OP(d, "div", s1, s2);
    } else {
        OP(d, "mod", s1, s2);
    }
    return d;
}


Result UnaryExpAST::DumpKoopa(std::ofstream &ofs) const {
    if (this->which == UnaryExpAST::UnaryExpEnum::into_primary) {
        return this->primary_exp->DumpKoopa(ofs);
    }
    if (this->which == UnaryExpAST::UnaryExpEnum::pos) {
        return this->unary_exp->DumpKoopa(ofs);
    }
    Result s = this->unary_exp->DumpKoopa(ofs);
    Result d = Result(Result::ResultEnum::reg, RegCount++);
    if (this->which == UnaryExpAST::UnaryExpEnum::neg) {
        OP(d, "sub", 0, s);
    } else { 
        OP(d, "eq", 0, s);
    }
    return d;
}

Result PrimaryExpAST::DumpKoopa(std::ofstream &ofs) const {
    if (this->which == PrimaryExpAST::PrimaryExpEnum::into_number) {
        return Result(Result::ResultEnum::imm, this->number);
    } else {
        return this->exp->DumpKoopa(ofs);
    }
}