#include <core/driver.h>
#include <debug.h>

class SampleMouseDriver : public Driver {
public:
    SampleMouseDriver() {
        SetName("Dynamic Mouse v1.0");
    }

    void Activate() override {
        printf("Dynamic Mouse Driver ACTIVATED via .o file!\n");
    }

    void Deactivate() override {
        printf("Dynamic Mouse Driver Unloaded.\n");
    }
};


DYNAMIC_DRIVER(SampleMouseDriver)