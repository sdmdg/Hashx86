/**
 * @file        memory.cpp
 * @brief       Memory Manager Implementation with SSE support
 *
 * @date        29/01/2026
 * @version     1.0.0-beta
 */

#define KDBG_COMPONENT "K.HEAP"
#include <core/memory.h>

// --- SSE HELPERS ---
static inline void __cpuid(int code, uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d) {
    asm volatile("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(code));
}

bool CheckSSE() {
    uint32_t eax, ebx, ecx, edx;
    __cpuid(1, &eax, &ebx, &ecx, &edx);
    // Bit 25 of EDX is SSE, Bit 26 is SSE2 (we usually want SSE2 for 128-bit moves)
    return (edx & (1 << 25)) || (edx & (1 << 26));
}

void EnableSSE_ASM() {
    // Set CR0 and CR4 bits to enable SSE context
    asm volatile(
        "mov %cr0, %eax\n\t"
        "and $0xFFFB, %ax\n\t"  // Clear EM (Bit 2)
        "or  $0x2, %ax\n\t"     // Set MP (Bit 1)
        "mov %eax, %cr0\n\t"

        "mov %cr4, %eax\n\t"
        "or  $0x600, %ax\n\t"  // Set OSFXSR (Bit 9) and OSXMMEXCPT (Bit 10)
        "mov %eax, %cr4\n\t");
}

void init_memory_optimizations() {
    if (CheckSSE()) {
        EnableSSE_ASM();
        g_sse_active = true;
        KDBG1("SSE Enabled");
    } else {
        g_sse_active = false;
    }
}

// --- MEMORY COPY IMPLEMENTATIONS ---

// The Standard C++ Copy (Fallback)
static void* memcpy_standard(void* destination, const void* source, size_t size) {
    uint32_t* dst32 = static_cast<uint32_t*>(destination);
    const uint32_t* src32 = static_cast<const uint32_t*>(source);

    size_t word_count = size / 4;
    for (size_t i = 0; i < word_count; i++) dst32[i] = src32[i];

    uint8_t* dst8 = reinterpret_cast<uint8_t*>(dst32) + word_count * 4;
    const uint8_t* src8 = reinterpret_cast<const uint8_t*>(src32) + word_count * 4;
    for (size_t i = 0; i < (size % 4); i++) dst8[i] = src8[i];

    return destination;
}

// The SSE Optimized Copy (Assembly)
__attribute__((target("sse"))) static void* memcpy_sse(void* dest, const void* src, size_t count) {
    size_t num_blocks = count / 16;
    size_t remaining = count % 16;

    char* d = (char*)dest;
    const char* s = (const char*)src;

    // Use XMM0 to move 16 bytes at a time
    for (size_t i = 0; i < num_blocks; i++) {
        asm volatile(
            "movups (%0), %%xmm0\n\t"  // Load unaligned 128-bit
            "movups %%xmm0, (%1)\n\t"  // Store unaligned 128-bit
            :
            : "r"(s), "r"(d)
            : "memory", "%xmm0");
        s += 16;
        d += 16;
    }

    // Copy remaining bytes
    while (remaining--) {
        *d++ = *s++;
    }

    return dest;
}

// The Main Wrapper
void* memcpy(void* destination, const void* source, size_t size) {
    if (g_sse_active) {
        return memcpy_sse(destination, source, size);
    } else {
        return memcpy_standard(destination, source, size);
    }
}

void* memset(void* ptr, int value, size_t size) {
    uint8_t* byte_ptr = static_cast<uint8_t*>(ptr);
    for (size_t i = 0; i < size; i++) byte_ptr[i] = static_cast<uint8_t>(value);

    return ptr;
}

int memcmp(const void* ptr1, const void* ptr2, size_t size) {
    const uint8_t* byte_ptr1 = static_cast<const uint8_t*>(ptr1);
    const uint8_t* byte_ptr2 = static_cast<const uint8_t*>(ptr2);

    for (size_t i = 0; i < size; i++)
        if (byte_ptr1[i] != byte_ptr2[i]) return byte_ptr1[i] - byte_ptr2[i];

    return 0;
}

// start & end addresses pointing to memory
void *g_kheap_start_addr = NULL, *g_kheap_end_addr = NULL;
unsigned long g_total_size = 0;
unsigned long g_total_used_size = 0;
// list head
KHEAP_BLOCK* g_head = NULL;

// Global flag to indicate if kheap is initialized
static bool kheap_initialized = false;

/**
 * initialize heap and set total memory size
 */
int kheap_init(void* start_addr, void* end_addr) {
    if (start_addr > end_addr) {
        KDBG1("Init failed start=0x%x end=0x%x", start_addr, end_addr);
        return -1;
    }

    // ENABLE OPTIMIZATIONS
    init_memory_optimizations();

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
void* kbrk(int size) {
    void* addr = NULL;
    if (size <= 0) return NULL;
    // check memory is available or not
    if ((int)(g_total_size - g_total_used_size) <= size) {
        KDBG1("HeapExhausted req=%d available=%d", size, (g_total_size - g_total_used_size));
        return NULL;
    }
    // add start addr with total previously used memory
    addr = (void*)((unsigned long)g_kheap_start_addr + g_total_used_size);
    g_total_used_size += size;
    // KDBG3("HeapExpand size=%d addr=0x%x", size, addr);
    return addr;
}

/**
 * print list of allocated blocks
 */
void kheap_print_blocks() {
    KHEAP_BLOCK* temp = g_head;
    KDBG3("PrintBlocks size=%d", sizeof(KHEAP_BLOCK));
    while (temp != NULL) {
        KDBG3("Block size=%d free=%d data=0x%x curr=0x%x next=0x%x", temp->metadata.size,
              temp->metadata.is_free, temp->data, temp, temp->next);
        temp = temp->next;
    }
}

bool is_block_free(KHEAP_BLOCK* block) {
    if (!block) return false;
    return (block->metadata.is_free == true);
}

/**
 * this just check freed memory is greater than the required one
 */
KHEAP_BLOCK* worst_fit(int size) {
    KHEAP_BLOCK* temp = g_head;
    while (temp != NULL) {
        if (is_block_free(temp)) {
            if ((int)temp->metadata.size >= size) return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

// allocate a new heap block
KHEAP_BLOCK* allocate_new_block(int size) {
    KHEAP_BLOCK* temp = g_head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    KHEAP_BLOCK* new_block = (KHEAP_BLOCK*)kbrk(sizeof(KHEAP_BLOCK));
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
void* kmalloc(int size) {
    InterruptGuard guard;
    if (size <= 0) {
        KDBG2("AllocInvalid invalid_size=%d", size);
        return NULL;
    }
    if (g_head == NULL) {
        g_head = (KHEAP_BLOCK*)kbrk(sizeof(KHEAP_BLOCK));
        if (!g_head) {
            KDBG1("AllocFail reason=InitHeapMetadataOM");
            return NULL;
        }

        g_head->metadata.is_free = false;
        g_head->metadata.size = size;
        g_head->next = NULL;
        g_head->data = kbrk(size);
        if (!g_head->data) return NULL;

        return g_head->data;
    } else {
        KHEAP_BLOCK* worst = worst_fit(size);
        if (worst == NULL) {
            KHEAP_BLOCK* new_block = allocate_new_block(size);
            if (!new_block) {
                KDBG1("AllocFail size=%d reason=NoBlock/OOM", size);
                return NULL;
            }

            new_block->metadata.is_free = false;
            new_block->metadata.size = size;
            new_block->data = kbrk(size);
            if (!new_block->data) return NULL;

            return new_block->data;
        } else {
            worst->metadata.is_free = false;
            // if (size != 8) KDBG3("Alloc size=%d ptr=0x%x", size, worst->data);
            return worst->data;
        }
    }
    return NULL;
}

void* aligned_kmalloc(size_t size, size_t alignment) {
    InterruptGuard guard;
    uintptr_t raw_addr = (uintptr_t)kmalloc(size + alignment);
    if (!raw_addr) return nullptr;

    uintptr_t aligned_addr = (raw_addr + alignment - 1) & ~(alignment - 1);
    KDBG3("AlignedAlloc size=%d align=%d raw=0x%x addr=0x%x", size, alignment, raw_addr,
          aligned_addr);
    return (void*)aligned_addr;
}

/**
 * allocate memory n * size & zeroing out
 */
void* kcalloc(int n, int size) {
    InterruptGuard guard;
    if (n < 0 || size < 0) return NULL;
    void* mem = kmalloc(n * size);
    if (mem) memset(mem, 0, n * size);
    KDBG3("Calloc n=%d size=%d addr=0x%x", n, size, mem);
    return mem;
}

/**
 * allocate a new block of memory
 * copy previous block data & set free the previous block
 */
void* krealloc(void* ptr, int size) {
    InterruptGuard guard;
    if (!ptr) return kmalloc(size);
    if (size <= 0) {
        kfree(ptr);
        return NULL;
    }

    KHEAP_BLOCK* temp = g_head;
    while (temp != NULL) {
        if (temp->data == ptr) {
            void* new_ptr = kmalloc(size);
            if (!new_ptr) return NULL;

            // Uses the optimized memcpy automatically
            memcpy(new_ptr, ptr, temp->metadata.size < size ? temp->metadata.size : size);
            temp->metadata.is_free = true;
            // if (size != 8) KDBG3("Realloc ptr=0x%x new_ptr=0x%x size=%d", ptr, new_ptr, size);
            return new_ptr;
        }
        temp = temp->next;
    }
    return NULL;
}

/**
 * set free the block
 */
void kfree(void* addr) {
    InterruptGuard guard;
    if (!addr) {
        KDBG2("FreeInvalid ptr=NULL");
        return;
    }

    KHEAP_BLOCK* temp = g_head;
    while (temp != NULL) {
        if (temp->data == addr) {
            temp->metadata.is_free = true;
            // if (temp->metadata.size != 8) KDBG3("Free ptr=0x%x", addr);
            return;
        }
        temp = temp->next;
    }
    KDBG1("FreeError ptr=0x%x reason=NotFound", addr);
}

// --- C++ OPERATORS ---

void* operator new(size_t size) {
    return kmalloc(size);
}

void* operator new[](size_t size) {
    return kmalloc(size);
}

void* operator new(size_t size, std::align_val_t alignment) {
    return aligned_kmalloc(size, static_cast<size_t>(alignment));
}

void* operator new[](size_t size, std::align_val_t alignment) {
    return aligned_kmalloc(size, static_cast<size_t>(alignment));
}

void operator delete(void* ptr) noexcept {
    kfree(ptr);
}

void operator delete[](void* ptr) noexcept {
    kfree(ptr);
}

void operator delete(void* ptr, size_t size) noexcept {
    kfree(ptr);
}

void operator delete[](void* ptr, size_t size) noexcept {
    kfree(ptr);
}

void operator delete(void* ptr, std::align_val_t) noexcept {
    kfree(ptr);
}

void operator delete[](void* ptr, std::align_val_t) noexcept {
    kfree(ptr);
}
