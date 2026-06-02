/**
 * @file        keyboard.cpp
 * @brief       Generic Keyboard Driver for #x86
 *
 * @date        13/01/2025
 * @version     1.0.0-beta
 */

#define KDBG_COMPONENT "KEYBOARD"
#include <core/drivers/keyboard.h>
#include <core/memory.h>

KeyboardDriver* KeyboardDriver::activeInstance = nullptr;

// Modifier key states
bool leftShiftPressed = false;
bool rightShiftPressed = false;
bool leftCtrlPressed = false;
bool rightCtrlPressed = false;
bool leftAltPressed = false;
bool rightAltPressed = false;
bool capsLockActive = false;

/**
 * KeyboardEventHandler constructor
 */
KeyboardEventHandler::KeyboardEventHandler() {}

/**
 * Virtual method to handle key press events
 */
void KeyboardEventHandler::OnKeyDown(const char* key) {}

/**
 * Virtual method to handle key release events
 */
void KeyboardEventHandler::OnKeyUp(const char* key) {}

/**
 * Virtual method to handle special key press events
 */
void KeyboardEventHandler::OnSpecialKeyDown(uint8_t key) {}

/**
 * Virtual method to handle special key release events
 */
void KeyboardEventHandler::OnSpecialKeyUp(uint8_t key) {}

/**
 * KeyboardDriver constructor
 *
 * Initializes the keyboard driver with the interrupt manager and event handler.
 */
KeyboardDriver::KeyboardDriver(InterruptManager* manager, KeyboardEventHandler* handler)
    : InterruptHandler(0x21, manager), dataPort(0x60), commandPort(0x64) {
    this->eventHandler = handler;
    this->driverName = "Generic Keyboard Driver  ";
    memset(this->keyStates, 0, sizeof(this->keyStates));
    activeInstance = this;
}

/**
 * KeyboardDriver destructor
 */
KeyboardDriver::~KeyboardDriver() {}

/**
 * Activates the keyboard driver and initializes the hardware
 */
void KeyboardDriver::Activate() {
    // Clear the keyboard buffer
    while (commandPort.Read() & 0x1) dataPort.Read();

    // Enable the keyboard
    commandPort.Write(0xAE);

    // Configure keyboard settings
    commandPort.Write(0x20);
    uint8_t status = (dataPort.Read() | 1) & ~0x10;  // Enable IRQ1, disable key lock
    commandPort.Write(0x60);
    dataPort.Write(status);

    // Activate the keyboard
    dataPort.Write(0xF4);
    this->is_Active = true;
}

/**
 * Handles keyboard interrupts and processes key events
 *
 * @param esp Current stack pointer
 * @return Updated stack pointer after handling interrupt
 */
