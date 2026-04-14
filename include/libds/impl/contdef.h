/**
 * @file    contdef.h
 * @brief   Base container for lists, stacks, and queues
 *
 * @author  Gabriel Souza
 * @date    2026-04-14
 */

#ifndef LIBDS_IMPL_CONTDEF_H
#define LIBDS_IMPL_CONTDEF_H

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>

#include "libds/core.h"
#include "nodechain.h"

#define LIBDS_DEFINE_CONTAINER(Type, ContainerType, Prefix,                     \
    CopyFunc, DestroyFunc)                                                      \
                                                                                \
    typedef struct ContainerType                                                \
    {                                                                           \
        const ds_copier_fn      copy;                                           \
        const ds_destructor_fn  destroy;                                        \
        struct ds_node_chain    *_nodes; /* must NOT be modified directly */    \
    } ContainerType;                                                            \
                                                                                \
    static inline ContainerType                                                 \
    Prefix##_create(void)                                                       \
    {                                                                           \
        size_t value_size  = sizeof(Type);                                      \
        size_t value_align = alignof(Type);                                     \
                                                                                \
        ContainerType cont = {                                                  \
            .copy    = (CopyFunc),                                              \
            .destroy = (DestroyFunc),                                           \
            ._nodes  = ds_nc_alloc(value_size, value_align)                     \
        };                                                                      \
        return cont;                                                            \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_delete(ContainerType *cont)                                        \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_free(&cont->_nodes, cont->destroy)                            \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_clear(ContainerType cont)                                          \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_clear(cont._nodes, cont.destroy, false)                       \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_deep_clear(ContainerType cont)                                     \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_clear(cont._nodes, cont.destroy, true)                        \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_copy(ContainerType dst_cont, const ContainerType src_cont)         \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_copy(                                                         \
                dst_cont._nodes,                                                \
                src_cont._nodes,                                                \
                sizeof(Type),                                                   \
                dst_cont.copy,                                                  \
                dst_cont.destroy                                                \
            )                                                                   \
        );                                                                      \
    }                                                                           \
                                                                                \
    /* support of both `#_length` and `#_size` */                               \
    static inline size_t                                                        \
    Prefix##_length(const ContainerType cont)                                   \
    {                                                                           \
        return ds_nc_length(cont._nodes);                                       \
    }                                                                           \
    static inline size_t                                                        \
    Prefix##_size(const ContainerType cont)                                     \
    {                                                                           \
        return ds_nc_length(cont._nodes);                                       \
    }                                                                           \
                                                                                \
    static inline size_t                                                        \
    Prefix##_bytes(const ContainerType cont)                                    \
    {                                                                           \
        return ds_nc_bytes(cont._nodes);                                        \
    }                                                                           \
                                                                                \
    static inline bool                                                          \
    Prefix##_is_empty(const ContainerType cont)                                 \
    {                                                                           \
        return ds_nc_is_empty(cont._nodes);                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_get_front(ContainerType cont, Type *out)                           \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_front(cont._nodes, &data)                                 \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_get_back(ContainerType cont, Type *out)                            \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_back(cont._nodes, &data)                                  \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_get_at(ContainerType cont, const size_t index, Type *out)          \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_at(cont._nodes, index, &data)                             \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
/* end of macro */

#endif //LIBDS_IMPL_CONTDEF_H