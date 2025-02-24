/**
 * @file        vga.cpp
 * @brief       VGA Graphics for #x86 (Discontinued)
 * 
 * @date        19/01/2025
 * @version     1.0.0-final (Discontinued)
 */

#include <core/drivers/vga.h>

static uint8_t backBuffer[320 * 200];
uint8_t* frameBuffer;

VideoGraphicsArray::VideoGraphicsArray() : 
    miscPort(0x3c2),
    crtcIndexPort(0x3d4),
    crtcDataPort(0x3d5),
    sequencerIndexPort(0x3c4),
    sequencerDataPort(0x3c5),
    graphicsControllerIndexPort(0x3ce),
    graphicsControllerDataPort(0x3cf),
    attributeControllerIndexPort(0x3c0),
    attributeControllerReadPort(0x3c1),
    attributeControllerWritePort(0x3C0),
    attributeControllerResetPort(0x3DA),
    paletteIndexPort(0x3C8),
    paletteDataPort(0x3C9)
{
    // Initialize off-screen buffer (back buffer)
    frameBuffer = (uint8_t*)0xA0000;
}

VideoGraphicsArray::~VideoGraphicsArray()
{
}


// Setup VGA

void VideoGraphicsArray::WriteRegisters(uint8_t* registers)
{
    // Write to the Miscellaneous Output Register
    miscPort.Write(*(registers++));

    // Write to the Sequencer Registers
    for (uint8_t i = 0; i < 5; i++)
    {
        sequencerIndexPort.Write(i);
        sequencerDataPort.Write(*(registers++));
    }

    // Unlock CRTC registers
    crtcIndexPort.Write(0x03);
    crtcDataPort.Write(crtcDataPort.Read() | 0x80);
    crtcIndexPort.Write(0x11);
    crtcDataPort.Write(crtcDataPort.Read() & ~0x80);

    // Write to CRTC Registers
    for (uint8_t i = 0; i < 25; i++)
    {
        crtcIndexPort.Write(i);
        crtcDataPort.Write(*(registers++));
    }

    // Write to the Graphics Controller Registers
    for (uint8_t i = 0; i < 9; i++)
    {
        graphicsControllerIndexPort.Write(i);
        graphicsControllerDataPort.Write(*(registers++));
    }

    // Write to the Attribute Controller Registers
    for (uint8_t i = 0; i < 21; i++)
    {
        attributeControllerResetPort.Read(); // Reset flip-flop
        attributeControllerIndexPort.Write(i);
        attributeControllerWritePort.Write(*(registers++));
    }

    // Signal end of Attribute Controller writes
    attributeControllerResetPort.Read();
    attributeControllerIndexPort.Write(0x20);
}

bool VideoGraphicsArray::SupportsMode(uint32_t width, uint32_t height, uint32_t colordepth)
{
    return  width == 320 && height == 200 && colordepth == 8;
}

void VideoGraphicsArray::SetVGAPalette(const uint8_t palette[256][3]) {
    // Write the starting index to the Palette Write Index Register
    paletteIndexPort.Write(0); // Start at index 0

    // Write each of the 256 colors to the Palette Data Register
    for (int i = 0; i < 256; i++) {
        paletteDataPort.Write(palette[i][0]); // Red
        paletteDataPort.Write(palette[i][1]); // Green
        paletteDataPort.Write(palette[i][2]); // Blue
    }
}

uint8_t VideoGraphicsArray::GetClosestColorIndex(uint8_t r, uint8_t g, uint8_t b) {
    int closestIndex = -1;
    int closestDistance = 0x7FFFFFFF;

    for (int i = 0; i < 256; ++i) {
        // Calculate the squared distance
        int dr = palette[i][0] - r;
        int dg = palette[i][1] - g;
        int db = palette[i][2] - b;
        int distance = dr * dr + dg * dg + db * db; // Squared distance

        // Check if this distance is smaller
        if (distance < closestDistance) {
            closestDistance = distance;
            closestIndex = i;
        }
    }

    return closestIndex; // Returns the index of the closest match
}

bool VideoGraphicsArray::SetMode(uint32_t width, uint32_t height, uint32_t colordepth)
{
    if (width != 320 || height != 200 || colordepth != 8)
        return false;

    // Register values for 320x200 mode 13h (256 colors)

    /*
    Intel® OpenSource HD Graphics PRM pdf file
    for following defined data for each vga register
    and its explaination
    */
    unsigned char g_320x200x256[] =
    {
        /* MISC */
        0x63,
        /* SEQ */
        0x03, 0x01, 0x0F, 0x00, 0x0E,
        /* CRTC */
        0x5F, 0x4F, 0x50, 0x82, 
        0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00,
        0x9C, 0x0E, 0x8F, 0x28,	
        0x40, 0x96, 0xB9, 0xA3,
        0xFF,
        /* GC */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
        0xFF,
        /* AC */
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F,
        0x41, 0x00, 0x0F, 0x00,
        0x00     
    };

    WriteRegisters(g_320x200x256);

    SetVGAPalette(palette);

    return true;
}

