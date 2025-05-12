#ifndef PORT_H
#define PORT_H

#include <types.h>  // Include for type definitions like uint16_t, uint8_t, etc.

/**
 * @brief Functions representing a generic I/O. For direct acces without port constructor.
 * @param portNumber The I/O port number.
 * @return Data.
 */
uint8_t inb(uint16_t portNumber);
/**
 * @brief Functions representing a generic I/O. For direct acces without port constructor.
 * @param portNumber The I/O port number.
 * @param value Data.
 */
void outb(uint16_t portNumber, uint8_t value);


/**
 * Base class representing a generic I/O port.
 * Provides the foundation for accessing hardware ports at different data widths.
 */
class Port {
protected:
    uint16_t portNumber;  // The port number associated with the I/O operation

    /**
     * Protected constructor to initialize the port number.
     * This class is abstract and not intended for direct instantiation.
     * @param portNumber The I/O port number.
     */
    Port(uint16_t portNumber);

    /**
     * Protected virtual destructor.
     * Ensures proper cleanup for derived classes.
     */
    ~Port();
};

/**
 * Class representing an 8-bit I/O port.
 * Allows for reading and writing one byte (8 bits) of data.
 */
class Port8Bit : public Port {
public:
    /**
     * Constructor to initialize an 8-bit port with the given port number.
     * @param portNumber The I/O port number.
     */
    Port8Bit(uint16_t portNumber);

    /**
     * Destructor for the 8-bit port class.
     */
    ~Port8Bit();

    /**
     * Write a byte (8 bits) of data to the port.
     * @param data The data to write.
     */
    virtual void Write(uint8_t data);

    /**
     * Read a byte (8 bits) of data from the port.
     * @return The data read from the port.
     */
    virtual uint8_t Read();
};

/**
 * Class representing an 8-bit I/O port with a slower write operation.
 * Introduces an artificial delay to account for slower hardware requirements.
 */
class Port8BitSlow : public Port8Bit {
public:
    /**
     * Constructor to initialize a slow 8-bit port with the given port number.
     * @param portNumber The I/O port number.
     */
    Port8BitSlow(uint16_t portNumber);

    /**
     * Destructor for the slow 8-bit port class.
     */
    ~Port8BitSlow();

    /**
     * Write a byte (8 bits) of data to the port with a delay.
     * @param data The data to write.
     */
    virtual void Write(uint8_t data);
};

/**
 * Class representing a 16-bit I/O port.
 * Allows for reading and writing two bytes (16 bits) of data.
 */
class Port16Bit : public Port {
public:
    /**
     * Constructor to initialize a 16-bit port with the given port number.
     * @param portNumber The I/O port number.
     */
    Port16Bit(uint16_t portNumber);

    /**
     * Destructor for the 16-bit port class.
     */
    ~Port16Bit();

    /**
     * Write a 16-bit word of data to the port.
     * @param data The data to write.
     */
    virtual void Write(uint16_t data);

    /**
     * Read a 16-bit word of data from the port.
     * @return The data read from the port.
     */
    virtual uint16_t Read();
};

/**
 * Class representing a 32-bit I/O port.
 * Allows for reading and writing four bytes (32 bits) of data.
 */
class Port32Bit : public Port {
public:
    /**
     * Constructor to initialize a 32-bit port with the given port number.
     * @param portNumber The I/O port number.
     */
    Port32Bit(uint16_t portNumber);

    /**
     * Destructor for the 32-bit port class.
     */
    ~Port32Bit();

    /**
     * Write a 32-bit word of data to the port.
     * @param data The data to write.
     */
    virtual void Write(uint32_t data);

    /**
     * Read a 32-bit word of data from the port.
     * @return The data read from the port.
     */
    virtual uint32_t Read();
};

#endif  // PORT_H
