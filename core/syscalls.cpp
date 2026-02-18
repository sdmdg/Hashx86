/**
 * @file        syscalls.cpp
 * @brief       System Calls Interface for #x86
 *
 * @date        20/01/2026
 * @version     1.0.0-beta
 */

#include <core/drivers/keyboard.h>
#include <core/drivers/mouse.h>
#include <core/filesystem/msdospart.h>
#include <core/globals.h>
#include <core/paging.h>
#include <core/pmm.h>
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

        case sys_peek_memory:
            SyscallHandlers::Handle_sys_peek_memory(esp);
            break;

        case sys_clone:
            SyscallHandlers::Handle_sys_clone(esp);
            break;

        case sys_sleep:
            SyscallHandlers::Handle_sys_sleep(esp);
            break;

        case sys_sbrk:
            SyscallHandlers::Handle_sys_sbrk(esp);
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
    Scheduler* sched = Scheduler::activeInstance;
    if (!sched) return;

    // Save PID before ExitCurrentThread potentially destroys the process
    ProcessControlBlock* process = sched->GetCurrentProcess();
    uint32_t pid = process ? process->pid : 0;

    bool processKilled = sched->ExitCurrentThread();

    // Clean up GUI resources only if the entire process was terminated
    if (processKilled && pid) {
        Desktop::activeInstance->RemoveAppByPID(pid);
        HguiHandler::activeInstance->RemoveAppByPID(pid);

        // Restore Desktop rendering if the owner exits
        if (g_stop_gui_rendering && g_gui_owner_pid == (int)pid) {
            DEBUG_LOG("sys_exit: Releasing GUI lock from PID %d", pid);
            g_stop_gui_rendering = false;
            g_gui_owner_pid = -1;
            // Force redraw of desktop
            if (Desktop::activeInstance) Desktop::activeInstance->MarkDirty();
        }

        DEBUG_LOG("sys_exit: Process PID %d terminated\n", pid);
    }
}

