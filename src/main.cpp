#include <bits/stdc++.h>

#include "ast.hpp"
#include "asm.hpp"
#include "koopa.h"

extern FILE *yyin;
extern int yyparse(std::unique_ptr<BaseAST> &ast);

std::ofstream koopa_ofs;
std::ofstream riscv_ofs;
static char koopaIR[1 << 20];

void help() {
    std::cout << "usage: ./compiler -koopa input_file -o output_file" << std::endl
              << "       ./compiler -riscv input_file -o output_file" << std::endl;
}

int main(int argc, const char *argv[]) {

    if (argc != 5) {
        help();
        return 0;
    }

    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];

    yyin = fopen(input, "r");
    if (!yyin) {
        std::cout << "Error: failed to open " << input << std::endl;
        return 0;
    }

    std::unique_ptr<BaseAST> ast;
    if (yyparse(ast)) {
        std::cout << "Error: failed to parse " << std::endl;
    }

    if (strcmp(mode, "-koopa") == 0) {
        koopa_ofs = std::ofstream(output);
        ast->DumpKoopa();
        koopa_ofs.close();
    }
    else if (strcmp(mode, "-riscv") == 0) {
        koopa_ofs = std::ofstream("IgnoreMe.koopa");
        ast->DumpKoopa();
        koopa_ofs.close();
        std::ifstream koopa_ifs("IgnoreMe.koopa");
        koopa_ifs.read(koopaIR, 1 << 20);
        riscv_ofs = std::ofstream(output);
        parse_koopa(koopaIR);
    }
    else {
        help();
    }

    return 0;
}