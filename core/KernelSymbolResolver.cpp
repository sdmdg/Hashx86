/**
 * @file        KernelSymbolResolver.cpp
 * @brief       Kernel Symbol Resolver for #x86
 *
 * @date        29/01/2026
 * @version     1.0.0-beta
 */

#include <core/KernelSymbolResolver.h>

static char* fileBuffer = nullptr;
static SymbolEntry* symbolIndex = nullptr;
static uint32_t symbolCount = 0;

void KernelSymbolTable::Load(FAT32* fs, const char* path) {
    if (!fs) return;

    printf("[KernelSymbolTable] Loading map file: %s\n", path);
    File* file = fs->Open((char*)path);
    if (!file) {
        printf("[KernelSymbolTable] Failed to open %s\n", path);
        return;
    }

    if (file->size == 0) {
        printf("[KernelSymbolTable] Map file is empty!\n");
        file->Close();
        delete file;
        return;
    }

    // Load file
    fileBuffer = (char*)kmalloc(file->size + 1);
    file->Read((uint8_t*)fileBuffer, file->size);
    fileBuffer[file->size] = 0;  // Null terminate
    file->Close();

    // Allocate Index
    uint32_t maxEntries = file->size / 20;
    symbolIndex = (SymbolEntry*)kmalloc(maxEntries * sizeof(SymbolEntry));
    symbolCount = 0;
    delete file;

    // Parse
    char* cursor = fileBuffer;
    while (*cursor) {
        // Skip empty lines or leading whitespace
        while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n' || *cursor == '\r') {
            cursor++;
            if (*cursor == 0) break;
        }
        if (*cursor == 0) break;

        // Pattern: "0x00100000    functionName"
        if (cursor[0] == '0' && cursor[1] == 'x') {
            uint32_t addr = HexStrToInt(cursor);

            // Skip the address just read
            while (*cursor != ' ' && *cursor != '\t' && *cursor != '\n' && *cursor != '\r' &&
                   *cursor != 0)
                cursor++;

            // Skip whitespace between address and name
            while (*cursor == ' ' || *cursor == '\t') cursor++;

            // EOL check
            if (*cursor != '\n' && *cursor != '\r' && *cursor != 0) {
                // Store the symbol
                symbolIndex[symbolCount].addr = addr;
                symbolIndex[symbolCount].name = cursor;
                symbolCount++;

                // Fast-forward to end of line to terminate the string
                while (*cursor != '\n' && *cursor != '\r' && *cursor != 0) cursor++;

                // Replace newline with NULL to terminate the name string
                if (*cursor != 0) {
                    *cursor = 0;
                    cursor++;  // Move to next char for next loop iteration
                }
                continue;
            }
        }

        // If line didn't start with 0x, skip to next line
        while (*cursor != '\n' && *cursor != '\r' && *cursor != 0) cursor++;
    }

    printf("[KernelSymbolTable] Parsed %d functions.\n", (int32_t)symbolCount);
}

const char* KernelSymbolTable::Lookup(uint32_t eip, uint32_t* offset) {
    if (symbolCount == 0) return nullptr;

    uint32_t bestAddr = 0;
    const char* bestName = nullptr;

    // Find the closest symbol strictly <= EIP
    for (uint32_t i = 0; i < symbolCount; i++) {
        uint32_t addr = symbolIndex[i].addr;

        if (addr <= eip) {
            if (addr >= bestAddr) {
                bestAddr = addr;
                bestName = symbolIndex[i].name;
            }
        }
    }

    if (bestName) {
        *offset = eip - bestAddr;
        // Sanity check: if offset is huge (>100KB), likely a mismatch
        if (*offset > 0x100000) return nullptr;
        return bestName;
    }
    return nullptr;
}

void KernelSymbolTable::PrintStackTrace(unsigned int maxFrames) {
    StackFrame* stack;

    // Get the current EBP register
    asm volatile("mov %%ebp, %0" : "=r"(stack));

    printf("\n[ Stack Trace ]\n");

    for (unsigned int i = 0; i < maxFrames; ++i) {
        // If the stack pointer is null or invalid, stop
        if (!stack) break;

        // Safety: stop if EBP is outside kernel-mapped memory (0 - 256MB)
        // Following user-mode EBP pointers after switching to KernelPageDirectory
        // would cause a page fault and infinite loop since activeInstance=0.
        if ((uint32_t)stack < 0x1000 || (uint32_t)stack >= 0x10000000) break;

        // Print
        uint32_t offset = 0;
        const char* name = KernelSymbolTable::Lookup(stack->eip, &offset);
        if (name)
            printf(" 0x%x <%s+%d>\n", stack->eip, name, (int32_t)offset);
        else
            printf(" 0x%x\n", stack->eip);

        // Move to the previous frame (walk up the stack)
        stack = stack->ebp;
    }
}
