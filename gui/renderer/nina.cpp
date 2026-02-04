/**
 * @file        nina.cpp
 * @brief       NINA UI Renderer for #x86
 *
 * @date        10/01/2026
 * @version     1.0.0-beta
 */

#include <gui/renderer/nina.h>

NINA* NINA::activeInstance = nullptr;

NINA::NINA() {
    PrecomputeAlphaTable();
    activeInstance = this;
}

void NINA::PrecomputeAlphaTable() {
    for (int c = 0; c < 256; c++) {
        for (int a = 0; a < 256; a++) {
            this->alphaTable[a][c] = (c * a) / 255;
        }
    }
}

NINA::~NINA() {}

void NINA::DrawBitmapToBuffer(uint32_t* dst, int dstW, int dstH, int dstX, int dstY, uint32_t* src,
                              int srcW, int srcH) {
    for (int y = 0; y < srcH; ++y) {
        int dy = dstY + y;
        if (dy < 0 || dy >= dstH) continue;

        uint32_t* dstRow = dst + dy * dstW + dstX;
        uint32_t* srcRow = src + y * srcW;

        for (int x = 0; x < srcW; ++x) {
            int dx = dstX + x;
            if (dx < 0 || dx >= dstW) continue;

            uint32_t srcPixel = srcRow[x];
            uint8_t srcA = (srcPixel >> 24) & 0xFF;

            if (srcA == 255) {
                // Fully opaque — just copy
                dstRow[x] = srcPixel;
            } else if (srcA > 0) {
                // Blend using alpha table
                uint32_t dstPixel = dstRow[x];

                uint8_t srcR = (srcPixel >> 16) & 0xFF;
                uint8_t srcG = (srcPixel >> 8) & 0xFF;
                uint8_t srcB = srcPixel & 0xFF;

                uint8_t dstR = (dstPixel >> 16) & 0xFF;
                uint8_t dstG = (dstPixel >> 8) & 0xFF;
                uint8_t dstB = dstPixel & 0xFF;

                uint8_t invA = 255 - srcA;

                uint8_t outR = alphaTable[srcA][srcR] + alphaTable[invA][dstR];
                uint8_t outG = alphaTable[srcA][srcG] + alphaTable[invA][dstG];
                uint8_t outB = alphaTable[srcA][srcB] + alphaTable[invA][dstB];

                dstRow[x] = (0xFF << 24) | (outR << 16) | (outG << 8) | outB;
            }
            // else: srcA == 0, fully transparent, skip
        }
    }
}

void NINA::DrawBitmap(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x,
                      int32_t y, const uint32_t* bitmapData, int32_t bitmapWidth,
                      int32_t bitmapHeight) {
    for (int32_t row = 0; row < bitmapHeight; ++row) {
        int32_t screenY = y + row;
        if (screenY < 0 || screenY >= bufferHeight) continue;

        uint32_t* pixelPtr = &buffer[screenY * bufferWidth + x];
        const uint32_t* bmpPtr = &bitmapData[row * bitmapWidth];

        int32_t col = 0;

        // Fast-path memcpy if fully opaque and fits
        while (col < bitmapWidth) {
            if ((x + col) >= bufferWidth) break;

            int32_t runStart = col;
            while (col < bitmapWidth && ((x + col) < bufferWidth) && ((bmpPtr[col] >> 24) == 255)) {
                col++;
            }

            int runLength = col - runStart;
            if (runLength > 0) {
                memcpy(&pixelPtr[runStart], &bmpPtr[runStart], runLength * sizeof(uint32_t));
            }

            // Blending for rest
            while (col < bitmapWidth && (x + col) < bufferWidth) {
                uint32_t srcColor = bmpPtr[col];
                uint8_t alpha = (srcColor >> 24) & 0xFF;

                if (alpha > 0) {
                    uint32_t dstColor = pixelPtr[col];
                    uint32_t invAlpha = 255 - alpha;

                    uint8_t srcRed = (srcColor >> 16) & 0xFF;
                    uint8_t srcGreen = (srcColor >> 8) & 0xFF;
                    uint8_t srcBlue = srcColor & 0xFF;

                    uint8_t dstRed = (dstColor >> 16) & 0xFF;
                    uint8_t dstGreen = (dstColor >> 8) & 0xFF;
                    uint8_t dstBlue = dstColor & 0xFF;

                    uint8_t blendedRed = alphaTable[alpha][srcRed] + alphaTable[invAlpha][dstRed];
                    uint8_t blendedGreen =
                        alphaTable[alpha][srcGreen] + alphaTable[invAlpha][dstGreen];
                    uint8_t blendedBlue =
                        alphaTable[alpha][srcBlue] + alphaTable[invAlpha][dstBlue];

                    pixelPtr[col] =
                        (255 << 24) | (blendedRed << 16) | (blendedGreen << 8) | blendedBlue;
                }
                col++;
            }
        }
    }
}

