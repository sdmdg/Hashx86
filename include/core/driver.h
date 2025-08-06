#ifndef DRIVER_H
#define DRIVER_H

#include <debug.h>


/**
 * @brief Base class for hardware drivers.
 * 
 * This class provides a generic interface for drivers, including methods
 * for activation, deactivation and resetting. It also holds the name
 * of the driver.
 */
class Driver {
public:
    const char* driverName; ///< Name of the driver (up to 25 characters).

    /**
     * @brief Constructs a Driver object.
     */
    Driver();

    /**
     * @brief Destroys the Driver object.
     */
    ~Driver();

    /**
     * @brief Activates the driver.
     * 
     * This method can be overridden to implement specific driver activation logic.
     */
    virtual void Activate();

    /**
     * @brief Resets the driver.
     * 
     * This method can be overridden to reset the driver to its initial state.
     * @return An integer status code.
     */
    virtual int Reset();

    /**
     * @brief Deactivates the driver.
     * 
     * This method can be overridden to implement specific driver deactivation logic.
     */
    virtual void Deactivate();
};

/**
 * @brief Manages a collection of drivers.
 * 
 * This class allows for adding drivers, keeping track of them and activating all drivers.
 */
class DriverManager {
private:
    Driver* drivers[255]; ///< Array to store up to 255 drivers.
    int numDrivers;       ///< Number of currently added drivers.

public:
    /**
     * @brief Constructs a DriverManager object.
     */
    DriverManager();

    /**
     * @brief Adds a driver to the manager.
     * @param driver Pointer to the driver to be added.
     */
    void AddDriver(Driver* driver);

    /**
     * @brief Activates all registered drivers.
     */
    void ActivateAll();
};

#endif // DRIVER_H
