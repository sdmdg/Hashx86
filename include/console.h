#ifndef CONSOLE_H
#define CONSOLE_H

#include <core/ports.h>
#include <stdarg.h>

// Base address for video memory in text mode
#define VIDEO_MEMORY_ADDRESS 0xb8000  

// Screen dimensions for text mode (80x25 characters)
#define SCREEN_WIDTH 80                ///< Number of columns on the screen
#define SCREEN_HEIGHT 25               ///< Number of rows on the screen

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
#define PRINT(tag, format, ...) MSGPrintf(LIGHT_BLUE, tag, format, ##__VA_ARGS__)

/**
 * @brief Enum representing text colors for foreground and background.
 * 
 * Each value corresponds to a specific color code used in text mode.
 */
typedef enum {
    BLACK = 0x0,          ///< Black color
    BLUE = 0x1,           ///< Blue color
    GREEN = 0x2,          ///< Green color
    CYAN = 0x3,           ///< Cyan color
    RED = 0x4,            ///< Red color
    MAGENTA = 0x5,        ///< Magenta color
    BROWN = 0x6,          ///< Brown color
    LIGHT_GRAY = 0x7,     ///< Light gray color
    DARK_GRAY = 0x8,      ///< Dark gray color
    LIGHT_BLUE = 0x9,     ///< Light blue color
    LIGHT_GREEN = 0xA,    ///< Light green color
    LIGHT_CYAN = 0xB,     ///< Light cyan color
    LIGHT_RED = 0xC,      ///< Light red color
    LIGHT_MAGENTA = 0xD,  ///< Light magenta color
    YELLOW = 0xE,         ///< Yellow color
    WHITE = 0xF           ///< White color
} TextColor;

/**
 * @brief Prints formatted text with a specified color.
 * 
 * Displays text on the screen with the given color. Accepts a format string
 * and optional arguments.
 * 
 * @param color Text color for the output.
 * @param format Format string for the message.
 * @param ... Additional arguments for the format string.
 */
void printf(TextColor color, const char* format, ...);

/**
 * @brief Prints debug messages with a specific tag and format.
 * 
 * Logs debug messages prefixed with a tag. Useful for debugging purposes.
 * 
 * @param tag The tag identifying the source of the debug message.
 * @param format Format string for the message.
 * @param ... Additional arguments for the format string.
 */
void DebugPrintf(const char* tag, const char* format, ...);

/**
 * @brief Prints messages from modules with a specific tag and format.
 * 
 * Logs messages prefixed with a tag. The color of the tag is customizable.
 * 
 * @param cTag Color for the tag.
 * @param tag The tag identifying the module or context.
 * @param format Format string for the message.
 * @param ... Additional arguments for the format string.
 */
void MSGPrintf(TextColor cTag, const char* tag, const char* format, ...);

/**
 * @brief Clears the entire screen.
 * 
 * Resets the screen to a blank state by clearing all text and resetting the cursor.
 */
void clearScreen();

/**
 * @brief Scrolls the screen upwards when the buffer is full.
 * 
 * Moves all visible text up by one row to make space for new text at the bottom.
 */
void scrollScreen();

/**
 * @brief Combines foreground and background colors into a single byte.
 * 
 * Used to encode color information for text mode.
 * 
 * @param foreground The foreground color.
 * @param background The background color.
 * @return TextColor The combined color.
 */
TextColor combineColors(TextColor foreground, TextColor background);

#endif // CONSOLE_H
