#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <core/Iguard.h>
#include <core/gdt.h>
#include <core/globals.h>
#include <core/memory.h>
#include <core/paging.h>
#include <core/process_types.h>
#include <core/tss.h>

class Scheduler {
private:
    LinkedList<ProcessControlBlock*> globalProcessList;
    // STATE QUEUES
    LinkedList<ThreadControlBlock*> readyQueue;       // Runnable threads
    LinkedList<ThreadControlBlock*> blockedQueue;     // Sleeping threads
    LinkedList<ThreadControlBlock*> terminatedQueue;  // Dead threads

    uint32_t _pidCounter;
    uint32_t _tidCounter;
    Paging* _pager;
    uint32_t _trampolinePhys;  // Physical page holding user-mode exit trampoline code

public:
    static Scheduler* activeInstance;
    ThreadControlBlock* currentThread;
    ThreadControlBlock* idleThread;

    Scheduler(Paging* pager);

    // CREATION & MANAGEMENT
    ProcessControlBlock* CreateProcess(bool isKernel, void (*entrypoint)(void*), void* arg);
    ThreadControlBlock* CreateThread(ProcessControlBlock* parent, void (*entrypoint)(void*),
                                     void* arg);

    bool KillProcess(uint32_t pid);
    void TerminateThread(ThreadControlBlock* thread);
    bool ExitCurrentThread();
    void Sleep(uint32_t milliseconds);

    // CORE SCHEDULING (Called by Interrupt Handler)
    CPUState* Schedule(CPUState* context);

    // Helpers
    ThreadControlBlock* GetCurrentThread() {
        return currentThread;
    }
    ProcessControlBlock* GetCurrentProcess() {
        return currentThread ? currentThread->parent : nullptr;
    }
    Paging* GetPager() {
        return _pager;
    }
};

#endif  // SCHEDULER_H
