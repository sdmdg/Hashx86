#include <core/pmm.h>
#include <debug.h>
#include <core/memory.h>

PMM_INFO g_pmm_info;

// set bit in memory map array with bounds check
static inline void pmm_mmap_set(int bit) {
    if (bit < g_pmm_info.max_blocks * 32)
        g_pmm_info.memory_map_array[bit / 32] |= (1 << (bit % 32));
}

// unset bit in memory map array with bounds check
static inline void pmm_mmap_unset(int bit) {
    if (bit < g_pmm_info.max_blocks * 32)
        g_pmm_info.memory_map_array[bit / 32] &= ~(1 << (bit % 32));
}

// test if given nth bit is set with bounds check
static inline char pmm_mmap_test(int bit) {
    if (bit < g_pmm_info.max_blocks * 32)
        return g_pmm_info.memory_map_array[bit / 32] & (1 << (bit % 32));
    return 0;
}

uint32_t pmm_get_max_blocks() {
    return g_pmm_info.max_blocks;
}

uint32_t pmm_get_used_blocks() {
    return g_pmm_info.used_blocks;
}

// find first free frame in bitmap array and return its index
int pmm_mmap_first_free() {
    for (uint32_t i = 0; i < g_pmm_info.max_blocks; i++) {
        if (g_pmm_info.memory_map_array[i] != 0xffffffff) {
            for (uint32_t j = 0; j < 32; j++) {
                if (!(g_pmm_info.memory_map_array[i] & (1 << j)))
                    return i * 32 + j;
            }
        }
    }
    return -1;
}

// find first free number of frames(size) and return its index
int pmm_mmap_first_free_by_size(uint32_t size) {
    if (size == 0)
        return -1;

    uint32_t free = 0;
    int start_index = -1;

    for (uint32_t i = 0; i < g_pmm_info.max_blocks; i++) {
        if (g_pmm_info.memory_map_array[i] != 0xffffffff) {
            for (uint32_t j = 0; j < 32; j++) {
                int bit = i * 32 + j;

                if (bit >= g_pmm_info.max_blocks * 32)
                    return -1;

                if (!pmm_mmap_test(bit)) {
                    if (free == 0)
                        start_index = bit;
                    free++;
                    if (free == size)
                        return start_index;
                } else {
                    free = 0;
                    start_index = -1;
                }
            }
        }
    }
    return -1;
}

/**
 * returns index of bitmap array if no of bits(size) are free
 */
int pmm_next_free_frame(int size) {
    return pmm_mmap_first_free_by_size(size);
}

// initialize memory bitmap array by making blocks from total memory size
void pmm_init(PMM_PHYSICAL_ADDRESS bitmap, uint32_t total_memory_size) {
    g_pmm_info.memory_size = total_memory_size;
    g_pmm_info.memory_map_array = (uint32_t *)bitmap;
    
    g_pmm_info.max_blocks = total_memory_size / PMM_BLOCK_SIZE;
    g_pmm_info.used_blocks = g_pmm_info.max_blocks;

    // Clear all memory to mark as used
    memset(g_pmm_info.memory_map_array, 0xff, (g_pmm_info.max_blocks / 8));

    // Calculate the ending address of map array
    g_pmm_info.memory_map_end = (uint32_t)&g_pmm_info.memory_map_array[g_pmm_info.max_blocks / 32];
}

// initialize/request for a free region of region_size from pmm
void pmm_init_region(PMM_PHYSICAL_ADDRESS base, uint32_t region_size) {
    if (region_size == 0)
        return;

    int align = base / PMM_BLOCK_SIZE;
    int blocks = region_size / PMM_BLOCK_SIZE;

    while (blocks > 0) {
        pmm_mmap_unset(align++);
        g_pmm_info.used_blocks--;
        blocks--;
    }
}

// de-initialize/free allocated region of region_size from pmm
void pmm_deinit_region(PMM_PHYSICAL_ADDRESS base, uint32_t region_size) {
    if (region_size == 0)
        return;

    int align = base / PMM_BLOCK_SIZE;
    int blocks = region_size / PMM_BLOCK_SIZE;

    while (blocks > 0) {
        pmm_mmap_set(align++);
        g_pmm_info.used_blocks++;
        blocks--;
    }
}

/**
 * request to allocate a single block of memory from pmm
 */
void* pmm_alloc_block() {
    // out of memory
    if ((g_pmm_info.max_blocks - g_pmm_info.used_blocks) <= 0)
        return NULL;

    int frame = pmm_mmap_first_free();
    if (frame == -1)
        return NULL;

    pmm_mmap_set(frame);

    // get actual address by skipping memory map
    PMM_PHYSICAL_ADDRESS addr = (frame * PMM_BLOCK_SIZE) + g_pmm_info.memory_map_end;
    g_pmm_info.used_blocks++;

    return (void *)addr;
}

/**
 * free given requested single block of memory from pmm
 */
void pmm_free_block(void* p) {
    PMM_PHYSICAL_ADDRESS addr = (PMM_PHYSICAL_ADDRESS)p;
    // go to the bitmap array address
    addr -= g_pmm_info.memory_map_end;
    int frame = addr / PMM_BLOCK_SIZE;
    pmm_mmap_unset(frame);
    g_pmm_info.used_blocks--;
}

/**
 * request to allocate no of blocks of memory from pmm
 */
void* pmm_alloc_blocks(uint32_t size) {
    uint32_t i;

    // out of memory
    if ((g_pmm_info.max_blocks - g_pmm_info.used_blocks) <= size)
        return NULL;

    int frame = pmm_mmap_first_free_by_size(size);
    if (frame == -1)
        return NULL;

    // set bits in memory map
    for (i = 0; i < size; i++)
        pmm_mmap_set(frame + i);

    PMM_PHYSICAL_ADDRESS addr = (frame * PMM_BLOCK_SIZE) + g_pmm_info.memory_map_end;
    g_pmm_info.used_blocks += size;

    return (void *)addr;
}

/**
 * free given requested no of blocks of memory from pmm
 */
void pmm_free_blocks(void* p, uint32_t size) {
    uint32_t i;

    PMM_PHYSICAL_ADDRESS addr = (PMM_PHYSICAL_ADDRESS)p;
    // go to the bitmap array address
    addr -= g_pmm_info.memory_map_end;
    int frame = addr / PMM_BLOCK_SIZE;
    for (i = 0; i < size; i++)
        pmm_mmap_unset(frame + i);
    g_pmm_info.used_blocks -= size;
}
