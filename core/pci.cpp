/**
 * @file        pci.cpp
 * @brief       Peripheral Component Interconnect for #x86
 * 
 * @date        15/01/2025
 * @version     1.0.0-beta
 */

#include <core/pci.h>

PeripheralComponentInterconnectDeviceDescriptor::PeripheralComponentInterconnectDeviceDescriptor(){

}
PeripheralComponentInterconnectDeviceDescriptor::~PeripheralComponentInterconnectDeviceDescriptor(){
    
}

const PCIDevice* FindPCIDevice(uint16_t vendorID, uint16_t deviceID) {
    for (size_t i = 0; i < sizeof(pciDevices) / sizeof(pciDevices[0]); ++i) {
        if (pciDevices[i].vendorID == vendorID && pciDevices[i].deviceID == deviceID) {
            return &pciDevices[i];
        }
    }
    return nullptr; // Not found
}



PeripheralComponentInterconnectController::PeripheralComponentInterconnectController()
:dataPort(0xCFC),
 commandPort(0xCF8)
{

};

PeripheralComponentInterconnectController::~PeripheralComponentInterconnectController()
{

};

uint32_t PeripheralComponentInterconnectController::Read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset){
    uint32_t id =
        0x1 << 31
            | ((bus & 0xFF) << 16)
            | ((device & 0x1F) << 11)
            | ((function & 0x07) << 8)
            | (registeroffset & 0xFC);
    commandPort.Write(id);
    uint32_t result = dataPort.Read();
    return result >> (8* (registeroffset % 4));
};
void PeripheralComponentInterconnectController::Write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t value){
    uint32_t id =
        0x1 << 31
            | ((bus & 0xFF) << 16)
            | ((device & 0x1F) << 11)
            | ((function & 0x07) << 8)
            | (registeroffset & 0xFC);
    commandPort.Write(id);
    dataPort.Write(value); 
};

bool PeripheralComponentInterconnectController::DeviceHasFunctions(uint16_t bus, uint16_t device){
    return Read(bus, device, 0, 0x0E) & (1<<7);
};

void PeripheralComponentInterconnectController::SelectDrivers(DriverManager* driverManager, InterruptManager* interrupts) {
    for (int bus = 0; bus < 8; bus++) {
        for (int device = 0; device < 32; device++) {
            int numFunctions = DeviceHasFunctions(bus, device) ? 8 : 1;
            for (int function = 0; function < numFunctions; function++) {
                PeripheralComponentInterconnectDeviceDescriptor dev = GetDeviceDescriptor(bus, device, function);

                if (dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF) {
                    continue;
                }

                for(int barNum = 0; barNum < 6; barNum++)
                {
                    BaseAddressRegister bar = GetBaseAddressRegister(bus, device, function, barNum);
                    if(bar.address && (bar.type == InputOutput))
                        dev.portBase = (uint32_t)bar.address;
                }

                Driver* driver = GetDriver(dev, interrupts);
                if(driver != 0)
                    driverManager->AddDriver(driver);

                // Match vendor and device IDs with the table
                const PCIDevice* knownDevice = FindPCIDevice(dev.vendor_id, dev.device_id);

                // Print PCI device information
                printf("PCI BUS 0x%x, DEVICE 0x%x, FUNCTION 0x%x = ", 
                       (bus & 0xFF), (device & 0xFF), (function & 0xFF));
                printf("VENDOR 0x%x, DEVICE 0x%x\n", dev.vendor_id, dev.device_id);
                
                if (knownDevice) {
                    printf(" %s - %s\n", knownDevice->vendorName, knownDevice->deviceName);
                } else {
                    printf(" Unknown Device\n");
                }

            }
        }
    }
}

BaseAddressRegister PeripheralComponentInterconnectController::GetBaseAddressRegister(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar)
{
    BaseAddressRegister result;
    
    
    uint32_t headertype = Read(bus, device, function, 0x0E) & 0x7F;
    int maxBARs = 6 - (4*headertype);
    if(bar >= maxBARs)
        return result;
    
    
    uint32_t bar_value = Read(bus, device, function, 0x10 + 4*bar);
    result.type = (bar_value & 0x1) ? InputOutput : MemoryMapping;
    uint32_t temp;
    
    
    
    if(result.type == MemoryMapping)
    {
        
        switch((bar_value >> 1) & 0x3)
        {
            
            case 0: // 32 Bit Mode
            case 1: // 20 Bit Mode
            case 2: // 64 Bit Mode
                break;
        }
        
    }
    else // InputOutput
    {
        result.address = (uint8_t*)(bar_value & ~0x3);
        result.prefetchable = false;
    }

    return result;
}



Driver* PeripheralComponentInterconnectController::GetDriver(PeripheralComponentInterconnectDeviceDescriptor dev, InterruptManager* interrupts)
{
    Driver* driver = 0;
    switch(dev.vendor_id)
    {
        case 0x1022: // AMD
        case 0x1234: // Bochs
        case 0x1B36: // QEMU
        case 0x8086: // Intel
            break;
    }
    
    
    switch(dev.class_id)
    {
        case 0x03: // graphics
            switch(dev.subclass_id)
            {
                case 0x00: // VGA
                    printf("VGA\n");
                    break;
            }
            break;
    }
    
    
    return driver;
}

PeripheralComponentInterconnectDeviceDescriptor PeripheralComponentInterconnectController::GetDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function){
    PeripheralComponentInterconnectDeviceDescriptor result;
    
    result.bus = bus;
    result.device = device;
    result.function = function;
    
    result.vendor_id = Read(bus, device, function, 0x00);
    result.device_id = Read(bus, device, function, 0x02);

    result.class_id = Read(bus, device, function, 0x0b);
    result.subclass_id = Read(bus, device, function, 0x0a);
    result.interface_id = Read(bus, device, function, 0x09);

    result.revision = Read(bus, device, function, 0x08);
    result.interrupt = Read(bus, device, function, 0x3c);
    
    return result;
}