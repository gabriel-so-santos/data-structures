/**
 * @file core.h
 *
 * @brief
 *
 * @author Gabriel Souza
 * @date   2026-02-21
 */

#ifndef LIBDS_CORE_H
#define LIBDS_CORE_H

#include <stddef.h>
#include <stdbool.h>


/**
 * @def     LIBDS_ENABLE_ERROR_PRINT
 * @brief   Enables or disables automatic error logging to stderr.
 *
 * When defined to a non-zero value, internal errors intercepted by the library
 * will be printed to standard error (`stderr`). The log includes the error type,
 * file, line, and function where the failure occurred.
 *
 * @note    Defaults to 1 (enabled). Define this as 0 to silence library output
 *          and give no error calling overhead.
 *
 * @note    The functions will return error codes regardless of this setting.
 */
#ifndef LIBDS_ENABLE_ERROR_PRINT
#define LIBDS_ENABLE_ERROR_PRINT 1
#endif


/**
 * @def     LIBDS_ENABLE_EXIT_ON_FAIL
 * @brief   Enables or disables immediate program termination on error.
 *
 * When defined to a non-zero value, any error intercepted by the library will
 * cause the application to immediately terminate via `exit(EXIT_FAILURE)`.
 * This is highly useful for strict debug environments where data structure
 * failures should fail fast.
 *
 * @note    Defaults to 0 (disabled). Can be combined with LIBDS_ENABLE_ERROR_PRINT.
 */
#ifndef LIBDS_ENABLE_EXIT_ON_FAIL
#define LIBDS_ENABLE_EXIT_ON_FAIL 0
#endif


/**
 * @def     LIBDS_NC_MIN_BATCH_SIZE
 * @brief   Minimum number of nodes to allocate in a batch.
 *
 * When no free nodes are available, the allocator creates new nodes in batches.
 * This value sets the lower bound for batch size.
 * This value must be a positive integer number.
 *
 * @note    batch_size = max(MIN_BATCH_SIZE, length * GROWTH_FACTOR).
 */
#ifndef LIBDS_NC_MIN_BATCH_SIZE
#define LIBDS_NC_MIN_BATCH_SIZE 8
#endif


/**
 * @def     LIBDS_NC_GROWTH_FACTOR
 * @brief   Growth factor for dynamic batch sizing.
 *
 * Determines the dynamic batch size of allocated nodes.
 * This allows the allocator to scale with the data structure size.
 * This value must be a float.
 *
 * @note    batch_size = max(MIN_BATCH_SIZE, length * GROWTH_FACTOR).
 */
#ifndef LIBDS_NC_GROWTH_FACTOR
#define LIBDS_NC_GROWTH_FACTOR 0.125f
#endif

#if LIBDS_ENABLE_ERROR_PRINT || LIBDS_ENABLE_EXIT_ON_FAIL
#define LIBDS_CHECK(expr) ds_handle_err((expr), #expr, __FILE__, __LINE__, __func__)
#else //LIBDS_ENABLE_ERROR_PRINT || LIBDS_ENABLE_EXIT_ON_FAIL
#define LIBDS_CHECK(expr) (expr)
#endif //LIBDS_ENABLE_ERROR_PRINT || LIBDS_ENABLE_EXIT_ON_FAIL #else

/* Debug helpers */
#define LIBDS_STRINGIFY(Name) #Name
#define LIBDS_TOSTRING(Name) LIBDS_STRINGIFY(Name)

/**
 * internal node chain opaque struct
 */
struct ds_node_chain;

/**
 *
 */
enum ds_error
{
    DS_ERR_NONE = 0,            /* No error (will be always 0) */
    DS_ERR_ALLOCATION_FAILED,   /* Memory allocation failed */
    DS_ERR_NULL_POINTER,        /* Invalid null pointer provided */
    DS_ERR_INDEX_OUT_OF_BOUNDS, /* Index out of bounds */
    DS_ERR_EMPTY_STRUCTURE,     /* Invalid operation on an empty structure */
    DS_ERR_COPY_FAILED,         /* Custom copy function failed */
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
typedef void (*ds_destructor_fn)(void *);

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
typedef bool (*ds_copier_fn)(void *dst_value, const void *src_value);

const char *
ds_err_to_string(enum ds_error err);

enum ds_error
ds_handle_err(enum ds_error err, const char *expr, const char *file, int line, const char *func);

#endif //LIBDS_CORE_H
