/**
 * @file        interrupts.cpp
 * @brief       Interrupts Manager
 *
 * @date        20/01/2025
 * @version     1.0.0-beta
 */

#include <core/Iguard.h>
#include <core/KernelSymbolResolver.h>
#include <core/filesystem/FAT32.h>
#include <core/interrupts.h>

static uint16_t HWInterruptOffset = 0x20;
uint32_t audioTickCounter = 0;

InterruptHandler::InterruptHandler(uint8_t InterruptNumber, InterruptManager* interruptManager) {
    this->InterruptNumber = InterruptNumber;
    this->interruptManager = interruptManager;
    interruptManager->handlers[InterruptNumber] = this;
}

InterruptHandler::~InterruptHandler() {
    if (interruptManager->handlers[InterruptNumber] == this)
        interruptManager->handlers[InterruptNumber] = 0;
}

uint32_t InterruptHandler::HandleInterrupt(uint32_t esp) {
    return esp;
}

InterruptManager::GateDescriptor InterruptManager::interruptDescriptorTable[256];
InterruptManager* InterruptManager::activeInstance = 0;

void InterruptManager::SetInterruptDescriptorTableEntry(uint8_t interruptNumber,
                                                        uint16_t codeSegmentSelectorOffset,
                                                        void (*handler)(),
                                                        uint8_t DescriptorPrivilegeLevel,
                                                        uint8_t DescriptorType) {
    const uint8_t IDT_DESC_PRESENT = 0x80;

    interruptDescriptorTable[interruptNumber].handlerAddressLowBits = ((uint32_t)handler) & 0xFFFF;
    interruptDescriptorTable[interruptNumber].handlerAddressHighBits =
        (((uint32_t)handler) >> 16) & 0xFFFF;
    interruptDescriptorTable[interruptNumber].gdt_codeSegmentSelector = codeSegmentSelectorOffset;
    interruptDescriptorTable[interruptNumber].access =
        IDT_DESC_PRESENT | ((DescriptorPrivilegeLevel & 3) << 5) | DescriptorType;
    interruptDescriptorTable[interruptNumber].reserved = 0;
}

InterruptManager::InterruptManager(Scheduler* scheduler, Paging* pager)
    : picMasterCommand(0x20), picMasterData(0x21), picSlaveCommand(0xA0), picSlaveData(0xA1) {
    activeInstance = this;
    uint16_t CodeSegment = KERNEL_CODE_SELECTOR;
    this->scheduler = scheduler;
    this->pager = pager;
    const uint8_t IDT_INTERRUPT_GATE = 0xE;

    for (uint16_t i = 0; i < 256; i++) {
        SetInterruptDescriptorTableEntry(i, CodeSegment, &IgnoreInterruptRequest, 0,
                                         IDT_INTERRUPT_GATE);
        handlers[i] = 0;
    };

    SetInterruptDescriptorTableEntry(0x00, CodeSegment, &HandleException0x00, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x01, CodeSegment, &HandleException0x01, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x02, CodeSegment, &HandleException0x02, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x03, CodeSegment, &HandleException0x03, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x04, CodeSegment, &HandleException0x04, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x05, CodeSegment, &HandleException0x05, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x06, CodeSegment, &HandleException0x06, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x07, CodeSegment, &HandleException0x07, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x08, CodeSegment, &HandleException0x08, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x09, CodeSegment, &HandleException0x09, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0A, CodeSegment, &HandleException0x0A, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0B, CodeSegment, &HandleException0x0B, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0C, CodeSegment, &HandleException0x0C, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0D, CodeSegment, &HandleException0x0D, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0E, CodeSegment, &HandleException0x0E, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0F, CodeSegment, &HandleException0x0F, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x10, CodeSegment, &HandleException0x10, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x11, CodeSegment, &HandleException0x11, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x12, CodeSegment, &HandleException0x12, 0,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x13, CodeSegment, &HandleException0x13, 0,
                                     IDT_INTERRUPT_GATE);

    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x00, CodeSegment,
                                     &HandleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x01, CodeSegment,
                                     &HandleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x02, CodeSegment,
                                     &HandleInterruptRequest0x02, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x03, CodeSegment,
                                     &HandleInterruptRequest0x03, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x04, CodeSegment,
                                     &HandleInterruptRequest0x04, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x05, CodeSegment,
                                     &HandleInterruptRequest0x05, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x06, CodeSegment,
                                     &HandleInterruptRequest0x06, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x07, CodeSegment,
                                     &HandleInterruptRequest0x07, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x08, CodeSegment,
                                     &HandleInterruptRequest0x08, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x09, CodeSegment,
                                     &HandleInterruptRequest0x09, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x0A, CodeSegment,
                                     &HandleInterruptRequest0x0A, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x0B, CodeSegment,
                                     &HandleInterruptRequest0x0B, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x0C, CodeSegment,
                                     &HandleInterruptRequest0x0C, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x0D, CodeSegment,
                                     &HandleInterruptRequest0x0D, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x0E, CodeSegment,
                                     &HandleInterruptRequest0x0E, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(HWInterruptOffset + 0x0F, CodeSegment,
                                     &HandleInterruptRequest0x0F, 0, IDT_INTERRUPT_GATE);

    SetInterruptDescriptorTableEntry(0x80, CodeSegment, &HandleInterruptRequest0x80, 3,
                                     IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x81, CodeSegment, &HandleInterruptRequest0x81, 3,
                                     IDT_INTERRUPT_GATE);

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
    // picMasterData.Write(0xFD);
    // picSlaveData.Write(0xFF);

    InterruptDescriptorTablePointer idt;
    idt.size = 256 * sizeof(GateDescriptor) - 1;
    idt.base = (uint32_t)interruptDescriptorTable;
    asm volatile("lidt %0" : : "m"(idt));
}

