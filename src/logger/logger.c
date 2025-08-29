/* 
 *
 *      logger.c
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
#include <stdarg.h>

#include "logger.h"

void logger_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "[%s:%d] ", __FILE__, __LINE__);
    vfprintf(stderr, format, args);
    va_end(args);
}

void logger_print(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stdout, "[INFO] ");
    vfprintf(stdout, format, args);
    va_end(args);
}
