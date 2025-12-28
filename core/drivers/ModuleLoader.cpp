#include <core/drivers/ModuleLoader.h>

// ELF Macros for x86 Relocation
#define ELF32_R_SYM(i)  ((i) >> 8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define R_386_32        1                       // Absolute 32-bit
#define R_386_PC32      2                       // PC-Relative 32-bit

void* ModuleLoader::LoadMatchingDriver(File* file, uint16_t target_vid, uint16_t target_did) {
    if (!file) return 0;

    DriverManifest manifest;

    // Read the header
    if (!ModuleLoader::Probe(file, &manifest)) {
        printf("[ModuleLoader] Error: File is not a valid driver (Missing .driver_info)\n");
        return 0;
    }

    // CHECK: Loop through the supported devices array
    bool match = false;
    for (int i = 0; i < 4; i++) {
        // Break if hit an empty slot
        if (manifest.devices[i].vendor_id == 0) break; 

        if (manifest.devices[i].vendor_id == target_vid && 
            manifest.devices[i].device_id == target_did) {
            match = true;
            break;
        }
    }

    if (match) {
        printf("[ModuleLoader] Match: %s v%s supports hardware %x:%x. Loading...\n", manifest.name, manifest.version, target_vid, target_did);
        return ModuleLoader::LoadDriver(file); 
        
    } else {
        printf("[ModuleLoader] Skip: %s does not support %x:%x\n", manifest.name, target_vid, target_did);
        return 0;
    }
}


void* ModuleLoader::LoadDriver(File* file) {
    if (!file) return 0;

    // Read ELF Header
    struct elf_header header;
    file->Seek(0);
    file->Read((uint8_t*)&header, sizeof(header));

    if (header.magic != ELF_MAGIC) {
        printf("Module Error: Invalid ELF Magic\n");
        return 0;
    }
    if (header.type != 1) { // ET_REL = 1 (Relocatable)
        printf("Module Error: Not a relocatable object (.o)\n");
        return 0;
    }

    // Read Section Headers
    uint32_t sh_size = header.sh_entry_count * header.sh_size;
    struct elf_section_header* sections = (struct elf_section_header*)kmalloc(sh_size);
    file->Seek(header.sh_offset);
    file->Read((uint8_t*)sections, sh_size);

    // Read Section String Table (to find section names if needed)
    struct elf_section_header* strtab_hdr = &sections[header.sh_str_index];
    char* strtab = (char*)kmalloc(strtab_hdr->size);
    file->Seek(strtab_hdr->offset);
    file->Read((uint8_t*)strtab, strtab_hdr->size);

    // Allocate Memory for Sections
    // Iterate all sections. If flags has SHF_ALLOC (0x2).
    for (int i = 0; i < header.sh_entry_count; i++) {
        if (sections[i].flags & 0x2) { 
            void* mem = kmalloc(sections[i].size);
            
            // If it is NOT BSS (NOBITS), read data from file
            if (sections[i].type != 8) { 
                file->Seek(sections[i].offset);
                file->Read((uint8_t*)mem, sections[i].size);
            } else {
                // Zero out BSS
                memset(mem, 0, sections[i].size); 
            }
            // Store the KERNEL VIRTUAL ADDRESS in the section header's 'addr' field
            // Use this later to resolve addresses.
            sections[i].addr = (uint32_t)mem;
        } else {
            sections[i].addr = 0;
        }
    }

    // Link (Relocate)
    struct elf32_symbol* symtab = 0;
    uint32_t symtab_count = 0;
    char* strtab_sym = 0;

    // Find Symbol Table
    for (int i = 0; i < header.sh_entry_count; i++) {
        if (sections[i].type == 2) { // SHT_SYMTAB
            symtab = (struct elf32_symbol*)kmalloc(sections[i].size);
            file->Seek(sections[i].offset);
            file->Read((uint8_t*)symtab, sections[i].size);
            symtab_count = sections[i].size / sizeof(elf32_symbol);
            
            // Load associated string table
            int link = sections[i].link;
            strtab_sym = (char*)kmalloc(sections[link].size);
            file->Seek(sections[link].offset);
            file->Read((uint8_t*)strtab_sym, sections[link].size);
            break;
        }
    }

    // Process Relocation Sections
    for (int i = 0; i < header.sh_entry_count; i++) {
        if (sections[i].type == 9) { // SHT_REL (Relocation without Addend)
            
            uint32_t count = sections[i].size / sections[i].ent_size;
            
            // Allocate buffer for relocs
            struct elf32_rel* rels = (struct elf32_rel*)kmalloc(sections[i].size);
            file->Seek(sections[i].offset);
            file->Read((uint8_t*)rels, sections[i].size);

            // The section modifying (patching)
            uint32_t target_section_idx = sections[i].info;
            uint32_t target_base = sections[target_section_idx].addr;

            for (uint32_t r = 0; r < count; r++) {
                uint32_t sym_idx = ELF32_R_SYM(rels[r].info);
                uint32_t type = ELF32_R_TYPE(rels[r].info);
                uint32_t offset = rels[r].offset; // Offset inside the target section
                
                // Need to write the patched address
                uint32_t* patch_addr = (uint32_t*)(target_base + offset);
                
                // Resolve Symbol Value
                uint32_t sym_val = 0;
                
                if (symtab[sym_idx].shndx == 0) { 
                    // SHN_UNDEF: External Symbol (e.g. printf)
                    // Lookup in Kernel Symbol Table
                    const char* name = strtab_sym + symtab[sym_idx].name;
                    sym_val = SymbolTable::Lookup(name);
                    
                    if (sym_val == 0) {
                        printf("Module Link Error: Undefined symbol '%s'\n", name);
                    }
                } else {
                    // Internal Symbol (defined in another section of this module)
                    uint32_t sec_idx = symtab[sym_idx].shndx;
                    // Address = Base of Section + Offset inside section
                    sym_val = sections[sec_idx].addr + symtab[sym_idx].value;
                }

                // Apply Logic
                if (type == R_386_32) {
                    // S + A (Addend is implicit in the target memory location)
                    *patch_addr += sym_val; 
                } else if (type == R_386_PC32) {
                    // S + A - P (Symbol - Location)
                    *patch_addr += (sym_val - (uint32_t)patch_addr); 
                }
            }
            kfree(rels);
        }
    }

    // Find Entry Point (CreateDriverInstance)
    void* entry_point = 0;
    if (symtab) {
        for (uint32_t i = 0; i < symtab_count; i++) {
            const char* name = strtab_sym + symtab[i].name;
            
            // Check for the magic function name
            // Simple manual strcmp
            const char* target = "CreateDriverInstance";
            bool match = true;
            for(int c=0; target[c]; c++) if (target[c] != name[c]) match=false;
            
            if (match) {
                uint32_t sec_idx = symtab[i].shndx;
                if (sec_idx != 0 && sec_idx < header.sh_entry_count) {
                    entry_point = (void*)(sections[sec_idx].addr + symtab[i].value);
                    break;
                }
            }
        }
    }

    // Cleanup
    kfree(sections);
    kfree(strtab);
    kfree(symtab);
    kfree(strtab_sym);

    return entry_point;
}


