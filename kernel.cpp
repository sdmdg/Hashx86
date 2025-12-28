/**
 * @file        kernal.cpp
 * @brief       Main Kernal of #x86
 * 
 * @date        24/02/2025
 * @version     1.2.0-beta
 */

#include <kernel.h>

#define screen_ENABLED true;
#define DEBUG_ENABLED true;

KERNEL_MEMORY_MAP g_kmap;



int get_kernel_memory_map(KERNEL_MEMORY_MAP *kmap, MultibootInfo *mboot_info) {
    if (kmap == NULL) return -1;
    kmap->kernel.k_start_addr = (uint32_t)&__kernel_section_start;
    kmap->kernel.k_end_addr = (uint32_t)&__kernel_section_end;
    kmap->kernel.k_len = ((uint32_t)&__kernel_section_end - (uint32_t)&__kernel_section_start);

    kmap->kernel.text_start_addr = (uint32_t)&__kernel_text_section_start;
    kmap->kernel.text_end_addr = (uint32_t)&__kernel_text_section_end;
    kmap->kernel.text_len = ((uint32_t)&__kernel_text_section_end - (uint32_t)&__kernel_text_section_start);

    kmap->kernel.data_start_addr = (uint32_t)&__kernel_data_section_start;
    kmap->kernel.data_end_addr = (uint32_t)&__kernel_data_section_end; 
    kmap->kernel.data_len = ((uint32_t)&__kernel_data_section_end - (uint32_t)&__kernel_data_section_start);

    kmap->kernel.rodata_start_addr = (uint32_t)&__kernel_rodata_section_start;
    kmap->kernel.rodata_end_addr = (uint32_t)&__kernel_rodata_section_end;
    kmap->kernel.rodata_len = ((uint32_t)&__kernel_rodata_section_end - (uint32_t)&__kernel_rodata_section_start);

    kmap->kernel.bss_start_addr = (uint32_t)&__kernel_bss_section_start;
    kmap->kernel.bss_end_addr = (uint32_t)&__kernel_bss_section_end; 
    kmap->kernel.bss_len = ((uint32_t)&__kernel_bss_section_end - (uint32_t)&__kernel_bss_section_start);

    kmap->system.total_memory = mboot_info->mem_lower + mboot_info->mem_upper;

    for (int i = 0; i < mboot_info->mmap_length; i += sizeof(MULTIBOOT_MEMORY_MAP)) {
        MULTIBOOT_MEMORY_MAP *mmap = (MULTIBOOT_MEMORY_MAP *)(mboot_info->mmap_addr + i);
        if (mmap->type != MULTIBOOT_MEMORY_AVAILABLE) continue;
        // make sure kernel is loaded at 0x100000 by bootloader(see linker.ld)
        if (mmap->addr_low == kmap->kernel.text_start_addr) {
            // set available memory starting from end of our kernel, leaving 1MB size for functions exceution
            kmap->available.start_addr = kmap->kernel.k_end_addr + 1024 * 1024;
            kmap->available.end_addr = mmap->addr_low + mmap->len_low;
            // get availabel memory in bytes
            kmap->available.size = kmap->available.end_addr - kmap->available.start_addr;
            return 0;
        }
    }

    return -1;
}

