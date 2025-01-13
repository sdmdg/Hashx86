/**
 * @file        interrupts.cpp
 * @brief       Interrupts Manager
 * 
 * @author      Malaka Gunawardana
 * @date        13/01/2025
 * @version     1.0.0
 */

#include <core/interrupts.h>

/**
 * @brief Constructor for the InterruptHandler class.
 * 
 * Registers the handler for a specific interrupt number in the interrupt manager.
 * 
 * @param InterruptNumber  The interrupt number this handler will handle.
 * @param interruptManager Pointer to the interrupt manager managing this handler.
 */
InterruptHandler::InterruptHandler(uint8_t InterruptNumber, InterruptManager* interruptManager)
{
    this->InterruptNumber = InterruptNumber;
    this->interruptManager = interruptManager;
    interruptManager->handlers[InterruptNumber] = this; // Register this handler
}

/**
 * @brief Destructor for the InterruptHandler class.
 * 
 * Removes this handler from the interrupt manager if it is still registered.
 */
InterruptHandler::~InterruptHandler()
{
    if(interruptManager->handlers[InterruptNumber] == this)
        interruptManager->handlers[InterruptNumber] = 0; // Deregister the handler
}

/**
 * @brief Default interrupt handler.
 * 
 * This is a placeholder that simply returns the stack pointer unchanged.
 * 
 * @param esp Stack pointer at the time of the interrupt.
 * @return    Updated stack pointer (unchanged in this case).
 */
uint32_t InterruptHandler::HandleInterrupt(uint32_t esp)
{
    return esp;
}

// Static members of InterruptManager
InterruptManager::GateDescriptor InterruptManager::interruptDescriptorTable[256];
InterruptManager* InterruptManager::ActiveInterruptManager = 0;

/**
 * @brief Sets an entry in the Interrupt Descriptor Table (IDT).
 * 
 * Configures the IDT entry for a specific interrupt number with the given handler.
 * 
 * @param interruptNumber The interrupt number to configure.
 * @param codeSegmentSelectorOffset Code segment selector in the GDT.
 * @param handler Pointer to the interrupt handler function.
 * @param DescriptorPrivilegeLevel Privilege level required to invoke the interrupt.
 * @param DescriptorType Type of the descriptor (interrupt gate, trap gate, etc.).
 */
void InterruptManager::SetInterruptDescriptorTableEntry(
    uint8_t interruptNumber,
    uint16_t codeSegmentSelectorOffset,
    void (*handler)(),
    uint8_t DescriptorPrivilegeLevel,
    uint8_t DescriptorType) 
{
    const uint8_t IDT_DESC_PRESENT = 0x80;

    interruptDescriptorTable[interruptNumber].handlerAddressLowBits = ((uint32_t)handler) & 0xFFFF;
    interruptDescriptorTable[interruptNumber].handlerAddressHighBits = (((uint32_t)handler) >> 16) & 0xFFFF;
    interruptDescriptorTable[interruptNumber].gdt_codeSegmentSelector = codeSegmentSelectorOffset;
    interruptDescriptorTable[interruptNumber].access = IDT_DESC_PRESENT | ((DescriptorPrivilegeLevel & 3) << 5) | DescriptorType;
    interruptDescriptorTable[interruptNumber].reserved = 0;
}

/**
 * @brief Constructor for the InterruptManager class.
 * 
 * Initializes the interrupt manager, sets up the IDT and configures the PIC.
 * 
 * @param gdt Pointer to the global descriptor table.
 */
