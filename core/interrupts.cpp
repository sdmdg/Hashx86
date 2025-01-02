#include <core/interrupts.h>

InterruptHandler::InterruptHandler(uint8_t InterruptNumber, InterruptManager* interruptManager)
{
    this->InterruptNumber = InterruptNumber;
    this->interruptManager = interruptManager;
    interruptManager->handlers[InterruptNumber] = this;
}

InterruptHandler::~InterruptHandler()
{
    if(interruptManager->handlers[InterruptNumber] == this)
        interruptManager->handlers[InterruptNumber] = 0;
}

uint32_t InterruptHandler::HandleInterrupt(uint32_t esp)
{
    return esp;
}

InterruptManager::GateDescriptor InterruptManager::interruptDescriptorTable[256];
InterruptManager* InterruptManager::ActiveInterruptManager = 0;

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

InterruptManager::InterruptManager(GlobalDescriptorTable* gdt)
: picMasterCommand(0x20),
  picMasterData(0x21),
  picSlaveCommand(0xA0),
  picSlaveData(0xA1)
{
    PRINT("InterruptManager", "Loading...\n");
    uint16_t CodeSegment = gdt->CodeSegmentSelector();
    const uint8_t IDT_INTERRUPT_GATE = 0xE;

    for (uint16_t i = 0; i < 256; i++){
        SetInterruptDescriptorTableEntry(i, CodeSegment, &IgnoreInterruptRequest, 0, IDT_INTERRUPT_GATE);
        handlers[i] = 0;
    };

    SetInterruptDescriptorTableEntry(0x20, CodeSegment, &HandleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x21, CodeSegment, &HandleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x2C, CodeSegment, &HandleInterruptRequest0x0C, 0, IDT_INTERRUPT_GATE);
    
    picMasterCommand.Write(0x11);
    picSlaveCommand.Write(0x11);

    picMasterData.Write(0x20);
    picSlaveData.Write(0x28);

    picMasterData.Write(0x04);
    picSlaveData.Write(0x02);

    picMasterData.Write(0x01);
    picSlaveData.Write(0x01);

    picMasterData.Write(0x00);
    picSlaveData.Write(0x00);

    // Mask
    //picMasterData.Write(0xFD);
    //picSlaveData.Write(0xFF);

    InterruptDescriptorTablePointer idt;
    idt.size  = 256 * sizeof(GateDescriptor) - 1;
    idt.base  = (uint32_t)interruptDescriptorTable;
    asm volatile("lidt %0" : : "m" (idt));
}

InterruptManager::~InterruptManager() {
}

void InterruptManager:: Activate() {
    PRINT("InterruptManager", "Activating InterruptManager...");
    if (ActiveInterruptManager != 0){
        DEBUG_LOG("An active InterruptManager found.");
        ActiveInterruptManager->Deactivate();
    }
    ActiveInterruptManager = this;
    asm("sti");
    printf(LIGHT_GREEN, " [ OK ]\n\n");
}

void InterruptManager:: Deactivate() {
    if (ActiveInterruptManager == this){
        PRINT("InterruptManager", "Deactivating InterruptManager...");
        ActiveInterruptManager = 0;
        asm("cli");
        printf(LIGHT_GREEN, " [ OK ]\n\n");
    }
}

uint32_t InterruptManager::handleInterrupt(uint8_t interruptNumber, uint32_t esp) {
    if (ActiveInterruptManager != 0){
        return ActiveInterruptManager->DoHandleInterrupt(interruptNumber, esp);
    } else {
        return esp;
    }
}

uint32_t InterruptManager::DoHandleInterrupt(uint8_t interruptNumber, uint32_t esp) {
    if (interruptNumber >= 256) {
        printf(RED, "INVALID INTERRUPT: 0x%x\n", interruptNumber);
        return esp;
    }

    if (handlers[interruptNumber] != 0) {
        //printf(YELLOW, "Handling interrupt: 0x%x, Handler address: 0x%x\n", interruptNumber, handlers[interruptNumber]);
        esp = handlers[interruptNumber]->HandleInterrupt(esp);
    }

    else if(0x20 != interruptNumber)
        printf(RED, "UNHANDLED INTERRUPT: 0x%x, Handler address: 0x%x\n", interruptNumber, handlers[interruptNumber]);


    uint16_t hardwareInterruptOffset = 0x20;
    if(hardwareInterruptOffset <= interruptNumber && interruptNumber < hardwareInterruptOffset+16)
    {
        picMasterCommand.Write(0x20);
        if(hardwareInterruptOffset + 8 <= interruptNumber)
            picSlaveCommand.Write(0x20);
    }


    return esp;
}