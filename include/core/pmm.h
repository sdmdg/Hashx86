#ifndef PMM_H
#define PMM_H

#include <core/memory.h>
#include <debug.h>
#include <types.h>

typedef uint32_t PMM_PHYSICAL_ADDRESS;
#define PMM_BLOCK_SIZE 4096  // 4096 Bytes / 4KB

typedef struct {
    uint32_t memory_size;
    uint32_t max_blocks;
    uint32_t* memory_map_array;
    uint32_t memory_map_end;
    uint32_t used_blocks;
} PMM_INFO;

uint32_t pmm_get_max_blocks();
uint32_t pmm_get_used_blocks();

int pmm_mmap_first_free();

// find first free number of frames(size) and return its index
int pmm_mmap_first_free_by_size(uint32_t size);

/**
 * returns index of bitmap array if no of bits(size) are free
 */
int pmm_next_free_frame(int size);

/**
 * initialize memory bitmap array by making blocks from total memory size
 *
 */
void pmm_init(PMM_PHYSICAL_ADDRESS bitmap, uint32_t total_memory_size);
/**
 * initialize/request for a free region of region_size from pmm
 */
void pmm_init_region(PMM_PHYSICAL_ADDRESS base, uint32_t region_size);

/**
 * de-initialize/free allocated region of region_size from pmm
 */
void pmm_deinit_region(PMM_PHYSICAL_ADDRESS base, uint32_t region_size);

/**
 * request to allocate a single block of memory from pmm
 */
void* pmm_alloc_block();

/**
 * free given requested single block of memory from pmm
 */
void pmm_free_block(void* p);

/**
 * request to allocate no of blocks of memory from pmm
 */
void* pmm_alloc_blocks(uint32_t size);

/**
 * free given requested no of blocks of memory from pmm
 */
void pmm_free_blocks(void* p, uint32_t size);

#endif
