#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <types.h>
#include <core/interrupts.h>
#include <core/ports.h>
#include <console.h>
#include <core/driver.h>

// Base class for handling keyboard events
class KeyboardEventHandler
{
public:
    KeyboardEventHandler();
    virtual void OnKeyDown(const char* key);
    virtual void OnKeyUp(const char* key);
};

// Keyboard driver class
class KeyboardDriver : public InterruptHandler,  public Driver
{
    Port8Bit dataPort;
    Port8Bit commandPort;
    KeyboardEventHandler eventHandler; // Changed to a stack-based object

public:
    KeyboardDriver(InterruptManager* manager);
    ~KeyboardDriver();
    void Activate();
    virtual uint32_t HandleInterrupt(uint32_t esp);
};

#endif // KEYBOARD_H
