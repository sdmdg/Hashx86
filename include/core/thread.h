#ifndef THREAD_H
#define THREAD_H

#include <types.h>
#include <core/gdt.h>
#include <debug.h>
#include <core/memory.h>

struct CPUState
{
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp;
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

public:
    Thread(Process* parent, GlobalDescriptorTable *gdt, void (*entrypoint)(void*), void* arg);
    ~Thread();
    uint32_t getTID() { return tid; }
    Process* getParentProcess() { return parentProcess; }
};

#endif // THREAD_H
