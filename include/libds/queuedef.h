/**
 * @file    queuedef.h
 * @brief   Type-safe queue (FIFO) generator macro.
 *
 * @author  Gabriel Souza
 * @date    2026-04-13
 *
 * This module provides a generic Queue (First-In, First-Out) implementation
 * through the @ref LIBDS_DEF_QUEUE macro. It shares the same robust backend
 * as the list and stack implementations.
 *
 * Key features:
 * - Strict FIFO semantics via enqueue/dequeue operations
 * - O(1) insertion and removal
 * - Type-safe code generation (no void* casting required)
 * - Intrusive memory layout for cache efficiency
 * - Ownership transfer via dequeue operation
 *
 * @note Requires C11 or later due to _Alignof() usage
 * @warning This implementation does NOT PROVIDE THREAD-SAFETY.
 * @warning Direct manipulation of the `_nodes` member causes undefined behavior.
 *
 * @see listdef.h, stackdef.h, core.h, impl/nodechain.h
 */

#ifndef LIBDS_QUEUEDEF_H
#define LIBDS_QUEUEDEF_H

#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>

#include "core.h"
#include "impl/nodechain.h"
#include "impl/contdef.h"

/**
 * @defgroup QueueContainer Queue Container
 * @brief   FIFO (First-In, First-Out) container with O(1) enqueue and dequeue.
 * @{
 */

/**
 * @def LIBDS_DEF_QUEUE
 * @brief   Generate a complete type-safe queue container interface
 * @param   Type        The data type to store (must be a complete type)
 * @param   QueueType   Name of the generated container structure
 * @param   Prefix      Function prefix for all generated operations
 * @param   CopyFunc    Copy function (ds_copier_fn) or NULL for simple assignment
 * @param   DestroyFunc Destroy function (ds_destructor_fn) or NULL for no cleanup
 *
 * This macro expands to define a structure type `QueueType` and a suite of
 * operations prefixed with `Prefix_`.
 *
 * @par Ownership Transfer
 * The `dequeue` function transfers ownership of the payload to the caller when a
 * valid `out` pointer is provided. Passing NULL triggers automatic destruction.
 *
 * @par Example: Basic Queue Usage
 * @code
 *  #include <stdio.h>
 *  #include <libds/queuedef.h>
 *
 *  typedef struct
 *  {
 *      int id;
 *      char descr[32];
 *  } Task;
 *
 *  LIBDS_DEF_QUEUE(Task, TaskQueue, tq, NULL, NULL)
 *
 *  int main()
 *  {
 *      TaskQueue queue = tq_create();
 *
 *      tq_enqueue(queue, (Task){.id=101, .descr="Do the dishes"});
 *      tq_enqueue(queue, (Task){.id=102, .descr="Take out the trash"});
 *      tq_enqueue(queue, (Task){.id=103, .descr="Water the plants"});
 *
 *      Task out;
 *      tq_dequeue(queue, &out);
 *      printf("Task(%d): %s\n", out.id, out.descr);
 *      // Output: "Task(101): Do the dishes"
 *
 *      tq_delete(&queue);
 *      return 0;
 *  }
 * @endcode
 *
 * @par Generated Functions (All prefixed with `Prefix_`)
 *
 * **Creation & Destruction:**
 * - `create(void)` - Allocate and initialize new container
 * - `delete(QueueType*)` - Free all nodes and nullify reference
 * - `clear(QueueType)` - Remove all elements (preserves capacity)
 * - `deep_clear(QueueType)` - Clear and free recycled nodes
 * - `copy(QueueType, const QueueType)` - Deep copy container
 *
 * **Queue Operations:**
 * - `enqueue(QueueType, Type)` - Insert element at the back O(1)
 * - `dequeue(QueueType, Type*)` - Remove element from the front with ownership
 * transfer O(1)
 *
 * **Access (Common):**
 * - `get_front(QueueType, Type*)` - Peek the front element O(1)
 * - `get_back(QueueType, Type*)` - Peek the back element O(1)
 * - `get_at(QueueType, size_t, Type*)` - Peek at index O(N)
 *
 * **Query:**
 * - `length(QueueType)` / `size(QueueType)` - Element count O(1)
 * - `bytes(QueueType)` - Total allocated memory O(log N)
 * - `is_empty(QueueType)` - Check if empty O(1)
 *
 * @note All operations that return @ref ds_error will report failures via stderr
 * if @ref LIBDS_ENABLE_ERROR_PRINT is enabled.
 */
#define LIBDS_DEF_QUEUE(Type, QueueType, Prefix, CopyFunc, DestroyFunc)         \
                                                                                \
    LIBDS_DEF_CONTAINER(Type, QueueType, Prefix, CopyFunc, DestroyFunc)         \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_enqueue(QueueType queue, Type value)                               \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_push_back(queue._nodes, &data)                                \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!queue.copy)                                                        \
            *((Type *)data) = value;                                            \
        else                                                                    \
        {                                                                       \
            if ( !queue.copy(data, &value) )                                    \
            {                                                                   \
                ds_nc_pop_back(queue._nodes, NULL, NULL);                       \
                                                                                \
                LIBDS_HANDLE_ERR(                                               \
                    DS_ERR_COPY_FAILED,                                         \
                    LIBDS_STRINGIFY(queue.copy(data, &value)),                  \
                    __FILE__, __LINE__, __func__                                \
                );                                                              \
                return DS_ERR_COPY_FAILED;                                      \
            }                                                                   \
        }                                                                       \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_dequeue(QueueType queue, Type *out)                                \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_pop_front(queue._nodes, &data, queue.destroy)                 \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
/* end of macro */    

/** @} */ //end of QueueContainer group

#endif //LIBDS_QUEUEDEF_H