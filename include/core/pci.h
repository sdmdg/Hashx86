
#ifndef PCI_H
#define PCI_H

#include <core/driver.h>
#include <core/interrupts.h>
#include <core/ports.h>
#include <debug.h>
#include <types.h>

// Temporary
struct PCIDevice {
    uint16_t vendorID;
    uint16_t deviceID;
    const char* vendorName;
    const char* deviceName;
};

static const PCIDevice pciDevices[] = {
    {0x10DE, 0x1C82, "NVIDIA Corporation", "GeForce GTX 1050"},
    {0x1022, 0x1481, "AMD", "Ryzen Controller"},
    {0x10EC, 0x8168, "Realtek Semiconductor", "RTL8111/8168/8411 Ethernet Controller"},
    {0x106B, 0x003F, "Apple Inc.", "KeyLargo/Intrepid USB"},

    {0x8086, 0x1234, "Intel Corporation", "Sample Device A"},
    {0x8086, 0x29C0, "Intel Corporation", "PCI Express Root Port"},
    {0x8086, 0x3B64, "Intel Corporation", "Lynx Point USB xHCI Host Controller"},
    {0x8086, 0x9D03, "Intel Corporation", "HD Audio Controller"},
    {0x8086, 0x1237, "Intel Corporation", "440FX - 82441FX PMC [Natoma]"},
    {0x8086, 0x7000, "Intel Corporation", "82371SB PIIX3 ISA [Natoma/Triton II]"},
    {0x8086, 0x7010, "Intel Corporation", "82371SB PIIX3 IDE [Natoma/Triton II]"},
    {0x8086, 0x7111, "Intel Corporation", "82371AB/EB/MB PIIX4 IDE"},
    {0x8086, 0x2415, "Intel Corporation", "82801AA AC'97 Audio Controller"},
    {0x8086, 0x7113, "Intel Corporation", "82371AB/EB/MB PIIX4 ACPI"},
    {0x8086, 0x265C, "Intel Corporation",
     "82801FB/FBM/FR/FW/FRW (ICH6 Family) USB2 EHCI Controller"},
    {0x8086, 0x2829, "Intel Corporation",
     "82801HM/HEM (ICH8M/ICH8M-E) SATA Controller [AHCI mode]"},
    {0x8086, 0x100E, "Intel Corporation", "82540EM Gigabit Ethernet Controller"},

    {0x1274, 0x5000, "Ensoniq", "ES1370 AudioPCI"},
    {0x1274, 0x1371, "Ensoniq", "ES1371 AudioPCI-97"},

    {0x15AD, 0x0405, "VMware", "SVGA II Adapter"},
    {0x80EE, 0xCAFE, "InnoTek Systemberatung GmbH", "VirtualBox Guest Service"},

    {0x1AF4, 0x1000, "Red Hat, Inc.", "Virtio Network Device"},
    {0x1D0F, 0xEC20, "Amazon.com, Inc.", "Elastic Network Adapter"},
    {0x1B36, 0x000D, "QEMU", "QEMU PCIe Host Bridge"},
    {0x1234, 0x1111, "Bochs", "Bochs VGA Device"},
    {0x1A03, 0x1150, "ASPEED Technology", "Graphics Family"},
};

// Device Descriptor (Location on Bus)
class PeripheralComponentInterconnectDeviceDescriptor {
public:
    uint32_t portBase;
    uint32_t interrupt;

    uint16_t bus;
    uint16_t device;
    uint16_t function;

    uint16_t vendor_id;
    uint16_t device_id;

    uint8_t class_id;
    uint8_t subclass_id;
    uint8_t interface_id;

    uint8_t revision;

    PeripheralComponentInterconnectDeviceDescriptor();
    ~PeripheralComponentInterconnectDeviceDescriptor();
};

enum BaseAddressRegisterType { MemoryMapping = 0, InputOutput = 1 };

class BaseAddressRegister {
public:
    bool prefetchable;
    uint8_t* address;
    uint32_t size;
    BaseAddressRegisterType type;
};

class PeripheralComponentInterconnectController {
    Port32Bit dataPort;
    Port32Bit commandPort;

public:
    PeripheralComponentInterconnectController();
    ~PeripheralComponentInterconnectController();

    uint32_t Read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset);
    void Write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset,
               uint32_t value);
    bool DeviceHasFunctions(uint16_t bus, uint16_t device);

    // void SelectDrivers(DriverManager* driverManager, InterruptManager* interrupts); // Commented
    // out dependencies for now

    PeripheralComponentInterconnectDeviceDescriptor* GetDeviceDescriptor(uint16_t bus,
                                                                         uint16_t device,
                                                                         uint16_t function);
    BaseAddressRegister GetBaseAddressRegister(uint16_t bus, uint16_t device, uint16_t function,
                                               uint16_t bar);

    // Scans specifically for one hardware ID
    PeripheralComponentInterconnectDeviceDescriptor* FindHardwareDevice(uint16_t vendorID,
                                                                        uint16_t deviceID);
};

#endif
