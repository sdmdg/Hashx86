#ifndef PAGING_H
#define PAGING_H

#include <core/memory.h>
#include <core/pmm.h>
#include <debug.h>
#include <stdint.h>

// Standard x86 Paging Flags
#define PAGE_PRESENT 0x1
#define PAGE_RW 0x2
#define PAGE_USER 0x4
#define PAGE_WRITE_THRU 0x8
#define PAGE_NO_CACHE 0x10

#define PAGE_SIZE 4096

class Paging {
public:
    Paging();
    ~Paging();

    // 1. Initialization
    void Activate();

    // 2. Process Management (The Fix)
    // Creates a directory with Empty User Space + Shared Kernel Space
    uint32_t* CreateProcessDirectory();

    // 3. Context Switching
    void SwitchDirectory(uint32_t* new_dir);

    // 4. Mapping Primitive
    // Maps a Physical Address to a Virtual Address in a specific directory
    bool MapPage(uint32_t* directory, uint32_t virtual_addr, uint32_t physical_addr,
                 uint32_t flags);

    // 5. Query
    uint32_t GetPhysicalAddress(uint32_t* directory, uint32_t virtual_addr);

    // The Master Directory (Template for all processes)
    uint32_t* KernelPageDirectory;  // Master Directory

    // We store the tables for the Master Directory here
    // (In a real OS, these are dynamically allocated, but for now this is fine)
    uint32_t KernelPageTables[1024][1024];

private:
    bool is_paging_active;
};

#endif
