/* 
 *
 *      opcode.c
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

#include "instruction.h"
#include "logger.h"
#include "vm.h"
#include "trap.h"

static size_t read_value(vm_t *vm) {
    uint64_t value = 0;

    if (vm->pc + 8 > vm->code_size) {
        logger_error("read_value: out of bounds read\n");
        vm->running = false;
        return 0;
    }

    for (int i = 0; i < 8; i++) {
        value |= (uint64_t)vm->memory[vm->pc++] << (i * 8);
    }
    return (size_t)value;
}

void op_load_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x07;
    size_t value = read_value(vm);
    if (!vm->running) return;

    vm->registers[reg] = value;

    logger_print("LD: R%d = %zu\n", reg, value);

    return;
}

void op_sa_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x07;
    size_t addr = read_value(vm);
    if (!vm->running) return;

    if (addr >= vm->memory_size || vm->memory_size - addr < 8) {
        logger_error("SA: address out of bounds: 0x%zx\n", addr);
        vm->running = false;
        return;
    }

    uint64_t value = (uint64_t)vm->registers[reg];
    for (int i = 0; i < 8; i++) {
        vm->memory[addr + i] = (value >> (i * 8)) & 0xFF;
    }

    logger_print("SA: [%zx] = R%d = %zx\n", addr, reg, vm->registers[reg]);
}

void op_la_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x07;
    size_t addr = read_value(vm);
    if (!vm->running) return;

    if (addr >= vm->memory_size || vm->memory_size - addr < 8) {
        logger_error("LA: address out of bounds: 0x%zx\n", addr);
        vm->running = false;
        return;
    }

    uint64_t value = 0;
    for (int i = 0; i < 8; i++) {
        value |= (uint64_t)vm->memory[addr + i] << (i * 8);
    }
    vm->registers[reg] = (size_t)value;

    logger_print("LA: R%d = %zx = [%zx]\n", reg, vm->registers[reg], addr);
}

void op_mov_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src = vm->memory[vm->pc++] & 0x07;
    vm->registers[reg_dest] = vm->registers[reg_src];

    logger_print("MOV: R%d = R%d = %zu\n", 
          reg_dest, reg_src, vm->registers[reg_dest]);

    return;
}

void op_add_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x07;
    vm->registers[reg_dest] = vm->registers[reg_src1] + vm->registers[reg_src2];

    logger_print("ADD: R%d = R%d + R%d = %zu\n", 
          reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);

    return;
}

void op_sub_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x07;
    vm->registers[reg_dest] = vm->registers[reg_src1] - vm->registers[reg_src2];

    logger_print("SUB: R%d = R%d - R%d = %zu\n", 
          reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);

    return;
}

void op_multi_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x07;
    vm->registers[reg_dest] = vm->registers[reg_src1] * vm->registers[reg_src2];

    logger_print("MUL: R%d = R%d * R%d = %zu\n", 
          reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);

    return;
}

void op_divide_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x07;

    if (vm->registers[reg_src2] == 0) {
        logger_error("DIV: division by zero\n");
        vm->running = false;
        return;
    }

    vm->registers[reg_dest] = vm->registers[reg_src1] / vm->registers[reg_src2];

    logger_print("DIV: R%d = R%d / R%d = %zu\n", 
          reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);

    return;
}

void op_increase_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x07;
    vm->registers[reg]++;

    logger_print("INC: R%d = %zu\n", reg, vm->registers[reg]);

    return;
}

void op_decrease_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x07;
    vm->registers[reg]--;

    logger_print("DEC: R%d = %zu\n", reg, vm->registers[reg]);

    return;
}

void op_and_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x07;
    vm->registers[reg_dest] = vm->registers[reg_src1] & vm->registers[reg_src2];

    logger_print("AND: R%d = R%d & R%d = %zu\n", 
          reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);

    return;
}

void op_not_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x07;
    vm->registers[reg] = ~(vm->registers[reg]);

    logger_print("NOT: R%d = %zu\n", reg, vm->registers[reg]);

    return;
}

void op_or_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x07;
    vm->registers[reg_dest] = vm->registers[reg_src1] | vm->registers[reg_src2];

    logger_print("OR: R%d = R%d | R%d = %zu\n", 
          reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);

    return;
}

void op_xor_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x07;
    vm->registers[reg_dest] = vm->registers[reg_src1] ^ vm->registers[reg_src2];

    logger_print("XOR: R%d = R%d ^ R%d = %zu\n", 
          reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);

    return;
}

void op_cmp_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x07;

    if (vm->registers[reg_src1] == vm->registers[reg_src2]){
        vm->registers[reg_dest] = 1;

        logger_print("CMP: R%d == R%d R%d = %zu\n", 
              reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);
    } 
    else {
        vm->registers[reg_dest] = 0;

        logger_print("CMP: R%d != R%d R%d = %zu\n", 
              reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);
    }

    return;
}

void op_jump_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x07;

    if (vm->registers[reg] >= vm->code_size) {
        logger_error("JMP: target out of bounds\n");
        vm->running = false;
        return;
    }

    vm->pc = vm->registers[reg];

    logger_print("JMP: R%d = %zu\n", reg, vm->registers[reg]);

    return;
}

void op_jnz_handler(vm_t *vm){
    uint8_t reg_bool = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_addr = vm->memory[vm->pc++] & 0x07;

    if ((!(vm->registers[reg_bool])) == 1) {
        logger_print("JNZ: R%d is false\n", reg_bool);
    }
    else {
        if (vm->registers[reg_addr] >= vm->code_size) {
            logger_error("JNZ: target out of bounds\n");
            vm->running = false;
            return;
        }

        vm->pc = vm->registers[reg_addr];

        logger_print("JNZ: JMP %zu\n", vm->registers[reg_addr]);
    }

    return;
}

void op_jz_handler(vm_t *vm){
    uint8_t reg_bool = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_addr = vm->memory[vm->pc++] & 0x07;

    if ((!(vm->registers[reg_bool])) == 1) {
        if (vm->registers[reg_addr] >= vm->code_size) {
            logger_error("JZ: target out of bounds\n");
            vm->running = false;
            return;
        }

        vm->pc = vm->registers[reg_addr];

        logger_print("JZ: JMP %zu\n", vm->registers[reg_addr]);
    }
    else {
        
        logger_print("JZ: R%d is true\n", reg_bool);
    }

    return;
}

void op_loop_handler(vm_t *vm){
    uint8_t reg_counter = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_addr = vm->memory[vm->pc++] & 0x07;

    if (vm->registers[reg_counter] == 0) {
        logger_print("LOOP: R%d is false & STOP\n",
          reg_counter);
    }
    else {
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

            logger_print("LOOP: R%d = %zu & JMP %zu\n",
              reg_counter, vm->registers[reg_counter], vm->registers[reg_addr]);
        } else {
            logger_print("LOOP: R%d = 0 & STOP\n", reg_counter);
        }
    }

    return;
}

void op_trap_handler(vm_t *vm) {
    uint8_t reg_num = vm->memory[vm->pc++] & 0x07;
    uint8_t reg_value = vm->memory[vm->pc++] & 0x07;

    size_t trap_number = vm->registers[reg_num];

    switch (trap_number)
    {
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

    return;
}

void op_print_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x07;

    logger_print("PRT: R%d = %zu\n", reg, vm->registers[reg]);

    return;
}