uint8_t* VideoGraphicsArray::GetFrameBufferSegment()
{
    graphicsControllerIndexPort.Write(0x06);
    uint8_t segmentNumber = graphicsControllerDataPort.Read() & (3<<2);
    switch(segmentNumber)
    {
        default:
        case 0<<2: return (uint8_t*)0x00000;
        case 1<<2: return (uint8_t*)0xA0000;
        case 2<<2: return (uint8_t*)0xB0000;
        case 3<<2: return (uint8_t*)0xB8000;
    }
}

void VideoGraphicsArray::Flush()
{
    // Wait for the start of vertical retrace
    while (!(attributeControllerResetPort.Read() & 0x08)) {
        // Wait until bit 3 is set (start of VSync)
    }

    // Safely update the screen
    for (int y = 0; y < VGA_SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < VGA_SCREEN_WIDTH; ++x) {
            *(frameBuffer + VGA_SCREEN_WIDTH * y + x) = *(backBuffer + VGA_SCREEN_WIDTH * y + x);
        }
    }
    // Wait for the end of vertical retrace
    while (attributeControllerResetPort.Read() & 0x08) {
        // Wait until bit 3 is cleared (end of VSync)
    }
}


// Graphics

void VideoGraphicsArray::PutPixel(int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b)
{
    // Get the color index from the RGB values
    uint8_t colorIndex = GetClosestColorIndex(r, g, b);
    PutPixel(x,y,colorIndex);
}

void VideoGraphicsArray::PutPixel(int32_t x, int32_t y, uint8_t colorIndex)
{
    if (colorIndex != 0xFF) {
        // Bounds check
        if (x < 0 || VGA_SCREEN_WIDTH <= x || y < 0 || VGA_SCREEN_HEIGHT <= y)
            return;

        *(backBuffer + VGA_SCREEN_WIDTH * y + x) = colorIndex;  // Draw to the back buffer
    }
}


void VideoGraphicsArray::DrawBitmap(int32_t x, int32_t y, const uint8_t* bitmapData, int32_t bitmapWidth, int32_t bitmapHeight)
{
    // Loop through each pixel of the bitmap
    for (int32_t row = 0; row < bitmapHeight; ++row)
    {
        for (int32_t col = 0; col < bitmapWidth; ++col)
        {
            // Calculate the index in the bitmap data array
            uint8_t colorIndex = bitmapData[row * bitmapWidth + col];

            // Calculate the screen coordinates (offset by x, y)
            int32_t screenX = x + col;
            int32_t screenY = y + row;

            // Bounds check: Ensure the pixel is within the screen's bounds
            if (screenX >= 0 && screenX < VGA_SCREEN_WIDTH && screenY >= 0 && screenY < VGA_SCREEN_HEIGHT)
            {
                // Draw the pixel to the screen
                PutPixel(screenX, screenY, colorIndex);
            }
        }
    }
}

void VideoGraphicsArray::FillRectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t colorIndex)
{
    for (int32_t Y = y; Y < y + h; Y++) {
        for (int32_t X = x; X < x + w; X++) {
            PutPixel(X, Y, colorIndex);
        }
    }
}

void VideoGraphicsArray::DrawRectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t colorIndex) {
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

void VideoGraphicsArray::FillRoundedRectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t radius, uint8_t colorIndex) {
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

void VideoGraphicsArray::DrawCircle(uint32_t cx, uint32_t cy, uint32_t radius, uint8_t colorIndex) {
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

void VideoGraphicsArray::FillCircle(uint32_t cx, uint32_t cy, uint32_t radius, uint8_t colorIndex) {
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

void VideoGraphicsArray::DrawHorizontalLine(int32_t x, int32_t y, int32_t length, uint8_t colorIndex) {
    for (int32_t i = 0; i < length; ++i) {
        PutPixel(x + i, y, colorIndex);
    }
}

void VideoGraphicsArray::DrawVerticalLine(int32_t x, int32_t y, int32_t length, uint8_t colorIndex) {
    for (int32_t i = 0; i < length; ++i) {
        PutPixel(x, y + i, colorIndex);
    }
}

void VideoGraphicsArray::DrawCharacter(int32_t x, int32_t y, char c, uint8_t colorIndex) {
    // Validate the character range (ASCII 0-127)
    if (c < 0 || c > 127) {
        c = '?'; // Replace unsupported characters with '?'
    }

    // Get the font bitmap for the character (directly from the font array)
    const uint8_t* bitmap = font8x8_basic[c]; // { Discontinued }

    // Draw each row of the character
    for (int32_t row = 0; row < 8; ++row) {
        uint8_t rowBitmap = bitmap[row];

        // Draw each pixel in the row (left to right, no bit reversal)
        for (int32_t col = 0; col < 8; ++col) {
            // Check if the bit is set (pixel is on)
            if (rowBitmap & (1 << col)) {  // Note: Here no need for bit reversal
                // Draw the pixel at the correct location
                PutPixel(x + col, y + row, colorIndex);
            }
        }
    }
}

void VideoGraphicsArray::DrawString(int32_t x, int32_t y, const char* str, uint8_t colorIndex) {
    int32_t cursorX = x;

    // Iterate through the string and draw each character
    while (*str) {
        if (*str == '\n') { // Handle newline characters
            y += 8;        // Move to the next line
            cursorX = x;   // Reset to the starting x position
        } else {
            DrawCharacter(cursorX, y, *str, colorIndex); // Draw the character
            cursorX += 8; // Move to the next character position
        }
        ++str; // Move to the next character
    }
}
