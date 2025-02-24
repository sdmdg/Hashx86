#ifndef KERNEL_H
#define KERNEL_H

#include <debug.h>
#include <stdint.h>
#include <stdlib.h>
#include <console.h>
#include <core/gdt.h>
#include <core/interrupts.h>
#include <core/drivers/keyboard.h>
#include <core/drivers/mouse.h>
#include <core/driver.h>
#include <core/pci.h>
#include <core/memory.h>
#include <core/drivers/vga.h>
#include <gui/gui.h>
#include <core/multiboot.h>
#include <core/drivers/vbe.h>


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
