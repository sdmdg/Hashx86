#ifndef THREAD_H
#define THREAD_H

#include <types.h>
#include <core/gdt2.h>
#include <debug.h>
#include <core/memory.h>

struct CPUState
{
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp;
    uint32_t ds, es, fs, gs;
    uint32_t error, eip, cs, eflags, esp, ss;
} __attribute__((packed));

class Process; // Forward declaration

class Thread
{
friend class Process;
friend class ProcessManager;
private:
    uint8_t stack[8192]; // 8 KB stack per thread
    CPUState* cpustate;
    uint32_t tid;
    Process* parentProcess;
    uint8_t kernelStack[8192];

public:
    Thread(Process* parent, void (*entrypoint)(void*), void* arg);
    ~Thread();
    uint32_t getTID() { return tid; }
    Process* getParentProcess() { return parentProcess; }

    uint32_t kernelStackTop();
};

#endif // THREAD_H
