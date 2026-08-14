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

/* Apply RVM_LOGGER_LEVEL. Called automatically on the first log message and
 * by logger_set_level(); call it once at program start so that the inline
 * threshold checks below see the environment-configured level. */
void logger_init(void);

/* Current log level. Read it through logger_level_enabled() only. */
extern int logger_log_level;

/* Cheap inline threshold check; suppressed messages skip the formatting
 * call entirely (and their arguments are not evaluated). */
static inline int logger_level_enabled(const int level) {
    return level >= logger_log_level;
}

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

#define logger_debug(...) \
    do { \
        if (logger_level_enabled(LOG_LEVEL_DEBUG)) \
            logger_log_impl(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__); \
    } while (0)

#define logger_info(...) \
    do { \
        if (logger_level_enabled(LOG_LEVEL_INFO)) \
            logger_log_impl(LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__); \
    } while (0)

#define logger_warning(...) \
    do { \
        if (logger_level_enabled(LOG_LEVEL_WARNING)) \
            logger_log_impl(LOG_LEVEL_WARNING, __FILE__, __LINE__, __VA_ARGS__); \
    } while (0)

#define logger_error(...) \
    do { \
        if (logger_level_enabled(LOG_LEVEL_ERROR)) \
            logger_log_impl(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__); \
    } while (0)

#endif // INCLUDE_LOGGER_H_
