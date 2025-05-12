#include <Hx86/Hsyscalls/syscalls.h>

/* void outb(uint16_t portNumber, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(portNumber));
} */

void syscall_exit(uint32_t status)
{
    asm volatile (
        "int $0x80"
        : 
        : "a" (sys_exit), "b" (status)
    );
    while (1);
}

HeapData syscall_heap()
{
    uint32_t retun_data1;
    uint32_t retun_data2;
    asm volatile (
        "int $0x80"
        : 
        : "a" (sys_Hcall), "b" (Hsys_getHeap), "c" ((void*)&retun_data1), "d" ((void*)&retun_data2)
    );

    while (!retun_data1 && !retun_data2);
    HeapData retun_data = {retun_data1, retun_data2};
    return (HeapData)retun_data;
}

uint32_t syscall_clone(void (*entrypoint)(void*), void* arg)
{
    int32_t retun_data = -1;
    asm volatile (
        "int $0x80"
        : 
        : "a" (sys_clone), "b" (entrypoint), "c" (arg), "d" ((void*)&retun_data)
    );

    while (retun_data == -1);
    return (uint32_t)retun_data;
}

void syscall_debug(char c)
{
    asm volatile (
        "int $0x80"
        : 
        : "a" (sys_debug), "b" (c)
    );
}

uint32_t syscall_Hgui(uint32_t element, uint32_t mode, void* data)
{
    int32_t retun_data;
    asm volatile (
        "int $0x81"
        : 
        : "a" (element), "b" (mode), "c" (data), "d" ((void*)&retun_data)
    );

    while (!retun_data);
    return (uint32_t)retun_data;
}