#ifndef STRING_H
#define STRING_H

#include <core/memory.h>
#include <types.h>

int strlen(const char *s);

int strcmp(const char *s1, const char *s2);

int strcpy(char *dst, const char *src);

void strcat(char *dest, const char *src);

int strncmp(const char *s1, const char *s2, size_t n);

int isspace(char c);

int isalpha(char c);
char upper(char c);
char lower(char c);

void itoa(char *buf, int base, int d);
int atoi(const char *str);
uint32_t HexStrToInt(const char *str);

#endif