void NINA::FillRectangle(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x,
                         int32_t y, uint32_t w, uint32_t h, uint32_t colorIndex) {
    int32_t startX = (x < 0) ? 0 : x;
    int32_t startY = (y < 0) ? 0 : y;
    int32_t endX = ((x + w) > bufferWidth) ? bufferWidth : (x + w);
    int32_t endY = ((y + h) > bufferHeight) ? bufferHeight : (y + h);

    for (int32_t Y = startY; Y < endY; ++Y) {
        for (int32_t X = startX; X < endX; ++X) {
            buffer[Y * bufferWidth + X] = colorIndex;
        }
    }
}

void NINA::DrawRectangle(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x,
                         int32_t y, uint32_t w, uint32_t h, uint32_t colorIndex) {
    int32_t startX = (x < 0) ? 0 : x;
    int32_t startY = (y < 0) ? 0 : y;
    int32_t endX = (x + w > bufferWidth) ? bufferWidth : x + w;
    int32_t endY = (y + h > bufferHeight) ? bufferHeight : y + h;

    for (int32_t X = startX; X < endX; X++) {
        if (startY >= 0 && startY < bufferHeight) buffer[startY * bufferWidth + X] = colorIndex;
        if (endY - 1 >= 0 && endY - 1 < bufferHeight)
            buffer[(endY - 1) * bufferWidth + X] = colorIndex;
    }

    for (int32_t Y = startY; Y < endY; Y++) {
        if (startX >= 0 && startX < bufferWidth) buffer[Y * bufferWidth + startX] = colorIndex;
        if (endX - 1 >= 0 && endX - 1 < bufferWidth)
            buffer[Y * bufferWidth + (endX - 1)] = colorIndex;
    }
}

void NINA::FillRoundedRectangle(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight,
                                int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t radius,
                                uint32_t colorIndex) {
    FillRectangle(buffer, bufferWidth, bufferHeight, x + radius, y, w - 2 * radius, h, colorIndex);
    FillRectangle(buffer, bufferWidth, bufferHeight, x, y + radius, w, h - 2 * radius, colorIndex);

    for (int32_t dy = 0; dy <= static_cast<int32_t>(radius); ++dy) {
        for (int32_t dx = 0; dx <= static_cast<int32_t>(radius); ++dx) {
            if ((dx * dx + dy * dy) <= static_cast<int32_t>(radius * radius)) {
                int32_t tlX = x + radius - dx;
                int32_t tlY = y + radius - dy;
                int32_t trX = x + w - radius + dx - 1;
                int32_t brY = y + h - radius + dy - 1;

                if (tlX >= 0 && tlY >= 0 && tlX < bufferWidth && tlY < bufferHeight)
                    buffer[tlY * bufferWidth + tlX] = colorIndex;
                if (trX >= 0 && tlY >= 0 && trX < bufferWidth && tlY < bufferHeight)
                    buffer[tlY * bufferWidth + trX] = colorIndex;
                if (tlX >= 0 && brY >= 0 && tlX < bufferWidth && brY < bufferHeight)
                    buffer[brY * bufferWidth + tlX] = colorIndex;
                if (trX >= 0 && brY >= 0 && trX < bufferWidth && brY < bufferHeight)
                    buffer[brY * bufferWidth + trX] = colorIndex;
            }
        }
    }
}

