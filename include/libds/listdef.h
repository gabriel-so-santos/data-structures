//
// Created by Gabriel Souza on 09/04/2026.
//

#ifndef DATA_STRUCTURES_LIST_DEF_H
#define DATA_STRUCTURES_LIST_DEF_H

#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>

#include "core.h"
#include "impl/nodechain.h"

#define LIBDS_DEFINE_LIST(Type, Prefix, ListType, CopyFunc, DestroyFunc)        \
                                                                                \
    typedef struct ListType                                                     \
    {                                                                           \
        const ds_copier_t      copy_fn;                                         \
        const ds_destructor_t  destroy_fn;                                      \
        struct ds_node_chain   *_chain_ptr; /* must NOT be modified directly */ \
    } ListType;                                                                 \
                                                                                \
    static inline ListType                                                      \
    Prefix##_create(void)                                                       \
    {                                                                           \
        size_t size  = sizeof(Type);                                            \
        size_t align = alignof(Type);                                           \
                                                                                \
        ListType list = {                                                       \
            .copy_fn     = (CopyFunc),                                          \
            .destroy_fn  = (DestroyFunc),                                       \
            ._chain_ptr  = ds_nc_alloc(size, align)                             \
        };                                                                      \
        return list;                                                            \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_delete(ListType *list)                                             \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_free(&list->_chain_ptr, list->destroy_fn)                     \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_clear(ListType list)                                               \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_clear(list._chain_ptr, list.destroy_fn)                       \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_copy(ListType dst_list, const ListType src_list)                   \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_copy(                                                         \
                dst_list._chain_ptr,                                            \
                src_list._chain_ptr,                                            \
                sizeof(Type),                                                   \
                dst_list.copy_fn,                                               \
                dst_list.destroy_fn                                             \
            )                                                                   \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_reverse(ListType list)                                             \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_reverse(list._chain_ptr)                                      \
        );                                                                      \
    }                                                                           \
                                                                                \
    /* support of both `#_length` and `#_size` */                               \
    static inline size_t                                                        \
    Prefix##_length(ListType list)                                              \
    {                                                                           \
        return ds_nc_length(list._chain_ptr);                                   \
    }                                                                           \
    static inline size_t                                                        \
    Prefix##_size(ListType list)                                                \
    {                                                                           \
        return Prefix##_length(list);                                           \
    }                                                                           \
                                                                                \
    static inline size_t                                                        \
    Prefix##_bytes(ListType list)                                               \
    {                                                                           \
        return ds_nc_bytes(list._chain_ptr);                                    \
    }                                                                           \
                                                                                \
    static inline bool                                                          \
    Prefix##_is_empty(ListType list)                                            \
    {                                                                           \
        return ds_nc_is_empty(list._chain_ptr);                                 \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                    \
    Prefix##_get_front(ListType list, Type *out_ptr)                            \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        enum ds_error error = LIBDS_CHECK(                                         \
            ds_nc_get_front(list._chain_ptr, &data_ptr)                         \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        *out_ptr = *((Type *)data_ptr);                                         \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                    \
    Prefix##_get_back(ListType list, Type *out_ptr)                             \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        enum ds_error error = LIBDS_CHECK(                                         \
            ds_nc_get_back(list._chain_ptr, &data_ptr)                          \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        *out_ptr = *((Type *)data_ptr);                                         \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                    \
    Prefix##_get_at(ListType list, Type *out_ptr, long long index)              \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        enum ds_error error = LIBDS_CHECK(                                         \
            ds_nc_get_at(list._chain_ptr, &data_ptr, index)                     \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        *out_ptr = *((Type *)data_ptr);                                         \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    /* support of both `#_push_front` and `#_prepend` */                        \
    static inline enum ds_error                                                    \
    Prefix##_push_front(ListType list, Type value)                              \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        enum ds_error error = LIBDS_CHECK(                                         \
            ds_nc_push_front(list._chain_ptr, &data_ptr)                        \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!list.copy_fn)                                                      \
            *((Type *)data_ptr) = value;                                        \
        else                                                                    \
        {                                                                       \
            if (!list.copy_fn(data_ptr, &value))                                \
            {                                                                   \
                ds_nc_pop_front(list._chain_ptr, NULL, NULL);                   \
                return DS_ERR_COPY_FAILED;                                      \
            }                                                                   \
        }                                                                       \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
    static inline enum ds_error                                                    \
    Prefix##_prepend(ListType list, Type value)                                 \
    {                                                                           \
        return Prefix##_push_front(list, value);                                \
    }                                                                           \
                                                                                \
    /* support of both `#_push_back` and `#_append` */                          \
    static inline enum ds_error                                                    \
    Prefix##_push_back(ListType list, Type value)                               \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        enum ds_error error = LIBDS_CHECK(                                         \
            ds_nc_push_back(list._chain_ptr, &data_ptr)                         \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!list.copy_fn)                                                      \
            *((Type *)data_ptr) = value;                                        \
        else                                                                    \
        {                                                                       \
            if (!list.copy_fn(data_ptr, &value))                                \
            {                                                                   \
                ds_nc_pop_back(list._chain_ptr, NULL, NULL);                    \
                return DS_ERR_COPY_FAILED;                                      \
            }                                                                   \
        }                                                                       \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
    static inline enum ds_error                                                    \
    Prefix##_append(ListType list, Type value)                                  \
    {                                                                           \
        return Prefix##_push_back(list, value);                                 \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                    \
    Prefix##_push_at(ListType list, Type value, long long index)                \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        enum ds_error error = LIBDS_CHECK(                                         \
            ds_nc_push_at(list._chain_ptr, &data_ptr, index)                    \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!list.copy_fn)                                                      \
            *((Type *)data_ptr) = value;                                        \
        else                                                                    \
        {                                                                       \
            if (!list.copy_fn(data_ptr, &value))                                \
            {                                                                   \
                ds_nc_pop_at(list._chain_ptr, NULL, NULL, index);               \
                return DS_ERR_COPY_FAILED;                                      \
            }                                                                   \
        }                                                                       \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                    \
    Prefix##_drop_front(ListType list)                                          \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_pop_front(list._chain_ptr, list.destroy_fn, NULL)             \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                    \
    Prefix##_drop_back(ListType list)                                           \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_pop_back(list._chain_ptr, list.destroy_fn, NULL)              \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                    \
    Prefix##_drop_at(ListType list, long long index)                            \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_pop_at(list._chain_ptr, list.destroy_fn, NULL, index)         \
        );                                                                      \
    }                                                                           \
    static inline enum ds_error                                                    \
    Prefix##_pop_front(ListType list, Type *out_ptr)                            \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        enum ds_error error = LIBDS_CHECK(                                         \
            ds_nc_pop_front(list._chain_ptr, list.destroy_fn, &data_ptr)        \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out_ptr) *out_ptr = *((Type *)data_ptr);                            \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                    \
    Prefix##_pop_back(ListType list, Type *out_ptr)                             \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        enum ds_error error = LIBDS_CHECK(                                         \
            ds_nc_pop_back(list._chain_ptr, list.destroy_fn, &data_ptr)         \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out_ptr) *out_ptr = *((Type *)data_ptr);                            \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                    \
    Prefix##_pop_at(ListType list, Type *out_ptr, long long index)              \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        enum ds_error error = LIBDS_CHECK(                                         \
            ds_nc_pop_at(list._chain_ptr, list.destroy_fn, &data_ptr, index)    \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out_ptr) *out_ptr = *((Type *)data_ptr);                            \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
/* end of macro */

#endif //DATA_STRUCTURES_LIST_DEF_H
