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
