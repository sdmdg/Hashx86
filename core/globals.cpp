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
AudioMixer* g_systemMixer = nullptr;
GraphicsDriver* g_systemcGraphicsDriver = nullptr;
AudioDriver* g_systemcAudioDriver = nullptr;
