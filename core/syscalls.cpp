/**
 * @file        syscalls.cpp
 * @brief       System Calls Interface for #x86
 *
 * @date        20/01/2026
 * @version     1.0.0-beta
 */

#include <core/syscalls.h>

SyscallHandler::SyscallHandler(uint8_t InterruptNumber, InterruptManager* interruptManager)
    : InterruptHandler(InterruptNumber + 0x20, interruptManager) {}

SyscallHandler::~SyscallHandler() {}

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    int32_t* return_data = (int32_t*)cpu->edx;

    switch ((uint32_t)cpu->eax) {
        case sys_restart:
            SyscallHandlers::Handle_sys_restart(esp);
            break;

        case sys_exit:
            SyscallHandlers::Handle_sys_exit(esp);
            break;

        case sys_clone:
            SyscallHandlers::Handle_sys_clone(esp);
            break;

        case sys_sleep:
            SyscallHandlers::Handle_sys_sleep(esp);
            break;

        case sys_debug:
            SyscallHandlers::Handle_sys_debug(esp);
            break;

        case sys_Hcall:
            SyscallHandlers::Handle_sys_Hcall(esp);
            break;

        default:
            DEBUG_LOG("Unknown system call at 0x80: %u\n", cpu->eax);
            break;
    }

    return esp;
}

void SyscallHandlers::Handle_sys_restart(uint32_t esp) {
    DEBUG_LOG("sys_restart\n");

    // Use a triple fault to restart the system (Not the best way, but for now this is good :) )
    asm volatile(
        "cli;"
        "lidt (%0);"  // Load an invalid IDT
        "int3;"       // Trigger an interrupt
        ::"r"(0));
}

void SyscallHandlers::Handle_sys_exit(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    uint32_t currentPID = Scheduler::activeInstance->GetCurrentProcess()->pid;
    Desktop::activeInstance->RemoveAppByPID(currentPID);
    HguiHandler::activeInstance->RemoveAppByPID(currentPID);

    Scheduler::activeInstance->KillProcess(currentPID);

    DEBUG_LOG("sys_exit: Exit status: %u, PID: %d\n", cpu->ebx, currentPID);
}

void SyscallHandlers::Handle_sys_clone(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    int32_t* return_data = (int32_t*)cpu->edx;
    DEBUG_LOG("sys_clone: Creating a new Thread");

    ProcessControlBlock* current_process = Scheduler::activeInstance->GetCurrentProcess();
    *return_data = (int32_t)Scheduler::activeInstance->CreateThread(
        current_process, reinterpret_cast<void (*)(void*)>(cpu->ebx),
        reinterpret_cast<void*>(cpu->ecx));
}

void SyscallHandlers::Handle_sys_sleep(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    ThreadControlBlock* t = Scheduler::activeInstance->GetCurrentThread();
    // DEBUG_LOG("sys_sleep: Sleeping TID : %d", t->tid);
    Scheduler::activeInstance->Sleep((cpu->ebx));
}

void SyscallHandlers::Handle_sys_debug(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    char* userString = (char*)cpu->ebx;

    // ATOMIC PRINT
    InterruptGuard guard;
    printf("%s", userString);
}

void SyscallHandlers::Handle_sys_Hcall(uint32_t esp) {
    DEBUG_LOG("sys_Hcall:");
    CPUState* cpu = (CPUState*)esp;
    ProcessControlBlock* current_process = Scheduler::activeInstance->GetCurrentProcess();

    multi_para_model* data = (multi_para_model*)cpu->ecx;
    int32_t* return_data = (int32_t*)cpu->edx;

    if (cpu->ebx == Hsys_getHeap) {
        // Write heap addresses to the pointers provided in param0 and param1
        data->param0 = current_process->heap.startAddress;
        data->param1 = current_process->heap.endAddress;
        *return_data = 1;
    } else if (cpu->ebx == Hsys_regEventH) {
        DEBUG_LOG("Hsys_regEventH: Creating a new Thread for handler");
        void* threadArgs = (void*)data->param0;
        void* entryPoint = (void*)data->param1;

        ThreadControlBlock* thread = Scheduler::activeInstance->CreateThread(
            current_process, reinterpret_cast<void (*)(void*)>(entryPoint),
            reinterpret_cast<void*>(threadArgs));

        Desktop::activeInstance->createNewHandler(current_process->pid, thread);

        *return_data = (uint32_t)thread->tid;
    } else {
        // Default case (optional: handle unknown Hcalls)
        DEBUG_LOG("Unknown Hcall ID: %d", cpu->ebx);
    }
}
