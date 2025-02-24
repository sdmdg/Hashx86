/**
 * @file        vbe.cpp
 * @brief       VESA BIOS Extensions (VBE) interface for #x86 
 * 
 * @date        08/02/2025
 * @version     1.0.0-beta
 */

#include <core/drivers/vbe.h>

/**
 * @brief Constructs a VESA BIOS Extensions (VBE) interface and initializes the framebuffer.
 * 
 * @param mbinfo Pointer to the Multiboot information structure containing framebuffer details.
 */
VESA_BIOS_Extensions::VESA_BIOS_Extensions(MultibootInfo* mbinfo) 
{
    // Check if the framebuffer information is available in the Multiboot structure
    if (mbinfo->flags & (1 << 12)) {
        this->framebuffer = (uint32_t*)mbinfo->framebuffer_addr; // Set framebuffer pointer
        this->VGA_SCREEN_WIDTH = mbinfo->framebuffer_width;      // Set screen width
        this->VGA_SCREEN_HEIGHT = mbinfo->framebuffer_height;    // Set screen height
        this->VGA_SCREEN_BPP = mbinfo->framebuffer_bpp;          // Set bits per pixel (BPP)

        // Calculate the total framebuffer size
        uint32_t framebuffer_size = mbinfo->framebuffer_width * mbinfo->framebuffer_height * (mbinfo->framebuffer_bpp / 8);

        // Precompute alpha blending values to optimize blending calculations
        PrecomputeAlphaTable();

        // Fill the entire screen with black (initial clearing)
        this->FillRectangle(0, 0, VGA_SCREEN_WIDTH, VGA_SCREEN_HEIGHT, 0x0);
    }
}

/**
 * @brief Precomputes an alpha blending table for efficient per-pixel alpha blending.
 * 
 * The table stores precomputed values for blending color intensity based on alpha values.
 */
void VESA_BIOS_Extensions::PrecomputeAlphaTable() {
    for (int c = 0; c < 256; c++) {
        for (int a = 0; a < 256; a++) {
            this->alphaTable[a][c] = (c * a) / 255;
        }
    }
}

/**
 * @brief Destructor for the VESA_BIOS_Extensions class.
 */
VESA_BIOS_Extensions::~VESA_BIOS_Extensions()
{
}

/**
 * @brief Copies the contents of the back buffer to the framebuffer, updating the display.
 */
void VESA_BIOS_Extensions::Flush()
{
    memcpy(framebuffer, backBuffer, VGA_SCREEN_WIDTH * VGA_SCREEN_HEIGHT * sizeof(uint32_t));
}

/**
 * @brief Draws a pixel on the screen with individual alpha, red, green, and blue values.
 * 
 * @param x X-coordinate of the pixel.
 * @param y Y-coordinate of the pixel.
 * @param a Alpha (transparency) value (0-255).
 * @param r Red color component (0-255).
 * @param g Green color component (0-255).
 * @param b Blue color component (0-255).
 */
void VESA_BIOS_Extensions::PutPixel(int32_t x, int32_t y, uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
    // Combine ARGB values into a single 32-bit color index
    uint32_t colorIndex = (a << 24) | (r << 16) | (g << 8) | b;

    // Call the overloaded PutPixel function with the packed color index
    this->PutPixel(x, y, colorIndex);
}

/**
 * @brief Draws a pixel on the screen with a 32-bit color value, applying alpha blending.
 * 
 * @param x X-coordinate of the pixel.
 * @param y Y-coordinate of the pixel.
 * @param colorIndex The 32-bit color value in ARGB format.
 */
