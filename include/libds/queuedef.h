/**
 * @file    queuedef.h
 * @brief   Type-safe singly-linked queue generator macro.
 *
 * @author  Gabriel Souza
 * @date    2026-04-13
 */

#ifndef LIBDS_QUEUEDEF_H
#define LIBDS_QUEUEDEF_H

#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>

#include "core.h"
#include "impl/nodechain.h"

#define LIBDS_DEFINE_QUEUE(Type, QueueType, Prefix, CopyFunc, DestroyFunc)      \
                                                                                \
    typedef struct QueueType                                                    \
    {                                                                           \
        const ds_copier_fn      copy;                                           \
        const ds_destructor_fn  destroy;                                        \
        struct ds_node_chain    *_nodes; /* must NOT be modified directly */    \
    } QueueType;                                                                \
                                                                                \
    static inline QueueType                                                     \
    Prefix##_create(void)                                                       \
    {                                                                           \
        size_t value_size  = sizeof(Type);                                      \
        size_t value_align = alignof(Type);                                     \
                                                                                \
        QueueType queue = {                                                     \
            .copy    = (CopyFunc),                                              \
            .destroy = (DestroyFunc),                                           \
            ._nodes  = ds_nc_alloc(value_size, value_align)                     \
        };                                                                      \
        return queue;                                                           \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_delete(QueueType *queue)                                           \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_free(&queue->_nodes, queue->destroy)                          \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_clear(QueueType queue)                                             \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_clear(queue._nodes, queue.destroy, false)                     \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_deep_clear(QueueType queue)                                        \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_clear(queue._nodes, queue.destroy, true)                      \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_copy(QueueType dst_queue, const QueueType src_queue)               \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_copy(                                                         \
                dst_queue._nodes,                                               \
                src_queue._nodes,                                               \
                sizeof(Type),                                                   \
                dst_queue.copy,                                                 \
                dst_queue.destroy                                               \
            )                                                                   \
        );                                                                      \
    }                                                                           \
                                                                                \
    /* support of both `#_length` and `#_size` */                               \
    static inline size_t                                                        \
    Prefix##_length(const QueueType queue)                                        \
    {                                                                           \
        return ds_nc_length(queue._nodes);                                       \
    }                                                                           \
    static inline size_t                                                        \
    Prefix##_size(const QueueType queue)                                          \
    {                                                                           \
        return Prefix##_length(queue);                                           \
    }                                                                           \
                                                                                \
    static inline size_t                                                        \
    Prefix##_bytes(const QueueType queue)                                         \
    {                                                                           \
        return ds_nc_bytes(queue._nodes);                                        \
    }                                                                           \
                                                                                \
    static inline bool                                                          \
    Prefix##_is_empty(const QueueType queue)                                      \
    {                                                                           \
        return ds_nc_is_empty(queue._nodes);                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_get_front(QueueType queue, Type *out)                              \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_front(queue._nodes, &data)                                \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_get_back(QueueType queue, Type *out)                               \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_back(queue._nodes, &data)                                 \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_get_at(QueueType queue, Type *out, const size_t index)             \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_at(queue._nodes, &data, index)                            \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_enqueue(QueueType queue, Type value)                               \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_push_back(queue._nodes, &data)                                 \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!queue.copy)                                                         \
            *((Type *)data) = value;                                            \
        else                                                                    \
        {                                                                       \
            if ( !queue.copy(data, &value) )                                     \
            {                                                                   \
                ds_nc_pop_back(queue._nodes, NULL, NULL);                        \
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
            ds_nc_pop_back(queue._nodes, &data, queue.destroy)                  \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
/* end of macro */    

#endif //LIBDS_QUEUEDEF_H