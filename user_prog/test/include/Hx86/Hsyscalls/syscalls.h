#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <Hx86/stdint.h>

typedef enum {
    sys_restart = 0,
    sys_exit = 1,
    sys_fork = 2,
    sys_read = 3,
    sys_write = 4,
    sys_open = 5,
    sys_close = 6,
    sys_clone = 41,
    sys_Hcall = 199,
    sys_debug = 200,
} SYSCALL;

typedef enum {
    Hsys_getHeap = 0
} HSYSCALL;

struct HeapData {
    uint32_t param0;
    uint32_t param1;
};

//void outb(uint16_t portNumber, uint8_t value);

void syscall_exit(uint32_t status);
HeapData syscall_heap();
uint32_t syscall_clone(void (*entrypoint)(void*), void* arg);
void syscall_debug(char c);
uint32_t syscall_Hgui(uint32_t element, uint32_t mode, void* data);

#endif // SYSCALLS_H