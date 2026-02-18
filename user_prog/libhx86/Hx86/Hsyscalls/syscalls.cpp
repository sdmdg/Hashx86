/**
 * @file        syscalls.cpp
 * @brief       Hx86 System Call Implementation
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <Hx86/Hsyscalls/syscalls.h>

void syscall_exit(uint32_t status = 0) {
    asm volatile("int $0x80" : : "a"(sys_exit), "b"(status));
}

HeapData syscall_heap() {
    int32_t retun_data = -1;

    uint32_t retun_data1 = 0;
    uint32_t retun_data2 = 0;
    multi_para_model data = {retun_data1, retun_data2, 0, 0, 0};
    asm volatile("int $0x80"
                 :
                 : "a"(sys_Hcall), "b"(Hsys_getHeap), "c"((void*)&data), "d"((void*)&retun_data));

    while (retun_data == -1);
    HeapData heapdata = {data.param0, data.param1};
    return heapdata;
}

uint32_t syscall_register_event_handler(void (*entrypoint)(void*), void* arg) {
    int32_t retun_data = -1;
    multi_para_model data = {(uint32_t)arg, (uint32_t)entrypoint, 0, 0, 0};
    asm volatile("int $0x80"
                 :
                 : "a"(sys_Hcall), "b"(Hsys_regEventH), "c"(&data), "d"((void*)&retun_data));

    while (retun_data == -1);
    return (uint32_t)retun_data;
}

uint32_t syscall_clone(void (*entrypoint)(void*), void* arg) {
    int32_t retun_data = -1;
    asm volatile("int $0x80"
                 :
                 : "a"(sys_clone), "b"(entrypoint), "c"(arg), "d"((void*)&retun_data));

    while (retun_data == -1);
    return (uint32_t)retun_data;
}

void syscall_sleep(uint32_t ms) {
    asm volatile("int $0x80" : : "a"(sys_sleep), "b"(ms));
}

void syscall_debug(const char* str) {
    asm volatile("int $0x80" : : "a"(sys_debug), "b"(str));
}

int32_t syscall_sbrk(int32_t increment) {
    int32_t return_data;
    asm volatile("int $0x80" : : "a"(sys_sbrk), "b"(increment), "d"(&return_data));
    return return_data;
}

uint32_t syscall_peek_memory(uint32_t address, uint32_t size) {
    int32_t return_data = 0;
    asm volatile("int $0x80"
                 :
                 : "a"(sys_peek_memory), "b"(address), "c"(size), "d"((void*)&return_data)
                 : "memory");
    return (uint32_t)return_data;
}

uint32_t syscall_Hgui(uint32_t element, uint32_t mode, void* data) {
    int32_t retun_data;
    asm volatile("int $0x81" : : "a"(element), "b"(mode), "c"(data), "d"((void*)&retun_data));

    while (!retun_data);
    return (uint32_t)retun_data;
}

FramebufferInfo syscall_get_framebuffer() {
    int32_t retun_data = -1;
    multi_para_model data = {0, 0, 0, 0, 0};
    asm volatile("int $0x80"
                 :
                 : "a"(sys_Hcall), "b"(Hsys_getFramebuffer), "c"((void*)&data),
                   "d"((void*)&retun_data)
                 : "memory");
    while (retun_data == -1);
    FramebufferInfo info;
    info.buffer = data.param0;
    info.width = data.param1;
    info.height = data.param2;
    return info;
}

void syscall_get_input(InputState* state) {
    int32_t retun_data = -1;
    multi_para_model data = {(uint32_t)state, 0, 0, 0, 0};
    asm volatile("int $0x80"
                 :
                 : "a"(sys_Hcall), "b"(Hsys_getInput), "c"((void*)&data), "d"((void*)&retun_data)
                 : "memory");
    while (retun_data == -1);
}

int32_t syscall_read_file(const char* filename, uint8_t* buffer, uint32_t maxSize,
                          uint32_t* actualSize) {
    int32_t retun_data = -1;
    multi_para_model data = {(uint32_t)filename, (uint32_t)buffer, maxSize, 0, 0};
    asm volatile("int $0x80"
                 :
                 : "a"(sys_Hcall), "b"(Hsys_readFile), "c"((void*)&data), "d"((void*)&retun_data)
                 : "memory");
    // retun_data == -1 means failure; don't spin-wait
    if (actualSize) *actualSize = data.param3;
    return retun_data;
}
