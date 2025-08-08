#include <utils/string.h>

int strlen(const char *s) {
    int len = 0;
    while (*s++)
        len++;
    return len;
}

int strcmp(const char *s1, char *s2) {
    int i = 0;

    while ((s1[i] == s2[i])) {
        if (s2[i++] == 0)
            return 0;
    }
    return 1;
}

int strcpy(char *dst, const char *src) {
    int i = 0;
    while ((*dst++ = *src++) != 0)
        i++;
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
    if ((c >= 'a') && (c <= 'z'))
        return (c - 32);
    return c;
}

char lower(char c) {
    if ((c >= 'A') && (c <= 'Z'))
        return (c + 32);
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

    if (is_negative)
        *p++ = '-';

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

