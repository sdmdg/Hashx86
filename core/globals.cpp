/**
 * @file        globals.cpp
 * @brief       Global Variables for Kernel
 *
 * @date        11/02/2026
 * @version     1.0.0
 */

#include <core/globals.h>

char Buffer[32];
uint64_t timerTicks = 0;
bool g_sse_active = false;
bool g_stop_gui_rendering = false;
int g_gui_owner_pid = -1;

Paging* g_paging = nullptr;
FontManager* g_fManager = nullptr;
InterruptManager* g_interrupts = nullptr;
Scheduler* g_scheduler = nullptr;
SyscallHandler* g_sysCalls = nullptr;
DriverManager* g_driverManager = nullptr;
AudioMixer* g_AudioMixer = nullptr;
GraphicsDriver* g_GraphicsDriver = nullptr;
AudioDriver* g_AudioDriver = nullptr;
FAT32* g_bootPartition = nullptr;
ELFLoader* g_elfLoader = nullptr;
