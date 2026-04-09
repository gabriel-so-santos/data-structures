//
// Created by Gabriel Souza on 17/02/2026.
//

#ifndef DATA_STRUCTURES_LIST_H
#define DATA_STRUCTURES_LIST_H

#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>

#include "libds/core.h"
#include "internal/nodechain.h"

#define LIBDS_DEFINE_LIST(Type, Prefix, ListType, CopyFunc, DestroyFunc)        \
                                                                                \
    typedef struct ListType                                                     \
    {                                                                           \
        const size_t           value_size;                                      \
        const size_t           value_align;                                     \
        const ds_copier_t      copy_fn;                                         \
        const ds_destructor_t  destroy_fn;                                      \
        struct ds_node_chain   *_chain_ptr; /* must not be directly modified */ \
    } ListType;                                                                 \
                                                                                \
    static inline ListType                                                      \
    Prefix##_create(void)                                                       \
    {                                                                           \
        size_t size  = sizeof(Type);                                            \
        size_t align = alignof(Type);                                           \
                                                                                \
        ListType list = {                                                       \
            .value_size  = size,                                                \
            .value_align = align,                                               \
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
                                                                                \
                dst_list.value_size,                                            \
                dst_list.value_align,                                           \
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
    static inline bool                                                          \
    Prefix##_is_empty(ListType list)                                            \
    {                                                                           \
        return ds_nc_is_empty(list._chain_ptr);                                 \
    }                                                                           \
                                                                                \
    static inline ds_error_t                                                    \
    Prefix##_get_front(ListType list, Type *out_ptr)                            \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        ds_error_t status = LIBDS_CHECK(                                        \
            ds_nc_get_front(list._chain_ptr, &data_ptr)                         \
        );                                                                      \
        if (status == DS_ERR_NONE)                                              \
            *out_ptr = *((Type *)data_ptr);                                     \
                                                                                \
        return status;                                                          \
    }                                                                           \
                                                                                \
    static inline ds_error_t                                                    \
    Prefix##_get_back(ListType list, Type *out_ptr)                             \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        ds_error_t status = LIBDS_CHECK(                                        \
            ds_nc_get_back(list._chain_ptr, &data_ptr)                          \
        );                                                                      \
        if (status == DS_ERR_NONE)                                              \
            *out_ptr = *((Type *)data_ptr);                                     \
                                                                                \
        return status;                                                          \
    }                                                                           \
                                                                                \
    static inline ds_error_t                                                    \
    Prefix##_get_at(ListType list, Type *out_ptr, long long index)              \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        ds_error_t status = LIBDS_CHECK(                                        \
            ds_nc_get_at(list._chain_ptr, &data_ptr, index)                     \
        );                                                                      \
        if (status == DS_ERR_NONE)                                              \
            *out_ptr = *((Type *)data_ptr);                                     \
                                                                                \
        return status;                                                          \
    }                                                                           \
                                                                                \
    /* support of both `#_push_front` and `#_prepend` */                        \
    static inline ds_error_t                                                    \
    Prefix##_push_front(ListType list, Type value)                              \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        ds_error_t status = LIBDS_CHECK(                                        \
            ds_nc_push_front(list._chain_ptr, &data_ptr)                        \
        );                                                                      \
        if (status == DS_ERR_NONE)                                              \
            *((Type *)data_ptr) = value;                                        \
                                                                                \
        return status;                                                          \
    }                                                                           \
    static inline ds_error_t                                                    \
    Prefix##_prepend(ListType list, Type value)                                 \
    {                                                                           \
        return Prefix##_push_front(list, value);                                \
    }                                                                           \
                                                                                \
    /* support of both `#_push_back` and `#_append` */                          \
    static inline ds_error_t                                                    \
    Prefix##_push_back(ListType list, Type value)                               \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        ds_error_t status = LIBDS_CHECK(                                        \
            ds_nc_push_back(list._chain_ptr, &data_ptr)                         \
        );                                                                      \
        if (status == DS_ERR_NONE)                                              \
            *((Type *)data_ptr) = value;                                        \
                                                                                \
        return status;                                                          \
    }                                                                           \
    static inline ds_error_t                                                    \
    Prefix##_append(ListType list, Type value)                                  \
    {                                                                           \
        return Prefix##_push_back(list, value);                                 \
    }                                                                           \
                                                                                \
    static inline ds_error_t                                                    \
    Prefix##_push_at(ListType list, Type value, long long index)                \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        ds_error_t status = LIBDS_CHECK(                                        \
            ds_nc_push_at(list._chain_ptr, &data_ptr, index)                    \
        );                                                                      \
        if (status == DS_ERR_NONE)                                              \
            *((Type *)data_ptr) = value;                                        \
                                                                                \
        return status;                                                          \
    }                                                                           \
                                                                                \
    static inline ds_error_t                                                    \
    Prefix##_drop_front(ListType list)                                          \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_drop_front(list._chain_ptr, list.destroy_fn)                  \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline ds_error_t                                                    \
    Prefix##_drop_back(ListType list)                                           \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_drop_back(list._chain_ptr, list.destroy_fn)                   \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline ds_error_t                                                    \
    Prefix##_drop_at(ListType list, long long index)                            \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_drop_at(list._chain_ptr, list.destroy_fn, index)              \
        );                                                                      \
    }                                                                           \
/* end of macro */

#endif //DATA_STRUCTURES_LIST_H
