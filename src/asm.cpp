#include <bits/stdc++.h>

#include "koopa.h"

extern std::ofstream riscv_ofs;
int Offset;

std::unordered_map<koopa_raw_value_t, int> Map; // map koopa_raw_value_t to offset on stack

void move_to_reg(koopa_raw_value_t value, std::string dest_reg) {
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
        riscv_ofs << "    li " << dest_reg << ", " << value->kind.data.integer.value << "\n";
    } else {
        riscv_ofs << "    lw " << dest_reg << ", " << Map[value] << "(sp)\n";
    }
}

void solve_return(koopa_raw_value_t value, int i) {
    move_to_reg(value->kind.data.ret.value, "a0");
    riscv_ofs << "    j FuncReturn" << i << "\n";
}

void solve_binary(koopa_raw_value_t value) {
    move_to_reg(value->kind.data.binary.lhs, "t0");
    move_to_reg(value->kind.data.binary.rhs, "t1");
    switch (value->kind.data.binary.op) {
        case KOOPA_RBO_NOT_EQ:
            riscv_ofs << "    xor t0, t0, t1\n";
            riscv_ofs << "    snez t0, t0\n";
            break;
        case KOOPA_RBO_EQ:
            riscv_ofs << "    xor t0, t0, t1\n";
            riscv_ofs << "    seqz t0, t0\n";
            break;
        case KOOPA_RBO_GT:
            riscv_ofs << "    slt t0, t1, t0\n";
            break;
        case KOOPA_RBO_LT:
            riscv_ofs << "    slt t0, t0, t1\n";
            break;
        case KOOPA_RBO_GE:
            riscv_ofs << "    slt t0, t0, t1\n";
            riscv_ofs << "    xori t0, t0, 1\n";
            break;
        case KOOPA_RBO_LE:
            riscv_ofs << "    slt t0, t1, t0\n";
            riscv_ofs << "    xori t0, t0, 1\n";
            break;
        case KOOPA_RBO_ADD:
            riscv_ofs << "    add t0, t0, t1\n";
            break;
        case KOOPA_RBO_SUB:
            riscv_ofs << "    sub t0, t0, t1\n";
            break;
        case KOOPA_RBO_MUL:
            riscv_ofs << "    mul t0, t0, t1\n";
            break;
        case KOOPA_RBO_DIV:
            riscv_ofs << "    div t0, t0, t1\n";
            break;
        case KOOPA_RBO_MOD:
            riscv_ofs << "    rem t0, t0, t1\n";
            break;
        case KOOPA_RBO_AND:
            riscv_ofs << "    and t0, t0, t1\n";
            break;
        case KOOPA_RBO_OR:
            riscv_ofs << "    or t0, t0, t1\n";
            break;
        case KOOPA_RBO_XOR:
            riscv_ofs << "    xor t0, t0, t1\n";
            break;
    }
    riscv_ofs << "    sw t0, " << Offset << "(sp)\n";
    Map[value] = Offset;
    Offset += 4;
}

void solve_alloc(koopa_raw_value_t value) {
    // do nothing
}

void solve_load(koopa_raw_value_t value) {
    Map[value] = Map[value->kind.data.load.src];
}

void solve_branch(koopa_raw_value_t value) {
    move_to_reg(value->kind.data.branch.cond, "t0");
    riscv_ofs << "    bnez t0, " << value->kind.data.branch.true_bb->name + 1 << "\n";
    riscv_ofs << "    beqz t0, " << value->kind.data.branch.false_bb->name + 1 << "\n";
}

void solve_jump(koopa_raw_value_t value) {
    riscv_ofs << "    j " << value->kind.data.jump.target->name + 1 << "\n";    
}

void solve_store(koopa_raw_value_t value) {
    move_to_reg(value->kind.data.store.value, "t0");
    if (Map.find(value->kind.data.store.dest) == Map.end()) {
        Map[value->kind.data.store.dest] = Offset;
        Offset += 4;
    }
    riscv_ofs << "    sw t0, " << Map[value->kind.data.store.dest] << "(sp)\n";
}

void parse_koopa(const char* str) {

    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(str, &program);
    assert(ret == KOOPA_EC_SUCCESS);
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    koopa_delete_program(program);
  
    riscv_ofs << "    .text\n";

    for (int i = 0; i < raw.funcs.len; ++i) {

        assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
        koopa_raw_function_t func = (koopa_raw_function_t) raw.funcs.buffer[i];
      
        riscv_ofs << "    .globl " << func->name + 1 << "\n"; // func->name[0] = '@'
        riscv_ofs << func->name + 1 << ":\n";

        riscv_ofs << "    addi sp, sp, -256\n";
        Offset = 0;

        for (int j = 0; j < func->bbs.len; ++j) {

            assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
            koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t)func->bbs.buffer[j];
            if (bb->name)
                riscv_ofs << bb->name + 1 << ":\n";

            for (int k = 0; k < bb->insts.len; ++k){

                koopa_raw_value_t value = (koopa_raw_value_t)bb->insts.buffer[k];

                if (value->kind.tag == KOOPA_RVT_RETURN) {
                    solve_return(value, i);
                }
                if (value->kind.tag == KOOPA_RVT_BINARY) {
                    solve_binary(value);
                }
                if (value->kind.tag == KOOPA_RVT_ALLOC) {
                    solve_alloc(value);
                }
                if (value->kind.tag == KOOPA_RVT_LOAD) {
                    solve_load(value);
                }
                if (value->kind.tag == KOOPA_RVT_STORE) {
                    solve_store(value);
                }
                if (value->kind.tag == KOOPA_RVT_BRANCH) {
                    solve_branch(value);
                }
                if (value->kind.tag == KOOPA_RVT_JUMP) {
                    solve_jump(value);
                }
            }
        }
        riscv_ofs << "FuncReturn" << i << ":\n";
        riscv_ofs << "    addi sp, sp, 256\n";
        riscv_ofs << "    ret\n\n";
    }

    koopa_delete_raw_program_builder(builder);
}