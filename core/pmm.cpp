/**
 * @file        pmm.cpp
 * @brief       Physical Memory Manager for #x86
 *
 * @date        29/01/2026
 * @version     1.0.0-beta
 */

#define KDBG_COMPONENT "PMM"
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
    KDBG3("get_max_blocks=%u", g_pmm_info.max_blocks);
    return g_pmm_info.max_blocks;
}

uint32_t pmm_get_used_blocks() {
    KDBG3("get_used_blocks=%u", g_pmm_info.used_blocks);
    return g_pmm_info.used_blocks;
}

// Find first free frame in bitmap array and return its index
int pmm_mmap_first_free() {
    for (uint32_t i = 0; i < g_pmm_info.max_blocks; i++) {
        if (g_pmm_info.memory_map_array[i] != 0xffffffff) {
            for (uint32_t j = 0; j < 32; j++) {
                if (!(g_pmm_info.memory_map_array[i] & (1 << j))) {
                    int bit = i * 32 + j;
                    KDBG3("first_free bit=%d", bit);
                    return bit;
                }
            }
        }
    }
    KDBG2("single-frame search result=none");
    return -1;
}

// Find first free frame below a certain limit (for Low Mem Alloc)
int pmm_mmap_first_free_low(uint32_t limit_frame) {
    for (uint32_t i = 0; i < g_pmm_info.max_blocks; i++) {
        // If we exceed limit, stop early (i*32 is the first bit in this chunk)
        if (i * 32 >= limit_frame) {
            KDBG2("low-memory search result=none limit_frame=%u", limit_frame);
            return -1;
        }

        if (g_pmm_info.memory_map_array[i] != 0xffffffff) {
            for (uint32_t j = 0; j < 32; j++) {
                int bit = i * 32 + j;
                if (bit >= (int)limit_frame) {
                    KDBG2("low-memory search result=none limit_frame=%u", limit_frame);
                    return -1;
                }

                if (!(g_pmm_info.memory_map_array[i] & (1 << j))) {
                    KDBG3("first_free_low limit_frame=%u bit=%d", limit_frame, bit);
                    return bit;
                }
            }
        }
    }
    KDBG2("low-memory search result=none limit_frame=%u", limit_frame);
    return -1;
}

// Find first free number of frames(size) and return its index
int pmm_mmap_first_free_by_size(uint32_t size) {
    if (size == 0) {
        KDBG2("contiguous search invalid request size=0");
        return -1;
    }

    uint32_t free = 0;
    int start_index = -1;

    for (uint32_t i = 0; i < g_pmm_info.max_blocks; i++) {
        if (g_pmm_info.memory_map_array[i] != 0xffffffff) {
            for (uint32_t j = 0; j < 32; j++) {
                int bit = i * 32 + j;

                if (bit >= g_pmm_info.max_blocks * 32) {
                    KDBG2("contiguous search aborted reason=out_of_range size=%u", size);
                    return -1;
                }

                if (!pmm_mmap_test(bit)) {
                    if (free == 0) start_index = bit;
                    free++;
                    if (free == size) {
                        KDBG3("first_free_by_size size=%u start=%d", size, start_index);
                        return start_index;
                    }
                } else {
                    free = 0;
                    start_index = -1;
                }
            }
        }
    }
    KDBG2("contiguous search result=none size=%u", size);
    return -1;
}

// Initialize memory bitmap
int pmm_next_free_frame(int size) {
    int next = pmm_mmap_first_free_by_size(size);
    KDBG3("next_free_frame size=%d next=%d", size, next);
    return next;
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

    KDBG1("startup bitmap=0x%x total=%uKB blocks=%u", bitmap, total_memory_size / 1024,
          g_pmm_info.max_blocks);
    KDBG2("map_array=0x%x map_end=0x%x used=%u", (uint32_t)g_pmm_info.memory_map_array,
          g_pmm_info.memory_map_end, g_pmm_info.used_blocks);
}

void pmm_init_region(PMM_PHYSICAL_ADDRESS base, uint32_t region_size) {
    if (region_size == 0) {
        KDBG3("init_region skipped size=0");
        return;
    }

    int align = base / PMM_BLOCK_SIZE;
    int blocks = region_size / PMM_BLOCK_SIZE;

    while (blocks > 0) {
        pmm_mmap_unset(align++);
        g_pmm_info.used_blocks--;
        blocks--;
    }

    KDBG2("region free-mark base=0x%x size=%uKB used=%u", base, region_size / 1024,
          g_pmm_info.used_blocks);
}

