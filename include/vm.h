#ifndef INCLUDE_VM_H_
#define INCLUDE_VM_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// 虚拟机状态
struct vm_state {
    uint8_t registers[4];  // 4个通用寄存器
    uint8_t *memory;       // 内存数据指针
    size_t pc;             // 程序计数器
    bool running;          // 运行标志
    size_t code_size;      // 记录代码大小
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
