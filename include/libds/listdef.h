/**
 * @file    listdef.h
 * @brief   Type-safe singly-linked list generator macro.
 *
 * @author  Gabriel Souza
 * @date    2026-04-09
 *
 * This module provides a generic list implementation through the @ref LIBDS_DEF_LIST macro.
 * The list features geometric memory allocation, node recycling, and configurable
 * copy/destroy functions for managing payload memory of any data type.
 *
 * Key features:
 * - Type-safe code generation (no void* casting required)
 * - Intrusive memory layout for cache efficiency
 * - Node recycling for O(1) reuse of deleted elements
 * - Custom copy/destroy functions for complex data types
 * - Ownership transfer via pop operations
 *
 * @note Requires C11 or later due to _Alignof() usage
 * @warning This implementation does NOT PROVIDE THREAD-SAFETY.
 * @warning Direct manipulation of the `_nodes` member causes undefined behavior.
 *
 * @see stackdef.h, queuedef.h, core.h, impl/nodechain.h
 */

#ifndef LIBDS_LISTDEF_H
#define LIBDS_LISTDEF_H

#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>

#include "core.h"
#include "impl/nodechain.h"
#include "impl/contdef.h"

/**
 * @defgroup SinglyLinkedList List Container
 * @brief   Singly-linked list with O(1) front operations and O(N) indexed access
 * @{
 */

