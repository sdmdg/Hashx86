#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <core/globals.h>
#include <types.h>
#include <core/ports.h>
#include <core/gdt.h>
#include <core/paging.h>
#include <core/drivers/GraphicsDriver.h>
#include <core/process.h>
#include <debug.h>
#include <gui/config/config.h>
#include <gui/bmp.h>
#include <core/timing.h>
#include <utils/string.h>

/**
 * @brief Forward declaration of the InterruptManager class.
 * 
 * Used to allow InterruptHandler to reference InterruptManager without including its full definition at this point.
 */
class InterruptManager;

/**
 * @class InterruptHandler
 * @brief Base class for handling hardware interrupts.
 * 
 * Provides a common interface for handling interrupts and manages the association with an InterruptManager.
 */
class InterruptHandler {
protected:
    uint8_t InterruptNumber;               ///< The interrupt number associated with this handler.
    InterruptManager* interruptManager;    ///< Pointer to the associated InterruptManager instance.

    /**
     * @brief Constructor for InterruptHandler.
     * 
     * Initializes the handler with an interrupt number and associates it with an InterruptManager.
     * 
     * @param InterruptNumber The interrupt number to handle.
     * @param interruptManager Pointer to the InterruptManager managing this handler.
     */
    InterruptHandler(uint8_t InterruptNumber, InterruptManager* interruptManager);

    /**
     * @brief Destructor for InterruptHandler.
     * 
     * Cleans up resources when an InterruptHandler is destroyed.
     */
    ~InterruptHandler();

public:
    /**
     * @brief Virtual method to handle an interrupt.
     * 
     * Subclasses should override this method to provide specific handling logic.
     * 
     * @param esp The stack pointer at the time of the interrupt.
     * @return The new stack pointer after the interrupt is handled.
     */
    virtual uint32_t HandleInterrupt(uint32_t esp);
};

/**
 * @class InterruptManager
 * @brief Manages the handling of hardware interrupts.
 * 
 * Sets up and manages the Interrupt Descriptor Table (IDT), Programmable Interrupt Controller (PIC),
 * and associated interrupt handlers.
 */
class InterruptManager {
    friend class InterruptHandler; ///< Allows InterruptHandler to access private/protected members of InterruptManager.
public:
    static InterruptManager* activeInstance; ///< Pointer to the currently active InterruptManager instance.
protected:
    InterruptHandler* handlers[256];                 ///< Array of interrupt handlers for each interrupt.
    ProcessManager* processManager;
    GraphicsDriver* vbe;
    Paging* pager;
    Bitmap* panicImg;

    /**
     * @struct GateDescriptor
     * @brief Represents an entry in the Interrupt Descriptor Table (IDT).
     */
    struct GateDescriptor {
        uint16_t handlerAddressLowBits;   ///< Lower 16 bits of the interrupt handler's address.
        uint16_t gdt_codeSegmentSelector; ///< Code segment selector in the Global Descriptor Table (GDT).
        uint8_t reserved;                 ///< Reserved field, must be zero.
        uint8_t access;                   ///< Access flags for the interrupt gate.
        uint16_t handlerAddressHighBits;  ///< Upper 16 bits of the interrupt handler's address.
    } __attribute__((packed));            ///< Prevents padding to ensure correct memory layout.

    static GateDescriptor interruptDescriptorTable[256]; ///< The Interrupt Descriptor Table (IDT).

    /**
     * @struct InterruptDescriptorTablePointer
     * @brief Pointer structure for loading the IDT using the `lidt` instruction.
     */
    struct InterruptDescriptorTablePointer {
        uint16_t size;  ///< Size of the IDT in bytes.
        uint32_t base;  ///< Base address of the IDT.
    } __attribute__((packed)); ///< Prevents padding to ensure correct memory layout.

    /**
     * @brief Sets an entry in the IDT.
     * 
     * Configures a specific interrupt gate in the IDT.
     * 
     * @param InterruptNumber The interrupt number to configure.
     * @param codeSegmentSelectorOffset The code segment selector in the GDT.
     * @param handler Pointer to the interrupt handler function.
     * @param DescriptorPrivilegeLevel The privilege level (0-3) for accessing this interrupt.
     * @param DescriptorType The type of descriptor (e.g., interrupt gate).
     */
    static void SetInterruptDescriptorTableEntry(
        uint8_t InterruptNumber,
        uint16_t codeSegmentSelectorOffset,
        void (*handler)(),
        uint8_t DescriptorPrivilegeLevel,
        uint8_t DescriptorType
    );

