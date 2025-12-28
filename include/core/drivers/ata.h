 
#ifndef ATA_H
#define ATA_H

#include <types.h>
#include <core/interrupts.h>
#include <core/ports.h>
#include <debug.h>

class AdvancedTechnologyAttachment
{
private:
    uint32_t ata_size;
protected:
    bool master;
    Port16Bit dataPort;
    Port8Bit errorPort;
    Port8Bit sectorCountPort;
    Port8Bit lbaLowPort;
    Port8Bit lbaMidPort;
    Port8Bit lbaHiPort;
    Port8Bit devicePort;
    Port8Bit commandPort;
    Port8Bit controlPort;
public:
    AdvancedTechnologyAttachment(bool master, uint16_t portBase);
    ~AdvancedTechnologyAttachment();

    uint32_t Identify();
    void Read28(uint32_t sectorNum, uint8_t* data, int count = 512);

    void Write28(uint32_t sectorNum, uint8_t* data, uint32_t count);
    void Flush();

    uint32_t GetSizeInSectors() { return ata_size; }
};


#endif // ATA_H