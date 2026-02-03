#ifndef KERNELSYMBOLRESOLVER_H
#define KERNELSYMBOLRESOLVER_H

#include <core/filesystem/FAT32.h>
#include <core/memory.h>
#include <debug.h>
#include <types.h>
#include <utils/string.h>

struct StackFrame {
    struct StackFrame* ebp;  // Pointer to the previous stack frame
    uint32_t eip;            // The instruction pointer (return address)
};

struct SymbolEntry {
    uint32_t addr;
    char* name;
};

class KernelSymbolTable {
public:
    static void Load(FAT32* fs, const char* path);
    static const char* Lookup(uint32_t address, uint32_t* offset);
    static void PrintStackTrace(unsigned int maxFrames);
};

#endif  // KERNELSYMBOLRESOLVER_H
