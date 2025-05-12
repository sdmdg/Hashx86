
#include <core/syscalls.h>

SyscallHandler::SyscallHandler(uint8_t InterruptNumber, InterruptManager* interruptManager)
:    InterruptHandler(InterruptNumber + 0x20, interruptManager)
{
}

SyscallHandler::~SyscallHandler()
{
}

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    int32_t* return_data = (int32_t*)cpu->edx;
    int32_t* h1 = (int32_t*)cpu->ebx;
    int32_t* h2 = (int32_t*)cpu->ecx;
    uint32_t tmp1;
    uint32_t tmp2;
    int32_t tmp3;
    int32_t tmp4;
    Widget* tmpWidget=0;


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

void SyscallHandlers::Handle_sys_restart(uint32_t esp)
{
    DEBUG_LOG("sys_restart\n");

    // Use a triple fault to restart the system (Not the best way, but for now this is good :) )
    asm volatile (
        "cli;"
        "lidt (%0);"  // Load an invalid IDT
        "int3;"       // Trigger an interrupt
        ::"r"(0)
    );
}

void SyscallHandlers::Handle_sys_exit(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    uint32_t currentPID = ProcessManager::activeInstance->getCurrentPID();
    Desktop::activeInstance->RemoveAppByPID(currentPID);
    HguiHandler::activeInstance->RemoveAppByPID(currentPID);

    ProcessManager::activeInstance->RemoveProcess(ProcessManager::activeInstance->getCurrentProcess());

    DEBUG_LOG("sys_exit: Exit status: %u, PID: %d\n", cpu->ebx, currentPID);
}

void SyscallHandlers::Handle_sys_clone(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    int32_t* return_data = (int32_t*)cpu->edx;
    DEBUG_LOG("sys_clone: Creating a new Thread");
    *return_data = (int32_t) ProcessManager::activeInstance->getCurrentProcess()->AddThread(
        reinterpret_cast<void (*)(void*)>(cpu->ebx),
        reinterpret_cast<void*>(cpu->ecx)
    );
}

void SyscallHandlers::Handle_sys_debug(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    writeSerial((char)cpu->ebx);
}

void SyscallHandlers::Handle_sys_Hcall(uint32_t esp)
{
    DEBUG_LOG("sys_Hcall:");
    CPUState* cpu = (CPUState*)esp;
    switch (cpu->ebx)
    {
    case Hsys_getHeap:
        *(int32_t*)cpu->ecx = ProcessManager::activeInstance->getCurrentProcess()->heapData.heap_start;
        *(int32_t*)cpu->edx = ProcessManager::activeInstance->getCurrentProcess()->heapData.heap_end;
        break;
    
    default:
        break;
    }
    
}