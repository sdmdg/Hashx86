#include <core/drivers/SymbolTable.h>

// Helper string compare
static int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

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
        if (strcmp(symbols[i].name, name) == 0) {
            return symbols[i].address;
        }
    }
    return 0; // Not Found
}