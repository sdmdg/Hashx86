/**
 * @file        elf.cpp
 * @brief       ELF Binary Loader for #x86
 *
 * @date        29/01/2026
 * @version     1.0.0-beta
 */

#include <core/elf.h>

ELFLoader::ELFLoader(Paging* pager, Scheduler* scheduler) {
    this->pager = pager;
    this->scheduler = scheduler;
};
ELFLoader::~ELFLoader(){

};

ProcessControlBlock* ELFLoader::loadELF(File* elf, void* args) {
    if (!elf) return nullptr;

    // Validate Header
    struct elf_header header;
    elf->Seek(0);
    if (elf->Read((uint8_t*)&header, sizeof(elf_header)) != sizeof(elf_header)) {
        DEBUG_LOG("Error: ELF file too short");
        return nullptr;
    }

    if (header.magic != ELF_MAGIC) {
        DEBUG_LOG("Error: Invalid ELF Magic!");
        return nullptr;
    }

    // Create new PCB
    ProcessControlBlock* pELF =
        scheduler->CreateProcess(false, (void (*)(void*))header.entry, args);

    // Read ELF Headers
    uint32_t ph_size = sizeof(elf_program_header) * header.ph_entry_count;
    elf_program_header* ph_table = new elf_program_header[header.ph_entry_count];
    if (!ph_table) {
        HALT("CRITICAL: Failed to allocate ELF program header table!\n");
    }

    elf->Seek(header.ph_offset);
    if (elf->Read((uint8_t*)ph_table, ph_size) != ph_size) {
        DEBUG_LOG("Error: Could not read Program Headers");
        delete[] ph_table;
        // Ideally kill the process here too
        return nullptr;
    }

    uint32_t max_virt_end = 0;

    // Load ELF Segments
    for (int i = 0; i < header.ph_entry_count; i++) {
        elf_program_header* ph = &ph_table[i];

        if (ph->type != 1) continue;  // PT_LOAD only

        // Calculate alignment
        uint32_t start = (uint32_t)ph->virt_addr;
        uint32_t end = start + ph->mem_size;
        uint32_t page_start = start & ~(PAGE_SIZE - 1);
        uint32_t page_end = (end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

        // Allocate Pages
        for (uint32_t addr = page_start; addr < page_end; addr += PAGE_SIZE) {
            uint32_t phys_frame = (uint32_t)pmm_alloc_block();
            this->pager->MapPage(pELF->page_directory, addr, phys_frame,
                                 PAGE_PRESENT | PAGE_RW | PAGE_USER  // Allow User Mode Access!
            );
        }

        // Load Data into those pages
        uint32_t bytes_to_read = ph->file_size;
        uint32_t virtual_addr = (uint32_t)ph->virt_addr;

        // Read ph header
        elf->Seek(ph->offset);

        while (bytes_to_read > 0) {
            uint32_t phys_ptr = pager->GetPhysicalAddress(pELF->page_directory, virtual_addr);

            uint32_t offset_in_page = virtual_addr % PAGE_SIZE;
            uint32_t space_in_page = PAGE_SIZE - offset_in_page;
            uint32_t chunk = (bytes_to_read < space_in_page) ? bytes_to_read : space_in_page;

            elf->Read((uint8_t*)phys_ptr, chunk);

            virtual_addr += chunk;
            bytes_to_read -= chunk;
        }

        // Zero out BSS
        uint32_t bytes_to_zero = ph->mem_size - ph->file_size;
        while (bytes_to_zero > 0) {
            uint32_t phys_ptr = pager->GetPhysicalAddress(pELF->page_directory, virtual_addr);

            uint32_t offset_in_page = virtual_addr % PAGE_SIZE;
            uint32_t space_in_page = PAGE_SIZE - offset_in_page;
            uint32_t chunk = (bytes_to_zero < space_in_page) ? bytes_to_zero : space_in_page;

            memset((void*)phys_ptr, 0, chunk);

            virtual_addr += chunk;
            bytes_to_zero -= chunk;
        }

        if (end > max_virt_end) max_virt_end = end;
    }

    delete[] ph_table;

    // Allocate User Heap (Simple)
    max_virt_end = (max_virt_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    const int HEAP_PAGE_COUNT = 64;  // 256KB Initial Heap
    uint32_t heap_start = max_virt_end;
    uint32_t heap_end = heap_start + HEAP_PAGE_COUNT * PAGE_SIZE;

    for (uint32_t addr = heap_start; addr < heap_end; addr += PAGE_SIZE) {
        uint32_t phys_frame = (uint32_t)pmm_alloc_block();
        memset((void*)phys_frame, 0,
               PAGE_SIZE);  // Zero heap pages to prevent uninitialized memory bugs
        this->pager->MapPage(pELF->page_directory, addr, phys_frame,
                             PAGE_PRESENT | PAGE_RW | PAGE_USER);
    }

    pELF->heap.startAddress = heap_start;
    pELF->heap.endAddress = heap_end;
    pELF->heap.maxAddress = heap_end + (1024 * 1024 * 16);  // Allow growing up to 16MB later

    DEBUG_LOG("ELF Loaded. Entry: 0x%x Heap: 0x%x - 0x%x", header.entry, heap_start, heap_end);

    return pELF;
};

void ELFLoader::ElevatetoKernel(ProcessControlBlock* pELF) {
    // pELF->isKernelProcess = true;
};
