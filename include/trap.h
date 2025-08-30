/* 
 *
 *      trap.h
 * 
 *      By Rainy101112 2025/8/30
 *      Public under MIT license
 * 
 *      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *      IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *      FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * 
 */

#ifndef INCLUDE_TRAP_H_
#define INCLUDE_TRAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "vm.h"

enum trap_number {
    TRAP_PUTC = 0,  // Print character to stdout
    TRAP_GETC,      // Get character from stdin
};

void trap_putc(vm_t *vm, uint8_t reg);
void trap_getc(vm_t *vm, uint8_t reg);

#endif // INCLUDE_TRAP_H_
