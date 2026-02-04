/**
 * @file        bga.cpp
 * @brief       BGA Graphics Driver Implementation
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <core/driver.h>
#include <core/drivers/GraphicsDriver.h>
#include <core/drivers/driver_info.h>
#include <core/pci.h>
#include <gui/config/config.h>

// --- BGA Constants ---
#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA 0x01CF
#define VBE_DISPI_INDEX_ID 0
#define VBE_DISPI_INDEX_XRES 1
#define VBE_DISPI_INDEX_YRES 2
#define VBE_DISPI_INDEX_BPP 3
#define VBE_DISPI_INDEX_ENABLE 4
#define VBE_DISPI_INDEX_BANK 5
#define VBE_DISPI_INDEX_VIRT_WIDTH 6

#define VBE_DISPI_INDEX_X_OFFSET 8
#define VBE_DISPI_INDEX_Y_OFFSET 9

#define VBE_DISPI_DISABLED 0x00
#define VBE_DISPI_ENABLED 0x01
#define VBE_DISPI_LFB_ENABLED 0x40

DEFINE_DRIVER_INFO("BGA Driver for Hashx86", "0.1.0", {0x1234, 0x1111},  // QEMU / Bochs
                   {0x80EE, 0xBEEF},                                     // VirtualBox
                   {0x15AD, 0x0405}                                      // VMware / VBox SVGA
);

class DynamicBGADriver : public Driver, public GraphicsDriver {
private:
    uint32_t physFramebufferAddr;

    void WriteRegister(uint16_t index, uint16_t value) {
        outw(VBE_DISPI_IOPORT_INDEX, index);
        outw(VBE_DISPI_IOPORT_DATA, value);
    }

    uint16_t ReadRegister(uint16_t index) {
        outw(VBE_DISPI_IOPORT_INDEX, index);
        return inw(VBE_DISPI_IOPORT_DATA);
    }

    // Returns Physical Address of LFB, or 0 if failed
    uint32_t FindFramebufferPCI() {
        PeripheralComponentInterconnectController pci;
        PeripheralComponentInterconnectDeviceDescriptor* dev;

        dev = pci.FindHardwareDevice(0x1234, 0x1111);
        if (dev->vendor_id == 0) dev = pci.FindHardwareDevice(0x80EE, 0xBEEF);
        if (dev->vendor_id == 0) dev = pci.FindHardwareDevice(0x15AD, 0x0405);
        if (dev->vendor_id == 0) return 0;

        // Enable Bus Master
        uint32_t pci_cmd = pci.Read(dev->bus, dev->device, dev->function, 0x04);
        pci.Write(dev->bus, dev->device, dev->function, 0x04, pci_cmd | 0x07);

        // Scan BARs
        for (int i = 0; i < 6; i++) {
            BaseAddressRegister bar =
                pci.GetBaseAddressRegister(dev->bus, dev->device, dev->function, i);

            if (bar.type == MemoryMapping && bar.address != 0) {
                // PCI Spec: Lower 4 bits are flags (prefetchable, type, etc.)
                return (uint32_t)((uint32_t)bar.address & (uint32_t)0xFFFFFFF0);
            }
        }

        return 0xE0000000;  // Fallback
    }

public:
    DynamicBGADriver() : GraphicsDriver(GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT, GUI_SCREEN_BPP, 0) {
        this->driverName = "BGA Driver for Hashx86";
        this->physFramebufferAddr = 0;
    }

    void Activate() override {
        // Find the Hardware
        this->physFramebufferAddr = FindFramebufferPCI();

        if (this->physFramebufferAddr == 0) {
            printf("[BGA] Error: No compatible Graphics Card found via PCI.\n");
            return;
        }

        printf("[BGA] Hardware Found. LFB @ 0x%x\n", this->physFramebufferAddr);

        // Set Video Mode
        WriteRegister(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
        WriteRegister(VBE_DISPI_INDEX_ID, 0xB0C5);  // VBE 3.0
        WriteRegister(VBE_DISPI_INDEX_X_OFFSET, 0);
        WriteRegister(VBE_DISPI_INDEX_Y_OFFSET, 0);
        WriteRegister(VBE_DISPI_INDEX_XRES, this->width);
        WriteRegister(VBE_DISPI_INDEX_YRES, this->height);
        WriteRegister(VBE_DISPI_INDEX_BPP, 32);
        WriteRegister(VBE_DISPI_INDEX_VIRT_WIDTH, this->width);
        WriteRegister(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);

        uint16_t bpp_result = ReadRegister(VBE_DISPI_INDEX_BPP);
        if (bpp_result != 32) {
            printf("[BGA] Warning: Hardware refused 32-bit mode! Got: %d\n", bpp_result);
        }
        // Update GraphicsDriver Pointers
        this->videoMemory = (uint32_t*)this->physFramebufferAddr;

        printf("[BGA] Mode Set: %dx%d\n", this->width, this->height);
        this->is_Active = true;
    }

    void Deactivate() override {
        WriteRegister(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    }

    int Reset() override {
        Activate();
        return 0;
    }

    // Helper for the kernel to know where to map pages
    uint32_t GetPhysicalAddress() {
        return physFramebufferAddr;
    }

    GraphicsDriver* AsGraphicsDriver() override {
        return this;
    }
};

extern "C" Driver* CreateDriverInstance() {
    return new DynamicBGADriver();
}
