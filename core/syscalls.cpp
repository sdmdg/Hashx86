/**
 * @file        syscalls.cpp
 * @brief       System Calls Interface for #x86
 *
 * @date        20/01/2026
 * @version     1.0.0-beta
 */

#define KDBG_COMPONENT "SYSCALL"
#include <core/drivers/keyboard.h>
#include <core/drivers/mouse.h>
#include <core/filesystem/msdospart.h>
#include <core/globals.h>
#include <core/paging.h>
#include <core/pmm.h>
#include <core/syscalls.h>
#include <core/filesystem/File.h>

SyscallHandler::SyscallHandler(uint8_t InterruptNumber, InterruptManager* interruptManager)
    : InterruptHandler(InterruptNumber + 0x20, interruptManager) {}

SyscallHandler::~SyscallHandler() {}

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;

    // Linux standard x86:
    // eax = syscall number
    // ebx = arg1
    // ecx = arg2
    // edx = arg3
    // esi = arg4
    // edi = arg5
    // ebp = arg6

    int32_t return_val = 0;

    switch ((uint32_t)cpu->eax) {
        case sys_restart_syscall:
            return_val = SyscallHandlers::Handle_sys_restart_syscall();
            break;

        case sys_exit:
            return_val = SyscallHandlers::Handle_sys_exit(cpu->ebx);
            break;

        case sys_read:
            return_val = SyscallHandlers::Handle_sys_read(cpu->ebx, (char*)cpu->ecx, cpu->edx);
            break;

        case sys_open:
            return_val = SyscallHandlers::Handle_sys_open((const char*)cpu->ebx, cpu->ecx);
            break;
        case sys_close:
            return_val = SyscallHandlers::Handle_sys_close(cpu->ebx);
            break;
        case sys_execve:
            return_val = SyscallHandlers::Handle_sys_execve(
                (const char*)cpu->ebx, (char* const*)cpu->ecx, (char* const*)cpu->edx);
            break;

        case sys_brk:
            return_val = SyscallHandlers::Handle_sys_brk(cpu->ebx);
            break;

        case sys_stat:
            return_val =
                SyscallHandlers::Handle_sys_stat((const char*)cpu->ebx, (struct stat*)cpu->ecx);
            break;

        case sys_clone:
            return_val = SyscallHandlers::Handle_sys_clone(
                cpu->ebx, (void*)cpu->ecx, (void*)cpu->edx, (void*)cpu->esi, (void*)cpu->edi);
            break;

        case sys_getdents:
            return_val = SyscallHandlers::Handle_sys_getdents(
                cpu->ebx, (struct linux_dirent*)cpu->ecx, cpu->edx);
            break;

        case sys_nanosleep:
            return_val = SyscallHandlers::Handle_sys_nanosleep((struct timespec*)cpu->ebx,
                                                               (struct timespec*)cpu->ecx);
            break;

        case sys_debug:
            return_val = SyscallHandlers::Handle_sys_debug((char*)cpu->ebx);
            break;

        case sys_peek_memory:
            return_val =
                SyscallHandlers::Handle_sys_peek_memory(cpu->ebx, cpu->ecx, (int32_t*)cpu->edx);
            break;

        case sys_Hcall:
            return_val =
                SyscallHandlers::Handle_sys_Hcall(cpu->ebx, cpu->ecx, cpu->edx, cpu->esi, cpu->edi);
            break;

        default:
            KDBG1("Unknown system call: %u\n", cpu->eax);
            return_val = -1;
            break;
    }

    cpu->eax = return_val;
    return esp;
}

int32_t SyscallHandlers::Handle_sys_restart_syscall() {
    KDBG1("sys_restart\n");

    // Use a triple fault to restart the system (Not the best way, but for now this is good :) )
    asm volatile(
        "cli;"
        "lidt (%0);"  // Load an invalid IDT
        "int3;"       // Trigger an interrupt
        ::"r"(0));
    return 0;
}

int32_t SyscallHandlers::Handle_sys_exit(uint32_t status) {
    Scheduler* sched = Scheduler::activeInstance;
    if (!sched) return -1;

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
            KDBG1("sys_exit: Releasing GUI lock from PID %d", pid);
            g_stop_gui_rendering = false;
            g_gui_owner_pid = -1;
            // Force redraw of desktop
            if (Desktop::activeInstance) Desktop::activeInstance->MarkDirty();
        }

        KDBG1("sys_exit: Process PID %d terminated with status %d", pid, status);
    }
    return 0;  // Technically never returns
}

