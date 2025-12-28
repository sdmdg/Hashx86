#ifndef NINA_H
#define NINA_H

#include <types.h>
#include <gui/fonts/font.h>
#include <gui/icons.h>
#include <core/memory.h>

class NINA
{
protected:
    uint8_t alphaTable[256][256];
    void PrecomputeAlphaTable();

public:
    NINA();
    ~NINA();
    static NINA* activeInstance;

    void DrawBitmapToBuffer(uint32_t* dst, int dstW, int dstH, int dstX, int dstY, uint32_t* src, int srcW, int srcH);
    

    void DrawBitmap(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x, int32_t y, const uint32_t* bitmapData, int32_t bitmapWidth, int32_t bitmapHeight);

    void FillRectangle(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t colorIndex);

    void DrawRectangle(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t colorIndex);

    void FillRoundedRectangle(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t radius, uint32_t colorIndex);

    void DrawRoundedRectangle(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t radius, uint32_t colorIndex);

    void FillCircle(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t cx, int32_t cy, uint32_t radius, uint32_t colorIndex);

    void DrawCircle(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t cx, int32_t cy, uint32_t radius, uint32_t colorIndex);

    void DrawHorizontalLine(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x, int32_t y, int32_t length, uint32_t colorIndex);

    void DrawVerticalLine(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x, int32_t y, int32_t length, uint32_t colorIndex);

    void DrawCharacter(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x, int32_t y, char c, Font* font, uint32_t colorIndex);

    void DrawString(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x, int32_t y, const char* str, Font* font, uint32_t colorIndex);
};

#endif // NINA_H
