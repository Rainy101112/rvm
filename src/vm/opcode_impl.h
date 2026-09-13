/*
 *
 *      opcode_impl.h
 *
 *      Per-instruction implementations, inlined into the dispatch loop in
 *      vm.c. Private to the VM: the public op_*_handler wrappers in
 *      opcode.c forward here.
 *
 *      Each impl validates its own operand bytes up front (same error
 *      messages the dispatch switch used to emit), so the dispatch loop
 *      itself never needs a length check per opcode.
 *
 *      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *      IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *      FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 */

#ifndef SRC_VM_OPCODE_IMPL_H_
#define SRC_VM_OPCODE_IMPL_H_

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "logger.h"
#include "trap.h"
#include "vm.h"

/* Floating point needs full 64-bit registers: fail loudly at compile time
 * rather than silently truncating on 32-bit size_t platforms. */
_Static_assert(sizeof(double) == 8, "RVM requires 64-bit doubles");
_Static_assert(sizeof(double) <= sizeof(size_t), "RVM requires 64-bit registers for floating point");

/* Halt the VM with the "Incomplete <name> instruction" diagnostic. */
static inline void op_incomplete(vm_t *vm, const char *name) {
    logger_error("Incomplete %s instruction\n", name);
    vm->running = false;
}

/* Read the 8-byte little-endian immediate at pc (advances pc). The caller
 * guarantees 8 bytes are available. memcpy handles unaligned addresses and
 * compiles to a single 64-bit load on common targets. */
static inline size_t read_value(vm_t *vm) {
    uint64_t value = 0;
    memcpy(&value, &vm->memory[vm->pc], sizeof(value));
    vm->pc += sizeof(value);
    return (size_t)value;
}

/* Store a value in one native-word slot on the call stack (grows down).
 * Halts the VM on stack overflow. */
static inline bool stack_push(vm_t *vm, uint64_t value) {
    if (vm->sp == 0) {
        logger_error("Stack overflow (stack full)\n");
        vm->running = false;
        return false;
    }

    vm->stack[--vm->sp] = (size_t)value;

    return true;
}

/* Pop a native-word slot from the call stack.
 * Halts the VM on stack underflow. */
static inline bool stack_pop(vm_t *vm, uint64_t *value) {
    if (vm->sp >= vm->stack_size) {
        logger_error("Stack underflow (stack empty)\n");
        vm->running = false;
        return false;
    }

    *value = (uint64_t)vm->stack[vm->sp++];

    return true;
}

static inline void op_halt_impl(vm_t *vm) {
    vm->running = false;

    logger_debug("HLT: Program terminated\n");
}

static inline void op_load_impl(vm_t *vm) {
    if (vm->pc + 9 > vm->code_size) {
        op_incomplete(vm, "LOAD");
        return;
    }

    uint8_t reg = vm->memory[vm->pc++] & 0x07;
    size_t value = read_value(vm);
    vm->registers[reg] = value;

    logger_debug("LD: R%d = %zu\n", reg, value);
}

static inline void op_la_impl(vm_t *vm) {
    if (vm->pc + 9 > vm->code_size) {
        op_incomplete(vm, "LA");
        return;
    }

    uint8_t reg = vm->memory[vm->pc++] & 0x07;
    size_t addr = read_value(vm);

    if (addr >= vm->memory_size || vm->memory_size - addr < 8) {
        logger_error("LA: address out of bounds: 0x%zx\n", addr);
        vm->running = false;
        return;
    }

    uint64_t value = 0;
    memcpy(&value, &vm->memory[addr], sizeof(value));
    vm->registers[reg] = (size_t)value;

    logger_debug("LA: R%d = %zx = [%zx]\n", reg, vm->registers[reg], addr);
}

