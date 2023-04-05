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
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
  std::vector<std::unique_ptr<BaseAST>> *vec_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val

%token INT VOID RETURN CONST IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> CompUnit FuncDef Type FuncParam Block BlockItem 
                ConstDecl ConstDef ConstInitVal ConstExp 
                VarDecl VarDef InitVal
                Stmt LVal
                Exp UnaryExp PrimaryExp MulExp AddExp RelExp EqExp LogicalAndExp LogicalOrExp
%type <int_val> Number
%type <vec_val> MoreCompUnits MoreFuncParams MoreBlockItems MoreConstDefs MoreVarDefs MoreCallParams

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
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ($5)->insert(($5)->begin(), unique_ptr<BaseAST>($4));
    ast->func_params = std::move(*($5));
    ast->block = unique_ptr<BaseAST>($7);
    $$ = ast;
  } |
  Type IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->func_params = std::move(*(new vector<unique_ptr<BaseAST>>()));
    ast->block = unique_ptr<BaseAST>($5);
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
    ($1)->push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  } | 
  %empty {
    $$ = new vector<unique_ptr<BaseAST>>();
  };

FuncParam:
  Type IDENT {
    auto ast = new FuncParamAST();
    ast->type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
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
    ($4)->insert(($4)->begin(), unique_ptr<BaseAST>($3));
    ast->const_defs = std::move(*($4));
    $$ = ast;
  };

VarDecl:
  Type VarDef MoreVarDefs ';' {
    auto ast = new VarDeclAST();
    ($3)->insert(($3)->begin(), unique_ptr<BaseAST>($2));
    ast->var_defs = std::move(*($3));
    $$ = ast;
  };

MoreConstDefs:
  MoreConstDefs ',' ConstDef {
    ($1)->push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  } | 
  %empty {
    $$ = new vector<unique_ptr<BaseAST>>();
  };

ConstDef:
  IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->const_init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  };

MoreVarDefs:
  MoreVarDefs ',' VarDef {
    ($1)->push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  } | 
  %empty {
    $$ = new vector<unique_ptr<BaseAST>>();
  };

VarDef:
  IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  } | 
  IDENT {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->init_val = nullptr;
    $$ = ast;
  };

ConstInitVal:
  ConstExp {
    auto ast = new ConstInitValAST();
    ast->const_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };

ConstExp:
  Exp {
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };

InitVal:
  Exp {
    auto ast = new InitValAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };

Stmt: 
  LVal '=' Exp ';' {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::assign;
    ast->lval = unique_ptr<BaseAST>($1);
    ast->exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  } |
  IF '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::if_;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->then_stmt = unique_ptr<BaseAST>($5);
    ast->else_stmt = nullptr;
    $$ = ast;
  } |
  IF '(' Exp ')' Stmt ELSE Stmt {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::if_;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->then_stmt = unique_ptr<BaseAST>($5);
    ast->else_stmt = unique_ptr<BaseAST>($7);
    $$ = ast;
  } |
  WHILE '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::while_;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->then_stmt = unique_ptr<BaseAST>($5);
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
    ast->exp = unique_ptr<BaseAST>($2);
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
    ast->block = unique_ptr<BaseAST>($1);
    $$ = ast;
  } |
  Exp ';' {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::exp;
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  } |
  ';' {
    auto ast = new StmtAST();
    ast->which = StmtAST::StmtEnum::empty;
    $$ = ast;
  };

LVal:
  IDENT {
    auto ast = new LValAST();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  };

Exp:
  LogicalOrExp {
    auto ast = new ExpAST();
    ast->logical_or_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };

LogicalOrExp:
  LogicalAndExp {
    auto ast = new LogicalOrExpAST();
    ast->which = LogicalOrExpAST::LogicalOrExpEnum::into_logical_and;
    ast->logical_and_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  } | 
  LogicalOrExp '|' '|' LogicalAndExp {
    auto ast = new LogicalOrExpAST();
    ast->which = LogicalOrExpAST::LogicalOrExpEnum::logical_or;
    ast->logical_or_exp = unique_ptr<BaseAST>($1);
    ast->logical_and_exp = unique_ptr<BaseAST>($4);
    $$ = ast;
  };

LogicalAndExp:
  EqExp {
    auto ast = new LogicalAndExpAST();
    ast->which = LogicalAndExpAST::LogicalAndExpEnum::into_eq;
    ast->eq_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  } | 
  LogicalAndExp '&' '&' EqExp {
    auto ast = new LogicalAndExpAST();
    ast->which = LogicalAndExpAST::LogicalAndExpEnum::logical_and;
    ast->logical_and_exp = unique_ptr<BaseAST>($1);
    ast->eq_exp = unique_ptr<BaseAST>($4);
    $$ = ast;
  };