int32_t SyscallHandlers::Handle_sys_read(uint32_t fd, char* buf, uint32_t count) {
    if (fd <= 2 || !buf || count == 0) return -1;
    File* file = GetFileByFd(fd);
    if (!file) return -1;

    // Validate Buffer is User Space
    if ((uint32_t)buf < 0x10000000) {
        KDBG1("sys_read: SECURITY VIOLATION: Buffer in Kernel Space! 0x%x", buf);
        return -1;
    }

    int bytesRead = file->Read((uint8_t*)buf, count);
    return bytesRead;
}

int32_t SyscallHandlers::Handle_sys_open(const char* path, int32_t flags) {
    (void)flags;
    if (!path) return -1;

    extern MSDOSPartitionTable* g_PartitionTable;
    if (MSDOSPartitionTable::activeInstance && MSDOSPartitionTable::activeInstance->partitions[0]) {
        FAT32* fs = MSDOSPartitionTable::activeInstance->partitions[0];

        File* f = fs->Open((char*)path);
        if (!f) return -1;

        int32_t fd = AllocateFd(f);
        if (fd < 0) {
            f->Close();
            delete f;
            return -1;
        }
        return fd;
    }
    return -1;
}

int32_t SyscallHandlers::Handle_sys_close(uint32_t fd) {
    if (fd <= 2) return 0;  // stdin/stdout/stderr placeholders

    File* file = GetFileByFd(fd);
    if (!file) return -1;

    file->Close();
    delete file;
    ReleaseFd(fd);
    return 0;
}

int32_t SyscallHandlers::Handle_sys_execve(const char* path, char* const argv[],
                                           char* const envp[]) {
    if (!path || !g_elfLoader) return -1;

    extern MSDOSPartitionTable* g_PartitionTable;
    if (MSDOSPartitionTable::activeInstance && MSDOSPartitionTable::activeInstance->partitions[0]) {
        FAT32* fs = MSDOSPartitionTable::activeInstance->partitions[0];
        File* f = fs->Open((char*)path);
        if (f && f->size > 0) {
            // Hashx86 native loading actually spins up a new process completely instead of
            // replacing context To achieve typical execve we could terminate the current process
            // after loading, but to preserve system stability for now we will just load the elf
            // program identically to the GUI Taskbar behavior.
            ProgramArguments* args = new ProgramArguments{"ARG1", "ARG2", "ARG3", "ARG4", "ARG5"};
            ProcessControlBlock* child = g_elfLoader->loadELF(f, args);
            f->Close();
            delete f;
            if (child) {
                // Return child PID
                return child->pid;
            }
        } else if (f) {
            f->Close();
            delete f;
        }
    }
    return -1;
}