void VESA_BIOS_Extensions::PutPixel(int32_t x, int32_t y, uint32_t colorIndex)
{
    // Bounds checking: Ensure the pixel coordinates are within the screen limits
    if ((uint32_t)x >= (uint32_t)VGA_SCREEN_WIDTH || (uint32_t)y >= (uint32_t)VGA_SCREEN_HEIGHT)
        return;

    // Get a pointer to the pixel location in the back buffer
    uint32_t* pixel = &backBuffer[y * VGA_SCREEN_WIDTH + x];

    // Extract alpha value from the color index
    uint8_t alpha = (colorIndex >> 24) & 0xFF;

    if (alpha == 0) return; // Fully transparent pixel, do nothing
    if (alpha == 255) { 
        *pixel = colorIndex; // Fully opaque, directly copy the color
        return;
    }

    // Extract RGB components from the color index
    uint32_t newRed   = (colorIndex >> 16) & 0xFF;
    uint32_t newGreen = (colorIndex >> 8) & 0xFF;
    uint32_t newBlue  = colorIndex & 0xFF;

    // Extract RGB components from the existing pixel color
    uint32_t existingColor = *pixel;
    uint32_t existingRed   = (existingColor >> 16) & 0xFF;
    uint32_t existingGreen = (existingColor >> 8) & 0xFF;
    uint32_t existingBlue  = existingColor & 0xFF;

    // Optimized alpha blending using precomputed values
    uint32_t invAlpha = 255 - alpha;
    uint32_t blendedRed   = alphaTable[alpha][newRed] + alphaTable[invAlpha][existingRed];
    uint32_t blendedGreen = alphaTable[alpha][newGreen] + alphaTable[invAlpha][existingGreen];
    uint32_t blendedBlue  = alphaTable[alpha][newBlue] + alphaTable[invAlpha][existingBlue];

    // Store the blended color back into the back buffer
    *pixel = (blendedRed << 16) | (blendedGreen << 8) | blendedBlue;
}


void VESA_BIOS_Extensions::DrawBitmap(int32_t x, int32_t y, const uint32_t* bitmapData, int32_t bitmapWidth, int32_t bitmapHeight)
{
    for (int32_t row = 0; row < bitmapHeight; ++row)
    {
        int32_t screenY = y + row;
        if ((uint32_t)screenY >= (uint32_t)VGA_SCREEN_HEIGHT) continue;

        int32_t rowOffset = screenY * VGA_SCREEN_WIDTH + x;
        int32_t bmpRowOffset = row * bitmapWidth;

        for (int32_t col = 0; col < bitmapWidth; col += 4) // Process 4 pixels per iteration (loop unrolling)
        {
            int32_t screenX = x + col;
            if ((uint32_t)screenX >= (uint32_t)VGA_SCREEN_WIDTH) continue;

            PutPixel(screenX, screenY, bitmapData[bmpRowOffset + col]);
            if (screenX + 1 < VGA_SCREEN_WIDTH) PutPixel(screenX + 1, screenY, bitmapData[bmpRowOffset + col + 1]);
            if (screenX + 2 < VGA_SCREEN_WIDTH) PutPixel(screenX + 2, screenY, bitmapData[bmpRowOffset + col + 2]);
            if (screenX + 3 < VGA_SCREEN_WIDTH) PutPixel(screenX + 3, screenY, bitmapData[bmpRowOffset + col + 3]);
        }
    }
}


void VESA_BIOS_Extensions::FillRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t colorIndex)
{
    // Clip negative x, y values
    int32_t startX = (x < 0) ? 0 : x;
    int32_t startY = (y < 0) ? 0 : y;

    int32_t endX = x + w;
    int32_t endY = y + h;

    // Clip width and height to screen bounds
    if (endX > VGA_SCREEN_WIDTH) endX = VGA_SCREEN_WIDTH;
    if (endY > VGA_SCREEN_HEIGHT) endY = VGA_SCREEN_HEIGHT;

    for (int32_t Y = startY; Y < endY; Y++) {
        for (int32_t X = startX; X < endX; X++) {
            PutPixel(X, Y, colorIndex);
        }
    }
}


