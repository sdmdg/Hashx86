/**
 * @file        SymbolTable.cpp
 * @brief       Kernel Symbol Table Implementation
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <core/drivers/SymbolTable.h>

KernelSymbol SymbolTable::symbols[1024];
int SymbolTable::count = 0;

void SymbolTable::Register(const char* name, uint32_t addr) {
    if (count >= 1024) {
        printf("Error: Kernel Symbol Table Full!\n");
        return;
    }
    symbols[count].name = name;
    symbols[count].address = addr;
    count++;
}

uint32_t SymbolTable::Lookup(const char* name) {
    for (int i = 0; i < count; i++) {
        if (strncmp(symbols[i].name, name, 64) == 0) {
            return symbols[i].address;
        }
    }
    return 0;  // Not Found
}
