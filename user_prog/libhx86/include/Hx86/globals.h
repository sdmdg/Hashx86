#ifndef GLOBALS_H
#define GLOBALS_H

#include <Hx86/Hgui/desktop.h>
#include <Hx86/stdint.h>

struct ProgramArguments {
    char* str1;
    char* str2;
    char* str3;
    char* str4;
    char* str5;
};

extern ProgramArguments* args;
extern HeapData heapData;
extern Desktop* desktop;

#endif  // GLOBALS_H
