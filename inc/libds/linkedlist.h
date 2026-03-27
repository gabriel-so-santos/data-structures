//
// Created by Gabriel Souza on 17/02/2026.
//

#ifndef DATA_STRUCTURES_LINKEDLIST_H
#define DATA_STRUCTURES_LINKEDLIST_H

#include <stddef.h>
#include <libds/core.h>
#include <libds/err.h>

#define DS_DEFINE_LIST(Type, ListName, destructor_func)                                                                \
                                                                                                                       \
    typedef TypeWrapper ds_##ListName##_t;                                                                             \
                                                                                                                       \
    static inline ds_##ListName##_t                                                                                    \
    ds_##ListName##_create(void)                                                                                       \
    {                                                                                                                  \
        ds_##ListName##_t list = {                                                                                     \
            .value_size = sizeof(Type),                                                                                \
            .destructor = (destructor_func),                                                                           \
            .nodechain_ptr = ds__nc_alloc(),                                                                           \
        };                                                                                                             \
        return list;                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    static inline ds_err_t                                                                                             \
    ds_##ListName##_destroy(ds_##ListName##_t list)                                                                    \
    {                                                                                                                  \
        DS__CHECK_ERR(                                                                                                 \
            ds__nc_free(&list.nodechain_ptr, list.destructor)                                                          \
        );                                                                                                             \
    }                                                                                                                  \
                                                                                                                       \
    static inline ds_err_t                                                                                             \
    ds_##ListName##_first(ds_##ListName##_t list, Type *output_ptr)                                                    \
    {                                                                                                                  \
        DS__CHECK_ERR(                                                                                                 \
            ds__nc_get_front(list.nodechain_ptr, output_ptr, list.value_size)                                          \
        );                                                                                                             \
    }                                                                                                                  \
                                                                                                                       \
    static inline ds_err_t                                                                                             \
    ds_##ListName##_last(ds_##ListName##_t list, Type *output_ptr)                                                     \
    {                                                                                                                  \
        DS__CHECK_ERR(                                                                                                 \
            ds__nc_get_back(list.nodechain_ptr, output_ptr, list.value_size)                                           \
        );                                                                                                             \
    }                                                                                                                  \
                                                                                                                       \
    static inline ds_err_t                                                                                             \
    ds_##ListName##_append(ds_##ListName##_t list, Type value)                                                         \
    {                                                                                                                  \
        DS__CHECK_ERR(                                                                                                 \
            ds__nc_push_back(list.nodechain_ptr, &value, list.value_size)                                              \
        );                                                                                                             \
    }                                                                                                                  \
                                                                                                                       \
    static inline ds_err_t                                                                                             \
    ds_##ListName##_prepend(ds_##ListName##_t list, Type value)                                                        \
    {                                                                                                                  \
        DS__CHECK_ERR(                                                                                                 \
            ds__nc_push_front(list.nodechain_ptr, &value, list.value_size)                                             \
        );                                                                                                             \
    }                                                                                                                  \
                                                                                                                       \
    static inline ds_err_t                                                                                             \
    ds_##ListName##_insert(ds_##ListName##_t list, Type value, long long index)                                        \
    {                                                                                                                  \
        DS__CHECK_ERR(                                                                                                 \
            ds__nc_push_at(list.nodechain_ptr, &value, list.value_size, index)                                         \
        );                                                                                                             \
    }

#endif //DATA_STRUCTURES_LINKEDLIST_H