void SyscallHandlers::Handle_sys_peek_memory(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    uint32_t address = cpu->ebx;
    uint32_t size = cpu->ecx;
    int32_t* return_data = (int32_t*)cpu->edx;

    // Only allow reading from identity-mapped kernel range (0 - 256MB)
    uint32_t limit = 256 * 1024 * 1024;
    if (address + size > limit || size == 0 || size > 4) {
        *return_data = 0;
        return;
    }

    uint32_t value = 0;
    switch (size) {
        case 1:
            value = *(uint8_t*)address;
            break;
        case 2:
            value = *(uint16_t*)address;
            break;
        case 4:
            value = *(uint32_t*)address;
            break;
    }
    *return_data = (int32_t)value;
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

void SyscallHandlers::Handle_sys_sbrk(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    ProcessControlBlock* process = Scheduler::activeInstance->GetCurrentProcess();
    int32_t increment = (int32_t)cpu->ebx;
    int32_t* return_data = (int32_t*)cpu->edx;

    uint32_t old_brk = process->heap.endAddress;
    uint32_t new_brk = old_brk + increment;

    if (increment > 0) {
        if (new_brk > process->heap.maxAddress) {
            DEBUG_LOG("sbrk: Heap Overflow! Max: 0x%x, Req: 0x%x", process->heap.maxAddress,
                      new_brk);
            *return_data = -1;
            return;
        }

        uint32_t page_start = (old_brk + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        uint32_t page_end = (new_brk + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

        if (g_paging && page_end > page_start) {
            for (uint32_t addr = page_start; addr < page_end; addr += PAGE_SIZE) {
                // Check if already mapped
                if (g_paging->GetPhysicalAddress(process->page_directory, addr) == 0) {
                    uint32_t phys_frame = (uint32_t)pmm_alloc_block();
                    if (!phys_frame) {
                        DEBUG_LOG("sbrk: Out of physical memory!");
                        *return_data = -1;
                        return;
                    }
                    if (!g_paging->MapPage(process->page_directory, addr, phys_frame,
                                           PAGE_PRESENT | PAGE_RW | PAGE_USER)) {
                        /* Mapping failed (likely out of low memory for page tables) */
                        pmm_free_block((void*)phys_frame);
                        DEBUG_LOG("sbrk: MapPage failed!");
                        *return_data = -1;
                        return;
                    }
                    // No memset needed - kernel doesn't identity map high memory
                }
            }
        }
    }

    // Update process heap end
    process->heap.endAddress = new_brk;
    *return_data = (int32_t)old_brk;
}

void SyscallHandlers::Handle_sys_debug(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    char* userString = (char*)cpu->ebx;

    // ATOMIC PRINT
    InterruptGuard guard;
    printf("%s", userString);
}

void SyscallHandlers::Handle_sys_Hcall(uint32_t esp) {
    // DEBUG_LOG("sys_Hcall:");
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
    } else if (cpu->ebx == Hsys_getFramebuffer) {
        // Return framebuffer info: param0=buffer
        extern GraphicsDriver* g_GraphicsDriver;
        extern Paging* g_paging;

        if (g_GraphicsDriver) {
            uint32_t bufferAddr = (uint32_t)g_GraphicsDriver->GetBackBuffer();
            uint32_t width = g_GraphicsDriver->GetWidth();
            uint32_t height = g_GraphicsDriver->GetHeight();

            data->param0 = bufferAddr;
            data->param1 = width;
            data->param2 = height;

            // GRANT ACCESS: Map the kernel backbuffer as USER accessible
            uint32_t size = width * height * 4;

            // Align start/end to page boundaries
            uint32_t startPage = bufferAddr & ~(PAGE_SIZE - 1);
            uint32_t endPage = (bufferAddr + size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

            ProcessControlBlock* process = Scheduler::activeInstance->GetCurrentProcess();

            for (uint32_t addr = startPage; addr < endPage; addr += PAGE_SIZE) {
                // Use identity mapping (phys = virt) for kernel heap
                // This updates the shared kernel page table with PAGE_USER
                g_paging->MapPage(process->page_directory, addr, addr,
                                  PAGE_PRESENT | PAGE_RW | PAGE_USER);
            }

            // Also update the Page Directory Entry (PDE) to allow User Access
            // MapPage does NOT update the PDE flags if the table is already present.
            // Since the kernel heap PDE is originally Supervisor-only, must enable User bit.
            uint32_t startPDIdx = startPage >> 22;
            uint32_t endPDIdx = endPage >> 22;

            for (uint32_t i = startPDIdx; i <= endPDIdx; i++) {
                process->page_directory[i] |= PAGE_USER;
            }

            // Flush TLB to ensure new permissions take effect immediately
            asm volatile("mov %%cr3, %%eax; mov %%eax, %%cr3" ::: "eax");

            // STOP KERNEL GUI RENDERING
            g_stop_gui_rendering = true;
            g_gui_owner_pid = Scheduler::activeInstance->GetCurrentProcess()->pid;
            DEBUG_LOG("Hsys_getFramebuffer: PID %d took ownership of screen", g_gui_owner_pid);

            *return_data = 1;
        } else {
            *return_data = -1;
        }
    } else if (cpu->ebx == Hsys_getInput) {
        struct InputState {
            uint8_t keyStates[128];
            int32_t mouseDX;
            int32_t mouseDY;
            uint8_t mouseButtons;
        } __attribute__((packed));

        InputState* userState = (InputState*)data->param0;
        if (userState) {
            // Copy keyboard state
            if (KeyboardDriver::activeInstance) {
                uint8_t* keys = KeyboardDriver::activeInstance->GetKeyStates();
                for (int i = 0; i < 128; i++) {
                    userState->keyStates[i] = keys[i];
                }
            }
            // Copy and reset mouse state
            if (MouseDriver::activeInstance) {
                int32_t dx, dy;
                MouseDriver::activeInstance->GetMouseDelta(dx, dy);
                userState->mouseDX = dx;
                userState->mouseDY = dy;
                userState->mouseButtons = MouseDriver::activeInstance->GetButtons();
            }
            *return_data = 1;
        } else {
            *return_data = -1;
        }
    } else if (cpu->ebx == Hsys_readFile) {
        extern MSDOSPartitionTable* g_PartitionTable;
        char* filename = (char*)data->param0;
        uint8_t* destBuffer = (uint8_t*)data->param1;
        uint32_t maxSize = data->param2;

        if (filename && destBuffer && maxSize > 0) {
            FAT32* fs = nullptr;
            if (MSDOSPartitionTable::activeInstance &&
                MSDOSPartitionTable::activeInstance->partitions[0]) {
                fs = MSDOSPartitionTable::activeInstance->partitions[0];
            }

            if (fs) {
                DEBUG_LOG("Hsys_readFile: Opening %s", filename);

                // Validate Buffer is User Space
                if ((uint32_t)destBuffer < 0x10000000) {
                    DEBUG_LOG("Hsys_readFile: SECURITY VIOLATION: Buffer in Kernel Space! 0x%x",
                              destBuffer);
                    *return_data = -1;
                    return;
                }

                File* file = fs->Open(filename);
                if (file && file->size > 0) {
                    DEBUG_LOG("Hsys_readFile: Opened %s, Size %d", filename, file->size);
                    uint32_t readSize = file->size < maxSize ? file->size : maxSize;
                    file->Seek(0);
                    DEBUG_LOG("Hsys_readFile: Reading...");
                    int bytesRead = file->Read(destBuffer, readSize);
                    DEBUG_LOG("Hsys_readFile: Read %d bytes. Closing...", bytesRead);

                    data->param3 = file->size;  // Report actual file size
                    file->Close();

                    DEBUG_LOG("Hsys_readFile: Deleting file object...");
                    delete file;

                    DEBUG_LOG("Hsys_readFile: Success.");
                    *return_data = bytesRead;
                } else {
                    if (file) {
                        DEBUG_LOG("Hsys_readFile: Empty file %s", filename);
                        file->Close();
                        delete file;
                    } else {
                        DEBUG_LOG("Hsys_readFile: Failed to open %s", filename);
                    }
                    *return_data = -1;
                }
            } else {
                *return_data = -1;
            }
        } else {
            *return_data = -1;
        }
    } else {
        // Default case (optional: handle unknown Hcalls)
        DEBUG_LOG("Unknown Hcall ID: %d", cpu->ebx);
    }
}
