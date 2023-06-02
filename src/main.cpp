#include <bits/stdc++.h>

#include "ast.hpp"
#include "asm.hpp"
#include "koopa.h"

extern FILE *yyin;
extern int yyparse(std::unique_ptr<ProgramAST> &ast);

std::ofstream koopa_ofs;
std::ofstream riscv_ofs;

void help() {
    std::cout << "usage: ./compiler -koopa input_file -o output_file" << std::endl
              << "       ./compiler -riscv input_file -o output_file" << std::endl;
}

int main(int argc, const char *argv[]) {
    
    if (argc != 5 || strcmp(argv[3], "-o") != 0) {
        help();
        return 0;
    }

    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];

    if (false) {
        int kase;
        std::ifstream in("/root/compiler/testcases/id.txt");
        in >> kase;
        in.close();
        std::ofstream out("/root/compiler/testcases/id.txt");
        out << kase + 1;
        out.close();
        std::ifstream cpp_ifs(input);
        char *cpp = new char [1 << 20];
        cpp_ifs.read(cpp, 1 << 20);
        std::ofstream cpp_ofs("/root/compiler/testcases/" + std::to_string(kase) + ".c");
        cpp_ofs << cpp; 
        cpp_ofs.close();
        delete [] cpp;
    }

    yyin = fopen(input, "r");
    if (!yyin) {
        std::cout << "Error: failed to open " << input << std::endl;
        return 0;
    }

    std::unique_ptr<ProgramAST> ast;
    if (yyparse(ast)) {
        std::cout << "Error: failed to parse " << std::endl;
        return 0;
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
        char *koopaIR = new char [1 << 20];
        koopa_ifs.read(koopaIR, 1 << 20);
        riscv_ofs = std::ofstream(output);
        parse_koopa(koopaIR);
        delete [] koopaIR;
    }
    else {
        help();
    }

    return 0;
}