// Returns true if valid metadata is found, filling the 'info' struct
bool ModuleLoader::Probe(File* file, DriverManifest* info) {
    if (!file || !info) return false;

    // Read ELF Header
    struct elf_header header;
    file->Seek(0);
    file->Read((uint8_t*)&header, sizeof(header));

    if (header.magic != ELF_MAGIC) return false;

    // Read Section Headers
    uint32_t sh_size = header.sh_entry_count * header.sh_size;
    struct elf_section_header* sections = (struct elf_section_header*)kmalloc(sh_size);
    file->Seek(header.sh_offset);
    file->Read((uint8_t*)sections, sh_size);

    // Read Section String Table (to find section names)
    // Need this to search for ".driver_info" by name
    struct elf_section_header* strtab_hdr = &sections[header.sh_str_index];
    char* strtab = (char*)kmalloc(strtab_hdr->size);
    file->Seek(strtab_hdr->offset);
    file->Read((uint8_t*)strtab, strtab_hdr->size);

    bool found = false;

    // Iterate Sections to find ".driver_info"
    for (int i = 0; i < header.sh_entry_count; i++) {
        const char* sec_name = strtab + sections[i].name;
        
        // Manual string comparison for ".driver_info"
        const char* target = ".driver_info";
        bool match = true;
        for(int c=0; target[c] != 0; c++) {
            if (target[c] != sec_name[c]) { match=false; break; }
        }
        // Ensure the names are the same length (null terminator check)
        if (match && sec_name[12] != 0) match = false; 

        // If found, verify size and read data
        if (match) {
            // Safety Check:
            // This prevents reading garbage if the driver was compiled with an old struct definition.
            if (sections[i].size >= sizeof(DriverManifest)) {
                
                file->Seek(sections[i].offset);
                file->Read((uint8_t*)info, sizeof(DriverManifest));
                
                if (info->magic == DRIVER_INFO_MAGIC) {
                    found = true;
                }
            } else {
                printf("[ModuleLoader] Warning: '.driver_info' section too small (Old driver version?)\n");
            }
            break; // Stop searching once found
        }
    }

    // Cleanup Heap
    kfree(sections);
    kfree(strtab);
    
    // Important: Reset file pointer to 0
    file->Seek(0);
    
    return found;
}