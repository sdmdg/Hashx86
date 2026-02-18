/**
 * @file        memory.cpp
 * @brief       Memory Manager Implementation
 *
 * @date        28/03/2025
 * @version     1.0.0-beta
 */

#include <Hx86/Hsyscalls/syscalls.h>
#include <Hx86/memory.h>

void *memcpy(void *destination, const void *source, size_t size) {
    uint32_t *dst32 = static_cast<uint32_t *>(destination);
    const uint32_t *src32 = static_cast<const uint32_t *>(source);

    size_t word_count = size / 4;
    for (size_t i = 0; i < word_count; i++) dst32[i] = src32[i];

    uint8_t *dst8 = reinterpret_cast<uint8_t *>(dst32) + word_count * 4;
    const uint8_t *src8 = reinterpret_cast<const uint8_t *>(src32) + word_count * 4;
    for (size_t i = 0; i < (size % 4); i++) dst8[i] = src8[i];

    return destination;
}

void *memset(void *ptr, int value, size_t size) {
    uint8_t *byte_ptr = static_cast<uint8_t *>(ptr);
    for (size_t i = 0; i < size; i++) byte_ptr[i] = static_cast<uint8_t>(value);

    return ptr;
}

int memcmp(const void *ptr1, const void *ptr2, size_t size) {
    const uint8_t *byte_ptr1 = static_cast<const uint8_t *>(ptr1);
    const uint8_t *byte_ptr2 = static_cast<const uint8_t *>(ptr2);

    for (size_t i = 0; i < size; i++)
        if (byte_ptr1[i] != byte_ptr2[i]) return byte_ptr1[i] - byte_ptr2[i];

    return 0;
}

// start & end addresses pointing to memory
void *g_heap_start_addr = NULL, *g_heap_end_addr = NULL;
unsigned long g_total_size = 0;
unsigned long g_total_used_size = 0;
// list head
heap_BLOCK *g_head = NULL;

// Global flag to indicate if heap is initialized
static bool heap_initialized = false;

/**
 * initialize heap and set total memory size
 */
int heap_init(void *start_addr, void *end_addr) {
    if (start_addr > end_addr) {
        printf("failed to init heap\n");
        return -1;
    }
    g_heap_start_addr = start_addr;
    g_heap_end_addr = end_addr;
    g_total_size = (unsigned long)end_addr - (unsigned long)start_addr;
    g_total_used_size = 0;
    heap_initialized = true;
    return 0;
}

/**
 * increase the heap memory by size & get its address
 */
void *kbrk(int size) {
    void *addr = NULL;
    if (size <= 0) return NULL;

    // check memory is available or not
    if ((long)(g_total_size - g_total_used_size) <= size) {
        // Request more memory from kernel (min 1MB growth)
        int needed = size - (g_total_size - g_total_used_size);
        int request_size = (needed + 4095) & ~4095;                  // Page align
        if (request_size < 1024 * 1024) request_size = 1024 * 1024;  // Min 1MB

        int32_t res = syscall_sbrk(request_size);
        if (res == -1) return NULL;

        g_total_size += request_size;
        g_heap_end_addr = (void *)((unsigned long)g_heap_end_addr + request_size);
    }

    // add start addr with total previously used memory
    addr = (void *)((unsigned long)g_heap_start_addr + g_total_used_size);
    g_total_used_size += size;
    return addr;
}

/**
 * print list of allocated blocks
 */
void heap_print_blocks() {
    heap_BLOCK *temp = g_head;
    printf("Block Size: %d\n", sizeof(heap_BLOCK));
    while (temp != NULL) {
        printf("size:%d, free:%d, data: 0x%x, curr: 0x%x, next: 0x%x\n", temp->metadata.size,
               temp->metadata.is_free, temp->data, temp, temp->next);
        temp = temp->next;
    }
}

bool is_block_free(heap_BLOCK *block) {
    if (!block) return false;
    return (block->metadata.is_free == true);
}

/**
 * this just check freed memory is greater than the required one
 */
