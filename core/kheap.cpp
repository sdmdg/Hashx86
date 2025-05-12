#include "core/kheap.h"
#include "debug.h"
#include "core/memory.h"

// start & end addresses pointing to memory
void *g_kheap_start_addr = NULL, *g_kheap_end_addr = NULL;
unsigned long g_total_size = 0;
unsigned long g_total_used_size = 0;
// list head
KHEAP_BLOCK *g_head = NULL;

// Global flag to indicate if kheap is initialized
static bool kheap_initialized = false;

/**
 * initialize heap and set total memory size
*/
int kheap_init(void *start_addr, void *end_addr) {
    if (start_addr > end_addr) {
        printf("failed to init kheap\n");
        return -1;
    }
    g_kheap_start_addr = start_addr;
    g_kheap_end_addr = end_addr;
    g_total_size = (unsigned long)end_addr - (unsigned long)start_addr;
    g_total_used_size = 0;
    kheap_initialized = true;
    return 0;
}

/**
 * increase the heap memory by size & get its address
*/
void *kbrk(int size) {
    void *addr = NULL;
    if (size <= 0)
        return NULL;
    // check memory is available or not
    if ((int)(g_total_size - g_total_used_size) <= size)
        return NULL;
    // add start addr with total previously used memory
    addr = (void*)((unsigned long)g_kheap_start_addr + g_total_used_size);
    g_total_used_size += size;
    return addr;
}

/**
 * print list of allocated blocks
*/
void kheap_print_blocks() {
    KHEAP_BLOCK *temp = g_head;
    printf("Block Size: %d\n", sizeof(KHEAP_BLOCK));
    while (temp != NULL) {
        printf("size:%d, free:%d, data: 0x%x, curr: 0x%x, next: 0x%x\n",
               temp->metadata.size, temp->metadata.is_free, temp->data, temp, temp->next);
        temp = temp->next;
    }
}

bool is_block_free(KHEAP_BLOCK *block) {
    if (!block)
        return false;
    return (block->metadata.is_free == true);
}

/**
 * this just check freed memory is greater than the required one
*/
KHEAP_BLOCK *worst_fit(int size) {
    KHEAP_BLOCK *temp = g_head;
    while (temp != NULL) {
        if (is_block_free(temp)) {
            if ((int)temp->metadata.size >= size)
                return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

// allocate a new heap block
KHEAP_BLOCK *allocate_new_block(int size) {
    KHEAP_BLOCK *temp = g_head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    KHEAP_BLOCK *new_block = (KHEAP_BLOCK *)kbrk(sizeof(KHEAP_BLOCK));
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
    if (size <= 0)
        return NULL;
    if (g_head == NULL) {
        g_head = (KHEAP_BLOCK *)kbrk(sizeof(KHEAP_BLOCK));
        if (!g_head) return NULL;
        
        g_head->metadata.is_free = false;
        g_head->metadata.size = size;
        g_head->next = NULL;
        g_head->data = kbrk(size);
        if (!g_head->data) return NULL;
        
        return g_head->data;
    } else {
        KHEAP_BLOCK *worst = worst_fit(size);
        if (worst == NULL) {
            KHEAP_BLOCK *new_block = allocate_new_block(size);
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

/**
 * allocate memory n * size & zeroing out
*/
void *kcalloc(int n, int size) {
    if (n < 0 || size < 0)
        return NULL;
    void *mem = kmalloc(n * size);
    if (mem)
        memset(mem, 0, n * size);
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

    KHEAP_BLOCK *temp = g_head;
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
    
    KHEAP_BLOCK *temp = g_head;
    while (temp != NULL) {
        if (temp->data == addr) {
            temp->metadata.is_free = true;
            return;
        }
        temp = temp->next;
    }
}

void* operator new(size_t size) {
    return kmalloc(size);
}

void* operator new[](size_t size) {
    return kmalloc(size);
}

void* operator new(size_t size, std::align_val_t) {
    return operator new(size); // Redirect to normal new
}

void* operator new[](size_t size, std::align_val_t) {
    return operator new(size); // Redirect to normal new
}

void operator delete(void* ptr) noexcept {
    kfree(ptr);
}

void operator delete[](void* ptr) noexcept {
    kfree(ptr);
}

// C++14-compliant sized delete operators
void operator delete(void* ptr, size_t size) noexcept {
    kfree(ptr);
}

void operator delete[](void* ptr, size_t size) noexcept {
    kfree(ptr);
}

void operator delete(void* ptr, std::align_val_t) noexcept {
    operator delete(ptr); // Redirect to normal delete
}
