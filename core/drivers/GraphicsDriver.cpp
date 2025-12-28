
#include <core/drivers/GraphicsDriver.h>

GraphicsDriver::GraphicsDriver(uint32_t w, uint32_t h, uint32_t b, uint32_t* vram) {
    this->width = w;
    this->height = h;
    this->bpp = b;
    this->videoMemory = vram;

    // Allocate Backbuffer
    this->backBuffer = new uint32_t[width * height];

    // Clear screen
    FillRectangle(0, 0, width, height, 0xFF000000);
    
    PrecomputeAlphaTable();
}

GraphicsDriver::~GraphicsDriver() {
    if(backBuffer) delete[] backBuffer;
}

void GraphicsDriver::Flush() {
    // Copy Backbuffer to Video Memory
    if(videoMemory && backBuffer) {
        uint32_t size = width * height * sizeof(uint32_t);
        memcpy(videoMemory, backBuffer, size);
    }
}

void GraphicsDriver::PrecomputeAlphaTable() {
    for (int c = 0; c < 256; c++) {
        for (int a = 0; a < 256; a++) {
            alphaTable[a][c] = (c * a) / 255;
        }
    }
}


void GraphicsDriver::PutPixel(int32_t x, int32_t y, uint32_t colorIndex) {
    if ((uint32_t)x >= width || (uint32_t)y >= height) return;

    uint32_t* pixel = &backBuffer[y * width + x];
    uint8_t alpha = (colorIndex >> 24) & 0xFF;

    if (alpha == 0) return;
    if (alpha == 255) { 
        *pixel = colorIndex;
        return;
    }

    uint32_t existingColor = *pixel;
    uint32_t invAlpha = 255 - alpha;

    uint8_t r = alphaTable[alpha][(colorIndex >> 16) & 0xFF] + alphaTable[invAlpha][(existingColor >> 16) & 0xFF];
    uint8_t g = alphaTable[alpha][(colorIndex >> 8) & 0xFF] + alphaTable[invAlpha][(existingColor >> 8) & 0xFF];
    uint8_t b = alphaTable[alpha][colorIndex & 0xFF] + alphaTable[invAlpha][existingColor & 0xFF];

    *pixel = (255 << 24) | (r << 16) | (g << 8) | b;
}

void GraphicsDriver::PutPixel(int32_t x, int32_t y, uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    PutPixel(x, y, (a << 24) | (r << 16) | (g << 8) | b);
}


void GraphicsDriver::DrawBitmap(int32_t x, int32_t y, const uint32_t* bitmapData, int32_t bitmapWidth, int32_t bitmapHeight) {
    // Clip top and bottom
    int32_t startRow = 0;
    int32_t endRow = bitmapHeight;
    if (y < 0) startRow = -y;
    if (y + bitmapHeight > this->height) endRow = this->height - y;

    // Clip left and right
    int32_t startCol = 0;
    int32_t endCol = bitmapWidth;
    if (x < 0) startCol = -x;
    if (x + bitmapWidth > this->width) endCol = this->width - x;

    for (int32_t row = startRow; row < endRow; ++row) {
        int32_t screenY = y + row;
        const uint32_t* bmpPtr = &bitmapData[row * bitmapWidth];
        uint32_t* rowDst = &backBuffer[screenY * this->width];

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


void GraphicsDriver::FillRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t colorIndex) {
    NINA::activeInstance->FillRectangle(this->backBuffer, this->width, this->height, x, y, w, h, colorIndex);
}

void GraphicsDriver::DrawRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t colorIndex) {
    NINA::activeInstance->DrawRectangle(this->backBuffer, this->width, this->height, x, y, w, h, colorIndex);
}


void GraphicsDriver::FillRoundedRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t radius, uint32_t colorIndex) {
    NINA::activeInstance->FillRoundedRectangle(this->backBuffer, this->width, this->height, x, y, w, h, radius, colorIndex);
}

void GraphicsDriver::DrawRoundedRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t radius, uint32_t colorIndex) {
    NINA::activeInstance->DrawRoundedRectangle(this->backBuffer, this->width, this->height, x, y, w, h, radius, colorIndex);
}


