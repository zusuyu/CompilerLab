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
int ArraySize;
int ArrayDimension[1024];


std::string func_params_to_print;
std::vector<std::pair<std::string, Value>> func_params_to_map;

void ProgramAST::DumpKoopa() const {
    /* lib_funcs */
    koopa_ofs << "decl @getint(): i32\n";
    koopa_ofs << "decl @getch(): i32\n";
    koopa_ofs << "decl @getarray(*i32): i32\n";
    koopa_ofs << "decl @putint(i32)\n";
    koopa_ofs << "decl @putch(i32)\n";
    koopa_ofs << "decl @putarray(i32, *i32)\n";
    koopa_ofs << "decl @starttime()\n";
    koopa_ofs << "decl @stoptime()\n\n";
    Map[0]["getint"] = Map[0]["getch"] = Map[0]["getarray"] = FuncInt;
    Map[0]["putint"] = Map[0]["putch"] = Map[0]["putarray"] = Map[0]["starttime"] = Map[0]["stoptime"] = FuncVoid;

    for (auto &ptr: this->comp_units) {
        ptr->DumpKoopa();
    }
}

void FuncDefAST::DumpKoopa() const {
    koopa_ofs << "fun @" << this->ident << "(";
    for (auto it = this->func_params.begin(); it != this->func_params.end(); ++it) {
        if (it != this->func_params.begin())
            koopa_ofs << ", ";
        (*it)->DumpKoopa();
    }
    koopa_ofs << ")";
    if (this->func_type->DumpKoopa() == 0)
        Map[0][this->ident] = FuncVoid;
    else
        Map[0][this->ident] = FuncInt;
    koopa_ofs << " {\n";
    koopa_basic_block("entry_" + this->ident);
    this->block->DumpKoopa();
    if (!BasicBlockEnds) {
        // add a "ret" instruction for unreturned void function
        assert(Map[0][this->ident].which == Value::ValueEnum::func_void);
        koopa_ret();
    }
    koopa_ofs << "\n}\n\n";
}

int TypeAST::DumpKoopa() const {
    koopa_ofs << this->type;
    return this->type == "" ? 0 : 1;
}

void FuncParamAST::DumpKoopa() const {
    koopa_ofs << "%" << this->ident;
    this->type->DumpKoopa();
    func_params_to_print += "  @" + WRAP(this->ident, BlockCount + 1) + " = alloc i32\n";
    func_params_to_print += "  store %" + this->ident + ", @" + WRAP(this->ident, BlockCount + 1) + "\n";
    func_params_to_map.push_back(std::make_pair(this->ident, Var));
}

void BlockAST::DumpKoopa() const {
    if (BasicBlockEnds)
        return;
    BlockID[++BlockDepth] = ++BlockCount;
    Map[BlockDepth].clear();
    koopa_ofs << func_params_to_print;
    func_params_to_print = "";
    for (auto &ptr: func_params_to_map) {
        Map[BlockDepth][ptr.first] = ptr.second;
    }
    func_params_to_map.clear();
    for (auto &ptr: this->block_items) {
        ptr->DumpKoopa();
    }
    --BlockDepth;
}

void ConstDeclAST::DumpKoopa() const {
    for (auto &ptr: this->const_defs) {
        ptr->DumpKoopa();
    }
}

void VarDeclAST::DumpKoopa() const {
    for (auto &ptr: this->var_defs) {
        ptr->DumpKoopa();
    }
}

void DumpKoopa_Array(const std::string &ident, const std::vector<std::unique_ptr<ExpAST>> &subscripts, const std::unique_ptr<InitValAST> &init_val) {
    int *len = new int [subscripts.size()];
    int tot = 1, k = 0;
    for (auto &ptr: subscripts) {
        Result res = ptr->DumpKoopa();
        assert(res.which == Result::ResultEnum::imm && res.val >= 1);
        len[k++] = res.val;
        tot *= res.val;
    }
    Result *buffer = new Result [tot];
    if (init_val != nullptr)
        init_val->DumpKoopa(len, k, buffer);
    if (BlockDepth == 0){
        // global constant array
        for (int i = 0; i < tot; ++i)
            assert(buffer[i].which == Result::ResultEnum::imm);
        koopa_ofs << "global @" << WRAP(ident, BlockID[BlockDepth]) << " = alloc ";
        koopa_array_type(len, k);
        koopa_ofs << ", ";
        koopa_aggregate(len, k, buffer);
        koopa_ofs << "\n\n";
    }
    else {
        // local constant array
        for (int i = 0; i < tot; ++i)
            assert(buffer[i].which == Result::ResultEnum::imm);
        koopa_ofs << "  @" << WRAP(ident, BlockID[BlockDepth]) << " = alloc ";
        koopa_array_type(len, k);
        koopa_ofs << "\n  store ";
        koopa_aggregate(len, k, buffer);
        koopa_ofs << ", @" << WRAP(ident, BlockID[BlockDepth]) << "\n";
    }
    Map[BlockDepth][ident] = Pointer;
    delete [] buffer;
    delete [] len;    
}

void ConstDefAST::DumpKoopa() const {
    assert(Map[BlockDepth].find(this->ident) == Map[BlockDepth].end()); // undefined identifier
    if (this->subscripts.size() == 0) {
        // global & local constant
        assert(this->init_val->which == InitValAST::InitValEnum::single);
        Result res = this->init_val->exp->DumpKoopa();
        assert(res.which == Result::ResultEnum::imm);
        Map[BlockDepth][this->ident] = Const(res.val);
    }
    else {
        DumpKoopa_Array(this->ident, this->subscripts, this->init_val);
    }
}

