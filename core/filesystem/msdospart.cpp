#include <core/filesystem/msdospart.h>

FAT32* MSDOSPartitionTable::partitions[4] = { 0, 0, 0, 0 };
MSDOSPartitionTable* MSDOSPartitionTable::activeInstance = nullptr;

MSDOSPartitionTable::MSDOSPartitionTable(AdvancedTechnologyAttachment* ata)
{
    this->ata = ata;
    this->activeInstance = this;
};
MSDOSPartitionTable::~MSDOSPartitionTable()
{
};

void MSDOSPartitionTable::Initialize()
{
    printf("Initializing Disk...\n");

    // Get Drive Size from ATA
    uint32_t totalSectors = ata->GetSizeInSectors();
    if(totalSectors == 0) {
        printf("Error: Could not identify drive size.\n");
        return;
    }

    // Calculate Partitions (Split in 2)
    // Reserve 63 sectors for MBR and alignment
    uint32_t available = totalSectors - 63;
    uint32_t p1_size = available / 2;
    uint32_t p2_size = available - p1_size; // Remainder

    uint32_t p1_start = 63;
    uint32_t p2_start = 63 + p1_size;

    printf("Partition 1: Start %d, Size %d\n", p1_start, p1_size);
    printf("Partition 2: Start %d, Size %d\n", p2_start, p2_size);

    // Create MBR
    MasterBootRecord mbr;
    // Zero out bootloader
    uint8_t* ptr = (uint8_t*)&mbr;
    for(int i=0; i<446; i++) ptr[i] = 0;

    mbr.signature = 0;
    mbr.unused = 0;
    mbr.magicnumber = 0xAA55;

    // Partition 1 Entry
    mbr.primaryPartition[0].bootable = 0x80; // Active
    mbr.primaryPartition[0].partition_id = 0x0C; // FAT32 LBA
    mbr.primaryPartition[0].start_lba = p1_start;
    mbr.primaryPartition[0].length = p1_size;
    mbr.primaryPartition[0].start_head = 0; // Legacy unused
    mbr.primaryPartition[0].end_head = 0;

    // Partition 2 Entry
    mbr.primaryPartition[1].bootable = 0x00;
    mbr.primaryPartition[1].partition_id = 0x0C; // FAT32 LBA
    mbr.primaryPartition[1].start_lba = p2_start;
    mbr.primaryPartition[1].length = p2_size;
    mbr.primaryPartition[1].start_head = 0;
    mbr.primaryPartition[1].end_head = 0;

    // Zero out entries 3 and 4
    for(int i=2; i<4; i++) {
        mbr.primaryPartition[i].partition_id = 0;
        mbr.primaryPartition[i].start_lba = 0;
        mbr.primaryPartition[i].length = 0;
        mbr.primaryPartition[i].bootable = 0;
    }

    // Write MBR
    ata->Write28(0, (uint8_t*)&mbr, 512);

    // 5. FORMAT Partitions
    FAT32::FormatRaw(ata, p1_start, p1_size);
    FAT32::FormatRaw(ata, p2_start, p2_size);

    printf("Initialization Complete.\n");
    
    return;
}

void MSDOSPartitionTable::ReadPartitions()
{
    MasterBootRecord mbr;
    ata->Read28(0, (uint8_t*)&mbr, sizeof(MasterBootRecord));

    // Check Signature. If invalid, Initialize drive.
    if (mbr.magicnumber != 0xAA55)
    {
        printf("MBR Invalid. Initializing Drive...\n");
        Initialize();
        printf("Please copy the OS data files using 'make hdd' command.\n");
        asm volatile ("hlt");
        while (1);
    }

    for(int i = 0; i < 4; i++)
    {
        if(mbr.primaryPartition[i].partition_id == 0) continue;

        printf("Partition %d ", i);
        if(mbr.primaryPartition[i].bootable == 0x80) printf("[Bootable] ");
        printf("Type 0x%x Start %d\n", mbr.primaryPartition[i].partition_id, mbr.primaryPartition[i].start_lba);

        // Mount FAT32
        if(mbr.primaryPartition[i].partition_id == 0x0C || mbr.primaryPartition[i].partition_id == 0x0B) 
        {
            FAT32* fat32 = new FAT32(ata, mbr.primaryPartition[i].start_lba);
            this->partitions[partitionsCounter++] = fat32;

/*             fat32.CreateFile("file1.txt");
            fat32.MakeDirectory("DIR1");
            fat32.CreateFile("DIR1/file1.txt");
            fat32.WriteFile("DIR1/file1.txt", (uint8_t*)"Hello, World!", 13);
            uint8_t buffer[101];
            fat32.ReadFile((char*)"DIR1/file1.txt", buffer, 33);
            printf("%s\n", (char*)buffer);
 */
        }
    }
}