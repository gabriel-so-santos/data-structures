/**
 * @file    stackdef.h
 * @brief   Type-safe stack (LIFO) generator macro.
 *
 * @author  Gabriel Souza
 * @date    2026-04-13
 *
 * This module provides a generic Stack (Last-In, First-Out) implementation
 * through the @ref LIBDS_DEF_STACK macro. Like the list implementation, it
 * features geometric memory allocation, node recycling, and configurable
 * copy/destroy functions.
 *
 * Key features:
 * - Strict LIFO semantics via push/pop operations
 * - O(1) insertion and removal
 * - Type-safe code generation (no void* casting required)
 * - Intrusive memory layout for cache efficiency
 * - Ownership transfer via pop operation
 *
 * @note Requires C11 or later due to _Alignof() usage
 * @warning This implementation does NOT PROVIDE THREAD-SAFETY.
 * @warning Direct manipulation of the `_nodes` member causes undefined behavior.
 *
 * @see listdef.h, queuedef.h, core.h, impl/nodechain.h
 */

#ifndef LIBDS_STACKDEF_H
#define LIBDS_STACKDEF_H

#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>

#include "core.h"
#include "impl/nodechain.h"
#include "impl/contdef.h"

/**
 * @defgroup StackContainer Stack Container
 * @brief   LIFO (Last-In, First-Out) container with O(1) push and pop.
 * @{
 */

/**
 * @def LIBDS_DEF_STACK
 * @brief   Generate a complete type-safe stack container interface
 * @param   Type        The data type to store (must be a complete type)
 * @param   StackType   Name of the generated container structure
 * @param   Prefix      Function prefix for all generated operations
 * @param   CopyFunc    Copy function (ds_copier_fn) or NULL for bitwise assignment
 * @param   DestroyFunc Destroy function (ds_destructor_fn) or NULL for no cleanup
 *
 * This macro expands to define a structure type `StackType` and a suite of
 * operations prefixed with `Prefix_`.
 *
 * @par Ownership Transfer
 * The `pop` function transfers ownership of the payload to the caller when a
 * valid `out` pointer is provided. Passing NULL triggers automatic destruction.
 *
 * @par Example: Basic Stack Usage
 * @code
 *  #include <stdio.h>
 *  #include <libds/stackdef.h>
 *
 *  LIBDS_DEF_STACK(double, StackDouble, sd, NULL, NULL)
 *
 *  int main()
 *  {
 *      StackDouble numbers = sd_create();
 *
 *      sd_push(numbers, 3.14159265358);
 *      sd_push(numbers, 2.71828182845);
 *      sd_push(numbers, 1.61803398875);
 *
 *      double value;
 *      sd_pop(numbers, &value);
 *      printf("Popped: %f\n", value); // Output: 1.61803398875
 *
 *      sd_delete(&numbers);
 *      return 0;
 *  }
 * @endcode
 *
 * @par Generated Functions (All prefixed with `Prefix_`)
 *
 * **Creation & Destruction:**
 * - `create(void)` - Allocate and initialize new container
 * - `delete(StackType*)` - Free all nodes and nullify reference
 * - `clear(StackType)` - Remove all elements (preserves capacity)
 * - `deep_clear(StackType)` - Clear and free recycled nodes
 * - `copy(StackType, const StackType)` - Deep copy container
 *
 * **Stack Operations:**
 * - `push(StackType, Type)` - Push element to top of stack O(1)
 * - `pop(StackType, Type*)` - Pop element from top with ownership transfer O(1)
 *
 * **Access (Common):**
 * - `get_front(StackType, Type*)` - Peek the top element O(1)
 * - `get_back(StackType, Type*)` - Peek the bottom element O(1)
 * - `get_at(StackType, size_t, Type*)` - Peek at index O(N)
 *
 * **Query:**
 * - `length(StackType)` / `size(StackType)` - Element count O(1)
 * - `bytes(StackType)` - Total allocated memory O(log N)
 * - `is_empty(StackType)` - Check if empty O(1)
 *
 * @note All operations that return @ref ds_error will report failures via stderr
 * if @ref LIBDS_ENABLE_ERROR_PRINT is enabled.
 */
#define LIBDS_DEF_STACK(Type, StackType, Prefix, CopyFunc, DestroyFunc)         \
                                                                                \
    LIBDS_DEF_CONTAINER(Type, StackType, Prefix, CopyFunc, DestroyFunc)         \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_push(StackType stack, Type value)                                  \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_push_front(stack._nodes, &data)                               \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!stack.copy)                                                        \
            *((Type *)data) = value;                                            \
        else                                                                    \
        {                                                                       \
            if ( !stack.copy(data, &value) )                                    \
            {                                                                   \
                ds_nc_pop_front(stack._nodes, NULL, NULL);                      \
                                                                                \
                LIBDS_HANDLE_ERR(                                               \
                    DS_ERR_COPY_FAILED,                                         \
                    LIBDS_STRINGIFY(stack.copy(data, &value)),                  \
                    __FILE__, __LINE__, __func__                                \
                );                                                              \
                return DS_ERR_COPY_FAILED;                                      \
            }                                                                   \
        }                                                                       \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_pop(StackType stack, Type *out)                                    \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_pop_front(stack._nodes, &data, stack.destroy)                 \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!out && stack.destroy)                                              \
            stack.destroy(data);                                                \
                                                                                \
        else if (out)                                                           \
            *out = *((Type *)data);                                             \
                                                                                \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
/* end of macro */

/**@}*/ //end of StackContainer group

#endif //LIBDS_STACKDEF_H