#include <bits/stdc++.h>

#include "koopa.h"

#define CHUNKSIZE 64

extern std::ofstream riscv_ofs;
int Offset, curFrameSize;

std::unordered_map<koopa_raw_value_t, int> Map; // map koopa_raw_value_t to offset on stack

void move_stack_pointer(int offset) {
    if (offset > 0) {
        while (offset) {
            riscv_ofs << "    addi sp, sp, " << std::min(1024, offset) << "\n";
            offset -= std::min(1024, offset);
        }
    } else {
        while (offset) {
            riscv_ofs << "    addi sp, sp, " << std::max(-2048, offset) << "\n";
            offset -= std::max(-2048, offset);
        }
    }
}

/* alocate a single value on stack (can be a integer or a pointer) */
void alloc_on_stack(koopa_raw_value_t value) {
    Offset += 4;
    Map[value] = -Offset;
    if (Offset > curFrameSize) {
        int size = ((Offset - curFrameSize - 1) / CHUNKSIZE + 1) * CHUNKSIZE;
        move_stack_pointer(-size);
        curFrameSize += size;
    }
}

/* allocate a piece of space (of size "size") on stack */
int alloc_on_stack(int size) {
    Offset += size;
    if (Offset > curFrameSize) {
        int size = ((Offset - curFrameSize - 1) / CHUNKSIZE + 1) * CHUNKSIZE;
        move_stack_pointer(-size);
        curFrameSize += size;
    }
    return -Offset;
}

/* move "value" to dest_reg 
 * all registers reserved except dest_reg
 */
void move_to_reg(koopa_raw_value_t value, std::string dest_reg) {
    if (Map.find(value) != Map.end()) {
        // value on stack (has allocated already)
        riscv_ofs << "    li " << dest_reg << ", " << Map[value] << "\n";
        riscv_ofs << "    add " << dest_reg << ", " << dest_reg << ", fp\n";
        riscv_ofs << "    lw " << dest_reg << ", (" << dest_reg << ")\n";
        return;
    }
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
        // immediate
        riscv_ofs << "    li " << dest_reg << ", " << value->kind.data.integer.value << "\n";
    }
    else if (value->kind.tag == KOOPA_RVT_FUNC_ARG_REF) {
        // function arg
        koopa_raw_func_arg_ref_t arg = value->kind.data.func_arg_ref;
        if (arg.index < 8) {
            riscv_ofs << "    lw " << dest_reg << ", " << -(int)arg.index * 4 - 12 << "(fp)\n";
        }
        else {
            riscv_ofs << "    lw " << dest_reg << ", " << ((int)arg.index - 8) * 4 << "(fp)\n";
        }
    }
    else if (value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
        // global symbol
        riscv_ofs << "    la " << dest_reg << ", " << value->name + 1 << "\n"; // value->name[0] = '@'
    }
    else {
        printf("move_to_reg tag error ! (%d)\n", value->kind.tag);
        exit(1);
    }
}

/* calculate the size of multi-dimension array */
int calc_size(koopa_raw_type_t ty) {
    if (ty->tag == KOOPA_RTT_INT32) {
        return 4;
    } else if (ty->tag == KOOPA_RTT_ARRAY) {
        return ty->data.array.len * calc_size(ty->data.array.base);
    } else {
        printf("calc_size tag error ! (%d)\n", ty->tag);
        exit(1);
    }
}

/* output global allocation initializer (a list of .word xxx) */
void solve_global_alloc(koopa_raw_value_t value) {
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
        riscv_ofs << "    .word " << value->kind.data.integer.value << "\n";
    } else if (value->kind.tag == KOOPA_RVT_AGGREGATE) {
        koopa_raw_slice_t elems = value->kind.data.aggregate.elems;
        for (int i = 0; i < elems.len; ++i) {
            koopa_raw_value_t val = (koopa_raw_value_t)elems.buffer[i];
            solve_global_alloc(val);
        }
    } else {
        printf("solve_global_alloc tag error ! (%d)\n", value->kind.tag);
        exit(1);
    }
}

/* store return value to a0 and jump to label "return" */
void solve_return(koopa_raw_value_t value, const char * func_name) {
    koopa_raw_return_t ret = value->kind.data.ret;
    if (ret.value != nullptr)
        move_to_reg(ret.value, "a0");
    riscv_ofs << "    j func_return_" << func_name << "\n";
}

