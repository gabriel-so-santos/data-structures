/**
 * @file    utils.h
 * @brief   Internal utilities, types, macros, and functions
 *
 * @author  Gabriel Souza
 * @date    2026-04-09
 */

#ifndef LIBDS_INTERNAL_UTILS_H
#define LIBDS_INTERNAL_UTILS_H

#include <stddef.h>


/**
 * @brief   Byte type for pointer arithmetic and byte-level operations
 */
typedef unsigned char byte;


/**
 * @brief   Returns the maximum of two values.
 * @param   x First value.
 * @param   y Second value.
 * @return  The larger of @p x and @p y.
 */
static inline size_t
max(const size_t x, const size_t y)
{
    return x > y ? x : y;
}


/**
 * @brief   Returns the minimum of two values.
 * @param   x First value.
 * @param   y Second value.
 * @return  The smaller of @p x and @p y.
 */
static inline size_t
min(const size_t x, const size_t y)
{
    return x < y ? x : y;
}


/**
 * @brief   Aligns a value to the next multiple of align.
 * @param   value Value to align.
 * @param   align Alignment boundary (must be power of two).
 * @return  Aligned value (rounded up).
 */
static inline size_t
align_value(const size_t value, const size_t align)
{
    return (value + align -1) & ~(align -1);
}

#endif //LIBDS_INTERNAL_UTILS_H