void VESA_BIOS_Extensions::DrawRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t colorIndex) {
    // Draw top and bottom horizontal lines
    for (int32_t X = x; X < x + w; X++) {
        PutPixel(X, y, colorIndex);          // Top line
        PutPixel(X, y + h - 1, colorIndex);  // Bottom line
    }

    // Draw left and right vertical lines
    for (int32_t Y = y; Y < y + h; Y++) {
        PutPixel(x, Y, colorIndex);          // Left line
        PutPixel(x + w - 1, Y, colorIndex);  // Right line
    }
}

void VESA_BIOS_Extensions::FillRoundedRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t radius, uint32_t colorIndex) {
    // Draw the central rectangle (excluding the rounded corners)
    FillRectangle(x + radius, y, w - 2 * radius, h, colorIndex);
    FillRectangle(x, y + radius, w, h - 2 * radius, colorIndex);

    // Draw the rounded corners using more precise quarter-circle logic
    for (int32_t dy = 0; dy <= radius; ++dy) {
        for (int32_t dx = 0; dx <= radius; ++dx) {
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                // Top-left corner
                PutPixel(x + radius - dx, y + radius - dy, colorIndex);
                // Top-right corner
                PutPixel(x + w - radius + dx - 1, y + radius - dy, colorIndex);
                // Bottom-left corner
                PutPixel(x + radius - dx, y + h - radius + dy - 1, colorIndex);
                // Bottom-right corner
                PutPixel(x + w - radius + dx - 1, y + h - radius + dy - 1, colorIndex);
            }
        }
    }
}

void VESA_BIOS_Extensions::DrawRoundedRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t radius, uint32_t colorIndex) {
    // Draw horizontal lines (excluding rounded corners)
    for (int32_t i = x + radius; i < x + w - radius; ++i) {
        PutPixel(i, y, colorIndex);           // Top edge
        PutPixel(i, y + h - 1, colorIndex);     // Bottom edge
    }
    
    // Draw vertical lines (excluding rounded corners)
    for (int32_t i = y + radius; i < y + h - radius; ++i) {
        PutPixel(x, i, colorIndex);           // Left edge
        PutPixel(x + w - 1, i, colorIndex);     // Right edge
    }
    
    // Draw the rounded corners using a narrow band to outline the quarter-circles
    for (int32_t dy = 0; dy <= static_cast<int32_t>(radius); ++dy) {
        for (int32_t dx = 0; dx <= static_cast<int32_t>(radius); ++dx) {
            int32_t d2 = dx * dx + dy * dy;
            // Check if the pixel lies on the border of the circle
            if (d2 >= static_cast<int32_t>((radius - 1) * (radius - 1)) && d2 <= static_cast<int32_t>(radius * radius)) {
                // Top-left corner
                PutPixel(x + radius - dx, y + radius - dy, colorIndex);
                // Top-right corner
                PutPixel(x + w - radius + dx - 1, y + radius - dy, colorIndex);
                // Bottom-left corner
                PutPixel(x + radius - dx, y + h - radius + dy - 1, colorIndex);
                // Bottom-right corner
                PutPixel(x + w - radius + dx - 1, y + h - radius + dy - 1, colorIndex);
            }
        }
    }
}