/* implement binary operation and store the result on the stack */
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
    riscv_ofs << "    li t1, " << Map[value] << "\n";
    riscv_ofs << "    add t1, t1, fp\n";
    riscv_ofs << "    sw t0, (t1)\n";
}

/* allocate local variables
 * notice that "value" here is a pointer
 * just allocate two pieces of sapce on stack: one for data and one for the "value" pointer
 */
void solve_alloc(koopa_raw_value_t value) {
    assert(value->ty->tag == KOOPA_RTT_POINTER);
    koopa_raw_type_t base = value->ty->data.pointer.base;
    int offset;
    switch (base->tag) {
        case KOOPA_RTT_INT32:
        case KOOPA_RTT_POINTER:
            offset = alloc_on_stack(4);
            alloc_on_stack(value);
            riscv_ofs << "    li t0, " << offset << "\n";
            riscv_ofs << "    add t0, t0, fp\n";
            riscv_ofs << "    li t1, " << Map[value] << "\n";
            riscv_ofs << "    add t1, t1, fp\n";
            riscv_ofs << "    sw t0, (t1)\n";
            return;
        case KOOPA_RTT_ARRAY:
            offset = alloc_on_stack(calc_size(base));
            alloc_on_stack(value);
            riscv_ofs << "    li t0, " << offset << "\n";
            riscv_ofs << "    add t0, t0, fp\n";
            riscv_ofs << "    li t1, " << Map[value] << "\n";
            riscv_ofs << "    add t1, t1, fp\n";
            riscv_ofs << "    sw t0, (t1)\n";
            return;
        default:
            printf("solve_alloc tag error ! (%d)\n", base->tag);
            exit(1);
    }
}

/* load an single value from a pointer (to an integer / a pointer)
 * load.src is a pointer to a single value (an integer / a pointer)
 */
void solve_load(koopa_raw_value_t value) {
    koopa_raw_load_t load = value->kind.data.load;
    assert(load.src->ty->tag == KOOPA_RTT_POINTER);
    assert(load.src->ty->data.pointer.base->tag == KOOPA_RTT_INT32 || load.src->ty->data.pointer.base->tag == KOOPA_RTT_POINTER);
    move_to_reg(load.src, "t0");
    riscv_ofs << "    lw t0, (t0)\n";
    assert(value->ty->tag == KOOPA_RTT_INT32 || value->ty->tag == KOOPA_RTT_POINTER);
    alloc_on_stack(value);
    riscv_ofs << "    li t1, " << Map[value] << "\n";
    riscv_ofs << "    add t1, t1, fp\n";
    riscv_ofs << "    sw t0, (t1)\n";
}

/* store aggregate to dest (t1) */
void solve_store_aggregate(koopa_raw_value_t value, int &cnt) {
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
        riscv_ofs << "    li t0, " << value->kind.data.integer.value << "\n";
        riscv_ofs << "    sw t0, (t1)\n";
        riscv_ofs << "    add t1, t1, 4\n";
        cnt += 4;
        return;
    }
    assert(value->kind.tag == KOOPA_RVT_AGGREGATE);
    koopa_raw_slice_t elems = value->kind.data.aggregate.elems;
    for (int i = 0; i < elems.len; ++i) {
        koopa_raw_value_t val = (koopa_raw_value_t)elems.buffer[i];
        solve_store_aggregate(val, cnt);
    }
}

/* store, value can be single or aggregate */
void solve_store(koopa_raw_value_t value) {
    koopa_raw_store_t store = value->kind.data.store;
    assert(store.dest->ty->tag == KOOPA_RTT_POINTER);
    move_to_reg(store.dest, "t1");
    if (store.value->kind.tag == KOOPA_RVT_AGGREGATE) {
        assert(store.dest->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY);
        int offset = 0;
        solve_store_aggregate(store.value, offset);
        assert(offset == calc_size(store.dest->ty->data.pointer.base));
    } else {
        assert(store.dest->ty->data.pointer.base->tag == KOOPA_RTT_INT32 || store.dest->ty->data.pointer.base->tag == KOOPA_RTT_POINTER);
        move_to_reg(store.value, "t0");
        riscv_ofs << "    sw t0, (t1)\n";
    }
}

