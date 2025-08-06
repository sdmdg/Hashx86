/**
 * @file        kernal.cpp
 * @brief       Main Kernal of #x86
 * 
 * @date        24/02/2025
 * @version     1.2.0-beta
 */

#include <kernel.h>

#define VBE_ENABLED false;
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
    if (get_kernel_memory_map(&g_kmap, mbinfo) < 0) {
        #ifdef DEBUG_ENABLED
            DEBUG_LOG("Error: Failed to get kernel memory map");
        #endif
        return;
    }

    #ifdef DEBUG_ENABLED
        //display_kernel_memory_map(&g_kmap);
        DEBUG_LOG("Total Memory: %d KB, %d Bytes", g_kmap.system.total_memory, g_kmap.available.size);
        DEBUG_LOG("Start addr: 0x%x, End addr: 0x%x", g_kmap.available.start_addr, g_kmap.available.end_addr);
        DEBUG_LOG("Kernal start addr: 0x%x, Kernal end addr: 0x%x", g_kmap.kernel.k_start_addr, g_kmap.kernel.data_end_addr);
    #endif


    // Initialize PMM (From 100MB to end)
    uint32_t offset = 25600 * PMM_BLOCK_SIZE;
    pmm_init(0 + offset, g_kmap.available.end_addr - offset);
    // Mark available memory regions as free
    pmm_init_region(0 + offset, g_kmap.available.end_addr - offset);
    DEBUG_LOG("Max blocks: %d\n\n", pmm_get_max_blocks());
    
    // Allocate heap (100MB)
    void* heap_start = pmm_alloc_blocks(25600);
    if (heap_start == NULL) {
        DEBUG_LOG("Failed to allocate kernel heap!");
    }

    void* heap_end = (void*)((uint32_t)heap_start + (25600 * PMM_BLOCK_SIZE));
    DEBUG_LOG("Kernel Heap: 0x%x - 0x%x\n", heap_start, heap_end);
    kheap_init(heap_start, heap_end);
    //MemoryManager memoryManager((uint32_t)heap_start, (void*)((uint32_t)heap_end-(uint32_t)heap_start));
}

