#include <core/thread.h>
#include <core/process.h>

Thread::Thread(Process* parent, void (*entrypoint)(void*), void* arg)
{
    this->parentProcess = parent;
    
    // Each thread gets its own stack space
    cpustate = (CPUState*)(stack + sizeof(stack) - sizeof(CPUState));

    // Set up stack
    uint32_t* stackTop = (uint32_t*)(stack + sizeof(stack));
    *(--stackTop) = (uint32_t)arg;

    cpustate->eax = cpustate->ebx = cpustate->ecx = cpustate->edx = 0;
    cpustate->esi = cpustate->edi = cpustate->ebp = 0;

    cpustate->eip = (uint32_t)entrypoint;
    cpustate->cs = 0x08;
    cpustate->eflags = 0x202;
    cpustate->esp = (uint32_t)stackTop;
    cpustate->ebp = (uint32_t)stackTop;
}

Thread::~Thread() {}