void GraphicsDriver::DrawRoundedRectangleShadow(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t shadowSize, uint32_t shadowRadius, uint32_t shadowColor) {
    int32_t startX = x - shadowSize;
    int32_t startY = y - shadowSize;
    int32_t endX = x + w + shadowSize;
    int32_t endY = y + h + shadowSize;

    uint8_t shadowAlpha = (shadowColor >> 24) & 0xFF;
    uint8_t shadowRed = (shadowColor >> 16) & 0xFF;
    uint8_t shadowGreen = (shadowColor >> 8) & 0xFF;
    uint8_t shadowBlue = shadowColor & 0xFF;

    for (int32_t Y = startY; Y < endY; ++Y) {
        if (Y < 0 || Y >= this->height) continue;
        uint32_t* pixelPtr = &backBuffer[Y * this->width + startX];

        for (int32_t X = startX; X < endX; ++X) {
            if (X < 0 || X >= this->width) continue;

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

void GraphicsDriver::BlurRoundedRectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t radius, uint32_t blurRadius) {
    // Temporary buffer for blurred pixels
    uint32_t tempBuffer[w * h];

    // First pass: Compute blurred colors
    for (int32_t dy = 0; dy < (int32_t)h; dy++) {
        for (int32_t dx = 0; dx < (int32_t)w; dx++) {
            int32_t px = x + dx;
            int32_t py = y + dy;

            // Skip if outside screen bounds
            if (px < 0 || py < 0 || px >= this->width || py >= this->height)
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
                    if (nx >= 0 && ny >= 0 && nx < this->width && ny < this->height) {
                        uint32_t color = backBuffer[ny * this->width + nx];
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
                tempBuffer[dy * w + dx] = backBuffer[py * this->width + px]; // Keep original color
            }
        }
    }

    // Second pass: Copy back blurred pixels using PutPixel()
    for (int32_t dy = 0; dy < (int32_t)h; dy++) {
        for (int32_t dx = 0; dx < (int32_t)w; dx++) {
            int32_t px = x + dx;
            int32_t py = y + dy;

            if (px < 0 || py < 0 || px >= this->width || py >= this->height)
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


void GraphicsDriver::FillCircle(int32_t cx, int32_t cy, uint32_t radius, uint32_t colorIndex) {
    NINA::activeInstance->FillCircle(this->backBuffer, this->width, this->height, cx, cy, radius, colorIndex);
}

void GraphicsDriver::DrawCircle(int32_t cx, int32_t cy, uint32_t radius, uint32_t colorIndex) {
    NINA::activeInstance->DrawCircle(this->backBuffer, this->width, this->height, cx, cy, radius, colorIndex);
}


void GraphicsDriver::DrawHorizontalLine(int32_t x, int32_t y, int32_t length, uint32_t colorIndex) {
    NINA::activeInstance->DrawHorizontalLine(this->backBuffer, this->width, this->height, x, y, length, colorIndex);
}

void GraphicsDriver::DrawVerticalLine(int32_t x, int32_t y, int32_t length, uint32_t colorIndex) {
    NINA::activeInstance->DrawVerticalLine(this->backBuffer, this->width, this->height, x, y, length, colorIndex);
}


void GraphicsDriver::DrawCharacter(int32_t x, int32_t y, char c, Font* font, uint32_t colorIndex) {
    NINA::activeInstance->DrawCharacter(this->backBuffer, this->width, this->height, x, y, c, font, colorIndex);
}

void GraphicsDriver::DrawString(int32_t startX, int32_t startY, const char* str, Font* font, uint32_t colorIndex) {
    NINA::activeInstance->DrawString(this->backBuffer, this->width, this->height, startX, startY, str, font, colorIndex);
}

// --- Utility Functions ---
void GraphicsDriver::GetScreenCenter(uint32_t w, uint32_t h, int32_t& x, int32_t& y) {
    // Determine horizontal center
    if (w >= this->width) {
        x = 0; // Object is wider than screen, align left
    } else {
        x = (this->width - w) / 2;
    }

    // Determine vertical center
    if (h >= this->height) {
        y = 0; // Object is taller than screen, align top
    } else {
        y = (this->height - h) / 2;
    }
}
