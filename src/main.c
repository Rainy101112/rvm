/* 
 *
 *      main.c
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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>

#include "vm.h"
#include "bytecode.h"
#include "logger.h"

int main(int argc, char *argv[]) {
    /* Apply RVM_LOGGER_LEVEL before the first (inline-checked) log call,
     * so DEBUG-level tracing via the environment actually turns on. */
    logger_init();

    if (argc < 2) {
        printf("Usage: <RVM> [FILE]\n");
        return 0;
    }
    
    size_t memsize = 0xffff;    // Set VM memory size

    /* Get memory size from argv */
    if (argc >= 3) {
        char *endptr = NULL;
        errno = 0;
        unsigned long parsed = strtoul(argv[2], &endptr, 0);
        if (endptr == argv[2] || *endptr != '\0' || errno == ERANGE) {
            logger_error("Invalid memory size: '%s'\n", argv[2]);
            return 1;
        }
        memsize = (size_t)parsed;
        if (memsize == 0) {
            memsize = 0xffff;
        }
        if (memsize > RVM_MAX_MEMSIZE) {
            logger_error("Memory size too large: %zu (max %zu)\n",
                         memsize, (size_t)RVM_MAX_MEMSIZE);
            return 1;
        }
    }

    clock_t start = 0, finish = 0;
    start = clock();    // Get current time

    binfile_t fstruct = binfile_get(argv[1]);   // Get byte code

    if (fstruct.buffer == NULL) {
        logger_error("Operation terminated.\n");
        return 1;
    }

    printf("File size: %zu bytes\n", fstruct.file_size);

    printf("Hex dump:\n");      // Print byte code
    for (size_t i = 0; i < fstruct.file_size; i++) {
        printf("%02x ", fstruct.buffer[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");

    vm_t vm;
    if (!vm_init(&vm, fstruct.buffer, fstruct.file_size, memsize)) {  // Create VM
        binfile_free(&fstruct);
        return 1;
    }

    /* Get max step limit from argv (0 = unlimited) */
    if (argc >= 4) {
        char *endptr = NULL;
        errno = 0;
        unsigned long long parsed = strtoull(argv[3], &endptr, 0);
        if (endptr == argv[3] || *endptr != '\0' || errno == ERANGE) {
            logger_error("Invalid max steps: '%s'\n", argv[3]);
            free(vm.memory);
            free(vm.stack);
            binfile_free(&fstruct);
            return 1;
        }
        vm.max_steps = (size_t)parsed;
        printf("Max steps: %zu%s\n", vm.max_steps,
               vm.max_steps == 0 ? " (unlimited)" : "");
    }

    vm_run(&vm);

    free(vm.memory);
    free(vm.stack);
    binfile_free(&fstruct);

    finish = clock();

    double total_time = (double)(finish - start) / CLOCKS_PER_SEC;  // Get total time usage
    printf("Total time: %f seconds\n", total_time);

    return 0;
}
