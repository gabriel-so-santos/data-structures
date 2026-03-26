//
// Created by Gabriel Souza on 17/02/2026.
//

#ifndef DATA_STRUCTURES_LINKEDLIST_H
#define DATA_STRUCTURES_LINKEDLIST_H

#include <stddef.h>
#include "dslib/core.h"
#include "dslib/err.h"

#define DEFINE_TYPED_LINKEDLIST(Type, ListName, destructor_func)                                                       \
                                                                                                                       \
    typedef TypeWrapper ListName;                                                                                      \
                                                                                                                       \
    static inline ListName new_##ListName(void)                                                                            \
    {                                                                                                                  \
        ListName list = {                                                                                              \
            .value_size = sizeof(Type),                                                                                \
            .destructor = (destructor_func),                                                                           \
            .nodechain_ptr = _dslib_nc_alloc(),                                                                        \
        };                                                                                                             \
        return list;                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    static inline dslib_err_t ListName##_free(ListName list)                                                           \
    {                                                                                                                  \
        return _dslib_nc_free(&list.nodechain_ptr, list.destructor);                                                    \
    }                                                                                                                  \
                                                                                                                       \
    static inline dslib_err_t ListName##_first(ListName list, Type *output_ptr)                                        \
    {                                                                                                                  \
        return _dslib_nc_get_front(list.nodechain_ptr, output_ptr, list.value_size);                                   \
    }                                                                                                                  \
                                                                                                                       \
    static inline dslib_err_t ListName##_last(ListName list, Type *output_ptr)                                         \
    {                                                                                                                  \
        return _dslib_nc_get_back(list.nodechain_ptr, output_ptr, list.value_size);                                    \
    }                                                                                                                  \
                                                                                                                       \
    static inline dslib_err_t ListName##_append(ListName list, Type value)                                             \
    {                                                                                                                  \
        return _dslib_nc_push_back(list.nodechain_ptr, &value, list.value_size);                                       \
    }                                                                                                                  \
                                                                                                                       \
    static inline dslib_err_t ListName##_prepend(ListName list, Type value)                                            \
    {                                                                                                                  \
        return _dslib_nc_push_front(list.nodechain_ptr, &value, list.value_size);                                      \
    }                                                                                                                  \
                                                                                                                       \
    static inline dslib_err_t ListName##_insert(ListName list, Type value, long long index)                            \
    {                                                                                                                  \
        return _dslib_nc_push_at(list.nodechain_ptr, &value, index, list.value_size);                                  \
    }

#endif //DATA_STRUCTURES_LINKEDLIST_H