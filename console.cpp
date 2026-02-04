/**
 * @file        console.cpp
 * @brief       Console
 *
 * @date        13/01/2025
 * @version     1.0.0
 */

#include <console.h>

int cursorRow = 0;
int cursorCol = 0;

Port8Bit port_1(0x3D4);
Port8Bit port_2(0x3D5);

TextColor combineColors(TextColor foreground, TextColor background) {
    return (TextColor)((background << 4) | foreground);
}

// Simple Wrapper Function
void MSGPrintf(TextColor cTag, const char* tag, const char* format, ...) {
    printf(cTag, "[%s]", tag);  // Print the tag
    printf(LIGHT_GRAY, ":");
    if (!format || !*format) {
        return;  // Do nothing if the format is null or empty
    }

    // Check if the format string contains placeholders
    bool hasPlaceholders = false;
    for (const char* p = format; *p != '\0'; p++) {
        if (*p == '%') {
            hasPlaceholders = true;
            break;
        }
    }

    // If there are no placeholders, print the format string as is
    if (!hasPlaceholders) {
        printf(LIGHT_GRAY, format);
        return;
    }

    // Process the format string with arguments
    va_list args;
    va_start(args, format);

    for (const char* p = format; *p != '\0'; p++) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case 's': {  // String
                    const char* str = va_arg(args, const char*);
                    printf(LIGHT_GRAY, str);
                    break;
                }
                case 'd': {  // Decimal integer
                    int num = va_arg(args, int);
                    printf(LIGHT_GRAY, "%d", num);
                    break;
                }
                case 'x': {  // Hexadecimal
                    int num = va_arg(args, int);
                    printf(LIGHT_GRAY, "%x", num);
                    break;
                }
                default:
                    printf(LIGHT_GRAY, "%%");
                    printf(LIGHT_GRAY, "%c", *p);
                    break;
            }
        } else {
            printf(LIGHT_GRAY, "%c", *p);
        }
    }

    va_end(args);
}

void scrollScreen() {
    unsigned short* VideoMemory = (unsigned short*)VIDEO_MEMORY_ADDRESS;

    // Scroll all rows up by one
    for (int row = 1; row < SCREEN_HEIGHT; row++) {
        for (int col = 0; col < SCREEN_WIDTH; col++) {
            VideoMemory[(row - 1) * SCREEN_WIDTH + col] = VideoMemory[row * SCREEN_WIDTH + col];
        }
    }

    // Clear the last row
    unsigned short blank = 0x20 | (WHITE << 8);  // Space with white text on black background
    for (int col = 0; col < SCREEN_WIDTH; col++) {
        VideoMemory[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + col] = blank;
    }

    // Adjust the cursor position if it moves out of bounds
    if (cursorRow > SCREEN_HEIGHT - 1) {
        cursorRow = SCREEN_HEIGHT - 1;
    }
}

void updateCursor(int row, int col) {
    unsigned short position = row * SCREEN_WIDTH + col;

    // Set cursor start and enable blinking
    port_1.Write(0x0A);  // Cursor Start Register
    port_2.Write(0x06);  // Start at scanline 6 (enables blinking)

    // Set cursor end
    port_1.Write(0x0B);  // Cursor End Register
    port_2.Write(0x0F);  // End at scanline 15

    // Update the cursor position
    port_1.Write(0x0E);  // High byte of cursor position
    port_2.Write((position >> 8) & 0xFF);

    port_1.Write(0x0F);  // Low byte of cursor position
    port_2.Write(position & 0xFF);
}

void clearScreen() {
    unsigned short* VideoMemory = (unsigned short*)VIDEO_MEMORY_ADDRESS;
    unsigned short blank = 0x20 | (WHITE << 8);  // Space with white text on black background

    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        VideoMemory[i] = blank;
    }

    cursorRow = 0;
    cursorCol = 0;
    updateCursor(0, 0);
}

