#ifndef FILE_H
#define FILE_H

#include <console.h>
#include <types.h>

class FAT32;

class File {
public:
    File();
    ~File();

    // Standard Metadata
    char name[128];
    uint32_t size;
    uint32_t id;     // Usually the First Cluster Number
    uint32_t flags;  // 1=Dir, 2=ReadOnly, etc.

    // For Reading
    uint32_t position;  // Current cursor position in the file

    // The Driver that handles this file
    FAT32* filesystem;

    // --- Operations ---
    // Reads 'length' bytes from current 'position' into buffer
    // Returns number of bytes actually read. Updates 'position'.
    int Read(uint8_t* buffer, uint32_t length);

    // Moves the cursor
    void Seek(uint32_t pos);

    // Writes
    void Write(uint8_t* buffer, uint32_t length);

    void Close();
};

#endif