void NINA::DrawRoundedRectangle(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight,
                                int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t radius,
                                uint32_t colorIndex) {
    for (int32_t i = x + radius; i < x + w - radius; ++i) {
        if (i >= 0 && i < bufferWidth) {
            if (y >= 0 && y < bufferHeight) buffer[y * bufferWidth + i] = colorIndex;
            if ((y + h - 1) >= 0 && (y + h - 1) < bufferHeight)
                buffer[(y + h - 1) * bufferWidth + i] = colorIndex;
        }
    }

    for (int32_t i = y + radius; i < y + h - radius; ++i) {
        if (i >= 0 && i < bufferHeight) {
            if (x >= 0 && x < bufferWidth) buffer[i * bufferWidth + x] = colorIndex;
            if ((x + w - 1) >= 0 && (x + w - 1) < bufferWidth)
                buffer[i * bufferWidth + (x + w - 1)] = colorIndex;
        }
    }

    for (int32_t dy = 0; dy <= static_cast<int32_t>(radius); ++dy) {
        for (int32_t dx = 0; dx <= static_cast<int32_t>(radius); ++dx) {
            int32_t d2 = dx * dx + dy * dy;
            if (d2 >= static_cast<int32_t>((radius - 1) * (radius - 1)) &&
                d2 <= static_cast<int32_t>(radius * radius)) {
                int32_t tlX = x + radius - dx;
                int32_t tlY = y + radius - dy;
                int32_t trX = x + w - radius + dx - 1;
                int32_t brY = y + h - radius + dy - 1;

                if (tlX >= 0 && tlY >= 0 && tlX < bufferWidth && tlY < bufferHeight)
                    buffer[tlY * bufferWidth + tlX] = colorIndex;
                if (trX >= 0 && tlY >= 0 && trX < bufferWidth && tlY < bufferHeight)
                    buffer[tlY * bufferWidth + trX] = colorIndex;
                if (tlX >= 0 && brY >= 0 && tlX < bufferWidth && brY < bufferHeight)
                    buffer[brY * bufferWidth + tlX] = colorIndex;
                if (trX >= 0 && brY >= 0 && trX < bufferWidth && brY < bufferHeight)
                    buffer[brY * bufferWidth + trX] = colorIndex;
            }
        }
    }
}

