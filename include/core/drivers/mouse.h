#ifndef MOUSE_H
#define MOUSE_H

#include <types.h>
#include <core/interrupts.h>
#include <core/ports.h>
#include <console.h>
#include <core/driver.h>

// Mouse driver class
class MouseDriver : public InterruptHandler, public Driver
{
    Port8Bit dataPort;
    Port8Bit commandPort;
    uint8_t buffer[3];
    uint8_t offset;
    uint8_t buttons;

public:
    MouseDriver(InterruptManager* manager);
    ~MouseDriver();
    void Activate();
    virtual uint32_t HandleInterrupt(uint32_t esp);
};

#endif // MOUSE_H
