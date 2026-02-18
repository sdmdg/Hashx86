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
    sys_sleep = 7,
    sys_sbrk = 8,
    sys_peek_memory = 9,
    sys_clone = 41,
    sys_Hcall = 199,
    sys_debug = 200,
} SYSCALL;

typedef enum {
    Hsys_getHeap = 0,
    Hsys_regEventH = 1,
    Hsys_getFramebuffer = 2,
    Hsys_getInput = 3,
    Hsys_readFile = 4
} HSYSCALL;

// Input state structure for Hsys_getInput
struct InputState {
    uint8_t keyStates[128];
    int32_t mouseDX;
    int32_t mouseDY;
    uint8_t mouseButtons;
} __attribute__((packed));

// Framebuffer info structure
struct FramebufferInfo {
    uint32_t buffer;
    uint32_t width;
    uint32_t height;
};

struct HeapData {
    uint32_t param0;
    uint32_t param1;
};

struct multi_para_model {
    uint32_t param0;
    uint32_t param1;
    uint32_t param2;
    uint32_t param3;
    uint32_t param4;
};

// void outb(uint16_t portNumber, uint8_t value);

void syscall_exit(uint32_t status);
HeapData syscall_heap();
uint32_t syscall_register_event_handler(void (*entrypoint)(void*), void* arg);
uint32_t syscall_clone(void (*entrypoint)(void*), void* arg);
void syscall_sleep(uint32_t ms);
void syscall_debug(const char* str);
uint32_t syscall_peek_memory(uint32_t address, uint32_t size);
int32_t syscall_sbrk(int32_t increment);
uint32_t syscall_Hgui(uint32_t element, uint32_t mode, void* data);

FramebufferInfo syscall_get_framebuffer();
void syscall_get_input(InputState* state);
int32_t syscall_read_file(const char* filename, uint8_t* buffer, uint32_t maxSize,
                          uint32_t* actualSize);

#endif  // SYSCALLS_H
