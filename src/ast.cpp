#include <bits/stdc++.h>

#include "ast.hpp"
#include "util.hpp"

std::ofstream koopa_ofs;
std::unordered_map<std::string, Value> Map[1024]; // symbol table for different depth
int RegCount = 0;
int BlockDepth = 0;
int BlockCount = 0;

void BaseAST::storeValue(Result res) const {
    return;
}

Result CompUnitAST::DumpKoopa() const {
    this->func_def->DumpKoopa();
    return Result();
}

Result FuncDefAST::DumpKoopa() const {
    koopa_ofs << "fun @" << this->ident << "(): ";
    this->func_type->DumpKoopa();
    koopa_ofs << " {\n" << "%entry:\n";
    this->block->DumpKoopa();
    koopa_ofs << "}\n";
    return Result();
}

Result FuncTypeAST::DumpKoopa() const {
    koopa_ofs << this->type;
    return Result();
}

Result BlockAST::DumpKoopa() const {
    ++BlockDepth;
    ++BlockCount;    
    Map[BlockDepth].clear();
    for (auto &ptr: this->block_item) {
        ptr->DumpKoopa();
    }
    --BlockDepth;
    return Result();
}

Result ConstDeclAST::DumpKoopa() const {
    for (auto &ptr: this->const_def) {
        ptr->DumpKoopa();
    }
    return Result();        
}

Result VarDeclAST::DumpKoopa() const {
    for (auto &ptr: this->var_def) {
        ptr->DumpKoopa();
    }
    return Result();
}

Result BTypeAST::DumpKoopa() const {
    return Result();        
}

Result ConstDefAST::DumpKoopa() const {
    assert(Map[BlockDepth].find(this->ident) == Map[BlockDepth].end()); // undefined identifier
    Result res = this->const_init_val->DumpKoopa();
    assert(res.which == Result::ResultEnum::imm); // must be const
    Map[BlockDepth][this->ident] = Const(res.val);
    return Result();        
}

Result VarDefAST::DumpKoopa() const {
    assert(Map[BlockDepth].find(this->ident) == Map[BlockDepth].end()); // undefined identifier
    koopa_ofs << "@" << this->ident << "_" << BlockDepth << " = alloc i32\n";
    if (this->init_val != nullptr) {
        Result res = this->init_val->DumpKoopa();
        koopa_ofs << "store " << res << ", @" << this->ident << "_" << BlockDepth << "\n";
    }
    Map[BlockDepth][this->ident] = Var;
    return Result();
}

Result ConstInitValAST::DumpKoopa() const {
    return this->const_exp->DumpKoopa();
}

Result InitValAST::DumpKoopa() const {
    return this->exp->DumpKoopa();
}

Result ConstExpAST::DumpKoopa() const {
    return this->exp->DumpKoopa();
}

Result StmtAST::DumpKoopa() const {
    if (this->which == StmtAST::StmtEnum::ret_with_value) {
        Result res = this->exp->DumpKoopa();
        koopa_ofs << "ret " << res << "\n";
    }
    else if (this->which == StmtAST::StmtEnum::ret_without_value) {
        koopa_ofs << "ret\n";
    }
    else if (this->which == StmtAST::StmtEnum::assignment) {
        this->lval->storeValue(this->exp->DumpKoopa());
    }
    else if (this->which == StmtAST::StmtEnum::another_block) {
        this->block->DumpKoopa();
    }
    else if (this->which == StmtAST::StmtEnum::exp) {
        this->exp->DumpKoopa();
    }
    return Result();
}

Result LValAST::DumpKoopa() const  {
    for (int d = BlockDepth; d >= 0; --d) {
        auto it = Map[d].find(this->ident);
        if (it != Map[d].end()) {
            if (it->second.which == Value::ValueEnum::const_) {
                return Imm(it->second.val);
            }
            else {
                Result res = Reg(RegCount++);
                koopa_ofs << res << " = load @" << this->ident << "_" << d << "\n";
                return res;
            }
        }
    }
    return Result();
}

void LValAST::storeValue(Result res) const {
    for (int d = BlockDepth; d >= 0; --d) {
        if (Map[d].find(this->ident) != Map[d].end()) {
            koopa_ofs << "store " << res << ", @" << this->ident << "_" << d << "\n";
            return;
        }
    }
}

