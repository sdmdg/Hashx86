/**
 * @file        pci.cpp
 * @brief       Peripheral Component Interconnect for #x86
 * 
 * @author      Malaka Gunawardana
 * @date        13/01/2025
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
                    break; // Invalid device, skip
                }

                // Match vendor and device IDs with the table
                const PCIDevice* knownDevice = FindPCIDevice(dev.vendor_id, dev.device_id);

                // Print PCI device information
                printf(WHITE, "PCI BUS 0x%x, DEVICE 0x%x, FUNCTION 0x%x = ", 
                       (bus & 0xFF), (device & 0xFF), (function & 0xFF));
                printf(WHITE, "VENDOR 0x%x, DEVICE 0x%x\n", dev.vendor_id, dev.device_id);
                
                if (knownDevice) {
                    printf(combineColors(BLACK, GREEN), " %s - %s\n", knownDevice->vendorName, knownDevice->deviceName);
                } else {
                    printf(combineColors(BLACK, RED), " Unknown Device\n");
                }

            }
        }
    }
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