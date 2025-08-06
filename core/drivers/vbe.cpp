/**
 * @file        vbe.cpp
 * @brief       VESA BIOS Extensions (VBE) interface for #x86 
 * 
 * @date        08/02/2025
 * @version     1.0.0-beta
 */

#include <core/drivers/vbe.h>


VESA_BIOS_Extensions::VESA_BIOS_Extensions(MultibootInfo* mbinfo) 
{
    // Check if the framebuffer information is available in the Multiboot structure
    if (mbinfo->flags & (1 << 12)) {
        this->framebuffer = (uint32_t*)mbinfo->framebuffer_addr; // Set framebuffer pointer
        DEBUG_LOG("Frame Buffer : 0x%x", framebuffer);
        this->VGA_SCREEN_WIDTH = mbinfo->framebuffer_width;      // Set screen width
        this->VGA_SCREEN_HEIGHT = mbinfo->framebuffer_height;    // Set screen height
        this->VGA_SCREEN_BPP = mbinfo->framebuffer_bpp;          // Set bits per pixel (BPP)

        // Calculate the total framebuffer size
        uint32_t framebuffer_size = mbinfo->framebuffer_width * mbinfo->framebuffer_height * (mbinfo->framebuffer_bpp / 8);

        // Precompute alpha blending values to optimize blending calculations
        PrecomputeAlphaTable();

        // Fill the entire screen with black (initial clearing)
        this->FillRectangle(0, 0, VGA_SCREEN_WIDTH, VGA_SCREEN_HEIGHT, 0xFF000000);
    }
}

void VESA_BIOS_Extensions::PrecomputeAlphaTable() {
    for (int c = 0; c < 256; c++) {
        for (int a = 0; a < 256; a++) {
            this->alphaTable[a][c] = (c * a) / 255;
        }
    }
}


VESA_BIOS_Extensions::~VESA_BIOS_Extensions()
{
}


void VESA_BIOS_Extensions::Flush()
{
    memcpy(framebuffer, backBuffer, VGA_SCREEN_WIDTH * VGA_SCREEN_HEIGHT * sizeof(uint32_t));
}


void VESA_BIOS_Extensions::PutPixel(int32_t x, int32_t y, uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
    // Combine ARGB values into a single 32-bit color index
    uint32_t colorIndex = (a << 24) | (r << 16) | (g << 8) | b;

    // Call the overloaded PutPixel function with the packed color index
    this->PutPixel(x, y, colorIndex);
}


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


void VESA_BIOS_Extensions::DrawBitmap(int32_t x, int32_t y, const uint32_t* bitmapData, int32_t bitmapWidth, int32_t bitmapHeight) {
    // Clip top and bottom
    int32_t startRow = 0;
    int32_t endRow = bitmapHeight;
    if (y < 0) startRow = -y;
    if (y + bitmapHeight > VGA_SCREEN_HEIGHT) endRow = VGA_SCREEN_HEIGHT - y;

    // Clip left and right
    int32_t startCol = 0;
    int32_t endCol = bitmapWidth;
    if (x < 0) startCol = -x;
    if (x + bitmapWidth > VGA_SCREEN_WIDTH) endCol = VGA_SCREEN_WIDTH - x;

    for (int32_t row = startRow; row < endRow; ++row) {
        int32_t screenY = y + row;
        const uint32_t* bmpPtr = &bitmapData[row * bitmapWidth];
        uint32_t* rowDst = &backBuffer[screenY * VGA_SCREEN_WIDTH];

        for (int32_t col = startCol; col < endCol; ++col) {
            int32_t screenX = x + col;

            uint32_t srcColor = bmpPtr[col];
            uint8_t alpha = (srcColor >> 24) & 0xFF;

            if (alpha == 255) {
                rowDst[screenX] = srcColor;
            } else if (alpha > 0) {
                uint32_t dstColor = rowDst[screenX];
                uint32_t invAlpha = 255 - alpha;

                uint8_t srcRed   = (srcColor >> 16) & 0xFF;
                uint8_t srcGreen = (srcColor >> 8)  & 0xFF;
                uint8_t srcBlue  =  srcColor        & 0xFF;

                uint8_t dstRed   = (dstColor >> 16) & 0xFF;
                uint8_t dstGreen = (dstColor >> 8)  & 0xFF;
                uint8_t dstBlue  =  dstColor        & 0xFF;

                uint8_t blendedRed   = alphaTable[alpha][srcRed] + alphaTable[invAlpha][dstRed];
                uint8_t blendedGreen = alphaTable[alpha][srcGreen] + alphaTable[invAlpha][dstGreen];
                uint8_t blendedBlue  = alphaTable[alpha][srcBlue] + alphaTable[invAlpha][dstBlue];

                rowDst[screenX] = (blendedRed << 16) | (blendedGreen << 8) | blendedBlue;
            }
        }
    }
}


