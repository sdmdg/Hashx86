#include <console.h>
#include <core/gdt.h>
#include <core/interrupts.h>
#include <core/drivers/keyboard.h>
#include <core/drivers/mouse.h>
#include <core/driver.h>

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors() {
    for (constructor* i = &start_ctors; i != &end_ctors; i++){
        (*i) ();
    }
}

extern "C" void kernelMain(void* multiboot_structure, unsigned int magicnumber) {
    clearScreen();
    GlobalDescriptorTable gdt;
    InterruptManager interrupts(&gdt);
    
    DEBUG_LOG("Initializing Hardware");
    DriverManager driverManager;

    MouseDriver mouse(&interrupts);
    driverManager.AddDriver(&mouse);
    KeyboardDriver keyboard(&interrupts);
    driverManager.AddDriver(&keyboard);

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
