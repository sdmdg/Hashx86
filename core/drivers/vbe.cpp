/**
 * @file        vbe.cpp
 * @brief       VESA BIOS Extensions (VBE) interface for #x86
 *
 * @date        08/02/2025
 * @version     1.0.0-beta
 */

#include <core/drivers/vbe.h>

VESA_BIOS_Extensions::VESA_BIOS_Extensions(uint32_t w, uint32_t h, uint32_t bpp, uint32_t* vram)
    : GraphicsDriver(w, h, bpp, vram) {}

VESA_BIOS_Extensions::~VESA_BIOS_Extensions() {}