    // Static methods to handle specific interrupts
    static void IgnoreInterruptRequest();     ///< Handles ignored interrupts.
    static void HandleInterruptRequest0x00();
    static void HandleInterruptRequest0x01();
    static void HandleInterruptRequest0x02();
    static void HandleInterruptRequest0x03();
    static void HandleInterruptRequest0x04();
    static void HandleInterruptRequest0x05();
    static void HandleInterruptRequest0x06();
    static void HandleInterruptRequest0x07();
    static void HandleInterruptRequest0x08();
    static void HandleInterruptRequest0x09();
    static void HandleInterruptRequest0x0A();
    static void HandleInterruptRequest0x0B();
    static void HandleInterruptRequest0x0C();
    static void HandleInterruptRequest0x0D();
    static void HandleInterruptRequest0x0E();
    static void HandleInterruptRequest0x0F();
    static void HandleInterruptRequest0x31();

    static void HandleInterruptRequest0x80();
    static void HandleInterruptRequest0x81();

    static void HandleException0x00();
    static void HandleException0x01();
    static void HandleException0x02();
    static void HandleException0x03();
    static void HandleException0x04();
    static void HandleException0x05();
    static void HandleException0x06();
    static void HandleException0x07();
    static void HandleException0x08();
    static void HandleException0x09();
    static void HandleException0x0A();
    static void HandleException0x0B();
    static void HandleException0x0C();
    static void HandleException0x0D();
    static void HandleException0x0E();
    static void HandleException0x0F();
    static void HandleException0x10();
    static void HandleException0x11();
    static void HandleException0x12();
    static void HandleException0x13();
    //static void HandleInterruptRequest0x10();

    // I/O Ports for the Programmable Interrupt Controller (PIC)
    Port8BitSlow picMasterCommand; ///< Command port for the master PIC.
    Port8BitSlow picMasterData;    ///< Data port for the master PIC.
    Port8BitSlow picSlaveCommand;  ///< Command port for the slave PIC.
    Port8BitSlow picSlaveData;     ///< Data port for the slave PIC.

public:
    /**
     * @brief Constructor for the InterruptManager class.
     * 
     * Sets up the IDT and initializes the PIC.
     * 
     * @param gdt, processManager, vbe Pointers to GlobalDescriptorTable, ProcessManager and GraphicsDriver
     */
    InterruptManager(ProcessManager* processManager, GraphicsDriver* vbe, Paging* pager);

    /**
     * @brief Destructor for the InterruptManager class.
     */
    ~InterruptManager();

    /**
     * @brief Activates the InterruptManager.
     * 
     * Makes this instance the active manager and enables interrupts.
     */
    void Activate();

    /**
     * @brief Deactivates the InterruptManager.
     * 
     * Disables interrupts and clears the active manager.
     */
    void Deactivate();

    /**
     * @brief Static method to handle an interrupt.
     * 
     * Passes the interrupt to the active InterruptManager for processing.
     * 
     * @param interruptNumber The interrupt number that occurred.
     * @param esp The stack pointer at the time of the interrupt.
     * @return The new stack pointer after the interrupt is handled.
     */
    static uint32_t handleInterrupt(uint8_t interruptNumber, uint32_t esp);

    /**
     * @brief Static method to handle an exception.
     * 
     * Passes the exception to the active InterruptManager for processing.
     * 
     * @param interruptNumber The exception number that occurred.
     * @param esp The stack pointer at the time of the exception.
     * @return The new stack pointer after the exception is handled.
     */
    static uint32_t handleException(uint8_t interruptNumber, uint32_t esp);

    /**
     * @brief Handles an interrupt for this manager instance.
     * 
     * Dispatches the interrupt to the appropriate handler.
     * 
     * @param interruptNumber The interrupt number that occurred.
     * @param esp The stack pointer at the time of the interrupt.
     * @return The new stack pointer after the interrupt is handled.
     */
    uint32_t DoHandleInterrupt(uint8_t interruptNumber, uint32_t esp);

    /**
     * @brief Handles an exception for this manager instance.
     * 
     * Dispatches the exception to the appropriate handler.
     * 
     * @param interruptNumber The exception number that occurred.
     * @param esp The stack pointer at the time of the exception.
     * @return The new stack pointer after the exception is handled.
     */
    uint32_t DohandleException(uint8_t interruptNumber, uint32_t esp);

};

#endif