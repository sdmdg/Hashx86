#ifndef InterruptGuard_H
#define InterruptGuard_H

#include <types.h>

class InterruptGuard {
    bool wasEnabled;

public:
    InterruptGuard() {
        uint32_t eflags;
        // 1. Save current EFLAGS register
        asm volatile(
            "pushf\n\t"
            "pop %0"
            : "=r"(eflags));

        // 2. Check if Interrupt Flag (IF) bit (0x200) was set
        wasEnabled = (eflags & 0x200);

        // 3. Disable Interrupts
        asm volatile("cli");
    }

    ~InterruptGuard() {
        // 4. Only re-enable if they were enabled before!
        if (wasEnabled) {
            asm volatile("sti");
        }
    }
};

#endif  // InterruptGuard_H