InterruptManager::~InterruptManager() {}

void InterruptManager::Activate() {
    DEBUG_LOG("Activating InterruptManager.");
    /*     if (activeInstance != 0){
            DEBUG_LOG("An active InterruptManager found.");
            activeInstance->Deactivate();
        } */
    asm("sti");
    DEBUG_LOG("InterruptManager Activated.");
}

void InterruptManager::Deactivate() {
    if (activeInstance == this) {
        DEBUG_LOG("Deactivating InterruptManager.");
        activeInstance = 0;
        asm("cli");
        DEBUG_LOG("InterruptManager Deactivated.");
    }
}

uint32_t InterruptManager::handleInterrupt(uint8_t interruptNumber, uint32_t esp) {
    InterruptGuard guard;
    if (activeInstance != 0) {
        return activeInstance->DoHandleInterrupt(interruptNumber, esp);
    } else {
        return esp;
    }
}

uint32_t InterruptManager::handleException(uint8_t interruptNumber, uint32_t esp) {
    InterruptGuard guard;
    if (activeInstance != 0) {
        return activeInstance->DohandleException(interruptNumber, esp);
    } else {
        return esp;
    }
}

uint32_t InterruptManager::DoHandleInterrupt(uint8_t interruptNumber, uint32_t esp) {
    CPUState* cpu = (CPUState*)esp;
    if (interruptNumber >= HWInterruptOffset && interruptNumber < HWInterruptOffset + 16) {
        picMasterCommand.Write(0x20);  // Send EOI to Master PIC

        if (interruptNumber >= HWInterruptOffset + 8)
            picSlaveCommand.Write(0x20);  // Send EOI to Slave PIC
    }

    // Handle Timer
    if (interruptNumber == HWInterruptOffset) {
        timerTicks++;
    }

    // Call Registered Handlers
    if (handlers[interruptNumber] != nullptr) {
        esp = handlers[interruptNumber]->HandleInterrupt(esp);
    } else if (interruptNumber != HWInterruptOffset && interruptNumber != 0x2E &&
               interruptNumber != 0x2F) {
        printf("UNHANDLED INTERRUPT: 0x%x\n", interruptNumber);
    }

    // After a syscall (int 0x80 arrives as 0xA0 because ASM adds IRQ_BASE=0x20),
    // check if the current thread was terminated
    if (interruptNumber == HWInterruptOffset + 0x80) {
        if (scheduler->currentThread &&
            scheduler->currentThread->state == THREAD_STATE_TERMINATED) {
            return (uint32_t)scheduler->Schedule((CPUState*)esp);
        }
        return esp;
    }

    // Timer Interrupt
    if (interruptNumber == HWInterruptOffset) {
        // Call mixer every 10ms
        audioTickCounter++;
        if (audioTickCounter >= 10) {
            audioTickCounter = 0;
            if (g_systemMixer) g_systemMixer->Update();
        }

        return (uint32_t)scheduler->Schedule((CPUState*)esp);
    }

    // Explicit Yield / Sleep
    if (interruptNumber == 0x2E) {
        return (uint32_t)scheduler->Schedule((CPUState*)esp);
    }

    // No context switch needed, return original stack pointer
    return esp;
}

