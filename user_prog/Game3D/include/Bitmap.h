#ifndef BITMAP_H
#define BITMAP_H

#include <Hx86/stdint.h>

#pragma pack(push, 1)

struct BitmapFileHeader {
    uint16_t type;  // Magic "BM" (0x4D42)
    uint32_t size;  // File size
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offBits;  // Offset to image data
};

struct BitmapInfoHeader {
    uint32_t size;  // Header size (40 bytes)
    int32_t width;
    int32_t height;
    uint16_t planes;       // Must be 1
    uint16_t bitCount;     // 24 or 32
    uint32_t compression;  // 0 = uncompressed
    uint32_t sizeImage;
    int32_t xPelsPerMeter;
    int32_t yPelsPerMeter;
    uint32_t clrUsed;
    uint32_t clrImportant;
};

#pragma pack(pop)

class Bitmap {
public:
    // Load from a raw file buffer (e.g. from syscall_read_file)
    Bitmap(uint8_t* rawData, uint32_t rawSize);
    // Create a solid-color bitmap
    Bitmap(int width, int height, uint32_t color);
    ~Bitmap();

    bool IsValid() {
        return valid;
    }
    int GetWidth() {
        return width;
    }
    int GetHeight() {
        return height;
    }
    uint32_t* GetBuffer() {
        return buffer;
    }

private:
    int width;
    int height;
    bool valid;
    uint32_t* buffer;

    void LoadFromMemory(uint8_t* rawData, uint32_t rawSize);
};

#endif
