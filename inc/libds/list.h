//
// Created by Gabriel Souza on 17/02/2026.
//

#ifndef DATA_STRUCTURES_LIST_H
#define DATA_STRUCTURES_LIST_H

#include <stddef.h>
#include <stdbool.h>
#include <libds/core.h>
#include <libds/err.h>

#define DS_DEFINE_LIST(Type, Name, destructor_func)                             \
                                                                                \
    typedef TypeWrapper ds_##Name##_t;                                          \
                                                                                \
    static inline ds_##Name##_t                                                 \
    ds_##Name##_create(void)                                                    \
    {                                                                           \
        ds_##Name##_t list = {                                                  \
            .value_size = sizeof(Type),                                         \
            .destructor = (destructor_func),                                    \
            .nodechain_ptr = ds__nc_alloc(),                                    \
        };                                                                      \
        return list;                                                            \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_destroy(ds_##Name##_t *list)                                    \
    {                                                                           \
        DS__CHECK_ERR(                                                          \
            ds__nc_free(&list->nodechain_ptr, list->destructor)                 \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline size_t                                                        \
    ds_##Name##_length_of(ds_##ListName##_t list)                               \
    {                                                                           \
        return ds__nc_length_of(list.nodechain_ptr);                            \
    }                                                                           \
                                                                                \
    static inline bool                                                          \
    ds_##Name##_is_empty(ds_##Name##_t list)                                    \
    {                                                                           \
        return ds__nc_is_empty(list.nodechain_ptr);                             \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_first(ds_##Name##_t list, Type *output_ptr)                     \
    {                                                                           \
        DS__CHECK_ERR(                                                          \
            ds__nc_get_front(list.nodechain_ptr, output_ptr, list.value_size)   \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_last(ds_##Name##_t list, Type *output_ptr)                      \
    {                                                                           \
        DS__CHECK_ERR(                                                          \
            ds__nc_get_back(list.nodechain_ptr, output_ptr, list.value_size)    \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_get_at(ds_##ListName##_t list, Type *output_ptr, long long index)\
    {                                                                           \
        DS__CHECK_ERR(                                                          \
            ds__nc_get_at(                                                      \
                list.nodechain_ptr,                                             \
                output_ptr,                                                     \
                list.value_size,                                                \
                index                                                           \
            )                                                                   \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_append(ds_##Name##_t list, Type value)                          \
    {                                                                           \
        DS__CHECK_ERR(                                                          \
            ds__nc_push_back(list.nodechain_ptr, &value, list.value_size)       \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_prepend(ds_##Name##_t list, Type value)                         \
    {                                                                           \
        DS__CHECK_ERR(                                                          \
            ds__nc_push_front(list.nodechain_ptr, &value, list.value_size)      \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_insert(ds_##Name##_t list, Type value, long long index)         \
    {                                                                           \
        DS__CHECK_ERR(                                                          \
            ds__nc_push_at(list.nodechain_ptr, &value, list.value_size, index)  \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline ds_err_t                                                      \
    ds_##Name##_remove_first(ds_##ListName##_t list)                            \
    {                                                                           \
        DS__CHECK_ERR(                                                          \
            ds__pop_front(list.nodechain_ptr, list.destructor)                  \
        );                                                                      \
    }                                                                           \
    static inline ds_err_t                                                      \
    ds_##Name##_remove_last(ds_##ListName##_t list)                             \
    {                                                                           \
        DS__CHECK_ERR(                                                          \
            ds__pop_back(list.nodechain_ptr, list.destructor)                   \
        );                                                                      \
    }

#endif //DATA_STRUCTURES_LIST_H