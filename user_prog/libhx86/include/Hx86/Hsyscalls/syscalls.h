#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <Hx86/Hsyscalls/syscalls_x86.h>
#include <Hx86/stdint.h>

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

void syscall_exit(uint32_t status);
int32_t syscall_read(uint32_t fd, char* buf, uint32_t count);
int32_t syscall_open(const char* path, int32_t flags);
int32_t syscall_close(uint32_t fd);
int32_t syscall_execve(const char* path, char* const argv[], char* const envp[]);
int32_t syscall_brk(int32_t increment);
int32_t syscall_stat(const char* path, struct stat* statbuf);
uint32_t syscall_clone(void (*entrypoint)(void*), void* arg);
int32_t syscall_getdents(uint32_t fd, struct linux_dirent* dirp, uint32_t count);
void syscall_nanosleep(struct timespec* req, struct timespec* rem);

void syscall_debug(const char* str);
uint32_t syscall_peek_memory(uint32_t address, uint32_t size);

typedef enum { Hsys_regEventH = 1, Hsys_getFramebuffer = 2, Hsys_getInput = 3 } HSYSCALL;

// Input state structure for Hsys_getInput
struct InputState {
    uint8_t keyStates[128];
    int32_t mouseDX;
    int32_t mouseDY;
    uint8_t mouseButtons;
} __attribute__((packed));

// Framebuffer info structure
struct FramebufferInfo {
    uint32_t buffer;
    uint32_t width;
    uint32_t height;
};

uint32_t syscall_Hgui(uint32_t element, uint32_t mode, void* data);
uint32_t syscall_register_event_handler(void (*entrypoint)(void*), void* arg);
void syscall_get_input(InputState* state);
FramebufferInfo syscall_get_framebuffer();
#endif  // SYSCALLS_H
