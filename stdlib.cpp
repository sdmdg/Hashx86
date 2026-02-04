/**
 * @file        stdlib.cpp
 * @brief       Standard Library Implementation
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <stdlib.h>

char* itoa(uint32_t num, char* str, uint32_t base) {
    uint32_t i = 0;
    bool isNegative = false;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    if (num < 0 && base == 10) {
        isNegative = true;
        num = -num;
    }

    while (num != 0) {
        uint32_t rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'A' : rem + '0';
        num = num / base;
    }

    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    // Reverse the string
    uint32_t start = 0, end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }

    return str;
}
