
#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <core/interrupts.h>
#include <core/syscalls_x86.h>
#include <debug.h>
#include <gui/Hgui.h>
#include <gui/gui.h>
#include <types.h>

struct timespec {
    int32_t tv_sec;
    int32_t tv_nsec;
};

struct stat {
    uint32_t st_dev;      // ID of device containing file
    uint32_t st_ino;      // Inode number
    uint32_t st_mode;     // File type and mode
    uint32_t st_nlink;    // Number of hard links
    uint32_t st_uid;      // User ID of owner
    uint32_t st_gid;      // Group ID of owner
    uint32_t st_rdev;     // Device ID (if special file)
    uint32_t st_size;     // Total size, in bytes
    uint32_t st_blksize;  // Block size for filesystem I/O
    uint32_t st_blocks;   // Number of 512B blocks allocated
};

struct linux_dirent {
    uint32_t d_ino;     // Inode number
    uint32_t d_off;     // Offset to next linux_dirent
    uint16_t d_reclen;  // Length of this linux_dirent
    char d_name[];      // Filename (null-terminated)
};

typedef enum { Hsys_regEventH = 1, Hsys_getFramebuffer = 2, Hsys_getInput = 3 } HSYSCALL;

class SyscallHandler : public InterruptHandler {
public:
    SyscallHandler(uint8_t InterruptNumber, InterruptManager* interruptManager);
    ~SyscallHandler();

    virtual uint32_t HandleInterrupt(uint32_t esp);
};

class SyscallHandlers {
public:
    static int32_t Handle_sys_restart_syscall();
    static int32_t Handle_sys_exit(uint32_t status);
    static int32_t Handle_sys_read(uint32_t fd, char* buf, uint32_t count);
    static int32_t Handle_sys_open(const char* path, int32_t flags);
    static int32_t Handle_sys_close(uint32_t fd);
    static int32_t Handle_sys_execve(const char* path, char* const argv[], char* const envp[]);
    static int32_t Handle_sys_brk(uint32_t brk);
    static int32_t Handle_sys_stat(const char* path, struct stat* statbuf);
    static int32_t Handle_sys_clone(CPUState* parent_context, uint32_t clone_flags,
                                    void* child_stack, void* parent_tid, void* tls,
                                    void* child_tid);
    static int32_t Handle_sys_getdents(uint32_t fd, struct linux_dirent* dirp, uint32_t count);
    static int32_t Handle_sys_nanosleep(struct timespec* req, struct timespec* rem);

    static int32_t Handle_sys_debug(char* str);
    static int32_t Handle_sys_peek_memory(uint32_t address, uint32_t size, int32_t* return_data);

    static int32_t Handle_sys_Hcall(uint32_t hcall_id, uint32_t arg1, uint32_t arg2, uint32_t arg3,
                                    uint32_t arg4);
};

#endif  // SYSCALLS_H
