/**
 * @file        driver.cpp
 * @brief       Driver class for #x86
 * 
 * @author      Malaka Gunawardana
 * @date        13/01/2025
 * @version     1.0.0-beta
 */

#include <core/driver.h>

// Implementation of the Driver class

/**
 * @brief Constructs a Driver object.
 * 
 * This constructor initializes the driver. Additional initialization logic
 * can be added in derived classes.
 */
Driver::Driver() {}

/**
 * @brief Destroys the Driver object.
 * 
 * Ensures proper cleanup when a Driver object is destroyed.
 */
Driver::~Driver() {}

/**
 * @brief Activates the driver.
 * 
 * This is a virtual method to be implemented by derived classes. 
 * It is called to start or enable the driver.
 */
void Driver::Activate() {}

/**
 * @brief Resets the driver.
 * 
 * Returns a default status code of `0`. Derived classes can override
 * this method to implement reset functionality.
 * @return int Status code indicating the result of the reset operation.
 */
int Driver::Reset() {
    return 0;
}

/**
 * @brief Deactivates the driver.
 * 
 * This method is virtual and can be implemented by derived classes to
 * handle specific deactivation logic for the driver.
 */
void Driver::Deactivate() {}


/**
 * @brief Constructs a DriverManager object.
 * 
 * Initializes the driver manager and sets the number of registered drivers to 0.
 * Also prints a message indicating that the manager is loading.
 */
DriverManager::DriverManager() {
    PRINT("DriverManager", "Loading...\n");
    numDrivers = 0;
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
        PRINT("DriverManager", "Starting Driver: %s", drivers[i]->driverName); // Log driver name.
        drivers[i]->Activate(); // Activate the driver.
        printf(LIGHT_GREEN, "[ OK ]\n"); // Indicate success for the driver.
    }
    printf(LIGHT_GREEN, "[ All OK ]\n\n"); // Indicate that all drivers were successfully activated.
}