heap_BLOCK *worst_fit(int size) {
    heap_BLOCK *temp = g_head;
    while (temp != NULL) {
        if (is_block_free(temp)) {
            if ((int)temp->metadata.size >= size) return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

// allocate a new heap block
heap_BLOCK *allocate_new_block(int size) {
    heap_BLOCK *temp = g_head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    heap_BLOCK *new_block = (heap_BLOCK *)kbrk(sizeof(heap_BLOCK));
    if (!new_block) return NULL;

    new_block->metadata.is_free = false;
    new_block->metadata.size = size;
    new_block->data = kbrk(size);
    if (!new_block->data) return NULL;

    new_block->next = NULL;
    temp->next = new_block;
    return new_block;
}

/**
 * allocate given size if list is null
 * otherwise try some memory allocation algorithm like best fit etc
 * to find best block to allocate
 */
void *kmalloc(int size) {
    if (size <= 0) return NULL;
    if (g_head == NULL) {
        g_head = (heap_BLOCK *)kbrk(sizeof(heap_BLOCK));
        if (!g_head) return NULL;

        g_head->metadata.is_free = false;
        g_head->metadata.size = size;
        g_head->next = NULL;
        g_head->data = kbrk(size);
        if (!g_head->data) return NULL;

        return g_head->data;
    } else {
        heap_BLOCK *worst = worst_fit(size);
        if (worst == NULL) {
            heap_BLOCK *new_block = allocate_new_block(size);
            if (!new_block) return NULL;

            new_block->metadata.is_free = false;
            new_block->metadata.size = size;
            new_block->data = kbrk(size);
            if (!new_block->data) return NULL;

            return new_block->data;
        } else {
            worst->metadata.is_free = false;
            return worst->data;
        }
    }
    return NULL;
}

void *aligned_kmalloc(size_t size, size_t alignment) {
    uintptr_t raw_addr = (uintptr_t)kmalloc(size + alignment);
    if (!raw_addr) return nullptr;

    uintptr_t aligned_addr = (raw_addr + alignment - 1) & ~(alignment - 1);
    return (void *)aligned_addr;
}

/**
 * allocate memory n * size & zeroing out
 */
void *kcalloc(int n, int size) {
    if (n < 0 || size < 0) return NULL;
    void *mem = kmalloc(n * size);
    if (mem) memset(mem, 0, n * size);
    return mem;
}

/**
 * allocate a new block of memory
 * copy previous block data & set free the previous block
 */
void *krealloc(void *ptr, int size) {
    if (!ptr) return kmalloc(size);
    if (size <= 0) {
        kfree(ptr);
        return NULL;
    }

    heap_BLOCK *temp = g_head;
    while (temp != NULL) {
        if (temp->data == ptr) {
            void *new_ptr = kmalloc(size);
            if (!new_ptr) return NULL;

            memcpy(new_ptr, ptr, temp->metadata.size < size ? temp->metadata.size : size);
            temp->metadata.is_free = true;
            return new_ptr;
        }
        temp = temp->next;
    }
    return NULL;
}

/**
 * set free the block
 */
void kfree(void *addr) {
    if (!addr) return;

    heap_BLOCK *temp = g_head;
    while (temp != NULL) {
        if (temp->data == addr) {
            temp->metadata.is_free = true;
            return;
        }
        temp = temp->next;
    }
}

void *operator new(size_t size) {
    return kmalloc(size);
}

void *operator new[](size_t size) {
    return kmalloc(size);
}

// Aligned `new`
void *operator new(size_t size, std::align_val_t alignment) {
    return aligned_kmalloc(size, static_cast<size_t>(alignment));
}

void *operator new[](size_t size, std::align_val_t alignment) {
    return aligned_kmalloc(size, static_cast<size_t>(alignment));
}

void operator delete(void *ptr) noexcept {
    kfree(ptr);
}

void operator delete[](void *ptr) noexcept {
    kfree(ptr);
}

// C++14-compliant sized delete operators
void operator delete(void *ptr, size_t size) noexcept {
    kfree(ptr);
}

void operator delete[](void *ptr, size_t size) noexcept {
    kfree(ptr);
}

// Aligned `delete`
void operator delete(void *ptr, std::align_val_t) noexcept {
    kfree(ptr);
}

void operator delete[](void *ptr, std::align_val_t) noexcept {
    kfree(ptr);
}
