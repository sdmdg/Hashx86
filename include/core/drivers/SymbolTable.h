#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <debug.h>
#include <types.h>
#include <utils/string.h>

struct KernelSymbol {
    const char* name;
    uint32_t address;
};

#define EXPORT_SYMBOL_ASM(mangled_name)                          \
    do {                                                         \
        uint32_t addr;                                           \
        asm volatile("movl $" mangled_name ", %0" : "=r"(addr)); \
        SymbolTable::Register(mangled_name, addr);               \
    } while (0)

class SymbolTable {
public:
    // Register a kernel function name -> address
    static void Register(const char* name, uint32_t addr);

    // Find a function address by name
    static uint32_t Lookup(const char* name);

// Helper macro for easy exporting in kernel.cpp
#define EXPORT_SYMBOL(func) SymbolTable::Register(#func, (uint32_t) & func)

private:
    static KernelSymbol symbols[1024];
    static int count;
};

#endif
