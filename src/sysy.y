%code requires {
  #include <memory>
  #include <string>
  #include "ast.hpp"
  #include "util.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>

#include "ast.hpp"
#include "util.hpp"

// 声明 lexer 函数和错误处理函数
int yylex();
template<typename T>
void yyerror(T &ast, const char *s);

using namespace std;

%}

%parse-param { std::unique_ptr<ProgramAST> &ast }

%union {
  std::string *str_val;
  int int_val;
  BaseAST *base_val;
  FuncDefAST *func_def_val;
  TypeAST *type_val;
  FuncParamAST *func_param_val;
  BlockAST *block_val;
  ConstDeclAST *const_decl_val;
  ConstDefAST *const_def_val;
  VarDeclAST *var_decl_val;
  VarDefAST *var_def_val;
  InitValAST *init_val_val;
  StmtAST *stmt_val;
  LValAST *lval_val;
  ExpAST *exp_val;
  LogicalOrExpAST *logical_or_exp_val;
  LogicalAndExpAST *logical_and_exp_val;
  EqExpAST *eq_exp_val;
  RelExpAST *rel_exp_val;
  AddExpAST *add_exp_val;
  MulExpAST *mul_exp_val;
  UnaryExpAST *unary_exp_val;
  PrimaryExpAST *primary_exp_val;
  std::vector<std::unique_ptr<BaseAST>> *base_vec_val;
  std::vector<std::unique_ptr<FuncParamAST>> *func_param_vec_val;
  std::vector<std::unique_ptr<ConstDefAST>> *const_def_vec_val;
  std::vector<std::unique_ptr<VarDefAST>> *var_def_vec_val;
  std::vector<std::unique_ptr<ExpAST>> *exp_vec_val;
  std::vector<std::unique_ptr<InitValAST>> *init_val_vec_val;
}

%token INT VOID RETURN CONST IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT
%token <int_val> INT_CONST

%type <int_val> Number
%type <base_val> CompUnit BlockItem
%type <func_def_val> FuncDef
%type <type_val> Type
%type <func_param_val> FuncParam
%type <block_val> Block
%type <const_decl_val> ConstDecl
%type <const_def_val> ConstDef
%type <var_decl_val> VarDecl
%type <var_def_val> VarDef
%type <init_val_val> InitVal
%type <stmt_val> Stmt
%type <lval_val> LVal
%type <exp_val> Exp
%type <logical_or_exp_val> LogicalOrExp
%type <logical_and_exp_val> LogicalAndExp
%type <eq_exp_val> EqExp
%type <rel_exp_val> RelExp
%type <add_exp_val> AddExp
%type <mul_exp_val> MulExp
%type <unary_exp_val> UnaryExp
%type <primary_exp_val> PrimaryExp
%type <base_vec_val> MoreCompUnits MoreBlockItems
%type <func_param_vec_val> MoreFuncParams
%type <const_def_vec_val> MoreConstDefs
%type <var_def_vec_val> MoreVarDefs
%type <exp_vec_val> MoreSubscripts MoreCallParams
%type <init_val_vec_val> MoreInitVals

%%

Program:
  MoreCompUnits CompUnit {
    auto ast_ = make_unique<ProgramAST>();
    ($1)->push_back(unique_ptr<BaseAST>($2));
    ast_->comp_units = std::move(*($1));
    ast = std::move(ast_);
  };

MoreCompUnits:
  MoreCompUnits CompUnit {
    ($1)->push_back(unique_ptr<BaseAST>($2));
    $$ = $1;
  } | 
  %empty {
    $$ = new vector<unique_ptr<BaseAST>>();
  };

CompUnit:
  FuncDef {
    $$ = $1;
  } |
  ConstDecl {
    $$ = $1;
  } | 
  VarDecl {
    $$ = $1;
  };

