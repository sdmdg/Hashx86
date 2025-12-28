#include <core/elf.h>

ELFLoader::ELFLoader(Paging* pager, ProcessManager* pManager){
    this->pager = pager;
    this->pManager = pManager;
};
ELFLoader::~ELFLoader(){
    
};

Process* ELFLoader::loadELF(uint32_t mod_start, uint32_t mod_end, void* args){
    uint32_t mod_length = mod_end - mod_start;

    DEBUG_LOG("Module start: 0x%x, end: 0x%x, length: %d bytes", mod_start, mod_end, mod_length);

    struct elf_header* header = (struct elf_header*)mod_start;
    struct elf_program_header* ph;

    if (header->magic != ELF_MAGIC) {
        DEBUG_LOG("Error: Invalid ELF Magic!");
        return nullptr; // Returning a valid default value
    }

    Process* pELF = new Process(pager, (void (*)(void*))header->entry, args);

    // Load ELF program headers
    ph = (struct elf_program_header*) (((char*) mod_start) + header->ph_offset);
    uint32_t start = ((uint32_t) ph->virt_addr) - PAGE_SIZE;  // Actual load address
    uint32_t end = start + ph->mem_size;        // Memory size to be allocated

    uint32_t max_virt_end = 0;

    for (int i = 0; i < header->ph_entry_count; i++, ph++) {
        if (ph->type != 1) { // PT_LOAD segment only
            continue;
        }

        void* dest = (void*) ph->virt_addr;
        void* src = ((char*) mod_start) + ph->offset;

        // Allocate pages for this segment
        uint32_t start = (uint32_t) dest;
        uint32_t end = start + ph->mem_size;
        for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
            this->pager->allocate_page(pELF->process_page_directory, (void*)addr);
        }

        //asm volatile("mov %0, %%cr3" : : "r"(pELF->process_page_directory) : "memory");
        // Zero out memory, then copy the segment
        memset(pager->get_physical_address(pELF->process_page_directory, dest), 0, ph->mem_size);
        memcpy(pager->get_physical_address(pELF->process_page_directory, dest), src, ph->file_size);
        //asm volatile("mov %0, %%cr3" : : "r"(this->pager->KernelPageDirectory) : "memory");

        // Track max used virtual address for heap placement
        if (end > max_virt_end) max_virt_end = end;
    }

    // Align to next page
    max_virt_end = (max_virt_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    // Allocate a heap (e.g., 32 pages = 128KB)
    const int HEAP_PAGE_COUNT = 32;
    uint32_t heap_start = max_virt_end;
    uint32_t heap_end = heap_start + HEAP_PAGE_COUNT * PAGE_SIZE;

    for (uint32_t addr = heap_start; addr < heap_end; addr += PAGE_SIZE) {
        pager->allocate_page(pELF->process_page_directory, (void*)addr);
    }

    // Save heap info in Process object
    pELF->heapData.heap_start = heap_start;
    pELF->heapData.heap_end = heap_end;

    DEBUG_LOG("Heap allocated: 0x%x - 0x%x", heap_start, heap_end);

    return pELF;
};

Process* ELFLoader::loadELF(File* elf, void* args) {
    if (!elf) return nullptr;

    // Read ELF Header
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

    // Create Process (Allocates Page Directory)
    Process* pELF = new Process(pager, (void (*)(void*))header.entry, args);

    // Read Program Header Table
    uint32_t ph_size = sizeof(elf_program_header) * header.ph_entry_count;
    elf_program_header* ph_table = new elf_program_header[header.ph_entry_count];
    
    elf->Seek(header.ph_offset);
    if (elf->Read((uint8_t*)ph_table, ph_size) != ph_size) {
        DEBUG_LOG("Error: Could not read Program Headers");
        delete[] ph_table;
        // Note: Should theoretically delete pELF here too
        return nullptr;
    }

    uint32_t max_virt_end = 0;

    // Iterate Segments
    for (int i = 0; i < header.ph_entry_count; i++) {
        elf_program_header* ph = &ph_table[i];
        
        if (ph->type != 1) continue; // PT_LOAD segments only

        // Allocate Virtual Memory Pages
        uint32_t start = (uint32_t)ph->virt_addr;
        uint32_t end = start + ph->mem_size;
        
        // Align to Page Boundaries for allocation
        uint32_t page_start = start & ~(PAGE_SIZE - 1);
        uint32_t page_end = (end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

        for (uint32_t addr = page_start; addr < page_end; addr += PAGE_SIZE) {
             this->pager->allocate_page(pELF->process_page_directory, (void*)addr);
        }

        // LOAD FILE DATA directly into Memory (Page by Page)
        // We cannot just read the whole chunk because physical pages might be non-contiguous.
        
        uint32_t bytes_to_read = ph->file_size;
        uint32_t virtual_addr = (uint32_t)ph->virt_addr;
        
        // Seek File to Segment Start
        elf->Seek(ph->offset);

        while (bytes_to_read > 0) {
             // Get the Physical Address where this Virtual Address maps to
             void* phys_ptr = pager->get_physical_address(pELF->process_page_directory, (void*)virtual_addr);
             
             // Calculate safe chunk size for this page
             uint32_t offset_in_page = virtual_addr % PAGE_SIZE;
             uint32_t space_in_page = PAGE_SIZE - offset_in_page;
             uint32_t chunk = (bytes_to_read < space_in_page) ? bytes_to_read : space_in_page;

             // Read directly from disk to RAM
             elf->Read((uint8_t*)((uint32_t)phys_ptr), chunk);

             virtual_addr += chunk;
             bytes_to_read -= chunk;
        }

        // ZERO OUT BSS (Remaining Memory Size)
        uint32_t bytes_to_zero = ph->mem_size - ph->file_size;
        
        while (bytes_to_zero > 0) {
             void* phys_ptr = pager->get_physical_address(pELF->process_page_directory, (void*)virtual_addr);
             uint32_t offset_in_page = virtual_addr % PAGE_SIZE;
             uint32_t space_in_page = PAGE_SIZE - offset_in_page;
             uint32_t chunk = (bytes_to_zero < space_in_page) ? bytes_to_zero : space_in_page;

             memset((void*)((uint32_t)phys_ptr), 0, chunk);

             virtual_addr += chunk;
             bytes_to_zero -= chunk;
        }

        if (end > max_virt_end) max_virt_end = end;
    }

    delete[] ph_table;

    // Allocate Heap (Standard logic)
    max_virt_end = (max_virt_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    const int HEAP_PAGE_COUNT = 32;
    uint32_t heap_start = max_virt_end;
    uint32_t heap_end = heap_start + HEAP_PAGE_COUNT * PAGE_SIZE;

    for (uint32_t addr = heap_start; addr < heap_end; addr += PAGE_SIZE) {
        pager->allocate_page(pELF->process_page_directory, (void*)addr);
    }

    pELF->heapData.heap_start = heap_start;
    pELF->heapData.heap_end = heap_end;

    DEBUG_LOG("ELF Loaded. Entry: 0x%x Heap: 0x%x - 0x%x", header.entry, heap_start, heap_end);

    return pELF;
};

void ELFLoader::runELF(Process* pELF){
    this->pManager->AddProcess(pELF);
};

void ELFLoader::ElevatetoKernel(Process* pELF){
    pManager->mapKernel(pELF);
};