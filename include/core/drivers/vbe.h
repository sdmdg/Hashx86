#ifndef VBE_H
#define VBE_H

#include <core/drivers/GraphicsDriver.h>

class VESA_BIOS_Extensions : public GraphicsDriver {
public:
    VESA_BIOS_Extensions(uint32_t w, uint32_t h, uint32_t bpp, uint32_t* vram);
    ~VESA_BIOS_Extensions();
};

#endif
