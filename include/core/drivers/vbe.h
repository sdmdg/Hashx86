#ifndef VBE_H
#define VBE_H

#include <types.h>
#include <core/ports.h>
#include <core/driver.h>
#include <gui/fonts/font.h>
#include <gui/fonts/segoeui.h>
#include <gui/icons.h>
#include <core/multiboot.h>
#include <core/memory.h>

/**
 * @brief Class for handling VESA BIOS Extensions (VBE) graphics.
 * 
 * This class provides functions for framebuffer manipulation, pixel drawing, 
 * alpha blending, and basic 2D graphics rendering (rectangles, circles, and text).
 */
class VESA_BIOS_Extensions
{
protected:
    /// Default screen resolution (VGA mode)
    uint32_t VGA_SCREEN_WIDTH = 1152;
    uint32_t VGA_SCREEN_HEIGHT = 864;
    uint8_t VGA_SCREEN_BPP; // Bits per pixel (BPP)

    /// Pointer to the raw framebuffer memory
    uint32_t* framebuffer;

    /// Maximum allowed screen dimensions
    static constexpr int MAX_WIDTH = 1152;
    static constexpr int MAX_HEIGHT = 864;

    /// Back buffer for off-screen rendering (avoids flickering)
    uint32_t backBuffer[MAX_WIDTH * MAX_HEIGHT];

    /// Alpha blending lookup table for optimized blending calculations
    uint8_t alphaTable[256][256];

    /**
     * @brief Precomputes an alpha blending table for efficient transparency handling.
     */
    void PrecomputeAlphaTable();

public:
    SegoeUI* VBE_font = new SegoeUI();
    
    /**
     * @brief Constructs a VESA BIOS Extensions instance using Multiboot information.
     * 
     * @param mbinfo Pointer to the Multiboot structure containing framebuffer details.
     */
    VESA_BIOS_Extensions(MultibootInfo* mbinfo);

    /**
     * @brief Destructor for the VESA_BIOS_Extensions class.
     */
    ~VESA_BIOS_Extensions();

    /**
     * @brief Copies the contents of the back buffer to the framebuffer (screen update).
     */
    void Flush();

    /**
     * @brief Draws a pixel at (x, y) with specified ARGB color components.
     * 
     * @param x X-coordinate of the pixel.
     * @param y Y-coordinate of the pixel.
     * @param a Alpha (transparency) component (0-255).
     * @param r Red component (0-255).
     * @param g Green component (0-255).
     * @param b Blue component (0-255).
     */
    virtual void PutPixel(int32_t x, int32_t y, uint8_t a, uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Draws a pixel at (x, y) with a 32-bit ARGB color value.
     * 
     * @param x X-coordinate of the pixel.
     * @param y Y-coordinate of the pixel.
     * @param colorIndex 32-bit ARGB color value.
     */
    virtual void PutPixel(int32_t x, int32_t y, uint32_t colorIndex);

    /**
     * @brief Draws a bitmap image at the specified coordinates.
     * 
     * @param x X-coordinate of the top-left corner.
     * @param y Y-coordinate of the top-left corner.
     * @param bitmapData Pointer to the bitmap data.
     * @param bitmapWidth Width of the bitmap.
     * @param bitmapHeight Height of the bitmap.
     */
    void DrawBitmap(int32_t x, int32_t y, const uint32_t* bitmapData, int32_t bitmapWidth, int32_t bitmapHeight);

    /**
     * @brief Fills a rectangular area with a solid color.
     * 
     * @param x X-coordinate of the top-left corner.
     * @param y Y-coordinate of the top-left corner.
     * @param w Width of the rectangle.
     * @param h Height of the rectangle.
     * @param colorIndex 32-bit ARGB color value.
     */
    virtual void FillRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t colorIndex);

    /**
     * @brief Draws a rectangular outline with a specified color.
     */
    void DrawRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t colorIndex);

    /**
     * @brief Fills a rounded rectangle with a solid color.
     */
    void FillRoundedRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t radius, uint32_t colorIndex);

    /**
     * @brief Draws a rounded rectangle outline.
     */
    void DrawRoundedRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t radius, uint32_t colorIndex);



    
    void DrawRoundedRectangleShadow(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t shadowSize, uint32_t shadowRadius, uint32_t shadowColor);
 


    /**
     * @brief Draws the outline of a circle.
     */
    void DrawCircle(int32_t cx, int32_t cy, uint32_t radius, uint32_t colorIndex);

    /**
     * @brief Fills a circle with a solid color.
     */
    void FillCircle(int32_t cx, int32_t cy, uint32_t radius, uint32_t colorIndex);

    /**
     * @brief Draws a horizontal line.
     */
    void DrawHorizontalLine(int32_t x, int32_t y, int32_t length, uint32_t colorIndex);

    /**
     * @brief Draws a vertical line.
     */
    void DrawVerticalLine(int32_t x, int32_t y, int32_t length, uint32_t colorIndex);

    /**
     * @brief Draws a single character at the specified coordinates using a font.
     */
    void DrawCharacter(int32_t x, int32_t y, char c, Font* font, uint32_t colorIndex);

    /**
     * @brief Draws a string of characters using a font.
     */
    void DrawString(int32_t x, int32_t y, const char* str, Font* font, uint32_t colorIndex);
    
    /**
     * @brief Draws a string of characters without a font.
     */
    void DrawString(int32_t x, int32_t y, const char* str, uint32_t colorIndex);

    /**
     * @brief Applies a blur effect to a rounded rectangle.
     */
    void BlurRoundedRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t radius, uint32_t blurRadius);
};

#endif // VBE_H
