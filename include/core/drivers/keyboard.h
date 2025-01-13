#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <types.h>
#include <core/interrupts.h>
#include <core/ports.h>
#include <core/driver.h>

/**
 * @brief Base class for handling keyboard events.
 * 
 * Provides virtual methods to handle key press and release events,
 * including special keys.
 */
class KeyboardEventHandler {
public:
    /**
     * @brief Default constructor.
     */
    KeyboardEventHandler();

    /**
     * @brief Handles a key press event.
     * 
     * @param key Pointer to the pressed key character.
     */
    virtual void OnKeyDown(const char* key);

    /**
     * @brief Handles a special key press event.
     * 
     * @param key Scancode of the pressed special key.
     */
    virtual void OnSpecialKeyDown(uint8_t key);

    /**
     * @brief Handles a key release event.
     * 
     * @param key Pointer to the released key character.
     */
    virtual void OnKeyUp(const char* key);

    /**
     * @brief Handles a special key release event.
     * 
     * @param key Scancode of the released special key.
     */
    virtual void OnSpecialKeyUp(uint8_t key);
};

/**
 * @brief Driver class for managing keyboard input.
 * 
 * This class handles keyboard interrupts and integrates with the 
 * interrupt manager to process key events.
 */
class KeyboardDriver : public InterruptHandler, public Driver {
    Port8Bit dataPort;                  ///< Port for reading keyboard data.
    Port8Bit commandPort;               ///< Port for sending commands to the keyboard.
    KeyboardEventHandler* eventHandler; ///< Event handler for keyboard events.

public:
    /**
     * @brief Constructor for the KeyboardDriver.
     * 
     * @param manager Pointer to the interrupt manager.
     * @param handler Pointer to the keyboard event handler.
     */
    KeyboardDriver(InterruptManager* manager, KeyboardEventHandler* handler);

    /**
     * @brief Destructor for the KeyboardDriver.
     */
    ~KeyboardDriver();

    /**
     * @brief Activates the keyboard driver and initializes the hardware.
     */
    void Activate();

    /**
     * @brief Handles keyboard interrupts and processes key events.
     * 
     * @param esp Current stack pointer.
     * @return Updated stack pointer after handling the interrupt.
     */
    virtual uint32_t HandleInterrupt(uint32_t esp);
};

#endif // KEYBOARD_H
