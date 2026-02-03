#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <core/Iguard.h>
#include <core/globals.h>
#include <debug.h>
#include <stddef.h>
#include <types.h>

// Memory manipulation functions
extern "C" {
void* memset(void* ptr, int value, size_t num);
void* memcpy(void* dest, const void* src, size_t num);
int memcmp(const void* ptr1, const void* ptr2, size_t num);
}

// Internal optimization helpers
void init_memory_optimizations();
extern bool g_sse_active;

// a singly linked list heap block
typedef struct _kheap_block {
    struct {
        uint32_t size;    // memory size
        uint8_t is_free;  // block is free or not
    } metadata;
    struct _kheap_block* next;
    void* data;  // data pointer
} __attribute__((packed)) KHEAP_BLOCK;

/**
 * initialize heap and set total memory size
 */
int kheap_init(void* start_addr, void* end_addr);

/**
 * increase the heap memory by size & get its address
 */
void* kbrk(int size);

/**
 * print list of allocated blocks
 */
void kheap_print_blocks();

/**
 * allocate given size if list is null
 * otherwise try some memory allocation algorithm like best fit etc
 * to find best block to allocate
 */
void* kmalloc(int size);

void* aligned_kmalloc(size_t size, size_t alignment);

/**
 * allocate memory n * size & zeroing out
 */
void* kcalloc(int n, int size);

/**
 * allocate a new block of memory
 * copy previous block data & set free the previous block
 */
void* krealloc(void* ptr, int size);

/**
 * set free the block
 */
void kfree(void* addr);

bool is_block_free(KHEAP_BLOCK* block);

/**
 * this just check freed memory is greater than the required one
 */
KHEAP_BLOCK* worst_fit(int size);

// allocate a new heap block
KHEAP_BLOCK* allocate_new_block(int size);

// C++ New/Delete Overloads
void* operator new(size_t size);
void* operator new[](size_t size);
void* operator new(size_t size, std::align_val_t);
void* operator new[](size_t size, std::align_val_t);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;
void operator delete(void* ptr, size_t size) noexcept;
void operator delete[](void* ptr, size_t size) noexcept;
void operator delete(void* ptr, std::align_val_t) noexcept;
void operator delete[](void* ptr, std::align_val_t) noexcept;

#endif  // MEMORY_MANAGER_H
