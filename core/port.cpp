#include "core/port.h"

// Constructor for the Port class, initializing the port number
Port::Port(uint16_t portNumber) {
    this->portNumber = portNumber;  // Set the port number for the instance
}

// Destructor for the Port class
Port::~Port() {
}

// Port8Bit Constructor: Inherits from Port, initializing with a port number
Port8Bit::Port8Bit(uint16_t portNumber) : Port(portNumber) {}

// Destructor for the Port8Bit class
Port8Bit::~Port8Bit() {}

// Method to write a byte of data to the port (8-bit I/O)
void Port8Bit::Write(uint8_t data) {
    // Inline assembly to write the data byte to the port number
    asm volatile ("outb %0, %1" : : "a" (data), "Nd" (portNumber));  
}

// Method to read a byte of data from the port (8-bit I/O)
uint8_t Port8Bit::Read() {
    uint8_t result;
    // Inline assembly to read the data byte from the port number
    asm volatile ("inb %1, %0" : "=a" (result) : "Nd" (portNumber)); 
    return result;
}

// Port8BitSlow Constructor: Inherits from Port8Bit, initializing with a port number
Port8BitSlow::Port8BitSlow(uint16_t portNumber) : Port8Bit(portNumber) {}

// Destructor for the Port8BitSlow class
Port8BitSlow::~Port8BitSlow() {}

// Method to write a byte of data to the port with a delay (8-bit I/O with slow write)
void Port8BitSlow::Write(uint8_t data) {
    // Inline assembly to write the byte to the port and introduce artificial delay
    asm volatile ("outb %0, %1\njmp 1f\n1: jmp 1f\n1:" : : "a" (data), "Nd" (portNumber)); 
    // The delay ensures the write operation completes before proceeding
}

// Port16Bit Constructor: Inherits from Port, initializing with a port number
Port16Bit::Port16Bit(uint16_t portNumber) : Port(portNumber) {}

// Destructor for the Port16Bit class
Port16Bit::~Port16Bit() {}

// Method to write a 16-bit word of data to the port (16-bit I/O)
void Port16Bit::Write(uint16_t data) {
    // Inline assembly to write the 16-bit word to the port number
    asm volatile ("outw %0, %1" : : "a"(data), "Nd"(portNumber)); 
}

// Method to read a 16-bit word of data from the port (16-bit I/O)
uint16_t Port16Bit::Read() {
    uint16_t result;
    // Inline assembly to read the 16-bit word from the port number
    asm volatile ("inw %1, %0" : "=a"(result) : "Nd"(portNumber)); 
    return result;
}

// Port32Bit Constructor: Inherits from Port, initializing with a port number
Port32Bit::Port32Bit(uint16_t portNumber) : Port(portNumber) {}

// Destructor for the Port32Bit class
Port32Bit::~Port32Bit() {}

// Method to write a 32-bit word of data to the port (32-bit I/O)
void Port32Bit::Write(uint32_t data) {
    // Inline assembly to write the 32-bit word to the port number
    asm volatile ("outl %0, %1" : : "a"(data), "Nd"(portNumber)); 
}

// Method to read a 32-bit word of data from the port (32-bit I/O)
uint32_t Port32Bit::Read() {
    uint32_t result;
    // Inline assembly to read the 32-bit word from the port number
    asm volatile ("inl %1, %0" : "=a"(result) : "Nd"(portNumber)); 
    return result;
}
