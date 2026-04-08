//
// Created by Gabriel Souza on 17/02/2026.
//

#ifndef DATA_STRUCTURES_LIST_H
#define DATA_STRUCTURES_LIST_H

#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>
#include <libds/core.h>
#include <libds/err.h>

#define DS_DEFINE_LIST(Type, Name, destructor_fn, copier_fn)                    \
                                                                                \
    typedef struct ds_##Name##                                                  \
    {                                                                           \
        const size_t value_size;                                                \
        const size_t value_align;                                               \
        const Destructor destructor;                                            \
        const Copier copier;                                                    \
        NodeChain *nodechain_ptr;                                               \
    } ds_##Name##_t;                                                            \
                                                                                \
    static inline ds_##Name##_t                                                 \
    ds_##Name##_create(void)                                                    \
    {                                                                           \
        size_t size = sizeof(Type);                                             \
        size_t align = alignof(Type);                                           \
                                                                                \
        ds_##Name##_t list = {                                                  \
            .value_size = size,                                                 \
            .value_align = align,                                               \
            .destructor = (destructor_fn),                                      \
            .copier = (copier_fn),                                              \
            .nodechain_ptr = ds__nc_alloc(size, align),                         \
        };                                                                      \
        return list;                                                            \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_destroy(ds_##Name##_t *list)                                    \
    {                                                                           \
        return DS_CHECK(                                                        \
            ds__nc_free(&list->nodechain_ptr, list->destructor)                 \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_clear(ds_##Name##_t list)                                       \
    {                                                                           \
        return DS_CHECK(                                                        \
            ds__nc_clear(list.nodechain_ptr, list.destructor)                   \
        );                                                                      \
    }                                                                           \
    static inline ds_err_t                                                      \
    ds_##Name##_assign(ds_##Name##_t dst_list, const ds_##Name##_t src_list)    \
    {                                                                           \
        return DS_CHECK(                                                        \
            ds__nc_assign(                                                      \
                dst_list.nodechain_ptr,                                         \
                src_list.nodechain_ptr,                                         \
                dst_list.value_size,                                            \
                dst_list.value_align,                                           \
                dst_list.destructor,                                            \
                dst_list.copier                                                 \
            )                                                                   \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_reverse(ds_##Name##_t list)                                     \
    {                                                                           \
        return DS_CHECK(                                                        \
            ds__nc_reverse(list.nodechain_ptr)                                  \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline size_t                                                        \
    ds_##Name##_length(ds_##Name##_t list)                                      \
    {                                                                           \
        return ds__nc_length(list.nodechain_ptr);                               \
    }                                                                           \
                                                                                \
    static inline bool                                                          \
    ds_##Name##_isempty(ds_##Name##_t list)                                     \
    {                                                                           \
        return ds__nc_isempty(list.nodechain_ptr);                              \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_first(ds_##Name##_t list, Type *output_ptr)                     \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        ds_err_t status = DS_CHECK(                                             \
            ds__nc_get_front(list.nodechain_ptr, &data_ptr)                     \
        );                                                                      \
        if (status == LIBDS_SUCCESS)                                            \
            *output_ptr = *((Type *)data_ptr);                                  \
        return status;                                                          \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_last(ds_##Name##_t list, Type *output_ptr)                      \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        ds_err_t status = DS_CHECK(                                             \
            ds__nc_get_back(list.nodechain_ptr, &data_ptr)                      \
        );                                                                      \
        if (status == LIBDS_SUCCESS)                                            \
            *output_ptr = *((Type *)data_ptr);                                  \
                                                                                \
        return status;                                                          \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_get(ds_##Name##_t list, Type *output_ptr, long long index)      \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        ds_err_t status = DS_CHECK(                                             \
            ds__nc_get_at(list.nodechain_ptr, &data_ptr, index)                 \
        );                                                                      \
        if (status == LIBDS_SUCCESS)                                            \
            *output_ptr = *((Type *)data_ptr);                                  \
                                                                                \
        return status;                                                          \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_prepend(ds_##Name##_t list, Type value)                         \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        ds_err_t status = DS_CHECK(                                             \
            ds__nc_push_front(list.nodechain_ptr, &data_ptr)                    \
        );                                                                      \
        if (status == LIBDS_SUCCESS)                                            \
            *((Type *)data_ptr) = value;                                        \
                                                                                \
        return status;                                                          \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_append(ds_##Name##_t list, Type value)                          \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        ds_err_t status = DS_CHECK(                                             \
            ds__nc_push_back(list.nodechain_ptr, &data_ptr)                     \
        );                                                                      \
        if (status == LIBDS_SUCCESS)                                            \
            *((Type *)data_ptr) = value;                                        \
                                                                                \
        return status;                                                          \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_insert(ds_##Name##_t list, Type value, long long index)         \
    {                                                                           \
        void *data_ptr = NULL;                                                  \
        ds_err_t status = DS_CHECK(                                             \
            ds__nc_push_at(list.nodechain_ptr, &data_ptr, index)                \
        );                                                                      \
        if (status == LIBDS_SUCCESS)                                            \
            *((Type *)data_ptr) = value;                                        \
                                                                                \
        return status;                                                          \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_dropfirst(ds_##Name##_t list)                                   \
    {                                                                           \
        return DS_CHECK(                                                        \
            ds__nc_drop_front(list.nodechain_ptr, list.destructor)              \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_droplast(ds_##Name##_t list)                                    \
    {                                                                           \
        return DS_CHECK(                                                        \
            ds__nc_drop_back(list.nodechain_ptr, list.destructor)               \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_remove(ds_##Name##_t list, long long index)                     \
    {                                                                           \
        return DS_CHECK(                                                        \
            ds__nc_drop_at(list.nodechain_ptr, list.destructor, index)          \
        );                                                                      \
    }

#endif //DATA_STRUCTURES_LIST_H
