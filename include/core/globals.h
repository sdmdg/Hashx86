#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>

// Forward declarations
class AudioMixer;
class GraphicsDriver;
class AudioDriver;

// Kernel Utils
extern char Buffer[32];

// Timer
extern uint64_t timerTicks;

// Config
extern bool g_sse_active;

// Modules
extern AudioMixer* g_systemMixer;
extern GraphicsDriver* g_systemcGraphicsDriver;
extern AudioDriver* g_systemcAudioDriver;

#endif  // GLOBALS_H
