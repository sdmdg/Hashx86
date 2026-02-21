/**
 * @file        paging.cpp
 * @brief       Page Table Manager for #x86
 *
 * @date        29/01/2026
 * @version     1.0.0-beta
 */

#define KDBG_COMPONENT "PAGING"
#include <core/paging.h>

Paging::Paging() : is_paging_active(false) {}

Paging::~Paging() {}

void Paging::Activate() {
    // Allocate the Master Page Directory
    // Must be in identity-mapped range (<256MB) so kernel can access it after paging
    KernelPageDirectory = (uint32_t*)pmm_alloc_block_low(256 * 1024 * 1024);

    if (!KernelPageDirectory || ((uint32_t)KernelPageDirectory & 0xFFF)) {
        KDBG1("CRITICAL ERROR: Page Directory NOT Aligned! Addr: 0x%x", KernelPageDirectory);
        while (1);  // Halt
    }

    // Memset 0;
    memset(KernelPageDirectory, 0, 4096);

    // --------------------------------------------------------
    // Map Lower Memory (0MB - 256MB) | Kernel Code
    // 256MB / 4MB per table = 64 Tables
    for (uint32_t i = 0; i < 64; i++) {
        // Allocate a Page Table (Holds 1024 pages)
        // Must be in identity-mapped range (<256MB)
        uint32_t* page_table = (uint32_t*)pmm_alloc_block_low(256 * 1024 * 1024);
        if (!page_table) {
            KDBG1("CRITICAL: Failed to allocate page table for index %d!", i);
            while (1);
        }
        memset(page_table, 0, 4096);

        // Fill the table (Identity Map: Virtual X = Physical X)
        for (uint32_t j = 0; j < 1024; j++) {
            uint32_t phys_addr = (i * 1024 + j) * 4096;
            // Flags: Present | ReadWrite
            page_table[j] = phys_addr | PAGE_PRESENT | PAGE_RW;
        }

        // Put the table into the Directory
        KernelPageDirectory[i] = ((uint32_t)page_table) | PAGE_PRESENT | PAGE_RW;
    }

    // --------------------------------------------------------
    // Map High Memory (3GB - 4GB) | VRAM / MMIO
    // Indices 768 to 1024. Covers 0xC0000000 to 0xFFFFFFFF.
    for (uint32_t i = 768; i < 1024; i++) {
        // Must be in identity-mapped range (<256MB)
        uint32_t* page_table = (uint32_t*)pmm_alloc_block_low(256 * 1024 * 1024);
        if (!page_table) {
            KDBG1("CRITICAL: Failed to allocate high-mem page table for index %d!", i);
            while (1);
        }
        memset(page_table, 0, 4096);

        // Identity map high memory address
        for (uint32_t j = 0; j < 1024; j++) {
            uint32_t phys_addr = (i * 1024 + j) * 4096;
            page_table[j] = phys_addr | PAGE_PRESENT | PAGE_RW;
        }

        KernelPageDirectory[i] = ((uint32_t)page_table) | PAGE_PRESENT | PAGE_RW;
    }

    // --------------------------------------------------------
    // Enable Paging
    // Load CR3 with the Physical Address of the Directory
    asm volatile("mov %0, %%cr3" : : "r"(KernelPageDirectory));

    // Enable PG bit in CR0
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));

    is_paging_active = true;
    KDBG1("Activated. Kernel (Low) and Hardware (High) Mapped.");
}

uint32_t* Paging::CreateProcessDirectory() {
    // Allocate a new Directory with pmm_alloc for 4kb alignment
    // Must be in identity-mapped range (<256MB) so kernel can read/write entries
    uint32_t* new_dir = (uint32_t*)pmm_alloc_block_low(256 * 1024 * 1024);
    if (!new_dir) return 0;

    // Clear user space
    memset(new_dir, 0, 4096);

    // Link Kernel Space (Low Memory: 0-256MB)
    for (int i = 0; i < 64; i++) {
        new_dir[i] = KernelPageDirectory[i];
    }

    // Link Hardware Space (High Memory: 3GB-4GB)
    for (int i = 768; i < 1024; i++) {
        new_dir[i] = KernelPageDirectory[i];
    }

    KDBG2("CreateProcessDirectory addr=0x%x", new_dir);
    return new_dir;
}

void Paging::SwitchDirectory(uint32_t* new_dir) {
    if (!new_dir) return;
    asm volatile("mov %0, %%cr3" : : "r"(new_dir));
    KDBG3("SwitchDirectory addr=0x%x", new_dir);
}

bool Paging::MapPage(uint32_t* directory, uint32_t virtual_addr, uint32_t physical_addr,
                     uint32_t flags) {
    uint32_t pd_idx = virtual_addr >> 22;
    uint32_t pt_idx = (virtual_addr >> 12) & 0x03FF;

    // Check if Page Table exists
    if (!(directory[pd_idx] & PAGE_PRESENT)) {
        // Allocate new table via PMM (LOW MEMORY < 256MB)
        uint32_t* new_table = (uint32_t*)pmm_alloc_block_low(256 * 1024 * 1024);

        if (!new_table) {
            KDBG1("MapPage: Failed to allocate Page Table! Low Memory Exhausted?");
            return false;
        }

        memset(new_table, 0, 4096);

        // Link it
        directory[pd_idx] = (uint32_t)new_table | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    }

    uint32_t* table = (uint32_t*)(directory[pd_idx] & 0xFFFFF000);
    table[pt_idx] = (physical_addr & 0xFFFFF000) | flags;

    // Invalidate TLB entry for this virtual address.
    // Without this, stale TLB entries can cause phantom page faults
    // when pages are newly mapped or permissions are changed.
    asm volatile("invlpg (%0)" ::"r"(virtual_addr) : "memory");
    KDBG2("MapPage virt=0x%x phys=0x%x flags=0x%x", virtual_addr, physical_addr, flags);
    return true;
}

uint32_t Paging::GetPhysicalAddress(uint32_t* directory, uint32_t virtual_addr) {
    uint32_t pd_idx = virtual_addr >> 22;
    uint32_t pt_idx = (virtual_addr >> 12) & 0x03FF;

    if (!(directory[pd_idx] & PAGE_PRESENT)) return 0;

    uint32_t* table = (uint32_t*)(directory[pd_idx] & 0xFFFFF000);
    if (!(table[pt_idx] & PAGE_PRESENT)) return 0;

    return (table[pt_idx] & 0xFFFFF000) + (virtual_addr & 0xFFF);
}
