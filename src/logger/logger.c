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

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "logger.h"

/* Default level. RVM_LOGGER_LEVEL overrides it unless logger_set_level()
 * is called first. Exposed so logger_level_enabled() can check it inline. */
int logger_log_level = LOG_LEVEL_INFO;
static int level_initialized = 0;

/* Case-insensitive string comparison (strcasecmp is not portable). */
static int logger_str_iequals(const char *a, const char *b) {
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

/* Parse a level name ("debug", "info", "warning", "error", "silent") or a
 * number 0-4; returns -1 if the string matches neither. */
static int logger_parse_level(const char *str) {
    if (logger_str_iequals(str, "debug")) {
        return LOG_LEVEL_DEBUG;
    }
    if (logger_str_iequals(str, "info")) {
        return LOG_LEVEL_INFO;
    }
    if (logger_str_iequals(str, "warning") || logger_str_iequals(str, "warn")) {
        return LOG_LEVEL_WARNING;
    }
    if (logger_str_iequals(str, "error")) {
        return LOG_LEVEL_ERROR;
    }
    if (logger_str_iequals(str, "silent") || logger_str_iequals(str, "quiet") ||
        logger_str_iequals(str, "off") || logger_str_iequals(str, "none")) {
        return LOG_LEVEL_SILENT;
    }

    char *endptr = NULL;
    long value = strtol(str, &endptr, 10);
    if (endptr != str && *endptr == '\0' &&
        value >= LOG_LEVEL_DEBUG && value <= LOG_LEVEL_SILENT) {
        return (int)value;
    }

    return -1;
}

/* Apply RVM_LOGGER_LEVEL once. */
void logger_init(void) {
    if (!level_initialized) {
        const char *env = getenv("RVM_LOGGER_LEVEL");
        if (env != NULL && env[0] != '\0') {
            int level = logger_parse_level(env);
            if (level >= 0) {
                logger_log_level = level;
            }
        }
        level_initialized = 1;
    }
}

void logger_set_level(const int level) {
    level_initialized = 1;    // Explicit call wins over RVM_LOGGER_LEVEL

    if (level < LOG_LEVEL_DEBUG) {
        logger_log_level = LOG_LEVEL_DEBUG;
    } else if (level > LOG_LEVEL_SILENT) {
        logger_log_level = LOG_LEVEL_SILENT;
    } else {
        logger_log_level = level;
    }
}

void logger_log_impl(int level, const char *file, int line,
                     const char *format, ...) {
    logger_init();

    if (level < logger_log_level) {
        return;
    }

    FILE *stream = (level >= LOG_LEVEL_WARNING) ? stderr : stdout;
    const char *name;

    switch (level) {
    case LOG_LEVEL_DEBUG:
        name = "DEBUG";
        break;
    case LOG_LEVEL_WARNING:
        name = "WARNING";
        break;
    case LOG_LEVEL_ERROR:
        name = "ERROR";
        break;
    case LOG_LEVEL_INFO:
        /* fall through */
    default:
        name = "INFO";
        break;
    }

    fprintf(stream, "[%s] ", name);
    if (level >= LOG_LEVEL_WARNING) {
        fprintf(stream, "%s:%d: ", file, line);
    }

    va_list args;
    va_start(args, format);
    vfprintf(stream, format, args);
    va_end(args);
}
