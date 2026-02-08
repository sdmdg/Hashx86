/**
 * @file        globals.cpp
 * @brief       Global Variables for Kernel
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <core/globals.h>

char Buffer[32];
uint64_t timerTicks = 0;
bool g_sse_active = false;

Paging* paging = nullptr;
FontManager* fManager = nullptr;
InterruptManager* interrupts = nullptr;
Scheduler* scheduler = nullptr;
SyscallHandler* sysCalls = nullptr;
DriverManager* driverManager = nullptr;
AudioMixer* g_systemMixer = nullptr;
GraphicsDriver* g_systemcGraphicsDriver = nullptr;
AudioDriver* g_systemcAudioDriver = nullptr;
