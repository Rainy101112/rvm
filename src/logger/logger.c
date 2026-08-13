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
#include <stdlib.h>
#include <stdarg.h>

#include "logger.h"

/* Per-instruction tracing is suppressed when RVM_QUIET is set, so the VM
 * can be benchmarked without printf dominating the cost. Errors always
 * print; program output (PRT, TRAP_PUTC) is unaffected. */
static int quiet = -1;

static int logger_is_quiet(void) {
    if (quiet < 0) {
        const char *q = getenv("RVM_QUIET");
        quiet = (q != NULL && q[0] != '\0') ? 1 : 0;
    }
    return quiet;
}

void logger_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "[%s:%d] ", __FILE__, __LINE__);
    vfprintf(stderr, format, args);
    va_end(args);
}

void logger_print(const char *format, ...) {
    if (logger_is_quiet()) {
        return;
    }

    va_list args;
    va_start(args, format);
    fprintf(stdout, "[INFO] ");
    vfprintf(stdout, format, args);
    va_end(args);
}
