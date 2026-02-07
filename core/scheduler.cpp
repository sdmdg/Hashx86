/**
 * @file        scheduler.cpp
 * @brief       Standard Round-Robin Scheduler with State Queues for #x86
 *
 * @date        25/01/2026
 * @version     1.0.0-beta
 */

#include <core/scheduler.h>

Scheduler* Scheduler::activeInstance = nullptr;
void FlushSerial();

void IdleTask(void* arg) {
    while (1) {
        // Clear the log buffer to the screen
        FlushSerial();

        asm volatile("sti");
        asm volatile("hlt");
    }
}

// This function acts as the "return address" for all threads.
void ThreadExit() {
    Scheduler* sched = Scheduler::activeInstance;

    // Terminate the currently running thread
    if (sched && sched->currentThread) {
        sched->TerminateThread(sched->currentThread);
    }

    while (1) {
        asm volatile("hlt");
    }
}

Scheduler::Scheduler(Paging* pager) {
    _pager = pager;
    _pidCounter = 0;
    _tidCounter = 0;
    currentThread = nullptr;
    activeInstance = this;
    idleThread = CreateThread(nullptr, IdleTask, nullptr);
}

ProcessControlBlock* Scheduler::CreateProcess(bool isKernel, void (*entrypoint)(void*), void* arg) {
    ProcessControlBlock* pcb = new ProcessControlBlock();
    if (!pcb) {
        HALT("CRITICAL: Failed to allocate ProcessControlBlock!\n");
    }
    pcb->pid = _pidCounter++;
    pcb->isKernelProcess = isKernel;

    // MEMORY SPACE SETUP
    if (isKernel) {
        pcb->page_directory = _pager->KernelPageDirectory;
    } else {
        pcb->page_directory = _pager->CreateProcessDirectory();
    }

    // Create the main thread (Stack setup)
    CreateThread(pcb, entrypoint, arg);

    // Register
    globalProcessList.PushBack(pcb);
    return pcb;
}

ThreadControlBlock* Scheduler::CreateThread(ProcessControlBlock* parent, void (*entrypoint)(void*),
                                            void* arg) {
    InterruptGuard guard;
    ThreadControlBlock* tcb = new ThreadControlBlock();
    if (!tcb) {
        HALT("CRITICAL: Failed to allocate ThreadControlBlock!\n");
    }
    tcb->tid = _tidCounter++;
    tcb->stack = (uint8_t*)pmm_alloc_block();

    if (parent != nullptr) {
        tcb->pid = parent->pid;
        tcb->parent = parent;
    }
    tcb->state = THREAD_STATE_NEW;
    tcb->wakeTime = 0;

    // Point to the top of the stack
    uint32_t* stackPtr = (uint32_t*)(tcb->stack + sizeof(tcb->stack));

    // Map the CPUState struct directly to the top of the stack
    tcb->context = (CPUState*)((uint8_t*)stackPtr - sizeof(CPUState));
    // Zero out
    memset(tcb->context, 0, sizeof(CPUState));

    // Setup Standard Registers
    tcb->context->eax = 0;
    tcb->context->ebx = 0;
    tcb->context->eip = (uint32_t)entrypoint;
    tcb->context->cs = 0x08;
    tcb->context->eflags = 0x202;

    // Position ESP (Stack Top): Acts as the "Return Address"
    // When the function starts, [ESP] is the return address.
    tcb->context->esp = (uint32_t)ThreadExit;

    // Position ESP+4: Acts as the "First Argument"
    // Repurpose the 'ss' field to hold the pointer!
    tcb->context->ss = (uint32_t)arg;

    if (parent != nullptr) {
        parent->threads.PushBack(tcb);
        tcb->state = THREAD_STATE_READY;
        readyQueue.PushBack(tcb);
    }

    if (arg == nullptr) {
        DEBUG_LOG("WARNING: Thread TID %d created with NULL arg!", tcb->tid);
    }

    return tcb;
}

bool Scheduler::KillProcess(uint32_t pid) {
    ProcessControlBlock* target = nullptr;
    int pCount = globalProcessList.GetSize();
    for (int i = 0; i < pCount; i++) {
        ProcessControlBlock* temp = globalProcessList.PopFront();
        if (temp->pid == pid) target = temp;
        globalProcessList.PushBack(temp);
        if (target) break;
    }
    if (!target) return false;

    int tCount = target->threads.GetSize();
    for (int i = 0; i < tCount; i++) {
        ThreadControlBlock* t = target->threads.PopFront();
        TerminateThread(t);
        target->threads.PushBack(t);
    }
    globalProcessList.Remove([target](ProcessControlBlock* p) { return p == target; });

    if (currentThread && currentThread->parent == target) {
        // Let next Schedule() handle switch
    }
    return true;
}

void Scheduler::TerminateThread(ThreadControlBlock* thread) {
    if (!thread) return;
    if (thread->state == THREAD_STATE_TERMINATED) return;
    thread->state = THREAD_STATE_TERMINATED;
    readyQueue.Remove([thread](ThreadControlBlock* t) { return t == thread; });
    blockedQueue.Remove([thread](ThreadControlBlock* t) { return t == thread; });
    terminatedQueue.PushBack(thread);

    if (thread->stack) {
        pmm_free_block((void*)thread->stack);
        thread->stack = nullptr;
    }
    delete thread;
}

void Scheduler::Sleep(uint32_t milliseconds) {
    InterruptGuard guard;
    if (!currentThread) return;
    currentThread->wakeTime = timerTicks + milliseconds;
    currentThread->state = THREAD_STATE_BLOCKED;
}

CPUState* Scheduler::Schedule(CPUState* context) {
    if (currentThread) {
        currentThread->context = context;
        if ((currentThread->state == THREAD_STATE_RUNNING) && currentThread != idleThread) {
            currentThread->state = THREAD_STATE_READY;
            readyQueue.PushBack(currentThread);
        } else if (currentThread->state == THREAD_STATE_BLOCKED) {
            blockedQueue.PushBack(currentThread);
        } else if (currentThread->state == THREAD_STATE_TERMINATED) {
            terminatedQueue.PushBack(currentThread);
        }
    }

    if (blockedQueue.GetSize() > 0) {
        int count = blockedQueue.GetSize();
        for (int i = 0; i < count; i++) {
            ThreadControlBlock* t = blockedQueue.PopFront();
            if (t->state == THREAD_STATE_BLOCKED && t->wakeTime <= timerTicks) {
                t->state = THREAD_STATE_READY;
                t->wakeTime = 0;
                readyQueue.PushBack(t);
            } else {
                blockedQueue.PushBack(t);
            }
        }
    }

    if (readyQueue.GetSize() == 0) {
        // No real work to do, Run the Idle Thread.
        currentThread = idleThread;
        currentThread->state = THREAD_STATE_RUNNING;
        _pager->SwitchDirectory((_pager->KernelPageDirectory));
        return currentThread->context;

    } else {
        // Normal Round Robin
        currentThread = readyQueue.PopFront();
    }
    currentThread->state = THREAD_STATE_RUNNING;

    // DEBUG_LOG("Switching to TID=%d, PID=%d, EIP=0x%x, ESP=0x%x", currentThread->tid,
    // currentThread->pid, currentThread->context->eip, currentThread->context->esp);

    _pager->SwitchDirectory((currentThread->parent->page_directory));

    return currentThread->context;
}
