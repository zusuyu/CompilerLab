%code requires {
  #include <memory>
  #include <string>
  #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>

#include "ast.hpp"

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
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Exp UnaryExp PrimaryExp MulExp AddExp RelExp EqExp LogicalAndExp LogicalOrExp
%type <int_val> Number

%%

CompUnit:
  FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  };

FuncDef: 
  FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  };

FuncType:
  INT {
    auto ast = new FuncTypeAST();
    ast->type = "i32";
    $$ = ast;
  };

Block:
 '{' Stmt '}' {
    auto ast = new BlockAST();
    ast->stmt = unique_ptr<BaseAST>($2);
    $$ = ast;
  };

Stmt: 
  RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  };

Exp:
  AddExp {
    auto ast = new ExpAST();
    ast->logical_or_exp = unique_ptr<BaseAST>($1);
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
  };

PrimaryExp:
  Number {
    auto ast = new PrimaryExpAST();
    ast->which = PrimaryExpAST::PrimaryExpEnum::into_number;
    ast->number = $1;
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
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}