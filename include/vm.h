#ifndef INCLUDE_VM_H_
#define INCLUDE_VM_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* VM state */
struct vm_state {
    uint8_t registers[4];  // 4 common registers
    uint8_t *memory;       // Memory pointer
    size_t pc;             // Program counter
    bool running;          // Running flag
    size_t code_size;      // Size of byte code
};

typedef struct vm_state vm_t;

void vm_init(vm_t *vm, uint8_t *code, size_t code_size, size_t memsize);
void vm_execute(vm_t *vm);
void vm_run(vm_t *vm);

void op_load_handler(vm_t *vm);
void op_mov_handler(vm_t *vm);
void op_add_handler(vm_t *vm);
void op_sub_handler(vm_t *vm);
void op_multi_handler(vm_t *vm);
void op_divide_handler(vm_t *vm);
void op_increase_handler(vm_t *vm);
void op_decrease_handler(vm_t *vm);
void op_and_handler(vm_t *vm);
void op_not_handler(vm_t *vm);
void op_or_handler(vm_t *vm);
void op_xor_handler(vm_t *vm);
void op_cmp_handler(vm_t *vm);
void op_jump_handler(vm_t *vm);
void op_jnz_handler(vm_t *vm);
void op_jz_handler(vm_t *vm);
void op_loop_handler(vm_t *vm);
void op_print_handler(vm_t *vm);

#endif // INCLUDE_VM_H_
