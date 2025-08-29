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

/* Initialize VM */
void vm_init(vm_t *vm, uint8_t *code, size_t code_size, size_t memsize) {
    memset(vm->registers, 0, sizeof(vm->registers));

    uint8_t *memory = (uint8_t *)malloc(sizeof(uint8_t) * memsize);
    if (memory == NULL) {
        logger_error("Failed to allocate VM memory\n");
        exit(1);
    }
    memset(memory, 0x00, sizeof(uint8_t) * memsize);
    
    // Copy byte code at the start of memory
    size_t copy_size = (code_size < memsize) ? code_size : memsize;
    memcpy(memory, code, copy_size);

    // Write memory.map
    FILE* fp = fopen("memory.map", "wb");
    if (fp) {
        fwrite(memory, sizeof(uint8_t), memsize, fp);
        fclose(fp);

        #if defined(_WIN32)
            logger_print("Memory map written: %lld bytes (filled with 0x00 + code at start)\n", memsize);
        #else
            logger_print("Memory map written: %ld bytes (filled with 0x00 + code at start)\n", memsize);
        #endif
    } else {
        logger_error("Failed to create memory.map file\n");
    }

    vm->memory = memory;
    vm->pc = 0;
    vm->running = true;
    vm->code_size = code_size;
}

/* Execute an instruction */
void vm_execute(vm_t *vm) {
    if (vm->pc >= vm->code_size) {
        vm->running = false;

        logger_print("HLT: Reached end of program\n");
        return;
    }
    
    uint8_t opcode = vm->memory[vm->pc++];
    
    switch (opcode) {
        case OP_HALT: {
            vm->running = false;

            logger_print("HLT: Program terminated\n");
            break;
        }
        
        case OP_LOAD: {
            if (vm->pc + 1 >= vm->code_size) {
                logger_error("Incomplete LOAD instruction\n");
                vm->running = false;
                break;
            }

            op_load_handler(vm);

            break;
        }

        case OP_LA: {
            if (vm->pc + 1 >= vm->code_size) {
                logger_error("Incomplete LOAD instruction\n");
                vm->running = false;
                break;
            }

            op_la_handler(vm);

            break;
        }

        case OP_SA: {
            if (vm->pc + 1 >= vm->code_size) {
                logger_error("Incomplete LOAD instruction\n");
                vm->running = false;
                break;
            }

            op_sa_handler(vm);

            break;
        }

        case OP_MOV: {
            if (vm->pc + 1 >= vm->code_size) {
                logger_error("Incomplete ADD instruction\n");
                vm->running = false;
                break;
            }

            op_mov_handler(vm);

            break;
        }
        
        case OP_ADD: {
            if (vm->pc + 2 >= vm->code_size) {
                logger_error("Incomplete ADD instruction\n");
                vm->running = false;
                break;
            }

            op_add_handler(vm);

            break;
        }

        case OP_SUB: {
            if (vm->pc + 2 >= vm->code_size) {
                logger_error("Incomplete ADD instruction\n");
                vm->running = false;
                break;
            }

            op_sub_handler(vm);

            break;
        }

        case OP_MULTI: {
            if (vm->pc + 2 >= vm->code_size) {
                logger_error("Incomplete ADD instruction\n");
                vm->running = false;
                break;
            }

            break;
        }

        case OP_DIVIDE: {
            if (vm->pc + 2 >= vm->code_size) {
                logger_error("Incomplete ADD instruction\n");
                vm->running = false;
                break;
            }

            op_multi_handler(vm);

            break;
        }

        case OP_INCREASE: {
            if (vm->pc >= vm->code_size) {
                logger_error("Incomplete PRINT instruction\n");
                vm->running = false;
                break;
            }

            op_increase_handler(vm);

            break;
        }

        case OP_DECREASE: {
            if (vm->pc >= vm->code_size) {
                logger_error("Incomplete PRINT instruction\n");
                vm->running = false;
                break;
            }

            op_decrease_handler(vm);

            break;
        }

        case OP_AND: {
            if (vm->pc + 2 >= vm->code_size) {
                logger_error("Incomplete ADD instruction\n");
                vm->running = false;
                break;
            }

            op_and_handler(vm);

            break;
        }

        case OP_NOT: {
            if (vm->pc >= vm->code_size) {
                logger_error("Incomplete PRINT instruction\n");
                vm->running = false;
                break;
            }

            op_not_handler(vm);

            break;
        }

        case OP_OR: {
            if (vm->pc + 2 >= vm->code_size) {
                logger_error("Incomplete ADD instruction\n");
                vm->running = false;
                break;
            }

            op_or_handler(vm);

            break;
        }

        case OP_XOR: {
            if (vm->pc + 2 >= vm->code_size) {
                logger_error("Incomplete ADD instruction\n");
                vm->running = false;
                break;
            }

            op_xor_handler(vm);

            break;
        }
        
        case OP_CMP: {
            if (vm->pc + 2) {
                logger_error("Incomplete PRINT instruction\n");
                vm->running = false;
                break;
            }

            op_cmp_handler(vm);

            break;
        }

        case OP_JUMP: {
            if (vm->pc >= vm->code_size) {
                logger_error("Incomplete PRINT instruction\n");
                vm->running = false;
                break;
            }

            op_jump_handler(vm);

            break;
        }

        case OP_JNZ: {
            if (vm->pc + 1 >= vm->code_size) {
                logger_error("Incomplete ADD instruction\n");
                vm->running = false;
                break;
            }

            op_jnz_handler(vm);

            break;
        }

        case OP_JZ: {
            if (vm->pc + 1 >= vm->code_size) {
                logger_error("Incomplete ADD instruction\n");
                vm->running = false;
                break;
            }

            op_jz_handler(vm);

            break;
        }

        case OP_LOOP: {
            if (vm->pc + 1 >= vm->code_size) {
                logger_error("Incomplete ADD instruction\n");
                vm->running = false;
                break;
            }

            op_loop_handler(vm);

            break;
        }

        case OP_PRINT: {
            if (vm->pc >= vm->code_size) {
                logger_error("Incomplete PRINT instruction\n");
                vm->running = false;
                break;
            }

            op_print_handler(vm);

            break;
        }
        
        default: {
            #if defined(_WIN32)
                logger_error("Unknown opcode: 0x%02X at position %lld\n", opcode, vm->pc - 1);
            #else
                logger_error("Unknown opcode: 0x%02X at position %ld\n", opcode, vm->pc - 1);
            #endif
            
            vm->running = false;

            break;
        }
    }
}

/* Run VM */
void vm_run(vm_t *vm) {
    logger_print("Starting VM execution...\n");
    while (vm->running && vm->pc < vm->code_size) {
        vm_execute(vm);
    }
    
    if (vm->running) {
        logger_print("VM execution completed\n");
    }
}

