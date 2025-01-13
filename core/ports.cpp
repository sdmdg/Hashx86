/**
 * @file        ports.cpp
 * @brief       Ports class for #x86
 * 
 * @author      Malaka Gunawardana
 * @date        13/01/2025
 * @version     1.0.0
 */

#include <core/ports.h>

/**
 * @brief Constructor for the Port class.
 * 
 * Initializes the base class with a port number.
 * 
 * @param portNumber The I/O port number to be used by this instance.
 */
Port::Port(uint16_t portNumber) {
    this->portNumber = portNumber;  // Assign the provided port number
}

/**
 * @brief Destructor for the Port class.
 * 
 * Cleans up resources when a Port instance is destroyed.
 */
Port::~Port() {
}

/**
 * @brief Constructor for the Port8Bit class.
 * 
 * Initializes the class with a port number.
 * 
 * @param portNumber The I/O port number for 8-bit operations.
 */
Port8Bit::Port8Bit(uint16_t portNumber) : Port(portNumber) {}

/**
 * @brief Destructor for the Port8Bit class.
 */
Port8Bit::~Port8Bit() {}

/**
 * @brief Writes an 8-bit value to the port.
 * 
 * Sends a single byte to the specified I/O port.
 * 
 * @param data The 8-bit value to write to the port.
 */
void Port8Bit::Write(uint8_t data) {
    asm volatile ("outb %0, %1" : : "a" (data), "Nd" (portNumber));  
}

/**
 * @brief Reads an 8-bit value from the port.
 * 
 * Retrieves a single byte from the specified I/O port.
 * 
 * @return The 8-bit value read from the port.
 */
uint8_t Port8Bit::Read() {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a" (result) : "Nd" (portNumber)); 
    return result;
}

/**
 * @brief Constructor for the Port8BitSlow class.
 * 
 * Initializes the class with a port number.
 * 
 * @param portNumber The I/O port number for slow 8-bit operations.
 */
Port8BitSlow::Port8BitSlow(uint16_t portNumber) : Port8Bit(portNumber) {}

/**
 * @brief Destructor for the Port8BitSlow class.
 */
Port8BitSlow::~Port8BitSlow() {}

/**
 * @brief Writes an 8-bit value to the port with a delay.
 * 
 * Sends a single byte to the specified I/O port with an artificial delay to ensure proper timing.
 * 
 * @param data The 8-bit value to write to the port.
 */
void Port8BitSlow::Write(uint8_t data) {
    asm volatile ("outb %0, %1\njmp 1f\n1: jmp 1f\n1:" : : "a" (data), "Nd" (portNumber)); 
}

/**
 * @brief Constructor for the Port16Bit class.
 * 
 * Initializes the class with a port number.
 * 
 * @param portNumber The I/O port number for 16-bit operations.
 */
Port16Bit::Port16Bit(uint16_t portNumber) : Port(portNumber) {}

/**
 * @brief Destructor for the Port16Bit class.
 */
Port16Bit::~Port16Bit() {}

/**
 * @brief Writes a 16-bit value to the port.
 * 
 * Sends a two-byte word to the specified I/O port.
 * 
 * @param data The 16-bit value to write to the port.
 */
void Port16Bit::Write(uint16_t data) {
    asm volatile ("outw %0, %1" : : "a"(data), "Nd"(portNumber)); 
}

/**
 * @brief Reads a 16-bit value from the port.
 * 
 * Retrieves a two-byte word from the specified I/O port.
 * 
 * @return The 16-bit value read from the port.
 */
uint16_t Port16Bit::Read() {
    uint16_t result;
    asm volatile ("inw %1, %0" : "=a"(result) : "Nd"(portNumber)); 
    return result;
}

/**
 * @brief Constructor for the Port32Bit class.
 * 
 * Initializes the class with a port number.
 * 
 * @param portNumber The I/O port number for 32-bit operations.
 */
Port32Bit::Port32Bit(uint16_t portNumber) : Port(portNumber) {}

/**
 * @brief Destructor for the Port32Bit class.
 */
Port32Bit::~Port32Bit() {}

/**
 * @brief Writes a 32-bit value to the port.
 * 
 * Sends a four-byte word to the specified I/O port.
 * 
 * @param data The 32-bit value to write to the port.
 */
void Port32Bit::Write(uint32_t data) {
    asm volatile ("outl %0, %1" : : "a"(data), "Nd"(portNumber)); 
}

/**
 * @brief Reads a 32-bit value from the port.
 * 
 * Retrieves a four-byte word from the specified I/O port.
 * 
 * @return The 32-bit value read from the port.
 */
uint32_t Port32Bit::Read() {
    uint32_t result;
    asm volatile ("inl %1, %0" : "=a"(result) : "Nd"(portNumber)); 
    return result;
}
