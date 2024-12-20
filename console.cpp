#include "console.h"
#include "core/port.h"
#include <stdarg.h>

int cursorRow = 0;
int cursorCol = 0;

Port8Bit port_1(0x3D4);
Port8Bit port_2(0x3D5);

TextColor combineColors(TextColor foreground, TextColor background) {
    return (TextColor)((background << 4) | foreground);
}


// Simple Debug Wrapper Function
void DebugPrintf(const char* tag, const char* format, ...) {
    printf(CYAN, "%s", tag);    // Print the tag

    va_list args;
    va_start(args, format);
    printf(CYAN, format, args); // Use the core printf
    va_end(args);

    printf(CYAN, "\n");         // Add newline for better output separation
}

void scrollScreen() {
    unsigned short* VideoMemory = (unsigned short*)VIDEO_MEMORY_ADDRESS;

    for (int row = 1; row < SCREEN_HEIGHT; row++) {
        for (int col = 0; col < SCREEN_WIDTH; col++) {
            VideoMemory[(row - 1) * SCREEN_WIDTH + col] = VideoMemory[row * SCREEN_WIDTH + col];
        }
    }

    unsigned short blank = 0x20 | (WHITE << 8); // Space with white on black
    for (int col = 0; col < SCREEN_WIDTH; col++) {
        VideoMemory[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + col] = blank;
    }
}

void updateCursor(int row, int col) {
    unsigned short position = row * SCREEN_WIDTH + col;

    port_1.Write(14);
    port_2.Write(position >> 8);
    port_1.Write(15);
    port_2.Write(position & 0xFF);
}

void clearScreen() {
    unsigned short* VideoMemory = (unsigned short*)VIDEO_MEMORY_ADDRESS;
    unsigned short blank = 0x20 | (WHITE << 8); // Space with white text on black background

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
                case 'd': { // Signed Integer
                    int num = va_arg(args, int);
                    char buffer[12]; // Enough for -2147483648 + '\0'
                    int index = 11;  // Start filling from the end
                    buffer[index] = '\0'; // Null-terminate

                    if (num == 0) {
                        buffer[--index] = '0';
                    } else if (num == -2147483648) { // Special case for INT_MIN
                        const char* minStr = "-2147483648";
                        for (int j = 0; minStr[j] != '\0'; j++) {
                            int position = cursorRow * SCREEN_WIDTH + cursorCol;
                            VideoMemory[position] = (color << 8) | minStr[j];
                            cursorCol++;
                            if (cursorCol >= SCREEN_WIDTH) { cursorCol = 0; cursorRow++; }
                            if (cursorRow >= SCREEN_HEIGHT) { scrollScreen(); cursorRow = SCREEN_HEIGHT - 1; }
                        }
                        break; // Exit the case here
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
                        if (cursorCol >= SCREEN_WIDTH) { cursorCol = 0; cursorRow++; }
                        if (cursorRow >= SCREEN_HEIGHT) { scrollScreen(); cursorRow = SCREEN_HEIGHT - 1; }
                    }
                    break;
                }


                case 'u': { // Unsigned Integer
                    uint32_t num = va_arg(args, uint32_t);
                    char buffer[11];        // Enough for 0 to 4294967295
                    int index = 10;         // Start filling from the end
                    buffer[index] = '\0';   // Null-terminate

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
                        if (cursorCol >= SCREEN_WIDTH) { cursorCol = 0; cursorRow++; }
                        if (cursorRow >= SCREEN_HEIGHT) { scrollScreen(); cursorRow = SCREEN_HEIGHT - 1; }
                    }
                    break;
                }

                case 's': { // String
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
            cursorCol = 0;
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
