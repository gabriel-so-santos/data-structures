//
// Created by Gabriel Souza on 21/02/2026.
//

#ifndef DATA_STRUCTURES_CORE_H
#define DATA_STRUCTURES_CORE_H

#include <stddef.h>
#include <stdbool.h>

#ifndef LIBDS_ENABLE_ERROR_PRINT
#define LIBDS_ENABLE_ERROR_PRINT 1
#endif

#ifndef LIBDS_ENABLE_EXIT_ON_FAIL
#define LIBDS_ENABLE_EXIT_ON_FAIL 0
#endif

#if LIBDS_ENABLE_ERROR_PRINT || LIBDS_ENABLE_EXIT_ON_FAIL
#define LIBDS_CHECK(expr) ds_handle_err((expr), #expr, __FILE__, __LINE__, __func__)
#else //LIBDS_ENABLE_ERROR_PRINT || LIBDS_ENABLE_EXIT_ON_FAIL
#define LIBDS_CHECK(expr) (expr)
#endif //LIBDS_ENABLE_ERROR_PRINT || LIBDS_ENABLE_EXIT_ON_FAIL #else

/* Debug helpers */
#define LIBDS_STRINGIFY(Name) #Name
#define LIBDS_TOSTRING(Name) LIBDS_STRINGIFY(Name)

struct ds_node_chain;

enum ds_error
{
    DS_ERR_NONE = 0,            /* No error */
    DS_ERR_ALLOCATION_FAILED,   /* Memory allocation failed */
    DS_ERR_NULL_POINTER,        /* Invalid null pointer provided */
    DS_ERR_INDEX_OUT_OF_BOUNDS, /* Index out of bounds */
    DS_ERR_EMPTY_STRUCTURE,     /* Invalid operation on an empty structure */
    DS_ERR_COPY_FAILED,
};

/**
 * @brief Destructor function contract.
 *
 * Expectation: The `value` pointer contains fully initialized valid data.
 *
 * Responsibility: It must free any dynamic memory allocated *inside* the
 * structure (e.g., nested pointers, strings).
 *
 * @note It should NOT free the `value` pointer itself, as the library
 * manages the node's memory.
 */
typedef void (*ds_destructor_t)(void *);

/**
 * @brief Copier function contract.
 *
 * Expectation: `src_value` is fully valid. `dst_value` is uninitialized
 * raw memory provided by the library.
 *
 * Responsibility: Perform a deep copy of `src_value` into `dst_value`.
 *
 * @return `true` on success, `false` on failure
 * - On Success: The library will assume `dst_value` is now fully valid.
 * - On Failure: If the copy fails halfway through (e.g., a nested malloc fails),
 * the function MUST clean up any memory it already allocated inside `dst_value`,
 * leave `dst_value` safely abandoned.
 *
 * @note The library will NOT call the destructor on `dst_value` if this returns `false`.
 */
typedef bool (*ds_copier_t)(void *dst_value, const void *src_value);

const char *
ds_err_to_string(enum ds_error err);

enum ds_error
ds_handle_err(enum ds_error err, const char *expr, const char *file, int line, const char *func);

#endif //DATA_STRUCTURES_CORE_H