InterruptManager::InterruptManager(GlobalDescriptorTable* gdt)
: picMasterCommand(0x20),
  picMasterData(0x21),
  picSlaveCommand(0xA0),
  picSlaveData(0xA1)
{
    PRINT("InterruptManager", "Loading...\n");
    uint16_t CodeSegment = gdt->CodeSegmentSelector();
    const uint8_t IDT_INTERRUPT_GATE = 0xE;

    // Initialize the IDT with default handlers
    for (uint16_t i = 0; i < 256; i++) {
        SetInterruptDescriptorTableEntry(i, CodeSegment, &IgnoreInterruptRequest, 0, IDT_INTERRUPT_GATE);
        handlers[i] = 0;
    }

    // Set specific handlers
    SetInterruptDescriptorTableEntry(0x20, CodeSegment, &HandleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x21, CodeSegment, &HandleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x2C, CodeSegment, &HandleInterruptRequest0x0C, 0, IDT_INTERRUPT_GATE);

    // Initialize the PIC
    picMasterCommand.Write(0x11);
    picSlaveCommand.Write(0x11);
    picMasterData.Write(0x20); // Master PIC vector offset
    picSlaveData.Write(0x28);  // Slave PIC vector offset
    picMasterData.Write(0x04); // Tell Master PIC there is a Slave PIC at IRQ2
    picSlaveData.Write(0x02);  // Tell Slave PIC its cascade identity
    picMasterData.Write(0x01); // 8086/88 mode
    picSlaveData.Write(0x01);  // 8086/88 mode
    picMasterData.Write(0x00); // Unmask interrupts
    picSlaveData.Write(0x00);  // Unmask interrupts

    // Load the IDT
    InterruptDescriptorTablePointer idt;
    idt.size = 256 * sizeof(GateDescriptor) - 1;
    idt.base = (uint32_t)interruptDescriptorTable;
    asm volatile("lidt %0" : : "m" (idt));
}

/**
 * @brief Destructor for the InterruptManager class.
 * 
 * Performs cleanup when the interrupt manager is destroyed.
 */
InterruptManager::~InterruptManager() {}

/**
 * @brief Activates the interrupt manager.
 * 
 * Sets this interrupt manager as the active one and enables interrupts.
 */
void InterruptManager::Activate() {
    PRINT("InterruptManager", "Activating InterruptManager...");
    if (ActiveInterruptManager != 0) {
        DEBUG_LOG("An active InterruptManager found.");
        ActiveInterruptManager->Deactivate();
    }
    ActiveInterruptManager = this;
    asm("sti");     // Enable interrupts
    printf(LIGHT_GREEN, " [ OK ]\n\n");
}

/**
 * @brief Deactivates the interrupt manager.
 * 
 * Disables interrupts and removes this manager as the active one.
 */
void InterruptManager::Deactivate() {
    if (ActiveInterruptManager == this) {
        PRINT("InterruptManager", "Deactivating InterruptManager...");
        ActiveInterruptManager = 0;
        asm("cli");  // Disable interrupts
        printf(LIGHT_GREEN, " [ OK ]\n\n");
    }
}

/**
 * @brief Main interrupt handler.
 * 
 * Delegates interrupt handling to the active interrupt manager.
 * 
 * @param interruptNumber The interrupt number that occurred.
 * @param esp The stack pointer at the time of the interrupt.
 * @return    Updated stack pointer.
 */
uint32_t InterruptManager::handleInterrupt(uint8_t interruptNumber, uint32_t esp) {
    if (ActiveInterruptManager != 0) {
        return ActiveInterruptManager->DoHandleInterrupt(interruptNumber, esp);
    } else {
        return esp;
    }
}

/**
 * @brief Handles a specific interrupt.
 * 
 * Calls the appropriate handler if registered, or logs an error if unhandled.
 * 
 * @param interruptNumber The interrupt number to handle.
 * @param esp The stack pointer at the time of the interrupt.
 * @return    Updated stack pointer.
 */
uint32_t InterruptManager::DoHandleInterrupt(uint8_t interruptNumber, uint32_t esp) {
    if (interruptNumber >= 256) {
        printf(RED, "INVALID INTERRUPT: 0x%x\n", interruptNumber);
        return esp;
    }

    if (handlers[interruptNumber] != 0) {
        esp = handlers[interruptNumber]->HandleInterrupt(esp);
    } else if (0x20 != interruptNumber) {
        printf(RED, "UNHANDLED INTERRUPT: 0x%x\n", interruptNumber);
    }

    // Acknowledge the interrupt if it was a hardware interrupt
    uint16_t hardwareInterruptOffset = 0x20;
    if (hardwareInterruptOffset <= interruptNumber && interruptNumber < hardwareInterruptOffset + 16) {
        picMasterCommand.Write(0x20);    // Acknowledge master PIC
        if (hardwareInterruptOffset + 8 <= interruptNumber)
            picSlaveCommand.Write(0x20); // Acknowledge slave PIC
    }

    return esp;
}