void VESA_BIOS_Extensions::BlurRoundedRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t radius, uint32_t blurRadius) {
    // Temporary buffer for blurred pixels
    uint32_t tempBuffer[w * h];

    // First pass: Compute blurred colors
    for (int32_t dy = 0; dy < (int32_t)h; dy++) {
        for (int32_t dx = 0; dx < (int32_t)w; dx++) {
            int32_t px = x + dx;
            int32_t py = y + dy;

            // Skip if outside screen bounds
            if (px < 0 || py < 0 || px >= VGA_SCREEN_WIDTH || py >= VGA_SCREEN_HEIGHT)
                continue;

            // Check if the pixel is inside the rounded region
            int32_t distX = (dx < radius) ? radius - dx : (dx >= (int32_t)w - radius) ? dx - ((int32_t)w - radius) : 0;
            int32_t distY = (dy < radius) ? radius - dy : (dy >= (int32_t)h - radius) ? dy - ((int32_t)h - radius) : 0;
            int32_t dist = (distX * distX + distY * distY);

            if (dist > radius * radius)
                continue; // Skip corners

            // Blur effect: Average nearby pixels, including alpha
            uint32_t red = 0, green = 0, blue = 0, alpha = 0, count = 0;
            for (int32_t blurY = -blurRadius; blurY <= blurRadius; blurY++) {
                for (int32_t blurX = -blurRadius; blurX <= blurRadius; blurX++) {
                    int32_t nx = px + blurX;
                    int32_t ny = py + blurY;

                    // Ensure pixel is within screen bounds
                    if (nx >= 0 && ny >= 0 && nx < VGA_SCREEN_WIDTH && ny < VGA_SCREEN_HEIGHT) {
                        uint32_t color = backBuffer[ny * VGA_SCREEN_WIDTH + nx];
                        red   += (color >> 16) & 0xFF;
                        green += (color >> 8) & 0xFF;
                        blue  += color & 0xFF;
                        alpha += (color >> 24) & 0xFF;  // Preserve alpha
                        count++;
                    }
                }
            }

            // Store the blurred pixel in the temporary buffer
            if (count > 0) {
                red   = (red + count / 2) / count;
                green = (green + count / 2) / count;
                blue  = (blue + count / 2) / count;
                alpha = (alpha + count / 2) / count;  // Preserve averaged alpha

                tempBuffer[dy * w + dx] = (alpha << 24) | (red << 16) | (green << 8) | blue;
            } else {
                tempBuffer[dy * w + dx] = backBuffer[py * VGA_SCREEN_WIDTH + px]; // Keep original color
            }
        }
    }

    // Second pass: Copy back blurred pixels using PutPixel()
    for (int32_t dy = 0; dy < (int32_t)h; dy++) {
        for (int32_t dx = 0; dx < (int32_t)w; dx++) {
            int32_t px = x + dx;
            int32_t py = y + dy;

            if (px < 0 || py < 0 || px >= VGA_SCREEN_WIDTH || py >= VGA_SCREEN_HEIGHT)
                continue;

            // Check if the pixel is inside the rounded region
            int32_t distX = (dx < radius) ? radius - dx : (dx >= (int32_t)w - radius) ? dx - ((int32_t)w - radius) : 0;
            int32_t distY = (dy < radius) ? radius - dy : (dy >= (int32_t)h - radius) ? dy - ((int32_t)h - radius) : 0;
            int32_t dist = (distX * distX + distY * distY);

            if (dist > radius * radius)
                continue;

            // Apply the blurred pixel using PutPixel() to respect alpha blending
            PutPixel(px, py, tempBuffer[dy * w + dx]);
        }
    }
}


