#ifndef MODULE_LOADER_H
#define MODULE_LOADER_H

#include <types.h>
#include <core/filesystem/File.h>
#include <core/drivers/SymbolTable.h>
#include <core/elf.h>
#include <console.h>
#include <core/memory.h>
#include <core/drivers/driver_info.h>

class ModuleLoader {
public:
    // Loads a relocatable ELF (.o file)
    // Returns the address of the "CreateDriverInstance" function (or entry point)
    static void* LoadMatchingDriver(File* file, uint16_t target_vid, uint16_t target_did);
    static bool Probe(File* file, DriverManifest* info);

private:
    static void* LoadDriver(File* file);


    // Internal ELF helpers (specific to x86 relocation)
    static int ApplyRelocation(uint32_t type, uint32_t* target, uint32_t value, uint32_t addend);
};

#endif