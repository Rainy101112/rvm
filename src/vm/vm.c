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

    uint8_t *stack = (uint8_t *)malloc(RVM_STACK_SIZE);
    if (stack == NULL) {
        logger_error("Failed to allocate VM stack\n");
        free(memory);
        return false;
    }
    memset(stack, 0x00, RVM_STACK_SIZE);

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
    vm->sp = RVM_STACK_SIZE;    // Grows down; empty when at the top
    vm->stack_size = RVM_STACK_SIZE;
    vm->max_steps = RVM_DEFAULT_MAX_STEPS;

    return true;
}

/* Execute an instruction */
void vm_execute(vm_t *vm) {
    if (vm->pc >= vm->code_size) {
        vm->running = false;

        logger_debug("HLT: Reached end of program\n");
        return;
    }
    
    uint8_t opcode = vm->memory[vm->pc++];
    
    switch (opcode) {
        case OP_HALT: {
            vm->running = false;

            logger_debug("HLT: Program terminated\n");
            break;
        }
        
        case OP_LOAD: {
            if (vm->pc + 8 >= vm->code_size) {
                logger_error("Incomplete LOAD instruction\n");
                vm->running = false;
                break;
            }

            op_load_handler(vm);

            break;
        }

        case OP_LA: {
            if (vm->pc + 8 >= vm->code_size) {
                logger_error("Incomplete LA instruction\n");
                vm->running = false;
                break;
            }

            op_la_handler(vm);

            break;
        }

        case OP_SA: {
            if (vm->pc + 8 >= vm->code_size) {
                logger_error("Incomplete SA instruction\n");
                vm->running = false;
                break;
            }

            op_sa_handler(vm);

            break;
        }

        case OP_MOV: {
            if (vm->pc + 1 >= vm->code_size) {
                logger_error("Incomplete MOV instruction\n");
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
                logger_error("Incomplete SUB instruction\n");
                vm->running = false;
                break;
            }

            op_sub_handler(vm);

            break;
        }

        case OP_MULTI: {
            if (vm->pc + 3 > vm->code_size) {
                logger_error("Incomplete MUL instruction\n");
                vm->running = false;
                break;
            }

            op_multi_handler(vm);

            break;
        }

        case OP_DIVIDE: {
            if (vm->pc + 3 > vm->code_size) {
                logger_error("Incomplete DIV instruction\n");
                vm->running = false;
                break;
            }

            op_divide_handler(vm);

            break;
        }

        case OP_INCREASE: {
            if (vm->pc >= vm->code_size) {
                logger_error("Incomplete INC instruction\n");
                vm->running = false;
                break;
            }

            op_increase_handler(vm);

            break;
        }

        case OP_DECREASE: {
            if (vm->pc >= vm->code_size) {
                logger_error("Incomplete DEC instruction\n");
                vm->running = false;
                break;
            }

            op_decrease_handler(vm);

            break;
        }

        case OP_AND: {
            if (vm->pc + 2 >= vm->code_size) {
                logger_error("Incomplete AND instruction\n");
                vm->running = false;
                break;
            }

            op_and_handler(vm);

            break;
        }

        case OP_NOT: {
            if (vm->pc >= vm->code_size) {
                logger_error("Incomplete NOT instruction\n");
                vm->running = false;
                break;
            }

            op_not_handler(vm);

            break;
        }

        case OP_OR: {
            if (vm->pc + 2 >= vm->code_size) {
                logger_error("Incomplete OR instruction\n");
                vm->running = false;
                break;
            }

            op_or_handler(vm);

            break;
        }

        case OP_XOR: {
            if (vm->pc + 2 >= vm->code_size) {
                logger_error("Incomplete XOR instruction\n");
                vm->running = false;
                break;
            }

            op_xor_handler(vm);

            break;
        }
        
        case OP_CMP: {
            if (vm->pc + 2 >= vm->code_size) {
                logger_error("Incomplete CMP instruction\n");
                vm->running = false;
                break;
            }

            op_cmp_handler(vm);

            break;
        }

        case OP_JUMP: {
            if (vm->pc >= vm->code_size) {
                logger_error("Incomplete JMP instruction\n");
                vm->running = false;
                break;
            }

            op_jump_handler(vm);

            break;
        }

        case OP_JNZ: {
            if (vm->pc + 1 >= vm->code_size) {
                logger_error("Incomplete JNZ instruction\n");
                vm->running = false;
                break;
            }

            op_jnz_handler(vm);

            break;
        }

        case OP_JZ: {
            if (vm->pc + 1 >= vm->code_size) {
                logger_error("Incomplete JZ instruction\n");
                vm->running = false;
                break;
            }

            op_jz_handler(vm);

            break;
        }

        case OP_LOOP: {
            if (vm->pc + 1 >= vm->code_size) {
                logger_error("Incomplete LOOP instruction\n");
                vm->running = false;
                break;
            }

            op_loop_handler(vm);

            break;
        }

        case OP_PUSH: {
            if (vm->pc >= vm->code_size) {
                logger_error("Incomplete PUSH instruction\n");
                vm->running = false;
                break;
            }

            op_push_handler(vm);

            break;
        }

        case OP_POP: {
            if (vm->pc >= vm->code_size) {
                logger_error("Incomplete POP instruction\n");
                vm->running = false;
                break;
            }

            op_pop_handler(vm);

            break;
        }

        case OP_CALL: {
            if (vm->pc >= vm->code_size) {
                logger_error("Incomplete CALL instruction\n");
                vm->running = false;
                break;
            }

            op_call_handler(vm);

            break;
        }

        case OP_RET: {
            op_ret_handler(vm);

            break;
        }

        case OP_TRAP: {
            if (vm->pc + 1 >= vm->code_size) {
                logger_error("Incomplete TRAP instruction\n");
                vm->running = false;
                break;
            }

            op_trap_handler(vm);

            break;
        }

        case OP_PRINT: {
            if (vm->pc >= vm->code_size) {
                logger_error("Incomplete PRT instruction\n");
                vm->running = false;
                break;
            }

            op_print_handler(vm);

            break;
        }
        
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
    size_t steps = 0;

    while (vm->running && vm->pc < vm->code_size) {
        if (vm->max_steps > 0 && ++steps > vm->max_steps) {
            logger_error("Step limit exceeded (%zu steps); possible infinite loop\n",
                         vm->max_steps);
            vm->running = false;
            break;
        }
        vm_execute(vm);
    }
    
    if (vm->running) {
        logger_info("VM execution completed\n");
    }
}

