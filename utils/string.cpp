/**
 * @file        string.cpp
 * @brief       String Utility Functions
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <utils/string.h>

int strlen(const char *s) {
    int len = 0;
    while (*s++) len++;
    return len;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n > 0) {
        unsigned char c1 = (unsigned char)*s1++;
        unsigned char c2 = (unsigned char)*s2++;
        if (c1 != c2) {
            return c1 - c2;
        }
        if (c1 == '\0') {
            return 0;
        }
        n--;
    }
    return 0;
}

int strcpy(char *dst, const char *src) {
    int i = 0;
    while ((*dst++ = *src++) != 0) i++;
    return i;
}

void strcat(char *dest, const char *src) {
    char *end = (char *)dest + strlen(dest);
    memcpy((void *)end, (void *)src, strlen(src));
    end = end + strlen(src);
    *end = '\0';
}

int isspace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

int isalpha(char c) {
    return (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')));
}

char upper(char c) {
    if ((c >= 'a') && (c <= 'z')) return (c - 32);
    return c;
}

char lower(char c) {
    if ((c >= 'A') && (c <= 'Z')) return (c + 32);
    return c;
}

void itoa(char *buf, int base, int d) {
    char *p = buf;
    char *p1, *p2;
    unsigned int ud = (unsigned int)d;
    int divisor = base;
    int is_negative = 0;

    if (base == 10 && d < 0) {
        is_negative = 1;
        ud = -d;
    }

    // Generate digits in reverse order
    do {
        int remainder = ud % divisor;
        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
    } while (ud /= divisor);

    if (is_negative) *p++ = '-';

    *p = 0;

    // Reverse the string
    p1 = buf;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}

// Standard ASCII to Integer (Base 10)
int atoi(const char *str) {
    int res = 0;
    int sign = 1;
    int i = 0;

    // Skip whitespace
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r') i++;

    // Handle sign
    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[i] == '+') {
        i++;
    }

    // Convert digits
    while (str[i] >= '0' && str[i] <= '9') {
        res = res * 10 + (str[i] - '0');
        i++;
    }

    return sign * res;
}

// Hex String to Unsigned Integer (Base 16)
uint32_t HexStrToInt(const char *str) {
    uint32_t result = 0;

    // Skip leading whitespace
    while (*str == ' ' || *str == '\t') str++;

    // Skip optional "0x" prefix
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
    }

    // Process characters
    while (*str) {
        char c = *str;
        uint32_t val = 0;

        if (c >= '0' && c <= '9') {
            val = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            val = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            val = c - 'A' + 10;
        } else {
            // Stop at first non-hex character (like space or null)
            break;
        }

        result = (result << 4) | val;  // Equivalent to: result * 16 + val
        str++;
    }
    return result;
}
