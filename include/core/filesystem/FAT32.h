#ifndef FAT32_H
#define FAT32_H

#include <core/drivers/ata.h>
#include <core/filesystem/File.h>
#include <core/memory.h>
#include <debug.h>
#include <types.h>
#include <utils/string.h>

// Standard FAT32 Structures
struct BiosParameterBlock32 {
    uint8_t jump[3];
    uint8_t softName[8];
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t fatCopies;
    uint16_t rootDirEntries;
    uint16_t totalSectors;
    uint8_t mediaType;
    uint16_t fatSectorCount;
    uint16_t sectorsPerTrack;
    uint16_t headCount;
    uint32_t hiddenSectors;
    uint32_t totalSectorCount;
    uint32_t tableSize;
    uint16_t extFlags;
    uint16_t fatVersion;
    uint32_t rootCluster;
    uint16_t fatInfo;
    uint16_t backupSector;
    uint8_t reserved[12];
    uint8_t driveNumber;
    uint8_t reserved2;
    uint8_t bootSignature;
    uint32_t volumeID;
    uint8_t fatTypeLabel[8];
    uint8_t fileSystemType[8];
} __attribute__((packed));

struct DirectoryEntryFat32 {
    uint8_t name[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t cTimeTenth;
    uint16_t cTime;
    uint16_t cDate;
    uint16_t aDate;
    uint16_t firstClusterHi;
    uint16_t wTime;
    uint16_t wDate;
    uint16_t firstClusterLow;
    uint32_t size;
} __attribute__((packed));

class FAT32 {
public:
    FAT32(AdvancedTechnologyAttachment* hd, uint32_t partitionOffset);
    ~FAT32();

    // User API
    File* Open(char* path);
    void ReadStream(File* file, uint8_t* buffer, uint32_t length);
    void ListRoot();
    void ListDir(char* path);
    void CreateFile(char* path);
    void MakeDirectory(char* path);
    void DeleteFile(char* path);
    void DeleteDirectory(char* path);
    void ReadFile(char* path, uint8_t* buffer, uint32_t length);
    void WriteFile(char* path, uint8_t* buffer, uint32_t length);
    uint32_t GetFileSize(char* path);

    void Format();
    static void FormatRaw(AdvancedTechnologyAttachment* hd, uint32_t startSector,
                          uint32_t sizeSectors);

private:
    AdvancedTechnologyAttachment* hd;
    BiosParameterBlock32 bpb;

    uint32_t partitionOffset;
    uint32_t fatStart;
    uint32_t dataStart;
    uint32_t rootStart;
    bool valid;

    // --- Helpers ---
    uint32_t ClusterToSector(uint32_t cluster);
    uint32_t GetFATEntry(uint32_t cluster);
    void SetFATEntry(uint32_t cluster, uint32_t value);
    uint32_t AllocateCluster();
    void FreeChain(uint32_t startCluster);

    // --- Directory Helpers ---
    bool FindEntryInCluster(uint32_t cluster, char* name, uint32_t& sectorOut, uint32_t& offsetOut,
                            DirectoryEntryFat32& entryOut);
    bool FindFreeEntryInCluster(uint32_t dirCluster, uint32_t& sectorOut, uint32_t& offsetOut);
    bool IsDirectoryEmpty(uint32_t dirCluster);

    // --- Path Logic ---
    uint32_t ParsePath(char* path, char* filenameOut);
    uint32_t ResolvePath(char* path);

    // --- Utils ---
    void StringToFatName(char* input, char* outName, char* outExt);
};

#endif  // FAT32_H
