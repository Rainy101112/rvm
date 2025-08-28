#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>

#include "instruction.h"
#include "vm.h"
#include "bytecode.h"
#include "logger.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: <RVM> [FILE]\n");
        return 0;
    }

    clock_t start = 0, finish = 0;
    start = clock();

    binfile_t fstruct = binfile_get(argv[1]);

    if (fstruct.buffer == NULL) {
        logger_error("Operation terminated.\n");
        return 1;
    }

    #if defined(_WIN32)
        printf("File size: %lld bytes\n", fstruct.file_size);
    #else
        printf("File size: %ld bytes\n", fstruct.file_size);
    #endif

    printf("Hex dump:\n");
    for (size_t i = 0; i < fstruct.file_size; i++) {
        printf("%02x ", fstruct.buffer[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");

    vm_t vm;
    size_t memsize = 0xffff;
    vm_init(&vm, fstruct.buffer, fstruct.file_size, memsize);
    vm_run(&vm);

    free(vm.memory);
    binfile_free(&fstruct);

    finish = clock();

    double total_time = (double)(finish - start) / CLOCKS_PER_SEC;
    printf("Total time: %f seconds\n", total_time);

    return 0;
}
