#ifndef TIMING_H
#define TIMING_H

#include <types.h>

void static wait(uint32_t milliseconds) {
    const uint32_t iterations_per_ms = 1000000;  // Only a rough estimate
    volatile uint32_t count = milliseconds * iterations_per_ms;
    while (count--) {
        __asm__ volatile("nop");
    }
}

#endif  // TIMING_H