/**
 * @def LIBDS_DEF_LIST
 * @brief   Generate a complete type-safe list container interface
 * @param   Type        The data type to store (must be a complete type)
 * @param   ListType    Name of the generated container structure
 * @param   Prefix      Function prefix for all generated operations
 * @param   CopyFunc    Copy function (ds_copier_fn) or NULL for bitwise assignment
 * @param   DestroyFunc Destroy function (ds_destructor_fn) or NULL for no cleanup
 *
 * This macro expands to define:
 * - A container structure type `ListType` wrapping @ref ds_node_chain
 * - A complete set of operations prefixed with `Prefix_`
 * - Automatic memory management with geometric growth
 * - Type-safe insertion/removal with custom copy/destroy semantics
 *
 * @par Memory Management
 * The generated container manages two aspects of memory:
 * - Node memory (handled automatically by the library)
 * - Payload memory (handled by CopyFunc and DestroyFunc)
 *
 * @par Custom Copy Function
 * The CopyFunc (type @ref ds_copier_fn) is called during insertion operations.
 * It must copy data from `src` into uninitialized memory at `dst`.
 *
 * @par Custom Destroy Function
 * The DestroyFunc (type @ref ds_destructor_fn) is called when removing elements.
 * It must free any dynamically allocated resources within the payload.
 *
 * @par Ownership Transfer
 * Functions that perform a pop (`pop_front`, `pop_back`, and `pop_at`) transfer
 * ownership of the payload to the caller, who must free it. Passing NULL as the
 * output parameter triggers automatic destruction.
 *
 * @par Example: Basic Usage with Integers
 * @code
 *  #include <stdio.h>
 *  #include <libds/listdef.h>
 *
 *  LIBDS_DEF_LIST(int, ListInt, li, NULL, NULL)
 *
 *  int main()
 *  {
 *      ListInt list = li_create();
 *
 *      li_push_back(list, 10);
 *      li_push_front(list, 5);
 *      li_push_at(list, 1, 7);
 *
 *      int value;
 *      li_pop_front(list, &value);
 *      printf("Value: %d\n", value); // Output: 5
 *
 *      printf("Length: %zu\n", li_length(list)); // Output: 2
 *      li_delete(&list);
 *
 *      return 0;
 *  }
 * @endcode
 *
 * @par Example: Storing Strings with Custom Copy/Destroy
 * @code
 *  #include <stdio.h>
 *  #include <stdlib.h>
 *  #include <string.h>
 *  #include <libds/listdef.h>
 *
 *  bool copy_str(void *dst, const void *src)
 *  {
 *      if (!dst || !src) return false;
 *      const size_t len = strlen(*(const char **)src);
 *
 *      char *new_str = malloc(len + 1);
 *      if (!new_str) return false;
 *      new_str[len] = '\0';
 *
 *      strncpy(new_str, *(const char **)src, len);
 *      *(char **)dst = new_str;
 *      return true;
 *  }
 *
 *  void destroy_str(void *data)
 *  {
 *      if (!data) return;
 *      free(*(char **)data);
 *      *(char **)data = NULL;
 *  }
 *
 *  LIBDS_DEF_LIST(char *, ListStr, ls, copy_str, destroy_str)
 *
 *  int main()
 *  {
 *      ListStr rock_bands = ls_create();
 *
 *      ls_prepend(rock_bands, "The Beatles");  // or ls_push_front
 *      ls_append(rock_bands, "Queen");         // or ls_push_back
 *      ls_push_at(rock_bands, 1, "AC/DC");
 *
 *      // new order: ["Queen", "AC/DC", "The Beatles"]
 *      ls_reverse(rock_bands);
 *
 *      // remove "The Beatles", destructor is called automatically
 *      ls_drop_at(rock_bands, 2);
 *
 *      char *out;
 *      ls_pop_at(rock_bands, 0, &out);
 *      printf("Band: %s\n", out);              // pops "Queen"
 *      free(out);                              // ownership transferred to 'out'
 *
 *      ls_delete(&rock_bands);                 // automatically frees "AC/DC"
 *      return 0;
 *  }
 * @endcode
 *
 * @par Generated Functions (All prefixed with `Prefix_`)
 *
 * **Creation & Destruction:**
 * - `create(void)` - Allocate and initialize new container
 * - `delete(ListType*)` - Free all nodes and nullify reference
 * - `clear(ListType)` - Remove all elements (preserves capacity)
 * - `deep_clear(ListType)` - Clear and free recycled nodes
 * - `copy(ListType, const ListType)` - Deep copy container
 *
 * **Insertion:**
 * - `push_front(ListType, Type)` / `prepend` - Insert at beginning O(1)
 * - `push_back(ListType, Type)` / `append` - Insert at end O(1)
 * - `push_at(ListType, size_t, Type)` - Insert at index O(N)
 *
 * **Removal (with ownership transfer):**
 * - `pop_front(ListType, Type*)` - Remove first element O(1)
 * - `pop_back(ListType, Type*)` - Remove last element O(N)
 * - `pop_at(ListType, size_t, Type*)` - Remove at index O(N)
 *
 * **Removal (automatic destruction):**
 * - `drop_front(ListType)` - Discard first element O(1)
 * - `drop_back(ListType)` - Discard last element O(N)
 * - `drop_at(ListType, size_t)` - Discard at index O(N)
 *
 * **Access:**
 * - `get_front(ListType, Type*)` - Peek first element O(1)
 * - `get_back(ListType, Type*)` - Peek last element O(1)
 * - `get_at(ListType, size_t, Type*)` - Peek at index O(N)
 *
 * **Modification:**
 * - `set_front(ListType, Type)` - Replace first element O(1)
 * - `set_back(ListType, Type)` - Replace last element O(1)
 * - `set_at(ListType, size_t, Type)` - Replace at index O(N)
 * - `reverse(ListType)` - Reverse list order O(N)
 *
 * **Query:**
 * - `length(ListType)` / `size(ListType)` - Element count O(1)
 * - `bytes(ListType)` - Total allocated memory O(log N)
 * - `is_empty(ListType)` - Check if empty O(1)
 *
 * @note The `prepend` and `append` functions are aliases for `push_front` and
 * `push_back`.
 *
 * @note All operations that return @ref ds_error will report failures via stderr
 * if @ref LIBDS_ENABLE_ERROR_PRINT is enabled.
 *
 * @note The library may call exit() if @ref LIBDS_ENABLE_EXIT_ON_FAIL is enabled.
 */
