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
#include "impl/contdef.h"

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

#endif //LIBDS_QUEUEDEF_H