void VESA_BIOS_Extensions::FillRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t colorIndex) {
    // Clip boundaries
    int32_t startX = (x < 0) ? 0 : x;
    int32_t startY = (y < 0) ? 0 : y;
    int32_t endX = (x + w > VGA_SCREEN_WIDTH) ? VGA_SCREEN_WIDTH : x + w;
    int32_t endY = (y + h > VGA_SCREEN_HEIGHT) ? VGA_SCREEN_HEIGHT : y + h;
    
    for (int32_t Y = startY; Y < endY; Y++) {
        for (int32_t X = startX; X < endX; X++) {
            backBuffer[Y * VGA_SCREEN_WIDTH + X] = colorIndex;
        }
    }
}

void VESA_BIOS_Extensions::DrawRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t colorIndex) {
    // Clip boundaries
    int32_t startX = (x < 0) ? 0 : x;
    int32_t startY = (y < 0) ? 0 : y;
    int32_t endX = (x + w > VGA_SCREEN_WIDTH) ? VGA_SCREEN_WIDTH : x + w;
    int32_t endY = (y + h > VGA_SCREEN_HEIGHT) ? VGA_SCREEN_HEIGHT : y + h;
    
    // Draw top and bottom lines
    for (int32_t X = startX; X < endX; X++) {
        backBuffer[startY * VGA_SCREEN_WIDTH + X] = colorIndex;
        backBuffer[(endY - 1) * VGA_SCREEN_WIDTH + X] = colorIndex;
    }
    // Draw left and right lines
    for (int32_t Y = startY; Y < endY; Y++) {
        backBuffer[Y * VGA_SCREEN_WIDTH + startX] = colorIndex;
        backBuffer[Y * VGA_SCREEN_WIDTH + (endX - 1)] = colorIndex;
    }
}

void VESA_BIOS_Extensions::FillRoundedRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t radius, uint32_t colorIndex) {
    FillRectangle(x + radius, y, w - 2 * radius, h, colorIndex);
    FillRectangle(x, y + radius, w, h - 2 * radius, colorIndex);

    for (int32_t dy = 0; dy <= radius; ++dy) {
        for (int32_t dx = 0; dx <= radius; ++dx) {
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                int32_t tlX = x + radius - dx;
                int32_t tlY = y + radius - dy;
                int32_t trX = x + w - radius + dx - 1;
                int32_t brY = y + h - radius + dy - 1;
                
                if (tlX >= 0 && tlY >= 0) backBuffer[tlY * VGA_SCREEN_WIDTH + tlX] = colorIndex;
                if (trX < VGA_SCREEN_WIDTH && tlY >= 0) backBuffer[tlY * VGA_SCREEN_WIDTH + trX] = colorIndex;
                if (tlX >= 0 && brY < VGA_SCREEN_HEIGHT) backBuffer[brY * VGA_SCREEN_WIDTH + tlX] = colorIndex;
                if (trX < VGA_SCREEN_WIDTH && brY < VGA_SCREEN_HEIGHT) backBuffer[brY * VGA_SCREEN_WIDTH + trX] = colorIndex;
            }
        }
    }
}

