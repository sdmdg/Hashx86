#ifndef GDT_H
#define GDT_H

#include <types.h>

/**
 * GlobalDescriptorTable
 * 
 * This class manages the GDT. The GDT is responsible for defining memory segments and 
 * their attributes, such as base address, limit and access permissions.
 */
class GlobalDescriptorTable {
public:
    /**
     * SegmentDescriptor
     * 
     * Represents a single entry in the GDT. Each entry specifies a memory
     * segment's base address, size and access permissions.
     */
    class SegmentDescriptor {
    private:
        uint16_t limit_lo;    // Lower 16 bits of the segment limit
        uint16_t base_lo;     // Lower 16 bits of the segment base address
        uint8_t base_hi;      // Middle 8 bits of the base address
        uint8_t type;         // Segment type and attributes
        uint8_t limit_hi;     // Upper 4 bits of the segment limit and flags
        uint8_t base_vhi;     // Upper 8 bits of the base address

    public:
        SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t type);
        uint32_t Base();        // Retrieves the segment base address
        uint32_t Limit();       // Retrieves the segment limit
    } __attribute__((packed));  // Ensures the structure layout matches hardware expectations

private:
    SegmentDescriptor nullSegmentSelector;   // Null descriptor (must be the first entry)
    SegmentDescriptor unusedSegmentSelector; // Reserved (not used)
    SegmentDescriptor codeSegmentSelector;   // Code segment descriptor
    SegmentDescriptor dataSegmentSelector;   // Data segment descriptor

public:
    GlobalDescriptorTable();
    ~GlobalDescriptorTable();
    uint16_t CodeSegmentSelector();          // Offset for the code segment
    uint16_t DataSegmentSelector();          // Offset for the data segment
};

#endif // GDT_H
