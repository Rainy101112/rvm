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

inline void op_load_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x03;
    uint8_t value = vm->memory[vm->pc++];
    vm->registers[reg] = value;

    logger_print("LD: R%d = %d\n", reg, value);

    return;
}

inline void op_la_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x03;
    uint8_t addr = vm->memory[vm->pc++];

    vm->registers[reg] = vm->memory[addr];

    logger_print("LA: R%d = %d = [%x]\n", reg, vm->memory[addr], addr);
}

inline void op_sa_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x03;
    uint8_t addr = vm->memory[vm->pc++];

    vm->memory[addr] = vm->registers[reg];

    logger_print("SA: [%x] = R%d = %d\n", addr, reg, vm->memory[addr]);
}

inline void op_mov_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src = vm->memory[vm->pc++] & 0x03;
    vm->registers[reg_dest] = vm->registers[reg_src];

    logger_print("MOV: R%d = R%d = %d\n", 
          reg_dest, reg_src, vm->registers[reg_dest]);

    return;
}

inline void op_add_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x03;
    vm->registers[reg_dest] = vm->registers[reg_src1] + vm->registers[reg_src2];

    logger_print("ADD: R%d = R%d + R%d = %d\n", 
          reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);

    return;
}

inline void op_sub_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x03;
    vm->registers[reg_dest] = vm->registers[reg_src1] - vm->registers[reg_src2];

    logger_print("SUB: R%d = R%d - R%d = %d\n", 
          reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);

    return;
}

inline void op_multi_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x03;
    vm->registers[reg_dest] = vm->registers[reg_src1] * vm->registers[reg_src2];

    logger_print("MUL: R%d = R%d * R%d = %d\n", 
          reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);

    return;
}

inline void op_divide_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x03;
    vm->registers[reg_dest] = vm->registers[reg_src1] / vm->registers[reg_src2];

    logger_print("DIV: R%d = R%d / R%d = %d\n", 
          reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);

    return;
}

inline void op_increase_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x03;
    vm->registers[reg]++;

    logger_print("INC: R%d = %d\n", reg, vm->registers[reg]);

    return;
}

inline void op_decrease_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x03;
    vm->registers[reg]--;

    logger_print("DEC: R%d = %d\n", reg, vm->registers[reg]);

    return;
}

inline void op_and_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x03;
    vm->registers[reg_dest] = vm->registers[reg_src1] & vm->registers[reg_src2];

    logger_print("AND: R%d = R%d / R%d = %d\n", 
          reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);

    return;
}

inline void op_not_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x03;
    vm->registers[reg] = !(vm->registers[reg]);

    logger_print("PRT: R%d = %d\n", reg, vm->registers[reg]);

    return;
}

inline void op_or_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x03;
    vm->registers[reg_dest] = vm->registers[reg_src1] | vm->registers[reg_src2];

    logger_print("AND: R%d = R%d / R%d = %d\n", 
          reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);

    return;
}

inline void op_xor_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x03;
    vm->registers[reg_dest] = vm->registers[reg_src1] ^ vm->registers[reg_src2];

    logger_print("AND: R%d = R%d / R%d = %d\n", 
          reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);

    return;
}

inline void op_cmp_handler(vm_t *vm){
    uint8_t reg_dest = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src1 = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_src2 = vm->memory[vm->pc++] & 0x03;

    if (vm->registers[reg_src1] == vm->registers[reg_src2]){
        vm->registers[reg_dest] = 1;

        logger_print("CMP: R%d == R%d R%d = %d\n", 
              reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);
    } 
    else {
        vm->registers[reg_dest] = 0;

        logger_print("CMP: R%d != R%d R%d = %d\n", 
              reg_dest, reg_src1, reg_src2, vm->registers[reg_dest]);
    }

    return;
}

inline void op_jump_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x03;
    vm->pc = vm->registers[reg];

    logger_print("JMP: R%d = %d\n", reg, vm->registers[reg]);

    return;
}

inline void op_jnz_handler(vm_t *vm){
    uint8_t reg_bool = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_addr = vm->memory[vm->pc++] & 0x03;

    if ((!(vm->registers[reg_bool])) == 1) {
        logger_print("JNZ: R%d is false\n", reg_bool);
    }
    else {
        vm->pc = vm->registers[reg_addr];

        logger_print("JNZ: JMP %d\n", vm->registers[reg_addr]);
    }

    return;
}

inline void op_jz_handler(vm_t *vm){
    uint8_t reg_bool = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_addr = vm->memory[vm->pc++] & 0x03;

    if ((!(vm->registers[reg_bool])) == 1) {
        vm->pc = vm->registers[reg_addr];

        logger_print("JZ: JMP %d\n", vm->registers[reg_addr]);
    }
    else {
        
        logger_print("JZ: R%d is true\n", reg_bool);
    }

    return;
}

inline void op_loop_handler(vm_t *vm){
    uint8_t reg_counter = vm->memory[vm->pc++] & 0x03;
    uint8_t reg_addr = vm->memory[vm->pc++] & 0x03;

    if ((!(vm->registers[reg_counter])) == 1) {
        logger_print("LOOP: R%d is false & STOP\n",
          reg_counter);
    }
    else {
        vm->registers[reg_counter]--;
        vm->pc = vm->registers[reg_addr];

        logger_print("JNZ: R%d = %d & JMP %d\n",
          reg_counter, vm->registers[reg_counter], vm->registers[reg_addr]);
    }

    return;
}

inline void op_print_handler(vm_t *vm){
    uint8_t reg = vm->memory[vm->pc++] & 0x03;

    logger_print("PRT: R%d = %d\n", reg, vm->registers[reg]);

    return;
}

