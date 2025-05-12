/**
 * @file        debug.cpp
 * @brief       Debug
 * 
 * @date        17/01/2025
 * @version     1.0.0
 */

#include <debug.h>

void vprintf(const char* format, va_list args);


void initSerial() {
    outb(0x3F8 + 1, 0x00); // Disable interrupts
    outb(0x3F8 + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(0x3F8 + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
    outb(0x3F8 + 1, 0x00); //                  (hi byte)
    outb(0x3F8 + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(0x3F8 + 2, 0xC7); // Enable FIFO, clear them, 14-byte threshold
    outb(0x3F8 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}


void writeSerial(char c) {
    while ((inb(0x3F8 + 5) & 0x20) == 0); // Wait for the transmit buffer to be empty
    outb(0x3F8, c);
}


void SerialPrint(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        writeSerial(str[i]);
    }
}


void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void vprintf(const char* format, va_list args) {
    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%') {
            i++;
            switch (format[i]) {
                case 'd': {
                    int num = va_arg(args, int);
                    char buffer[12]; // Enough for -2147483648 + '\0'
                    int index = 11;
                    buffer[index] = '\0';

                    if (num == 0) {
                        buffer[--index] = '0';
                    } else if (num == -2147483648) { // Special case for INT_MIN
                        const char* minStr = "-2147483648";
                        for (int j = 0; minStr[j] != '\0'; j++) {
                            writeSerial(minStr[j]);
                        }
                        break;
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
                        writeSerial(buffer[j]);
                    }
                    break;
                }

                case 'u': {
                    uint32_t num = va_arg(args, uint32_t);
                    char buffer[11];
                    int index = 10;
                    buffer[index] = '\0';

                    if (num == 0) {
                        buffer[--index] = '0';
                    } else {
                        while (num > 0) {
                            buffer[--index] = (num % 10) + '0';
                            num /= 10;
                        }
                    }

                    for (int j = index; buffer[j] != '\0'; j++) {
                        writeSerial(buffer[j]);
                    }
                    break;
                }

                case 'x': {
                    uint32_t num = va_arg(args, uint32_t);
                    char buffer[9];
                    int index = 8;
                    buffer[index] = '\0';
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
                        writeSerial(buffer[j]);
                    }
                    break;
                }

                case 's': {
                    const char* str = va_arg(args, const char*);
                    for (int j = 0; str[j] != '\0'; j++) {
                        writeSerial(str[j]);
                    }
                    break;
                }

                default:
                    break;
            }
        } else {
            writeSerial(format[i]);
        }
    }
}


void DebugPrintf(const char* tag, const char* format, ...) {
    printf("%s:", tag);    // Print the tag and colon

    va_list args;
    va_start(args, format);
    vprintf(format, args); // Use vprintf to handle `va_list`
    va_end(args);
    printf("\n");           // Print the newline chr
}


void Printf(const char* tag, const char* format, ...) {
    printf("%s:", tag);    // Print the tag and colon
    va_list args;
    va_start(args, format);
    vprintf(format, args); // Use vprintf to handle `va_list`
    va_end(args);
}