FuncDef: 
  Type IDENT '(' FuncParam MoreFuncParams ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<TypeAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ($5)->insert(($5)->begin(), unique_ptr<FuncParamAST>($4));
    ast->func_params = std::move(*($5));
    ast->block = unique_ptr<BlockAST>($7);
    $$ = ast;
  } |
  Type IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<TypeAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->func_params = std::move(*(new vector<unique_ptr<FuncParamAST>>()));
    ast->block = unique_ptr<BlockAST>($5);
    $$ = ast;
  };


Type:
  INT {
    auto ast = new TypeAST();
    ast->type = ": i32";
    $$ = ast;
  } |
  VOID {
    auto ast = new TypeAST();
    ast->type = "";
    $$ = ast;
  };

MoreFuncParams:
  MoreFuncParams ',' FuncParam {
    ($1)->push_back(unique_ptr<FuncParamAST>($3));
    $$ = $1;
  } | 
  %empty {
    $$ = new vector<unique_ptr<FuncParamAST>>();
  };

FuncParam:
  Type IDENT {
    auto ast = new FuncParamAST();
    ast->ident = *unique_ptr<string>($2);
    ast->subscripts = vector<unique_ptr<ExpAST>>();
    $$ = ast;
  } |
  Type IDENT '[' ']' MoreSubscripts {
    auto ast = new FuncParamAST();
    ast->ident = *unique_ptr<string>($2);
    ($5)->insert(($5)->begin(), nullptr);
    ast->subscripts = std::move(*($5));
    $$ = ast;
  };

Block:
 '{' MoreBlockItems '}' {
    auto ast = new BlockAST();
    ast->block_items = std::move(*($2));
    $$ = ast;
  };

MoreBlockItems:
  MoreBlockItems BlockItem {
    ($1)->push_back(unique_ptr<BaseAST>($2));
    $$ = $1;
  } | 
  %empty {
    $$ = new vector<unique_ptr<BaseAST>>();
  };

BlockItem:
  ConstDecl {
    $$ = $1;
  } | 
  VarDecl {
    $$ = $1;
  } |
  Stmt {
    $$ = $1;
  };

ConstDecl:
  CONST Type ConstDef MoreConstDefs ';' {
    auto ast = new ConstDeclAST();
    ($4)->insert(($4)->begin(), unique_ptr<ConstDefAST>($3));
    ast->const_defs = std::move(*($4));
    $$ = ast;
  };

VarDecl:
  Type VarDef MoreVarDefs ';' {
    auto ast = new VarDeclAST();
    ($3)->insert(($3)->begin(), unique_ptr<VarDefAST>($2));
    ast->var_defs = std::move(*($3));
    $$ = ast;
  };

MoreConstDefs:
  MoreConstDefs ',' ConstDef {
    ($1)->push_back(unique_ptr<ConstDefAST>($3));
    $$ = $1;
  } | 
  %empty {
    $$ = new vector<unique_ptr<ConstDefAST>>();
  };

ConstDef:
  IDENT MoreSubscripts '=' InitVal {
    auto ast = new ConstDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->subscripts = std::move(*($2));
    ast->init_val = unique_ptr<InitValAST>($4);
    $$ = ast;
  };

MoreVarDefs:
  MoreVarDefs ',' VarDef {
    ($1)->push_back(unique_ptr<VarDefAST>($3));
    $$ = $1;
  } | 
  %empty {
    $$ = new vector<unique_ptr<VarDefAST>>();
  };

VarDef:
  IDENT MoreSubscripts '=' InitVal {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->subscripts = std::move(*($2));
    ast->init_val = unique_ptr<InitValAST>($4);
    $$ = ast;
  } | 
  IDENT MoreSubscripts {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->subscripts = std::move(*($2));
    ast->init_val = nullptr;
    $$ = ast;
  };

MoreSubscripts:
  MoreSubscripts '[' Exp ']' {
    ($1)->push_back(unique_ptr<ExpAST>($3));
    $$ = $1;
  } |
  %empty {
    $$ = new vector<unique_ptr<ExpAST>>();
  };

