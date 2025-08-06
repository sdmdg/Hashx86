 
#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <types.h>
#include <core/interrupts.h>
#include <debug.h>
#include <gui/gui.h>
#include <gui/Hgui.h>

typedef enum {
    sys_restart = 0,
    sys_exit = 1,
    sys_fork = 2,
    sys_read = 3,
    sys_write = 4,
    sys_open = 5,
    sys_close = 6,
    sys_clone = 41,
    sys_Hcall = 199,
    sys_debug = 200,
} SYSCALL;

typedef enum {
    Hsys_getHeap = 0
} HSYSCALL;

class SyscallHandler : public InterruptHandler
{
    
public:
    SyscallHandler(uint8_t InterruptNumber, InterruptManager* interruptManager);
    ~SyscallHandler();
    
    virtual uint32_t HandleInterrupt(uint32_t esp);

};


class SyscallHandlers
{
    
public:
    static void Handle_sys_restart(uint32_t esp);
    static void Handle_sys_exit(uint32_t esp);
    static void Handle_sys_clone(uint32_t esp);
    static void Handle_sys_debug(uint32_t esp);
    static void Handle_sys_Hcall(uint32_t esp);
    
};

#endif // SYSCALLS_H