uint32_t InterruptManager::DohandleException(uint8_t interruptNumber, uint32_t esp) {
    CPUState* state = (CPUState*)esp;
    Deactivate();
    this->pager->SwitchDirectory(this->pager->KernelPageDirectory);
    // this->pager->Deactivate();
    Font* g_systemcGraphicsDriver_font = FontManager::activeInstance->getNewFont();
    uint32_t faulting_addr;
    asm volatile("mov %%cr2, %0" : "=r"(faulting_addr));
    DEBUG_LOG("Page fault at address: 0x%x", faulting_addr);

    // Print exception details
    DEBUG_LOG("Exception 0x%x occurred. Error Code: 0x%x", interruptNumber, state->error);
    DEBUG_LOG("EIP: 0x%x, CS: 0x%x, EFLAGS: 0x%x", state->eip, state->cs, state->eflags);
    DEBUG_LOG("EAX: 0x%x, EBX: 0x%x, ECX: 0x%x, EDX: 0x%x", state->eax, state->ebx, state->ecx,
              state->edx);
    DEBUG_LOG("ESP: 0x%x, EBP: 0x%x, ESI: 0x%x, EDI: 0x%x", state->esp, state->ebp, state->esi,
              state->edi);
    DEBUG_LOG("DS: 0x%x, SS: 0x%x", state->ds, state->ss);

    bool isUserFault = (state->cs & 0x3) == 3;
    if (isUserFault) {
        DEBUG_LOG(">> FAULT IN USER MODE (Ring 3)");
        if (scheduler && scheduler->currentThread) {
            DEBUG_LOG(">> Faulting Thread: TID=%d, PID=%d", scheduler->currentThread->tid,
                      scheduler->currentThread->pid);
        }
    }

    // Kernel stack trace (stops at kernel/user boundary)
    KernelSymbolTable::PrintStackTrace(100);

    // User-mode stack trace: walk EBP chain via physical address translation
    if (isUserFault && scheduler && scheduler->currentThread && scheduler->currentThread->parent) {
        uint32_t* userPD = scheduler->currentThread->parent->page_directory;
        printf("\n[ User Stack Trace (EBP chain) ]\n");
        printf(" 0x%x  <-- faulting EIP\n", state->eip);

        uint32_t userEBP = state->ebp;
        for (int i = 0; i < 32 && userEBP >= 0x1000; i++) {
            uint32_t physAddr = pager->GetPhysicalAddress(userPD, userEBP);
            if (!physAddr) {
                printf(" (EBP 0x%x not mapped)\n", userEBP);
                break;
            }

            uint32_t* frame = (uint32_t*)physAddr;
            uint32_t nextEBP = frame[0];  // saved EBP at [EBP+0]
            uint32_t retAddr = frame[1];  // return address at [EBP+4]

            if (retAddr == 0) break;
            printf(" 0x%x\n", retAddr);

            userEBP = nextEBP;
        }
    }

    // PANIC
    g_systemcGraphicsDriver->FillRectangle(0, 0, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT, 0x0);
    char* panicImageName = (char*)"BITMAPS/PANIC.BMP";
    Bitmap* panicImg = new Bitmap(panicImageName);
    if (!panicImg) {
        HALT("CRITICAL: Failed to allocate panic bitmap!\n");
    }
    if (panicImg->IsValid()) {
        g_systemcGraphicsDriver->DrawBitmap(100, 200, panicImg->GetBuffer(), panicImg->GetWidth(),
                                            panicImg->GetHeight());
    }
    delete panicImg;

    g_systemcGraphicsDriver_font->setSize(XLARGE);
    g_systemcGraphicsDriver->DrawString(
        120, 400, "Your PC ran into a problem and needs to restart.\nWe'll restart it for you.",
        g_systemcGraphicsDriver_font, 0xFFFFFFFF);
    g_systemcGraphicsDriver_font->setSize(MEDIUM);
    g_systemcGraphicsDriver->DrawString(120, 600, "Stop code : 0x", g_systemcGraphicsDriver_font,
                                        0xFFFFFFFF);
    itoa(Buffer, 16, interruptNumber);
    g_systemcGraphicsDriver->DrawString(
        120 + g_systemcGraphicsDriver_font->getStringLength("Stop code : 0x"), 600,
        (const char*)Buffer, g_systemcGraphicsDriver_font, 0xFFFFFFFF);

    const char* massage;
    switch (interruptNumber) {
        case 0x00:
            massage = "Division By Zero";
            break;
        case 0x01:
            massage = "Debug Exception";
            break;
        case 0x02:
            massage = "Non-Maskable Interrupt";
            break;
        case 0x03:
            massage = "Breakpoint Exception";
            break;
        case 0x04:
            massage = "Overflow Exception";
            break;
        case 0x05:
            massage = "BOUND Range Exceeded";
            break;
        case 0x06:
            massage = "Invalid Opcode";
            break;
        case 0x07:
            massage = "Device Not Available";
            break;
        case 0x08:
            massage = "Double Fault";
            break;
        case 0x09:
            massage = "Coprocessor Segment Overrun";
            break;
        case 0x0A:
            massage = "Invalid TSS";
            break;
        case 0x0B:
            massage = "Segment Not Present";
            break;
        case 0x0C:
            massage = "Stack Segment Fault";
            break;
        case 0x0D:
            massage = "General Protection Fault";
            break;
        case 0x0E:
            massage = "Page Fault";
            break;
        case 0x0F:
            massage = "Reserved (0x0F)";
            break;
        case 0x10:
            massage = "x87 FPU Error";
            break;
        case 0x11:
            massage = "Alignment Check";
            break;
        case 0x12:
            massage = "Machine Check";
            break;
        case 0x13:
            massage = "SIMD Floating Point Exception";
            break;
        case 0x14:
            massage = "Virtualization Exception";
            break;
        case 0x15:
            massage = "Control Protection Exception";
            break;
        case 0x16:
            massage = "Reserved (0x16)";
            break;
        case 0x17:
            massage = "Reserved (0x17)";
            break;
        case 0x18:
            massage = "Reserved (0x18)";
            break;
        case 0x19:
            massage = "Reserved (0x19)";
            break;
        case 0x1A:
            massage = "Reserved (0x1A)";
            break;
        case 0x1B:
            massage = "Reserved (0x1B)";
            break;
        case 0x1C:
            massage = "Reserved (0x1C)";
            break;
        case 0x1D:
            massage = "Reserved (0x1D)";
            break;
        case 0x1E:
            massage = "Security Exception";
            break;
        case 0x1F:
            massage = "Reserved (0x1F)";
            break;
        default:
            break;
    }
    g_systemcGraphicsDriver->DrawString(120, 620, massage, g_systemcGraphicsDriver_font,
                                        0xFFFFFFFF);

    // Show register dump
    int x = 450;
    int y = 540;
    g_systemcGraphicsDriver->DrawString(x, y, "Registers:", g_systemcGraphicsDriver_font,
                                        0xFFFFFFFF);
    y += 20;

    auto print_reg = [&](const char* name, uint32_t value) {
        char buf[32];
        g_systemcGraphicsDriver->DrawString(x, y, name, g_systemcGraphicsDriver_font, 0xFFFFFFFF);
        g_systemcGraphicsDriver->DrawString(x + 60, y, "0x", g_systemcGraphicsDriver_font,
                                            0xFFFFFFFF);
        itoa(buf, 16, value);
        g_systemcGraphicsDriver->DrawString(x + 77, y, buf, g_systemcGraphicsDriver_font,
                                            0xFFFFFFFF);
        y += 20;
    };

    print_reg("EAX", state->eax);
    print_reg("EBX", state->ebx);
    print_reg("ECX", state->ecx);
    print_reg("EDX", state->edx);
    print_reg("ESI", state->esi);
    print_reg("EDI", state->edi);
    print_reg("EBP", state->ebp);
    print_reg("EIP", state->eip);
    print_reg("CS", state->cs);
    print_reg("EFLAGS", state->eflags);

    g_systemcGraphicsDriver->Flush();
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
