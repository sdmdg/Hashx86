/**
 * @file        driver.cpp
 * @brief       Driver class for #x86
 *
 * @date        29/01/2026
 * @version     1.1.0-beta
 */

#define KDBG_COMPONENT "DRIVER.MGR"
#include <core/driver.h>

extern "C" void pci_enable_bus_master(uint16_t vendor, uint16_t device);
extern "C" void pci_find_bar0(uint16_t vendor, uint16_t device);
/**
 * @brief Constructs a DriverManager object.
 *
 * Initializes the driver manager and sets the number of registered drivers to 0.
 * Also prints a message indicating that the manager is loading.
 * Export symbols to load dynamic drivers.
 */
DriverManager::DriverManager() {
    KDBG1("Loading...");
    numDrivers = 0;

    // Export Kernel Symbols
    void (*printf_ptr)(const char*, ...) = printf;
    SymbolTable::Register("printf", (uint32_t)printf_ptr);
    SymbolTable::Register("_Z6printfPKcz", (uint32_t)printf_ptr);

    // Export Memory Functions
    EXPORT_SYMBOL(kmalloc);
    EXPORT_SYMBOL(kfree);

    // Export C++ Operators
    void* (*new_ptr)(size_t) = operator new;
    void (*delete_ptr)(void*) = operator delete;
    void (*delete_sized_ptr)(void*, size_t) = operator delete;
    // Export C++ Array Operators
    void* (*new_array_ptr)(size_t) = operator new[];
    void (*delete_array_ptr)(void*) = operator delete[];

    SymbolTable::Register("_Znaj", (uint32_t)new_array_ptr);
    SymbolTable::Register("_ZdaPv", (uint32_t)delete_array_ptr);
    SymbolTable::Register("_Znwj", (uint32_t)new_ptr);
    SymbolTable::Register("_ZdlPv", (uint32_t)delete_ptr);
    SymbolTable::Register("_ZdlPvj", (uint32_t)delete_sized_ptr);

    SymbolTable::Register("__cxa_pure_virtual", (uint32_t)__cxa_pure_virtual);

    // Export PCI and bus master
    SymbolTable::Register("pci_enable_bus_master", (uint32_t)pci_enable_bus_master);
    SymbolTable::Register("pci_find_bar0", (uint32_t)pci_find_bar0);

    SymbolTable::Register("memcpy", (uint32_t)memcpy);
    SymbolTable::Register("memset", (uint32_t)memset);

    EXPORT_SYMBOL_ASM("_Z7kmalloci");

    // Export InterruptManager
    EXPORT_SYMBOL_ASM("_ZN16InterruptHandlerD2Ev");
    EXPORT_SYMBOL_ASM("_ZN16InterruptManager14activeInstanceE");
    EXPORT_SYMBOL_ASM("_ZN16InterruptHandlerC2EhP16InterruptManager");

    // Export PIC
    EXPORT_SYMBOL_ASM("_ZN41PeripheralComponentInterconnectControllerC1Ev");  // Constructor
    EXPORT_SYMBOL_ASM("_ZN41PeripheralComponentInterconnectControllerD1Ev");  // Destructor
    EXPORT_SYMBOL_ASM("_ZN41PeripheralComponentInterconnectController18FindHardwareDeviceEtt");
    EXPORT_SYMBOL_ASM("_ZN41PeripheralComponentInterconnectController4ReadEtttj");  // Read
    EXPORT_SYMBOL_ASM("_ZN41PeripheralComponentInterconnectController5WriteEtttjj");
    EXPORT_SYMBOL_ASM(
        "_ZN41PeripheralComponentInterconnectController22GetBaseAddressRegisterEtttt");

    // Export GraphicsDriver Methods
    EXPORT_SYMBOL_ASM("_ZN14GraphicsDriverC2EjjjPj");  // Constructor
    EXPORT_SYMBOL_ASM("_ZN14GraphicsDriverD2Ev");      // Destructor
    EXPORT_SYMBOL_ASM("_ZN14GraphicsDriver5FlushEv");
    EXPORT_SYMBOL_ASM(
        "_ZN14GraphicsDriver8PutPixelEiij");  // The specific overload (int, int, uint32)
}

/**
 * @brief Adds a driver to the manager's registry.
 *
 * Stores the provided driver in the internal array and increments the driver count.
 *
 * @param drv Pointer to the driver to be added.
 */
void DriverManager::AddDriver(Driver* drv) {
    drivers[numDrivers] = drv;  // Add the driver to the array.
    numDrivers++;               // Increment the driver count.
}

/**
 * @brief Activates all registered drivers.
 *
 * Iterates through all the drivers stored in the manager, prints their names,
 * activates each driver and logs the success of the operation.
 */
void DriverManager::ActivateAll() {
    for (int i = 0; i < numDrivers; i++) {
        if (!drivers[i]->is_Active) {
            KDBG1("Activating Driver: %s", drivers[i]->driverName);
            drivers[i]->Activate();  // Activate the driver.
            KDBG1("Driver %s: [OK]", drivers[i]->driverName);
        }
    }
}
