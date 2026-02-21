/**
 * @file        kernel.cpp
 * @brief       Main Kernel of #x86
 *
 * @date        24/02/2025
 * @version     1.2.0-beta
 */

#define KDBG_COMPONENT "KERNEL"
#include <kernel.h>

#define DEBUG_ENABLED TRUE;
#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40

KERNEL_MEMORY_MAP g_kmap;

extern "C" uint32_t pci_find_bar0(uint16_t vendor, uint16_t device);
extern "C" void __cxa_pure_virtual() {
    HALT("Pure Virtual Function Called! System Halted.");
}

int get_kernel_memory_map(KERNEL_MEMORY_MAP* kmap, MultibootInfo* mboot_info) {
    if (kmap == NULL) return -1;
    kmap->kernel.k_start_addr = (uint32_t)&__kernel_section_start;
    kmap->kernel.k_end_addr = (uint32_t)&__kernel_section_end;
    kmap->kernel.k_len = ((uint32_t)&__kernel_section_end - (uint32_t)&__kernel_section_start);

    kmap->kernel.text_start_addr = (uint32_t)&__kernel_text_section_start;
    kmap->kernel.text_end_addr = (uint32_t)&__kernel_text_section_end;
    kmap->kernel.text_len =
        ((uint32_t)&__kernel_text_section_end - (uint32_t)&__kernel_text_section_start);

    kmap->kernel.data_start_addr = (uint32_t)&__kernel_data_section_start;
    kmap->kernel.data_end_addr = (uint32_t)&__kernel_data_section_end;
    kmap->kernel.data_len =
        ((uint32_t)&__kernel_data_section_end - (uint32_t)&__kernel_data_section_start);

    kmap->kernel.rodata_start_addr = (uint32_t)&__kernel_rodata_section_start;
    kmap->kernel.rodata_end_addr = (uint32_t)&__kernel_rodata_section_end;
    kmap->kernel.rodata_len =
        ((uint32_t)&__kernel_rodata_section_end - (uint32_t)&__kernel_rodata_section_start);

    kmap->kernel.bss_start_addr = (uint32_t)&__kernel_bss_section_start;
    kmap->kernel.bss_end_addr = (uint32_t)&__kernel_bss_section_end;
    kmap->kernel.bss_len =
        ((uint32_t)&__kernel_bss_section_end - (uint32_t)&__kernel_bss_section_start);

    kmap->system.total_memory = mboot_info->mem_lower + mboot_info->mem_upper;

    for (int i = 0; i < mboot_info->mmap_length; i += sizeof(MULTIBOOT_MEMORY_MAP)) {
        MULTIBOOT_MEMORY_MAP* mmap = (MULTIBOOT_MEMORY_MAP*)(mboot_info->mmap_addr + i);
        if (mmap->type != MULTIBOOT_MEMORY_AVAILABLE) continue;
        // make sure kernel is loaded at 0x100000 by bootloader(see linker.ld)
        if (mmap->addr_low == kmap->kernel.text_start_addr) {
            // set available memory starting from end of our kernel, leaving 1MB size for functions
            // exceution
            kmap->available.start_addr = kmap->kernel.k_end_addr + 1024 * 1024;
            kmap->available.end_addr = mmap->addr_low + mmap->len_low;
            // get availabel memory in bytes
            kmap->available.size = kmap->available.end_addr - kmap->available.start_addr;
            return 0;
        }
    }

    return -1;
}

void display_kernel_memory_map(KERNEL_MEMORY_MAP* kmap) {
    KDBG2("MemoryMap section=KERNEL start=0x%x end=0x%x len=%d", kmap->kernel.k_start_addr,
          kmap->kernel.k_end_addr, kmap->kernel.k_len);
    KDBG2("MemoryMap section=TEXT   start=0x%x end=0x%x len=%d", kmap->kernel.text_start_addr,
          kmap->kernel.text_end_addr, kmap->kernel.text_len);
    KDBG2("MemoryMap section=DATA   start=0x%x end=0x%x len=%d", kmap->kernel.data_start_addr,
          kmap->kernel.data_end_addr, kmap->kernel.data_len);
    KDBG2("MemoryMap section=RODATA start=0x%x end=0x%x len=%d", kmap->kernel.rodata_start_addr,
          kmap->kernel.rodata_end_addr, kmap->kernel.rodata_len);
    KDBG2("MemoryMap section=BSS    start=0x%x end=0x%x len=%d", kmap->kernel.bss_start_addr,
          kmap->kernel.bss_end_addr, kmap->kernel.bss_len);

    KDBG1("SystemMemory total=%d KB available_size=%d", kmap->system.total_memory,
          kmap->available.size);
    KDBG1("AvailableRAM start=0x%x end=0x%x", kmap->available.start_addr, kmap->available.end_addr);
}

