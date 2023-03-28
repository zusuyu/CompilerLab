#include <bits/stdc++.h>
#include "ast.hpp"


static int RegCount = 0;

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
    return this->add_exp->DumpKoopa(ofs);
}

Result AddExpAST::DumpKoopa(std::ofstream &ofs) const {
    if (this->which == AddExpAST::AddExpEnum::into_mul) {
        return this->mul_exp->DumpKoopa(ofs);
    }
    Result s1 = this->add_exp->DumpKoopa(ofs);
    Result s2 = this->mul_exp->DumpKoopa(ofs);
    Result d = Result(Result::ResultEnum::reg, RegCount++);
    if (this->which == AddExpAST::AddExpEnum::add) {
        ofs << d << " = add " << s1 << ", " << s2 << "\n";
    } else {
        ofs << d << " = sub " << s1 << ", " << s2 << "\n";
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
        ofs << d << " = mul " << s1 << ", " << s2 << "\n";
    } else if (this->which == MulExpAST::MulExpEnum::div) {
        ofs << d << " = div " << s1 << ", " << s2 << "\n";
    } else {
        ofs << d << " = rem " << s1 << ", " << s2 << "\n";
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
        ofs << d << " = sub 0, " << s << "\n";
    } else { 
        ofs << d << " = eq 0, " << s << "\n";
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