void NINA::FillCircle(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t cx,
                      int32_t cy, uint32_t radius, uint32_t colorIndex) {
    int32_t x = radius;
    int32_t y = 0;
    int32_t err = 0;

    while (x >= y) {
        for (int32_t dx = cx - x; dx <= cx + x; ++dx) {
            if (cy + y >= 0 && cy + y < bufferHeight && dx >= 0 && dx < bufferWidth)
                buffer[(cy + y) * bufferWidth + dx] = colorIndex;
            if (cy - y >= 0 && cy - y < bufferHeight && dx >= 0 && dx < bufferWidth)
                buffer[(cy - y) * bufferWidth + dx] = colorIndex;
        }
        for (int32_t dx = cx - y; dx <= cx + y; ++dx) {
            if (cy + x >= 0 && cy + x < bufferHeight && dx >= 0 && dx < bufferWidth)
                buffer[(cy + x) * bufferWidth + dx] = colorIndex;
            if (cy - x >= 0 && cy - x < bufferHeight && dx >= 0 && dx < bufferWidth)
                buffer[(cy - x) * bufferWidth + dx] = colorIndex;
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

void NINA::DrawCircle(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t cx,
                      int32_t cy, uint32_t radius, uint32_t colorIndex) {
    int32_t x = radius;
    int32_t y = 0;
    int32_t err = 0;

    while (x >= y) {
        auto set = [&](int32_t px, int32_t py) {
            if (px >= 0 && py >= 0 && px < bufferWidth && py < bufferHeight)
                buffer[py * bufferWidth + px] = colorIndex;
        };

        set(cx + x, cy + y);
        set(cx + y, cy + x);
        set(cx - y, cy + x);
        set(cx - x, cy + y);
        set(cx - x, cy - y);
        set(cx - y, cy - x);
        set(cx + y, cy - x);
        set(cx + x, cy - y);

        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

void NINA::DrawLine(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x0,
                    int32_t y0, int32_t x1, int32_t y1, uint32_t color) {
    // Bresenham's Line Algorithm
    int32_t dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);  // abs(x1 - x0)
    int32_t dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);  // abs(y1 - y0)

    int32_t sx = (x0 < x1) ? 1 : -1;
    int32_t sy = (y0 < y1) ? 1 : -1;

    int32_t err = dx - dy;

    while (true) {
        // Bounds check is critical to prevent kernel crashes
        if (x0 >= 0 && x0 < bufferWidth && y0 >= 0 && y0 < bufferHeight) {
            buffer[y0 * bufferWidth + x0] = color;
        }

        if (x0 == x1 && y0 == y1) break;

        int32_t e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void NINA::DrawHorizontalLine(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight,
                              int32_t x, int32_t y, int32_t length, uint32_t colorIndex) {
    if (y < 0 || y >= bufferHeight) return;
    for (int32_t i = 0; i < length; ++i) {
        int32_t px = x + i;
        if (px >= 0 && px < bufferWidth) buffer[y * bufferWidth + px] = colorIndex;
    }
}

void NINA::DrawVerticalLine(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x,
                            int32_t y, int32_t length, uint32_t colorIndex) {
    if (x < 0 || x >= bufferWidth) return;
    for (int32_t i = 0; i < length; ++i) {
        int32_t py = y + i;
        if (py >= 0 && py < bufferHeight) buffer[py * bufferWidth + x] = colorIndex;
    }
}

void NINA::DrawCharacter(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x,
                         int32_t y, char c, Font* font, uint32_t colorIndex) {
    int idx = (uint8_t)c - 32;
    int16_t* g = &font->font_glyphs[idx * 8];  // each glyph = 8 values

    int charID = g[0];
    int gridX = g[1];
    int gridY = g[2];
    int charWidth = g[3];
    int charHeight = g[4];
    int xoffset = g[5];
    int yoffset = g[6];
    int xadvance = g[7];

    // Allocate cropped bitmap
    uint32_t croppedBitmap[charWidth * charHeight];

    for (int row = 0; row < charHeight; ++row) {
        for (int col = 0; col < charWidth; ++col) {
            uint32_t fontPixel =
                font->font_atlas[(gridY + row) * font->atlas_width + (gridX + col)];
            uint8_t fontAlpha = (fontPixel >> 24) & 0xFF;

            uint8_t inputAlpha = (colorIndex >> 24) & 0xFF;
            uint8_t inputRed = (colorIndex >> 16) & 0xFF;
            uint8_t inputGreen = (colorIndex >> 8) & 0xFF;
            uint8_t inputBlue = colorIndex & 0xFF;

            uint8_t blendedAlpha = (fontAlpha * inputAlpha) / 255;
            uint32_t blendedColor =
                (blendedAlpha << 24) | (inputRed << 16) | (inputGreen << 8) | inputBlue;

            croppedBitmap[row * charWidth + col] = blendedColor;
        }
    }

    // Apply offsets from glyph table
    DrawBitmap(buffer, bufferWidth, bufferHeight, x + xoffset, y + yoffset, croppedBitmap,
               charWidth, charHeight);
}

void NINA::DrawString(uint32_t* buffer, int32_t bufferWidth, int32_t bufferHeight, int32_t x,
                      int32_t y, const char* str, Font* font, uint32_t colorIndex) {
    int penX = x;
    int penY = y;

    for (int i = 0; str[i] != '\0'; ++i) {
        char c = str[i];

        // Handle newline
        if (c == '\n') {
            penX = x;
            penY += font->getLineHeight();
            continue;
        }

        char next = str[i + 1];

        // Clamp unsupported characters
        if (c < 32 || c > 126) {
            c = '?';
        }

        int16_t* g = &font->font_glyphs[((uint8_t)c - 32) * 8];
        int xadvance = g[7];

        // Kerning lookup
        int kernAdjust = 0;
        if (next >= 32) {
            for (int k = 0; k < font->font_kerning_count; k++) {
                int16_t* kdata = &font->font_kernings[k * 3];
                if (kdata[0] == (uint8_t)c && kdata[1] == (uint8_t)next) {
                    kernAdjust = kdata[2];
                    break;
                }
            }
        }

        // Draw this character at current pen position
        DrawCharacter(buffer, bufferWidth, bufferHeight, penX, penY, c, font, colorIndex);

        // Advance pen
        penX += xadvance + kernAdjust;
    }
}
