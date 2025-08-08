#ifndef KERNEL_H
#define KERNEL_H

#include <core/globals.h>
#include <debug.h>
#include <stdint.h>
#include <stdlib.h>
#include <core/gdt.h>
#include <core/tss.h>
#include <core/interrupts.h>
#include <core/syscalls.h>
#include <gui/Hgui.h>
#include <core/elf.h>
#include <core/drivers/keyboard.h>
#include <core/drivers/mouse.h>
#include <core/drivers/ata.h>
#include <core/driver.h>
#include <core/pci.h>
#include <core/pmm.h>
#include <core/memory.h>
#include <core/paging.h>
#include <gui/renderer/nina.h>
#include <core/drivers/vga.h>
#include <gui/gui.h>
#include <core/multiboot.h>
#include <core/drivers/vbe.h>
#include <core/process.h>
#include <core/timing.h>


// symbols from linker.ld for section addresses
extern uint8_t __kernel_section_start;
extern uint8_t __kernel_section_end;
extern uint8_t __kernel_text_section_start;
extern uint8_t __kernel_text_section_end;
extern uint8_t __kernel_data_section_start;
extern uint8_t __kernel_data_section_end;
extern uint8_t __kernel_rodata_section_start;
extern uint8_t __kernel_rodata_section_end;
extern uint8_t __kernel_bss_section_start;
extern uint8_t __kernel_bss_section_end;

typedef struct {
    struct {
        uint32_t k_start_addr;
        uint32_t k_end_addr;
        uint32_t k_len;
        uint32_t text_start_addr;
        uint32_t text_end_addr;
        uint32_t text_len;
        uint32_t data_start_addr;
        uint32_t data_end_addr;
        uint32_t data_len;
        uint32_t rodata_start_addr;
        uint32_t rodata_end_addr;
        uint32_t rodata_len;
        uint32_t bss_start_addr;
        uint32_t bss_end_addr;
        uint32_t bss_len;
    } kernel;

    struct {
        uint32_t total_memory;
    } system;

    struct {
        uint32_t start_addr;
        uint32_t end_addr;
        uint32_t size;
    } available;
} KERNEL_MEMORY_MAP;

extern KERNEL_MEMORY_MAP g_kmap;

int get_kernel_memory_map(KERNEL_MEMORY_MAP *kmap, MultibootInfo *mboot_info);
void display_kernel_memory_map(KERNEL_MEMORY_MAP *kmap);



/**
 * @typedef constructor
 * @brief Defines a pointer to a function with no arguments and no return value.
 * 
 * This is used to reference global constructors during initialization.
 */
typedef void (*constructor)();

/**
 * @brief External declaration for the start and end of the constructors section.
 * 
 * These symbols are defined by the linker and mark the range of global constructors to call during initialization.
 */
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

/**
 * @brief Calls all global constructors in the range defined by `start_ctors` and `end_ctors`.
 * 
 * This function is called during kernel initialization to ensure all static/global objects are properly constructed.
 */
extern "C" void callConstructors() {
    for (constructor* i = &start_ctors; i != &end_ctors; i++) {
        (*i) (); // Call each constructor in the range.
    }
}

/**
 * @class ConsoleKeyboardEventHandler
 * @brief Handles keyboard events and processes them for the console.
 * 
 * A derived class from `KeyboardEventHandler` that implements custom event handling
 * for key presses and releases specific to the console.
 */
class ConsoleKeyboardEventHandler : public KeyboardEventHandler {
    /**
     * @brief Called when a key is pressed down.
     * 
     * @param key The ASCII representation of the key pressed.
     */
    void OnKeyDown(const char* key);

    /**
     * @brief Called when a key is released.
     * 
     * @param key The ASCII representation of the key released.
     */
    void OnKeyUp(const char* key);

    /**
     * @brief Called when a special key (e.g., function keys, arrow keys) is pressed down.
     * 
     * @param key The key code of the special key pressed.
     */
    void OnSpecialKeyDown(uint8_t key);

    /**
     * @brief Called when a special key is released.
     * 
     * @param key The key code of the special key released.
     */
    void OnSpecialKeyUp(uint8_t key);
};

/**
 * @class ConsoleMouseEventHandler
 * @brief Handles mouse events and processes them for the console.
 * 
 * A derived class from `MouseEventHandler` that implements custom event handling
 * for mouse movement and button actions specific to the console.
 */
class ConsoleMouseEventHandler : public MouseEventHandler {
public:
    /**
     * @brief Called when the mouse moves.
     * 
     * @param x The new x-coordinate of the mouse pointer.
     * @param y The new y-coordinate of the mouse pointer.
     */
    virtual void OnMouseMove(int x, int y);

    /**
     * @brief Called when the left mouse button is pressed.
     * 
     * @param x The x-coordinate of the mouse pointer at the time of the event.
     * @param y The y-coordinate of the mouse pointer at the time of the event.
     */
    virtual void OnLeftMouseDown(int x, int y);

    /**
     * @brief Called when the left mouse button is released.
     * 
     * @param x The x-coordinate of the mouse pointer at the time of the event.
     * @param y The y-coordinate of the mouse pointer at the time of the event.
     */
    virtual void OnLeftMouseUp(int x, int y);

    /**
     * @brief Called when the right mouse button is pressed.
     * 
     * @param x The x-coordinate of the mouse pointer at the time of the event.
     * @param y The y-coordinate of the mouse pointer at the time of the event.
     */
    virtual void OnRightMouseDown(int x, int y);

    /**
     * @brief Called when the right mouse button is released.
     * 
     * @param x The x-coordinate of the mouse pointer at the time of the event.
     * @param y The y-coordinate of the mouse pointer at the time of the event.
     */
    virtual void OnRightMouseUp(int x, int y);

private:
    int X = 80; ///< Previous x-coordinate of the mouse pointer.
    int Y = 24; ///< Previous y-coordinate of the mouse pointer.
};


#endif // KERNEL_H