#define LIBDS_DEF_LIST(Type, ListType, Prefix, CopyFunc, DestroyFunc)           \
                                                                                \
    LIBDS_DEF_CONTAINER(Type, ListType, Prefix, CopyFunc, DestroyFunc)          \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_reverse(ListType list)                                             \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_reverse(list._nodes)                                          \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_set_front(ListType list, Type value)                               \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_front(list._nodes, &data)                                 \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        *((Type *)data) = value;                                                \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_set_back(ListType list, Type value)                                \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_back(list._nodes, &data)                                  \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        *((Type *)data) = value;                                                \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_set_at(ListType list, const size_t index, Type value)              \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_at(list._nodes, index, &data)                             \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        *((Type *)data) = value;                                                \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_push_front(ListType list, Type value)                              \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_push_front(list._nodes, &data)                                \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!list.copy)                                                         \
            *((Type *)data) = value;                                            \
        else                                                                    \
        {                                                                       \
            if ( !list.copy(data, &value) )                                     \
            {                                                                   \
                ds_nc_pop_front(list._nodes, NULL, NULL);                       \
                                                                                \
                LIBDS_HANDLE_ERR(                                               \
                    DS_ERR_COPY_FAILED,                                         \
                    LIBDS_STRINGIFY(list.copy(data, &value)),                   \
                    __FILE__, __LINE__, __func__                                \
                );                                                              \
                return DS_ERR_COPY_FAILED;                                      \
            }                                                                   \
        }                                                                       \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
    /* support of both `#_push_front` and `#_prepend` */                        \
    static inline enum ds_error                                                 \
    Prefix##_prepend(ListType list, Type value)                                 \
    {                                                                           \
        return Prefix##_push_front(list, value);                                \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_push_back(ListType list, Type value)                               \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_push_back(list._nodes, &data)                                 \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!list.copy)                                                         \
            *((Type *)data) = value;                                            \
        else                                                                    \
        {                                                                       \
            if ( !list.copy(data, &value) )                                     \
            {                                                                   \
                ds_nc_pop_back(list._nodes, NULL, NULL);                        \
                                                                                \
                LIBDS_HANDLE_ERR(                                               \
                    DS_ERR_COPY_FAILED,                                         \
                    LIBDS_STRINGIFY(list.copy(data, &value)),                   \
                    __FILE__, __LINE__, __func__                                \
                );                                                              \
                return DS_ERR_COPY_FAILED;                                      \
            }                                                                   \
        }                                                                       \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
    /* support of both `#_push_back` and `#_append` */                          \
    static inline enum ds_error                                                 \
    Prefix##_append(ListType list, Type value)                                  \
    {                                                                           \
        return Prefix##_push_back(list, value);                                 \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_push_at(ListType list, const size_t index, Type value)             \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_push_at(list._nodes, index, &data)                            \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!list.copy)                                                         \
            *((Type *)data) = value;                                            \
        else                                                                    \
        {                                                                       \
            if (!list.copy(data, &value))                                       \
            {                                                                   \
                ds_nc_pop_at(list._nodes, index, NULL, NULL);                   \
                                                                                \
                LIBDS_HANDLE_ERR(                                               \
                    DS_ERR_COPY_FAILED,                                         \
                    LIBDS_STRINGIFY(list.copy(data, &value)),                   \
                    __FILE__, __LINE__, __func__                                \
                );                                                              \
                return DS_ERR_COPY_FAILED;                                      \
            }                                                                   \
        }                                                                       \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_drop_front(ListType list)                                          \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_pop_front(list._nodes, NULL, list.destroy)                    \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_drop_back(ListType list)                                           \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_pop_back(list._nodes, NULL, list.destroy)                     \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_drop_at(ListType list, const size_t index)                         \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_pop_at(list._nodes, index, NULL, list.destroy)                \
        );                                                                      \
    }                                                                           \
    static inline enum ds_error                                                 \
    Prefix##_pop_front(ListType list, Type *out)                                \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_pop_front(list._nodes, &data, list.destroy)                   \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_pop_back(ListType list, Type *out)                                 \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_pop_back(list._nodes, &data, list.destroy)                    \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_pop_at(ListType list, const size_t index, Type *out)               \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_pop_at(list._nodes, index, &data, list.destroy)               \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!out && list.destroy)                                               \
            list.destroy(data);                                                 \
                                                                                \
        else if (out)                                                           \
            *out = *((Type *)data);                                             \
                                                                                \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
/* end of macro */

/** @} */ //end of SinglyLinkedList group

#endif //LIBDS_LISTDEF_H
