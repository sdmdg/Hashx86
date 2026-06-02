/**
 * @file        KernelSymbolResolver.cpp
 * @brief       Kernel Symbol Resolver for #x86
 *
 * @date        29/01/2026
 * @version     1.0.0-beta
 */

#define KDBG_COMPONENT "K.SYMBOL"
#include <core/KernelSymbolResolver.h>

static char* fileBuffer = nullptr;
static SymbolEntry* symbolIndex = nullptr;
static uint32_t symbolCount = 0;

void KernelSymbolTable::Load(FAT32* fs, const char* path) {
    if (!fs) return;
    KDBG1("Loading map file: %s", path);
    File* file = fs->Open((char*)path);
    if (!file) {
        KDBG1("Failed to open %s", path);
        return;
    }

    if (file->size == 0) {
        KDBG1("Map file is empty!");
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

    KDBG1("Parsed %d functions.", (int32_t)symbolCount);
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

void KernelSymbolTable::PrintStackTrace(unsigned int maxFrames, uint32_t faultEip,
                                        uint32_t faultEbp) {
    StackFrame* stack;
    uint32_t currentEip = 0;
    unsigned int skipInternalFrames = 4;
    uint32_t walkedEips[64];
    unsigned int walkedCount = 0;

    if (faultEbp != 0) {
        stack = (StackFrame*)faultEbp;
        // faultEbp already points at the crashing context, so don't hide early frames.
        skipInternalFrames = 0;
    } else {
        // Fallback: current call-site EBP.
        asm volatile("mov %%ebp, %0" : "=r"(stack));
    }

    // Capture the current instruction pointer explicitly.
    asm volatile("call 1f\n\t"
                 "1: pop %0"
                 : "=r"(currentEip));

    KDBG1("[ Stack Trace ]");

    unsigned int printed = 0;

    if (faultEip != 0 && printed < maxFrames) {
        uint32_t faultOffset = 0;
        const char* faultName = Lookup(faultEip, &faultOffset);
        if (faultName)
            KDBG1(" #%d 0x%x <%s+%d> [FAULT]", (int32_t)printed, faultEip, faultName,
                  (int32_t)faultOffset);
        else
            KDBG1(" #%d 0x%x [FAULT]", (int32_t)printed, faultEip);
        printed++;
    }

    (void)currentEip;

    for (unsigned int i = 0; i < 64; ++i) {
        // If the stack pointer is null or invalid, stop
        if (!stack) break;

        // Safety: stop if EBP is outside kernel-mapped memory (0 - 256MB)
        // Following user-mode EBP pointers after switching to KernelPageDirectory
        // would cause a page fault and infinite loop since activeInstance=0.
        if ((uint32_t)stack < 0x1000 || (uint32_t)stack >= 0x10000000) break;

        /*
        [K.SYMBOL] [ Stack Trace ]
        Bypass [K.SYMBOL] 0 <InterruptManager::DohandleException(unsigned char, unsigned int)+328>
        Bypass [K.SYMBOL] 1 <InterruptManager::handleException(unsigned char, unsigned int)+72>
        Bypass [K.SYMBOL] 2 <InterruptManager::HandleInterruptRequest0x81()+80>
        */
        walkedEips[walkedCount++] = stack->eip;

        // Move to the previous frame (walk up the stack)
        stack = stack->ebp;
    }

    // Print only meaningful frames after skipping interrupt/tracer internals.
    for (unsigned int i = skipInternalFrames; i < walkedCount && printed < maxFrames; ++i) {
        if (faultEip != 0 && walkedEips[i] == faultEip) continue;

        uint32_t offset = 0;
        const char* name = Lookup(walkedEips[i], &offset);
        if (name)
            KDBG1(" #%d 0x%x <%s+%d>", (int32_t)printed, walkedEips[i], name, (int32_t)offset);
        else
            KDBG1(" #%d 0x%x", (int32_t)printed, walkedEips[i]);
        printed++;
    }

    KDBG1("[ End of Stack Trace ]\n");
}
