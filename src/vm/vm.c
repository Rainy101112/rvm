/*
 *
 *      vm.c
 *
 *      By Rainy101112 2025/8/28
 *      Public under MIT license
 *
 *      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *      IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *      FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "instruction.h"
#include "logger.h"
#include "vm.h"

#include "opcode_impl.h"

/* Inline threaded dispatch (computed goto) where the compiler supports it:
 * the end of each instruction jumps straight to the code of the next one
 * instead of funneling through a central switch, which is far friendlier
 * to branch prediction. A plain switch dispatch is the portable fallback
 * (e.g. MSVC). */
#if defined(__GNUC__) && !defined(RVM_NO_COMPUTED_GOTO)
#define RVM_COMPUTED_GOTO 1
#else
#define RVM_COMPUTED_GOTO 0
#endif

#if RVM_COMPUTED_GOTO
#define VM_CASE(name, OP) vm_op_##name
#define VM_NEXT()  goto vm_op_dispatch
#else
#define VM_CASE(name, OP) case OP_##OP
#define VM_NEXT()  break
#endif

/* Initialize VM. Returns false on invalid configuration or allocation
 * failure; nothing is left allocated in that case. */
bool vm_init(vm_t *vm, uint8_t *code, size_t code_size, size_t memsize) {
    memset(vm->registers, 0, sizeof(vm->registers));

    if (memsize == 0) {
        logger_error("Invalid VM memory size: 0\n");
        return false;
    }

    if (code == NULL && code_size > 0) {
        logger_error("Invalid bytecode pointer\n");
        return false;
    }

    uint8_t *memory = (uint8_t *)malloc(sizeof(uint8_t) * memsize);
    if (memory == NULL) {
        logger_error("Failed to allocate VM memory\n");
        return false;
    }
    memset(memory, 0x00, sizeof(uint8_t) * memsize);

    size_t *stack = (size_t *)malloc(RVM_STACK_SIZE * sizeof(size_t));
    if (stack == NULL) {
        logger_error("Failed to allocate VM stack\n");
        free(memory);
        return false;
    }
    memset(stack, 0x00, RVM_STACK_SIZE * sizeof(size_t));

    // Copy byte code at the start of memory
    size_t copy_size = (code_size < memsize) ? code_size : memsize;
    if (copy_size > 0) {
        memcpy(memory, code, copy_size);
    }

    if (copy_size < code_size) {
        logger_warning("Bytecode (%zu bytes) exceeds VM memory (%zu bytes); truncated\n", code_size, memsize);
    }

    /* Dump the full VM memory to ./memory.map only when explicitly
     * requested. It can contain sensitive program data and would otherwise
     * overwrite a user file on every run. */
    const char *dump_memory = getenv("RVM_DUMP_MEMORY");
    if (dump_memory != NULL && dump_memory[0] != '\0') {
        FILE* fp = fopen("memory.map", "wb");
        if (fp) {
            fwrite(memory, sizeof(uint8_t), memsize, fp);
            fclose(fp);

            logger_info("Memory map written: %zu bytes (filled with 0x00 + code at start)\n", memsize);
        } else {
            logger_error("Failed to create memory.map file\n");
        }
    }

    vm->memory = memory;
    vm->stack = stack;
    vm->pc = 0;
    vm->running = true;
    vm->code_size = copy_size;
    vm->memory_size = memsize;
    vm->sp = RVM_STACK_SIZE;    // Grows down in slots; empty when at the top
    vm->stack_size = RVM_STACK_SIZE;
    vm->max_steps = RVM_DEFAULT_MAX_STEPS;

    return true;
}

/* The dispatch loop. Executes instructions until the VM halts, errors, the
 * pc runs off the end of the code, or `step_limit` instructions have been
 * executed (SIZE_MAX = unlimited). */
