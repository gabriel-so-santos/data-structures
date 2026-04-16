/**
 * @file core.h
 *
 * @brief   Core configuration, error handling, and fundamental contracts
 *          for the libds data structure library.
 *
 * Provides compile-time configuration flags, error handling mechanisms,
 * and function contracts used across modules.
 *
 * @author  Gabriel Souza
 * @date    2026-02-21
 */

#ifndef LIBDS_CORE_H
#define LIBDS_CORE_H

#include <stddef.h>
#include <stdbool.h>


/**
 * @def     LIBDS_ENABLE_ERROR_PRINT
 * @brief   Controls automatic error reporting to stderr.
 *
 * When enabled (non-zero), any error intercepted by the library will be
 * reported to `stderr`, including contextual diagnostics such as the
 * failed expression, source file, line number, and function name.
 *
 * @note    Defaults to 1 (enabled).
 * @note    Disabling removes all logging overhead but does not affect
 *          error propagation—functions, they will still return error codes.
 */
#ifndef LIBDS_ENABLE_ERROR_PRINT
#define LIBDS_ENABLE_ERROR_PRINT 1
#endif


/**
 * @def     LIBDS_ENABLE_EXIT_ON_FAIL
 * @brief   Enables fail-fast behavior on internal errors.
 *
 * When enabled (non-zero), the library will immediately terminate the
 * program via `exit(EXIT_FAILURE)` whenever an error is detected.
 *
 * @note    Defaults to 0 (disabled).
 * @note    Can be combined with LIBDS_ENABLE_ERROR_PRINT for diagnostics
 *          prior to termination.
 */
#ifndef LIBDS_ENABLE_EXIT_ON_FAIL
#define LIBDS_ENABLE_EXIT_ON_FAIL 0
#endif


/**
 * @def     LIBDS_NC_MIN_BATCH_SIZE
 * @brief   Minimum allocation size for node batches.
 *
 * Defines the lower bound on how many nodes are allocated when the
 * allocator needs to grow. This prevents excessively small allocations
 * during early usage.
 *
 * @note    Must be a strictly positive integer.
 * @note    Effective batch size is:
 *          max(MIN_BATCH_SIZE, length * GROWTH_FACTOR)
 */
#ifndef LIBDS_NC_MIN_BATCH_SIZE
#define LIBDS_NC_MIN_BATCH_SIZE 8
#endif


/**
 * @def     LIBDS_NC_GROWTH_FACTOR
 * @brief   Geometric growth factor for node allocation.
 *
 * Controls how the allocator scales as the data structure grows.
 * Larger values increase allocation size more aggressively, reducing
 * allocation frequency at the cost of higher memory usage.
 *
 * @note    Must be a non-negative floating-point value.
 * @note    Effective batch size is:
 *          max(MIN_BATCH_SIZE, length * GROWTH_FACTOR)
 */
#ifndef LIBDS_NC_GROWTH_FACTOR
#define LIBDS_NC_GROWTH_FACTOR 0.5f
#endif


#if LIBDS_ENABLE_ERROR_PRINT || LIBDS_ENABLE_EXIT_ON_FAIL
#define LIBDS_CHECK(Expr) \
    ds_handle_err((Expr), #Expr, __FILE__, __LINE__, __func__)
#define LIBDS_HANDLE_ERR(ErrCode, Expr, File, Line, Func) \
    ds_handle_err((ErrCode), (Expr), (File), (Line), (Func))

#else //LIBDS_ENABLE_ERROR_PRINT || LIBDS_ENABLE_EXIT_ON_FAIL
#define LIBDS_CHECK(expr) (expr)
#define LIBDS_HANDLE_ERR(ErrCode, Expr, File, Line, Func)
#endif //LIBDS_ENABLE_ERROR_PRINT || LIBDS_ENABLE_EXIT_ON_FAIL #else

/* Debug helpers */
#define LIBDS_STRINGIFY(Name) #Name
#define LIBDS_TOSTRING(Name) LIBDS_STRINGIFY(Name)

/**
 * @struct  ds_node_chain
 * @brief   Opaque handle for the internal node allocator.
 *
 * This structure manages memory layout, allocation strategy, and node
 * lifecycle for all node-based data structures in the library.
 */
struct ds_node_chain;

/**
 * @enum    ds_error
 * @brief   Standard error codes returned by library operations.
 *
 * All primary API functions return one of these values to indicate success
 * or failure. Utility functions that cannot fail may return values directly.
 */
enum ds_error
{
   DS_ERR_NONE = 0,            /**< Operation completed successfully */
   DS_ERR_ALLOCATION_FAILED,   /**< Memory allocation failed */
   DS_ERR_NULL_POINTER,        /**< Invalid null pointer argument */
   DS_ERR_INDEX_OUT_OF_BOUNDS, /**< Index exceeds valid range */
   DS_ERR_EMPTY_STRUCTURE,     /**< Operation invalid on empty structure */
   DS_ERR_COPY_FAILED,         /**< User-defined copy operation failed */
};

/**
 * @brief   Destructor function contract for stored values.
 *
 * @param   data Pointer to a fully initialized element.
 *
 * The destructor is responsible for releasing any resources owned by the
 * value (e.g., heap-allocated fields, internal buffers).
 *
 * @note    The memory pointed to by `value` itself is managed by the library
 *          and must NOT be freed by this function.
 *
 * @warning The function must assume `value` is valid and initialized.
 */
typedef void (*ds_destructor_fn)(void *data);

/**
 * @brief   Copier function contract for value duplication.
 *
 * @param   dst Pointer to uninitialized destination memory.
 * @param   src Pointer to a valid source value.
 *
 * The function must perform a deep copy from `src_value` into `dst_value`,
 * fully initializing the destination.
 *
 * @return  true  if the copy succeeded
 * @return  false if the copy failed
 *
 * @failure If the function fails after partially allocating resources,
 *          it MUST clean up any intermediate allocations and leave
 *          `dst_value` in a safely discardable state.
 *
 * @note    On failure, the library will NOT invoke the destructor on
 *          `dst_value`.
 */
typedef bool (*ds_copier_fn)(void *dst, const void *src);

/**
 * @brief   Converts an error code into a string.
 *
 * @param   err Error code.
 * @return  Constant string describing the error.
 */
const char *
ds_err_to_string(enum ds_error err);

/**
 * @brief   Centralized error handling hook.
 *
 * @param   err   Error code returned by an operation.
 * @param   expr  Stringified expression that produced the error.
 * @param   file  Source file where the error occurred.
 * @param   line  Line number of the failure.
 * @param   func  Function name where the error occurred.
 *
 * @return  The same error code passed in.
 *
 * Depending on configuration macros, this function may:
 * - Log diagnostic information to stderr
 * - Terminate the program
 *
 * @note    Intended to be used through the LIBDS_CHECK macro.
 */
enum ds_error
ds_handle_err(enum ds_error err, const char *expr, const char *file, int line, const char *func);

#endif //LIBDS_CORE_H
