#ifndef PAGING_H
#define PAGING_H

#include "types.h"

#define PAGE_SIZE 4096

class Paging {
private:
    bool IS_PAGING_ENABLED;
    void* KernalMappingEnd;
    uint32_t KernelPageTables[1024][1024] __attribute__((aligned(4096)));
    
public:
    Paging();
    ~Paging();

    uint32_t KernelPageDirectory[1024] __attribute__((aligned(4096)));

    // Activate paging
    void Activate();

    // Deactivate paging
    void Deactivate();

    // Retrieves the physical address corresponding to a virtual address
    void* get_physical_address(uint32_t* PageDirectory, void* virtual_addr);

    // Creates a new page directory for a process
    uint32_t* create_page_directory();

    // Switches the active page directory
    void switch_page_directory(uint32_t* page_directory);

    // Maps kernel space into a process page directory
    void map_kernel(uint32_t* new_page_directory);

    // Allocates a page in a process-specific page table
    void allocate_page(uint32_t* page_directory, void* virtual_addr);

    void map_page(uint32_t* page_directory, void* phys_addr, void* virt_addr);

    // Frees a page from a process-specific page table
    void free_page(uint32_t* page_directory, void* virtual_addr);
};

#endif