void printf(TextColor color, const char* format, ...) {
    unsigned short* VideoMemory = (unsigned short*)VIDEO_MEMORY_ADDRESS;
    va_list args;
    va_start(args, format);

    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%') {
            i++;
            switch (format[i]) {
                case 'd': {  // Signed Integer
                    int num = va_arg(args, int);
                    char buffer[12];       // Enough for -2147483648 + '\0'
                    int index = 11;        // Start filling from the end
                    buffer[index] = '\0';  // Null-terminate

                    if (num == 0) {
                        buffer[--index] = '0';
                    } else if (num == -2147483648) {  // Special case for INT_MIN
                        const char* minStr = "-2147483648";
                        for (int j = 0; minStr[j] != '\0'; j++) {
                            int position = cursorRow * SCREEN_WIDTH + cursorCol;
                            VideoMemory[position] = (color << 8) | minStr[j];
                            cursorCol++;
                            if (cursorCol >= SCREEN_WIDTH) {
                                cursorCol = 0;
                                cursorRow++;
                            }
                            if (cursorRow >= SCREEN_HEIGHT) {
                                scrollScreen();
                                cursorRow = SCREEN_HEIGHT - 1;
                            }
                        }
                        break;  // Exit the case here
                    } else {
                        bool isNegative = (num < 0);
                        if (isNegative) num = -num;

                        while (num > 0) {
                            buffer[--index] = (num % 10) + '0';
                            num /= 10;
                        }

                        if (isNegative) buffer[--index] = '-';
                    }

                    for (int j = index; buffer[j] != '\0'; j++) {
                        int position = cursorRow * SCREEN_WIDTH + cursorCol;
                        VideoMemory[position] = (color << 8) | buffer[j];
                        cursorCol++;
                        if (cursorCol >= SCREEN_WIDTH) {
                            cursorCol = 0;
                            cursorRow++;
                        }
                        if (cursorRow >= SCREEN_HEIGHT) {
                            scrollScreen();
                            cursorRow = SCREEN_HEIGHT - 1;
                        }
                    }
                    break;
                }

                case 'u': {  // Unsigned Integer
                    uint32_t num = va_arg(args, uint32_t);
                    char buffer[11];       // Enough for 0 to 4294967295
                    int index = 10;        // Start filling from the end
                    buffer[index] = '\0';  // Null-terminate

                    if (num == 0) {
                        buffer[--index] = '0';
                    } else {
                        while (num > 0) {
                            buffer[--index] = (num % 10) + '0';
                            num /= 10;
                        }
                    }

                    for (int j = index; buffer[j] != '\0'; j++) {
                        int position = cursorRow * SCREEN_WIDTH + cursorCol;
                        VideoMemory[position] = (color << 8) | buffer[j];
                        cursorCol++;
                        if (cursorCol >= SCREEN_WIDTH) {
                            cursorCol = 0;
                            cursorRow++;
                        }
                        if (cursorRow >= SCREEN_HEIGHT) {
                            scrollScreen();
                            cursorRow = SCREEN_HEIGHT - 1;
                        }
                    }
                    break;
                }

                case 'x': {  // Hexadecimal
                    uint32_t num = va_arg(args, uint32_t);
                    char buffer[9];        // Enough for 8 hex digits + '\0'
                    int index = 8;         // Start filling from the end
                    buffer[index] = '\0';  // Null-terminate
                    const char* hexDigits = "0123456789ABCDEF";

                    if (num == 0) {
                        buffer[--index] = '0';
                    } else {
                        while (num > 0) {
                            buffer[--index] = hexDigits[num % 16];
                            num /= 16;
                        }
                    }

                    for (int j = index; buffer[j] != '\0'; j++) {
                        int position = cursorRow * SCREEN_WIDTH + cursorCol;
                        VideoMemory[position] = (color << 8) | buffer[j];
                        cursorCol++;
                        if (cursorCol >= SCREEN_WIDTH) {
                            cursorCol = 0;
                            cursorRow++;
                        }
                        if (cursorRow >= SCREEN_HEIGHT) {
                            scrollScreen();
                            cursorRow = SCREEN_HEIGHT - 1;
                        }
                    }
                    break;
                }

                case 's': {  // String
                    const char* str = va_arg(args, const char*);
                    for (int j = 0; str[j] != '\0'; j++) {
                        int position = cursorRow * SCREEN_WIDTH + cursorCol;
                        VideoMemory[position] = (color << 8) | str[j];
                        cursorCol++;

                        if (cursorCol >= SCREEN_WIDTH) {
                            cursorCol = 0;
                            cursorRow++;
                        }

                        if (cursorRow >= SCREEN_HEIGHT) {
                            scrollScreen();
                            cursorRow = SCREEN_HEIGHT - 1;
                        }
                    }
                    break;
                }

                default:
                    break;
            }
        } else if (format[i] == '\n') {
            cursorRow++;
            cursorCol = 0;  // Reset column to the beginning

            // Handle scrolling if we exceed the screen height
            if (cursorRow >= SCREEN_HEIGHT) {
                scrollScreen();
                cursorRow = SCREEN_HEIGHT - 1;  // Move cursor to the last row
                cursorCol = 0;                  // Ensure cursor starts at the beginning of the row
            }

        } else {
            int position = cursorRow * SCREEN_WIDTH + cursorCol;
            VideoMemory[position] = (color << 8) | format[i];
            cursorCol++;

            if (cursorCol >= SCREEN_WIDTH) {
                cursorCol = 0;
                cursorRow++;
            }

            if (cursorRow >= SCREEN_HEIGHT) {
                scrollScreen();
                cursorRow = SCREEN_HEIGHT - 1;
            }
        }
    }
    va_end(args);
    updateCursor(cursorRow, cursorCol);
}
