/*
 *
 *      logger.h
 *
 *      By Rainy101112 2025/8/28
 *      Public under MIT license
 *
 *      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *      IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *      FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 */

#ifndef INCLUDE_LOGGER_H_
#define INCLUDE_LOGGER_H_

/* Log levels, in increasing order of severity. A message is shown when its
 * level is at or above the current level. */
enum log_levels {
    LOG_LEVEL_DEBUG,        // Show all messages
    LOG_LEVEL_INFO,         // Show info, warning and error messages
    LOG_LEVEL_WARNING,      // Show warning and error messages
    LOG_LEVEL_ERROR,        // Show error messages
    LOG_LEVEL_SILENT,       // Don't show any messages
};

/* Set the current log level. Out-of-range values are clamped. An explicit
 * call takes precedence over the RVM_LOGGER_LEVEL environment variable. */
void logger_set_level(const int level);

/* Internal implementation; do not call directly. The macros below pass the
 * call site so warnings and errors can report file and line. */
#if defined(__GNUC__)
void logger_log_impl(int level, const char *file, int line,
                     const char *format, ...)
    __attribute__((format(printf, 4, 5)));
#else
void logger_log_impl(int level, const char *file, int line,
                     const char *format, ...);
#endif

#define logger_debug(...)   logger_log_impl(LOG_LEVEL_DEBUG,   __FILE__, __LINE__, __VA_ARGS__)
#define logger_info(...)    logger_log_impl(LOG_LEVEL_INFO,    __FILE__, __LINE__, __VA_ARGS__)
#define logger_warning(...) logger_log_impl(LOG_LEVEL_WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define logger_error(...)   logger_log_impl(LOG_LEVEL_ERROR,   __FILE__, __LINE__, __VA_ARGS__)

#endif // INCLUDE_LOGGER_H_
