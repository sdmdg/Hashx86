/**
 * @file        ac97.cpp
 * @brief       AC97 Audio Driver Implementation
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <core/driver.h>
#include <core/drivers/AudioDriver.h>
#include <core/drivers/driver_info.h>
#include <core/interrupts.h>
#include <core/memory.h>
#include <core/pci.h>
#include <debug.h>
#include <utils/string.h>

/* ================= IDs ================= */
#define AC97_VENDOR_ID 0x8086
#define AC97_DEVICE_ID 0x2415

/* ================= Mixer ================= */
#define AC97_REG_RESET 0x00
#define AC97_REG_MASTER_VOL 0x02
#define AC97_REG_PCM_VOL 0x18
#define AC97_REG_EXT_AUDIO 0x28
#define AC97_REG_EXT_CTRL 0x2A
#define AC97_REG_PCM_RATE 0x2C

/* ================= Bus Master ================= */
#define AC97_PO_BDBAR 0x10
#define AC97_PO_CIV 0x14
#define AC97_PO_LVI 0x15
#define AC97_PO_SR 0x16
#define AC97_PO_CR 0x1B

#define AC97_CR_RUN 0x01
#define AC97_CR_RESET 0x02
#define AC97_CR_IOCE 0x10

#define AC97_SR_DCH 0x01
#define AC97_SR_BCIS 0x08
#define AC97_SR_LVBCI 0x20

/* ================= Memory ================= */
#define AC97_PHYS_BUF 0x01000000
#define AC97_PHYS_BDL 0x01010000
#define AC97_TOTAL_SIZE 0x10000  // 64KB total RAM
#define AC97_HALF_SIZE (AC97_TOTAL_SIZE / 2)
#define AC97_BDL_ENTRIES 32  // Use full 32 entries

struct AC97_BDL_Entry {
    uint32_t addr;
    uint16_t length;  // words
    uint16_t flags;
} __attribute__((packed));

DEFINE_DRIVER_INFO("Intel AC97 Audio Driver", "2.2.0-MovingLVI", {AC97_VENDOR_ID, AC97_DEVICE_ID});

class DynamicAC97Driver;

/* ================= IRQ ================= */
class AC97IRQ : public InterruptHandler {
    DynamicAC97Driver* driver;

public:
    AC97IRQ(uint8_t irq, DynamicAC97Driver* drv)
        : InterruptHandler(irq, InterruptManager::activeInstance), driver(drv) {}
    uint32_t HandleInterrupt(uint32_t esp) override;
};

/* ================= DRIVER ================= */
class DynamicAC97Driver final : public Driver, public AudioDriver {
    friend class AC97IRQ;

private:
    uint16_t namBar;
    uint16_t nabmBar;
    AC97IRQ* irqHandler;

    // --- State ---
    // sw_lvi: The index we are currently preparing to write to (Software Pointer)
    volatile uint8_t sw_lvi;

    // buffersOccupied: How many buffers are queued but not yet finished by HW.
    // If 0, we can write. If 2, we are full (waiting for HW).
    volatile uint8_t buffersOccupied;

    void Delay(int ms) {
        for (volatile int i = 0; i < ms * 10000; i++);
    }

    bool FindHardware() {
        PeripheralComponentInterconnectController pci;
        auto* dev = pci.FindHardwareDevice(AC97_VENDOR_ID, AC97_DEVICE_ID);
        if (!dev || dev->vendor_id == 0) return false;

        uint32_t cmd = pci.Read(dev->bus, dev->device, dev->function, 0x04);
        pci.Write(dev->bus, dev->device, dev->function, 0x04, cmd | 0x07);

        namBar =
            (uint16_t)((uint32_t)pci.GetBaseAddressRegister(dev->bus, dev->device, dev->function, 0)
                           .address &
                       0xFFFC);
        nabmBar =
            (uint16_t)((uint32_t)pci.GetBaseAddressRegister(dev->bus, dev->device, dev->function, 1)
                           .address &
                       0xFFFC);

        irqHandler = new AC97IRQ(dev->interrupt + 0x20, this);
        if (!irqHandler) {
            HALT("CRITICAL: [AC97] Failed to allocate AC97 IRQ handler!\n");
        }
        printf("[AC97] Found device IRQ=%d\n", dev->interrupt);
        return true;
    }

    void OnInterrupt() {
        uint16_t sr = inw(nabmBar + AC97_PO_SR);

        if ((sr & AC97_SR_BCIS) || (sr & AC97_SR_LVBCI)) {
            // ACK interrupt
            outw(nabmBar + AC97_PO_SR, sr & (AC97_SR_BCIS | AC97_SR_LVBCI));

            // A buffer finished!
            // This means we have space in our logical queue.
            if (buffersOccupied > 0) {
                buffersOccupied--;
            }
        }
    }

public:
    DynamicAC97Driver() {
        driverName = "Intel AC97";
        namBar = nabmBar = 0;
        irqHandler = nullptr;
        sw_lvi = 0;
        buffersOccupied = 0;
    }

