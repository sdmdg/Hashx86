#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stddef.h>
#include <Hx86/types.h>
#include <Hx86/debug.h>

// Memory manipulation functions
void* memcpy(void* destination, const void* source, size_t size);
void* memset(void* ptr, int value, size_t size);
int memcmp(const void* ptr1, const void* ptr2, size_t size);

// a singly linked list heap block
typedef struct _heap_block {
    struct {
        uint32_t size;  // memory size
        uint8_t is_free; // block is free or not
    } metadata;
    struct _heap_block *next; 
    void *data;  // data pointer
} __attribute__((packed)) heap_BLOCK;

/**
 * initialize heap and set total memory size
*/
int heap_init(void *start_addr, void *end_addr);

/**
 * increase the heap memory by size & get its address
*/
void *kbrk(int size);

/**
 * print list of allocated blocks
*/
void heap_print_blocks();

/**
 * allocate given size if list is null
 * otherwise try some memory allocation algorithm like best fit etc
 * to find best block to allocate
 * # Need to work on internal/external segmentaion problem
*/
void *kmalloc(int size);

/**
 * allocate memory n * size & zeroing out
*/
void *kcalloc(int n, int size);

/**
 * allocate a new block of memory
 * copy previous block data & set free the previous block
*/
void *krealloc(void *ptr, int size);

/**
 * set free the block
*/
void kfree(void *addr);


bool is_block_free(heap_BLOCK *block);

/**
 * this just check freed memory is greater than the required one
*/
heap_BLOCK *worst_fit(int size);

// allocate a new heap block
heap_BLOCK *allocate_new_block(int size);



void* operator new(size_t size);

void* operator new[](size_t size);

void* operator new(size_t size, std::align_val_t);

void* operator new[](size_t size, std::align_val_t);

void operator delete(void* ptr) noexcept;

void operator delete[](void* ptr) noexcept;

// C++14-compliant sized delete operators
void operator delete(void* ptr, size_t size) noexcept;

void operator delete[](void* ptr, size_t size) noexcept;

void operator delete(void* ptr, std::align_val_t) noexcept;

void operator delete[](void* ptr, std::align_val_t) noexcept;

#endif // MEMORY_MANAGER_H
