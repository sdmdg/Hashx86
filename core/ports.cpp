/**
 * @file        ports.cpp
 * @brief       Ports class for #x86
 * 
 * @date        13/01/2025
 * @version     1.0.0
 */

#include <core/ports.h>


uint8_t inb(uint16_t portNumber)
{
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(portNumber));
    return result;
}

void outb(uint16_t portNumber, uint8_t value)
{
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(portNumber));
}


Port::Port(uint16_t portNumber)
{
    this->portNumber = portNumber;
}

Port::~Port()
{
}


Port8Bit::Port8Bit(uint16_t portNumber) : Port(portNumber)
{
}

Port8Bit::~Port8Bit() 
{
}

void Port8Bit::Write(uint8_t data)
{
    asm volatile ("outb %0, %1" : : "a" (data), "Nd" (portNumber));  
}

uint8_t Port8Bit::Read()
{
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a" (result) : "Nd" (portNumber)); 
    return result;
}


Port8BitSlow::Port8BitSlow(uint16_t portNumber) : Port8Bit(portNumber)
{
}

Port8BitSlow::~Port8BitSlow() 
{
}

void Port8BitSlow::Write(uint8_t data)
{
    asm volatile ("outb %0, %1\njmp 1f\n1: jmp 1f\n1:" : : "a" (data), "Nd" (portNumber)); 
}


Port16Bit::Port16Bit(uint16_t portNumber) : Port(portNumber)
{
}

Port16Bit::~Port16Bit()
{
}

void Port16Bit::Write(uint16_t data)
{
    asm volatile ("outw %0, %1" : : "a"(data), "Nd"(portNumber)); 
}

uint16_t Port16Bit::Read()
{
    uint16_t result;
    asm volatile ("inw %1, %0" : "=a"(result) : "Nd"(portNumber)); 
    return result;
}


Port32Bit::Port32Bit(uint16_t portNumber) : Port(portNumber)
{
}

Port32Bit::~Port32Bit()
{
}

void Port32Bit::Write(uint32_t data) {
    asm volatile ("outl %0, %1" : : "a"(data), "Nd"(portNumber)); 
}

uint32_t Port32Bit::Read() {
    uint32_t result;
    asm volatile ("inl %1, %0" : "=a"(result) : "Nd"(portNumber)); 
    return result;
}