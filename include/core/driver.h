#ifndef DRIVER_H
#define DRIVER_H

#include <core/drivers/ModuleLoader.h>
#include <core/pci.h>
#include <debug.h>
#include <types.h>

class AudioDriver;

/**
 * @brief Base class for hardware drivers.
 *
 * This class provides a generic interface for drivers, including methods
 * for activation, deactivation and resetting. It also holds the name
 * of the driver.
 */
class Driver {
    friend class DriverManager;

public:
    const char* driverName;

    inline Driver() {
        driverName = "Unknown";
    }

    inline virtual ~Driver() {}

    inline virtual void Activate() {}

    inline virtual int Reset() {
        return 0;
    }

    inline virtual void Deactivate() {}

    inline void SetName(const char* name) {
        driverName = name;
    }

    virtual AudioDriver* AsAudioDriver() {
        return 0;
    }
    virtual GraphicsDriver* AsGraphicsDriver() {
        return 0;
    }

protected:
    bool is_Active = false;
};

class DriverManager {
private:
    Driver* drivers[255];  ///< Array to store up to 255 drivers.
    int numDrivers;        ///< Number of currently added drivers.

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

// --- DYNAMIC LINKING EXTENSION ---
typedef Driver* (*GetDriverInstancePtr)();

#define DYNAMIC_DRIVER(ClassName)               \
    extern "C" Driver* CreateDriverInstance() { \
        return new ClassName();                 \
    }

#endif
