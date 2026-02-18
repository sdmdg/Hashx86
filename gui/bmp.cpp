/**
 * @file        bmp.cpp
 * @brief       Bitmap Handler (part of #x86 GUI Framework)
 *
 * @date        10/01/2026
 * @version     1.0.0
 */

#include <console.h>
#include <core/filesystem/FAT32.h>
#include <core/filesystem/msdospart.h>
#include <gui/bmp.h>

Bitmap::Bitmap(File* file) {
    // Initialize defaults
    this->valid = false;
    this->buffer = 0;
    this->width = 0;
    this->height = 0;
    Load(file);
}

Bitmap::Bitmap(char* path) {
    this->valid = false;
    this->buffer = 0;
    this->width = 0;
    this->height = 0;

    FAT32* fs = MSDOSPartitionTable::activeInstance->partitions[0];
    if (!fs) return;

    File* file = fs->Open(path);

    if (file == 0) {
        printf("BMP Error: File not found %s\n", path);
        return;
    }

    if (file->size == 0) {
        printf("BMP Error: File is empty %s\n", path);
        file->Close();
        delete file;
        return;
    }

    Load(file);
    file->Close();
    delete file;
}

Bitmap::Bitmap(int width, int height, uint32_t color) {
    this->valid = false;
    this->width = width;
    this->height = height;
    this->buffer = 0;

    if (width == 0 || height == 0) {
        printf("BMP Error: width or height is 0.\n");
        return;
    }

    // Allocate buffer
    this->buffer = new uint32_t[width * height];
    if (!this->buffer) {
        HALT("CRITICAL: Failed to allocate bitmap buffer!\n");
    }

    // Fill with color
    for (int i = 0; i < width * height; i++) {
        this->buffer[i] = color;
    }

    this->valid = true;
}

Bitmap::~Bitmap() {
    if (buffer) {
        delete[] buffer;
        buffer = 0;
    }
}

void Bitmap::Load(File* file) {
    if (file == 0) return;

    // Allocate Buffer for the raw file
    uint8_t* rawFile = new uint8_t[file->size];
    if (!rawFile) {
        HALT("CRITICAL: Failed to allocate bitmap raw file buffer!\n");
    }

    // Read entire file into RAM
    file->Seek(0);
    int bytesRead = file->Read(rawFile, file->size);

    if (bytesRead != file->size) {
        printf("BMP Warning: Read %d bytes, expected %d\n", bytesRead, file->size);
    }

    // Parse Headers
    BitmapFileHeader* fileHeader = (BitmapFileHeader*)rawFile;
    BitmapInfoHeader* infoHeader = (BitmapInfoHeader*)(rawFile + sizeof(BitmapFileHeader));

    // Validate
    if (fileHeader->type != 0x4D42) {  // 'BM'
        printf("BMP Error: Invalid signature 0x%x\n", fileHeader->type);
        delete[] rawFile;
        return;
    }

    if (infoHeader->bitCount != 24 && infoHeader->bitCount != 32) {
        printf("BMP Error: Only 24/32-bit supported (got %d)\n", infoHeader->bitCount);
        delete[] rawFile;
        return;
    }

    this->width = infoHeader->width;
    this->height = infoHeader->height;

    bool isTopDown = false;
    if (this->height < 0) {
        this->height = -this->height;
        isTopDown = true;
    }

    // Allocate Pixel Buffer
    this->buffer = new uint32_t[width * height];
    if (!this->buffer) {
        HALT("CRITICAL: Failed to allocate bitmap pixel buffer!\n");
    }

    // Decode
    uint8_t* pixelData = rawFile + fileHeader->offBits;
    int bytesPerPixel = infoHeader->bitCount / 8;
    int rowPadding = (4 - (width * bytesPerPixel) % 4) % 4;

    for (int y = 0; y < height; y++) {
        int targetY = isTopDown ? y : (height - 1 - y);

        for (int x = 0; x < width; x++) {
            uint8_t b = *pixelData++;
            uint8_t g = *pixelData++;
            uint8_t r = *pixelData++;
            uint8_t a = 255;

            // CORRECTED ALPHA CHECK
            if (infoHeader->bitCount == 32) {
                a = *pixelData++;
            }

            // Combine to 0xAARRGGBB
            uint32_t color = ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
            this->buffer[targetY * width + x] = color;
        }
        pixelData += rowPadding;
    }

    delete[] rawFile;
    this->valid = true;
    printf("BMP Loaded: %dx%d\n", width, height);
}
