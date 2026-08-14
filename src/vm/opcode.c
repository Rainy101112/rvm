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

/* Thin wrappers for the public handler API. The dispatch loop in vm.c
 * inlines the implementations from opcode_impl.h directly. */

#include "opcode_impl.h"

void op_load_handler(vm_t *vm)    { op_load_impl(vm); }
void op_la_handler(vm_t *vm)      { op_la_impl(vm); }
void op_sa_handler(vm_t *vm)      { op_sa_impl(vm); }
void op_mov_handler(vm_t *vm)     { op_mov_impl(vm); }
void op_add_handler(vm_t *vm)     { op_add_impl(vm); }
void op_sub_handler(vm_t *vm)     { op_sub_impl(vm); }
void op_multi_handler(vm_t *vm)   { op_multi_impl(vm); }
void op_divide_handler(vm_t *vm)  { op_divide_impl(vm); }
void op_increase_handler(vm_t *vm) { op_increase_impl(vm); }
void op_decrease_handler(vm_t *vm) { op_decrease_impl(vm); }
void op_and_handler(vm_t *vm)     { op_and_impl(vm); }
void op_not_handler(vm_t *vm)     { op_not_impl(vm); }
void op_or_handler(vm_t *vm)      { op_or_impl(vm); }
void op_xor_handler(vm_t *vm)     { op_xor_impl(vm); }
void op_cmp_handler(vm_t *vm)     { op_cmp_impl(vm); }
void op_jump_handler(vm_t *vm)    { op_jump_impl(vm); }
void op_jnz_handler(vm_t *vm)     { op_jnz_impl(vm); }
void op_jz_handler(vm_t *vm)      { op_jz_impl(vm); }
void op_loop_handler(vm_t *vm)    { op_loop_impl(vm); }
void op_push_handler(vm_t *vm)    { op_push_impl(vm); }
void op_pop_handler(vm_t *vm)     { op_pop_impl(vm); }
void op_call_handler(vm_t *vm)    { op_call_impl(vm); }
void op_ret_handler(vm_t *vm)     { op_ret_impl(vm); }
void op_trap_handler(vm_t *vm)    { op_trap_impl(vm); }
void op_print_handler(vm_t *vm)   { op_print_impl(vm); }
