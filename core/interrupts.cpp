/**
 * @file        interrupts.cpp
 * @brief       Interrupts Manager
 * 
 * @date        20/01/2025
 * @version     1.0.0-beta
 */

#include <core/interrupts.h>

static uint16_t HWInterruptOffset = 0x20;

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
InterruptManager* InterruptManager::activeInstance = 0;

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

InterruptManager::InterruptManager(ProcessManager* processManager, VESA_BIOS_Extensions* vbe, Paging* pager)
: picMasterCommand(0x20),
  picMasterData(0x21),
  picSlaveCommand(0xA0),
  picSlaveData(0xA1)
{
    uint16_t CodeSegment = KERNEL_CODE_SELECTOR;
    this->processManager = processManager;
    this->vbe = vbe;
    this->pager = pager;
    const uint8_t IDT_INTERRUPT_GATE = 0xE;

    for (uint16_t i = 0; i < 256; i++){
        SetInterruptDescriptorTableEntry(i, CodeSegment, &IgnoreInterruptRequest, 0, IDT_INTERRUPT_GATE);
        handlers[i] = 0;
    };

    SetInterruptDescriptorTableEntry(0x00, CodeSegment, &HandleException0x00, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x01, CodeSegment, &HandleException0x01, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x02, CodeSegment, &HandleException0x02, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x03, CodeSegment, &HandleException0x03, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x04, CodeSegment, &HandleException0x04, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x05, CodeSegment, &HandleException0x05, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x06, CodeSegment, &HandleException0x06, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x07, CodeSegment, &HandleException0x07, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x08, CodeSegment, &HandleException0x08, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x09, CodeSegment, &HandleException0x09, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0A, CodeSegment, &HandleException0x0A, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0B, CodeSegment, &HandleException0x0B, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0C, CodeSegment, &HandleException0x0C, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0D, CodeSegment, &HandleException0x0D, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0E, CodeSegment, &HandleException0x0E, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0F, CodeSegment, &HandleException0x0F, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x10, CodeSegment, &HandleException0x10, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x11, CodeSegment, &HandleException0x11, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x12, CodeSegment, &HandleException0x12, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x13, CodeSegment, &HandleException0x13, 0, IDT_INTERRUPT_GATE);



    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x00, CodeSegment, &HandleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x01, CodeSegment, &HandleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x02, CodeSegment, &HandleInterruptRequest0x02, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x03, CodeSegment, &HandleInterruptRequest0x03, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x04, CodeSegment, &HandleInterruptRequest0x04, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x05, CodeSegment, &HandleInterruptRequest0x05, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x06, CodeSegment, &HandleInterruptRequest0x06, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x07, CodeSegment, &HandleInterruptRequest0x07, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x08, CodeSegment, &HandleInterruptRequest0x08, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x09, CodeSegment, &HandleInterruptRequest0x09, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x0A, CodeSegment, &HandleInterruptRequest0x0A, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x0B, CodeSegment, &HandleInterruptRequest0x0B, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x0C, CodeSegment, &HandleInterruptRequest0x0C, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x0D, CodeSegment, &HandleInterruptRequest0x0D, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x0E, CodeSegment, &HandleInterruptRequest0x0E, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x0F, CodeSegment, &HandleInterruptRequest0x0F, 0, IDT_INTERRUPT_GATE);

    SetInterruptDescriptorTableEntry(0x80, CodeSegment, &HandleInterruptRequest0x80, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x81, CodeSegment, &HandleInterruptRequest0x81, 0, IDT_INTERRUPT_GATE);



    
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
    DEBUG_LOG("Activating InterruptManager.");
    if (activeInstance != 0){
        DEBUG_LOG("An active InterruptManager found.");
        activeInstance->Deactivate();
    }
    activeInstance = this;
    asm("sti");
    DEBUG_LOG("InterruptManager Activated.");
}

void InterruptManager:: Deactivate() {
    if (activeInstance == this){
        DEBUG_LOG("Deactivating InterruptManager.");
        activeInstance = 0;
        asm("cli");
        DEBUG_LOG("InterruptManager Deactivated.");
    }
}

uint32_t InterruptManager::handleInterrupt(uint8_t interruptNumber, uint32_t esp) {
    if (activeInstance != 0){
        return activeInstance->DoHandleInterrupt(interruptNumber, esp);
    } else {
        return esp;
    }
}

uint32_t InterruptManager::handleException(uint8_t interruptNumber, uint32_t esp) {
    if (activeInstance != 0){
        return activeInstance->DohandleException(interruptNumber, esp);
    } else {
        return esp;
    }
}

uint32_t InterruptManager::DoHandleInterrupt(uint8_t interruptNumber, uint32_t esp) {
    CPUState* state = (CPUState*)esp;
    Process* p = ProcessManager::activeInstance->getCurrentProcess();
    asm("cli");

    if(interruptNumber == HWInterruptOffset)
    {
        timerTicks++;
        esp = (uint32_t)processManager->Schedule((CPUState*)esp);
    }

    if (interruptNumber >= 256) {
        printf("INVALID INTERRUPT: 0x%x\n", interruptNumber);
        return esp;
    }

    if (handlers[interruptNumber] != 0) {
        //printf("Handling interrupt: 0x%x, Handler address: 0x%x\n", interruptNumber, handlers[interruptNumber]);
        esp = handlers[interruptNumber]->HandleInterrupt(esp);
    }

    else if(0x20 != interruptNumber)
        printf("UNHANDLED INTERRUPT: 0x%x, Handler address: 0x%x\n", interruptNumber, handlers[interruptNumber]);


    if(HWInterruptOffset <= interruptNumber && interruptNumber < HWInterruptOffset + 16)
    {
        picMasterCommand.Write(0x20);
        if(HWInterruptOffset + 8 <= interruptNumber)
            picSlaveCommand.Write(0x20);
    }

    asm("sti");
    return esp;
}