static inline void op_sa_impl(vm_t *vm) {
    if (vm->pc + 9 > vm->code_size) {
        op_incomplete(vm, "SA");
        return;
    }

    uint8_t reg = vm->memory[vm->pc++] & 0x07;
    size_t addr = read_value(vm);

    if (addr >= vm->memory_size || vm->memory_size - addr < 8) {
        logger_error("SA: address out of bounds: 0x%zx\n", addr);
        vm->running = false;
        return;
    }

    uint64_t value = (uint64_t)vm->registers[reg];
    memcpy(&vm->memory[addr], &value, sizeof(value));

    logger_debug("SA: [%zx] = R%d = %zx\n", addr, reg, vm->registers[reg]);
}

static inline void op_mov_impl(vm_t *vm) {
    if (vm->pc + 2 > vm->code_size) {
        op_incomplete(vm, "MOV");
        return;
    }

    uint8_t reg_dest = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src = vm->memory[vm->pc++] & 0x07;
    vm->registers[reg_dest] = vm->registers[reg_src];

    logger_debug("MOV: R%d = R%d = %zu\n",
                 reg_dest, reg_src, vm->registers[reg_dest]);
}

/* Binary register op on 3 operand bytes: REG_DEST REG_SRC1 REG_SRC2. */
#define OP_BINARY_IMPL(NAME, OP, MNEMONIC) \
    static inline void op_##NAME##_impl(vm_t *vm) { \
        if (vm->pc + 3 > vm->code_size) { \
            op_incomplete(vm, #MNEMONIC); \
            return; \
        } \
        uint8_t reg_dest = vm->memory[vm->pc++] & 0x07; \
        uint8_t reg_src1 = vm->memory[vm->pc++] & 0x07; \
        uint8_t reg_src2 = vm->memory[vm->pc++] & 0x07; \
        vm->registers[reg_dest] = vm->registers[reg_src1] OP vm->registers[reg_src2]; \
        logger_debug(#MNEMONIC ": R%d = R%d " #OP " R%d = %zu\n", \
                     reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]); \
    }

OP_BINARY_IMPL(add, +, ADD)
OP_BINARY_IMPL(sub, -, SUB)
OP_BINARY_IMPL(multi, *, MUL)
OP_BINARY_IMPL(and, &, AND)
OP_BINARY_IMPL(or, |, OR)
OP_BINARY_IMPL(xor, ^, XOR)

/* DIV halts on division by zero instead. */
static inline void op_divide_impl(vm_t *vm) {
    if (vm->pc + 3 > vm->code_size) {
        op_incomplete(vm, "DIV");
        return;
    }

    uint8_t reg_dest = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x07;

    if (vm->registers[reg_src2] == 0) {
        logger_error("DIV: division by zero\n");
        vm->running = false;
        return;
    }

    vm->registers[reg_dest] = vm->registers[reg_src1] / vm->registers[reg_src2];

    logger_debug("DIV: R%d = R%d / R%d = %zu\n",
                 reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);
}