void init_memory(MultibootInfo* mbinfo) {
    memset(&g_kmap, 0, sizeof(KERNEL_MEMORY_MAP));
    get_kernel_memory_map(&g_kmap, mbinfo);

    // Initialize PMM at end of Kernel (Respect BSS/Stack)
    // Use bss_end_addr to ensure we are past the stack
    uint32_t heap_start_addr = g_kmap.kernel.bss_end_addr;

    // Safety Fallback: Use k_end_addr if bss calc seems wrong
    if (g_kmap.kernel.k_end_addr > heap_start_addr) {
        heap_start_addr = g_kmap.kernel.k_end_addr;
    }

    if (heap_start_addr < 0x200000) {
        heap_start_addr = 0x200000;
    }

    // Add 4MB padding to be safe.
    heap_start_addr += 4 * 1024 * 1024;

    struct multiboot_module* modules = (struct multiboot_module*)mbinfo->mods_addr;

    if (mbinfo->mods_count > 0) {
        for (int mod_idx = 0; mod_idx < mbinfo->mods_count; mod_idx++) {
            uint32_t mod_end = modules[mod_idx].mod_end;
            if (mod_end > heap_start_addr) {
                heap_start_addr = mod_end;
            }
        }
    }

    // Align to Page Boundary
    if ((heap_start_addr & 0xFFF) != 0) {
        heap_start_addr = (heap_start_addr & 0xFFFFF000) + 0x1000;
    }

    // initialize PMM
    pmm_init(heap_start_addr, g_kmap.available.end_addr);
    // Calculate how big the bitmap
    uint32_t bitmap_size = (g_kmap.available.end_addr / PMM_BLOCK_SIZE) / 8;
    // Mark FREE Region, but SKIP the bitmap!
    pmm_init_region(heap_start_addr + bitmap_size, g_kmap.available.end_addr);

    // HEAP ALLOCATION
    uint32_t actual_heap_start = heap_start_addr + bitmap_size;
    if ((actual_heap_start & 0xFFF) != 0) {
        actual_heap_start = (actual_heap_start & 0xFFFFF000) + 0x1000;
    }

    // Define the Hard Ceiling (224 MB)
    // Reserve 32MB (224-256MB) of identity-mapped space for PMM allocations
    // (page tables, user stacks, process directories, ELF loading pages).
    // These MUST stay below 256MB because the kernel accesses them via
    // identity-mapped physical addresses after paging is activated.
    uint32_t paging_limit = 224 * 1024 * 1024;
    // Define the Physical Ceiling
    uint32_t physical_limit = g_kmap.available.end_addr;

    // Determine the Safe Limit
    uint32_t safe_limit = (paging_limit < physical_limit) ? paging_limit : physical_limit;

    // Apply a Safety Buffer
    safe_limit -= 4096;

    // Calculate Exact Size Needed
    if (safe_limit <= actual_heap_start) {
        HALT("CRITICAL: No memory left for Heap! (Kernel + PMM > Limit)\n");
    }

    uint32_t heap_size_bytes = safe_limit - actual_heap_start;
    uint32_t blocks_needed = heap_size_bytes / PMM_BLOCK_SIZE;

    KDBG1("Maximizing PMM Heap: Start=0x%x Limit=0x%x Size=%d MB", actual_heap_start, safe_limit,
          (int32_t)(heap_size_bytes / (1024 * 1024)));

    // Allocate
    void* heap_start = pmm_alloc_blocks(blocks_needed);

    if (heap_start == NULL) {
        HALT("CRITICAL: Failed to allocate calculated heap!\n");
    }

    // Force Heap Alignment (Crucial for Paging)
    uint32_t heap_val = (uint32_t)heap_start;
    if ((heap_val & 0xFFF) != 0) {
        heap_val = (heap_val + 0xFFF) & ~0xFFF;
        blocks_needed--;
    }
    heap_start = (void*)heap_val;

    size_t heap_size = blocks_needed * PMM_BLOCK_SIZE;
    void* heap_end = (void*)((uint32_t)heap_start + heap_size);

    KDBG1("Kernel Heap: 0x%x - 0x%x (%d MB)", heap_start, heap_end, heap_size / 1024 / 1024);

    kheap_init(heap_start, heap_end);
}