    ~DynamicAC97Driver() {
        if (irqHandler) delete irqHandler;
    }

    void Activate() override {
        if (!FindHardware()) return;

        // 1. Reset
        outw(namBar + AC97_REG_RESET, 0);
        Delay(50);
        outw(namBar + AC97_REG_MASTER_VOL, 0);
        outw(namBar + AC97_REG_PCM_VOL, 0);

        // 2. Enable VRA
        if (inw(namBar + AC97_REG_EXT_AUDIO) & 1) {
            outw(namBar + AC97_REG_EXT_CTRL, 1);
            Delay(10);
            outw(namBar + AC97_REG_PCM_RATE, 44100);
        }

        // 3. Reset Bus Master
        outb(nabmBar + AC97_PO_CR, AC97_CR_RESET);
        Delay(10);
        outb(nabmBar + AC97_PO_CR, 0);

        // 4. Setup BDL Pointer
        outl(nabmBar + AC97_PO_BDBAR, AC97_PHYS_BDL);

        // Clear RAM
        memset((void*)AC97_PHYS_BUF, 0, AC97_TOTAL_SIZE);
        memset((void*)AC97_PHYS_BDL, 0, sizeof(AC97_BDL_Entry) * AC97_BDL_ENTRIES);

        // 5. Initialize State
        sw_lvi = 0;           // Start at index 0
        buffersOccupied = 0;  // Empty

        // Reset HW LVI to 0 to start
        outb(nabmBar + AC97_PO_LVI, 0);

        is_Active = true;
        printf("[AC97] Ready (Moving LVI Mode)\n");
    }

    void Deactivate() override {
        Stop();
    }

    uint32_t GetBufferSize() override {
        return AC97_HALF_SIZE;
    }

    void SetFormat(uint32_t rate, uint8_t, uint8_t) override {
        outw(namBar + AC97_REG_PCM_RATE, (uint16_t)rate);
        sampleRate = rate;
    }

    void ApplyHardwareVolume() override {
        uint8_t att = 63 - ((masterVolume * 63) / 100);
        uint16_t vol = (att << 8) | att;
        outw(namBar + AC97_REG_MASTER_VOL, vol);
        outw(namBar + AC97_REG_PCM_VOL, vol);
    }

    void Start() override {
        // Run and Enable Interrupts
        outb(nabmBar + AC97_PO_CR, AC97_CR_RUN | AC97_CR_IOCE);
        isPlaying = true;
    }

    void Stop() override {
        outb(nabmBar + AC97_PO_CR, 0);
        isPlaying = false;
    }

    bool IsReadyForData() override {
        // We can buffer up to 2 frames ahead.
        return buffersOccupied < 2;
    }

    uint32_t WriteData(uint8_t* buffer, uint32_t size) override {
        if (size > AC97_HALF_SIZE) size = AC97_HALF_SIZE;
        if (buffersOccupied >= 2) return 0;

        // 1. Determine which Physical RAM chunk to use (Ping-Pong)
        // If sw_lvi is Even (0, 2, 4...) -> use Buffer 0
        // If sw_lvi is Odd  (1, 3, 5...) -> use Buffer 1
        uint32_t physAddr = (sw_lvi % 2 == 0) ? AC97_PHYS_BUF : (AC97_PHYS_BUF + AC97_HALF_SIZE);

        // 2. Copy Data to RAM
        memcpy((void*)physAddr, buffer, size);
        asm volatile("wbinvd" ::: "memory");  // Flush cache

        // 3. Setup the BDL Entry for this specific slot
        AC97_BDL_Entry* bdl = (AC97_BDL_Entry*)AC97_PHYS_BDL;

        bdl[sw_lvi].addr = physAddr;
        bdl[sw_lvi].length = (uint16_t)(size / 2);  // Length in words
        bdl[sw_lvi].flags = 0x8000;                 // IOC (Interrupt on Completion)

        // 4. "Push" the LVI (Last Valid Index)
        // This tells hardware: "You can proceed up to this index"
        outb(nabmBar + AC97_PO_LVI, sw_lvi);

        // 5. Advance our software pointer (Circular 0-31)
        sw_lvi++;
        if (sw_lvi >= AC97_BDL_ENTRIES) {
            sw_lvi = 0;
        }

        buffersOccupied++;
        return size;
    }

    AudioDriver* AsAudioDriver() override {
        return this;
    }
};

uint32_t AC97IRQ::HandleInterrupt(uint32_t esp) {
    if (driver) driver->OnInterrupt();
    return esp;
}

extern "C" Driver* CreateDriverInstance() {
    DynamicAC97Driver* drv = new DynamicAC97Driver();
    if (!drv) {
        HALT("CRITICAL: [AC97] Failed to allocate DynamicAC97Driver!\n");
    }
    return drv;
}