InitVal:
  Exp {
    auto ast = new InitValAST();
    ast->which = InitValAST::InitValEnum::single;
    ast->exp = unique_ptr<ExpAST>($1);
    $$ = ast;
  } |
  '{' '}' {
    auto ast = new InitValAST();
    ast->which = InitValAST::InitValEnum::list;
    ast->init_vals = vector<unique_ptr<InitValAST>>();
    $$ = ast;
  } |
  '{' InitVal MoreInitVals '}' {
    auto ast = new InitValAST();
    ast->which = InitValAST::InitValEnum::list;
    ($3)->insert(($3)->begin(), unique_ptr<InitValAST>($2));
    ast->init_vals = std::move(*($3));
    $$ = ast;
  };

MoreInitVals:
  MoreInitVals ',' InitVal {
    ($1)->push_back(unique_ptr<InitValAST>($3));
    $$ = $1;
  } |
  %empty {
    $$ = new vector<unique_ptr<InitValAST>>();
  };

Stmt: 
  LVal '=' Exp ';' {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::assign;
    ast->lval = unique_ptr<LValAST>($1);
    ast->exp = unique_ptr<ExpAST>($3);
    $$ = ast;
  } |
  IF '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::if_;
    ast->exp = unique_ptr<ExpAST>($3);
    ast->then_stmt = unique_ptr<StmtAST>($5);
    ast->else_stmt = nullptr;
    $$ = ast;
  } |
  IF '(' Exp ')' Stmt ELSE Stmt {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::if_;
    ast->exp = unique_ptr<ExpAST>($3);
    ast->then_stmt = unique_ptr<StmtAST>($5);
    ast->else_stmt = unique_ptr<StmtAST>($7);
    $$ = ast;
  } |
  WHILE '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::while_;
    ast->exp = unique_ptr<ExpAST>($3);
    ast->then_stmt = unique_ptr<StmtAST>($5);
    $$ = ast;
  } |
  BREAK ';' {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::break_;
    $$ = ast;
  } |
  CONTINUE ';' {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::continue_;
    $$ = ast;
  } |
  RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::ret;
    ast->exp = unique_ptr<ExpAST>($2);
    $$ = ast;
  } | 
  RETURN ';' {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::ret;
    ast->exp = nullptr;
    $$ = ast;
  } |
  Block {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::another_block;
    ast->block = unique_ptr<BlockAST>($1);
    $$ = ast;
  } |
  Exp ';' {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::exp;
    ast->exp = unique_ptr<ExpAST>($1);
    $$ = ast;
  } |
  ';' {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::empty;
    $$ = ast;
  };

LVal:
  IDENT MoreSubscripts {
    auto ast = new LValAST();
    ast->ident = *unique_ptr<string>($1);
    ast->subscripts = std::move(*($2));
    $$ = ast;
  };

Exp:
  LogicalOrExp {
    auto ast = new ExpAST();
    ast->logical_or_exp = unique_ptr<LogicalOrExpAST>($1);
    $$ = ast;
  };

LogicalOrExp:
  LogicalAndExp {
    auto ast = new LogicalOrExpAST();
    ast->which = LogicalOrExpAST::LogicalOrExpEnum::into_logical_and;
    ast->logical_and_exp = unique_ptr<LogicalAndExpAST>($1);
    $$ = ast;
  } | 
  LogicalOrExp '|' '|' LogicalAndExp {
    auto ast = new LogicalOrExpAST();
    ast->which = LogicalOrExpAST::LogicalOrExpEnum::logical_or;
    ast->logical_or_exp = unique_ptr<LogicalOrExpAST>($1);
    ast->logical_and_exp = unique_ptr<LogicalAndExpAST>($4);
    $$ = ast;
  };

