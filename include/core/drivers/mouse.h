#ifndef MOUSE_H
#define MOUSE_H

#include <core/driver.h>
#include <core/interrupts.h>
#include <core/ports.h>
#include <types.h>

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

    // virtual void OnScrollUp(int x, int y);
    // virtual void OnScrollDown(int x, int y);
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
    int32_t accumDX;                  ///< Accumulated mouse X delta for polling
    int32_t accumDY;                  ///< Accumulated mouse Y delta for polling

public:
    static MouseDriver* activeInstance;

    /// Get and reset accumulated mouse delta since last poll
    void GetMouseDelta(int32_t& dx, int32_t& dy) {
        dx = accumDX;
        dy = accumDY;
        accumDX = 0;
        accumDY = 0;
    }
    /// Get current button state (bit 0=left, 1=right, 2=middle)
    uint8_t GetButtons() {
        return buttons;
    }
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

#endif  // MOUSE_H
