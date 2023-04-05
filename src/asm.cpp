#include <bits/stdc++.h>

#include "koopa.h"

#define CHUNKSIZE 32

extern std::ofstream riscv_ofs;
int Offset, curFrameSize;

std::unordered_map<koopa_raw_value_t, int> Map; // map koopa_raw_value_t to offset on stack

void alloc_on_stack(koopa_raw_value_t value) {
    if (Offset == curFrameSize) {
        riscv_ofs << "    addi sp, sp, " << -CHUNKSIZE << "\n";
        curFrameSize += CHUNKSIZE;
    }
    Offset += 4;
    Map[value] = -Offset;
}

void move_to_reg(koopa_raw_value_t value, std::string dest_reg) {
    if (Map.find(value) != Map.end()) {
        riscv_ofs << "    lw " << dest_reg << ", " << Map[value] << "(fp)\n";
        return;
    }
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
        riscv_ofs << "    li " << dest_reg << ", " << value->kind.data.integer.value << "\n";
    }
    else if (value->kind.tag == KOOPA_RVT_FUNC_ARG_REF) {
        koopa_raw_func_arg_ref_t arg = value->kind.data.func_arg_ref;
        if (arg.index < 8) {
            riscv_ofs << "    lw " << dest_reg << ", " << -(int)arg.index * 4 - 12 << "(fp)\n";
        }
        else {
            riscv_ofs << "    lw " << dest_reg << ", " << ((int)arg.index - 8) * 4 << "(fp)\n";
        }
    }
    else if (value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
        riscv_ofs << "    la t0, " << value->name + 1 << "\n";
        riscv_ofs << "    lw t0, (t0)\n";
    }
    else {
        assert(false);
    }
}

void solve_return(koopa_raw_value_t value, int func_id) {
    koopa_raw_return_t ret = value->kind.data.ret;
    if (ret.value != nullptr)
        move_to_reg(ret.value, "a0");
    riscv_ofs << "    j FuncReturn" << func_id << "\n";
}

void solve_binary(koopa_raw_value_t value) {
    koopa_raw_binary_t binary = value->kind.data.binary;
    move_to_reg(binary.lhs, "t0");
    move_to_reg(binary.rhs, "t1");
    switch (binary.op) {
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
    alloc_on_stack(value);
    riscv_ofs << "    sw t0, " << Map[value] << "(fp)\n";
}

void solve_alloc(koopa_raw_value_t value) {
    // do nothing
}

void solve_load(koopa_raw_value_t value) {
    koopa_raw_load_t load = value->kind.data.load;
    if (Map.find(load.src) == Map.end()) {
        move_to_reg(load.src, "t0");
        alloc_on_stack(load.src);
        riscv_ofs << "    sw t0, " << Map[load.src] << "(fp)\n";
    }
    Map[value] = Map[load.src];
}

void solve_branch(koopa_raw_value_t value) {
    koopa_raw_branch_t branch = value->kind.data.branch;
    move_to_reg(branch.cond, "t0");
    riscv_ofs << "    bnez t0, " << branch.true_bb->name + 1 << "\n";
    riscv_ofs << "    beqz t0, " << branch.false_bb->name + 1 << "\n";
}

void solve_jump(koopa_raw_value_t value) {
    koopa_raw_jump_t jump = value->kind.data.jump;
    riscv_ofs << "    j " << jump.target->name + 1 << "\n";
}

void solve_store(koopa_raw_value_t value) {
    koopa_raw_store_t store = value->kind.data.store;
    move_to_reg(store.value, "t0");
    if (store.dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
        riscv_ofs << "    la t1, " << store.dest->name + 1 << "\n";
        riscv_ofs << "    sw t0, (t1)\n";
    }
    else {
        if (Map.find(store.dest) == Map.end()) {
            alloc_on_stack(store.dest);
        }
        riscv_ofs << "    sw t0, " << Map[store.dest] << "(fp)\n";
    }
}

void solve_call(koopa_raw_value_t value) {
    koopa_raw_call_t call = value->kind.data.call;
    int additional_frame_size = 0;
    if (call.args.len > 8)
        additional_frame_size = ((((int)call.args.len - 8) * 4 - 1) / CHUNKSIZE + 1) * CHUNKSIZE;
    if (additional_frame_size)
        riscv_ofs << "    add sp, sp, -" << additional_frame_size << "\n";
    for (int i = 0; i < call.args.len; ++i) {
        koopa_raw_value_t arg = (koopa_raw_value_t)call.args.buffer[i];
        if (i < 8) {
            move_to_reg(arg, "a" + std::to_string(i));
        }
        else {
            move_to_reg(arg, "t0");
            riscv_ofs << "    sw t0, " << (i - 8) * 4 << "(sp)\n";
        }
    }
    riscv_ofs << "    call " << call.callee->name + 1 << "\n";
    if (additional_frame_size)
        riscv_ofs << "    add sp, sp, " << additional_frame_size << "\n";
    alloc_on_stack(value);
    riscv_ofs << "    sw a0, " << Map[value] << "(fp)\n";
}

void parse_koopa(const char* str) {

    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(str, &program);
    assert(ret == KOOPA_EC_SUCCESS);
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    koopa_delete_program(program);
  

    // global variables
    if (raw.values.len)
        riscv_ofs << "    .data\n";
    for (int i = 0; i < raw.values.len; ++i) {

        assert(raw.values.kind == KOOPA_RSIK_VALUE);
        koopa_raw_value_t value = (koopa_raw_value_t)raw.values.buffer[i];

        riscv_ofs << "    .globl " << value->name + 1 << "\n";
        riscv_ofs << value->name + 1 << ":\n";

        koopa_raw_value_t init = value->kind.data.global_alloc.init;

        assert(init->kind.tag == KOOPA_RVT_INTEGER);
        riscv_ofs << "    .word " << init->kind.data.integer.value << "\n\n";
    }

    // functions
    for (int i = 0; i < raw.funcs.len; ++i) {

        assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
        koopa_raw_function_t func = (koopa_raw_function_t)raw.funcs.buffer[i];

        if (func->bbs.len == 0) {
            // function declaration
            continue;
        }

        riscv_ofs << "    .text\n";
        riscv_ofs << "    .globl " << func->name + 1 << "\n"; // func->name[0] = '@'
        riscv_ofs << func->name + 1 << ":\n";

        /* prologue */
        Offset = 8 + std::min((int)func->params.len, 8) * 4;
        curFrameSize = ((Offset - 1) / CHUNKSIZE + 1) * CHUNKSIZE;
        Map.clear();
        riscv_ofs << "    sw ra, -4(sp)\n";
        riscv_ofs << "    sw fp, -8(sp)\n";
        for (int j = 0; j < std::min((int)func->params.len, 8); ++j)
            riscv_ofs << "    sw a" << j << "," << -j * 4 - 12 << "(sp)\n";
        riscv_ofs << "    mv fp, sp\n";
        riscv_ofs << "    addi sp, sp, " << -curFrameSize << "\n";

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
                if (value->kind.tag == KOOPA_RVT_CALL) {
                    solve_call(value);
                }
            }
        }

        /* epilogue */
        riscv_ofs << "FuncReturn" << i << ":\n";
        riscv_ofs << "    addi sp, sp, " << curFrameSize << "\n";
        riscv_ofs << "    mv sp, fp\n";
        riscv_ofs << "    lw fp, -8(sp)\n";
        riscv_ofs << "    lw ra, -4(sp)\n";
        riscv_ofs << "    ret\n\n";
    }

    koopa_delete_raw_program_builder(builder);
}