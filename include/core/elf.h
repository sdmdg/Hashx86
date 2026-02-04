#ifndef ELF_H
#define ELF_H

#include <core/filesystem/File.h>
#include <core/memory.h>
#include <core/paging.h>
#include <core/pmm.h>
#include <core/scheduler.h>
#include <debug.h>
#include <stdint.h>
#include <types.h>

#define ELF_MAGIC 0x464C457F

// ELF Header
struct elf_header {
    uint32_t magic;
    uint8_t ident[12];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t ph_offset;
    uint32_t sh_offset;
    uint32_t flags;
    uint16_t header_size;
    uint16_t ph_entry_size;
    uint16_t ph_entry_count;
    uint16_t sh_size;
    uint16_t sh_entry_count;
    uint16_t sh_str_index;
} __attribute__((packed));

// Program Header
struct elf_program_header {
    uint32_t type;
    uint32_t offset;
    uint32_t virt_addr;
    uint32_t phys_addr;
    uint32_t file_size;
    uint32_t mem_size;
    uint32_t flags;
    uint32_t align;
} __attribute__((packed));

// Section Header
struct elf_section_header {
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t addr;
    uint32_t offset;
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t align;
    uint32_t ent_size;
} __attribute__((packed));

// Symbol Table Entry
struct elf32_symbol {
    uint32_t name;
    uint32_t value;
    uint32_t size;
    uint8_t info;
    uint8_t other;
    uint16_t shndx;
} __attribute__((packed));

// Relocation Entry (SHT_REL)
struct elf32_rel {
    uint32_t offset;
    uint32_t info;
} __attribute__((packed));

// Relocation Entry (SHT_RELA)
struct elf_rela_entry {
    uint32_t offset;
    uint32_t info;
    int32_t addend;
} __attribute__((packed));

struct ProgramArguments {
    const char* str1;
    const char* str2;
    const char* str3;
    const char* str4;
    const char* str5;
};

class ELFLoader {
private:
    Paging* pager;
    Scheduler* scheduler;  // Renamed to pManager for consistency, but implies Scheduler

public:
    ELFLoader(Paging* pager, Scheduler* pManager);
    ~ELFLoader();

    // Loads ELF from memory buffer
    ProcessControlBlock* loadELF(uint32_t mod_start, uint32_t mod_end, void* args);

    // Loads ELF from File object
    ProcessControlBlock* loadELF(File* elf, void* args);

    // Promotes a process to Kernel Privileges
    void ElevatetoKernel(ProcessControlBlock* pcb);
};

#endif  // ELF_H