void pmm_deinit_region(PMM_PHYSICAL_ADDRESS base, uint32_t region_size) {
    if (region_size == 0) {
        KDBG3("deinit_region skipped size=0");
        return;
    }

    int align = base / PMM_BLOCK_SIZE;
    int blocks = region_size / PMM_BLOCK_SIZE;

    while (blocks > 0) {
        pmm_mmap_set(align++);
        g_pmm_info.used_blocks++;
        blocks--;
    }

    KDBG2("region reserve-mark base=0x%x size=%uKB used=%u", base, region_size / 1024,
          g_pmm_info.used_blocks);
}

void* pmm_alloc_block() {
    if ((g_pmm_info.max_blocks - g_pmm_info.used_blocks) <= 0) {
        KDBG2("single-frame allocation failed reason=no_free_blocks");
        return NULL;
    }

    int frame = pmm_mmap_first_free();
    if (frame == -1) {
        KDBG2("single-frame allocation failed reason=frame_not_found");
        return NULL;
    }

    pmm_mmap_set(frame);

    // Use Absolute Addressing
    PMM_PHYSICAL_ADDRESS addr = (frame * PMM_BLOCK_SIZE);

    g_pmm_info.used_blocks++;

    KDBG3("alloc_block frame=%d addr=0x%x used=%u", frame, addr, g_pmm_info.used_blocks);

    return (void*)addr;
}

void* pmm_alloc_block_low(uint32_t limit_addr) {
    if ((g_pmm_info.max_blocks - g_pmm_info.used_blocks) <= 0) {
        KDBG2("low-memory allocation failed reason=no_free_blocks limit=0x%x", limit_addr);
        return NULL;
    }

    int limit_frame = limit_addr / PMM_BLOCK_SIZE;
    int frame = pmm_mmap_first_free_low(limit_frame);

    if (frame == -1) {
        KDBG2("low-memory allocation failed reason=frame_not_found limit=0x%x", limit_addr);
        return NULL;
    }

    pmm_mmap_set(frame);

    // Use Absolute Addressing
    PMM_PHYSICAL_ADDRESS addr = (frame * PMM_BLOCK_SIZE);
    g_pmm_info.used_blocks++;

    KDBG3("alloc_block_low limit=0x%x frame=%d addr=0x%x used=%u", limit_addr, frame, addr,
          g_pmm_info.used_blocks);

    return (void*)addr;
}

void pmm_free_block(void* p) {
    PMM_PHYSICAL_ADDRESS addr = (PMM_PHYSICAL_ADDRESS)p;

    // Use Absolute Addressing
    int frame = addr / PMM_BLOCK_SIZE;

    pmm_mmap_unset(frame);
    g_pmm_info.used_blocks--;

    KDBG3("free_block frame=%d addr=0x%x used=%u", frame, addr, g_pmm_info.used_blocks);
}

void* pmm_alloc_blocks(uint32_t size) {
    uint32_t i;

    if ((g_pmm_info.max_blocks - g_pmm_info.used_blocks) <= size) {
        KDBG2("contiguous allocation failed reason=no_free_blocks size=%u", size);
        return NULL;
    }

    int frame = pmm_mmap_first_free_by_size(size);
    if (frame == -1) {
        KDBG2("contiguous allocation failed reason=frame_not_found size=%u", size);
        return NULL;
    }

    for (i = 0; i < size; i++) pmm_mmap_set(frame + i);

    // Use Absolute Addressing
    PMM_PHYSICAL_ADDRESS addr = (frame * PMM_BLOCK_SIZE);

    g_pmm_info.used_blocks += size;

    KDBG2("contiguous allocation size=%u addr=0x%x used=%u", size, addr, g_pmm_info.used_blocks);
    KDBG3("alloc_blocks bytes=%u", size * PMM_BLOCK_SIZE);

    return (void*)addr;
}

void pmm_free_blocks(void* p, uint32_t size) {
    uint32_t i;

    PMM_PHYSICAL_ADDRESS addr = (PMM_PHYSICAL_ADDRESS)p;

    // Use Absolute Addressing
    int frame = addr / PMM_BLOCK_SIZE;

    for (i = 0; i < size; i++) pmm_mmap_unset(frame + i);

    g_pmm_info.used_blocks -= size;

    KDBG2("contiguous release size=%u addr=0x%x used=%u", size, addr, g_pmm_info.used_blocks);
}