LogicalAndExp:
  EqExp {
    auto ast = new LogicalAndExpAST();
    ast->which = LogicalAndExpAST::LogicalAndExpEnum::into_eq;
    ast->eq_exp = unique_ptr<EqExpAST>($1);
    $$ = ast;
  } | 
  LogicalAndExp '&' '&' EqExp {
    auto ast = new LogicalAndExpAST();
    ast->which = LogicalAndExpAST::LogicalAndExpEnum::logical_and;
    ast->logical_and_exp = unique_ptr<LogicalAndExpAST>($1);
    ast->eq_exp = unique_ptr<EqExpAST>($4);
    $$ = ast;
  };

EqExp:
  RelExp {
    auto ast = new EqExpAST();
    ast->which = EqExpAST::EqExpEnum::into_rel;
    ast->rel_exp = unique_ptr<RelExpAST>($1);
    $$ = ast;
  } | 
  EqExp '=' '=' RelExp {
    auto ast = new EqExpAST();
    ast->which = EqExpAST::EqExpEnum::eq;
    ast->eq_exp = unique_ptr<EqExpAST>($1);
    ast->rel_exp = unique_ptr<RelExpAST>($4);
    $$ = ast;
  } |
  EqExp '!' '=' RelExp {
    auto ast = new EqExpAST();
    ast->which = EqExpAST::EqExpEnum::ne;
    ast->eq_exp = unique_ptr<EqExpAST>($1);
    ast->rel_exp = unique_ptr<RelExpAST>($4);
    $$ = ast;
  };

RelExp:
  AddExp {
    auto ast = new RelExpAST();
    ast->which = RelExpAST::RelExpEnum::into_add;
    ast->add_exp = unique_ptr<AddExpAST>($1);
    $$ = ast;
  } | 
  RelExp '<' AddExp {
    auto ast = new RelExpAST();
    ast->which = RelExpAST::RelExpEnum::lt;
    ast->rel_exp = unique_ptr<RelExpAST>($1);
    ast->add_exp = unique_ptr<AddExpAST>($3);
    $$ = ast;
  } |
  RelExp '>' AddExp {
    auto ast = new RelExpAST();
    ast->which = RelExpAST::RelExpEnum::gt;
    ast->rel_exp = unique_ptr<RelExpAST>($1);
    ast->add_exp = unique_ptr<AddExpAST>($3);
    $$ = ast;
  } |
  RelExp '<' '=' AddExp {
    auto ast = new RelExpAST();
    ast->which = RelExpAST::RelExpEnum::le;
    ast->rel_exp = unique_ptr<RelExpAST>($1);
    ast->add_exp = unique_ptr<AddExpAST>($4);
    $$ = ast;
  } |
  RelExp '>' '=' AddExp {
    auto ast = new RelExpAST();
    ast->which = RelExpAST::RelExpEnum::ge;
    ast->rel_exp = unique_ptr<RelExpAST>($1);
    ast->add_exp = unique_ptr<AddExpAST>($4);
    $$ = ast;
  };

AddExp:
  MulExp {
    auto ast = new AddExpAST();
    ast->which = AddExpAST::AddExpEnum::into_mul;
    ast->mul_exp = unique_ptr<MulExpAST>($1);
    $$ = ast;
  } | 
  AddExp '+' MulExp {
    auto ast = new AddExpAST();
    ast->which = AddExpAST::AddExpEnum::add;
    ast->add_exp = unique_ptr<AddExpAST>($1);
    ast->mul_exp = unique_ptr<MulExpAST>($3);
    $$ = ast;
  } |
  AddExp '-' MulExp {
    auto ast = new AddExpAST();
    ast->which = AddExpAST::AddExpEnum::minus;
    ast->add_exp = unique_ptr<AddExpAST>($1);
    ast->mul_exp = unique_ptr<MulExpAST>($3);
    $$ = ast;
  };


