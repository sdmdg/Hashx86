#ifndef GRAPHICS_DRIVER_H
#define GRAPHICS_DRIVER_H

#include <types.h>
#include <gui/fonts/font.h>
#include <gui/renderer/nina.h>
#include <core/memory.h>

class GraphicsDriver {
protected:
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    NINA nina;
    
    // The Video Memory (Hardware)
    uint32_t* videoMemory;
    
    // Back Buffer
    uint32_t* backBuffer;

    // LUT for Alpha Blending
    uint8_t alphaTable[256][256];

    void PrecomputeAlphaTable();
public:
    GraphicsDriver(uint32_t w, uint32_t h, uint32_t bpp, uint32_t* vram);
    virtual ~GraphicsDriver();

    // The Hardware Interface
    virtual void Flush(); 
    
    // Getters
    uint32_t GetWidth() { return width; }
    uint32_t GetHeight() { return height; }
    uint32_t* GetVideoMemory() { return videoMemory; }
    uint32_t* GetBackBuffer() { return backBuffer; }

    // Drawing Primitives
    virtual void PutPixel(int32_t x, int32_t y, uint32_t color);
    void PutPixel(int32_t x, int32_t y, uint8_t a, uint8_t r, uint8_t g, uint8_t b);

    // Shapes   
    void FillRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t color);
    void DrawRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t color);

    void FillCircle(int32_t cx, int32_t cy, uint32_t r, uint32_t color);
    void DrawCircle(int32_t cx, int32_t cy, uint32_t r, uint32_t color);
    
    void FillRoundedRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t r, uint32_t color);
    void DrawRoundedRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t r, uint32_t color);

    void DrawRoundedRectangleShadow(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t size, uint32_t r, uint32_t color);
    void BlurRoundedRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t r, uint32_t blur);

    void DrawHorizontalLine(int32_t x, int32_t y, int32_t length, uint32_t colorIndex);
    void DrawVerticalLine(int32_t x, int32_t y, int32_t length, uint32_t colorIndex);

    // Text & Bitmaps
    void DrawBitmap(int32_t x, int32_t y, const uint32_t* data, int32_t w, int32_t h);
    void DrawCharacter(int32_t x, int32_t y, char c, Font* font, uint32_t color);
    void DrawString(int32_t x, int32_t y, const char* str, Font* font, uint32_t color);

    // Calculates the X/Y coordinates to center an object of size w/h on the screen
    void GetScreenCenter(uint32_t w, uint32_t h, int32_t& x, int32_t& y);
};

#endif