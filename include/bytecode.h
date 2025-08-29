/* 
 *
 *      bytecode.h
 * 
 *      By Rainy101112 2025/8/28
 *      Public under MIT license
 * 
 *      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *      IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *      FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * 
 */

#ifndef INCLUDE_BYTECODE_H_
#define INCLUDE_BYTECODE_H_

#include <stdint.h>

struct byte_code_file {
    uint8_t *buffer;
    size_t file_size;
};

typedef struct byte_code_file binfile_t;

binfile_t binfile_get(const char *filename);
void binfile_free(binfile_t *file);

#endif // INCLUDE_BYTECODE_H_
