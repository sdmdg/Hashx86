#ifndef DEBUG_H
#define DEBUG_H

#include <core/ports.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <types.h>

void initSerial();
void SerialPrint(const char* str);
void writeSerial(char c);

#ifndef KDBG_ENABLE
#define KDBG_ENABLE 0
#endif

#ifndef KDBG_LEVEL
#define KDBG_LEVEL 0
#endif

#ifndef KDBG_COMPONENT
#define KDBG_COMPONENT "GEN"
#endif

#define KDBG__EMIT(level, component, format, ...)              \
    do {                                                       \
        printf("[%s] " format "\n", component, ##__VA_ARGS__); \
    } while (0)

#if KDBG_ENABLE && (KDBG_LEVEL >= 1)
#define KDBG_L1(component, format, ...) KDBG__EMIT(1, component, format, ##__VA_ARGS__)
#define KDBG1(format, ...) KDBG__EMIT(1, KDBG_COMPONENT, format, ##__VA_ARGS__)
#else
#define KDBG_L1(component, format, ...) ((void)0)
#define KDBG1(format, ...) ((void)0)
#endif

#if KDBG_ENABLE && (KDBG_LEVEL >= 2)
#define KDBG_L2(component, format, ...) KDBG__EMIT(2, component, format, ##__VA_ARGS__)
#define KDBG2(format, ...) KDBG__EMIT(2, KDBG_COMPONENT, format, ##__VA_ARGS__)
#else
#define KDBG_L2(component, format, ...) ((void)0)
#define KDBG2(format, ...) ((void)0)
#endif

#if KDBG_ENABLE && (KDBG_LEVEL >= 3)
#define KDBG_L3(component, format, ...) KDBG__EMIT(3, component, format, ##__VA_ARGS__)
#define KDBG3(format, ...) KDBG__EMIT(3, KDBG_COMPONENT, format, ##__VA_ARGS__)
#else
#define KDBG_L3(component, format, ...) ((void)0)
#define KDBG3(format, ...) ((void)0)
#endif

/**
 * @brief Macro for logging debug messages with a consistent format.
 *
 * Logs debug messages prefixed with "[DEBUG]". Accepts a format string and
 * optional arguments to customize the message.
 *
 * @param format Format string for the debug message.
 * @param ... Additional arguments for the format string.
 */
#define DEBUG_LOG(format, ...) DebugPrintf("[DEBUG]", format, ##__VA_ARGS__)

/**
 * @brief Macro for printing messages from modules with a consistent format.
 *
 * Logs messages prefixed with the specified `tag`. The text is displayed in
 * light blue color by default.
 *
 * @param tag The tag identifying the module or context of the message.
 * @param format Format string for the message.
 * @param ... Additional arguments for the format string.
 */
#define PRINT(tag, format, ...) Printf(tag, format, ##__VA_ARGS__)

/**
 * @brief Prints debug messages with a specific tag and format.
 *
 * Logs debug messages prefixed with a tag. Useful for debugging purposes.
 *
 * @param tag The tag identifying the source of the debug message.
 * @param format Format string for the message.
 * @param ... Additional arguments for the format string.
 */

// printf Function for serial monitor
void printf(const char* format, ...);

// Simple Debug Wrapper Function
void DebugPrintf(const char* tag, const char* format, ...);

// Simple Printf Wrapper Function
void Printf(const char* tag, const char* format, ...);

#define HALT(msg)            \
    printf(msg);             \
    do {                     \
        asm volatile("hlt"); \
    } while (1)

#endif  // DEBUG_H