/* Unary register op on 1 operand byte: REG. */
#define OP_UNARY_IMPL(NAME, EXPR, MNEMONIC) \
    static inline void op_##NAME##_impl(vm_t *vm) { \
        if (vm->pc + 1 > vm->code_size) { \
            op_incomplete(vm, #MNEMONIC); \
            return; \
        } \
        uint8_t reg = vm->memory[vm->pc++] & 0x07; \
        EXPR; \
        logger_debug(#MNEMONIC ": R%d = %zu\n", reg, vm->registers[reg]); \
    }

OP_UNARY_IMPL(increase, vm->registers[reg]++, INC)
OP_UNARY_IMPL(decrease, vm->registers[reg]--, DEC)
OP_UNARY_IMPL(not, vm->registers[reg] = ~(vm->registers[reg]), NOT)

static inline void op_cmp_impl(vm_t *vm) {
    if (vm->pc + 3 > vm->code_size) {
        op_incomplete(vm, "CMP");
        return;
    }

    uint8_t reg_dest = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x07;

    if (vm->registers[reg_src1] == vm->registers[reg_src2]) {
        vm->registers[reg_dest] = 1;
        logger_debug("CMP: R%d == R%d R%d = %zu\n",
                     reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);
    } else {
        vm->registers[reg_dest] = 0;
        logger_debug("CMP: R%d != R%d R%d = %zu\n",
                     reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);
    }
}

static inline void op_jump_impl(vm_t *vm) {
    if (vm->pc + 1 > vm->code_size) {
        op_incomplete(vm, "JMP");
        return;
    }

    uint8_t reg = vm->memory[vm->pc++] & 0x07;

    if (vm->registers[reg] >= vm->code_size) {
        logger_error("JMP: target out of bounds\n");
        vm->running = false;
        return;
    }

    vm->pc = vm->registers[reg];

    logger_debug("JMP: R%d = %zu\n", reg, vm->registers[reg]);
}

static inline void op_jnz_impl(vm_t *vm) {
    if (vm->pc + 2 > vm->code_size) {
        op_incomplete(vm, "JNZ");
        return;
    }

    uint8_t reg_bool = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_addr = vm->memory[vm->pc++] & 0x07;

    if ((!(vm->registers[reg_bool])) == 1) {
        logger_debug("JNZ: R%d is false\n", reg_bool);
    } else {
        if (vm->registers[reg_addr] >= vm->code_size) {
            logger_error("JNZ: target out of bounds\n");
            vm->running = false;
            return;
        }

        vm->pc = vm->registers[reg_addr];

        logger_debug("JNZ: JMP %zu\n", vm->registers[reg_addr]);
    }
}

static inline void op_jz_impl(vm_t *vm) {
    if (vm->pc + 2 > vm->code_size) {
        op_incomplete(vm, "JZ");
        return;
    }

    uint8_t reg_bool = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_addr = vm->memory[vm->pc++] & 0x07;

    if ((!(vm->registers[reg_bool])) == 1) {
        if (vm->registers[reg_addr] >= vm->code_size) {
            logger_error("JZ: target out of bounds\n");
            vm->running = false;
            return;
        }

        vm->pc = vm->registers[reg_addr];

        logger_debug("JZ: JMP %zu\n", vm->registers[reg_addr]);
    } else {
        logger_debug("JZ: R%d is true\n", reg_bool);
    }
}

static inline void op_loop_impl(vm_t *vm) {
    if (vm->pc + 2 > vm->code_size) {
        op_incomplete(vm, "LOOP");
        return;
    }

    uint8_t reg_counter = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_addr = vm->memory[vm->pc++] & 0x07;

    if (vm->registers[reg_counter] == 0) {
        logger_debug("LOOP: R%d is false & STOP\n", reg_counter);
    } else {
        /* Decrement first (x86 LOOP semantics): a counter of N runs the
         * loop body exactly N times. */
        vm->registers[reg_counter]--;

        if (vm->registers[reg_counter] != 0) {
            if (vm->registers[reg_addr] >= vm->code_size) {
                logger_error("LOOP: target out of bounds\n");
                vm->running = false;
                return;
            }

            vm->pc = vm->registers[reg_addr];

            logger_debug("LOOP: R%d = %zu & JMP %zu\n",
                         reg_counter, vm->registers[reg_counter], vm->registers[reg_addr]);
        } else {
            logger_debug("LOOP: R%d = 0 & STOP\n", reg_counter);
        }
    }
}

static inline void op_push_impl(vm_t *vm) {
    if (vm->pc + 1 > vm->code_size) {
        op_incomplete(vm, "PUSH");
        return;
    }

    uint8_t reg = vm->memory[vm->pc++] & 0x07;

    if (!stack_push(vm, (uint64_t)vm->registers[reg])) {
        return;
    }

    logger_debug("PUSH: [%zx] = R%d = %zx\n", vm->sp, reg, vm->registers[reg]);
}

static inline void op_pop_impl(vm_t *vm) {
    if (vm->pc + 1 > vm->code_size) {
        op_incomplete(vm, "POP");
        return;
    }

    uint8_t reg = vm->memory[vm->pc++] & 0x07;
    uint64_t value;

    if (!stack_pop(vm, &value)) {
        return;
    }

    vm->registers[reg] = (size_t)value;

    logger_debug("POP: R%d = %zx = [%zx]\n", reg, vm->registers[reg], vm->sp - 1);
}

static inline void op_call_impl(vm_t *vm) {
    if (vm->pc + 1 > vm->code_size) {
        op_incomplete(vm, "CALL");
        return;
    }

    uint8_t reg = vm->memory[vm->pc++] & 0x07;
    size_t target = vm->registers[reg];

    if (target >= vm->code_size) {
        logger_error("CALL: target out of bounds: 0x%zx\n", target);
        vm->running = false;
        return;
    }

    /* Return address is pc, which already points past the operand */
    if (!stack_push(vm, (uint64_t)vm->pc)) {
        return;
    }

    vm->pc = target;

    logger_debug("CALL: JMP %zu\n", target);
}

static inline void op_ret_impl(vm_t *vm) {
    uint64_t ret;

    if (!stack_pop(vm, &ret)) {
        return;
    }

    if (ret >= vm->code_size) {
        logger_error("RET: return address out of bounds: 0x%zx\n", (size_t)ret);
        vm->running = false;
        return;
    }

    vm->pc = (size_t)ret;

    logger_debug("RET: JMP %zu\n", (size_t)ret);
}

static inline void op_trap_impl(vm_t *vm) {
    if (vm->pc + 2 > vm->code_size) {
        op_incomplete(vm, "TRAP");
        return;
    }

    uint8_t reg_num = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_value = vm->memory[vm->pc++] & 0x07;

    size_t trap_number = vm->registers[reg_num];

    switch (trap_number) {
    case TRAP_PUTC: {
        trap_putc(vm, reg_value);
        break;
    }

    case TRAP_GETC: {
        trap_getc(vm, reg_value);
        break;
    }

    default: {
        logger_error("Unknown trap number: %zu\n", trap_number);
        vm->running = false;
        break;
    }
    }
}

static inline void op_print_impl(vm_t *vm) {
    if (vm->pc + 1 > vm->code_size) {
        op_incomplete(vm, "PRT");
        return;
    }

    uint8_t reg = vm->memory[vm->pc++] & 0x07;

    /* Program output, not tracing: visible even in quiet mode */
    printf("PRT: R%d = %zu\n", reg, vm->registers[reg]);
}

/* --- Floating point (IEEE 754 double, raw register bits) --- */

/* Reinterpret register contents as a double and back. */
static inline double reg_to_double(size_t value) {
    double d;
    memcpy(&d, &value, sizeof(d));
    return d;
}

static inline size_t double_to_reg(double d) {
    size_t value;
    memcpy(&value, &d, sizeof(value));
    return value;
}

/* Binary float register op on 3 operand bytes: REG_DEST REG_SRC1 REG_SRC2.
 * IEEE semantics throughout: FDIV by zero yields +/-inf, never halts. */
#define OP_FBINARY_IMPL(NAME, OP, MNEMONIC) \
    static inline void op_##NAME##_impl(vm_t *vm) { \
        if (vm->pc + 3 > vm->code_size) { \
            op_incomplete(vm, #MNEMONIC); \
            return; \
        } \
        uint8_t reg_dest = vm->memory[vm->pc++] & 0x07; \
        uint8_t reg_src1 = vm->memory[vm->pc++] & 0x07; \
        uint8_t reg_src2 = vm->memory[vm->pc++] & 0x07; \
        double result = reg_to_double(vm->registers[reg_src1]) OP \
                        reg_to_double(vm->registers[reg_src2]); \
        vm->registers[reg_dest] = double_to_reg(result); \
        logger_debug(#MNEMONIC ": R%d = R%d " #OP " R%d = %g\n", \
                     reg_dest, reg_src1, reg_src2, result); \
    }

OP_FBINARY_IMPL(fadd, +, FADD)
OP_FBINARY_IMPL(fsub, -, FSUB)
OP_FBINARY_IMPL(fmul, *, FMUL)
OP_FBINARY_IMPL(fdiv, /, FDIV)

/* Float condition on 3 operand bytes: REG_DEST REG_SRC1 REG_SRC2.
 * Sets REG_DEST to 1 or 0; NaN compares false for every relation. */
#define OP_FCOND_IMPL(NAME, OP, MNEMONIC) \
    static inline void op_##NAME##_impl(vm_t *vm) { \
        if (vm->pc + 3 > vm->code_size) { \
            op_incomplete(vm, #MNEMONIC); \
            return; \
        } \
        uint8_t reg_dest = vm->memory[vm->pc++] & 0x07; \
        uint8_t reg_src1 = vm->memory[vm->pc++] & 0x07; \
        uint8_t reg_src2 = vm->memory[vm->pc++] & 0x07; \
        vm->registers[reg_dest] = \
            (reg_to_double(vm->registers[reg_src1]) OP \
             reg_to_double(vm->registers[reg_src2])) ? 1 : 0; \
        logger_debug(#MNEMONIC ": R%d = R%d " #OP " R%d = %zu\n", \
                     reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]); \
    }

