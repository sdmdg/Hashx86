#ifndef GDT_H
#define GDT_H

#include <types.h>

#define NO_GDT_DESCRIPTORS     8

// GDT Selectors (index * 8)
#define NULL_SELECTOR        0x00   // 0 * 8
#define KERNEL_CODE_SELECTOR 0x08   // 1 * 8
#define KERNEL_DATA_SELECTOR 0x10   // 2 * 8
#define USER_CODE_SELECTOR   0x18   // 3 * 8
#define USER_DATA_SELECTOR   0x20   // 4 * 8
#define TSS_SELECTOR         0x28   // 5 * 8

typedef struct {
    uint16_t segment_limit;  // segment limit first 0-15 bits
    uint16_t base_low;       // base first 0-15 bits
    uint8_t base_middle;     // base 16-23 bits
    uint8_t access;          // access byte
    uint8_t granularity;     // high 4 bits (flags) low 4 bits (limit 4 last bits)(limit is 20 bit wide)
    uint8_t base_high;       // base 24-31 bits
} __attribute__((packed)) GDT;

typedef struct {
    uint16_t limit;       // limit size of all GDT segments
    uint32_t base_address;  // base address of the first GDT segment
} __attribute__((packed)) GDT_PTR;

// asm gdt functions
extern "C" void load_gdt(uint32_t gdt_ptr);

void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

// initialize GDT
void gdt_init();

#endif  // GDT_H