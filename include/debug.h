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

#endif  // DEBUG_H
