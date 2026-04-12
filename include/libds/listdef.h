/**
 * @file    listdef.h
 * @brief   Type-safe singly-linked list generator macro.
 *
 * @author  Gabriel Souza
 * @date    2026-04-09
 */

#ifndef LIBDS_LIST_DEF_H
#define LIBDS_LIST_DEF_H

#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>

#include "core.h"
#include "impl/nodechain.h"

#define LIBDS_DEFINE_LIST(Type, ListType, Prefix, CopyFunc, DestroyFunc)        \
                                                                                \
    typedef struct ListType                                                     \
    {                                                                           \
        const ds_copier_fn      copy;                                           \
        const ds_destructor_fn  destroy;                                        \
        struct ds_node_chain    *_nodes; /* must NOT be modified directly */    \
    } ListType;                                                                 \
                                                                                \
    static inline ListType                                                      \
    Prefix##_create(void)                                                       \
    {                                                                           \
        size_t value_size  = sizeof(Type);                                      \
        size_t value_align = alignof(Type);                                     \
                                                                                \
        ListType list = {                                                       \
            .copy     = (CopyFunc),                                             \
            .destroy  = (DestroyFunc),                                          \
            ._nodes   = ds_nc_alloc(value_size, value_align)                    \
        };                                                                      \
        return list;                                                            \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_delete(ListType *list)                                             \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_free(&list->_nodes, list->destroy)                            \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_clear(ListType list)                                               \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_clear(list._nodes, list.destroy)                              \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_copy(ListType dst_list, const ListType src_list)                   \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_copy(                                                         \
                dst_list._nodes,                                                \
                src_list._nodes,                                                \
                sizeof(Type),                                                   \
                dst_list.copy,                                                  \
                dst_list.destroy                                                \
            )                                                                   \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_reverse(ListType list)                                             \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_reverse(list._nodes)                                          \
        );                                                                      \
    }                                                                           \
                                                                                \
    /* support of both `#_length` and `#_size` */                               \
    static inline size_t                                                        \
    Prefix##_length(const ListType list)                                        \
    {                                                                           \
        return ds_nc_length(list._nodes);                                       \
    }                                                                           \
    static inline size_t                                                        \
    Prefix##_size(const ListType list)                                          \
    {                                                                           \
        return Prefix##_length(list);                                           \
    }                                                                           \
                                                                                \
    static inline size_t                                                        \
    Prefix##_bytes(const ListType list)                                         \
    {                                                                           \
        return ds_nc_bytes(list._nodes);                                        \
    }                                                                           \
                                                                                \
    static inline bool                                                          \
    Prefix##_is_empty(const ListType list)                                      \
    {                                                                           \
        return ds_nc_is_empty(list._nodes);                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_get_front(ListType list, Type *out)                                \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_front(list._nodes, &data)                                 \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_get_back(ListType list, Type *out)                                 \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_back(list._nodes, &data)                                  \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_get_at(ListType list, Type *out, long long index)                  \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_at(list._nodes, &data, index)                             \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    /* support of both `#_push_front` and `#_prepend` */                        \
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
                return DS_ERR_COPY_FAILED;                                      \
            }                                                                   \
        }                                                                       \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
    static inline enum ds_error                                                 \
    Prefix##_prepend(ListType list, Type value)                                 \
    {                                                                           \
        return Prefix##_push_front(list, value);                                \
    }                                                                           \
                                                                                \
    /* support of both `#_push_back` and `#_append` */                          \
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
                return DS_ERR_COPY_FAILED;                                      \
            }                                                                   \
        }                                                                       \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
    static inline enum ds_error                                                 \
    Prefix##_append(ListType list, Type value)                                  \
    {                                                                           \
        return Prefix##_push_back(list, value);                                 \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_push_at(ListType list, Type value, long long index)                \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_push_at(list._nodes, &data, index)                            \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!list.copy)                                                         \
            *((Type *)data) = value;                                            \
        else                                                                    \
        {                                                                       \
            if ( !list.copy(data, &value) )                                     \
            {                                                                   \
                ds_nc_pop_at(list._nodes, NULL, index, NULL);                   \
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
    Prefix##_drop_at(ListType list, long long index)                            \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_pop_at(list._nodes, NULL, index, list.destroy)                \
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
    Prefix##_pop_at(ListType list, Type *out, long long index)                  \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_pop_at(list._nodes, &data, index, list.destroy)               \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
/* end of macro */

#endif //LIBDS_LIST_DEF_H