MulExp:
  UnaryExp {
    auto ast = new MulExpAST();
    ast->which = MulExpAST::MulExpEnum::into_unary;
    ast->unary_exp = unique_ptr<UnaryExpAST>($1);
    $$ = ast;
  } | 
  MulExp '*' UnaryExp {
    auto ast = new MulExpAST();
    ast->which = MulExpAST::MulExpEnum::mul;
    ast->mul_exp = unique_ptr<MulExpAST>($1);
    ast->unary_exp = unique_ptr<UnaryExpAST>($3);
    $$ = ast;
  } |
  MulExp '/' UnaryExp {
    auto ast = new MulExpAST();
    ast->which = MulExpAST::MulExpEnum::div;
    ast->mul_exp = unique_ptr<MulExpAST>($1);
    ast->unary_exp = unique_ptr<UnaryExpAST>($3);
    $$ = ast;
  } |
  MulExp '%' UnaryExp {
    auto ast = new MulExpAST();
    ast->which = MulExpAST::MulExpEnum::rem;
    ast->mul_exp = unique_ptr<MulExpAST>($1);
    ast->unary_exp = unique_ptr<UnaryExpAST>($3);
    $$ = ast;
  };

UnaryExp:
  PrimaryExp {
    auto ast = new UnaryExpAST();
    ast->which = UnaryExpAST::UnaryExpEnum::into_primary;
    ast->primary_exp = unique_ptr<PrimaryExpAST>($1);
    $$ = ast;
  } | 
  '+' UnaryExp {
    auto ast = new UnaryExpAST();
    ast->which = UnaryExpAST::UnaryExpEnum::pos;
    ast->unary_exp = unique_ptr<UnaryExpAST>($2);
    $$ = ast;
  } |
  '-' UnaryExp {
    auto ast = new UnaryExpAST();
    ast->which = UnaryExpAST::UnaryExpEnum::neg;
    ast->unary_exp = unique_ptr<UnaryExpAST>($2);
    $$ = ast;
  } |
  '!' UnaryExp {
    auto ast = new UnaryExpAST();
    ast->which = UnaryExpAST::UnaryExpEnum::logical_neg;
    ast->unary_exp = unique_ptr<UnaryExpAST>($2);
    $$ = ast;
  } |
  IDENT '(' Exp MoreCallParams ')' {
    auto ast = new UnaryExpAST();
    ast->which = UnaryExpAST::UnaryExpEnum::func_call;
    ast->ident = *unique_ptr<string>($1);
    ($4)->insert(($4)->begin(), unique_ptr<ExpAST>($3));
    ast->call_params = std::move(*($4));
    $$ = ast;
  } |
  IDENT '(' ')' {
    auto ast = new UnaryExpAST();
    ast->which = UnaryExpAST::UnaryExpEnum::func_call;
    ast->ident = *unique_ptr<string>($1);
    ast->call_params = std::move(*(new vector<unique_ptr<ExpAST>>()));
    $$ = ast;
  }

MoreCallParams:
  MoreCallParams ',' Exp {
    ($1)->push_back(unique_ptr<ExpAST>($3));
    $$ = $1;
  } | 
  %empty {
    $$ = new vector<unique_ptr<ExpAST>>();
  };

PrimaryExp:
  Number {
    auto ast = new PrimaryExpAST();
    ast->which = PrimaryExpAST::PrimaryExpEnum::into_number;
    ast->number = $1;
    $$ = ast;
  } | 
  LVal {
    auto ast = new PrimaryExpAST();
    ast->which = PrimaryExpAST::PrimaryExpEnum::into_lval;
    ast->lval = unique_ptr<LValAST>($1);
    $$ = ast;
  } |
  '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->which = PrimaryExpAST::PrimaryExpEnum::another_exp;
    ast->exp = unique_ptr<ExpAST>($2);
    $$ = ast;
  }

Number:
  INT_CONST {
    $$ = $1;
  };

%%

template<typename T>
void yyerror(T &ast, const char *s) {  
    extern int yylineno;    // defined and maintained in lex
    extern char *yytext;    // defined and maintained in lex
    fprintf(stderr, "ERROR: %s at symbol '%c' on line %d\n", s, yytext[0], yylineno);    
    for (int i = 0; i < 20; ++i)
      fprintf(stderr, "%c", yytext[i]);
    fprintf(stderr, "\n");
}