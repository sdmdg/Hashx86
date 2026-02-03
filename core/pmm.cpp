/**
 * @file        pmm.cpp
 * @brief       Physical Memory Manager for #x86
 *
 * @date        29/01/2026
 * @version     1.0.0-beta
 */

#include <core/pmm.h>

PMM_INFO g_pmm_info;

// Set bit in memory map array with bounds check
static inline void pmm_mmap_set(int bit) {
    if (bit < g_pmm_info.max_blocks * 32)
        g_pmm_info.memory_map_array[bit / 32] |= (1 << (bit % 32));
}

// Unset bit in memory map array with bounds check
static inline void pmm_mmap_unset(int bit) {
    if (bit < g_pmm_info.max_blocks * 32)
        g_pmm_info.memory_map_array[bit / 32] &= ~(1 << (bit % 32));
}

// Test if given nth bit is set with bounds check
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

// Find first free frame in bitmap array and return its index
int pmm_mmap_first_free() {
    for (uint32_t i = 0; i < g_pmm_info.max_blocks; i++) {
        if (g_pmm_info.memory_map_array[i] != 0xffffffff) {
            for (uint32_t j = 0; j < 32; j++) {
                if (!(g_pmm_info.memory_map_array[i] & (1 << j))) return i * 32 + j;
            }
        }
    }
    return -1;
}

// Find first free number of frames(size) and return its index
int pmm_mmap_first_free_by_size(uint32_t size) {
    if (size == 0) return -1;

    uint32_t free = 0;
    int start_index = -1;

    for (uint32_t i = 0; i < g_pmm_info.max_blocks; i++) {
        if (g_pmm_info.memory_map_array[i] != 0xffffffff) {
            for (uint32_t j = 0; j < 32; j++) {
                int bit = i * 32 + j;

                if (bit >= g_pmm_info.max_blocks * 32) return -1;

                if (!pmm_mmap_test(bit)) {
                    if (free == 0) start_index = bit;
                    free++;
                    if (free == size) return start_index;
                } else {
                    free = 0;
                    start_index = -1;
                }
            }
        }
    }
    return -1;
}

// Initialize memory bitmap
int pmm_next_free_frame(int size) {
    return pmm_mmap_first_free_by_size(size);
}

// Initialize memory bitmap
void pmm_init(PMM_PHYSICAL_ADDRESS bitmap, uint32_t total_memory_size) {
    g_pmm_info.memory_size = total_memory_size;
    g_pmm_info.memory_map_array = (uint32_t*)bitmap;

    g_pmm_info.max_blocks = total_memory_size / PMM_BLOCK_SIZE;
    g_pmm_info.used_blocks = g_pmm_info.max_blocks;

    // Mark ALL memory as Used (0xFF)
    memset(g_pmm_info.memory_map_array, 0xff, (g_pmm_info.max_blocks / 8));

    // Calculate End of Bitmap
    uint32_t map_size = g_pmm_info.max_blocks / 8;
    g_pmm_info.memory_map_end = (uint32_t)g_pmm_info.memory_map_array + map_size;

    // FORCE ALIGNMENT
    // Align the end marker to the next 4096 byte boundary.
    if (g_pmm_info.memory_map_end % PMM_BLOCK_SIZE != 0) {
        g_pmm_info.memory_map_end += PMM_BLOCK_SIZE - (g_pmm_info.memory_map_end % PMM_BLOCK_SIZE);
    }
}

void pmm_init_region(PMM_PHYSICAL_ADDRESS base, uint32_t region_size) {
    if (region_size == 0) return;

    int align = base / PMM_BLOCK_SIZE;
    int blocks = region_size / PMM_BLOCK_SIZE;

    while (blocks > 0) {
        pmm_mmap_unset(align++);
        g_pmm_info.used_blocks--;
        blocks--;
    }
}

void pmm_deinit_region(PMM_PHYSICAL_ADDRESS base, uint32_t region_size) {
    if (region_size == 0) return;

    int align = base / PMM_BLOCK_SIZE;
    int blocks = region_size / PMM_BLOCK_SIZE;

    while (blocks > 0) {
        pmm_mmap_set(align++);
        g_pmm_info.used_blocks++;
        blocks--;
    }
}

void* pmm_alloc_block() {
    if ((g_pmm_info.max_blocks - g_pmm_info.used_blocks) <= 0) return NULL;

    int frame = pmm_mmap_first_free();
    if (frame == -1) return NULL;

    pmm_mmap_set(frame);

    // Use Absolute Addressing
    PMM_PHYSICAL_ADDRESS addr = (frame * PMM_BLOCK_SIZE);

    g_pmm_info.used_blocks++;

    return (void*)addr;
}

void pmm_free_block(void* p) {
    PMM_PHYSICAL_ADDRESS addr = (PMM_PHYSICAL_ADDRESS)p;

    // Use Absolute Addressing
    int frame = addr / PMM_BLOCK_SIZE;

    pmm_mmap_unset(frame);
    g_pmm_info.used_blocks--;
}

void* pmm_alloc_blocks(uint32_t size) {
    uint32_t i;

    if ((g_pmm_info.max_blocks - g_pmm_info.used_blocks) <= size) return NULL;

    int frame = pmm_mmap_first_free_by_size(size);
    if (frame == -1) return NULL;

    for (i = 0; i < size; i++) pmm_mmap_set(frame + i);

    // Use Absolute Addressing
    PMM_PHYSICAL_ADDRESS addr = (frame * PMM_BLOCK_SIZE);

    g_pmm_info.used_blocks += size;

    return (void*)addr;
}

void pmm_free_blocks(void* p, uint32_t size) {
    uint32_t i;

    PMM_PHYSICAL_ADDRESS addr = (PMM_PHYSICAL_ADDRESS)p;

    // Use Absolute Addressing
    int frame = addr / PMM_BLOCK_SIZE;

    for (i = 0; i < size; i++) pmm_mmap_unset(frame + i);

    g_pmm_info.used_blocks -= size;
}
