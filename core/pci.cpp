/**
 * @file        pci.cpp
 * @brief       Peripheral Component Interconnect for #x86
 * 
 * @date        15/01/2025
 * @version     1.0.0-beta
 */

#include <core/pci.h>
#include <console.h>

PeripheralComponentInterconnectDeviceDescriptor::PeripheralComponentInterconnectDeviceDescriptor() {
    this->portBase = 0;
    this->interrupt = 0;
    this->bus = 0;
    this->device = 0;
    this->function = 0;
    this->vendor_id = 0;
    this->device_id = 0;
    this->class_id = 0;
    this->subclass_id = 0;
    this->interface_id = 0;
    this->revision = 0;
}

PeripheralComponentInterconnectDeviceDescriptor::~PeripheralComponentInterconnectDeviceDescriptor() {}

PeripheralComponentInterconnectController::PeripheralComponentInterconnectController()
: dataPort(0xCFC), commandPort(0xCF8)
{
}

PeripheralComponentInterconnectController::~PeripheralComponentInterconnectController()
{
}

uint32_t PeripheralComponentInterconnectController::Read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset) {
    uint32_t id =
        0x80000000
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (registeroffset & 0xFC);
    commandPort.Write(id);
    uint32_t result = dataPort.Read();
    return result >> (8 * (registeroffset % 4));
}

void PeripheralComponentInterconnectController::Write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t value) {
    uint32_t id =
        0x80000000
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (registeroffset & 0xFC);
    commandPort.Write(id);
    dataPort.Write(value); 
}

bool PeripheralComponentInterconnectController::DeviceHasFunctions(uint16_t bus, uint16_t device) {
    return Read(bus, device, 0, 0x0E) & (1<<7);
}

PeripheralComponentInterconnectDeviceDescriptor* PeripheralComponentInterconnectController::GetDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function) {
    PeripheralComponentInterconnectDeviceDescriptor* result = new PeripheralComponentInterconnectDeviceDescriptor();
    result->bus = bus;
    result->device = device;
    result->function = function;
    
    result->vendor_id = Read(bus, device, function, 0x00);
    result->device_id = Read(bus, device, function, 0x02);
    result->class_id = Read(bus, device, function, 0x0b);
    result->subclass_id = Read(bus, device, function, 0x0a);
    result->interface_id = Read(bus, device, function, 0x09);
    result->revision = Read(bus, device, function, 0x08);
    result->interrupt = Read(bus, device, function, 0x3c);
    return result;
}

BaseAddressRegister PeripheralComponentInterconnectController::GetBaseAddressRegister(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar) {
    BaseAddressRegister result;
    result.address = 0; // Init
    
    uint32_t headertype = Read(bus, device, function, 0x0E) & 0x7F;
    int maxBARs = 6 - (4 * headertype);
    if (bar >= maxBARs) return result;
    
    uint32_t bar_value = Read(bus, device, function, 0x10 + 4 * bar);
    result.type = (bar_value & 0x1) ? InputOutput : MemoryMapping;
    
    if (result.type == MemoryMapping) {
        // Mask low 4 bits (flags) to get 16-byte aligned address
        result.address = (uint8_t*)(bar_value & 0xFFFFFFF0);
        result.prefetchable = ((bar_value >> 3) & 0x1);
    } else { 
        // InputOutput
        result.address = (uint8_t*)(bar_value & ~0x3);
        result.prefetchable = false;
    }
    return result;
}

// Helper to find specific hardware
PeripheralComponentInterconnectDeviceDescriptor* PeripheralComponentInterconnectController::FindHardwareDevice(uint16_t vendorID, uint16_t deviceID) {
    for (int bus = 0; bus < 8; bus++) {
        for (int device = 0; device < 32; device++) {
            int numFunctions = DeviceHasFunctions(bus, device) ? 8 : 1;
            for (int function = 0; function < numFunctions; function++) {
                PeripheralComponentInterconnectDeviceDescriptor* dev = GetDeviceDescriptor(bus, device, function);
                
                if (dev->vendor_id == 0x0000 || dev->vendor_id == 0xFFFF) continue;

                if (dev->vendor_id == vendorID && dev->device_id == deviceID) {
                    return dev;
                }
            }
        }
    }
    // Return empty descriptor if not found
    PeripheralComponentInterconnectDeviceDescriptor* empty = new PeripheralComponentInterconnectDeviceDescriptor();
    empty->vendor_id = 0;
    return empty;
}



extern "C" {
    uint32_t pci_find_bar0(uint16_t vendor, uint16_t device) {
        PeripheralComponentInterconnectController pci;
        PeripheralComponentInterconnectDeviceDescriptor* dev = pci.FindHardwareDevice(vendor, device);
        
        if (dev->vendor_id == 0) return 0;
        
        // Return BAR0 Address directly
        BaseAddressRegister bar = pci.GetBaseAddressRegister(dev->bus, dev->device, dev->function, 0);
        return (uint32_t)bar.address;
    }
    
    // Function to enable Bus Master (needed for VBox)
    void pci_enable_bus_master(uint16_t vendor, uint16_t device) {
        PeripheralComponentInterconnectController pci;
        PeripheralComponentInterconnectDeviceDescriptor* dev = pci.FindHardwareDevice(vendor, device);
        if (dev->vendor_id != 0) {
            uint32_t cmd = pci.Read(dev->bus, dev->device, dev->function, 0x04);
            if ((cmd & 0x07) != 0x07) {
                 pci.Write(dev->bus, dev->device, dev->function, 0x04, cmd | 0x07);
            }
        }
    }
}