/**
 * @file        gdt.cpp
 * @brief       Global Descriptor Table
 * 
 * @date        13/01/2025
 * @version     1.0.0
 */

#include <core/gdt.h>

/**
 * Constructor for GlobalDescriptorTable.
 * 
 * Initializes the GDT with a null segment, an unused segment, a code segment,
 * and a data segment.
 */
GlobalDescriptorTable::GlobalDescriptorTable()
    : nullSegmentSelector(0, 0, 0),                     // Null descriptor
      unusedSegmentSelector(0, 0, 0),                   // Reserved descriptor
      codeSegmentSelector(0, 64 * 1024 * 1024, 0x9A),   // Code segment: execute/read
      dataSegmentSelector(0, 64 * 1024 * 1024, 0x92)    // Data segment: read/write
{
    LoadGDT();
}

/**
 * Loads the GDT using the `lgdt` instruction.
 */
void GlobalDescriptorTable::LoadGDT(){
    uint32_t gdt_descriptor[2];
    gdt_descriptor[1] = (uint32_t)this;                 // Base address of GDT
    gdt_descriptor[0] = sizeof(GlobalDescriptorTable) << 16; // Limit (size - 1)

    // Load the GDT and set the segment registers
    asm volatile(
        "lgdt %0\n"                // Load GDT
        "mov %2, %%ds\n"           // Set Data Segment Register
        "mov %2, %%es\n"           // Set Extra Segment Register
        "mov %2, %%fs\n"           // Set FS Register
        "mov %2, %%gs\n"           // Set GS Register
        "mov %2, %%ss\n"           // Set Stack Segment Register
        "push %1\n"                // Push Code Segment Selector
        "push $1f\n"               // Push Instruction Pointer
        "retf\n"                   // Far return to update CS
        "1:\n"                     // Target of far return
        :
        : "m"(*(((uint8_t *)gdt_descriptor) + 2)),  // Pass GDT descriptor
          "r"(CodeSegmentSelector()), 
          "r"(DataSegmentSelector())
        : "memory"
    );
}


/**
 * Destructor for GlobalDescriptorTable.
 */
GlobalDescriptorTable::~GlobalDescriptorTable() {
}

/**
 * Retrieves the offset for the data segment.
 * 
 * @return Offset from the start of the GDT to the data segment descriptor.
 */
uint16_t GlobalDescriptorTable::DataSegmentSelector() {
    return (uint8_t *)&dataSegmentSelector - (uint8_t *)this;
}

/**
 * Retrieves the offset for the code segment.
 * 
 * @return Offset from the start of the GDT to the code segment descriptor.
 */
uint16_t GlobalDescriptorTable::CodeSegmentSelector() {
    return (uint8_t *)&codeSegmentSelector - (uint8_t *)this;
}

/**
 * Constructor for SegmentDescriptor.
 * 
 * Initializes a GDT segment descriptor with the specified base, limit and type.
 * 
 * @param base Base address of the segment.
 * @param limit Size of the segment.
 * @param type Segment type and attributes.
 */
GlobalDescriptorTable::SegmentDescriptor::SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t type) {
    uint8_t *target = (uint8_t *)this;

    // Adjust limit and granularity
    if (limit <= 65536) {
        target[6] = 0x40;  // 16-bit segment
    } else {
        if ((limit & 0xFFF) != 0xFFF)
            limit = (limit >> 12) - 1;
        else
            limit = limit >> 12;
        target[6] = 0xC0;  // 32-bit segment with 4 KB granularity
    }

    // Encode limit
    target[0] = limit & 0xFF;           // Lower 8 bits of limit
    target[1] = (limit >> 8) & 0xFF;    // Next 8 bits of limit
    target[6] |= (limit >> 16) & 0xF;   // Upper 4 bits of limit

    // Encode base
    target[2] = base & 0xFF;            // Lower 8 bits of base
    target[3] = (base >> 8) & 0xFF;     // Next 8 bits of base
    target[4] = (base >> 16) & 0xFF;    // Next 8 bits of base
    target[7] = (base >> 24) & 0xFF;    // Upper 8 bits of base

    // Set type and access attributes
    target[5] = type;
}

/**
 * Retrieves the base address of the segment.
 * 
 * @return 32-bit base address of the segment.
 */
uint32_t GlobalDescriptorTable::SegmentDescriptor::Base() {
    uint8_t *target = (uint8_t *)this;
    uint32_t result = target[7];
    result = (result << 8) + target[4];
    result = (result << 8) + target[3];
    result = (result << 8) + target[2];
    return result;
}

/**
 * Retrieves the limit of the segment.
 * 
 * @return 32-bit limit of the segment.
 */
uint32_t GlobalDescriptorTable::SegmentDescriptor::Limit() {
    uint8_t *target = (uint8_t *)this;
    uint32_t result = target[6] & 0xF;
    result = (result << 8) + target[1];
    result = (result << 8) + target[0];

    // Adjust based on granularity
    if ((target[6] & 0xC0) == 0xC0) {
        result = (result << 12) | 0xFFF;
    }

    return result;
}
