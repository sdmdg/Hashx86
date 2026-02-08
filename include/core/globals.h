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

// Kernel Utils
extern char Buffer[32];

// Timer
extern uint64_t timerTicks;

// Config
extern bool g_sse_active;

// Modules
extern Paging* paging;
extern FontManager* fManager;
extern InterruptManager* interrupts;
extern Scheduler* scheduler;
extern SyscallHandler* sysCalls;
extern DriverManager* driverManager;
extern AudioMixer* g_systemMixer;
extern GraphicsDriver* g_systemcGraphicsDriver;
extern AudioDriver* g_systemcAudioDriver;

#endif  // GLOBALS_H
