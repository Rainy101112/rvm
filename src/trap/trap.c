/* 
 *
 *      trap.c
 * 
 *      By Rainy101112 2025/8/30
 *      Public under MIT license
 * 
 *      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *      IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *      FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

#include "trap.h"
#include "logger.h"

void trap_putc(vm_t *vm, uint8_t reg){
    int ch = vm->registers[reg];

    putc(ch, stdout);
}

void trap_getc(vm_t *vm, uint8_t reg) {
    int ch;
    
    #ifdef _WIN32
        ch = _getch();
    #else
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
        ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    #endif
    
    vm->registers[reg] = (size_t)ch;
    logger_print("TRAP_GETC: R%d = '%c' (0x%x)\n", reg, ch, ch);
    
    return;
}
