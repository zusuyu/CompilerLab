#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

#include "ast.hpp"

#include "koopa.h"

/*

void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);


void Visit(const koopa_raw_program_t &program) {
    Visit(program.values);
    Visit(program.funcs);
}

void Visit(const koopa_raw_slice_t &slice) {
    for (size_t i = 0; i < slice.len; ++i) {
        auto ptr = slice.buffer[i];
        switch (slice.kind) {
            case KOOPA_RSIK_FUNCTION:
                // visit function
                Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
                break;
            case KOOPA_RSIK_BASIC_BLOCK:
                // visit basic block
                Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
                break;
            case KOOPA_RSIK_VALUE:
                // visit value
                Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
                break;
            default:
                // nothing else so far
                assert(false);
        }
    }
}

void Visit(const koopa_raw_function_t &func) {
    Visit(func->bbs);
}


void Visit(const koopa_raw_basic_block_t &bb) {
    Visit(bb->insts);
}

void Visit(const koopa_raw_value_t &value) {
    const auto &kind = value->kind;
    switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            Visit(kind.data.ret);
            break;
        case KOOPA_RVT_INTEGER:
            Visit(kind.data.integer);
            break;
        default:
            assert(false);
    }
}
*/

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[]) {
    // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
    // compiler 模式 输入文件 -o 输出文件
    assert(argc == 5);
    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];

    yyin = fopen(input, "r");
    assert(yyin);

    unique_ptr<BaseAST> ast;
    auto ret = yyparse(ast);
    assert(!ret);

    if (strcmp(mode, "-koopa") == 0) {

        ofstream fout(output);
        ast->DumpKoopa(fout);
        fout.close();        

    } else if (strcmp(mode, "-riscv") == 0) {
/*
        const char *str = koopaIR.c_str();
        koopa_program_t program;
        koopa_error_code_t ret = koopa_parse_from_string(str, &program);
        assert(ret == KOOPA_EC_SUCCESS);

        koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
        koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
        koopa_delete_program(program);

        Visit(raw);
        
        koopa_delete_raw_program_builder(builder);
*/
    } else {
        cout << "usage: compiler -koopa input_file -o output_file" << endl
                    << "compiler -riscv input_file -o output_file" << endl;
    }

    return 0;
}