void pDesktop(void* arg) {
    DesktopArgs* args = (DesktopArgs*)arg;

    #ifdef DEBUG_ENABLED
    DEBUG_LOG("GUI task started");
        if (!args) {
            DEBUG_LOG("Error: args is null");
        return;
        }
        if (!args->vbe) {
            DEBUG_LOG("Error: args->vga is null");
            return;
        }
        if (!args->desktop) {
            DEBUG_LOG("Error: args->desktop is null");
            return;
        }
    #endif

    VESA_BIOS_Extensions* vbe = args->vbe;
    Desktop* desktop = args->desktop;
    

/*     Window* win1 = new Window(desktop, 150,300,400,500);
    win1->setWindowTitle("Welcome !");

    Button* button1 = new Button(win1, 10, 120, 110, 25, "Click Me");
    win1->AddChild(button1);

    Button* crashButton = new Button(win1, 200, 120, 110, 25, "Crash Me");
    win1->AddChild(crashButton);



    Window* win2 = new Window(desktop, 150,300,400,500);
    win2->setWindowTitle("Welcome !");

    button1 = new Button(win2, 10, 120, 110, 25, "Click Me");
    win2->AddChild(button1);

    crashButton = new Button(win2, 200, 120, 110, 25, "Crash Me");
    win2->AddChild(crashButton);



    Window* win3 = new Window(desktop, 150,300,400,500);
    win3->setWindowTitle("Welcome !");

    button1 = new Button(win3, 10, 120, 110, 25, "Click Me");
    win3->AddChild(button1);

    crashButton = new Button(win3, 200, 120, 110, 25, "Crash Me");
    win3->AddChild(crashButton);



    Window* win4 = new Window(desktop, 150,300,400,500);
    win4->setWindowTitle("Welcome !");

    button1 = new Button(win4, 10, 120, 110, 25, "Click Me");
    win4->AddChild(button1);

    crashButton = new Button(win4, 200, 120, 110, 25, "Crash Me");
    win4->AddChild(crashButton);



    desktop->AddChild(win1);
    desktop->AddChild(win2);
    desktop->AddChild(win3);
    desktop->AddChild(win4); */



    while (1) {
        desktop->Draw(vbe);
        vbe->Flush();
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
    tss_init();

    // Initialize PMM and Kheap with 100MB offset
    init_memory(mbinfo);

    #ifdef DEBUG_ENABLED
        DEBUG_LOG("Initializing paging...\n");
    #endif
    Paging paging;
    paging.Activate();

    ProcessManager pManager(&paging);
    VESA_BIOS_Extensions* vbe = new VESA_BIOS_Extensions(mbinfo);
    NINA nina;
    InterruptManager interrupts(&pManager, vbe, &paging);
    DriverManager driverManager;
    SyscallHandler sysCalls(0x80, &interrupts);
    ELFLoader elfLoader(&paging, &pManager);

    // VESA Graphics
    #ifdef VBE_ENABLED
        vbe->DrawBitmap(476, 250, (const uint32_t*)icon_main_200x200, 200, 200);
        SegoeUI* BOOT = new SegoeUI();
        BOOT->setSize(XLARGE);
        vbe->DrawString(((1152 - BOOT->getStringLength("Hash x86")) / 2),550,"Hash x86", BOOT, 0xFFFFFFFF);
        vbe->Flush();

        wait(500);

        Desktop desktop(1152, 864);
        HguiHandler guiCalls(0x81, &interrupts);
        
        MouseDriver mouse(&interrupts, &desktop);
        driverManager.AddDriver(&mouse);
        


/*         KeyboardDriver keyboard(&interrupts, &desktop);
        driverManager.AddDriver(&keyboard); */
/* 
        PeripheralComponentInterconnectController PCIcontroller;
        PCIcontroller.SelectDrivers(&driverManager, &interrupts); */


        DesktopArgs desktopArgs { vbe, &desktop };
        Process* process1 = new Process(&paging, pDesktop, &desktopArgs);
        pManager.mapKernel(process1);
        pManager.AddProcess(process1);


        ProgramArguments ProcessArgs { "ARG1", "ARG2" , "ARG3" , "ARG4", "ARG5"};
        if (mbinfo->mods_count > 0) {
            DEBUG_LOG("Found %d Modules", mbinfo->mods_count);
            struct multiboot_module* modules = (struct multiboot_module*)mbinfo->mods_addr;

            for (uint32_t mod_idx = 1; mod_idx < mbinfo->mods_count; mod_idx++) { // Bypass 1st mod
                uint32_t mod_start = modules[mod_idx].mod_start;
                uint32_t mod_end = modules[mod_idx].mod_end;

                Process* prog =  elfLoader.loadModule(mod_start, mod_end, &ProcessArgs);
                elfLoader.startModule(prog);
            }

        } else {
            DEBUG_LOG("No modules found");
        }

    #endif


    DEBUG_LOG("Welcome to #x86!\n");
    driverManager.ActivateAll();


    interrupts.Activate();

    while (1);
    








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



/* 

void ConsoleKeyboardEventHandler::OnKeyDown(const char* key){
    printf(WHITE, "%s", key);
}
void ConsoleKeyboardEventHandler::OnKeyUp(const char* key){
}
void ConsoleKeyboardEventHandler::OnSpecialKeyDown(uint8_t key){
    switch (key) {
        default:
            printf(RED, "Unhandled extended keyDown: 0x%x\n", key);
            break;
    }
}
void ConsoleKeyboardEventHandler::OnSpecialKeyUp(uint8_t key){
    switch (key) {
        default:
            printf(RED, "Unhandled extended keyUp: 0x%x\n", key);
            break;
    }
}

void ConsoleMouseEventHandler::OnMouseMove(int dx, int dy) {
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    int x,y;
    x += dx;
    y -= dy; // Invert Y-axis to match screen coordinates

    // Clamp cursor position within screen bounds
    x = (x < 0) ? 0 : (x >= 80 ? 79 : x);
    y = (y < 0) ? 0 : (y >= 25 ? 24 : y);


    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                        | (VideoMemory[80*y+x] & 0xF000) >> 4
                        | (VideoMemory[80*y+x] & 0x00FF);

    VideoMemory[80*Y+X] = (VideoMemory[80*Y+X] & 0x0F00) << 4
                        | (VideoMemory[80*Y+X] & 0xF000) >> 4
                        | (VideoMemory[80*Y+X] & 0x00FF);
    // Update the cursor position
    X = x;
    Y = y;
}

void ConsoleMouseEventHandler::OnLeftMouseDown(int dx, int dy) {
    printf(WHITE, "Left mouse button pressed at (%d, %d)\n", X, Y);
}

void ConsoleMouseEventHandler::OnLeftMouseUp(int dx, int dy) {
    printf(WHITE, "Left mouse button released at (%d, %d)\n", X, Y);
}

void ConsoleMouseEventHandler::OnRightMouseDown(int dx, int dy) {
    printf(WHITE, "Right mouse button pressed at (%d, %d)\n", X, Y);
}

void ConsoleMouseEventHandler::OnRightMouseUp(int dx, int dy) {
    printf(WHITE, "Right mouse button released at (%d, %d)\n", X, Y);
}


 */