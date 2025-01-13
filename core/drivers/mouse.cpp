/**
 * @file        mouse.cpp
 * @brief       Generic Mouse Driver for #x86
 * 
 * @author      Malaka Gunawardana
 * @date        13/01/2025
 * @version     1.0.0-beta
 */

#include <core/drivers/mouse.h>

/**
 * @brief Constructs the MouseDriver object.
 * 
 * @param manager Pointer to the interrupt manager.
 * @param handler Pointer to the mouse event handler.
 */
MouseDriver::MouseDriver(InterruptManager* manager, MouseEventHandler* handler)
    : InterruptHandler(0x2C, manager), // IRQ12 for mouse
      dataPort(0x60),
      commandPort(0x64) {
    this->eventHandler = handler;
    this->driverName = "Generic Mouse Driver     ";
    this->offset = 0;
    this->buttons = 0;
}

/**
 * @brief Destructor for the MouseDriver.
 */
MouseDriver::~MouseDriver() {}

/**
 * @brief Activates the mouse driver and initializes the mouse hardware.
 */
void MouseDriver::Activate() {
    // Access video memory to demonstrate activation (optional)
    unsigned short* VideoMemory = (unsigned short*)VIDEO_MEMORY_ADDRESS;
    VideoMemory[80 * 12 + 40] = ((VideoMemory[80 * 12 + 40] & 0xF000) >> 4)
                              | ((VideoMemory[80 * 12 + 40] & 0x0F00) << 4)
                              | ((VideoMemory[80 * 12 + 40] & 0x00FF));

    // Enable the mouse
    commandPort.Write(0xAB); // Enable the auxiliary device (mouse)

    // Set mouse configuration
    commandPort.Write(0x20); // Request current configuration byte
    uint8_t status = dataPort.Read() | 2; // Enable IRQ12 (mouse interrupts)
    commandPort.Write(0x60); // Set configuration byte
    dataPort.Write(status);

    // Enable mouse device
    commandPort.Write(0xD4); // Signal the mouse device
    dataPort.Write(0xF4);    // Enable packet streaming
    dataPort.Read();         // Acknowledge response
}

/**
 * @brief Handles mouse interrupts and processes mouse events.
 * 
 * @param esp Current stack pointer.
 * @return Updated stack pointer after handling the interrupt.
 */
uint32_t MouseDriver::HandleInterrupt(uint32_t esp) {
    uint8_t status = commandPort.Read();

    // Check if mouse data is available and an event handler is set
    if (!(status & 0x20) || eventHandler == nullptr)
        return esp;

    // Read mouse packet data into buffer
    buffer[offset] = dataPort.Read();
    offset = (offset + 1) % 3;

    // Process the packet when all 3 bytes are received
    if (offset == 0) {
        int8_t dx = buffer[1];       // Mouse movement in X direction
        int8_t dy = buffer[2];       // Mouse movement in Y direction (no scroll assumed)
        uint8_t currentButtons = buffer[0]; // Current button states

        // Handle mouse movement
        if (dx != 0 || dy != 0) {
            x += dx;
            y -= dy; // Invert Y-axis to match screen coordinates

            // Clamp cursor position within screen bounds
            x = (x < 0) ? 0 : (x >= 80 ? 79 : x);
            y = (y < 0) ? 0 : (y >= 25 ? 24 : y);

            eventHandler->OnMouseMove(x, y);
        }

        // Handle left mouse button events
        if ((currentButtons & 0x01) && !(buttons & 0x01)) {
            eventHandler->OnLeftMouseDown(x, y);
        } else if (!(currentButtons & 0x01) && (buttons & 0x01)) {
            eventHandler->OnLeftMouseUp(x, y);
        }

        // Handle right mouse button events
        if ((currentButtons & 0x02) && !(buttons & 0x02)) {
            eventHandler->OnRightMouseDown(x, y);
        } else if (!(currentButtons & 0x02) && (buttons & 0x02)) {
            eventHandler->OnRightMouseUp(x, y);
        }

        // Update the button state for the next interrupt
        buttons = currentButtons;
    }

    return esp;
}
