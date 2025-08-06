#ifndef MOUSE_H
#define MOUSE_H

#include <types.h>
#include <core/interrupts.h>
#include <core/ports.h>
#include <core/driver.h>

/**
 * @brief Base class for handling mouse events.
 * 
 * This class provides a set of virtual methods that can be overridden
 * to handle various mouse events such as movement, button presses and scrolling.
 */
class MouseEventHandler {
public:
    MouseEventHandler();

    /**
     * @brief Called when the mouse is moved.
     * @param x The new X coordinate.
     * @param y The new Y coordinate.
     */
    virtual void OnMouseMove(int dx, int dy);

    /**
     * @brief Called when the left/Right mouse button is pressed.
     * @param button The button at the time of the event.
     */
    virtual void OnMouseDown(uint8_t button);

    /**
     * @brief Called when the left/Right mouse button is released.
     * @param button The button at the time of the event.
     */
    virtual void OnMouseUp(uint8_t button);

    //virtual void OnScrollUp(int x, int y);
    //virtual void OnScrollDown(int x, int y);
};

/**
 * @brief Mouse driver class for handling mouse hardware and events.
 * 
 * This class interfaces with the mouse hardware through ports, processes
 * interrupts and notifies the event handler of mouse events.
 */
class MouseDriver : public InterruptHandler, public Driver {
    Port8Bit dataPort;                ///< Port for reading mouse data.
    Port8Bit commandPort;             ///< Port for sending commands to the mouse.
    MouseEventHandler* eventHandler;  ///< Event handler for mouse events.
    uint8_t buffer[3];                ///< Buffer for storing mouse packet data.
    uint8_t offset;                   ///< Current offset in the buffer.
    uint8_t buttons;                  ///< Current state of mouse buttons.
    int8_t x = 40, y = 12;            ///< Cursor position.

public:
    /**
     * @brief Constructs the MouseDriver object.
     * @param manager Pointer to the interrupt manager.
     * @param handler Pointer to the mouse event handler.
     */
    MouseDriver(InterruptManager* manager, MouseEventHandler* handler);

    /**
     * @brief Destructor for the MouseDriver.
     */
    ~MouseDriver();

    /**
     * @brief Activates the mouse driver and initializes the hardware.
     */
    void Activate();

    /**
     * @brief Handles mouse interrupts and processes mouse packets.
     * @param esp Current stack pointer.
     * @return Updated stack pointer after handling the interrupt.
     */
    virtual uint32_t HandleInterrupt(uint32_t esp);
};

#endif // MOUSE_H
