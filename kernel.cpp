/**
 * @file        kernel.cpp
 * @brief       Main Kernel of #x86
 *
 * @date        24/02/2025
 * @version     1.2.0-beta
 */

#include <core/drivers/AudioMixer.h>
#include <kernel.h>

#define DEBUG_ENABLED TRUE;
#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40

KERNEL_MEMORY_MAP g_kmap;

extern "C" uint32_t pci_find_bar0(uint16_t vendor, uint16_t device);
extern "C" void __cxa_pure_virtual() {
    // This is the ONE definition that everyone will share
    printf("Pure Virtual Function Called! System Halted.\n");
    while (1);
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
    printf("kernel:\n");
    printf("  kernel-start: 0x%x, kernel-end: 0x%x, TOTAL: %d bytes\n", kmap->kernel.k_start_addr,
           kmap->kernel.k_end_addr, kmap->kernel.k_len);
    printf("  text-start: 0x%x, text-end: 0x%x, TOTAL: %d bytes\n", kmap->kernel.text_start_addr,
           kmap->kernel.text_end_addr, kmap->kernel.text_len);
    printf("  data-start: 0x%x, data-end: 0x%x, TOTAL: %d bytes\n", kmap->kernel.data_start_addr,
           kmap->kernel.data_end_addr, kmap->kernel.data_len);
    printf("  rodata-start: 0x%x, rodata-end: 0x%x, TOTAL: %d\n", kmap->kernel.rodata_start_addr,
           kmap->kernel.rodata_end_addr, kmap->kernel.rodata_len);
    printf("  bss-start: 0x%x, bss-end: 0x%x, TOTAL: %d\n", kmap->kernel.bss_start_addr,
           kmap->kernel.bss_end_addr, kmap->kernel.bss_len);

    printf("total_memory: %d KB\n", kmap->system.total_memory);
    printf("available:\n");
    printf("  start_adddr: 0x%x\n  end_addr: 0x%x\n  size: %d\n", kmap->available.start_addr,
           kmap->available.end_addr, kmap->available.size);
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

    // Define the Hard Ceiling (240 MB)
    uint32_t paging_limit = 240 * 1024 * 1024;
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

    printf("[PMM] Maximizing Heap: Start=0x%x Limit=0x%x Size=%d MB\n", actual_heap_start,
           safe_limit, (int32_t)(heap_size_bytes / (1024 * 1024)));

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

    DEBUG_LOG("Kernel Heap: 0x%x - 0x%x (%d MB)", heap_start, heap_end, heap_size / 1024 / 1024);

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

    printf("PIT Initialized at %d Hz\n", (int32_t)frequency);
}

void init_pci(FAT32* boot_partition, DriverManager* driverManager) {
    printf("\n[Kernel] Initializing Drivers...\n");
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

        printf("[Kernel] BGA Hardware Detected (ID: %x:%x). Loading Driver... [%s]\n",
               dev->vendor_id, dev->device_id, BGAfilename);
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
                        printf("BGA Module Loaded Successfully.\n");
                    } else {
                        printf("[Kernel] Error: Driver loaded, but is not a GraphicsDriver!\n");
                    }
                }
            }

            bgaFile->Close();
            delete bgaFile;
        } else {
            printf("[Kernel] Hardware found, but %s missing!\n", BGAfilename);
        }
    } else {
        printf("[Kernel] No BGA Hardware found. Skipping driver load.\n");
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
        printf("[Kernel] Audio Hardware Detected. Loading... [%s]\n", driverName);

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
                        printf("[Kernel] Initializing Mixer...\n");
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
    DEBUG_LOG("GUI task started");
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
        DEBUG_LOG("Invalid magic number : [%x], Ignoring...\n", magicnumber);
#endif
        // return;
    }

    MultibootInfo* mbinfo = (MultibootInfo*)multiboot_structure;
#ifdef DEBUG_ENABLED
    DEBUG_LOG("Initializing Hardware");
#endif

    gdt_init();
    // tss_init();

    // Initialize PMM and Kheap
    init_memory(mbinfo);
    InitializePIT(1000);

#ifdef DEBUG_ENABLED
    DEBUG_LOG("Initializing paging...\n");
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
        printf("Checking Drive %d...\n", i);
        uint32_t ata_size = SATAList[i]->Identify();

        if (ata_size == 0) continue;  // No Drive detected

        ata = SATAList[i];

        // Use the FIRST drive found.
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
    KernelSymbolTable::Load(g_bootPartition, "kernel.map");

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
        printf(
            "Font error, file not found or empty: %s\nPlease reinstall the OS using 'make hdd'.\n",
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
        DEBUG_LOG("Found %d Modules", mbinfo->mods_count);
        struct multiboot_module* modules = (struct multiboot_module*)mbinfo->mods_addr;
        // fManager.LoadFile(modules[0].mod_start, modules[0].mod_end); // load font file
    } else {
        DEBUG_LOG("No modules found");
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

    DEBUG_LOG("Welcome to #x86!\n");
    g_driverManager->ActivateAll();
    g_interrupts->Activate();

    while (1) {
        asm volatile("hlt");
    }

    // VGA Graphics

    /* clearScreen();
    VideoGraphicsArray vga;

    GlobalDescriptorTable gdt;

    InterruptManager interrupts(&gdt);

    Desktop desktop(320, 200, 0);

    DEBUG_LOG("Initializing Hardware");
    DriverManager driverManager;

    ConsoleMouseEventHandler Mousehandler;
    MouseDriver mouse(&interrupts, &Mousehandler);
    driverManager.AddDriver(&mouse);

    ConsoleKeyboardEventHandler KeyBoardhandler;
    KeyboardDriver keyboard(&interrupts, &KeyBoardhandler);
    driverManager.AddDriver(&keyboard);

    PeripheralComponentInterconnectController PCIcontroller;
    PCIcontroller.SelectDrivers(&driverManager, &interrupts);


    driverManager.ActivateAll();

    interrupts.Activate();

    printf(YELLOW, "Welcome to #x86!\n");

    if (!vga.SetMode(320, 200, 8)) {
        // if the mode is not supported
        return;
    }

    vga.FillRectangle(0, 0, 320, 200, 0);
    vga.DrawString(128, 130, "Hash x86", 31);
    //vga.DrawBitmap(135, 50, icon_main_bw_50x50, 50, 50);
    vga.Flush();

    wait(500);


    Window win1(&desktop, 20,15,180,120, 10);
    win1.setWindowTitle("Welcome !");

    Label myLabel(&win1, 12, 10, 154, 20, "Welcome to #x86!\nThis is vga mode :)", 19);
    Button myButton(&win1, 108, 90, 66, 14, "Click Me", 19);
    win1.AddChild(&myLabel);
    win1.AddChild(&myButton);
    desktop.AddChild(&win1); */
}
