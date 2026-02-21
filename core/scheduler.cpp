/**
 * @file        scheduler.cpp
 * @brief       Standard Round-Robin Scheduler with State Queues for #x86
 *
 * @date        11/02/2026
 * @version     1.0.0
 */

#define KDBG_COMPONENT "SCHEDULER"
#include <core/scheduler.h>

extern TaskStateSegment g_tss;

// Virtual address for user-mode stacks (just below the 3GB hardware boundary)
#define USER_STACK_VIRT_TOP 0xC0000000

// Virtual address for the user-mode thread exit trampoline (1GB mark, in user space)
#define USER_EXIT_TRAMPOLINE_VIRT 0x40000000

#define KERNEL_STACK_SIZE (64 * 1024)

// Number of pages per User-Mode stack (16KB)
#define USER_STACK_PAGES 4

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

// This function acts as the "return address" for all kernel threads.
void ThreadExit() {
    if (Scheduler::activeInstance) {
        Scheduler::activeInstance->ExitCurrentThread();
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

    // Allocate and write a user-mode exit trampoline
    // Must be in identity-mapped range (<256MB)
    _trampolinePhys = (uint32_t)pmm_alloc_block_low(256 * 1024 * 1024);
    if (!_trampolinePhys) {
        HALT("CRITICAL: Failed to allocate trampoline page!");
    }
    memset((void*)_trampolinePhys, 0, 4096);

    // Position-Independent Code
    /*
    0xB8 0x01 0x00 0x00 0x00   →   mov eax, 1
    0xCD 0x80                  →   int 0x80
    0xEB 0xFE                  →   jmp $ (relative jump to itself)
    */
    uint8_t* code = (uint8_t*)_trampolinePhys;

    // mov eax, 1  (sys_exit)
    code[0] = 0xB8;
    code[1] = 0x01;
    code[2] = 0x00;
    code[3] = 0x00;
    code[4] = 0x00;
    // int 0x80
    code[5] = 0xCD;
    code[6] = 0x80;
    // jmp $  (safe infinite loop — hlt is privileged and would #GP in Ring 3)
    code[7] = 0xEB;
    code[8] = 0xFE;

    idleThread = CreateThread(nullptr, IdleTask, nullptr);
    KDBG1("Scheduler initialized. Trampoline=0x%x", _trampolinePhys);
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

    // Map the user-mode exit trampoline into this process's address space
    if (!isKernel) {
        _pager->MapPage(pcb->page_directory, USER_EXIT_TRAMPOLINE_VIRT, _trampolinePhys,
                        PAGE_PRESENT | PAGE_USER);
    }

    // Create the main thread (Stack setup)
    CreateThread(pcb, entrypoint, arg);

    // Register
    globalProcessList.PushBack(pcb);
    KDBG1("CreateProcess PID=%d Kernel=%d", pcb->pid, isKernel);
    return pcb;
}

ThreadControlBlock* Scheduler::CreateThread(ProcessControlBlock* parent, void (*entrypoint)(void*),
                                            void* arg) {
    InterruptGuard guard;
    ThreadControlBlock* tcb = new ThreadControlBlock();

    tcb->tid = _tidCounter++;
    tcb->parent = parent;
    tcb->pid = parent ? parent->pid : 0;

    // Allocate 64KB kernel stack
    tcb->stack = (uint8_t*)kmalloc(KERNEL_STACK_SIZE);

    // Calculate the TOP of the stack
    uint32_t* stackTop = (uint32_t*)(tcb->stack + KERNEL_STACK_SIZE);

    // Map the context struct to the top of the kernel stack
    tcb->context = (CPUState*)((uint8_t*)stackTop - sizeof(CPUState));
    memset(tcb->context, 0, sizeof(CPUState));

    // Determine if this is a Kernel or User thread
    bool isKernel = (parent == nullptr || parent->isKernelProcess);

    // COMMON SETUP
    tcb->context->eax = 0;
    tcb->context->ebx = 0;
    tcb->context->eip = (uint32_t)entrypoint;
    tcb->context->eflags = 0x202;  // Interrupts Enabled

    if (isKernel) {
        // KERNEL THREAD (Ring 0)
        tcb->context->cs = 0x08;
        tcb->context->ds = 0x10;
        tcb->context->es = 0x10;
        tcb->context->fs = 0x10;
        tcb->context->gs = 0x10;

        // The fake stack frame inside the struct
        tcb->context->esp = (uint32_t)ThreadExit;  // [ESP]   = Return Address
        tcb->context->ss = (uint32_t)arg;          // [ESP+4] = Argument
    } else {
        // USER THREAD (Ring 3)
        tcb->context->cs = 0x1B;  // User Code (0x18 | 3)
        tcb->context->ds = 0x23;  // User Data (0x20 | 3)
        tcb->context->es = 0x23;
        tcb->context->fs = 0x23;
        tcb->context->gs = 0x23;

        // Allocate USER-MODE stack (USER_STACK_PAGES pages)
        // Must be in identity-mapped range (<256MB) because kernel writes arg/retaddr to it
        uint32_t user_stack_size = USER_STACK_PAGES * PAGE_SIZE;
        uint32_t user_stack_base =
            USER_STACK_VIRT_TOP - (tcb->tid * user_stack_size) - user_stack_size;
        uint32_t top_page_phys = 0;

        for (uint32_t p = 0; p < USER_STACK_PAGES; p++) {
            uint32_t phys = (uint32_t)pmm_alloc_block_low(256 * 1024 * 1024);
            if (!phys) {
                KDBG1("CreateThread: Failed to allocate user stack page %d! Low Memory Exhausted?",
                      p);
                // Free previously allocated pages
                for (uint32_t q = 0; q < p; q++) {
                    uint32_t va = user_stack_base + q * PAGE_SIZE;
                    uint32_t pf = _pager->GetPhysicalAddress(parent->page_directory, va);
                    if (pf) pmm_free_block((void*)pf);
                }
                kfree(tcb->stack);
                delete tcb;
                return nullptr;
            }
            uint32_t vaddr = user_stack_base + p * PAGE_SIZE;
            if (!_pager->MapPage(parent->page_directory, vaddr, phys,
                                 PAGE_PRESENT | PAGE_RW | PAGE_USER)) {
                KDBG1("CreateThread: Failed to map user stack page %d!", p);
                pmm_free_block((void*)phys);
                for (uint32_t q = 0; q < p; q++) {
                    uint32_t va = user_stack_base + q * PAGE_SIZE;
                    uint32_t pf = _pager->GetPhysicalAddress(parent->page_directory, va);
                    if (pf) pmm_free_block((void*)pf);
                }
                kfree(tcb->stack);
                delete tcb;
                return nullptr;
            }
            if (p == USER_STACK_PAGES - 1) top_page_phys = phys;
        }

        // Write arg and return address to the TOP of the stack (highest page, last 8 bytes)
        uint32_t* user_stack_top_phys = (uint32_t*)(top_page_phys + PAGE_SIZE);
        user_stack_top_phys[-1] = (uint32_t)arg;              // Argument
        user_stack_top_phys[-2] = USER_EXIT_TRAMPOLINE_VIRT;  // Return to exit trampoline

        // IRET will pop SS:ESP for Ring 0 -> Ring 3 transition
        tcb->context->esp = user_stack_base + user_stack_size - 8;
        tcb->context->ss = 0x23;
    }

    if (parent != nullptr) {
        parent->threads.PushBack(tcb);
        tcb->state = THREAD_STATE_READY;
        readyQueue.PushBack(tcb);
    }

    if (arg == nullptr) {
        KDBG1("WARNING: Thread TID %d created with NULL arg!", tcb->tid);
    }

    KDBG1("CreateThread TID=%d PID=%d EIP=0x%x", tcb->tid, tcb->pid, entrypoint);
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

    // RESOURCE CLEANUP START
    uint32_t currentCR3;
    asm volatile("mov %%cr3, %0" : "=r"(currentCR3));
    if ((uint32_t)target->page_directory == currentCR3) {
        // Switch to Kernel Page Directory to safely free resources
        _pager->SwitchDirectory(_pager->KernelPageDirectory);
    }

    // Free User Stacks (USER_STACK_PAGES pages per thread)
    int tCount = target->threads.GetSize();
    for (int i = 0; i < tCount; i++) {
        ThreadControlBlock* t = target->threads.PopFront();

        // Calculate user stack base addr (matches CreateThread layout)
        uint32_t user_stack_size = USER_STACK_PAGES * PAGE_SIZE;
        uint32_t user_stack_base =
            USER_STACK_VIRT_TOP - (t->tid * user_stack_size) - user_stack_size;
        for (uint32_t p = 0; p < USER_STACK_PAGES; p++) {
            uint32_t vaddr = user_stack_base + p * PAGE_SIZE;
            uint32_t phys = _pager->GetPhysicalAddress(target->page_directory, vaddr);
            if (phys > 0) {
                pmm_free_block((void*)phys);
            }
        }

        TerminateThread(t);
    }

    // Free Heap Pages
    if (target->heap.startAddress > 0 && target->heap.endAddress > target->heap.startAddress) {
        for (uint32_t addr = target->heap.startAddress; addr < target->heap.endAddress;
             addr += PAGE_SIZE) {
            uint32_t phys = _pager->GetPhysicalAddress(target->page_directory, addr);
            if (phys > 0) {
                pmm_free_block((void*)phys);
            }
        }
    }

    // Free Page Tables and Page Directory (if not Kernel)
    if (!target->isKernelProcess) {
        // Free User Page Tables (Indices 64 to 768)
        // Kernel tables (0-63) and High Mem (768-1023) are shared, CANNOT FREE
        for (int i = 64; i < 768; i++) {
            if (target->page_directory[i] & PAGE_PRESENT) {
                uint32_t tablePhys = target->page_directory[i] & 0xFFFFF000;
                pmm_free_block((void*)tablePhys);
                target->page_directory[i] = 0;
            }
        }
        // Free the Directory itself
        pmm_free_block(target->page_directory);
    }

    // RESOURCE CLEANUP END

    // Remove from Global List
    globalProcessList.Remove([target](ProcessControlBlock* p) { return p == target; });

    delete target;

    KDBG1("KillProcess PID=%d success", pid);
    return true;
}

void Scheduler::TerminateThread(ThreadControlBlock* thread) {
    if (!thread) return;
    if (thread->state == THREAD_STATE_TERMINATED) return;

    KDBG1("TerminateThread TID=%d", thread->tid);

    // Null out currentThread BEFORE freeing/deleting.
    // Otherwise Schedule() will dereference dangling pointer.
    if (thread == currentThread) {
        currentThread = nullptr;
    }

    thread->state = THREAD_STATE_TERMINATED;
    readyQueue.Remove([thread](ThreadControlBlock* t) { return t == thread; });
    blockedQueue.Remove([thread](ThreadControlBlock* t) { return t == thread; });

    // Remove from parent's thread list to prevent KillProcess from
    // iterating over a dangling pointer later.
    if (thread->parent) {
        thread->parent->threads.Remove([thread](ThreadControlBlock* t) { return t == thread; });
    }

    if (thread->stack) {
        kfree((void*)thread->stack);
        thread->stack = nullptr;
    }
    delete thread;
}

bool Scheduler::ExitCurrentThread() {
    if (!currentThread) return false;

    ProcessControlBlock* parent = currentThread->parent;

    if (!parent) {
        // Kernel thread without parent process
        TerminateThread(currentThread);
        return false;
    }

    // Count non-terminated threads in the parent process
    int activeThreadCount = 0;
    int totalThreads = parent->threads.GetSize();
    for (int i = 0; i < totalThreads; i++) {
        ThreadControlBlock* thread = parent->threads.PopFront();
        if (thread->state != THREAD_STATE_TERMINATED) {
            activeThreadCount++;
        }
        parent->threads.PushBack(thread);
    }

    // Last thread standing -> kill the entire process
    if (activeThreadCount <= 1) {
        KDBG1("Thread TID %d is last in process PID %d - terminating process", currentThread->tid,
              parent->pid);
        KillProcess(parent->pid);
        return true;
    } else {
        KDBG1("Thread TID %d exiting, %d threads remain in process PID %d", currentThread->tid,
              activeThreadCount - 1, parent->pid);
        TerminateThread(currentThread);
        return false;
    }
}

void Scheduler::Sleep(uint32_t milliseconds) {
    InterruptGuard guard;
    if (!currentThread) return;
    currentThread->wakeTime = timerTicks + milliseconds;
    currentThread->state = THREAD_STATE_BLOCKED;
    // KDBG3("Sleep TID=%d ms=%d", currentThread->tid, milliseconds);
}

void Scheduler::WakeThread(ThreadControlBlock* thread) {
    InterruptGuard guard;
    if (!thread) return;
    if (thread->state != THREAD_STATE_BLOCKED) return;
    thread->state = THREAD_STATE_READY;
    thread->wakeTime = 0;
    blockedQueue.Remove([thread](ThreadControlBlock* t) { return t == thread; });
    readyQueue.PushBack(thread);
    // KDBG3("WakeThread TID=%d", thread->tid);
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

    // KDBG3("Switching to TID=%d, PID=%d, EIP=0x%x, ESP=0x%x", currentThread->tid,
    //        currentThread->pid, currentThread->context->eip, currentThread->context->esp);

    g_tss.esp0 = (uint32_t)(currentThread->stack + KERNEL_STACK_SIZE);

    if (currentThread->parent) {
        _pager->SwitchDirectory((currentThread->parent->page_directory));
    } else {
        _pager->SwitchDirectory((_pager->KernelPageDirectory));
    }

    return currentThread->context;
}