/* branch, use bnez & beqz */
void solve_branch(koopa_raw_value_t value) {
    koopa_raw_branch_t branch = value->kind.data.branch;
    move_to_reg(branch.cond, "t0");
    riscv_ofs << "    bnez t0, " << branch.true_bb->name + 1 << "\n";
    riscv_ofs << "    beqz t0, " << branch.false_bb->name + 1 << "\n";
}

/* jump to label */
void solve_jump(koopa_raw_value_t value) {
    koopa_raw_jump_t jump = value->kind.data.jump;
    riscv_ofs << "    j " << jump.target->name + 1 << "\n";
}

/* implement function calls, move all args to a0~7 or the stack, and then store the return value on the stack
 * assume that no function has at least 520 args (otherwise additional_frame_size >= 2048 which can not be represented by 12 bits)
 */
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
    riscv_ofs << "    li t0, " << Map[value] << "\n";
    riscv_ofs << "    add t0, t0, fp\n";
    riscv_ofs << "    sw a0, (t0)\n";
}

/* implement getelemptr
 * src is a pointer to an array
 */
void solve_get_elem_ptr(koopa_raw_value_t value) {
    koopa_raw_get_elem_ptr_t getelemptr = value->kind.data.get_elem_ptr;
    assert(getelemptr.src->ty->tag == KOOPA_RTT_POINTER);
    assert(getelemptr.src->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY);
    move_to_reg(getelemptr.src, "t0");
    move_to_reg(getelemptr.index, "t1");
    int size = calc_size(getelemptr.src->ty->data.pointer.base->data.array.base);
    riscv_ofs << "    li t2, " << size << "\n";
    riscv_ofs << "    mul t1, t1, t2\n";
    riscv_ofs << "    add t0, t0, t1\n";
    alloc_on_stack(value);
    riscv_ofs << "    li t3, " << Map[value] << "\n";
    riscv_ofs << "    add t3, t3, fp\n";
    riscv_ofs << "    sw t0, (t3)\n";
}

void solve_get_ptr(koopa_raw_value_t value) {
    koopa_raw_get_elem_ptr_t getelemptr = value->kind.data.get_elem_ptr;
    assert(getelemptr.src->ty->tag == KOOPA_RTT_POINTER);
    move_to_reg(getelemptr.src, "t0");
    move_to_reg(getelemptr.index, "t1");
    int size = calc_size(getelemptr.src->ty->data.pointer.base);
    riscv_ofs << "    li t2, " << size << "\n";
    riscv_ofs << "    mul t1, t1, t2\n";
    riscv_ofs << "    add t0, t0, t1\n";
    alloc_on_stack(value);
    riscv_ofs << "    li t3, " << Map[value] << "\n";
    riscv_ofs << "    add t3, t3, fp\n";
    riscv_ofs << "    sw t0, (t3)\n";
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
        solve_global_alloc(init);
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
        move_stack_pointer(-curFrameSize);

        for (int j = 0; j < func->bbs.len; ++j) {

            assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
            koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t)func->bbs.buffer[j];

            if (bb->name) 
                riscv_ofs << bb->name + 1 << ":\n";

            for (int k = 0; k < bb->insts.len; ++k){

                koopa_raw_value_t value = (koopa_raw_value_t)bb->insts.buffer[k];

                if (value->kind.tag == KOOPA_RVT_RETURN) {
                    solve_return(value, func->name + 1);
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
                if (value->kind.tag == KOOPA_RVT_GET_ELEM_PTR) {
                    solve_get_elem_ptr(value);
                }
                if (value->kind.tag == KOOPA_RVT_GET_PTR) {
                    solve_get_ptr(value);
                }
            }
        }

        /* epilogue */
        riscv_ofs << "func_return_" << func->name + 1 << ":\n";
        move_stack_pointer(curFrameSize);
        riscv_ofs << "    mv sp, fp\n";
        riscv_ofs << "    lw fp, -8(sp)\n";
        riscv_ofs << "    lw ra, -4(sp)\n";
        riscv_ofs << "    ret\n\n";
    }

    koopa_delete_raw_program_builder(builder);
}