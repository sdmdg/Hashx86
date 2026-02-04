/**
 * @file        debug.cpp
 * @brief       Debug
 *
 * @date        17/01/2025
 * @version     1.0.0
 */

#include <Hx86/debug.h>

void vprintf(const char* format, va_list args);

void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

// Helper to write to a buffer
void buffer_char(char* buffer, int* pIndex, int maxLen, char c) {
    if (*pIndex < maxLen - 1) {
        buffer[(*pIndex)++] = c;
    }
}

void vprintf(const char* format, va_list args) {
    char output[256];  // Stack buffer
    int idx = 0;

    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%') {
            i++;
            switch (format[i]) {
                case 'd': {
                    int num = va_arg(args, int);
                    char tmp[12];
                    int tIdx = 11;
                    tmp[tIdx] = '\0';
                    bool neg = (num < 0);
                    if (neg) num = -num;

                    if (num == 0)
                        tmp[--tIdx] = '0';
                    else {
                        while (num > 0) {
                            tmp[--tIdx] = (num % 10) + '0';
                            num /= 10;
                        }
                    }
                    if (neg) tmp[--tIdx] = '-';

                    // Copy tmp to main buffer
                    for (int k = tIdx; tmp[k]; k++) buffer_char(output, &idx, 256, tmp[k]);
                    break;
                }
                case 's': {
                    const char* s = va_arg(args, const char*);
                    for (int k = 0; s[k]; k++) buffer_char(output, &idx, 256, s[k]);
                    break;
                }
                case 'x': {
                    uint32_t num = va_arg(args, uint32_t);
                    char tmp[9];
                    int tIdx = 8;
                    tmp[tIdx] = '\0';
                    const char* hex = "0123456789ABCDEF";
                    if (num == 0)
                        tmp[--tIdx] = '0';
                    else {
                        while (num > 0) {
                            tmp[--tIdx] = hex[num % 16];
                            num /= 16;
                        }
                    }
                    for (int k = tIdx; tmp[k]; k++) buffer_char(output, &idx, 256, tmp[k]);
                    break;
                }
                // Add other cases (u, c, etc) here...
                default:
                    break;
            }
        } else {
            buffer_char(output, &idx, 256, format[i]);
        }
    }

    output[idx] = '\0';  // Null terminate

    // SEND TO KERNEL IN ONE GO
    syscall_debug(output);
}

void DebugPrintf(const char* tag, const char* format, ...) {
    printf("%s:", tag);  // Print the tag and colon

    va_list args;
    va_start(args, format);
    vprintf(format, args);  // Use vprintf to handle `va_list`
    va_end(args);
    printf("\n");  // Print the newline chr
}

void Printf(const char* tag, const char* format, ...) {
    printf("%s:", tag);  // Print the tag and colon
    va_list args;
    va_start(args, format);
    vprintf(format, args);  // Use vprintf to handle `va_list`
    va_end(args);
}