void VESA_BIOS_Extensions::FillCircle(int32_t cx, int32_t cy, uint32_t radius, uint32_t colorIndex) {
    int32_t x = radius;
    int32_t y = 0;
    int32_t err = 0;

    while (x >= y) {
        // Draw horizontal lines to fill the circle
        for (int32_t dx = cx - x; dx <= cx + x; ++dx) {
            PutPixel(dx, cy + y, colorIndex); // Fill upper arc
            PutPixel(dx, cy - y, colorIndex); // Fill lower arc
        }
        for (int32_t dx = cx - y; dx <= cx + y; ++dx) {
            PutPixel(dx, cy + x, colorIndex); // Fill right arc
            PutPixel(dx, cy - x, colorIndex); // Fill left arc
        }

        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

void VESA_BIOS_Extensions::DrawCircle(int32_t cx, int32_t cy, uint32_t radius, uint32_t colorIndex) {
    int32_t x = radius;
    int32_t y = 0;
    int32_t err = 0;

    while (x >= y) {
        PutPixel(cx + x, cy + y, colorIndex);
        PutPixel(cx + y, cy + x, colorIndex);
        PutPixel(cx - y, cy + x, colorIndex);
        PutPixel(cx - x, cy + y, colorIndex);
        PutPixel(cx - x, cy - y, colorIndex);
        PutPixel(cx - y, cy - x, colorIndex);
        PutPixel(cx + y, cy - x, colorIndex);
        PutPixel(cx + x, cy - y, colorIndex);

        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

void VESA_BIOS_Extensions::DrawHorizontalLine(int32_t x, int32_t y, int32_t length, uint32_t colorIndex) {
    for (int32_t i = 0; i < length; ++i) {
        PutPixel(x + i, y, colorIndex);
    }
}

void VESA_BIOS_Extensions::DrawVerticalLine(int32_t x, int32_t y, int32_t length, uint32_t colorIndex) {
    for (int32_t i = 0; i < length; ++i) {
        PutPixel(x, y + i, colorIndex);
    }
}

void VESA_BIOS_Extensions::DrawCharacter(int32_t x, int32_t y, char c, Font* font, uint32_t colorIndex) {
    if (c < 32 || c > 126) {
        c = '?';  // Replace unsupported characters
    }

    // Constants for font bitmap layout
    const int charWidth = 36;
    const int charHeight = 36;
    const int charsPerRow = 19;

    // Calculate character position in the font grid
    uint32_t charIndex = c - 32;
    uint32_t gridX = (charIndex % charsPerRow) * charWidth;
    uint32_t gridY = (charIndex / charsPerRow) * charHeight;

    uint32_t croppedBitmap[charWidth * charHeight];  // Array to store pixel data

    for (uint32_t row = 0; row < charHeight; ++row) {
        for (uint32_t col = 0; col < charWidth; ++col) {
            // Extract the alpha value
            uint32_t fontPixel = font->font_36x36[gridY + row][gridX + col];
            uint8_t fontAlpha = (fontPixel >> 24) & 0xFF; // Extract alpha byte from font bitmap

            // Extract ARGB values from colorIndex
            uint8_t inputAlpha = (colorIndex >> 24) & 0xFF;
            uint8_t inputRed = (colorIndex >> 16) & 0xFF;
            uint8_t inputGreen = (colorIndex >> 8) & 0xFF;
            uint8_t inputBlue = colorIndex & 0xFF;

            // Blend alpha values for fade effect
            uint8_t blendedAlpha;
            if (fontAlpha != 0) {
                blendedAlpha = (fontAlpha * inputAlpha) / 285;
            } else {
                blendedAlpha = 0; // Fully transparent
            }
            
            // Combine blended alpha with input RGB values
            uint32_t blendedColor = (blendedAlpha << 24) | (inputRed << 16) | (inputGreen << 8) | inputBlue;

            // Store the blended color in the croppedBitmap
            croppedBitmap[row * charWidth + col] = blendedColor;
        }
    }

    // Draw the cropped bitmap on the screen
    DrawBitmap(x - font->font_36x36_config[charIndex][0], y + font->chrAdvanceY, croppedBitmap, charWidth, charHeight);
}

void VESA_BIOS_Extensions::DrawString(int32_t x, int32_t y, const char* str, Font* font, uint32_t colorIndex) {
    int32_t cursorX = x;
    int8_t chrAdvanceX = font->chrAdvanceX;
    int8_t chrAdvanceY = font->chrAdvanceY;

    // Iterate through the string and draw each character
    while (*str) {
        if (*str == '\n') { // Handle newline characters
            y += 20;        // Move to the next line
            cursorX = x;    // Reset to the starting x position
        } else {
            DrawCharacter(cursorX, y, *str, font, colorIndex);
            cursorX += 36 + chrAdvanceX - (font->font_36x36_config[*str - 32][0] + font->font_36x36_config[*str - 32][1]);

        }

        ++str;
    }
}


