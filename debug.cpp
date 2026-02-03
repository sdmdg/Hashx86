/**
 * @file        debug.cpp
 * @brief       Debug with Ring Buffer & Fast Serial
 *
 * @date        17/01/2025
 * @version     1.1.0
 */

#include <core/Iguard.h>
#include <debug.h>

// --- Ring Buffer Configuration ---
#define SERIAL_BUFFER_SIZE 2048
static char serialBuffer[SERIAL_BUFFER_SIZE];
static volatile uint32_t readHead = 0;
static volatile uint32_t writeHead = 0;

void vprintf(const char* format, va_list args);

void initSerial() {
    outb(0x3F8 + 1, 0x00);  // Disable interrupts
    outb(0x3F8 + 3, 0x80);  // Enable DLAB (set baud rate divisor)

    // Set baud rate to max speed (115200 Baud)
    outb(0x3F8 + 0, 0x01);  // Set divisor to 1 (lo byte)
    outb(0x3F8 + 1, 0x00);  // Set divisor to 1 (hi byte)

    outb(0x3F8 + 3, 0x03);  // 8 bits, no parity, one stop bit
    outb(0x3F8 + 2, 0xC7);  // Enable FIFO, clear them, 14-byte threshold
    outb(0x3F8 + 4, 0x0B);  // IRQs enabled, RTS/DSR set
}

// Helper: Check if Serial Port is ready to transmit
bool IsSerialReady() {
    return (inb(0x3F8 + 5) & 0x20) != 0;
}

void FlushSerial() {
    // Keep flushing as long as hardware is ready AND we have data
    while (readHead != writeHead && IsSerialReady()) {
        char c = serialBuffer[readHead];
        outb(0x3F8, c);
        readHead = (readHead + 1) % SERIAL_BUFFER_SIZE;
    }
}

void SerialPush(char c) {
    // Push to buffer first
    uint32_t nextHead = (writeHead + 1) % SERIAL_BUFFER_SIZE;
    if (nextHead != readHead) {
        serialBuffer[writeHead] = c;
        writeHead = nextHead;
    }

    // Attempt to flush immediately if hardware is ready
    // This ensures logs continue even if scheduler is slow
    FlushSerial();
}

void writeSerial(char c) {
    SerialPush(c);
}

void SerialPrint(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        writeSerial(str[i]);
    }
}

void printf(const char* format, ...) {
    InterruptGuard guard;  // Protects the formatting & buffer push
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
                    char buffer[12];
                    int index = 11;
                    buffer[index] = '\0';

                    if (num == 0) {
                        buffer[--index] = '0';
                    } else if (num == -2147483648) {
                        const char* minStr = "-2147483648";
                        for (int j = 0; minStr[j] != '\0'; j++) writeSerial(minStr[j]);
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
                    for (int j = index; buffer[j] != '\0'; j++) writeSerial(buffer[j]);
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
                    for (int j = index; buffer[j] != '\0'; j++) writeSerial(buffer[j]);
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
                    for (int j = index; buffer[j] != '\0'; j++) writeSerial(buffer[j]);
                    break;
                }
                case 's': {
                    const char* str = va_arg(args, const char*);
                    for (int j = 0; str[j] != '\0'; j++) writeSerial(str[j]);
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
    InterruptGuard guard;
    // This runs extremely fast now (microseconds) because it only writes to RAM
    printf("%s:", tag);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void Printf(const char* tag, const char* format, ...) {
    InterruptGuard guard;
    printf("%s:", tag);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
