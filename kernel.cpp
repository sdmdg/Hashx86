/**
 * @file        kernal.cpp
 * @brief       Main Kernal of #x86
 * 
 * @date        24/02/2025
 * @version     1.2.0-beta
 */

#include <kernel.h>

bool is_VBE_ENABLED = true;
char Buffer[32]; // For itoa only


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

void wait(uint32_t milliseconds) {
    const uint32_t iterations_per_ms = 1000000; // Only a rough estimate
    volatile uint32_t count = milliseconds * iterations_per_ms;
    while (count--) {
        __asm__ volatile("nop");
    }
}


void onmyButtonClick() {
    DEBUG_LOG("You Clicked Me :)");
}


extern "C" void kernelMain(void* multiboot_structure, uint32_t magicnumber) {
    initSerial();
    // VESA Graphics
    if (is_VBE_ENABLED) {
        if (magicnumber != 0x2BADB002) {
            SerialPrint("Invalid magic number : [");
            SerialPrint(itoa(magicnumber, Buffer, 16));
            SerialPrint("], Ignoring...\n");
            //return;
        }
        
        MultibootInfo* mbinfo = (MultibootInfo*)multiboot_structure;
        VESA_BIOS_Extensions vga(mbinfo);

        vga.DrawBitmap(412, 200, (const uint32_t*)icon_main_200x200, 200, 200);
        SegoeUI BOOT;
        BOOT.setSize(XLARGE);
        uint32_t x;
        x = (1024 - BOOT.getStringLength("Hash x86")) / 2;
        vga.DrawString(x,500,"Hash x86", &BOOT, 0xFFFFFFFF);
        vga.Flush();

        wait(500);

        GlobalDescriptorTable gdt;
        InterruptManager interrupts(&gdt);

        Desktop desktop(1024, 768);

        DEBUG_LOG("Initializing Hardware");
        DriverManager driverManager;
        
        MouseDriver mouse(&interrupts, &desktop);
        driverManager.AddDriver(&mouse);
        
        KeyboardDriver keyboard(&interrupts, &desktop);
        driverManager.AddDriver(&keyboard);

        PeripheralComponentInterconnectController PCIcontroller;
        PCIcontroller.SelectDrivers(&driverManager, &interrupts);

        driverManager.ActivateAll();
        interrupts.Activate();

        DEBUG_LOG("Welcome to #x86!\n");
        
        Window win1(&desktop, 150,100,400,500);
        win1.setWindowTitle("Welcome !");

        //Label myLabel(&win1, 12, 10, 154, 20, "Welcome to #x86!\nThis is VESA mode :)", 19);
        Button myButton(&win1, 108, 90, 110, 25, "Click Me");
        myButton.OnClick(&onmyButtonClick);
        //win1.AddChild(&myLabel);
        win1.AddChild(&myButton);
        desktop.AddChild(&win1);

        while (1){
            desktop.Draw(&vga);
            vga.Flush();
            wait(1);
        }



    // VGA Graphics
    } else {
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
        vga.DrawBitmap(135, 50, icon_main_bw_50x50, 50, 50);
        vga.Flush();

        wait(500);


        Window win1(&desktop, 20,15,180,120, 10);
        win1.setWindowTitle("Welcome !");

        Label myLabel(&win1, 12, 10, 154, 20, "Welcome to #x86!\nThis is vga mode :)", 19);
        Button myButton(&win1, 108, 90, 66, 14, "Click Me", 19);
        win1.AddChild(&myLabel);
        win1.AddChild(&myButton);
        desktop.AddChild(&win1);
     */
    }
} 