int32_t SyscallHandlers::Handle_sys_brk(uint32_t brk) {
    ProcessControlBlock* process = Scheduler::activeInstance->GetCurrentProcess();

    uint32_t old_brk = process->heap.endAddress;

    // If brk is 0, just return the current limit.
    if (brk == 0) {
        return (int32_t)old_brk;
    }

    // Only allow increasing the brk boundary for now.
    if (brk > old_brk) {
        if (brk > process->heap.maxAddress) {
            KDBG1("sys_brk: Heap Overflow! Max: 0x%x, Req: 0x%x", process->heap.maxAddress, brk);
            return -1;
        }

        uint32_t page_start = (old_brk + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        uint32_t page_end = (brk + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

        if (g_paging && page_end > page_start) {
            for (uint32_t addr = page_start; addr < page_end; addr += PAGE_SIZE) {
                // Check if already mapped
                if (g_paging->GetPhysicalAddress(process->page_directory, addr) == 0) {
                    uint32_t phys_frame = (uint32_t)pmm_alloc_block();
                    if (!phys_frame) {
                        KDBG1("sys_brk: Out of physical memory!");
                        return -1;
                    }
                    if (!g_paging->MapPage(process->page_directory, addr, phys_frame,
                                           PAGE_PRESENT | PAGE_RW | PAGE_USER)) {
                        pmm_free_block((void*)phys_frame);
                        KDBG1("sys_brk: MapPage failed!");
                        return -1;
                    }
                }
            }
        }
        process->heap.endAddress = brk;
    }

    // We do not handle shrinking heap at the moment

    return (int32_t)process->heap.endAddress;
}

int32_t SyscallHandlers::Handle_sys_stat(const char* path, struct stat* statbuf) {
    if (!path || !statbuf) return -1;

    extern MSDOSPartitionTable* g_PartitionTable;
    if (MSDOSPartitionTable::activeInstance && MSDOSPartitionTable::activeInstance->partitions[0]) {
        FAT32* fs = MSDOSPartitionTable::activeInstance->partitions[0];

        // Handling Root Drive Stat Check
        if (path[0] == '/' && path[1] == '\0') {
            statbuf->st_mode = 0x4000;  // S_IFDIR
            statbuf->st_size = 0;
            return 0;
        }

        File* f = fs->Open((char*)path);
        if (f) {
            statbuf->st_size = f->size;
            statbuf->st_ino = f->id;
            statbuf->st_blksize = 512;
            statbuf->st_blocks = (f->size + 511) / 512;
            if (f->flags & 1) {             // Directory Flag mapped loosely
                statbuf->st_mode = 0x4000;  // S_IFDIR
            } else {
                statbuf->st_mode = 0x8000;  // S_IFREG
            }
            f->Close();
            delete f;
            return 0;
        }
    }
    return -1;
}

int32_t SyscallHandlers::Handle_sys_clone(uint32_t clone_flags, void* child_stack, void* parent_tid,
                                          void* tls, void* child_tid) {
    KDBG1("sys_clone: Creating a new Thread");

    ProcessControlBlock* current_process = Scheduler::activeInstance->GetCurrentProcess();

    // Fallback/adapt to old thread creation (we're hacking clone to map to CreateThread)
    // Note: older logic passed entrypoint via EBX and arg via ECX.
    // We assume child_stack holds the argument context here for now just to maintain compilation.
    // In reality sys_clone behaves fundamentally different than thread spawning like this,
    // but we will maintain functional parity.
    return (int32_t)Scheduler::activeInstance->CreateThread(
        current_process, reinterpret_cast<void (*)(void*)>(clone_flags),
        reinterpret_cast<void*>(child_stack));
}

int32_t SyscallHandlers::Handle_sys_getdents(uint32_t fd, struct linux_dirent* dirp,
                                             uint32_t count) {
    if (fd <= 2 || !dirp || count == 0) return -1;
    File* dirFile = GetFileByFd(fd);
    if (!dirFile || !dirFile->filesystem) return -1;
    if ((dirFile->flags & 1) == 0) return -1;

    // We are streaming raw directory entries 512 bytes at a time
    // DirectoryEntryFat32 is 32 bytes each. We buffer enough for a few.
    uint8_t buffer[512];
    int bytesRead = dirFile->Read(buffer, 512);

    if (bytesRead <= 0) return 0;  // EOF

    uint32_t offsetWritten = 0;
    DirectoryEntryFat32* entries = (DirectoryEntryFat32*)buffer;

    for (int i = 0; i < (bytesRead / 32); i++) {
        DirectoryEntryFat32* e = &entries[i];

        if (e->name[0] == 0x00) {
            return offsetWritten;  // End of directory structure completely
        }
        if (e->name[0] == 0xE5) continue;                                           // Deleted Entry
        if (e->name[0] == '.' && e->name[1] == ' ' && e->name[2] == ' ') continue;  // Root dots

        // VFAT Long File Names Flag (0x0F)
        if ((e->attributes & 0x0F) == 0x0F) continue;

        // Calculate needed spacing for dynamic string
        // Ex: "GAME3D" + "." + "BIN" + \0
        char parsedName[13];
        int j = 0, nameIdx = 0;
        for (j = 0; j < 8 && e->name[j] != ' '; j++) {
            parsedName[nameIdx++] = e->name[j];
        }
        if (e->ext[0] != ' ') {
            parsedName[nameIdx++] = '.';
            for (j = 0; j < 3 && e->ext[j] != ' '; j++) {
                parsedName[nameIdx++] = e->ext[j];
            }
        }
        parsedName[nameIdx] = '\0';

        uint32_t reclen = sizeof(struct linux_dirent) + nameIdx + 1;
        // Align length to 4-byte boundaries roughly
        reclen = (reclen + 3) & ~3;

        if (offsetWritten + reclen > count) {
            // Buffer full, rollback file position up to here so we catch it next request!
            dirFile->position -= (bytesRead - (i * 32));
            return offsetWritten;
        }

        // Drop directly into struct linux_dirent output
        struct linux_dirent* currentDirent = (struct linux_dirent*)((uint8_t*)dirp + offsetWritten);
        currentDirent->d_ino = ((uint32_t)e->firstClusterHi << 16) | e->firstClusterLow;
        currentDirent->d_off = dirFile->position;
        currentDirent->d_reclen = reclen;

        for (j = 0; j <= nameIdx; j++) {
            currentDirent->d_name[j] = parsedName[j];
        }

        offsetWritten += reclen;
    }

    return offsetWritten;
}

int32_t SyscallHandlers::Handle_sys_nanosleep(struct timespec* req, struct timespec* rem) {
    ThreadControlBlock* t = Scheduler::activeInstance->GetCurrentThread();

    // Very basic mapping for now: (sec * 1000) + (ns / 1M)
    uint32_t sleep_ms = 0;
    if (req) {
        sleep_ms = (req->tv_sec * 1000) + (req->tv_nsec / 1000000);
    }
    Scheduler::activeInstance->Sleep(sleep_ms);

    // We don't implement remaining time out for now
    if (rem) {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }
    return 0;
}

int32_t SyscallHandlers::Handle_sys_debug(char* str) {
    // ATOMIC PRINT
    InterruptGuard guard;
    KDBG1N("PID %d, %s", g_scheduler->GetCurrentProcess()->pid, str);
    return 0;
}

int32_t SyscallHandlers::Handle_sys_peek_memory(uint32_t address, uint32_t size,
                                                int32_t* return_data) {
    // Only allow reading from identity-mapped kernel range (0 - 256MB)
    uint32_t limit = 256 * 1024 * 1024;
    if (address + size > limit || size == 0 || size > 4) {
        if (return_data) *return_data = 0;
        return -1;
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
    if (return_data) *return_data = (int32_t)value;
    return 0;
}

int32_t SyscallHandlers::Handle_sys_Hcall(uint32_t hcall_id, uint32_t arg1, uint32_t arg2,
                                          uint32_t arg3, uint32_t arg4) {
    ProcessControlBlock* current_process = Scheduler::activeInstance->GetCurrentProcess();

    if (hcall_id == Hsys_regEventH) {
        KDBG1("Hsys_regEventH: Creating a new Thread for handler");
        void* threadArgs = (void*)arg1;
        void* entryPoint = (void*)arg2;

        ThreadControlBlock* thread = Scheduler::activeInstance->CreateThread(
            current_process, reinterpret_cast<void (*)(void*)>(entryPoint),
            reinterpret_cast<void*>(threadArgs));

        Desktop::activeInstance->createNewHandler(current_process->pid, thread);

        return (int32_t)thread->tid;
    } else if (hcall_id == Hsys_getFramebuffer) {
        // Return framebuffer info: ptr to buffer, width, height passed in
        uint32_t* pBuffer = (uint32_t*)arg1;
        uint32_t* pWidth = (uint32_t*)arg2;
        uint32_t* pHeight = (uint32_t*)arg3;

        extern GraphicsDriver* g_GraphicsDriver;
        extern Paging* g_paging;

        if (g_GraphicsDriver) {
            uint32_t bufferAddr = (uint32_t)g_GraphicsDriver->GetBackBuffer();
            uint32_t width = g_GraphicsDriver->GetWidth();
            uint32_t height = g_GraphicsDriver->GetHeight();

            if (pBuffer) *pBuffer = bufferAddr;
            if (pWidth) *pWidth = width;
            if (pHeight) *pHeight = height;

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
            KDBG1("Hsys_getFramebuffer: PID %d took ownership of screen", g_gui_owner_pid);

            return 1;
        } else {
            return -1;
        }
    } else if (hcall_id == Hsys_getInput) {
        struct InputState {
            uint8_t keyStates[128];
            int32_t mouseDX;
            int32_t mouseDY;
            uint8_t mouseButtons;
        } __attribute__((packed));

        InputState* userState = (InputState*)arg1;
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
            return 1;
        } else {
            return -1;
        }
    } else {
        // Default case (optional: handle unknown Hcalls)
        KDBG1("Unknown Hcall ID: %d", hcall_id);
    }
    return -1;
}
