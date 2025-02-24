/**
 * @file        mouse.cpp
 * @brief       Generic Mouse Driver for #x86
 * 
 * @date        19/01/2025
 * @version     1.0.2-beta
 */

#include <core/drivers/mouse.h>

/**
 * MouseEventHandler constructor
 */
MouseEventHandler::MouseEventHandler(){}

/**
 * Virtual method to handle MouseMove events
 */
void MouseEventHandler::OnMouseMove(int dx, int dy){};

/**
 * Virtual method to handle MouseDown events
 */
void MouseEventHandler::OnMouseDown(uint8_t button){}

/**
 * Virtual method to handle MouseUp events
 */
void MouseEventHandler::OnMouseUp(uint8_t button){}


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
    if (!(status & 0x20))
        return esp;

    buffer[offset] = dataPort.Read();
    
    if(eventHandler == 0)
        return esp;
    
    offset = (offset + 1) % 3;

    if(offset == 0)
    {
        if(buffer[1] != 0 || buffer[2] != 0)
        {
            eventHandler->OnMouseMove((int8_t)buffer[1], -((int8_t)buffer[2]));
        }

        for(uint8_t i = 0; i < 3; i++)
        {
            if((buffer[0] & (0x1<<i)) != (buttons & (0x1<<i)))
            {
                if(buttons & (0x1<<i))
                    eventHandler->OnMouseUp(i+1);
                else
                    eventHandler->OnMouseDown(i+1);
            }
        }
        buttons = buffer[0];
    }
    
    return esp;
}
