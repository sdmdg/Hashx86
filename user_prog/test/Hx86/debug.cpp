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
                             syscall_debug(minStr[j]);
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
                         syscall_debug(buffer[j]);
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
                         syscall_debug(buffer[j]);
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
                         syscall_debug(buffer[j]);
                     }
                     break;
                 }
 
                 case 's': {
                     const char* str = va_arg(args, const char*);
                     for (int j = 0; str[j] != '\0'; j++) {
                         syscall_debug(str[j]);
                     }
                     break;
                 }
 
                 default:
                     break;
             }
         } else {
             syscall_debug(format[i]);
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