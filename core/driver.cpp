#include <core/driver.h>
#include <console.h>

Driver::Driver(){
}

Driver::~Driver(){
}

void Driver::Activate(){
}

int Driver::Reset(){
    return 0;
}

void Driver::Deactivate(){
}



DriverManager::DriverManager(){
    PRINT("DriverManager", "Loading...\n");
    numDrivers = 0;
}

void DriverManager::AddDriver(Driver* drv){
    drivers[numDrivers] = drv;
    numDrivers++;
}

void DriverManager::ActivateAll(){
    for (int i = 0; i < numDrivers; i++){
        PRINT("DriverManager", "Starting Driver: %s", drivers[i]->driverName);
        drivers[i]->Activate();
        printf(LIGHT_GREEN, "[ OK ]\n");
    }
    printf(LIGHT_GREEN, "[ All OK ]\n\n");
}