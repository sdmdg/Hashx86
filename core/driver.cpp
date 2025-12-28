/**
 * @file        driver.cpp
 * @brief       Driver class for #x86
 * 
 * @date        13/01/2025
 * @version     1.0.0-beta
 */

#include <core/driver.h>

/**
 * @brief Constructs a DriverManager object.
 * 
 * Initializes the driver manager and sets the number of registered drivers to 0.
 * Also prints a message indicating that the manager is loading.
 * Export symbols to load dynamic drivers.
 */
DriverManager::DriverManager() {
    PRINT("DriverManager", "Loading...\n");
    numDrivers = 0;

    
    // Export Kernel Symbols
    void (*printf_ptr)(const char*, ...) = printf;
    SymbolTable::Register("printf", (uint32_t)printf_ptr);
    SymbolTable::Register("_Z6printfPKcz", (uint32_t)printf_ptr);

    // Export Memory Functions
    EXPORT_SYMBOL(kmalloc);
    EXPORT_SYMBOL(kfree);
    
    // Export C++ Operators (Fixes _Znwj, _ZdlPvj)
    void* (*new_ptr)(size_t) = operator new;
    void (*delete_ptr)(void*) = operator delete;
    void (*delete_sized_ptr)(void*, size_t) = operator delete;
    SymbolTable::Register("_Znwj", (uint32_t)new_ptr);
    SymbolTable::Register("_ZdlPv", (uint32_t)delete_ptr);
    SymbolTable::Register("_ZdlPvj", (uint32_t)delete_sized_ptr);

    // Export PIC
    EXPORT_SYMBOL_ASM("_ZN41PeripheralComponentInterconnectControllerC1Ev"); // Constructor
    EXPORT_SYMBOL_ASM("_ZN41PeripheralComponentInterconnectControllerD1Ev"); // Destructor
    EXPORT_SYMBOL_ASM("_ZN41PeripheralComponentInterconnectController18FindHardwareDeviceEtt");
    EXPORT_SYMBOL_ASM("_ZN41PeripheralComponentInterconnectController4ReadEtttj"); // Read
    EXPORT_SYMBOL_ASM("_ZN41PeripheralComponentInterconnectController5WriteEtttjj");
    EXPORT_SYMBOL_ASM("_ZN41PeripheralComponentInterconnectController22GetBaseAddressRegisterEtttt");

    // Export GraphicsDriver Methods
    EXPORT_SYMBOL_ASM("_ZN14GraphicsDriverC2EjjjPj"); // Constructor
    EXPORT_SYMBOL_ASM("_ZN14GraphicsDriverD2Ev");     // Destructor
    EXPORT_SYMBOL_ASM("_ZN14GraphicsDriver5FlushEv");
    EXPORT_SYMBOL_ASM("_ZN14GraphicsDriver8PutPixelEiij"); // The specific overload (int, int, uint32)

}

/**
 * @brief Adds a driver to the manager's registry.
 * 
 * Stores the provided driver in the internal array and increments the driver count.
 * 
 * @param drv Pointer to the driver to be added.
 */
void DriverManager::AddDriver(Driver* drv) {
    drivers[numDrivers] = drv; // Add the driver to the array.
    numDrivers++;              // Increment the driver count.
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
            PRINT("DriverManager", "Starting Driver: %s", drivers[i]->driverName); // Log driver name.
            drivers[i]->Activate(); // Activate the driver.
            printf("[ OK ]\n"); // Indicate success for the driver.
        }
    }
}