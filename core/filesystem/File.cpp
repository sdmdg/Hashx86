/**
 * @file        File.cpp
 * @brief       File System Abstraction
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <core/filesystem/FAT32.h>
#include <core/filesystem/File.h>

File::File() {
    this->size = 0;
    this->id = 0;
    this->position = 0;
    this->filesystem = 0;
    this->flags = 0;
    for (int i = 0; i < 128; i++) this->name[i] = 0;
}

File::~File() {
    if (this->filesystem) {
        this->Close();
    }
}

int File::Read(uint8_t* buffer, uint32_t length) {
    if (this->filesystem == 0) return 0;

    // Safety
    if (this->position >= this->size) return 0;

    if (this->position + length > this->size) {
        length = this->size - this->position;
    }

    this->filesystem->ReadStream(this, buffer, length);

    this->position += length;
    return length;
}

void File::Seek(uint32_t pos) {
    this->position = pos;
    if (this->position > this->size) this->position = this->size;
}

void File::Write(uint8_t* buffer, uint32_t length) {}

void File::Close() {
    // Cleanup
}
