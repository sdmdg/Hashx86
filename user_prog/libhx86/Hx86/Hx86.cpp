/**
 * @file        Hx86.cpp
 * @brief       Core Hx86 System Initialization
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <Hx86/Hx86.h>
#include <Hx86/debug.h>

void init_sys(void* arg) {
    if (!args) {
        args = (ProgramArguments*)arg;

        // Initialize heap via brk
        int32_t heap_start = syscall_brk(0);  // Get current program break
        syscall_brk(256 * 1024);              // Grow 256KB initial heap
        int32_t heap_end = syscall_brk(0);    // Get new program break
        heap_init((void*)heap_start, (void*)heap_end);
        printf("[PROG] : Heap :- 0x%x - 0x%x\n", heap_start, heap_end);
    }
}

void syscall_sleep(uint32_t ms) {
    struct timespec req;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000;
    syscall_nanosleep(&req, nullptr);
}
