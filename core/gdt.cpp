/**
 * @file        gdt.cpp
 * @brief       Global Descriptor Table (GDT) Implementation
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

/**
 * Global Descriptor Table(GDT) setup
 */
#include <core/gdt.h>
#include <core/tss.h>

extern "C" void tss_flush();

GDT g_gdt[NO_GDT_DESCRIPTORS];

GDT_PTR g_gdt_ptr;
TaskStateSegment g_tss;

/**
 * fill entries of GDT
 */
void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    GDT *entry = &g_gdt[index];

    entry->segment_limit = limit & 0xFFFF;
    entry->base_low = base & 0xFFFF;
    entry->base_middle = (base >> 16) & 0xFF;
    entry->access = access;

    entry->granularity = (limit >> 16) & 0x0F;
    entry->granularity = entry->granularity | (gran & 0xF0);

    entry->base_high = (base >> 24 & 0xFF);
}

// initialize GDT
void gdt_init() {
    g_gdt_ptr.limit = sizeof(g_gdt) - 1;
    g_gdt_ptr.base_address = (uint32_t)g_gdt;

    // NULL segment
    gdt_set_entry(0, 0, 0, 0, 0);
    // code segment
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    // data segment
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    // user code segment
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    // user data segment
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    // TSS segment
    // memset(&g_tss, 0, sizeof(TaskStateSegment));

    // Set the Kernel Stack Segment (SS0) to Kernel Data (0x10)
    g_tss.ss0 = 0x10;
    // Point iomap_base to size (disables bitmap)
    g_tss.iomap_base = sizeof(TaskStateSegment);

    // Add to GDT
    // Base: &g_tss
    // Limit: size - 1
    // Access: 0x89 (Present | Ring 0 | System Segment | Type=32bit TSS Available)
    // Flags: 0x00 (Byte granularity)
    gdt_set_entry(5, (uint32_t)&g_tss, sizeof(g_tss) - 1, 0x89, 0x00);

    // Load GDT
    load_gdt((uint32_t)&g_gdt_ptr);

    // Load TSS (Task Register)
    tss_flush();
}
