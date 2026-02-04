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
        heapData = syscall_heap();
        heap_init((void*)heapData.param0, (void*)heapData.param1);
        printf("[PROG] : Heap :- 0x%x - 0x%x\n", heapData.param0, heapData.param1);
    }
}