void display_kernel_memory_map(KERNEL_MEMORY_MAP *kmap) {
    printf("kernel:\n");
    printf("  kernel-start: 0x%x, kernel-end: 0x%x, TOTAL: %d bytes\n", 
            kmap->kernel.k_start_addr, kmap->kernel.k_end_addr, kmap->kernel.k_len);
    printf("  text-start: 0x%x, text-end: 0x%x, TOTAL: %d bytes\n", 
            kmap->kernel.text_start_addr, kmap->kernel.text_end_addr, kmap->kernel.text_len);
    printf("  data-start: 0x%x, data-end: 0x%x, TOTAL: %d bytes\n", 
            kmap->kernel.data_start_addr, kmap->kernel.data_end_addr, kmap->kernel.data_len);
    printf("  rodata-start: 0x%x, rodata-end: 0x%x, TOTAL: %d\n",
            kmap->kernel.rodata_start_addr, kmap->kernel.rodata_end_addr, kmap->kernel.rodata_len);
    printf("  bss-start: 0x%x, bss-end: 0x%x, TOTAL: %d\n",
            kmap->kernel.bss_start_addr, kmap->kernel.bss_end_addr, kmap->kernel.bss_len);

    printf("total_memory: %d KB\n", kmap->system.total_memory);
    printf("available:\n");
    printf("  start_adddr: 0x%x\n  end_addr: 0x%x\n  size: %d\n", 
            kmap->available.start_addr, kmap->available.end_addr, kmap->available.size);
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

    // Add 16KB padding to be safe.
    heap_start_addr += 0x4000; 

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
    pmm_init_region(heap_start_addr, g_kmap.available.end_addr);
    
    // --- HEAP ALLOCATION ---
    uint32_t blocks_needed = 8192 * 6;
    void* heap_start = pmm_alloc_blocks(blocks_needed);
    
    if (heap_start == NULL) {
        // Fallback to 12MB if 64MB fails (e.g. on smaller VMs)
        blocks_needed = 3072;
        heap_start = pmm_alloc_blocks(blocks_needed);
        if (heap_start == NULL) {
            printf("CRITICAL: Failed to allocate kernel heap!\n");
            while(1) asm volatile("hlt");
        }
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
    
    DEBUG_LOG("Kernel Heap: 0x%x - 0x%x (%d MB)", heap_start, heap_end, heap_size/1024/1024);
    
    kheap_init(heap_start, heap_end);
}

void init_pci(FAT32* boot_partition, DriverManager* driverManager,  GraphicsDriver*& screen) {
    printf("\n[Kernel] Initializing Drivers...\n");
    // ---------------------------------------------------------
    // Dynamic Graphics Loading (with PCI)
    // ---------------------------------------------------------

    // Scan for known BGA/SVGA Vendors
    // 0x1234:0x1111 = Bochs / QEMU
    // 0x80EE:0xBEEF = VirtualBox Graphics Adapter
    // 0x15AD:0x0405 = VMware SVGA II (often used by VBox)

    // Check if BGA Hardware Exists via PCI
    PeripheralComponentInterconnectController* pciCheck = new PeripheralComponentInterconnectController();
    PeripheralComponentInterconnectDeviceDescriptor* dev = nullptr;
    if (dev == nullptr) dev = pciCheck->FindHardwareDevice(0x1234, 0x1111);
    if (dev->vendor_id == 0) dev = pciCheck->FindHardwareDevice(0x80EE, 0xBEEF);
    if (dev->vendor_id == 0) dev = pciCheck->FindHardwareDevice(0x15AD, 0x0405);

    // Only Proceed if Device Found
    if (dev->vendor_id != 0) {
        char* BGAfilename = "DRIVERS/BGA.SYS";

        printf("[Kernel] BGA Hardware Detected (ID: %x:%x). Loading Driver... [%s]\n", dev->vendor_id, dev->device_id, BGAfilename);
        File* bgaFile = boot_partition->Open(BGAfilename);

        if (bgaFile) {
            void* entryPoint = ModuleLoader::LoadMatchingDriver(bgaFile, dev->vendor_id, dev->device_id);
            
            if (entryPoint) {
                GetDriverInstancePtr createDriver = (GetDriverInstancePtr)entryPoint;
                void* raw = createDriver();
                
                if (raw) {
                    Driver* drv = (Driver*)raw;
                    
                    // Activate initializes the card and sets internal videoMemory
                    drv->Activate(); 
                    driverManager->AddDriver(drv);

                    GraphicsDriver* oldDR = screen;

                    // This pointer math is required because of the multiple inheritance shift
                    screen = (GraphicsDriver*)((uint32_t)raw + sizeof(Driver));

                    int32_t x ,y;
                    screen->GetScreenCenter(oldDR->GetWidth(), oldDR->GetHeight(), x, y);
                    screen->DrawBitmap(x, y, oldDR->GetBackBuffer(), oldDR->GetWidth(), oldDR->GetHeight());
                    screen->Flush();
                    delete oldDR;
                    printf("BGA Module Loaded Successfully.\n");
                }
            } else {
                printf("[Kernel] Failed to link BGA module.\n");
            }
            bgaFile->Close();
            delete bgaFile;
        } else {
            printf("[Kernel] Hardware found, but %s missing!\n", BGAfilename);
        }
    } else {
        printf("[Kernel] No BGA Hardware found. Skipping driver load.\n");
    }

    delete pciCheck;
    delete dev;
};


void pDesktop(void* arg) {
    DesktopArgs* args = (DesktopArgs*)arg;

    #ifdef DEBUG_ENABLED
        DEBUG_LOG("GUI task started");
        if (!args) {
            DEBUG_LOG("Error: args is null");
        return;
        }
        if (!args->screen) {
            DEBUG_LOG("Error: args->vga is null");
            return;
        }
        if (!args->desktop) {
            DEBUG_LOG("Error: args->desktop is null");
            return;
        }
    #endif

    GraphicsDriver* screen = args->screen;
    Desktop* desktop = args->desktop;

/*     Window* win1 = new Window(desktop, 150,300,400,500);
    InputBox* input = new InputBox(win1, 10, 120, 110, 25, 20);
    win1->setWindowTitle("Welcome !");
    win1->AddChild(input);
    desktop->AddChild(win1); */
    

/*  Window* win1 = new Window(desktop, 150,300,400,500);
    win1->setWindowTitle("Welcome !");

    Button* button1 = new Button(win1, 10, 120, 110, 25, "Click Me");
    win1->AddChild(button1);

    Button* crashButton = new Button(win1, 200, 120, 110, 25, "Crash Me");
    win1->AddChild(crashButton);



    Window* win4 = new Window(desktop, 150,300,400,500);
    win4->setWindowTitle("Welcome !");

    button1 = new Button(win4, 10, 120, 110, 25, "Click Me");
    win4->AddChild(button1);

    crashButton = new Button(win4, 200, 120, 110, 25, "Crash Me");
    win4->AddChild(crashButton);



    desktop->AddChild(win1);
    desktop->AddChild(win4); */

    //uint8_t a = 0/0;

    while (1) {
        //uint32_t start = timerTicks;
        desktop->Draw(screen);
        screen->Flush();
        //uint32_t end = timerTicks;
        //DEBUG_LOG("Taken : %d ms", (end-start));
    }
}


extern "C" void kernelMain(void* multiboot_structure, uint32_t magicnumber) {
    initSerial();
    if (magicnumber != 0x2BADB002) {
        #ifdef DEBUG_ENABLED
            DEBUG_LOG("Invalid magic number : [%x], Ignoring...\n", magicnumber);
        #endif
        //return;
    }

    MultibootInfo* mbinfo = (MultibootInfo*)multiboot_structure;
    #ifdef DEBUG_ENABLED
        DEBUG_LOG("Initializing Hardware");
    #endif

    gdt_init();
    //tss_init();

    // Initialize PMM and Kheap
    init_memory(mbinfo);

    #ifdef DEBUG_ENABLED
        DEBUG_LOG("Initializing paging...\n");
    #endif
    Paging* paging = new Paging();
    paging->Activate();

    // Initialize ATA
    AdvancedTechnologyAttachment* ata = nullptr;
    AdvancedTechnologyAttachment* SATAList[] = { 
        new AdvancedTechnologyAttachment(true, 0x1F0),  // Primary Master
        new AdvancedTechnologyAttachment(false, 0x1F0), // Primary Slave
        new AdvancedTechnologyAttachment(true, 0x170),  // Secondary Master
        new AdvancedTechnologyAttachment(false, 0x170), // Secondary Slave
        0
    };

    for (int i = 0; SATAList[i] != 0; i++) 
    { 
        printf("Checking Drive %d...\n", i);
        uint32_t ata_size = SATAList[i]->Identify();

        if (ata_size == 0) continue; // No Drive detected

        ata = SATAList[i];
        
        // Use the FIRST drive found.
        break; 
    }

    if (ata == nullptr) {
        printf("Error: No ATA drive detected!\nPlease connect an ATA drive and restart the system.\n");
        while (1) asm volatile ("hlt");
    }

    // Initialize MBR and Partitions
    MSDOSPartitionTable* MSDOS = new MSDOSPartitionTable(ata);
    MSDOS->ReadPartitions();
    // Get Boot Partition
    FAT32* boot_partition = MSDOS->partitions[0];
    if (!boot_partition) {
        printf("Error: No valid boot partition found!\nPlease reinstall the OS using 'make hdd'.\n");
        while (1) { asm volatile ("hlt"); }
    }
    boot_partition->ListRoot();

    GraphicsDriver* screen = new VESA_BIOS_Extensions(
                mbinfo->framebuffer_width, mbinfo->framebuffer_height, 
                32, (uint32_t *)mbinfo->framebuffer_addr
            );

    // Load Boot Image
    char* bootImageName = (char *)"BITMAPS/BOOT.BMP";
    Bitmap* bootImg = new Bitmap(bootImageName);
    if (bootImg->IsValid()) {
        int32_t x,y;
        screen->GetScreenCenter(bootImg->GetWidth(), bootImg->GetHeight(), x, y);
        screen->DrawBitmap(x, (int32_t)((screen->GetHeight() * 1) / 3), bootImg->GetBuffer(), bootImg->GetWidth(), bootImg->GetHeight());
        screen->Flush();
    }
    
    delete bootImg;

    // Load Font File
    FontManager* fManager = new FontManager();
    char * fontFileName = (char *)"FONTS/SEGOEUI.BIN";
    File* fontFile = boot_partition->Open(fontFileName);
    if (fontFile->size == 0) {
        printf("Font error, file not found or empty: %s\nPlease reinstall the OS using 'make hdd'.\n", fontFile);
        while (1) { asm volatile ("hlt"); }
    }
    fManager->LoadFile(fontFile);
    fontFile->Close();
    delete fontFile;

    Font* BOOT = fManager->getNewFont();
    if (BOOT != nullptr){
        BOOT->setSize(XLARGE);
        int32_t x,y;
        screen->GetScreenCenter(BOOT->getStringLength("Hash x86"), 0, x, y);
        screen->DrawString(x, (int32_t)((screen->GetHeight() * 1) / 3 + 300),"Hash x86", BOOT, 0xFFFFFFFF);
        screen->Flush();
    }


    DriverManager* driverManager = new DriverManager();
    init_pci(boot_partition, driverManager, screen);


    ProcessManager* pManager = new ProcessManager(paging);
    InterruptManager interrupts(pManager, screen, paging);
    SyscallHandler* sysCalls = new SyscallHandler(0x80, &interrupts);

    // Load Desktop
    Desktop* desktop = new Desktop(GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    HguiHandler* guiCalls = new HguiHandler(0x81, &interrupts);
    
    MouseDriver* mouse = new MouseDriver(&interrupts, desktop);
    driverManager->AddDriver(mouse);
    KeyboardDriver* keyboard = new KeyboardDriver(&interrupts, desktop);
    driverManager->AddDriver(keyboard);

    DesktopArgs* desktopArgs = new DesktopArgs{ screen, desktop };
    Process* process1 = new Process(paging, pDesktop, desktopArgs);
    pManager->mapKernel(process1);
    pManager->AddProcess(process1);

    
    if (mbinfo->mods_count > 0) {
        DEBUG_LOG("Found %d Modules", mbinfo->mods_count);
        struct multiboot_module* modules = (struct multiboot_module*)mbinfo->mods_addr;
        //fManager.LoadFile(modules[0].mod_start, modules[0].mod_end); // load font file
    } else {
        DEBUG_LOG("No modules found");
    }

    // Load and start sample programs
    ELFLoader* elfLoader = new ELFLoader(paging, pManager);
    ProgramArguments* ProcessArgs = new ProgramArguments{ "ARG1", "ARG2", "ARG3", "ARG4", "ARG5" };
    
    const char* binList[] = { 
        "BIN/MEMVIEW.BIN", 
        "BIN/TEST.BIN", 
        0
    };

    for (int i = 0; binList[i] != 0; i++) 
    {   
        // Open the file
        File* file = boot_partition->Open((char*)binList[i]);
        
        if (file == 0) {
            printf("File not found: %s\nPlease reinstall OS.\n", binList[i]);
            delete file;
            continue;
        }

        // Load the ELF from the file
        Process* prog = elfLoader->loadELF(file, (void*)ProcessArgs);

        if (prog) {
            elfLoader->runELF(prog);
        } else {
            printf("Failed to load ELF: %s\n", binList[i]);
        }

        file->Close();
        delete file; 
    }


    DEBUG_LOG("Welcome to #x86!\n");
    driverManager->ActivateAll();

    interrupts.Activate();


    while (1) { asm volatile ("hlt"); }
    


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