void VESA_BIOS_Extensions::DrawRoundedRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t radius, uint32_t colorIndex) {
    for (int32_t i = x + radius; i < x + w - radius; ++i) {
        if (i >= 0 && i < VGA_SCREEN_WIDTH) {
            if (y >= 0 && y < VGA_SCREEN_HEIGHT) backBuffer[y * VGA_SCREEN_WIDTH + i] = colorIndex;
            if (y + h - 1 >= 0 && y + h - 1 < VGA_SCREEN_HEIGHT) backBuffer[(y + h - 1) * VGA_SCREEN_WIDTH + i] = colorIndex;
        }
    }
    
    for (int32_t i = y + radius; i < y + h - radius; ++i) {
        if (i >= 0 && i < VGA_SCREEN_HEIGHT) {
            if (x >= 0 && x < VGA_SCREEN_WIDTH) backBuffer[i * VGA_SCREEN_WIDTH + x] = colorIndex;
            if (x + w - 1 >= 0 && x + w - 1 < VGA_SCREEN_WIDTH) backBuffer[i * VGA_SCREEN_WIDTH + (x + w - 1)] = colorIndex;
        }
    }
    
    for (int32_t dy = 0; dy <= static_cast<int32_t>(radius); ++dy) {
        for (int32_t dx = 0; dx <= static_cast<int32_t>(radius); ++dx) {
            int32_t d2 = dx * dx + dy * dy;
            if (d2 >= static_cast<int32_t>((radius - 1) * (radius - 1)) && d2 <= static_cast<int32_t>(radius * radius)) {
                int32_t tlX = x + radius - dx;
                int32_t tlY = y + radius - dy;
                int32_t trX = x + w - radius + dx - 1;
                int32_t brY = y + h - radius + dy - 1;
                
                if (tlX >= 0 && tlY >= 0) backBuffer[tlY * VGA_SCREEN_WIDTH + tlX] = colorIndex;
                if (trX < VGA_SCREEN_WIDTH && tlY >= 0) backBuffer[tlY * VGA_SCREEN_WIDTH + trX] = colorIndex;
                if (tlX >= 0 && brY < VGA_SCREEN_HEIGHT) backBuffer[brY * VGA_SCREEN_WIDTH + tlX] = colorIndex;
                if (trX < VGA_SCREEN_WIDTH && brY < VGA_SCREEN_HEIGHT) backBuffer[brY * VGA_SCREEN_WIDTH + trX] = colorIndex;
            }
        }
    }
}


void VESA_BIOS_Extensions::DrawRoundedRectangleShadow(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t shadowSize, uint32_t shadowRadius, uint32_t shadowColor) {
    int32_t startX = x - shadowSize;
    int32_t startY = y - shadowSize;
    int32_t endX = x + w + shadowSize;
    int32_t endY = y + h + shadowSize;

    uint8_t shadowAlpha = (shadowColor >> 24) & 0xFF;
    uint8_t shadowRed = (shadowColor >> 16) & 0xFF;
    uint8_t shadowGreen = (shadowColor >> 8) & 0xFF;
    uint8_t shadowBlue = shadowColor & 0xFF;

    for (int32_t Y = startY; Y < endY; ++Y) {
        if (Y < 0 || Y >= VGA_SCREEN_HEIGHT) continue;
        uint32_t* pixelPtr = &backBuffer[Y * VGA_SCREEN_WIDTH + startX];

        for (int32_t X = startX; X < endX; ++X) {
            if (X < 0 || X >= VGA_SCREEN_WIDTH) continue;

            int32_t dx = 0, dy = 0;
            if (X < x) dx = x - X;
            else if (X >= x + w) dx = X - (x + w - 1);
            if (Y < y) dy = y - Y;
            else if (Y >= y + h) dy = Y - (y + h - 1);

            int32_t distanceSquared = dx * dx + dy * dy;
            if (distanceSquared <= (shadowRadius * shadowRadius)) {
                uint8_t alpha = alphaTable[shadowAlpha][255 - (distanceSquared * 255) / (shadowRadius * shadowRadius)];
                uint32_t dstColor = *pixelPtr;
                uint32_t invAlpha = 255 - alpha;

                uint8_t dstRed = (dstColor >> 16) & 0xFF;
                uint8_t dstGreen = (dstColor >> 8) & 0xFF;
                uint8_t dstBlue = dstColor & 0xFF;

                uint8_t blendedRed = alphaTable[alpha][shadowRed] + alphaTable[invAlpha][dstRed];
                uint8_t blendedGreen = alphaTable[alpha][shadowGreen] + alphaTable[invAlpha][dstGreen];
                uint8_t blendedBlue = alphaTable[alpha][shadowBlue] + alphaTable[invAlpha][dstBlue];

                *pixelPtr = (blendedRed << 16) | (blendedGreen << 8) | blendedBlue;
            }
            pixelPtr++;
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

void VESA_BIOS_Extensions::DrawString(int32_t x, int32_t y, const char* str, uint32_t colorIndex) {
    int32_t cursorX = x;
    int8_t chrAdvanceX = VBE_font->chrAdvanceX;
    int8_t chrAdvanceY = VBE_font->chrAdvanceY;

    // Iterate through the string and draw each character
    while (*str) {
        if (*str == '\n') { // Handle newline characters
            y += 20;        // Move to the next line
            cursorX = x;    // Reset to the starting x position
        } else {
            DrawCharacter(cursorX, y, *str, VBE_font, colorIndex);
            cursorX += 36 + chrAdvanceX - (VBE_font->font_36x36_config[*str - 32][0] + VBE_font->font_36x36_config[*str - 32][1]);
        }

        ++str;
    }
}