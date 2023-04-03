#include <bits/stdc++.h>

#include "ast.hpp"
#include "util.hpp"

extern std::ofstream koopa_ofs;
std::unordered_map<std::string, Value> Map[1024]; // symbol table for different depth
int BlockID[1024];
int WhileID[1024];
int RegCount = 0;
int BlockDepth = 0;
int BlockCount = 0;
int BranchCount = 0;
int WhileDepth = 0;
bool BasicBlockEnds = false;

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
    koopa_ofs << " {\n";
    koopa_basic_block("entry");
    this->block->DumpKoopa();
    koopa_ofs << "}\n";
    return Result();
}

Result FuncTypeAST::DumpKoopa() const {
    koopa_ofs << this->type;
    return Result();
}

Result BlockAST::DumpKoopa() const {
    if (BasicBlockEnds)
        return Result();
    BlockID[++BlockDepth] = ++BlockCount;
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
    koopa_inst("@", this->ident, "_", BlockID[BlockDepth], " = alloc i32");
    if (this->init_val != nullptr) {
        Result res = this->init_val->DumpKoopa();
        koopa_inst("store ", res, ", @", this->ident, "_", BlockID[BlockDepth]);
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
    if (BasicBlockEnds)
        return Result();
    if (this->which == StmtAST::StmtEnum::assign) {
        this->lval->storeValue(this->exp->DumpKoopa());
    }
    else if (this->which == StmtAST::StmtEnum::if_) {
        Result res = this->exp->DumpKoopa();
        int id = ++BranchCount;
        if (this->else_stmt != nullptr) {
            koopa_inst("br ", res, ", %then", id, ", %else", id);
            koopa_basic_block("then" + std::to_string(id));
            this->then_stmt->DumpKoopa();
            koopa_inst("jump %end", id);
            koopa_basic_block("else" + std::to_string(id));
            this->else_stmt->DumpKoopa();
            koopa_inst("jump %end", id);
            koopa_basic_block("end" + std::to_string(id));
        }
        else {
            koopa_inst("br ", res, ", %then", id, ", %end", id);
            koopa_basic_block("then" + std::to_string(id));
            this->then_stmt->DumpKoopa();
            koopa_inst("jump %end", id);
            koopa_basic_block("end" + std::to_string(id));
        }
    }
    else if (this->which == StmtAST::StmtEnum::while_) {
        int id = WhileID[++WhileDepth] = ++BranchCount;
        koopa_inst("jump %while_entry", id);
        koopa_basic_block("while_entry" + std::to_string(id));
        Result res = this->exp->DumpKoopa();
        koopa_inst("br ", res, ", %while_body", id, ", %while_end", id);
        koopa_basic_block("while_body" + std::to_string(id));
        this->then_stmt->DumpKoopa();
        koopa_inst("jump %while_entry", id);
        koopa_basic_block("while_end" + std::to_string(id));
        --WhileDepth;
    }
    else if (this->which == StmtAST::StmtEnum::break_) {
        assert(WhileDepth > 0);
        koopa_inst("jump %while_end", WhileID[WhileDepth]);
        koopa_basic_block("break" + std::to_string(++BranchCount));
    }
    else if (this->which == StmtAST::StmtEnum::continue_) {
        assert(WhileDepth > 0);
        koopa_inst("jump %while_entry", WhileID[WhileDepth]);
        koopa_basic_block("continue" + std::to_string(++BranchCount));
    }
    else if (this->which == StmtAST::StmtEnum::ret) {
        if (this->exp != nullptr) {
            Result res = this->exp->DumpKoopa();
            koopa_ret(res);
        }
        else {
            koopa_ret();
        }
        BasicBlockEnds = true;
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
                koopa_inst(res, " = load @", this->ident, "_", BlockID[d]);
                return res;
            }
        }
    }
    return Result();
}

void LValAST::storeValue(Result res) const {
    for (int d = BlockDepth; d >= 0; --d) {
        if (Map[d].find(this->ident) != Map[d].end()) {
            koopa_inst("store ", res, ", @", this->ident, "_", BlockID[d]);
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
    if (s1.which == Result::ResultEnum::imm) {
        if (s1.val != 0)
            return Imm(1);
        else {
            Result s2 = this->logical_and_exp->DumpKoopa();
            return calc("ne", s2, Imm(0));
        }
    } else {
        int id = ++BranchCount;
        koopa_inst("@result", id, " = alloc i32");
        koopa_inst("br ", s1, ", %then", id, ", %else", id);
        koopa_basic_block("then" + std::to_string(id));
        koopa_inst("store 1, @result", id);
        koopa_inst("jump %end", id);
        koopa_basic_block("else" + std::to_string(id));
        Result s2 = this->logical_and_exp->DumpKoopa();
        s2 = calc("ne", s2, Imm(0));
        koopa_inst("store ", s2, ", @result", id);
        koopa_inst("jump %end", id);
        koopa_basic_block("end" + std::to_string(id));
        Result res = Reg(RegCount++);
        koopa_inst(res, " = load @result", id);
        return res;
    }
}

Result LogicalAndExpAST::DumpKoopa() const {
    if (this->which == LogicalAndExpAST::LogicalAndExpEnum::into_eq) {
        return this->eq_exp->DumpKoopa();
    }
    Result s1 = this->logical_and_exp->DumpKoopa();
    if (s1.which == Result::ResultEnum::imm) {
        if (s1.val == 0)
            return Imm(0);
        else {
            Result s2 = this->logical_and_exp->DumpKoopa();
            return calc("ne", s2, Imm(0));
        }
    } else {
        int id = ++BranchCount;
        koopa_inst("@result", id, " = alloc i32");
        koopa_inst("br ", s1, ", %then", id, ", %else", id);
        koopa_basic_block("then" + std::to_string(id));
        Result s2 = this->eq_exp->DumpKoopa();
        s2 = calc("ne", s2, Imm(0));
        koopa_inst("store ", s2, ", @result", id);
        koopa_inst("jump %end", id);
        koopa_basic_block("else" + std::to_string(id));
        koopa_inst("store 0, @result", id);
        koopa_inst("jump %end", id);
        koopa_basic_block("end" + std::to_string(id));
        Result res = Reg(RegCount++);
        koopa_inst(res, " = load @result", id);
        return res;
    }
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