static void vm_run_loop(vm_t *vm, size_t step_limit) {
    size_t steps = 0;
    uint8_t opcode = 0;

#if RVM_COMPUTED_GOTO
    /* One label per opcode; a range check in the dispatch (below) sends
     * unknown opcode bytes to vm_op_invalid. */
    static void *const vm_dispatch[] = {
        [OP_HALT]     = &&vm_op_halt,
        [OP_LOAD]     = &&vm_op_load,
        [OP_LA]       = &&vm_op_la,
        [OP_SA]       = &&vm_op_sa,
        [OP_MOV]      = &&vm_op_mov,
        [OP_ADD]      = &&vm_op_add,
        [OP_SUB]      = &&vm_op_sub,
        [OP_MULTI]    = &&vm_op_multi,
        [OP_DIVIDE]   = &&vm_op_divide,
        [OP_INCREASE] = &&vm_op_increase,
        [OP_DECREASE] = &&vm_op_decrease,
        [OP_AND]      = &&vm_op_and,
        [OP_NOT]      = &&vm_op_not,
        [OP_OR]       = &&vm_op_or,
        [OP_XOR]      = &&vm_op_xor,
        [OP_CMP]      = &&vm_op_cmp,
        [OP_JUMP]     = &&vm_op_jump,
        [OP_JNZ]      = &&vm_op_jnz,
        [OP_JZ]       = &&vm_op_jz,
        [OP_LOOP]     = &&vm_op_loop,
        [OP_PUSH]     = &&vm_op_push,
        [OP_POP]      = &&vm_op_pop,
        [OP_CALL]     = &&vm_op_call,
        [OP_RET]      = &&vm_op_ret,
        [OP_TRAP]     = &&vm_op_trap,
        [OP_PRINT]    = &&vm_op_print,
        [OP_FADD]     = &&vm_op_fadd,
        [OP_FSUB]     = &&vm_op_fsub,
        [OP_FMUL]     = &&vm_op_fmul,
        [OP_FDIV]     = &&vm_op_fdiv,
        [OP_FCMP]     = &&vm_op_fcmp,
        [OP_FLT]      = &&vm_op_flt,
        [OP_FLE]      = &&vm_op_fle,
        [OP_ITOF]     = &&vm_op_itof,
        [OP_FTOI]     = &&vm_op_ftoi,
        [OP_FLD]      = &&vm_op_fld,
        [OP_FPRT]     = &&vm_op_fprt,
    };
    goto vm_op_dispatch;
#else
    for (;;) {
        if (!vm->running || vm->pc >= vm->code_size) {
            break;
        }
        if (++steps > step_limit) {
            logger_error("Step limit exceeded (%zu steps); possible infinite loop\n",
                         step_limit);
            vm->running = false;
            break;
        }
        opcode = vm->memory[vm->pc++];
        switch (opcode) {
#endif

    VM_CASE(halt, HALT): {
        op_halt_impl(vm);
        VM_NEXT();
    }

    VM_CASE(load, LOAD): {
        op_load_impl(vm);
        VM_NEXT();
    }

    VM_CASE(la, LA): {
        op_la_impl(vm);
        VM_NEXT();
    }

    VM_CASE(sa, SA): {
        op_sa_impl(vm);
        VM_NEXT();
    }

    VM_CASE(mov, MOV): {
        op_mov_impl(vm);
        VM_NEXT();
    }

    VM_CASE(add, ADD): {
        op_add_impl(vm);
        VM_NEXT();
    }

    VM_CASE(sub, SUB): {
        op_sub_impl(vm);
        VM_NEXT();
    }

    VM_CASE(multi, MULTI): {
        op_multi_impl(vm);
        VM_NEXT();
    }

    VM_CASE(divide, DIVIDE): {
        op_divide_impl(vm);
        VM_NEXT();
    }

    VM_CASE(increase, INCREASE): {
        op_increase_impl(vm);
        VM_NEXT();
    }

    VM_CASE(decrease, DECREASE): {
        op_decrease_impl(vm);
        VM_NEXT();
    }

    VM_CASE(and, AND): {
        op_and_impl(vm);
        VM_NEXT();
    }

    VM_CASE(not, NOT): {
        op_not_impl(vm);
        VM_NEXT();
    }

    VM_CASE(or, OR): {
        op_or_impl(vm);
        VM_NEXT();
    }

    VM_CASE(xor, XOR): {
        op_xor_impl(vm);
        VM_NEXT();
    }

    VM_CASE(cmp, CMP): {
        op_cmp_impl(vm);
        VM_NEXT();
    }

    VM_CASE(jump, JUMP): {
        op_jump_impl(vm);
        VM_NEXT();
    }

    VM_CASE(jnz, JNZ): {
        op_jnz_impl(vm);
        VM_NEXT();
    }

    VM_CASE(jz, JZ): {
        op_jz_impl(vm);
        VM_NEXT();
    }

    VM_CASE(loop, LOOP): {
        op_loop_impl(vm);
        VM_NEXT();
    }

    VM_CASE(push, PUSH): {
        op_push_impl(vm);
        VM_NEXT();
    }

    VM_CASE(pop, POP): {
        op_pop_impl(vm);
        VM_NEXT();
    }

    VM_CASE(call, CALL): {
        op_call_impl(vm);
        VM_NEXT();
    }

    VM_CASE(ret, RET): {
        op_ret_impl(vm);
        VM_NEXT();
    }

    VM_CASE(trap, TRAP): {
        op_trap_impl(vm);
        VM_NEXT();
    }

    VM_CASE(print, PRINT): {
        op_print_impl(vm);
        VM_NEXT();
    }

    VM_CASE(fadd, FADD): {
        op_fadd_impl(vm);
        VM_NEXT();
    }

    VM_CASE(fsub, FSUB): {
        op_fsub_impl(vm);
        VM_NEXT();
    }

    VM_CASE(fmul, FMUL): {
        op_fmul_impl(vm);
        VM_NEXT();
    }

    VM_CASE(fdiv, FDIV): {
        op_fdiv_impl(vm);
        VM_NEXT();
    }

    VM_CASE(fcmp, FCMP): {
        op_fcmp_impl(vm);
        VM_NEXT();
    }

    VM_CASE(flt, FLT): {
        op_flt_impl(vm);
        VM_NEXT();
    }

    VM_CASE(fle, FLE): {
        op_fle_impl(vm);
        VM_NEXT();
    }

    VM_CASE(itof, ITOF): {
        op_itof_impl(vm);
        VM_NEXT();
    }

    VM_CASE(ftoi, FTOI): {
        op_ftoi_impl(vm);
        VM_NEXT();
    }

    VM_CASE(fld, FLD): {
        op_fld_impl(vm);
        VM_NEXT();
    }

    VM_CASE(fprt, FPRT): {
        op_fprt_impl(vm);
        VM_NEXT();
    }

#if RVM_COMPUTED_GOTO
    vm_op_invalid: {
#else
    default: {
#endif
        logger_error("Unknown opcode: 0x%02X at position %zu\n", opcode, vm->pc - 1);
        vm->running = false;
        VM_NEXT();
    }

#if RVM_COMPUTED_GOTO
    vm_op_dispatch: {
        if (!vm->running || vm->pc >= vm->code_size) {
            goto vm_done;
        }
        if (++steps > step_limit) {
            logger_error("Step limit exceeded (%zu steps); possible infinite loop\n",
                         step_limit);
            vm->running = false;
            goto vm_done;
        }
        opcode = vm->memory[vm->pc++];
        if (opcode > OP_FPRT) {
            goto vm_op_invalid;
        }
        goto *vm_dispatch[opcode];
    }
    vm_done: ;
#else
        }   /* switch */
    }   /* for */
#endif
}

/* Execute a single instruction (kept for API compatibility; vm_run is the
 * hot path). */
void vm_execute(vm_t *vm) {
    if (vm->pc >= vm->code_size) {
        vm->running = false;

        logger_debug("HLT: Reached end of program\n");
        return;
    }

    uint8_t opcode = vm->memory[vm->pc++];

    switch (opcode) {
        case OP_HALT:     op_halt_impl(vm);     break;
        case OP_LOAD:     op_load_impl(vm);     break;
        case OP_LA:       op_la_impl(vm);       break;
        case OP_SA:       op_sa_impl(vm);       break;
        case OP_MOV:      op_mov_impl(vm);      break;
        case OP_ADD:      op_add_impl(vm);      break;
        case OP_SUB:      op_sub_impl(vm);      break;
        case OP_MULTI:    op_multi_impl(vm);    break;
        case OP_DIVIDE:   op_divide_impl(vm);   break;
        case OP_INCREASE: op_increase_impl(vm); break;
        case OP_DECREASE: op_decrease_impl(vm); break;
        case OP_AND:      op_and_impl(vm);      break;
        case OP_NOT:      op_not_impl(vm);      break;
        case OP_OR:       op_or_impl(vm);       break;
        case OP_XOR:      op_xor_impl(vm);      break;
        case OP_CMP:      op_cmp_impl(vm);      break;
        case OP_JUMP:     op_jump_impl(vm);     break;
        case OP_JNZ:      op_jnz_impl(vm);      break;
        case OP_JZ:       op_jz_impl(vm);       break;
        case OP_LOOP:     op_loop_impl(vm);     break;
        case OP_PUSH:     op_push_impl(vm);     break;
        case OP_POP:      op_pop_impl(vm);      break;
        case OP_CALL:     op_call_impl(vm);     break;
        case OP_RET:      op_ret_impl(vm);      break;
        case OP_TRAP:     op_trap_impl(vm);     break;
        case OP_PRINT:    op_print_impl(vm);    break;
        case OP_FADD:     op_fadd_impl(vm);     break;
        case OP_FSUB:     op_fsub_impl(vm);     break;
        case OP_FMUL:     op_fmul_impl(vm);     break;
        case OP_FDIV:     op_fdiv_impl(vm);     break;
        case OP_FCMP:     op_fcmp_impl(vm);     break;
        case OP_FLT:      op_flt_impl(vm);      break;
        case OP_FLE:      op_fle_impl(vm);      break;
        case OP_ITOF:     op_itof_impl(vm);     break;
        case OP_FTOI:     op_ftoi_impl(vm);     break;
        case OP_FLD:      op_fld_impl(vm);      break;
        case OP_FPRT:     op_fprt_impl(vm);     break;

        default: {
            logger_error("Unknown opcode: 0x%02X at position %zu\n", opcode, vm->pc - 1);
            vm->running = false;
            break;
        }
    }
}

/* Run VM */
void vm_run(vm_t *vm) {
    logger_info("Starting VM execution...\n");

    /* 0 means unlimited; use SIZE_MAX so the per-step check is a single
     * comparison in both cases. */
    const size_t step_limit = (vm->max_steps > 0) ? vm->max_steps : SIZE_MAX;

    vm_run_loop(vm, step_limit);

    if (vm->running) {
        logger_info("VM execution completed\n");
    }
}
