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
    if (this->filesystem == 0 || buffer == 0 || length == 0) return 0;

    bool isDirectory = (this->flags & 1) != 0;

    if (!isDirectory) {
        // Safety for regular files
        if (this->position >= this->size) return 0;

        if (this->position + length > this->size) {
            length = this->size - this->position;
        }
    }

    uint32_t bytesRead = this->filesystem->ReadStream(this, buffer, length);
    this->position += bytesRead;

    return (int)bytesRead;
}

void File::Seek(uint32_t pos) {
    this->position = pos;
    if (this->position > this->size) this->position = this->size;
}

void File::Write(uint8_t* buffer, uint32_t length) {}

void File::Close() {
    // Cleanup
}

namespace {
constexpr uint32_t FD_MIN = 3;
constexpr uint32_t FD_MAX = 128;
File* g_fdTable[FD_MAX] = {nullptr};
}  // namespace

int32_t AllocateFd(File* file) {
    if (!file) return -1;
    for (uint32_t fd = FD_MIN; fd < FD_MAX; fd++) {
        if (!g_fdTable[fd]) {
            g_fdTable[fd] = file;
            return (int32_t)fd;
        }
    }
    return -1;
}

File* GetFileByFd(uint32_t fd) {
    if (fd < FD_MIN || fd >= FD_MAX) return nullptr;
    return g_fdTable[fd];
}

void ReleaseFd(uint32_t fd) {
    if (fd < FD_MIN || fd >= FD_MAX) return;
    g_fdTable[fd] = nullptr;
}