void InitializePIT(uint32_t frequency) {
    // The PIT's input clock is approximately 1.193182 MHz
    uint32_t divisor = 1193180 / frequency;

    // Send the command byte.
    // 0x36 = 00 11 01 10
    // 00 (Channel 0)
    // 11 (Access mode: lobyte/hibyte)
    // 011 (Mode 3: Square Wave Generator)
    // 0 (Binary mode)
    outb(PIT_COMMAND_PORT, 0x36);

    // Divisor has to be sent byte-wise, so split it into Low and High bytes.
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

    // Send the frequency divisor
    outb(PIT_CHANNEL0_PORT, low);
    outb(PIT_CHANNEL0_PORT, high);

    KDBG1("PIT Initialized at %d Hz", (int32_t)frequency);
}

void init_pci(FAT32* boot_partition, DriverManager* driverManager) {
    KDBG1("Initializing Drivers (PCI Scan)...");
    // ---------------------------------------------------------
    // Dynamic Graphics Loading (with PCI)
    // ---------------------------------------------------------

    // Scan for known BGA/SVGA Vendors
    // 0x1234:0x1111 = Bochs / QEMU
    // 0x80EE:0xBEEF = VirtualBox Graphics Adapter
    // 0x15AD:0x0405 = VMware SVGA II (often used by VBox)

    // Check if BGA Hardware Exists via PCI
    PeripheralComponentInterconnectController* pciCheck =
        new PeripheralComponentInterconnectController();
    if (!pciCheck) {
        HALT("CRITICAL: Failed to allocate PCI controller!\n");
    }
    PeripheralComponentInterconnectDeviceDescriptor* dev = nullptr;

    if (dev == nullptr) dev = pciCheck->FindHardwareDevice(0x1234, 0x1111);
    if (dev->vendor_id == 0) dev = pciCheck->FindHardwareDevice(0x80EE, 0xBEEF);
    if (dev->vendor_id == 0) dev = pciCheck->FindHardwareDevice(0x15AD, 0x0405);

    // Only Proceed if Device Found
    if (dev->vendor_id != 0) {
        char* BGAfilename = "DRIVERS/BGA.SYS";

        KDBG1("BGA Hardware Detected (ID: %x:%x). Loading Driver... [%s]", dev->vendor_id,
              dev->device_id, BGAfilename);
        File* bgaFile = boot_partition->Open(BGAfilename);

        if (bgaFile) {
            void* entryPoint =
                ModuleLoader::LoadMatchingDriver(bgaFile, dev->vendor_id, dev->device_id);

            if (entryPoint) {
                GetDriverInstancePtr createDriver = (GetDriverInstancePtr)entryPoint;
                void* raw = createDriver();  // Create the object

                if (raw) {
                    Driver* drv = (Driver*)raw;
                    drv->Activate();
                    driverManager->AddDriver(drv);

                    // SAFE CAST
                    GraphicsDriver* newScreen = drv->AsGraphicsDriver();

                    if (newScreen) {
                        GraphicsDriver* oldDR = g_GraphicsDriver;

                        // Update Local Reference
                        g_GraphicsDriver = newScreen;

                        // UPDATE GLOBAL VARIABLE
                        g_GraphicsDriver = newScreen;

                        // Copy Old Screen to New Screen
                        int32_t x, y;
                        g_GraphicsDriver->GetScreenCenter(oldDR->GetWidth(), oldDR->GetHeight(), x,
                                                          y);

                        if (oldDR->GetBackBuffer()) {
                            g_GraphicsDriver->DrawBitmap(x, y, oldDR->GetBackBuffer(),
                                                         oldDR->GetWidth(), oldDR->GetHeight());
                        }
                        g_GraphicsDriver->Flush();
                        KDBG1("BGA Module Loaded Successfully.");
                    } else {
                        KDBG1("Error: Driver loaded, but is not a GraphicsDriver!");
                    }
                }
            }

            bgaFile->Close();
            delete bgaFile;
        } else {
            KDBG1("Hardware found, but %s missing!", BGAfilename);
        }
    } else {
        KDBG1("No BGA Hardware found. Skipping driver load.");
    }

    // Clean up dev for next search
    if (dev) delete dev;
    dev = nullptr;

    // ---------------------------------------------------------
    // 2. Dynamic Audio Loading
    // ---------------------------------------------------------

    // Check for AC97 (0x8086:0x2415)
    dev = pciCheck->FindHardwareDevice(0x8086, 0x2415);

    if (dev->vendor_id != 0) {
        char* driverName = "DRIVERS/ac97.sys";
        KDBG1("Audio Hardware Detected. Loading... [%s]", driverName);

        File* drvFile = boot_partition->Open(driverName);
        if (drvFile) {
            void* entryPoint =
                ModuleLoader::LoadMatchingDriver(drvFile, dev->vendor_id, dev->device_id);
            if (entryPoint) {
                GetDriverInstancePtr createDriver = (GetDriverInstancePtr)entryPoint;
                void* raw = createDriver();
                if (raw) {
                    Driver* drv = (Driver*)raw;

                    // CRITICAL: Activate() registers the IRQ.
                    // InterruptManager MUST exist before this line runs.
                    drv->Activate();

                    driverManager->AddDriver(drv);
                    AudioDriver* audio = drv->AsAudioDriver();

                    if (audio) {
                        KDBG1("Initializing Audio Mixer...");
                        // Create the Mixer and link it to the driver
                        g_AudioMixer = new AudioMixer(audio);
                        if (!g_AudioMixer) {
                            HALT("CRITICAL: Failed to allocate AudioMixer!\n");
                        }

                        // Set Master Volume
                        audio->SetVolume(90);
                    }
                }
            }
            drvFile->Close();
            delete drvFile;
        }
    }
    delete pciCheck;
};

