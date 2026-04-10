//
// Created by Gabriel Souza on 09/04/2026.
//

#ifndef DATA_STRUCTURES_UTILS_H
#define DATA_STRUCTURES_UTILS_H

#include <stddef.h>

/* Alias to a single-byte type for pointer arithmetic and byte-level operations */
typedef unsigned char byte;

static inline size_t
max(const size_t x, const size_t y)
{
    return x > y ? x : y;
}

static inline size_t
min(const size_t x, const size_t y)
{
    return x < y ? x : y;
}

static inline size_t
align_value(const size_t value, const size_t align)
{
    return (value + align -1) & ~(align -1);
}

#endif //DATA_STRUCTURES_UTILS_H
