#ifndef ELF_H
#define ELF_H

#include <stdint.h>
#include <core/memory.h>
#include <core/process.h>

#define ELF_MAGIC 0x464C457F

// ELF Header
struct elf_header {
    uint32_t    magic;
    uint32_t    version;
    uint64_t    reserved;
    uint64_t    version2;
    uint32_t    entry;
    uint32_t    ph_offset;
    uint32_t    sh_offset;
    uint32_t    flags;
    uint16_t    header_size;
    uint16_t    ph_entry_size;
    uint16_t    ph_entry_count;
    uint16_t    sh_entry_size;
    uint16_t    sh_entry_count;
    uint16_t    sh_str_table_index;
} __attribute__((packed));

// Program Header (Segment)
struct elf_program_header {
    uint32_t    type;
    uint32_t    offset;
    uint32_t    virt_addr;
    uint32_t    phys_addr;
    uint32_t    file_size;
    uint32_t    mem_size;
    uint32_t    flags;
    uint32_t    alignment;
} __attribute__((packed));

// Section Header (Used for relocations)
struct elf_section_header {
    uint32_t    name;
    uint32_t    type;
    uint32_t    flags;
    uint32_t    addr;
    uint32_t    offset;
    uint32_t    size;
    uint32_t    link;
    uint32_t    info;
    uint32_t    addr_align;
    uint32_t    entry_size;
} __attribute__((packed));

// Relocation Types
#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_HASH     5
#define SHT_DYNAMIC  6
#define SHT_NOTE     7
#define SHT_NOBITS   8
#define SHT_REL      9
#define SHT_SHLIB    10
#define SHT_DYNSYM   11

// Relocation Entry (SHT_REL)
struct elf_rel_entry {
    uint32_t    offset;
    uint32_t    info;
} __attribute__((packed));

// Relocation Entry (SHT_RELA)
struct elf_rela_entry {
    uint32_t    offset;
    uint32_t    info;
    int32_t     addend;
} __attribute__((packed));

// Macros to extract relocation info
#define ELF32_R_SYM(info)   ((info) >> 8)
#define ELF32_R_TYPE(info)  ((uint8_t)(info))


struct ProgramArguments {
    const char* str1;
    const char* str2;
    const char* str3;
    const char* str4;
    const char* str5;
};


class ELFLoader
{
private:
    GlobalDescriptorTable *gdt;
    Paging* pager;
    ProcessManager* pManager;
    
public:
    ELFLoader(GlobalDescriptorTable *gdt, Paging* pager, ProcessManager* pManager);
    ~ELFLoader();
    Process* loadModule(uint32_t mod_start, uint32_t mod_end, void* args);
    void startModule(Process* pELF);
    void ElevatetoKernel(Process* pELF);
};


#endif