void pDesktop(void* arg) {
    DesktopArgs* args = (DesktopArgs*)arg;

#ifdef DEBUG_ENABLED
    KDBG1("GUI task started");
    if (!args) {
        HALT("Error: args is null");
    }
    if (!args->screen) {
        HALT("Error: args->vga is null");
    }
    if (!args->desktop) {
        HALT("Error: args->desktop is null");
    }

#endif

    GraphicsDriver* screen = args->screen;
    Desktop* desktop = args->desktop;

    Font* VBE_font = FontManager::activeInstance->getNewFont();
    VBE_font->setSize(MEDIUM);

    while (true) {
        // Only swap buffers if something actually changed
        uint32_t start = timerTicks;

        // If a fullscreen app is running, skip desktop drawing
        if (g_stop_gui_rendering) {
            screen->Flush();
            Scheduler::activeInstance->Sleep(16);
            continue;
        }

        // Periodic clock update check for taskbar
        static uint32_t lastClockTick = 0;
        if (timerTicks - lastClockTick >= 1000) {
            lastClockTick = timerTicks;
            desktop->MarkDirty();
        }

        if (desktop->isDirty || desktop->MouseMoved()) {
            desktop->Draw(screen);
            uint32_t end = timerTicks;
            uint32_t diff = (uint32_t)(end - start);
            char buf[32];
            itoa(buf, 16, diff);
            screen->FillRectangle(5, 5, 50, 35, 0x0);
            screen->DrawString(10, 10, buf, VBE_font, 0xFFFFFFFF);
            screen->DrawString(25, 10, "ms", VBE_font, 0xFFFFFFFF);
            screen->Flush();
        } else {
            Scheduler::activeInstance->Sleep(16);
        }
    }
}

