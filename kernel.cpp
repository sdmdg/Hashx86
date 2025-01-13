/**
 * @file        kernal.cpp
 * @brief       Main Kernal of #x86
 * 
 * @author      Malaka Gunawardana
 * @date        13/01/2025
 * @version     1.0.0-beta
 */

#include <kernel.h>


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


void ConsoleMouseEventHandler::OnMouseMove(int x, int y) {
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                        | (VideoMemory[80*y+x] & 0xF000) >> 4
                        | (VideoMemory[80*y+x] & 0x00FF);

    VideoMemory[80*previousY+previousX] = (VideoMemory[80*previousY+previousX] & 0x0F00) << 4
                        | (VideoMemory[80*previousY+previousX] & 0xF000) >> 4
                        | (VideoMemory[80*previousY+previousX] & 0x00FF);
    // Update the previous cursor position
    previousX = x;
    previousY = y;
}

void ConsoleMouseEventHandler::OnLeftMouseDown(int x, int y) {
    printf(WHITE, "Left mouse button pressed at (%d, %d)\n", x, y);
}

void ConsoleMouseEventHandler::OnLeftMouseUp(int x, int y) {
    printf(WHITE, "Left mouse button released at (%d, %d)\n", x, y);
}

void ConsoleMouseEventHandler::OnRightMouseDown(int x, int y) {
    printf(WHITE, "Right mouse button pressed at (%d, %d)\n", x, y);
}

void ConsoleMouseEventHandler::OnRightMouseUp(int x, int y) {
    printf(WHITE, "Right mouse button released at (%d, %d)\n", x, y);
}




extern "C" void kernelMain(void* multiboot_structure, unsigned int magicnumber) {
    clearScreen();
    GlobalDescriptorTable gdt;
    InterruptManager interrupts(&gdt);
    
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
    //printf(WHITE, "Unsigned 8-bit: %u\n", (uint8_t)255);
    //printf(WHITE, "Unsigned 16-bit: %u\n", (uint16_t)65535);
    //printf(WHITE, "Unsigned 32-bit: %u\n", (uint32_t)4294967295);
    //printf(WHITE, "Signed 32-bit: %d\n", -2147483648);

    //for (int i = 0; i < 5; i++) {
    //    printf(LIGHT_GREEN, "Line %d: Example\n", i + 1);
    //}
    while (1);
} 
