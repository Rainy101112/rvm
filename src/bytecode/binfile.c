#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "bytecode.h"

binfile_t binfile_get(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    binfile_t dummy = { .buffer = NULL, .file_size = 0 };

    if (fp == NULL) {
        logger_error("Error while opening file: %s\n", filename);
        return dummy;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size <= 0) {
        logger_error("Invalid file size or empty file: %s\n", filename);
        fclose(fp);
        return dummy;
    }

    uint8_t *buffer = (uint8_t *)malloc(file_size);
    if (buffer == NULL) {
        logger_error("Memory allocation failed\n");
        fclose(fp);
        return dummy;
    }

    size_t bytes_read = fread(buffer, 1, file_size, fp);
    if ((long)bytes_read != file_size) {
        logger_error("Error while reading file: expected %ld bytes, got %zu\n", 
                    file_size, bytes_read);
        free(buffer);
        fclose(fp);
        return dummy;
    }

    binfile_t file_struct = {
        .buffer = buffer,
        .file_size = (size_t)file_size
    };

    fclose(fp);
    return file_struct;
}

void binfile_free(binfile_t *file) {
    if (file && file->buffer) {
        free(file->buffer);
        file->buffer = NULL;
        file->file_size = 0;
    }
}
