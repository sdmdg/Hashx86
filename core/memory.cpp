/**
 * @file        memory.cpp
 * @brief       Memory Manager
 * 
 * @date        02/02/2025
 * @version     1.0.0-beta
 */

#include <core/memory.h>

void* memcpy(void* dst, const void* src, size_t num)
{
    uint32_t* u32Dst = (uint32_t*)dst;
    const uint32_t* u32Src = (const uint32_t*)src;

    size_t count = num / 4;
    for (size_t i = 0; i < count; i++)
        u32Dst[i] = u32Src[i];

    uint8_t* u8Dst = (uint8_t*)u32Dst + count * 4;
    const uint8_t* u8Src = (const uint8_t*)u32Src + count * 4;
    for (size_t i = 0; i < (num % 4); i++)
        u8Dst[i] = u8Src[i];

    return dst;
}

void * memset(void * ptr, int value, uint16_t num)
{
    uint8_t* u8Ptr = (uint8_t *)ptr;

    for (uint16_t i = 0; i < num; i++)
        u8Ptr[i] = (uint8_t)value;

    return ptr;
}

int memcmp(const void* ptr1, const void* ptr2, uint16_t num)
{
    const uint8_t* u8Ptr1 = (const uint8_t *)ptr1;
    const uint8_t* u8Ptr2 = (const uint8_t *)ptr2;

    for (uint16_t i = 0; i < num; i++)
        if (u8Ptr1[i] != u8Ptr2[i])
            return 1;

    return 0;
}