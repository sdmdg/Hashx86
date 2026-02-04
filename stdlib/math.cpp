/**
 * @file        math.cpp
 * @brief       Math Library Implementation
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <stdlib/math.h>

double sin(double x) {
    double res;
    asm volatile("fsin" : "=t"(res) : "0"(x));
    return res;
}

double cos(double x) {
    double res;
    asm volatile("fcos" : "=t"(res) : "0"(x));
    return res;
}

double tan(double x) {
    double res;
    // fptan pushes 1.0, then tan(x).
    // We execute fstp %st(0) to pop that extra 1.0 off the stack.
    asm volatile("fptan; fstp %%st(0)" : "=t"(res) : "0"(x));
    return res;
}

double sqrt(double x) {
    double res;
    asm volatile("fsqrt" : "=t"(res) : "0"(x));
    return res;
}

double abs(double x) {
    return (x < 0) ? -x : x;
}

double pow(double base, int exp) {
    double res = 1.0;
    for (int i = 0; i < exp; i++) res *= base;
    return res;
}

int floor(double x) {
    return (int)x;
}

int ceil(double x) {
    int intPart = (int)x;

    // If x is greater than its integer part (e.g. 2.3 > 2), round up.
    if (x > intPart) {
        return intPart + 1;
    }

    // Otherwise (e.g. 3.0 or -2.5), the integer part is the ceiling.
    return intPart;
}