Result ExpAST::DumpKoopa() const {
    return this->logical_or_exp->DumpKoopa();
}

Result LogicalOrExpAST::DumpKoopa() const {
    if (this->which == LogicalOrExpAST::LogicalOrExpEnum::into_logical_and) {
        return this->logical_and_exp->DumpKoopa();
    }
    Result s1 = this->logical_or_exp->DumpKoopa();
    Result s2 = this->logical_and_exp->DumpKoopa();
    return calc("ne", calc("or", s1, s2), Imm(0));
}

Result LogicalAndExpAST::DumpKoopa() const {
    if (this->which == LogicalAndExpAST::LogicalAndExpEnum::into_eq) {
        return this->eq_exp->DumpKoopa();
    }
    Result s1 = this->logical_and_exp->DumpKoopa();
    Result s2 = this->eq_exp->DumpKoopa();
    return calc("and", calc("ne", s1, Imm(0)), calc("ne", s2, Imm(0)));
}

Result EqExpAST::DumpKoopa() const {
    if (this->which == EqExpAST::EqExpEnum::into_rel) {
        return this->rel_exp->DumpKoopa();
    }
    Result s1 = this->eq_exp->DumpKoopa();
    Result s2 = this->rel_exp->DumpKoopa();
    if (this->which == EqExpAST::EqExpEnum::eq) {
        return calc("eq", s1, s2);
    }
    else {
        return calc("ne", s1, s2);
    }
}

Result RelExpAST::DumpKoopa() const {
    if (this->which == RelExpAST::RelExpEnum::into_add) {
        return this->add_exp->DumpKoopa();
    }
    Result s1 = this->rel_exp->DumpKoopa();
    Result s2 = this->add_exp->DumpKoopa();
    if (this->which == RelExpAST::RelExpEnum::lt) {
        return calc("lt", s1, s2);
    }
    else if (this->which == RelExpAST::RelExpEnum::gt) {
        return calc("gt", s1, s2);
    }
    else if (this->which == RelExpAST::RelExpEnum::le) {
        return calc("le", s1, s2);
    }
    else {
        return calc("ge", s1, s2);
    }
}

Result AddExpAST::DumpKoopa() const {
    if (this->which == AddExpAST::AddExpEnum::into_mul) {
        return this->mul_exp->DumpKoopa();
    }
    Result s1 = this->add_exp->DumpKoopa();
    Result s2 = this->mul_exp->DumpKoopa();
    if (this->which == AddExpAST::AddExpEnum::add) {
        return calc("add", s1, s2);
    }
    else {
        return calc("sub", s1, s2);
    }
}

Result MulExpAST::DumpKoopa() const {
    if (this->which == MulExpAST::MulExpEnum::into_unary) {
        return this->unary_exp->DumpKoopa();
    }
    Result s1 = this->mul_exp->DumpKoopa();
    Result s2 = this->unary_exp->DumpKoopa();
    if (this->which == MulExpAST::MulExpEnum::mul) {
        return calc("mul", s1, s2);
    }
    else if (this->which == MulExpAST::MulExpEnum::div) {
        return calc("div", s1, s2);
    }
    else {
        return calc("mod", s1, s2);
    }
}

Result UnaryExpAST::DumpKoopa() const {
    if (this->which == UnaryExpAST::UnaryExpEnum::into_primary) {
        return this->primary_exp->DumpKoopa();
    }
    if (this->which == UnaryExpAST::UnaryExpEnum::pos) {
        return this->unary_exp->DumpKoopa();
    }
    Result s = this->unary_exp->DumpKoopa();
    if (this->which == UnaryExpAST::UnaryExpEnum::neg) {
        return calc("sub", Imm(0), s);
    }
    else { 
        return calc("eq", Imm(0), s);
    }
}

Result PrimaryExpAST::DumpKoopa() const {
    if (this->which == PrimaryExpAST::PrimaryExpEnum::into_number) {
        return Imm(this->number);
    }
    else if (this->which == PrimaryExpAST::PrimaryExpEnum::into_lval){
        return this->lval->DumpKoopa();
    }
    else {
        return this->exp->DumpKoopa();
    }
}