uint32_t InterruptManager::DohandleException(uint8_t interruptNumber, uint32_t esp) {
    CPUState* state = (CPUState*)esp;
    Deactivate();
    this->pager->switch_page_directory(this->pager->KernelPageDirectory);
    this->pager->Deactivate();
    vbe->VBE_font = FontManager::activeInstance->getNewFont();
    uint32_t faulting_addr;
    asm volatile("mov %%cr2, %0" : "=r"(faulting_addr));
    DEBUG_LOG("Page fault at address: 0x%x", faulting_addr);

    
    // Print exception details
    DEBUG_LOG("Exception 0x%x occurred. Error Code: 0x%x", interruptNumber, state->error);
    DEBUG_LOG("EIP: 0x%x, CS: 0x%x, EFLAGS: 0x%x", state->eip, state->cs, state->eflags);


    // PANIC
        vbe->FillRectangle(0,0,1152, 864, 0x0);
        vbe->DrawBitmap(100,200,(const uint32_t*)icon_blue_screen_face_200x200,200,200);
        vbe->VBE_font->setSize(XLARGE);
        vbe->DrawString(120,400,"Your PC ran into a problem and needs to restart.\nWe'll restart it for you.",0xFFFFFFFF);
        vbe->VBE_font->setSize(MEDIUM);
        vbe->DrawString(120,600,"Stop code : 0x",0xFFFFFFFF);
        itoa(Buffer, 16, interruptNumber);
        vbe->DrawString(120 + vbe->VBE_font->getStringLength("Stop code : 0x"), 600, (const char*)Buffer, 0xFFFFFFFF);

        const char* massage;
        switch (interruptNumber)
        {
            case 0x00: massage = "Division By Zero"; break;
            case 0x01: massage = "Debug Exception"; break;
            case 0x02: massage = "Non-Maskable Interrupt"; break;
            case 0x03: massage = "Breakpoint Exception"; break;
            case 0x04: massage = "Overflow Exception"; break;
            case 0x05: massage = "BOUND Range Exceeded"; break;
            case 0x06: massage = "Invalid Opcode"; break;
            case 0x07: massage = "Device Not Available"; break;
            case 0x08: massage = "Double Fault"; break;
            case 0x09: massage = "Coprocessor Segment Overrun"; break;
            case 0x0A: massage = "Invalid TSS"; break;
            case 0x0B: massage = "Segment Not Present"; break;
            case 0x0C: massage = "Stack Segment Fault"; break;
            case 0x0D: massage = "General Protection Fault"; break;
            case 0x0E: massage = "Page Fault"; break;
            case 0x0F: massage = "Reserved (0x0F)"; break;
            case 0x10: massage = "x87 FPU Error"; break;
            case 0x11: massage = "Alignment Check"; break;
            case 0x12: massage = "Machine Check"; break;
            case 0x13: massage = "SIMD Floating Point Exception"; break;
            case 0x14: massage = "Virtualization Exception"; break;
            case 0x15: massage = "Control Protection Exception"; break;
            case 0x16: massage = "Reserved (0x16)"; break;
            case 0x17: massage = "Reserved (0x17)"; break;
            case 0x18: massage = "Reserved (0x18)"; break;
            case 0x19: massage = "Reserved (0x19)"; break;
            case 0x1A: massage = "Reserved (0x1A)"; break;
            case 0x1B: massage = "Reserved (0x1B)"; break;
            case 0x1C: massage = "Reserved (0x1C)"; break;
            case 0x1D: massage = "Reserved (0x1D)"; break;
            case 0x1E: massage = "Security Exception"; break;
            case 0x1F: massage = "Reserved (0x1F)"; break;
            default:
                break;
        }
        vbe->DrawString(120, 620, massage, 0xFFFFFFFF);


        // Show register dump
        int x = 450;
        int y = 540;
        vbe->DrawString(x, y, "Registers:", 0xFFFFFFFF);
        y += 20;

        auto print_reg = [&](const char* name, uint32_t value) {
            char buf[32];
            vbe->DrawString(x, y, name, 0xFFFFFFFF);
            vbe->DrawString(x+60, y, "0x", 0xFFFFFFFF);
            itoa(buf, 16, value);
            vbe->DrawString(x+77, y, buf, 0xFFFFFFFF);
            y += 20;
        };

        print_reg("EAX", state->eax);
        print_reg("EBX", state->ebx);
        print_reg("ECX", state->ecx);
        print_reg("EDX", state->edx);
        print_reg("ESI", state->esi);
        print_reg("EDI", state->edi);
        print_reg("EBP", state->ebp);
        print_reg("ESP", state->esp);
        print_reg("EIP", state->eip);
        print_reg("CS",  state->cs);
        print_reg("SS",  state->ss);
        print_reg("EFLAGS", state->eflags);

        vbe->Flush();
        wait(10000);
    // END OF PANIC

    DEBUG_LOG("Attempting system reboot...\n");
    
    Port8Bit keyboard_command_port(0x64);
    
    // Disable interrupts to prevent interference during the reset sequence
    asm volatile("cli");

    // Wait for the keyboard controller to be ready (input buffer empty)
    // Bit 1 (0x2) of status register (port 0x64) indicates input buffer status.
    // Loop while input buffer is full.
    while ((keyboard_command_port.Read() & 0x02) != 0);

    // Send the "CPU reset" command (0xFE) to the keyboard controller
    keyboard_command_port.Write(0xFE);

    // Halt the CPU
    while (1) {
        asm volatile("hlt");
    }

    return esp;
}