OP_FCOND_IMPL(fcmp, ==, FCMP)
OP_FCOND_IMPL(flt, <, FLT)
OP_FCOND_IMPL(fle, <=, FLE)

static inline void op_itof_impl(vm_t *vm) {
    if (vm->pc + 2 > vm->code_size) {
        op_incomplete(vm, "ITOF");
        return;
    }

    uint8_t reg_dest = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src = vm->memory[vm->pc++] & 0x07;
    double result = (double)vm->registers[reg_src];
    vm->registers[reg_dest] = double_to_reg(result);

    logger_debug("ITOF: R%d = %g\n", reg_dest, result);
}

static inline void op_ftoi_impl(vm_t *vm) {
    if (vm->pc + 2 > vm->code_size) {
        op_incomplete(vm, "FTOI");
        return;
    }

    uint8_t reg_dest = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src = vm->memory[vm->pc++] & 0x07;

    double value = reg_to_double(vm->registers[reg_src]);

    /* Out-of-range and NaN float->int conversion is undefined in C; halt
     * instead, matching the strict integer DIV-by-zero behavior. */
    if (!(value >= 0.0 && value < 18446744073709551616.0)) {   /* 2^64 */
        logger_error("FTOI: value out of range: %g\n", value);
        vm->running = false;
        return;
    }

    vm->registers[reg_dest] = (size_t)value;

    logger_debug("FTOI: R%d = %zu\n", reg_dest, vm->registers[reg_dest]);
}

static inline void op_fld_impl(vm_t *vm) {
    if (vm->pc + 9 > vm->code_size) {
        op_incomplete(vm, "FLD");
        return;
    }

    uint8_t reg = vm->memory[vm->pc++] & 0x07;
    size_t value = read_value(vm);    /* the double's raw bit pattern */
    vm->registers[reg] = value;

    logger_debug("FLD: R%d = %g\n", reg, reg_to_double(value));
}

static inline void op_fprt_impl(vm_t *vm) {
    if (vm->pc + 1 > vm->code_size) {
        op_incomplete(vm, "FPRT");
        return;
    }

    uint8_t reg = vm->memory[vm->pc++] & 0x07;

    /* Program output, not tracing: visible even in quiet mode.
     * %.17g round-trips the exact double value. */
    printf("FPRT: R%d = %.17g\n", reg, reg_to_double(vm->registers[reg]));
}

#endif // SRC_VM_OPCODE_IMPL_H_
