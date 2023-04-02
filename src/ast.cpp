#include <bits/stdc++.h>
#include "ast.hpp"

extern std::ofstream koopa_ofs;

static int RegCount = 0;
class Value {
public:
    enum class ValueEnum {const_, var_} which;
    int val;
    Value() {}
    Value(ValueEnum which_, int val_): which(which_), val(val_) {}
};
static std::unordered_map<std::string, Value> Map;

#define Imm(v) (Result(Result::ResultEnum::imm, (v)))
#define Reg(v) (Result(Result::ResultEnum::reg, (v)))

#define Const(v) (Value(Value::ValueEnum::const_, (v)))
#define Var(v) (Value(Value::ValueEnum::var_, (v)))

#define OP(d, oprand, s1, s2) \
    ofs << d << " = " << oprand << " " << s1 << ", " << s2 << "\n";

std::ostream & operator << (std::ostream &ofs, Result res) {
    if (res.which == Result::ResultEnum::reg)
        ofs << '%';
    ofs << res.val;
    return ofs;
}

Result calc(std::string oprand, Result s1, Result s2) {
    if (s1.which == Result::ResultEnum::reg || s2.which == Result::ResultEnum::reg) {
        Result d = Reg(RegCount++);
        koopa_ofs << d << "= " << oprand << " " << s1 << ", " << s2 << "\n";
        return d;
    }
    if (oprand == "ne") {
        return Imm(s1.val != s2.val);
    }
    if (oprand == "eq") {
        return Imm(s1.val == s2.val);
    }
    if (oprand == "gt") {
        return Imm(s1.val > s2.val);
    }
    if (oprand == "lt") {
        return Imm(s1.val < s2.val);
    }
    if (oprand == "ge") {
        return Imm(s1.val >= s2.val);
    }
    if (oprand == "le") {
        return Imm(s1.val <= s2.val);
    }
    if (oprand == "add") {
        return Imm(s1.val + s2.val);
    }
    if (oprand == "sub") {
        return Imm(s1.val - s2.val);
    }
    if (oprand == "mul") {
        return Imm(s1.val * s2.val);
    }
    if (oprand == "div") {
        return Imm(s1.val / s2.val);
    }
    if (oprand == "mod") {
        return Imm(s1.val % s2.val);
    }
    if (oprand == "and") {
        return Imm(s1.val & s2.val);
    }
    if (oprand == "or") {
        return Imm(s1.val | s2.val);
    }
    if (oprand == "xor") {
        return Imm(s1.val ^ s2.val);
    }
    return Result();    
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
    for (auto &ptr: this->block_item) {
        ptr->DumpKoopa();
    }
    return Result();
}

Result ConstDeclAST::DumpKoopa() const {
    for (auto &ptr: this->const_def) {
        ptr->DumpKoopa();
    }
    return Result();
}

Result BTypeAST::DumpKoopa() const {
    return Result();
}

Result ConstDefAST::DumpKoopa() const {
    assert(Map.find(this->ident) == Map.end()); // undefined identifier
    Result res = this->const_init_val->DumpKoopa();
    assert(res.which == Result::ResultEnum::imm); // must be const
    Map[this->ident] = Const(res.val);
    return Result();
}

Result ConstInitValAST::DumpKoopa() const {
    return this->const_exp->DumpKoopa();
}

Result ConstExpAST::DumpKoopa() const {
    return this->exp->DumpKoopa();
}

Result StmtAST::DumpKoopa() const {
    if (this->which == StmtAST::StmtEnum::ret) {
        Result res = this->exp->DumpKoopa();
        koopa_ofs << "ret " << res << "\n";
        return Result();
    } else {
        return Result();
    }
}

Result LValAST::DumpKoopa() const {
    auto it = Map.find(this->ident);
    assert(it != Map.end());
    if (it->second.which == Value::ValueEnum::const_)
        return Imm(it->second.val);
    return Result();
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
    } else {
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
    } else if (this->which == RelExpAST::RelExpEnum::gt) {
        return calc("gt", s1, s2);
    } else if (this->which == RelExpAST::RelExpEnum::le) {
        return calc("le", s1, s2);
    } else {
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
    } else {
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
    } else if (this->which == MulExpAST::MulExpEnum::div) {
        return calc("div", s1, s2);
    } else {
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
    } else { 
        return calc("eq", Imm(0), s);
    }
}

Result PrimaryExpAST::DumpKoopa() const {
    if (this->which == PrimaryExpAST::PrimaryExpEnum::into_number) {
        return Imm(this->number);
    } else if (this->which == PrimaryExpAST::PrimaryExpEnum::into_lval){
        return this->lval->DumpKoopa();
    } else {
        return this->exp->DumpKoopa();
    }
}