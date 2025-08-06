#include <core/paging.h>
#include <debug.h>
#include <core/pmm.h>
#include <core/memory.h>

#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))

// Paging class constructor
Paging::Paging() : IS_PAGING_ENABLED(false) {
    memset(KernelPageDirectory, 0, sizeof(KernelPageDirectory));
    memset(KernelPageTables, 0, sizeof(KernelPageTables));
}

// Paging class destructor
Paging::~Paging() {}

// Activate paging
void Paging::Activate() {
    uint32_t cr0;

    KernalMappingEnd = pmm_alloc_block();
    DEBUG_LOG("Paging: Kernal is mapping upto: 0x%x", KernalMappingEnd);

    for (int i = 0; i < 1024; i++) {
        KernelPageDirectory[i] = ((uint32_t)KernelPageTables[i]) | 3; // Present, Read/Write
        for (int j = 0; j < 1024; j++) {
            KernelPageTables[i][j] = (i * 1024 + j) * PAGE_SIZE | 3; // Identity mapping
        }
    }

    asm volatile("mov %0, %%cr3" : : "r"(KernelPageDirectory));
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= (1 << 31);
    asm volatile("mov %0, %%cr0" : : "r"(cr0));

    IS_PAGING_ENABLED = true;
}

// Deactivate paging
void Paging::Deactivate() {
    asm volatile (
        "cli \n"                  // Disable interrupts
        "mov %%cr0, %%eax \n"
        "and $0x7FFFFFFF, %%eax \n" // Clear PG bit (bit 31)
        "mov %%eax, %%cr0 \n"
        "jmp 1f \n"                // Flush TLB
        "1: mov %%cr3, %%eax \n"
        "mov %%eax, %%cr3 \n"
        "sti \n"                   // Re-enable interrupts
        :
        :
        : "eax", "memory"
    );
    IS_PAGING_ENABLED = false;
}

// Retrieve the physical address for a virtual address
void* Paging::get_physical_address(uint32_t* PageDirectory, void* virtual_addr) {
    if (!IS_PAGING_ENABLED) return virtual_addr;

    uint32_t page_dir_index = (uint32_t)virtual_addr >> 22;
    uint32_t page_table_index = ((uint32_t)virtual_addr >> 12) & 0x03FF;
    uint32_t page_frame_offset = (uint32_t)virtual_addr & 0xFFF;

    if (!CHECK_BIT(PageDirectory[page_dir_index], 1)) return NULL;

    uint32_t *page_table = (uint32_t *)(PageDirectory[page_dir_index] & 0xFFFFF000);
    if (!CHECK_BIT(page_table[page_table_index], 1)) return NULL;

    return (void *)((page_table[page_table_index] & 0xFFFFF000) + page_frame_offset);
}

// Create a new page directory for a process
uint32_t* Paging::create_page_directory() {
    uint32_t *new_page_directory = (uint32_t *)kmalloc(PAGE_SIZE);
    if (!new_page_directory){
        DEBUG_LOG ("Error: Failed to allocate memory for page directory.");
        return NULL;
    }

    // Initialize the page directory to 0
    memset(new_page_directory, 0, PAGE_SIZE);

    // Ensure the kernel is mapped correctly
    map_kernel(new_page_directory);

    return new_page_directory;
}

// Map kernel memory into a process's page directory
void Paging::map_kernel(uint32_t* new_page_directory) {
    if (!new_page_directory) return;

    // Clear the entire page directory
    memset(new_page_directory, 0, 1024 * sizeof(uint32_t));

    // Calculate the number of kernel page directory entries
    uint32_t kernel_entries = ((uint32_t)KernalMappingEnd + (1 << 22) - 1) >> 22; // Round up to the nearest 4MB

    // Copy only the kernel mappings up to KernalMappingEnd
    memcpy(new_page_directory, KernelPageDirectory, kernel_entries * sizeof(uint32_t));
}


// Switch to a new page directory
void Paging::switch_page_directory(uint32_t* page_directory) {
    if (!IS_PAGING_ENABLED || !page_directory) return;
    asm volatile("mov %0, %%cr3" : : "r"(page_directory) : "memory");
    
/*     uint_fast32_t cr3pagetable;
    asm volatile("mov %%cr3, %0" : "=r"(cr3pagetable));
    DEBUG_LOG("Now Page : 0x%x", cr3pagetable);
 */
}

// Allocate a page for a virtual address
void Paging::allocate_page(uint32_t* page_directory, void* virtual_addr) {
    if (!IS_PAGING_ENABLED || !page_directory) return;

    uint32_t page_dir_index = (uint32_t)virtual_addr >> 22;
    uint32_t page_table_index = ((uint32_t)virtual_addr >> 12) & 0x03FF;
    
    // Check if this is the current active directory (can directly modify)
    uint32_t current_dir;
    asm volatile("mov %%cr3, %0" : "=r"(current_dir));
    bool is_active = (current_dir == (uint32_t)page_directory);

    // If page table doesn't exist, create it
    if (!(page_directory[page_dir_index] & 0x1)) {
        // Allocate a new physical page for the table
        uint32_t new_table_phys = (uint32_t)pmm_alloc_block();
        if (!new_table_phys) {
            DEBUG_LOG("Error: Memory allocation failed for page table");
            return;
        }

        // Zero the page table - this requires special handling
        // If it's not the active directory, we need a temporary mapping
        if (is_active) {
            // We can directly access it through the current page mapping
            memset((void*)new_table_phys, 0, PAGE_SIZE);
        } else {
            // We need to use identity mapping or temporarily map this page
            // This assumes your kernel space has identity mapping still enabled
            memset((void*)new_table_phys, 0, PAGE_SIZE);
        }

        // Mark the page table as present in the directory
        page_directory[page_dir_index] = new_table_phys | 3;  // Present, RW
    }

    // Get the physical address of the page table
    uint32_t page_table_phys = page_directory[page_dir_index] & 0xFFFFF000;
    
    // Access the page table - special handling needed if not active directory
    uint32_t* page_table;
    if (is_active) {
        // Can directly access through current mappings
        page_table = (uint32_t*)page_table_phys;
    } else {
        // Need to use identity mapping or temporarily map this page
        // Assuming identity mapping is still valid in kernel space
        page_table = (uint32_t*)page_table_phys;
    }

    // Allocate the actual page if not already present
    if (!(page_table[page_table_index] & 0x1)) {
        uint32_t frame = (uint32_t)pmm_alloc_block();
        if (!frame) {
            DEBUG_LOG("Error: Memory allocation failed for page frame");
            return;
        }

        page_table[page_table_index] = frame | 3;  // Present, RW
    }
}


// Free a page from a process-specific page table
void Paging::free_page(uint32_t* page_directory, void* virtual_addr) {
    if (!IS_PAGING_ENABLED || !page_directory) return;

    uint32_t page_dir_index = (uint32_t)virtual_addr >> 22;
    uint32_t page_table_index = ((uint32_t)virtual_addr >> 12) & 0x03FF;

    if (!CHECK_BIT(page_directory[page_dir_index], 1)) return;

    uint32_t *page_table = (uint32_t *)(page_directory[page_dir_index] & 0xFFFFF000);
    if (!CHECK_BIT(page_table[page_table_index], 1)) return;

    // Free the allocated physical memory
    pmm_free_block((void *)(page_table[page_table_index] & 0xFFFFF000));
    page_table[page_table_index] = 0;
}