extern "C" void kernelMain(void* multiboot_structure, uint32_t magicnumber) {
    initSerial();
    if (magicnumber != 0x2BADB002) {
#ifdef DEBUG_ENABLED
        KDBG1("Invalid magic number : [%x], Ignoring...", magicnumber);
#endif
        // return;
    }

    MultibootInfo* mbinfo = (MultibootInfo*)multiboot_structure;
#ifdef DEBUG_ENABLED
    KDBG1("Initializing Hardware");
#endif

    gdt_init();
    // tss_init();

    // Initialize PMM and Kheap
    init_memory(mbinfo);
    InitializePIT(1000);

#ifdef DEBUG_ENABLED
    KDBG1("Initializing paging...");
#endif

    g_paging = new Paging();
    if (!g_paging) {
        HALT("CRITICAL: Failed to allocate Paging object!\n");
    }
    g_paging->Activate();

    // Initialize ATA
    AdvancedTechnologyAttachment* ata = nullptr;
    AdvancedTechnologyAttachment* SATAList[] = {
        new AdvancedTechnologyAttachment(true, 0x1F0),   // Primary Master
        new AdvancedTechnologyAttachment(false, 0x1F0),  // Primary Slave
        new AdvancedTechnologyAttachment(true, 0x170),   // Secondary Master
        new AdvancedTechnologyAttachment(false, 0x170),  // Secondary Slave
        0};
    for (int i = 0; i < 4; i++) {
        if (!SATAList[i]) {
            HALT("CRITICAL: Failed to allocate ATA object!\n");
        }
    }

    for (int i = 0; SATAList[i] != 0; i++) {
        KDBG3("Checking Drive %d...", i);
        uint32_t ata_size = SATAList[i]->Identify();

        if (ata_size == 0) continue;  // No Drive detected

        ata = SATAList[i];

        // Use the FIRST drive found.
        KDBG1("Using ATA drive %d (Master/Slave)", i);
        break;
    }

    if (ata == nullptr) {
        HALT(
            "Error: No ATA drive detected!\nPlease connect an ATA drive and restart the system.\n");
    }

    // Initialize MBR and Partitions
    MSDOSPartitionTable* MSDOS = new MSDOSPartitionTable(ata);
    if (!MSDOS) {
        HALT("CRITICAL: Failed to allocate MSDOSPartitionTable!\n");
    }
    MSDOS->ReadPartitions();
    // Get Boot Partition
    g_bootPartition = MSDOS->partitions[0];
    if (!g_bootPartition) {
        HALT("Error: No valid boot partition found!\nPlease reinstall the OS using 'make hdd'.\n");
    }
    g_bootPartition->ListRoot();
    KDBG1("Boot partition mounted. Root listed.");
    KernelSymbolTable::Load(g_bootPartition, "kernel.map");
    KDBG1("Kernel symbols loaded from kernel.map");

    g_GraphicsDriver =
        new VESA_BIOS_Extensions(mbinfo->framebuffer_width, mbinfo->framebuffer_height, 32,
                                 (uint32_t*)mbinfo->framebuffer_addr);
    if (!g_GraphicsDriver) {
        HALT("CRITICAL: Failed to allocate VESA_BIOS_Extensions!\n");
    }

    // Load Boot Image
    char* bootImageName = (char*)"BITMAPS/BOOT.BMP";
    Bitmap* bootImg = new Bitmap(bootImageName);
    if (!bootImg) {
        HALT("CRITICAL: Failed to allocate Bitmap for boot image!\n");
    }
    if (bootImg->IsValid()) {
        int32_t x, y;
        g_GraphicsDriver->GetScreenCenter(bootImg->GetWidth(), bootImg->GetHeight(), x, y);
        g_GraphicsDriver->DrawBitmap(x, (int32_t)((g_GraphicsDriver->GetHeight() * 1) / 3),
                                     bootImg->GetBuffer(), bootImg->GetWidth(),
                                     bootImg->GetHeight());
        g_GraphicsDriver->Flush();
    }

    delete bootImg;

    // Load Font File
    g_fManager = new FontManager();
    if (!g_fManager) {
        HALT("CRITICAL: Failed to allocate FontManager!\n");
    }
    char* fontFileName = (char*)"FONTS/SEGOEUI.BIN";
    File* fontFile = g_bootPartition->Open(fontFileName);
    if (fontFile->size == 0) {
        KDBG1("Font error, file not found or empty: %s. Please reinstall the OS using 'make hdd'.",
              fontFile);
        while (1) {
            asm volatile("hlt");
        }
    }
    g_fManager->LoadFile(fontFile);
    fontFile->Close();
    delete fontFile;

    Font* BOOT = g_fManager->getNewFont();
    if (BOOT != nullptr) {
        BOOT->setSize(XLARGE);
        int32_t x, y;
        g_GraphicsDriver->GetScreenCenter(BOOT->getStringLength("Hash x86"), 0, x, y);
        g_GraphicsDriver->DrawString(x, (int32_t)((g_GraphicsDriver->GetHeight() * 1) / 3 + 300),
                                     "Hash x86", BOOT, 0xFFFFFFFF);
        g_GraphicsDriver->Flush();
    }

    // Load Desktop //
    Desktop* desktop = new Desktop(GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    if (!desktop) {
        HALT("CRITICAL: Failed to allocate Desktop!\n");
    }
    // gameSDK* desktop = new gameSDK(GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT, g_bootPartition);

    g_scheduler = new Scheduler(g_paging);
    if (!g_scheduler) {
        HALT("CRITICAL: Failed to allocate Scheduler!\n");
    }
    g_interrupts = new InterruptManager(g_scheduler, g_paging);
    if (!g_interrupts) {
        HALT("CRITICAL: Failed to allocate InterruptManager!\n");
    }
    g_sysCalls = new SyscallHandler(0x80, g_interrupts);
    if (!g_sysCalls) {
        HALT("CRITICAL: Failed to allocate SyscallHandler!\n");
    }
    HguiHandler* guiCalls = new HguiHandler(0x81, g_interrupts);  // Needs desktop initialized
    if (!guiCalls) {
        HALT("CRITICAL: Failed to allocate HguiHandler!\n");
    }

    g_driverManager = new DriverManager();
    if (!g_driverManager) {
        HALT("CRITICAL: Failed to allocate DriverManager!\n");
    }

    init_pci(g_bootPartition, g_driverManager);

    MouseDriver* mouse = new MouseDriver(g_interrupts, desktop);
    if (!mouse) {
        HALT("CRITICAL: Failed to allocate MouseDriver!\n");
    }
    g_driverManager->AddDriver(mouse);
    KeyboardDriver* keyboard = new KeyboardDriver(g_interrupts, desktop);
    if (!keyboard) {
        HALT("CRITICAL: Failed to allocate KeyboardDriver!\n");
    }
    g_driverManager->AddDriver(keyboard);

    // PROCESS MAIN //
    DesktopArgs* desktopArgs = new DesktopArgs{g_GraphicsDriver, desktop, g_bootPartition};
    if (!desktopArgs) {
        HALT("CRITICAL: Failed to allocate DesktopArgs!\n");
    }
    ProcessControlBlock* process1 = g_scheduler->CreateProcess(true, pDesktop, desktopArgs);

    if (mbinfo->mods_count > 0) {
        KDBG1("Found %d Modules", mbinfo->mods_count);
        struct multiboot_module* modules = (struct multiboot_module*)mbinfo->mods_addr;
        // fManager.LoadFile(modules[0].mod_start, modules[0].mod_end); // load font file
    } else {
        KDBG1("No modules found");
    }

    // Load and start sample programs
    g_elfLoader = new ELFLoader(g_paging, g_scheduler);
    if (!g_elfLoader) {
        HALT("CRITICAL: Failed to allocate ELFLoader!\n");
    }

    if (g_AudioMixer) {
        Wav* sound = new Wav("audio/boot.wav");
        if (!sound) {
            HALT("CRITICAL: Failed to allocate Wav object for boot sound!\n");
        }
        sound->Play();
    }

    KDBG1("Welcome to #x86!");
    g_driverManager->ActivateAll();
    KDBG1("System Drivers Activated.");
    g_interrupts->Activate();
    KDBG1("Interrupts Enabled. Entering Halt Loop.");

    while (1) {
        asm volatile("hlt");
    }
}