void VarDefAST::DumpKoopa() const {
    assert(Map[BlockDepth].find(this->ident) == Map[BlockDepth].end()); // undefined identifier
    if (this->subscripts.size() == 0) {
        Result res = Imm(0);
        if (BlockDepth == 0) {
            // global variable
            if (this->init_val != nullptr) {
                assert(this->init_val->which == InitValAST::InitValEnum::single);
                res = this->init_val->exp->DumpKoopa();
                assert(res.which == Result::ResultEnum::imm);
            }
            koopa_ofs << "global @" << WRAP(this->ident, BlockID[BlockDepth]) << " = alloc i32, " << res.val << "\n\n";
        }
        else {
            // local variable
            koopa_inst("@", WRAP(this->ident, BlockID[BlockDepth]), " = alloc i32");
            if (this->init_val != nullptr) {
                res = this->init_val->exp->DumpKoopa();
                koopa_inst("store ", res, ", @", WRAP(this->ident, BlockID[BlockDepth]));
            }
        }
        Map[BlockDepth][this->ident] = Var;
    }
    else {
        DumpKoopa_Array(this->ident, this->subscripts, this->init_val);        
    }
}

/*
 * initialize a k-dimension array of size len[0] * len[1] * ... * len[k - 1]
 * and write them into buffer
 */
void InitValAST::DumpKoopa(int *len, int k, Result *buffer) const {
    assert(this->which == InitValAST::InitValEnum::list);
    int *suffix_prod = new int [k + 1];
    suffix_prod[k] = 1;
    for (int i = k - 1; i >= 0; --i)
        suffix_prod[i] = suffix_prod[i + 1] * len[i];
    int cnt = 0;
    for (auto &ptr: this->init_vals) {
        if (ptr->which == InitValAST::InitValEnum::single) {
            buffer[cnt++] = ptr->exp->DumpKoopa();
        }
        else {
            int p = k;
            while (p > 1 && cnt % suffix_prod[p - 1] == 0)
                --p;
            // k - p is subarray's dimension, which is <= k - 1
            ptr->DumpKoopa(len + p, k - p, buffer + cnt);
            cnt += suffix_prod[p];
        }
        if (cnt == suffix_prod[0])
            break;
    }
    delete [] suffix_prod;
}

void StmtAST::DumpKoopa() const {
    if (BasicBlockEnds)
        return;
    if (this->which == StmtAST::StmtEnum::assign) {
        this->lval->StoreValue(this->exp->DumpKoopa());
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
}

Result LValAST::DumpKoopa() const  {
    for (int d = BlockDepth; d >= 0; --d) {
        auto it = Map[d].find(this->ident);
        if (it != Map[d].end()) {
            if (it->second.which == Value::ValueEnum::const_) {
                return Imm(it->second.const_val);
            }
            else if (it->second.which == Value::ValueEnum::var_) {
                Result res = Reg(RegCount++);
                koopa_inst(res, " = load @", WRAP(this->ident, BlockID[d]));
                return res;
            }
            else {
                // pointer
                assert(it->second.which == Value::ValueEnum::pointer_);
                assert(this->subscripts.size() > 0);
                Result ptr;
                for (int i = 0; i < this->subscripts.size(); ++i) {
                    Result sub = this->subscripts[i]->DumpKoopa();
                    Result new_ptr = Reg(RegCount++);
                    if (i == 0)
                        koopa_inst(new_ptr, " = getelemptr @", WRAP(this->ident, BlockID[d]), ", ", sub);
                    else
                        koopa_inst(new_ptr, " = getelemptr ", ptr, ", ", sub);
                    ptr = new_ptr;
                }
                Result res = Reg(RegCount++);
                koopa_inst(res, " = load ", ptr);
                return res;
            }
        }
    }
    assert(0);
}

void LValAST::StoreValue(Result res) const {
    for (int d = BlockDepth; d >= 0; --d) {
        auto it = Map[d].find(this->ident);
        if (it != Map[d].end()) {
            if (it->second.which == Value::ValueEnum::var_) {
                koopa_inst("store ", res, ", @", WRAP(this->ident, BlockID[d]));
            }
            else {
                assert(it->second.which == Value::ValueEnum::pointer_);
                assert(this->subscripts.size() > 0);
                Result ptr;
                for (int i = 0; i < this->subscripts.size(); ++i) {
                    Result sub = this->subscripts[i]->DumpKoopa();
                    Result new_ptr = Reg(RegCount++);
                    if (i == 0)
                        koopa_inst(new_ptr, " = getelemptr @", WRAP(this->ident, BlockID[d]), ", ", sub);
                    else
                        koopa_inst(new_ptr, " = getelemptr ", ptr, ", ", sub);
                    ptr = new_ptr;
                }
                koopa_inst("store ", res, ", ", ptr);
            }
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
    if (this->which == UnaryExpAST::UnaryExpEnum::func_call) {
        assert(!BasicBlockEnds);
        std::vector<Result> ress;
        for (auto &ptr: this->call_params) {
            ress.push_back(ptr->DumpKoopa());
        }
        if (Map[0][this->ident].which == Value::ValueEnum::func_int) {
            Result res = Reg(RegCount++);
            koopa_ofs << "  " << res << " = call @" << this->ident << "(";
            for (auto it = ress.begin(); it != ress.end(); ++it) {
                if (it != ress.begin())
                    koopa_ofs << ", ";
                koopa_ofs << *it;
            }
            koopa_ofs << ")\n";
            return res;
        }
        else {
            koopa_ofs << "  call @" << this->ident << "(";
            for (auto it = ress.begin(); it != ress.end(); ++it) {
                if (it != ress.begin())
                    koopa_ofs << ", ";
                koopa_ofs << *it;
            }
            koopa_ofs << ")\n";
            return Result();
        }
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