EqExp:
  RelExp {
    auto ast = new EqExpAST();
    ast->which = EqExpAST::EqExpEnum::into_rel;
    ast->rel_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  } | 
  EqExp '=' '=' RelExp {
    auto ast = new EqExpAST();
    ast->which = EqExpAST::EqExpEnum::eq;
    ast->eq_exp = unique_ptr<BaseAST>($1);
    ast->rel_exp = unique_ptr<BaseAST>($4);
    $$ = ast;
  } |
  EqExp '!' '=' RelExp {
    auto ast = new EqExpAST();
    ast->which = EqExpAST::EqExpEnum::ne;
    ast->eq_exp = unique_ptr<BaseAST>($1);
    ast->rel_exp = unique_ptr<BaseAST>($4);
    $$ = ast;
  };

RelExp:
  AddExp {
    auto ast = new RelExpAST();
    ast->which = RelExpAST::RelExpEnum::into_add;
    ast->add_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  } | 
  RelExp '<' AddExp {
    auto ast = new RelExpAST();
    ast->which = RelExpAST::RelExpEnum::lt;
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  } |
  RelExp '>' AddExp {
    auto ast = new RelExpAST();
    ast->which = RelExpAST::RelExpEnum::gt;
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  } |
  RelExp '<' '=' AddExp {
    auto ast = new RelExpAST();
    ast->which = RelExpAST::RelExpEnum::le;
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->add_exp = unique_ptr<BaseAST>($4);
    $$ = ast;
  } |
  RelExp '>' '=' AddExp {
    auto ast = new RelExpAST();
    ast->which = RelExpAST::RelExpEnum::ge;
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->add_exp = unique_ptr<BaseAST>($4);
    $$ = ast;
  };

AddExp:
  MulExp {
    auto ast = new AddExpAST();
    ast->which = AddExpAST::AddExpEnum::into_mul;
    ast->mul_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  } | 
  AddExp '+' MulExp {
    auto ast = new AddExpAST();
    ast->which = AddExpAST::AddExpEnum::add;
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->mul_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  } |
  AddExp '-' MulExp {
    auto ast = new AddExpAST();
    ast->which = AddExpAST::AddExpEnum::minus;
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->mul_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };


MulExp:
  UnaryExp {
    auto ast = new MulExpAST();
    ast->which = MulExpAST::MulExpEnum::into_unary;
    ast->unary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  } | 
  MulExp '*' UnaryExp {
    auto ast = new MulExpAST();
    ast->which = MulExpAST::MulExpEnum::mul;
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  } |
  MulExp '/' UnaryExp {
    auto ast = new MulExpAST();
    ast->which = MulExpAST::MulExpEnum::div;
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  } |
  MulExp '%' UnaryExp {
    auto ast = new MulExpAST();
    ast->which = MulExpAST::MulExpEnum::rem;
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };

UnaryExp:
  PrimaryExp {
    auto ast = new UnaryExpAST();
    ast->which = UnaryExpAST::UnaryExpEnum::into_primary;
    ast->primary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  } | 
  '+' UnaryExp {
    auto ast = new UnaryExpAST();
    ast->which = UnaryExpAST::UnaryExpEnum::pos;
    ast->unary_exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  } |
  '-' UnaryExp {
    auto ast = new UnaryExpAST();
    ast->which = UnaryExpAST::UnaryExpEnum::neg;
    ast->unary_exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  } |
  '!' UnaryExp {
    auto ast = new UnaryExpAST();
    ast->which = UnaryExpAST::UnaryExpEnum::logical_neg;
    ast->unary_exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  } |
  IDENT '(' Exp MoreCallParams ')' {
    auto ast = new UnaryExpAST();
    ast->which = UnaryExpAST::UnaryExpEnum::func_call;
    ast->ident = *unique_ptr<string>($1);
    ($4)->insert(($4)->begin(), unique_ptr<BaseAST>($3));
    ast->call_params = std::move(*($4));
    $$ = ast;
  } |
  IDENT '(' ')' {
    auto ast = new UnaryExpAST();
    ast->which = UnaryExpAST::UnaryExpEnum::func_call;
    ast->ident = *unique_ptr<string>($1);
    ast->call_params = std::move(*(new vector<unique_ptr<BaseAST>>()));
    $$ = ast;
  }

MoreCallParams:
  MoreCallParams ',' Exp {
    ($1)->push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  } | 
  %empty {
    $$ = new vector<unique_ptr<BaseAST>>();
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
    ast->lval = unique_ptr<BaseAST>($1);
    $$ = ast;
  } |
  '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->which = PrimaryExpAST::PrimaryExpEnum::another_exp;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }

Number:
  INT_CONST {
    $$ = $1;
  };

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s) {
  
    extern int yylineno;    // defined and maintained in lex
    extern char *yytext;    // defined and maintained in lex
    fprintf(stderr, "ERROR: %s at symbol '%c' on line %d\n", s, yytext[0], yylineno);    
    for (int i = 0; i < 20; ++i)
      fprintf(stderr, "%c", yytext[i]);
    fprintf(stderr, "\n");
}