uint32_t KeyboardDriver::HandleInterrupt(uint32_t esp) {
    uint8_t key = dataPort.Read();

    if (this->eventHandler == 0) return esp;

    // KDBG1(WHITE, "\nKey is 0x%x\n", key);
    static bool isExtendedScancode = false;

    if (key == 0xE0) {
        isExtendedScancode = true;  // Mark the next scancode as extended
        return esp;                 // Skip processing for now
    }

    // Key mappings
    const char normalKeyMap[128] = {
        0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0', '-', '=',  0,  // 0x00 - 0x0E
        0,   'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',  '[', ']', '\n', 0,  // 0x0F - 0x1D
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\',     // 0x1E - 0x2C
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,    '*', 0,   ' ',      // 0x2D - 0x39
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,        // 0x3A - 0x48
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0         // 0x49 - 0x58
    };

    const char shiftKeyMap[128] = {
        0,   0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',  0,  // 0x00 - 0x0E
        0,   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,  // 0x0F - 0x1D
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,   '|',      // 0x1E - 0x2C
        'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ',      // 0x2D - 0x39
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,        // 0x3A - 0x48
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0         // 0x49 - 0x58
    };

    if (isExtendedScancode) {
        isExtendedScancode = false;  // Reset the flag
        switch (key) {
            case 0x1D:  // Right Ctrl Pressed
                rightCtrlPressed = true;
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("R Ctrl pressed");
                break;
            case 0x38:  // Right Alt Pressed
                rightAltPressed = true;
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("R Alt pressed");
                break;
            case 0x48:  // Up Arrow
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("Up Arrow pressed");
                break;
            case 0x50:  // Down Arrow
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("Down Arrow pressed");
                break;
            case 0x4B:  // Left Arrow
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("Left Arrow pressed");
                break;
            case 0x4D:  // Right Arrow
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("Right Arrow pressed");
                break;
            case 0x53:  // Delete
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("Delete pressed");
                break;
            case 0x9D:  // Right Ctrl Released
                rightCtrlPressed = false;
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("R Ctrl released");
                break;
            case 0xB8:  // Right Alt Released
                rightAltPressed = false;
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("R Alt released");
                break;
            case 0xC8:  // Up Arrow Released
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("Up Arrow released");
                break;
            case 0xD0:  // Down Arrow Released
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("Down Arrow released");
                break;
            case 0xCB:  // Left Arrow Released
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("Left Arrow released");
                break;
            case 0xCD:  // Right Arrow Released
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("Right Arrow released");
                break;
            case 0xD3:  // Delete Released
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("Delete released");
                break;

            default:
                // KDBG1("Error : Unhandled extended key: 0x%x", key);
                break;
        }
        return esp;
    }

    // Normal scancodes
    // Key down event
    if (key < 0x80) {
        keyStates[key] = 1;  // Track key down
        switch (key) {
            case 0x1C:  // Enter Pressed
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("Enter pressed");
                break;
            case 0x2A:  // Left Shift Pressed
                eventHandler->OnSpecialKeyDown(key);
                leftShiftPressed = true;
                // KDBG1("L Shift pressed");
                break;
            case 0x36:  // Right Shift Pressed
                eventHandler->OnSpecialKeyDown(key);
                rightShiftPressed = true;
                // KDBG1("R Shift pressed");
                break;
            case 0x1D:  // Left Ctrl Pressed
                eventHandler->OnSpecialKeyDown(key);
                leftCtrlPressed = true;
                // KDBG1("L Ctrl pressed");
                break;
            case 0x38:  // Left Alt Pressed
                eventHandler->OnSpecialKeyDown(key);
                leftAltPressed = true;
                // KDBG1("L Alt pressed");
                break;
            case 0x3A:  // Caps Lock Pressed
                eventHandler->OnSpecialKeyDown(key);
                capsLockActive = !capsLockActive;
                // KDBG1(capsLockActive ? "CAP activated" : "CAP deactivated");
                break;
            case 0x0F:  // Tab Pressed
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("Tab pressed");
                break;
            case 0x0E:  // Backspace Pressed
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("Backspace pressed");
                break;

            case 0x01:  // ESC
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("ESC pressed");
                break;
            case 0x3B:  // F1
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("F1 pressed");
                break;
            case 0x3C:  // F2
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("F2 pressed");
                break;
            case 0x3D:  // F3
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("F3 pressed");
                break;
            case 0x3E:  // F4
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("F4 pressed");
                break;
            case 0x3F:  // F5
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("F5 pressed");
                break;
            case 0x40:  // F6
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("F6 pressed");
                break;
            case 0x41:  // F7
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("F7 pressed");
                break;
            case 0x42:  // F8
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("F8 pressed");
                break;
            case 0x43:  // F9
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("F9 pressed");
                break;
            case 0x44:  // F10
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("F10 pressed");
                break;
            case 0x57:  // F11
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("F11 pressed");
                break;
            case 0x58:  // F12
                eventHandler->OnSpecialKeyDown(key);
                // KDBG1("F12 pressed");
                break;

            default:
                if (key < 128) {  // Valid keycode range
                    char character = normalKeyMap[key];
                    if (((leftShiftPressed || rightShiftPressed) && !capsLockActive) ||
                        (!(leftShiftPressed || rightShiftPressed) && capsLockActive &&
                         character >= 'a' && character <= 'z')) {
                        character = shiftKeyMap[key];  // Use shifted map
                    }

                    if (character != 0) {  // Valid character
                        eventHandler->OnKeyDown(&character);
                    }
                }
                break;
        }
        // Key up event
    } else {
        uint8_t releaseScancode = key & 0x7F;
        if (releaseScancode < 128) keyStates[releaseScancode] = 0;  // Track key up
        switch (key) {
            case 0x9C:  // Enter Released
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("Enter released");
                break;
            case 0xAA:  // Left Shift Released
                leftShiftPressed = false;
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("L Shift released");
                break;
            case 0xB6:  // Right Shift Released
                rightShiftPressed = false;
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("R Shift released");
                break;
            case 0x9D:  // Left Ctrl Released
                leftCtrlPressed = false;
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("L Ctrl released");
                break;
            case 0xB8:  // Left Alt Released
                leftAltPressed = false;
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("L Alt released");
                break;
            case 0x8F:  // Tab
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("Tab released");
                break;
            case 0x8E:  // Backspace
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("Backspace released");
                break;

            case 0x81:  // ESC
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("ESC released");
                break;
            case 0xBB:  // F1
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("F1 released");
                break;
            case 0xBC:  // F2
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("F2 released");
                break;
            case 0xBD:  // F3
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("F3 released");
                break;
            case 0xBE:  // F4
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("F4 released");
                break;
            case 0xBF:  // F5
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("F5 released");
                break;
            case 0xC0:  // F6
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("F6 released");
                break;
            case 0xC1:  // F7
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("F7 released");
                break;
            case 0xC2:  // F8
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("F8 released");
                break;
            case 0xC3:  // F9
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("F9 released");
                break;
            case 0xC4:  // F10
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("F10 released");
                break;
            case 0xD7:  // F11
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("F11 released");
                break;
            case 0xD8:  // F12
                eventHandler->OnSpecialKeyUp(key);
                // KDBG1("F12 released");
                break;

            default:
                break;
        }
    }

    return esp;
}
