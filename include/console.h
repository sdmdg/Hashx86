#ifndef CONSOLE_H
#define CONSOLE_H

// Base address for video memory in text mode
#define VIDEO_MEMORY_ADDRESS 0xb8000  

// Screen dimensions for text mode (80x25 characters)
#define SCREEN_WIDTH 80               
#define SCREEN_HEIGHT 25              

// Macro for logging debug messages with a consistent format
#define DEBUG_LOG(format, ...) DebugPrintf("[DEBUG]:", format, ##__VA_ARGS__)

// Enum for text colors to represent foreground and background colors
typedef enum {
    BLACK = 0x0,          // Black color
    BLUE = 0x1,           // Blue color
    GREEN = 0x2,          // Green color
    CYAN = 0x3,           // Cyan color
    RED = 0x4,            // Red color
    MAGENTA = 0x5,        // Magenta color
    BROWN = 0x6,          // Brown color
    LIGHT_GRAY = 0x7,     // Light gray color
    DARK_GRAY = 0x8,      // Dark gray color
    LIGHT_BLUE = 0x9,     // Light blue color
    LIGHT_GREEN = 0xA,    // Light green color
    LIGHT_CYAN = 0xB,     // Light cyan color
    LIGHT_RED = 0xC,      // Light red color
    LIGHT_MAGENTA = 0xD,  // Light magenta color
    YELLOW = 0xE,         // Yellow color
    WHITE = 0xF           // White color
} TextColor;

// Function to print formatted text with a specified color
void printf(TextColor color, const char* format, ...);

// Function to print debug messages with a specific tag and format
void DebugPrintf(const char* tag, const char* format, ...);

// Function to clear the entire screen
void clearScreen();

// Function to scroll the screen upwards when the buffer is full
void scrollScreen();

// Function to combine foreground and background colors into a single byte
TextColor combineColors(TextColor foreground, TextColor background);

#endif // CONSOLE_H
