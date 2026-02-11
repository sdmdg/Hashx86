#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>

// Forward declarations
class AudioMixer;
class GraphicsDriver;
class AudioDriver;
class Paging;
class FontManager;
class InterruptManager;
class Scheduler;
class SyscallHandler;
class DriverManager;
class FAT32;
class ELFLoader;

// Kernel Utils
extern char Buffer[32];

// Timer
extern uint64_t timerTicks;

// Config
extern bool g_sse_active;

// Modules
extern Paging* g_paging;
extern FontManager* g_fManager;
extern InterruptManager* g_interrupts;
extern Scheduler* g_scheduler;
extern SyscallHandler* g_sysCalls;
extern DriverManager* g_driverManager;
extern AudioMixer* g_AudioMixer;
extern GraphicsDriver* g_GraphicsDriver;
extern AudioDriver* g_AudioDriver;
extern FAT32* g_bootPartition;
extern ELFLoader* g_elfLoader;

#endif  // GLOBALS_H
