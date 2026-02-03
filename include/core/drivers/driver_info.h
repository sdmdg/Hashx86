#ifndef DRIVER_INFO_H
#define DRIVER_INFO_H

#include <stdint.h>

#define DRIVER_INFO_MAGIC 0x44525649

// Simple struct for a pair of IDs
struct HardwareID {
    uint16_t vendor_id;
    uint16_t device_id;
};

struct DriverManifest {
    uint32_t magic;
    char name[32];
    char version[16];

    // Allow up to 4 different Device IDs
    HardwareID devices[4];
};

#define DEFINE_DRIVER_INFO(name_str, version_str, ...)                                            \
    extern "C" __attribute__((section(".driver_info"), used)) DriverManifest _driver_metadata = { \
        DRIVER_INFO_MAGIC, name_str, version_str, {__VA_ARGS__}};

#endif
